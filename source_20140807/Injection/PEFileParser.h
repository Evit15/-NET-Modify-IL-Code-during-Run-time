#pragma once

CStringA GetVirtualAddresses(LPSTR szDllFile, LPSTR lpszMethods []);

std::map<CStringA, ULONGLONG> GetAddresses(HMODULE hModule, LPSTR lpszMethods []);

VOID GetPdbFileInfo(LPCSTR lpszFilePath
	, LPSTR szPdbFileName
	, DWORD dwPdbFileNameCch
	, LPSTR szPdbGuid
	, DWORD dwPdbGuidCch
	, LPSTR szChecksum
	, DWORD dwChecksumCch
	);