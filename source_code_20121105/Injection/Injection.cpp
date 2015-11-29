#include "StdAfx.h"
#include "Injection.h"
#include "PdbHelper.h"
#include "DataStructure.h"
#include "SymbolAddressCache.h"
#include "Utility.h"

#if defined(_WIN64)
#define DBG_HELP_DLL _T("x64\\dbg64.dll")
#else
#define DBG_HELP_DLL _T("x86\\dbg32.dll")
#endif

#if defined(_WIN64)
#define SYMCHK_EXE _T("x64\\symchk.exe")
#else
#define SYMCHK_EXE _T("x86\\symchk.exe")
#endif

ICorJitCompiler::PFN_compileMethod ICorJitCompiler::s_pfnComplieMethod = NULL;

HANDLE CInjection::s_hEvent = ::CreateEventW( NULL, TRUE, FALSE, NULL);;
ICorJitCompiler * CInjection::s_pCorJitCompiler = NULL;
std::map< CORINFO_METHOD_HANDLE, ILCodeBuffer> CInjection::s_mpILBuffers;
HOOK_TRACE_INFO CInjection::s_hHookCompileMethod = {0};
INT CInjection::s_nStatus = CInjection::Status_Uninitialized;





//----------Exported-------------
extern "C"
BOOL __declspec (dllexport) __stdcall 
UpdateILCodes( MethodTable * pMethodTable, CORINFO_METHOD_HANDLE pMethodHandle, mdMethodDef md, LPBYTE pBuffer, DWORD dwSize)
{
	return CInjection::StartUpdateILCodes(pMethodTable, pMethodHandle, md, pBuffer, dwSize);
}

// enum{ Status_Uninitialized = 0, Status_Ready = 1, Status_Error = -1 };
extern "C"
	INT __declspec (dllexport) __stdcall 
	GetStatus()
{
	return CInjection::GetStatus();
}

// wait until the initialization is completed
extern "C"
	INT __declspec (dllexport) __stdcall 
	WaitForIntializationCompletion()
{
	::WaitForSingleObject( CInjection::s_hEvent, INFINITE);
	return CInjection::GetStatus();
}
//----------Exported-------------




BOOL CInjection::StartUpdateILCodes( MethodTable * pMethodTable
	, CORINFO_METHOD_HANDLE pMethodHandle
	, mdMethodDef md
	, LPBYTE pBuffer
	, DWORD dwSize
	)
{
	if( s_nStatus != Status_Ready || !pMethodHandle )
		return FALSE;

	MethodDesc * pMethodDesc = (MethodDesc*)pMethodHandle;
	pMethodDesc->Reset();

	MethodDesc * pStripMethodDesc = pMethodDesc->StripMethodInstantiation();
	if( pStripMethodDesc )
		pStripMethodDesc->Reset();

	ILCodeBuffer tILCodeBuffer;
	tILCodeBuffer.pBuffer = pBuffer;
	tILCodeBuffer.dwSize = dwSize;
	tILCodeBuffer.bIsGeneric = FALSE;

	// this is a generic method
	if( pMethodDesc->ContainsGenericVariables() || pMethodDesc->HasClassOrMethodInstantiation() )
	{
		tILCodeBuffer.bIsGeneric = TRUE;
		MethodDesc * pWrappedMethodDesc = pMethodDesc->GetWrappedMethodDesc();
		if( pWrappedMethodDesc )
		{
			pWrappedMethodDesc->Reset();
		}

		// find out all the instantiations of this generic method
		Module * pModule = pMethodDesc->GetLoaderModule();
		AppDomain * pAppDomain = pMethodDesc->GetDomain();
		if( pModule )
		{
			LoadedMethodDescIterator * pLoadedMethodDescIter = new LoadedMethodDescIterator( pAppDomain, pModule, md);
			while(pLoadedMethodDescIter->Next())
			{
				MethodDesc * pMD = pLoadedMethodDescIter->Current();
				if( pMD )
					pMD->Reset();
			}
			delete pLoadedMethodDescIter;
		}
	}

	std::map< CORINFO_METHOD_HANDLE, ILCodeBuffer>::iterator iter = s_mpILBuffers.find(pMethodHandle);
	if( iter != s_mpILBuffers.end() )
	{
		LocalFree(iter->second.pBuffer);
		s_mpILBuffers.erase(iter);
	}

	
	s_mpILBuffers.insert( std::pair< CORINFO_METHOD_HANDLE, ILCodeBuffer>( pMethodHandle, tILCodeBuffer) );

	return TRUE;
}



