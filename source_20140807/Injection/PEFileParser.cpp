#include "StdAfx.h"
#include "Injection.h"
#include "PEFileParser.h"
#include "Utility.h"

typedef struct _json_memory_chunk_t {
	char *memory;
	size_t size;
} json_memory_chunk_t;


#define CV_SIGNATURE_NB10   '01BN'
#define CV_SIGNATURE_RSDS   'SDSR'

// CodeView header 
typedef struct CV_HEADER 
{
	DWORD CvSignature; // NBxx
	LONG  Offset;      // Always 0 for NB10
} * PCV_HEADER;

// CodeView NB10 debug information 
// (used when debug information is stored in a PDB 2.00 file) 
typedef struct CV_INFO_PDB20 
{
	CV_HEADER  Header; 
	DWORD      Signature;       // seconds since 01.01.1970
	DWORD      Age;             // an always-incrementing value 
	BYTE       PdbFileName[1];  // zero terminated string with the name of the PDB file 
} * PCV_INFO_PDB20;

// CodeView RSDS debug information 
// (used when debug information is stored in a PDB 7.00 file) 
typedef struct CV_INFO_PDB70 
{
	DWORD      CvSignature; 
	GUID       Signature;       // unique identifier 
	DWORD      Age;             // an always-incrementing value 
	BYTE       PdbFileName[1];  // zero terminated string with the name of the PDB file 
} * PCV_INFO_PDB70;


static LPBYTE GetRVAOffset(LPBYTE pBuffer, DWORD dwVirtualOffset);
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);
static CStringA DownloadFromServer( LPSTR szDllFile
	, LPSTR szPdbFileName 
	, LPSTR szPdbGuid
	, LPSTR szChecksum
	, LPSTR lpszMethods []
);

// this export method is only intended to be used by sample application to collect the installed .NET versions
extern "C"
	void __declspec (dllexport) __stdcall 
	__CollectAddress(LPSTR lpszDllFile)
{
	try
	{
		GetVirtualAddresses(lpszDllFile, NULL);
	}
	catch(...)
	{
	}
} 

