﻿
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#define _SCL_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

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
#include <afxdlgs.h>
#include <afxdhtml.h>


#include "C:/dev/Global/global.h"
#include "C:/dev/Global/win32.h"
using namespace jlib;

inline std::string get_data_path_a()
{
	return get_exe_path_a() + "\\data";
}

inline std::wstring get_data_path()
{
	return get_exe_path() + L"\\data";
}

inline std::string get_config_path_a()
{
	return get_data_path_a() + "\\config";
}

inline std::wstring get_config_path()
{
	return get_data_path() + L"\\config";
}


#define USES_ADEMCO_EVENT_TO_STRING
#include "ademco_event.h"


#define ENABLE_SEQ_CONFIRM


// user-defined messages

#define WM_BNCLKEDEX			(WM_USER+1)

#define WM_REPAINT				(WM_USER+2)

// ademco event
#define WM_ADEMCOEVENT			(WM_USER+3)

// traverse zone info
#define WM_TRAVERSEZONE			(WM_USER+4)

// dispatch event
#define WM_DISPATCHEVENT		(WM_USER+5)

// on connect to transmit server ok
#define WM_NETWORKSTARTUPOK		(WM_USER+6)

// on user logout and loggon
#define WM_CURUSERCHANGED		(WM_USER+7)

// new history record need to show in dialog
#define WM_NEWRECORD			(WM_USER+8)

// call back
#define WM_INVERSIONCONTROL		(WM_USER+9)

// load from db
#define WM_PROGRESSEX			(WM_USER+10)

// 16 hour missmeeting submachine check
#define WM_NEEDQUERYSUBMACHINE	(WM_USER+11)

// after picked up a point from baidu map
#define WM_CHOSEN_BAIDU_PT		(WM_USER+12)

// notify user to export history record to excel
#define WM_NEED_TO_EXPORT_HR	(WM_USER+13)

// notify video user mgr dlg to refresh device info
#define WM_VIDEO_INFO_CHANGE	(WM_USER+14)

// exit alarm center
#define WM_EXIT_ALARM_CENTER	(WM_USER+15)

// reminder timeup
#define WM_REMINDER_TIME_UP		(WM_USER+16)

// service timeup
#define WM_SERVICE_TIME_UP		(WM_USER+17)

// disarm passwd wrong
#define WM_DISARM_PASSWD_WRONG  (WM_USER+18)

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

