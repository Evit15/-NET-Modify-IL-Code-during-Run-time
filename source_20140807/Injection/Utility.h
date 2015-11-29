#pragma once
class CUtility
{
public:
	static BOOL GetFileVersion(LPWSTR lpszFile, VS_FIXEDFILEINFO * pFixedFileInfo);

	static CString GetFileName(LPCTSTR lpszFilePath);

	static CString GetCurrentDirectory(void);

	static CStringA GetResourceContent(LPCSTR lpszResName);

	static void WriteCache(LPCSTR lpszID, LPCSTR lpszContent);

	static CStringA ReadCache(LPCSTR lpszID);

};