VOID GetPdbFileInfo(LPCSTR lpszFilePath
	, LPSTR szPdbFileName
	, DWORD dwPdbFileNameCch
	, LPSTR szPdbGuid
	, DWORD dwPdbGuidCch
	, LPSTR szChecksum
	, DWORD dwChecksumCch
	)
{
	CStringA strError;

	// read the file into memory
	HANDLE hFile = ::CreateFileA( lpszFilePath
		, GENERIC_READ
		, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE
		, NULL
		, OPEN_EXISTING
		, FILE_ATTRIBUTE_NORMAL
		, NULL
		);
	if( hFile == INVALID_HANDLE_VALUE )
	{
		strError.Format( "Failed to read file [%s].", lpszFilePath);
		throw (LPCSTR)strError;
	}

	DWORD dwSize = GetFileSize( hFile, NULL);
	if( dwSize < 4096 ) // hardcode for file size limit
	{
		::CloseHandle(hFile);
		strError.Format( "The size of file [%s] is too small.", lpszFilePath);
		throw (LPCSTR)strError;
	}

	LPBYTE pBuffer = (LPBYTE)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
	ATLVERIFY(pBuffer);

	DWORD dwTotalRead = 0, dwRead = 0;
	while( dwTotalRead < dwSize &&
		ReadFile( hFile, pBuffer + dwTotalRead, dwSize - dwTotalRead, &dwRead, NULL) )
	{
		dwTotalRead += dwRead;
	}
	ATLVERIFY(dwTotalRead == dwSize);
	::CloseHandle(hFile);

	// start parse
	BOOL bFound = FALSE;

	// locate the DEBUG section
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pBuffer;
	if( pDosHeader->e_magic == IMAGE_DOS_SIGNATURE )
	{
		PIMAGE_DATA_DIRECTORY pDataDic;
		PIMAGE_NT_HEADERS32  pNtHeader32 = (PIMAGE_NT_HEADERS32)(pBuffer + pDosHeader->e_lfanew);
		PIMAGE_NT_HEADERS64  pNtHeader64 = (PIMAGE_NT_HEADERS64)(pBuffer + pDosHeader->e_lfanew);
		if( pNtHeader32->Signature == IMAGE_NT_SIGNATURE )
		{
			BOOL bIsX64 = pNtHeader32->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC;
			if( !bIsX64 )
				pDataDic = &pNtHeader32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
			else
				pDataDic = &pNtHeader64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];

			// http://www.debuginfo.com/articles/debuginfomatch.html
			if( pDataDic && pDataDic->Size > 0 )
			{
				//The number of entries in the debug directory can be obtained by dividing the size of the debug directory (as specified in the optional header¡¯s data directory entry) by the size of IMAGE_DEBUG_DIRECTORY structure.
				int nNumberOfEntries = pDataDic->Size / sizeof(IMAGE_DEBUG_DIRECTORY);
				PIMAGE_DEBUG_DIRECTORY pDebugDic = (PIMAGE_DEBUG_DIRECTORY)GetRVAOffset( pBuffer, pDataDic->VirtualAddress);
				
				for( int i = 0; i < nNumberOfEntries && !bFound ; i++)
				{
					// CodeView debug information (stored in the executable) or Program Database debug information (stored in PDB file)
					if( pDebugDic->Type == IMAGE_DEBUG_TYPE_CODEVIEW )
					{
						LPBYTE pDebugData = pBuffer + pDebugDic->PointerToRawData;
						DWORD dwCVSignature = *(LPDWORD)pDebugData; 
						if( dwCVSignature == CV_SIGNATURE_RSDS  )
						{
							PCV_INFO_PDB70 pCvInfo = (PCV_INFO_PDB70)pDebugData; 
							sprintf_s( szPdbGuid, dwPdbFileNameCch
								, "%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X%d"
								, pCvInfo->Signature.Data1
								, pCvInfo->Signature.Data2
								, pCvInfo->Signature.Data3
								, pCvInfo->Signature.Data4[0]
								, pCvInfo->Signature.Data4[1]
								, pCvInfo->Signature.Data4[2]
								, pCvInfo->Signature.Data4[3]
								, pCvInfo->Signature.Data4[4]
								, pCvInfo->Signature.Data4[5]
								, pCvInfo->Signature.Data4[6]
								, pCvInfo->Signature.Data4[7]
								, pCvInfo->Age
								);

							strcpy_s( szPdbFileName, dwPdbFileNameCch, (LPCSTR)pCvInfo->PdbFileName);

							if( bIsX64 )
							{
								sprintf_s( szChecksum, dwChecksumCch
									, "%x%x"
									, pNtHeader64->FileHeader.TimeDateStamp
									, pNtHeader64->OptionalHeader.SizeOfImage
									);
							}
							else
							{
								sprintf_s( szChecksum, dwChecksumCch
									, "%x%x"
									, pNtHeader32->FileHeader.TimeDateStamp
									, pNtHeader32->OptionalHeader.SizeOfImage
									);
							}

							bFound = TRUE;
						}
					}

					pDebugDic++;
				}
			}
		}
	}

	HeapFree( GetProcessHeap(), 0, pBuffer);

	if( !bFound )
	{
		strError.Format( "Unable to find symbol ID for [%s].", lpszFilePath);
		throw (LPCSTR)strError;
	}
}

LPBYTE GetRVAOffset(LPBYTE pBuffer, DWORD dwVirtualOffset)  
{
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pBuffer;
	PIMAGE_NT_HEADERS32  pNtHeader32 = (PIMAGE_NT_HEADERS32)(pBuffer + pDosHeader->e_lfanew);
	PIMAGE_NT_HEADERS64  pNtHeader64 = (PIMAGE_NT_HEADERS64)(pBuffer + pDosHeader->e_lfanew);
	BOOL bIsX64 = pNtHeader32->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC;

	int nSectionNum = bIsX64 ? pNtHeader64->FileHeader.NumberOfSections : pNtHeader32->FileHeader.NumberOfSections;
	PIMAGE_SECTION_HEADER pSection = (PIMAGE_SECTION_HEADER)(pBuffer + pDosHeader->e_lfanew + (bIsX64 ? sizeof(IMAGE_NT_HEADERS64) : sizeof(IMAGE_NT_HEADERS32)));

	// search for absolute offset
	for( int i = 0; i < nSectionNum; i++)
	{
		DWORD dwStart = pSection->VirtualAddress;  
		if( dwStart <= dwVirtualOffset && dwVirtualOffset < dwStart + pSection->SizeOfRawData){  
			return pBuffer + pSection->PointerToRawData + (dwVirtualOffset - dwStart);  
		}
		pSection++;
	}

    return 0;  
}  

