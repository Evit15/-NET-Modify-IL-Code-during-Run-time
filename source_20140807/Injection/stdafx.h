// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>


#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <atlbase.h>
#include <atlstr.h>
#include <atlfile.h>

#include <easyhook.h>




#if defined(_WIN64)
#	pragma comment( lib, "GS_x64.LIB")
#	pragma comment( lib, "AUX_ULIB_x64.LIB")
#	ifdef DEBUG
#		pragma comment( lib, "libcurl_x64_d.lib")
#		pragma comment( lib, "EasyHookLib_x64_d.lib")
#	else
#		pragma comment( lib, "libcurl_x64.lib")
#		pragma comment( lib, "EasyHookLib_x64.lib")
#	endif
#elif  defined(_M_IX86)
#	pragma comment( lib, "GS_x86.LIB")
#	pragma comment( lib, "AUX_ULIB_x86.LIB")
#	ifdef DEBUG
#		pragma comment( lib, "libcurl_d.lib")
#		pragma comment( lib, "EasyHookLib_d.lib")
#	else
#		pragma comment( lib, "libcurl.lib")
#		pragma comment( lib, "EasyHookLib.lib")
#	endif
#else
#	error "Unknown platform"
#endif
#pragma comment( lib, "Aux_ulib.lib")
#pragma comment( lib, "psapi.lib")
#pragma comment( lib, "Version.lib")
//#pragma comment( lib, "Advapi32.lib")
#pragma comment( lib, "Ws2_32.lib")

#define CURL_STATICLIB
#include <curl/curl.h>


#define DBGHELP_TRANSLATE_TCHAR
#include <Dbghelp.h>


extern HMODULE g_hModule;


typedef struct CORINFO_METHOD_STRUCT_*      CORINFO_METHOD_HANDLE;
typedef struct CORINFO_MODULE_STRUCT_*      CORINFO_MODULE_HANDLE;

enum CorInfoOptions
{
	CORINFO_OPT_INIT_LOCALS                 = 0x00000010, // zero initialize all variables

	CORINFO_GENERICS_CTXT_FROM_THIS         = 0x00000020, // is this shared generic code that access the generic context from the this pointer?  If so, then if the method has SEH then the 'this' pointer must always be reported and kept alive.
	CORINFO_GENERICS_CTXT_FROM_PARAMTYPEARG = 0x00000040, // is this shared generic code that access the generic context from the ParamTypeArg?  If so, then if the method has SEH then the 'ParamTypeArg' must always be reported and kept alive. Same as CORINFO_CALLCONV_PARAMTYPE
	CORINFO_GENERICS_CTXT_MASK              = (CORINFO_GENERICS_CTXT_FROM_THIS |
	CORINFO_GENERICS_CTXT_FROM_PARAMTYPEARG),
	CORINFO_GENERICS_CTXT_KEEP_ALIVE        = 0x00000080, // Keep the generics context alive throughout the method even if there is no explicit use, and report its location to the CLR
};

struct CORINFO_METHOD_INFO
{
	CORINFO_METHOD_HANDLE       ftn;
	CORINFO_MODULE_HANDLE       scope;
	BYTE *                      ILCode;
	unsigned                    ILCodeSize;
	unsigned short              maxStack;
	unsigned short              EHcount;
	CorInfoOptions              options;
};

typedef ULONG32 mdToken;                // Generic token
typedef mdToken mdMethodDef;            // Method in this scope

enum CorJitResult
{
	// Note that I dont use FACILITY_NULL for the facility number,
	// we may want to get a 'real' facility number
	CORJIT_OK            =     NO_ERROR,
	CORJIT_BADCODE       =     MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 1),
	CORJIT_OUTOFMEM      =     MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 2),
	CORJIT_INTERNALERROR =     MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 3),
	CORJIT_SKIPPED       =     MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 4),
};

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L) 
#endif
