#pragma once
#include "PdbHelper.h"
class MethodDesc;
class Module;
class AppDomain;
class LoadedMethodDescIterator;
class CSymbolAddressCache;
class CInjection;
class ICorJitInfo{};
class MethodTable {};

enum DotNetVersion
{
	// .Net Framework 2.0/3.5
	DotNetVersion_20,

	// .Net Framework 4.0
	DotNetVersion_40,

	// .Net Framework 4.5
	DotNetVersion_45,
};
extern DotNetVersion g_tDotNetVersion;
extern HMODULE g_hJitModule;
extern HMODULE g_hClrModule;

BOOL DetermineDotNetVersion(void);
////////////////////////////////////////////////////////////////////
// ICorJitCompiler interface from JIT dll
class ICorJitCompiler 
{
	friend class CInjection;
	friend class CSymbolAddressCache;
public:
	typedef CorJitResult (__stdcall ICorJitCompiler::*PFN_compileMethod)(ICorJitInfo * pJitInfo, CORINFO_METHOD_INFO * pMethodInfo, UINT nFlags, LPBYTE * pEntryAddress, ULONG * pSizeOfCode);

	CorJitResult compileMethod(ICorJitInfo * pJitInfo, CORINFO_METHOD_INFO * pMethodInfo, UINT nFlags, LPBYTE * pEntryAddress, ULONG * pSizeOfCode)
	{
		return (this->*s_pfnComplieMethod)( pJitInfo, pMethodInfo, nFlags, pEntryAddress, pSizeOfCode);
	}
private:
	static PFN_compileMethod s_pfnComplieMethod;
};


////////////////////////////////////////////////////////////////////
// MethodDesc

class MethodDesc
{
	friend class CSymbolAddressCache;

