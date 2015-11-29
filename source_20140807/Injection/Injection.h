#pragma once
#include <map>
#include "DataStructure.h"





// structure to store the IL code for replacement
typedef struct _ILCodeBuffer
{
	LPBYTE						pBuffer;
	DWORD						dwSize;
	BOOL						bIsGeneric;
	INT							nMaxStack;
} ILCodeBuffer, *LPILCodeBuffer;

#define LAST_ERROR_MSG_SIZE 4096

class CInjection
{
	typedef CorJitResult (__stdcall CInjection::*PFN_compileMethod)(ICorJitInfo * pJitInfo, CORINFO_METHOD_INFO * pMethodInfo, UINT nFlags, LPBYTE * pEntryAddress, ULONG * pSizeOfCode);
public:
	static DWORD __stdcall Initialize(LPVOID lpParameter);
	static void Uninitialize(void);
	static BOOL IsInitialized(void) { return WAIT_OBJECT_0 == ::WaitForSingleObject( CInjection::s_hEvent, 0); }
	static BOOL IsSucceeded(void) { return CInjection::s_bIsSucceeded; }
	static HANDLE s_hEvent;
	static BOOL StartUpdateILCodes( MethodTable * pMethodTable
		, CORINFO_METHOD_HANDLE pMethodHandle
		, mdMethodDef md
		, LPBYTE pBuffer
		, DWORD dwSize
		, int nMaxStack
		);

	static CHAR s_szLastError[LAST_ERROR_MSG_SIZE];
private:

	
	
	static BOOL s_bIsSucceeded;
	static ICorJitCompiler * s_pCorJitCompiler;
	
	static std::map< CORINFO_METHOD_HANDLE, ILCodeBuffer> s_mpILBuffers;
	static HOOK_TRACE_INFO s_hHookCompileMethod;


	static VOID HookApi(void);
	


	CorJitResult __stdcall compileMethod(ICorJitInfo * pJitInfo
	, CORINFO_METHOD_INFO * pMethodInfo
	, UINT nFlags
	, LPBYTE * pEntryAddress
	, ULONG * pSizeOfCode
	);


	
};