CorJitResult __stdcall CInjection::compileMethod(ICorJitInfo * pJitInfo
	, CORINFO_METHOD_INFO * pCorMethodInfo
	, UINT nFlags
	, LPBYTE * pEntryAddress
	, ULONG * pSizeOfCode
	)
{
	ICorJitCompiler * pCorJitCompiler = (ICorJitCompiler *)this;
	LPBYTE pOriginalILCode = pCorMethodInfo->ILCode;
	unsigned int nOriginalSize = pCorMethodInfo->ILCodeSize;

	// find the method to be replaced
	ILCodeBuffer tILCodeBuffer = {0};
	if( pCorMethodInfo && GetStatus() == Status_Ready )
	{
		MethodDesc * pMethodDesc = (MethodDesc*)pCorMethodInfo->ftn;
		std::map< CORINFO_METHOD_HANDLE, ILCodeBuffer>::iterator iter = s_mpILBuffers.find((CORINFO_METHOD_HANDLE)pMethodDesc);

		// if the current method is not found, try to search its generic definition method
		if( iter == s_mpILBuffers.end() && pMethodDesc->HasClassOrMethodInstantiation() )
		{
			MethodDesc * pStripMD = pMethodDesc->StripMethodInstantiation();
			if( pStripMD )
				iter = s_mpILBuffers.find((CORINFO_METHOD_HANDLE)pStripMD);

			if( iter == s_mpILBuffers.end() )
			{
				MethodDesc * pWrappedMD = pMethodDesc->GetWrappedMethodDesc();
				if( pWrappedMD )
					iter = s_mpILBuffers.find((CORINFO_METHOD_HANDLE)pWrappedMD);
			}
		}

		if( iter != s_mpILBuffers.end() )
		{
			tILCodeBuffer = iter->second;
			pCorMethodInfo->ILCode = tILCodeBuffer.pBuffer;
			pCorMethodInfo->ILCodeSize = tILCodeBuffer.dwSize;
			if( !tILCodeBuffer.bIsGeneric )
				s_mpILBuffers.erase(iter);
		}
	}

	CorJitResult result = pCorJitCompiler->compileMethod( pJitInfo, pCorMethodInfo, nFlags, pEntryAddress, pSizeOfCode);

	if( tILCodeBuffer.pBuffer )
	{
		pCorMethodInfo->ILCode = pOriginalILCode;
		pCorMethodInfo->ILCodeSize = nOriginalSize;
		if( !tILCodeBuffer.bIsGeneric )
			LocalFree(tILCodeBuffer.pBuffer);		
	}

	return result;
}



DWORD __stdcall CInjection::Initialize(LPVOID lpParameter )
{
	USES_CONVERSION;

	HMODULE hInstance = (HMODULE)lpParameter;
	// get the current directory
	CString strCurrentDir = CUtility::GetCurrentDirectory();

	// dbghelp.dll
	{
		CString strDbgHelpDll;
		strDbgHelpDll.Format( _T("%s%s"), strCurrentDir, DBG_HELP_DLL);
		HMODULE hModule = ::LoadLibrary(strDbgHelpDll);
		if( hModule == NULL || !CPdbHelper::Initialize(hModule) )
		{
			s_nStatus = Status_Error_DbgHelpNotFound;
			SetEvent( s_hEvent );
			return FALSE;
		}
	}

	// find the JIT module
	g_hJitModule = GetModuleHandleA("clrjit.dll");
	if( !g_hJitModule )
		g_hJitModule = GetModuleHandleA("mscorjit.dll");
	if( g_hJitModule == NULL )
	{
		s_nStatus = Status_Error_JITNotFound;
		SetEvent( s_hEvent );
		return FALSE;
	}

	// find the CLR module
	g_hClrModule = GetModuleHandleA("clr.dll");
	if( !g_hClrModule )
		g_hClrModule = GetModuleHandleA("mscorwks.dll");
	if( g_hClrModule == NULL || !DetermineDotNetVersion() )
	{
		s_nStatus = Status_Error_CLRNotFound;
		SetEvent( s_hEvent );
		return FALSE;
	}	

	// try to quick load the symbol address base on the binary hash
	if( !CSymbolAddressCache::TryCache() )
	{
		// get the pdb directory
		CString strDestPath(strCurrentDir);
		{
			strDestPath.AppendFormat( _T("PDB_symbols\\") );
			::CreateDirectory(strDestPath, NULL);
		}

		// copy the JIT dll
		{
			TCHAR tszFilename[MAX_PATH] = {0};
			GetModuleFileName( g_hJitModule, tszFilename, MAX_PATH);

			::CopyFile( tszFilename, strDestPath + CUtility::GetFileName(tszFilename), FALSE);
		}

		// copy the CLR dll
		{
			TCHAR tszFilename[MAX_PATH] = {0};
			GetModuleFileName( g_hClrModule, tszFilename, MAX_PATH);

			::CopyFile( tszFilename, strDestPath + CUtility::GetFileName(tszFilename), FALSE);
		}

		// Set Environment Variable 
		{
			CString strVariable;
			strVariable.Format( _T("symsrv*symsrv.dll*%s*http://msdl.microsoft.com/download/symbols"), strDestPath);
			SetEnvironmentVariable( _T("_NT_SYMBOL_PATH"), strVariable.GetBuffer());
			strVariable.ReleaseBuffer();
		}


		if( !SearchMethodAddresses(T2W(strDestPath.GetBuffer())) )
		{
			// download the pdb
			// symchk.exe /if "C:\Windows\Microsoft.NET\Framework\v2.0.50727\mscorjit.dll" /s srv*G:\HookDotNet*http://msdl.microsoft.com/download/symbols
			CString strCmd;
			strCmd.Format( _T("\"%s%s\" /if \"%s*.dll\" /s symsrv*symsrv.dll*%s*http://msdl.microsoft.com/download/symbols")
				, strCurrentDir
				, SYMCHK_EXE
				, strDestPath
				, strDestPath
				);
			ATLTRACE( _T("\n%s"), strCmd);

			STARTUPINFO si = { sizeof(si) };
			si.dwFlags = STARTF_USESHOWWINDOW;
			si.wShowWindow = SW_HIDE;
			PROCESS_INFORMATION  pi = {0};
			BOOL bRet = CreateProcess( NULL
				, strCmd.GetBuffer()
				, NULL
				, NULL
				, FALSE
				, 0
				, NULL
				, strCurrentDir
				, &si
				, &pi
				);
			strCmd.ReleaseBuffer();
			if( !bRet )
			{
				s_nStatus = Status_Error_DownloadPDBFailed;
				SetEvent( s_hEvent );
				return FALSE;
			}

			WaitForSingleObject(pi.hProcess, INFINITE); 

			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);


			// find all the pdb files
			if( !SearchMethodAddresses(NULL) )
				SearchMethodAddresses(NULL, FALSE);
		}	

		// cache the address offset according to the binary hash
		if( ICorJitCompiler::s_pfnComplieMethod )
		{
			CSymbolAddressCache::GenerateJitCache();
		};

		if( MethodDesc::IsInitialized() && LoadedMethodDescIterator::IsInitialized() )
		{
			CSymbolAddressCache::GenerateClrCache();
		}
	}

	
	HookApi();
	SetEvent( s_hEvent );
	return TRUE;
}