	typedef void (MethodDesc::*PFN_Reset)(void);
	typedef BOOL (MethodDesc::*PFN_IsGenericMethodDefinition)(void);
	typedef ULONG (MethodDesc::*PFN_GetNumGenericMethodArgs)(void);
	typedef MethodDesc * (MethodDesc::*PFN_StripMethodInstantiation)(void);
	typedef BOOL (MethodDesc::*PFN_HasClassOrMethodInstantiation)(void);
	typedef BOOL (MethodDesc::*PFN_ContainsGenericVariables)(void);	
	typedef MethodDesc * (MethodDesc::*PFN_GetWrappedMethodDesc)(void);
	typedef AppDomain * (MethodDesc::*PFN_GetDomain)(void);
	typedef Module * (MethodDesc::*PFN_GetLoaderModule)(void);

public:
	void Reset(void) { (this->*s_pfnReset)(); }
	BOOL IsGenericMethodDefinition(void) { return (this->*s_pfnIsGenericMethodDefinition)(); }
	ULONG GetNumGenericMethodArgs(void) { return (this->*s_pfnGetNumGenericMethodArgs)(); }
	MethodDesc * StripMethodInstantiation(void) { return (this->*s_pfnStripMethodInstantiation)(); }
	BOOL HasClassOrMethodInstantiation(void)  { return (this->*s_pfnHasClassOrMethodInstantiation)(); }
	BOOL ContainsGenericVariables(void) { return (this->*s_pfnContainsGenericVariables)(); }
	MethodDesc * GetWrappedMethodDesc(void) { return (this->*s_pfnGetWrappedMethodDesc)(); }
	AppDomain * GetDomain(void) { return (this->*s_pfnGetDomain)(); }
	Module * GetLoaderModule(void) { return (this->*s_pfnGetLoaderModule)(); }
	
private:
	static PFN_Reset s_pfnReset;
	static PFN_IsGenericMethodDefinition s_pfnIsGenericMethodDefinition;
	static PFN_GetNumGenericMethodArgs s_pfnGetNumGenericMethodArgs;
	static PFN_StripMethodInstantiation s_pfnStripMethodInstantiation;
	static PFN_HasClassOrMethodInstantiation s_pfnHasClassOrMethodInstantiation;
	static PFN_ContainsGenericVariables s_pfnContainsGenericVariables;
	static PFN_GetWrappedMethodDesc s_pfnGetWrappedMethodDesc;
	static PFN_GetDomain s_pfnGetDomain;
	static PFN_GetLoaderModule s_pfnGetLoaderModule;

public:
	static void MatchAddress(LPCWSTR wszName, ULONG64 ulAddr)
	{
		LPVOID* pDest = NULL;
		if( wcscmp( L"MethodDesc::Reset", wszName) == 0 )
			pDest = (LPVOID*)&(MethodDesc::s_pfnReset);
		else if( wcscmp( L"MethodDesc::IsGenericMethodDefinition", wszName) == 0 )
			pDest = (LPVOID*)&(MethodDesc::s_pfnIsGenericMethodDefinition);
		else if( wcscmp( L"MethodDesc::GetNumGenericMethodArgs", wszName) == 0 )
			pDest = (LPVOID*)&(MethodDesc::s_pfnGetNumGenericMethodArgs);
		else if( wcscmp( L"MethodDesc::StripMethodInstantiation", wszName) == 0 )
			pDest = (LPVOID*)&(MethodDesc::s_pfnStripMethodInstantiation);
		else if( wcscmp( L"MethodDesc::HasClassOrMethodInstantiation", wszName) == 0 )
			pDest = (LPVOID*)&(MethodDesc::s_pfnHasClassOrMethodInstantiation);
		else if( wcscmp( L"MethodDesc::ContainsGenericVariables", wszName) == 0 )
			pDest = (LPVOID*)&(MethodDesc::s_pfnContainsGenericVariables);
		else if( wcscmp( L"MethodDesc::GetWrappedMethodDesc", wszName) == 0 )
			pDest = (LPVOID*)&(MethodDesc::s_pfnGetWrappedMethodDesc);
		else if( wcscmp( L"MethodDesc::GetDomain", wszName) == 0 )
			pDest = (LPVOID*)&(MethodDesc::s_pfnGetDomain);
		else if( wcscmp( L"MethodDesc::GetLoaderModule", wszName) == 0 )
			pDest = (LPVOID*)&(MethodDesc::s_pfnGetLoaderModule);

		if( pDest )
			*pDest = (LPVOID)ulAddr;
	}

	static BOOL IsInitialized(void)
	{
		return s_pfnReset &&
			s_pfnIsGenericMethodDefinition &&
			s_pfnGetNumGenericMethodArgs &&
			s_pfnStripMethodInstantiation &&
			s_pfnHasClassOrMethodInstantiation &&
			s_pfnHasClassOrMethodInstantiation &&
			s_pfnContainsGenericVariables &&
			s_pfnGetWrappedMethodDesc &&
			s_pfnGetDomain &&
			s_pfnGetLoaderModule;
	}
};

////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////
/// LoadedMethodDescIterator
enum AssemblyIterationMode { AssemblyIterationMode_Default = 0 };

class LoadedMethodDescIterator
{
	friend class CSymbolAddressCache;

	typedef void (LoadedMethodDescIterator::*PFN_LoadedMethodDescIteratorConstructor)(AppDomain * pAppDomain, Module *pModule,	mdMethodDef md);
	typedef void (LoadedMethodDescIterator::*PFN_LoadedMethodDescIteratorConstructor_v45)(AppDomain * pAppDomain, Module *pModule,	mdMethodDef md, AssemblyIterationMode mode);
	typedef void (LoadedMethodDescIterator::*PFN_Start)(AppDomain * pAppDomain, Module *pModule, mdMethodDef md);
	typedef BOOL (LoadedMethodDescIterator::*PFN_Next_v4)(LPVOID pParam);
	typedef BOOL (LoadedMethodDescIterator::*PFN_Next_v2)(void);
	typedef MethodDesc* (LoadedMethodDescIterator::*PFN_Current)(void);
public:
	LoadedMethodDescIterator(AppDomain * pAppDomain, Module *pModule, mdMethodDef md)
	{
		memset( dummy, 0, sizeof(dummy));
		memset( dummy2, 0, sizeof(dummy2));
		if( s_pfnConstructor )
			(this->*s_pfnConstructor)( pAppDomain, pModule, md);
		if( s_pfnConstructor_v45 )
			(this->*s_pfnConstructor_v45)( pAppDomain, pModule, md, AssemblyIterationMode_Default);
	}

