
// AlarmCenterVideo.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "../AlarmCenter/res.h"
#include "../Installer/lang/string_table.h"



// CAlarmCenterVideoApp:
// See AlarmCenterVideo.cpp for the implementation of this class
//

class CAlarmCenterVideoApp : public CWinApp
{
public:
	CAlarmCenterVideoApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};

extern CAlarmCenterVideoApp theApp;