void CInjection::Uninitialize(void)
{
	if( s_nStatus == Status_Ready )
	{
		s_nStatus = Status_Uninitialized;
		LhUninstallAllHooks();
	}
	
	std::map< CORINFO_METHOD_HANDLE, ILCodeBuffer>::iterator iter;
	for( iter = s_mpILBuffers.begin(); iter != s_mpILBuffers.end(); iter ++)
	{
		LocalFree(iter->second.pBuffer);
	}
	s_mpILBuffers.clear();

	if( s_hEvent )
	{
		CloseHandle(s_hEvent);
		s_hEvent = NULL;
	}
}

BOOL CInjection::SearchMethodAddresses(LPCWSTR lpwszSearchPath, BOOL bDisableSymSrv /* = TRUE */)
{
	BOOL bRet = TRUE;

	CPdbHelper tPdbHelper(lpwszSearchPath, bDisableSymSrv);

	LPVOID pAddr = tPdbHelper.GetJitCompileMethodAddress();
	if( !pAddr )
		return FALSE;

	// save the real address
	LPVOID* pDest = (LPVOID*)&ICorJitCompiler::s_pfnComplieMethod;
	*pDest = pAddr;
	
	return tPdbHelper.EnumClrSymbols() && MethodDesc::IsInitialized() && LoadedMethodDescIterator::IsInitialized();
}



BOOL CInjection::HookApi(void)
{
	if( !MethodDesc::IsInitialized() )
	{
		s_nStatus = Status_Error_MethodDescInitializationFailed;
		return FALSE;
	}

	if( !LoadedMethodDescIterator::IsInitialized() )
	{
		s_nStatus = Status_Error_LoadedMethodDescIteratorInitializationFailed;
		return FALSE;
	}

	PFN_compileMethod pfnCompileMethod = &CInjection::compileMethod;
	LPVOID * pAddr = (LPVOID*)&pfnCompileMethod;
	NTSTATUS ntStatus = LhInstallHook( (PVOID&)ICorJitCompiler::s_pfnComplieMethod 
		, *pAddr
		, NULL
		, &s_hHookCompileMethod
		);

	if( ntStatus != STATUS_SUCCESS )
	{
		s_nStatus = Status_Error_HookCompileMethodFailed;
		return FALSE;
	}


	ULONG ulThreadID = GetCurrentProcessId();
	LhSetExclusiveACL( &ulThreadID, 1, &s_hHookCompileMethod);


	
	s_nStatus = Status_Ready;
	return TRUE;
}




