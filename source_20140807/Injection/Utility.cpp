#include "StdAfx.h"
#include "Utility.h"
#include <wincrypt.h>

#define BUFSIZE 1024
#define MD5LEN  16



BOOL CUtility::GetFileVersion(LPWSTR lpszFile, VS_FIXEDFILEINFO * pFixedFileInfo)
{
	if( !pFixedFileInfo )
		return FALSE;

	DWORD  dwHandle = NULL;
	UINT   nSize = 0;
	
	
	DWORD  dwSize = GetFileVersionInfoSizeW( lpszFile, &dwHandle);
	if( dwSize == 0 )
		return FALSE;
	
	BYTE   szTempBuf[4096] = {0};
	LPVOID pData = szTempBuf;
	ATLVERIFY( dwSize < sizeof(szTempBuf) );

	if (GetFileVersionInfo( lpszFile, dwHandle, dwSize, pData))
	{
		LPBYTE pBuffer = NULL;
		if (VerQueryValueW( pData, L"\\",(VOID FAR* FAR*)&pBuffer,&nSize))
		{
			if (nSize > 0)
			{
				VS_FIXEDFILEINFO * pVerInfo = (VS_FIXEDFILEINFO *)pBuffer;
				memcpy_s( pFixedFileInfo, sizeof(VS_FIXEDFILEINFO), pBuffer, sizeof(VS_FIXEDFILEINFO));
				return TRUE;
			}
		}
	}
	return FALSE;
}


CString CUtility::GetFileName(LPCTSTR lpszFilePath)
{
	CString strRet(lpszFilePath);
	int nIndex = strRet.ReverseFind('\\');
	if( nIndex > 0 )
		strRet = strRet.Mid( nIndex + 1 );
	return strRet;
}


CString CUtility::GetCurrentDirectory(void)
{
	TCHAR tszCurPath[MAX_PATH] = {0};
	{
		int nLen = ::GetModuleFileName( g_hModule, tszCurPath, MAX_PATH);
		for( ; nLen > 0; nLen --)
		{
			if( tszCurPath[nLen-1] == '\\' )
			{
				tszCurPath[nLen] = '\0';
				break;
			}
		}
	}
	return tszCurPath;
}


CStringA CUtility::GetResourceContent(LPCSTR lpszResName)
{
	CStringA strContent;
	CHAR szTemp[4098] = {0};
	BOOL bRet = FALSE;

	HRSRC hRsrc = FindResourceA( g_hModule, lpszResName, "RT_RCDATA" );
	if( !hRsrc )
		hRsrc = FindResourceA( g_hModule, lpszResName, MAKEINTRESOURCEA(10));
	if( !hRsrc )
		return FALSE;

	DWORD dwSize = SizeofResource(g_hModule, hRsrc);
	if (dwSize == 0 || dwSize > (sizeof(szTemp) - 1) )
		return FALSE;

	HGLOBAL hGlobal = LoadResource( g_hModule, hRsrc);
	if( !hGlobal )
		return FALSE;

	LPVOID pBuffer = LockResource(hGlobal);
	if( pBuffer )
	{
		memcpy_s( szTemp, sizeof(szTemp), pBuffer, dwSize);
		strContent = szTemp;
	}

	FreeResource(hGlobal);

	return strContent;
}

void CUtility::WriteCache(LPCSTR lpszID, LPCSTR lpszContent)
{
	USES_CONVERSION;

	CString strFileName = CUtility::GetCurrentDirectory();
	strFileName.Append( _T("cache\\") );
	::CreateDirectory( strFileName, NULL);

	strFileName.Append( A2T(lpszID) );


	CAtlFile file;
	HRESULT hr = file.Create( strFileName, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, CREATE_ALWAYS);
	if( SUCCEEDED(hr) )
	{
		DWORD dwWritten = 0;
		file.Write( lpszContent, strlen(lpszContent), &dwWritten);
		file.Flush();
		file.Close();
	}
}


CStringA CUtility::ReadCache(LPCSTR lpszID)
{
	CStringA strContent;
	USES_CONVERSION;

	CString strFileName = CUtility::GetCurrentDirectory();
	strFileName.Append( _T("cache\\") );
	::CreateDirectory( strFileName, NULL);

	strFileName.Append( A2T(lpszID) );


	CAtlFile file;
	HRESULT hr = file.Create( strFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING);
	if( FAILED(hr) )
		return "";

	ULONGLONG ullSize = 0;
	file.GetSize(ullSize);
	LPSTR pStr = (LPSTR)calloc(ullSize + 1, 1);
	file.Read( pStr, ullSize);
	strContent = pStr;
	free(pStr);
	file.Close();

	return strContent;
}