CStringA DownloadFromServer( LPSTR szDllFile
	, LPSTR szPdbFileName 
	, LPSTR szPdbGuid
	, LPSTR szChecksum
	, LPSTR lpszMethods []
	)
{
	CStringA strContent;

	CURL *curl = curl_easy_init();
	CURLcode res;

	json_memory_chunk_t chunk;
	chunk.memory = (char*)calloc(1, 1);
	chunk.size = 0;

	CStringA strParameters;
	{
		LPSTR pGuid = curl_easy_escape( curl, szPdbGuid, 0);
		LPSTR pChecksum = curl_easy_escape( curl, szChecksum, 0);
		LPSTR pPDB = curl_easy_escape( curl, szPdbFileName, 0);

		CHAR * szFilename = &szDllFile[strlen(szDllFile) - 1];
		while( *(szFilename - 1) != '/' && *(szFilename - 1) != '\\' )
			szFilename --;
		LPSTR pFilename = curl_easy_escape( curl, szFilename, 0);
		strParameters.Format( "filename=%s&checksum=%s&pdb=%s&guid=%s"
			, pFilename
			, pChecksum
			, pPDB
			, pGuid
			);
		curl_free(pGuid);
		curl_free(pChecksum);
		curl_free(pPDB);
		curl_free(pFilename);

		if( lpszMethods ){
			int nIdx = 0;
			while( lpszMethods[nIdx] ){
				LPSTR pMethod = curl_easy_escape( curl, lpszMethods[nIdx], 0);
				strParameters.AppendFormat( "&method%d=%s"
					, ++nIdx
					, pMethod
					);

				curl_free(pMethod);
			}
		}
	}

	curl_easy_setopt(curl, CURLOPT_HTTPPOST, 1);
	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 240);
	curl_easy_setopt(curl, CURLOPT_URL, "http://109.205.93.50:8080");
	curl_easy_setopt(curl, CURLOPT_NOPROXY, "*");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strParameters.GetBuffer());

	res = curl_easy_perform(curl);

	CStringA strError;
	if(res == CURLE_OK) 
	{
		long lHttpCode = 0;
		curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &lHttpCode);
		if( lHttpCode == 200 )
			strContent = chunk.memory;
		else
		{
			strError = chunk.memory;
		}
	}
	else
	{
		strError.Format("Failed to get addresses from server, %s.", curl_easy_strerror(res));
	}

	strParameters.ReleaseBuffer();
	curl_easy_cleanup(curl);
	if(chunk.memory)
		free(chunk.memory);

	if( !strError.IsEmpty() )
		throw (LPCSTR)strError;
	
	return strContent;
}

CStringA GetVirtualAddresses(LPSTR szDllFile, LPSTR lpszMethods [])
{
	CHAR szPdbFileName[100] = {0};
	CHAR szPdbGuid[100] = {0};
	CHAR szChecksum[100] = {0};
	GetPdbFileInfo( szDllFile
		, szPdbFileName
		, sizeof(szPdbFileName)
		, szPdbGuid
		, sizeof(szPdbGuid)
		, szChecksum
		, sizeof(szChecksum)
		);


	// search in resource
	CStringA strContent = CUtility::GetResourceContent(szPdbGuid);

	// if not found, try local cache
	if( strContent.IsEmpty() )
	{
		strContent = CUtility::ReadCache(szPdbGuid);
		// if still not found, get from server
		if( strContent.IsEmpty() )
		{
			strContent = DownloadFromServer( szDllFile, szPdbFileName, szPdbGuid, szChecksum, lpszMethods);
			if( !strContent.IsEmpty() )
				CUtility::WriteCache( szPdbGuid, strContent);
		}
	}

	if( strContent.IsEmpty() )
		throw "Unable to load the virtual addresses";

	// ensure there is a line-break ended.
	if( strContent[strContent.GetLength() - 1] != '\n' )
		strContent.AppendChar('\n');

	return strContent;
}

std::map<CStringA, ULONGLONG> GetAddresses(HMODULE hModule, LPSTR lpszMethods [])
{
	CHAR szDllFile[MAX_PATH] = {0};
	GetModuleFileNameA( hModule, szDllFile, MAX_PATH);

	CStringA strContent = GetVirtualAddresses( szDllFile, lpszMethods);

	std::map<CStringA, ULONGLONG> map;
	int nLast = 0;
	int nFind = strContent.Find("\n");
	while( nFind > 0 )
	{
		CStringA strLine = strContent.Mid( nLast, nFind - nLast);
		nLast = nFind + 1;
		nFind = strContent.Find("\n", nLast);

		int nIndex = strLine.ReverseFind('=');
		if( nIndex <= 0 )
			continue;

		CStringA strName = strLine.Mid( 0, nIndex);
		ULONGLONG ullVA = (ULONGLONG)_atoi64(strLine.Mid( nIndex + 1 ));

		map.insert( std::pair<CStringA, ULONGLONG>( strName, ullVA + (ULONGLONG)hModule) );
	}
	
	return map;
}

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	json_memory_chunk_t *mem = (json_memory_chunk_t *)userp;

	mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
	ATLVERIFY(mem->memory);

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

