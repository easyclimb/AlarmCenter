
#include "stdafx.h"

#include "mdump.h"

LPCTSTR MiniDumper::m_szAppName;

MiniDumper::MiniDumper()
{
	m_szAppName = _T("Application-AlarmCenter");
	::SetUnhandledExceptionFilter(TopLevelFilter);
}

LONG MiniDumper::TopLevelFilter(struct _EXCEPTION_POINTERS *pExceptionInfo)
{
	//MessageBox(nullptr, "TopLevelFilter", "tip", 0);
	LONG retval = EXCEPTION_CONTINUE_SEARCH;

	// firstly see if dbghelp.dll is around and has the function we need
	// look next to the EXE first, as the one in System32 might be old 
	// (e.g. Windows 2000)
	HMODULE hDll = nullptr;
	TCHAR szDbgHelpPath[_MAX_PATH];

	if (GetModuleFileName(nullptr, szDbgHelpPath, _MAX_PATH)) {
		TCHAR *pSlash = _tcsrchr(szDbgHelpPath, _T('\\'));
		if (pSlash) {
			_tcscpy_s(pSlash + 1, _MAX_PATH, _T("DBGHELP.DLL"));
			hDll = ::LoadLibrary(szDbgHelpPath);
		}
	}

	if (hDll == nullptr) {
		// load any version we can
		hDll = ::LoadLibrary(_T("DBGHELP.DLL"));
	}

	LPCTSTR szResult = nullptr;

	if (hDll) {
		MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress(hDll,
																	  "MiniDumpWriteDump");
		if (pDump) {
			TCHAR szDumpPath[_MAX_PATH];
			TCHAR szScratch[_MAX_PATH];

			// work out a good place for the dump file
			//if (!GetTempPath( _MAX_PATH, szDumpPath ))
			_tcscpy_s(szDumpPath, _T("c:\\"));

			_tcscat_s(szDumpPath, m_szAppName);
			_tcscat_s(szDumpPath, _T(".dmp"));

			// ask the user if they want to save a dump file
			//if (::MessageBox( nullptr, _T("Something bad happened in your program, would you like to save a diagnostic file?"), m_szAppName, MB_YESNO )==IDYES)
			{
				// create the file
				HANDLE hFile = ::CreateFile(szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS,
											FILE_ATTRIBUTE_NORMAL, nullptr);

				if (hFile != INVALID_HANDLE_VALUE) {
					_MINIDUMP_EXCEPTION_INFORMATION ExInfo;

					ExInfo.ThreadId = ::GetCurrentThreadId();
					ExInfo.ExceptionPointers = pExceptionInfo;
					ExInfo.ClientPointers = 0;

					// write the dump
					BOOL bOK = pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, nullptr, nullptr);
					if (bOK) {
						_stprintf_s(szScratch, _T("Saved dump file to '%s'"), szDumpPath);
						szResult = szScratch;
						retval = EXCEPTION_EXECUTE_HANDLER;
					} else {
						_stprintf_s(szScratch, _T("Failed to save dump file to '%s' (error %d)"), szDumpPath, GetLastError());
						szResult = szScratch;
					}
					::CloseHandle(hFile);
				} else {
					_stprintf_s(szScratch, _T("Failed to create dump file '%s' (error %d)"), szDumpPath, GetLastError());
					szResult = szScratch;
				}
			}
		} else {
			szResult = _T("DBGHELP.DLL too old");
		}
	} else {
		szResult = _T("DBGHELP.DLL not found");
	}

	if (szResult)
		::MessageBox(nullptr, szResult, m_szAppName, MB_OK);

	return retval;
}
