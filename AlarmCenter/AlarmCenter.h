
// AlarmCenter.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CAlarmCenterApp:
// See AlarmCenter.cpp for the implementation of this class
//

class CAlarmCenterApp : public CWinApp
{
public:
	CAlarmCenterApp();

// Overrides
public:
	virtual BOOL InitInstance();
	char m_transmit_server_ip[32];
	WORD m_transmit_server_port;
	WORD m_local_port;
// Implementation

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};

extern CAlarmCenterApp theApp;
