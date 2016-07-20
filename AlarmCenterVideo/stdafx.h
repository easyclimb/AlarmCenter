
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
#define _SCL_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS


#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions


#include <afxdisp.h>        // MFC Automation classes



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC support for ribbons and control bars

#include "C:/dev/Global/global.h"
#include "C:/dev/Global/win32.h"
using namespace jlib;

inline std::string get_data_path()
{
	return get_exe_path_a() + "\\data";
}

inline std::string get_config_path()
{
	return get_data_path() + "\\config";
}


#include "../AlarmCenter/ademco_event.h"
#include "../AlarmCenter/core.h"
#include "../video/video.h"

#define WM_INVERSIONCONTROL		(WM_USER+1)
#define WM_EXIT_ALARM_CENTER	(WM_USER+2)
#define WM_VIDEO_INFO_CHANGE	(WM_USER+3)
#define WM_SHOW_USER_MGR_DLG	(WM_USER+4)


#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


