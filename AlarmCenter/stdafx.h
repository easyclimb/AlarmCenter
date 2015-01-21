
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

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

#include "C:/Global/LocalLock.h"
#include "C:/Global/mtverify.h"
#include "C:/Global/FileOper.h"
#include "C:/Global/Log.h"
#include "C:/Global/MyWSAError.h"

#define NAMESPACE_END };

#define DECLARE_UNCOPYABLE(classname) \
private:\
	classname(const classname&) {}\
	classname& operator=(const classname&) {}

#define DECLARE_GETTER(type, val) \
	type get##val() const { \
		return val; \
	}

#define DECLARE_SETTER(type, val) \
	void set##val(type param) { \
		val = param;\
	}

#define DEALARE_GETTER_SETTER(type, val) \
	DECLARE_GETTER(type, val) \
	DECLARE_SETTER(type, val)

#define DEALARE_GETTER_SETTER_INT(val) \
	DECLARE_GETTER(int, val) \
	DECLARE_SETTER(int, val)

#define DECLARE_STRING_GETTER(val) \
	const wchar_t* get##val() const { \
		return val; \
	}

#define DECLARE_STRING_SETTER(val) \
	void set##val(const wchar_t* param) { \
		if (param) { \
			int len = wcslen(param); \
			if (val) { delete[] val; } \
			val = new wchar_t[len + 1]; \
			wcscpy_s(val, len + 1, param); \
		} else { \
			if (val) { delete[] val; } \
			val = new wchar_t[1]; \
			val[0] = 0; \
		} \
	}

namespace core{
typedef enum MachineStatus
{
	MS_OFFLINE,
	MS_ONLINE,
	MS_DISARM,
	MS_ARM,
	MS_HALFARM,
	MS_EMERGENCY,
	MS_BUGLAR,
	MS_FIRE,
	MS_DURESS,
	MS_GAS,
	MS_WATER,
	MS_TEMPER,
	MS_LOWBATTERY,
	MS_SOLARDISTURB,
	MS_DISCONNECT,
	MS_SERIAL485DIS,
	MS_SERIAL485CONN,
	MS_DOORRINGING,
}MachineStatus;
NAMESPACE_END

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#include "resource.h"
