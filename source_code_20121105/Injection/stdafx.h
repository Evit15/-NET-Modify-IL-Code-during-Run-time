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

// uncomment the following line to static link with EasyHook
//#define STATIC_LINK_EASYHOOK

#ifdef STATIC_LINK_EASYHOOK
#	include <easyhook_static.h>
#else
#	include <easyhook.h>
#endif


#ifdef STATIC_LINK_EASYHOOK

#	if defined(_WIN64)
#		if _MSC_VER == 1600
#			pragma comment( lib, "EasyHook64_Static_VS2010.lib")
#		elif _MSC_VER == 1700
#			pragma comment( lib, "EasyHook64_Static_VS2012.lib")
#		endif
#	elif  defined(_M_IX86)
#		if _MSC_VER == 1600
#			pragma comment( lib, "EasyHook32_Static_VS2010.lib")
#		elif _MSC_VER == 1700
#			pragma comment( lib, "EasyHook32_Static_VS2012.lib")
#		endif
#	else
#		error "Unknown platform"
#	endif
#	pragma comment( lib, "Aux_ulib.lib")
#	pragma comment( lib, "psapi.lib")
#else
#	if defined(_WIN64)
#		pragma comment( lib, "EasyHook64.lib")
#	elif  defined(_M_IX86)
#		pragma comment( lib, "EasyHook32.lib")
#	else
#		error "Unknown platform"
#	endif
#endif


#define DBGHELP_TRANSLATE_TCHAR
#include <Dbghelp.h>



#pragma comment( lib, "Version.lib")

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