	void Start(AppDomain * pAppDomain, Module *pModule, mdMethodDef md) 
	{ 
		(this->*s_pfnStart)( pAppDomain, pModule, md); 
	}
	BOOL Next() 
	{
		if( s_pfnNext_v4 )
			return (this->*s_pfnNext_v4)(dummy2); 

		if( s_pfnNext_v2 )
			return (this->*s_pfnNext_v2)(); 

		return FALSE;
	}
	MethodDesc* Current() { return (this->*s_pfnCurrent)(); }
private:
	// we don't know the exact size of LoadedMethodDescIterator, so add enough memory here
	BYTE dummy[10240]; 

	// class CollectibleAssemblyHolder<class DomainAssembly *> parameter for Next() in .Net4.0 and above	
	BYTE dummy2[10240]; 

	// constructor for .Net2.0 & .Net 4.0
	static PFN_LoadedMethodDescIteratorConstructor s_pfnConstructor;

	// constructor for .Net4.5
	static PFN_LoadedMethodDescIteratorConstructor_v45 s_pfnConstructor_v45;

	static PFN_Start s_pfnStart;
	static PFN_Next_v4 s_pfnNext_v4;
	static PFN_Next_v2 s_pfnNext_v2; 
	static PFN_Current s_pfnCurrent;

public:
	static void MatchAddress(PSYMBOL_INFOW pSymbolInfo)
	{
		LPVOID* pDest = NULL;
		if( wcscmp( L"LoadedMethodDescIterator::LoadedMethodDescIterator", pSymbolInfo->Name) == 0 )
		{
			switch(g_tDotNetVersion)
			{
			case DotNetVersion_20:
			case DotNetVersion_40:
				pDest = (LPVOID*)&(LoadedMethodDescIterator::s_pfnConstructor);
				break;

			
			case DotNetVersion_45:
				pDest = (LPVOID*)&(LoadedMethodDescIterator::s_pfnConstructor_v45);
				break;

			default:
				ATLASSERT(FALSE);
				return;
			}
		}
		else if( wcscmp( L"LoadedMethodDescIterator::Next", pSymbolInfo->Name) == 0 )
		{
			switch(g_tDotNetVersion)
			{
			case DotNetVersion_20:
				pDest = (LPVOID*)&(LoadedMethodDescIterator::s_pfnNext_v2);
				break;

			case DotNetVersion_40:
			case DotNetVersion_45:
				pDest = (LPVOID*)&(LoadedMethodDescIterator::s_pfnNext_v4);
				break;

			default:
				ATLASSERT(FALSE);
				return;
			}
		}
		else if( wcscmp( L"LoadedMethodDescIterator::Start", pSymbolInfo->Name) == 0 )
			pDest = (LPVOID*)&(LoadedMethodDescIterator::s_pfnStart);
		else if( wcscmp( L"LoadedMethodDescIterator::Current", pSymbolInfo->Name) == 0 )
			pDest = (LPVOID*)&(LoadedMethodDescIterator::s_pfnCurrent);
		
		if( pDest )
			*pDest = (LPVOID)pSymbolInfo->Address;
	}

	static BOOL IsInitialized(void)
	{
		return (s_pfnConstructor || s_pfnConstructor_v45) &&
			(s_pfnNext_v4 || s_pfnNext_v2) &&
			s_pfnCurrent;
	}
};

