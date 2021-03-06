
// AlarmCenter.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "AppResource.h"
#include "res.h"

// CAlarmCenterApp:
// See AlarmCenter.cpp for the implementation of this class
//

class CAlarmCenterApp : public CWinApp
{
public:
	CAlarmCenterApp();
	int exit_code_ = 0;
// Overrides
public:
	virtual BOOL InitInstance();
	HANDLE m_hMutex;
// Implementation

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
	BOOL IfProcessRunning();
};

extern CAlarmCenterApp theApp;

inline void PostMessageToMainWnd(UINT msg, WPARAM wParam = 0, LPARAM lParam = 0)
{
	if (theApp.m_pMainWnd) {
		theApp.m_pMainWnd->PostMessageW(msg, wParam, lParam);
	}
}

inline void QuitApplication(int exit_code)
{
	theApp.exit_code_ = exit_code;
	PostMessageToMainWnd(WM_EXIT_ALARM_CENTER);
}

