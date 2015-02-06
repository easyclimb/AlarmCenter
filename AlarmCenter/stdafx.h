
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
#define LOG CLog::WriteLog
#define LOGA CLog::WriteLogA
#define LOGW CLog::WriteLogW
#include "C:/Global/MyWSAError.h"

#define NAMESPACE_END };

#define DECLARE_UNCOPYABLE(classname) \
private:\
	classname(const classname&) {}\
	classname& operator=(const classname&) {}


// singleton
#define DECLARE_SINGLETON(class_name) \
private: \
	class_name(); \
	static class_name* m_pInstance; \
	static CLock m_lock4Instance; \
public: \
	static class_name* GetInstance() { \
		m_lock4Instance.Lock(); \
		if (m_pInstance == NULL){ \
			static class_name obj; \
			m_pInstance = &obj; \
		} \
		m_lock4Instance.UnLock(); \
		return m_pInstance; \
	}

#define IMPLEMENT_SINGLETON(class_name) \
	class_name* class_name::m_pInstance = NULL; \
	CLock class_name::m_lock4Instance;

// getter & setter
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

#define DECLARE_GETTER_STRING(val) \
	const wchar_t* get##val() const { \
		return val; \
	}

#define DECLARE_SETTER_STRING(val) \
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

#define DECLARE_GETTER_SETTER_STRING(val) \
	DECLARE_GETTER_STRING(val); \
	DECLARE_SETTER_STRING(val);


// trik
//#define ARRAY_INITIALIZE_N(type, val, size) \
//	val = new type[size]; \
//	memset(val, 0, sizeof(val) * size);
//
//#define ARRAY_INITIALIZE_1(type, val) \
//	ARRAY_INITIALIZE_N(type, val, 1);
//
//#define STRING_ARRAY_INITIALIZE(val) \
//	ARRAY_INITIALIZE_1(wchar_t, val);

#define MS_OFFLINE 0
#define MS_ONLINE  1
#include "ademco_event.h"

//namespace core{
//typedef enum int
//{
//	MS_OFFLINE,
//	MS_ONLINE,
//	MS_DISARM,
//	MS_ARM,
//	MS_HALFARM,
//	MS_EMERGENCY,
//	MS_BUGLAR,
//	MS_FIRE,
//	MS_DURESS,
//	MS_GAS,
//	MS_WATER,
//	MS_TEMPER,
//	MS_LOWBATTERY,
//	MS_SOLARDISTURB,
//	MS_DISCONNECT,
//	MS_SERIAL485DIS,
//	MS_SERIAL485CONN,
//	MS_DOORRINGING,
//}int;
//NAMESPACE_END

enum ParseResult
{
	RESULT_OK,
	RESULT_NOT_ENOUGH,
	RESULT_DATA_ERROR,
};


// user-defined messages
#define WM_BNCLKEDEX	(WM_USER+1)
#define WM_REPAINT		(WM_USER+2)
#define WM_ADEMCOEVENT	(WM_USER+3)
#define WM_TRAVERSEZONE	(WM_USER+4)










#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

//#include "resource.h"
