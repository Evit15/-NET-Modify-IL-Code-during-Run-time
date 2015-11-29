#pragma once
#include <map>
#include "DataStructure.h"





// structure to store the IL code for replacement
typedef struct _ILCodeBuffer
{
	LPBYTE						pBuffer;
	DWORD						dwSize;
	BOOL						bIsGeneric;
} ILCodeBuffer, *LPILCodeBuffer;



class CInjection
{
	typedef CorJitResult (__stdcall CInjection::*PFN_compileMethod)(ICorJitInfo * pJitInfo, CORINFO_METHOD_INFO * pMethodInfo, UINT nFlags, LPBYTE * pEntryAddress, ULONG * pSizeOfCode);
public:
	static DWORD __stdcall Initialize(LPVOID lpParameter);
	static void Uninitialize(void);
	static INT GetStatus(void) { return s_nStatus; }
	static HANDLE s_hEvent;
	static BOOL StartUpdateILCodes( MethodTable * pMethodTable
		, CORINFO_METHOD_HANDLE pMethodHandle
		, mdMethodDef md
		, LPBYTE pBuffer
		, DWORD dwSize
		);
private:

	enum
	{
		Status_Ready = 1,
		Status_Uninitialized = 0,
		Status_Error_HookCompileMethodFailed = -1,
		Status_Error_LoadedMethodDescIteratorInitializationFailed = -2,
		Status_Error_MethodDescInitializationFailed = -3,
		Status_Error_DbgHelpNotFound = -4,
		Status_Error_JITNotFound = -5,
		Status_Error_DownloadPDBFailed = -6,
		Status_Error_CLRNotFound = -7,
	};

	static INT s_nStatus;
	static ICorJitCompiler * s_pCorJitCompiler;
	
	static std::map< CORINFO_METHOD_HANDLE, ILCodeBuffer> s_mpILBuffers;
	static HOOK_TRACE_INFO s_hHookCompileMethod;


	static BOOL HookApi(void);
	static BOOL SearchMethodAddresses(LPCWSTR lpwszSearchPath, BOOL bDisableSymSrv = TRUE);
	


	CorJitResult __stdcall compileMethod(ICorJitInfo * pJitInfo
	, CORINFO_METHOD_INFO * pMethodInfo
	, UINT nFlags
	, LPBYTE * pEntryAddress
	, ULONG * pSizeOfCode
	);


	
};

