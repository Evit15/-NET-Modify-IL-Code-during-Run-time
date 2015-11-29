#include "StdAfx.h"
#include "Injection.h"
#include "DataStructure.h"
#include "Utility.h"
#include "PEFileParser.h"


ICorJitCompiler::PFN_compileMethod ICorJitCompiler::s_pfnComplieMethod = NULL;

HANDLE CInjection::s_hEvent = ::CreateEventW( NULL, TRUE, FALSE, NULL);;
ICorJitCompiler * CInjection::s_pCorJitCompiler = NULL;
std::map< CORINFO_METHOD_HANDLE, ILCodeBuffer> CInjection::s_mpILBuffers;
HOOK_TRACE_INFO CInjection::s_hHookCompileMethod = {0};
BOOL CInjection::s_bIsSucceeded = FALSE;
CHAR CInjection::s_szLastError[LAST_ERROR_MSG_SIZE] = {0};




//----------Exported-------------
extern "C"
BOOL __declspec (dllexport) __stdcall 
UpdateILCodes( MethodTable * pMethodTable, CORINFO_METHOD_HANDLE pMethodHandle, mdMethodDef md, LPBYTE pBuffer, DWORD dwSize, INT nMaxStack)
{
	return CInjection::StartUpdateILCodes(pMethodTable, pMethodHandle, md, pBuffer, dwSize, nMaxStack);
}

extern "C"
	VOID __declspec (dllexport) __stdcall 
	GetErrorMessage(LPSTR lpszOut, DWORD dwOutSize)
{
	strcpy_s( (char*)lpszOut, (size_t)dwOutSize, (char*)CInjection::s_szLastError);
}

extern "C"
	BOOL __declspec (dllexport) __stdcall 
	IsInitialized()
{
	return CInjection::IsInitialized();
}

extern "C"
	BOOL __declspec (dllexport) __stdcall 
	IsInjectionSucceeded()
{
	return CInjection::IsSucceeded();
}

// wait until the initialization is completed
extern "C"
	BOOL __declspec (dllexport) __stdcall 
	WaitForIntializationCompletion()
{
	::WaitForSingleObject( CInjection::s_hEvent, INFINITE);
	return CInjection::IsSucceeded();
}


//----------Exported-------------


DWORD __stdcall CInjection::Initialize(LPVOID lpParameter )
{
	USES_CONVERSION;
	try
	{
		HMODULE hInstance = (HMODULE)lpParameter;
		// get the current directory
		CString strCurrentDir = CUtility::GetCurrentDirectory();

		// find the JIT module
		g_hJitModule = GetModuleHandleA("clrjit.dll");
		if( !g_hJitModule )
			g_hJitModule = GetModuleHandleA("mscorjit.dll");
		if( g_hJitModule == NULL )
		{
			throw "Unable to find the JIT module";
		}
		LPSTR aryNames1[] = { "CILJit::compileMethod", "PreJit::compileMethod", NULL };
		std::map<CStringA, ULONGLONG> map = GetAddresses(g_hJitModule, aryNames1);
		ICorJitCompiler::FillAddresses(map);

		// find the CLR module
		g_hClrModule = GetModuleHandleA("clr.dll");
		if( !g_hClrModule )
			g_hClrModule = GetModuleHandleA("mscorwks.dll");
		if( g_hClrModule == NULL || !DetermineDotNetVersion() )
		{
			throw "Unable to find the CLR module";
		}
		LPSTR aryNames2[] = 
		{
			"MethodDesc::Reset",
			"MethodDesc::IsGenericMethodDefinition",
			"MethodDesc::GetNumGenericMethodArgs",
			"MethodDesc::StripMethodInstantiation",
			"MethodDesc::HasClassOrMethodInstantiation",
			"MethodDesc::ContainsGenericVariables",
			"MethodDesc::GetWrappedMethodDesc",
			"MethodDesc::GetDomain",
			"MethodDesc::GetLoaderModule",
			"LoadedMethodDescIterator::LoadedMethodDescIterator",
			"LoadedMethodDescIterator::Next",
			"LoadedMethodDescIterator::Start",
			"LoadedMethodDescIterator::Current",
			NULL 
		};
		map = GetAddresses(g_hClrModule, aryNames2);
		MethodDesc::FillAddresses(map);
		LoadedMethodDescIterator::FillAddresses(map);

		HookApi();
		s_bIsSucceeded = TRUE;
	}
	catch(LPCSTR lpszError)
	{
		memcpy_s( s_szLastError, LAST_ERROR_MSG_SIZE, lpszError, strlen(lpszError));
	}

	SetEvent( s_hEvent );
	return 0;
}



BOOL CInjection::StartUpdateILCodes( MethodTable * pMethodTable
	, CORINFO_METHOD_HANDLE pMethodHandle
	, mdMethodDef md
	, LPBYTE pBuffer
	, DWORD dwSize
	, int nMaxStack
	)
{
	if( !s_bIsSucceeded || !pMethodHandle )
		return FALSE;

	MethodDesc * pMethodDesc = (MethodDesc*)pMethodHandle;
	pMethodDesc->Reset();

	MethodDesc * pStripMethodDesc = pMethodDesc->StripMethodInstantiation();
	if( pStripMethodDesc )
		pStripMethodDesc->Reset();

	ILCodeBuffer tILCodeBuffer;
	tILCodeBuffer.nMaxStack = nMaxStack;
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
	if( pCorMethodInfo && s_bIsSucceeded )
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

			if( tILCodeBuffer.nMaxStack >= 0 && tILCodeBuffer.nMaxStack <= 65535 )
				pCorMethodInfo->maxStack = tILCodeBuffer.nMaxStack;

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





void CInjection::Uninitialize(void)
{
	if( s_bIsSucceeded )
	{
		s_bIsSucceeded = FALSE;
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




VOID CInjection::HookApi(void)
{
	PFN_compileMethod pfnCompileMethod = &CInjection::compileMethod;
	LPVOID * pAddr = (LPVOID*)&pfnCompileMethod;
	NTSTATUS ntStatus = LhInstallHook( (PVOID&)ICorJitCompiler::s_pfnComplieMethod 
		, *pAddr
		, NULL
		, &s_hHookCompileMethod
		);

	ULONG ulThreadID = GetCurrentProcessId();
	LhSetExclusiveACL( &ulThreadID, 1, &s_hHookCompileMethod);

	if( ntStatus != STATUS_SUCCESS )
		throw "Failed to hook the API";
}




