
// TestHikvision.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

#include "Include\\INS_ErrorCode.h"
#include "Include\\OpenNetStreamDefine.h"
#include "Include\\OpenNetStreamError.h"
#include "Include\\OpenNetStreamInterFace.h"
// CTestHikvisionApp:
// See TestHikvision.cpp for the implementation of this class
//

class CTestHikvisionApp : public CWinApp
{
public:
	CTestHikvisionApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};

extern CTestHikvisionApp theApp;
