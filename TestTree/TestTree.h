
// TestTree.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CTestTreeApp:
// See TestTree.cpp for the implementation of this class
//

class CTestTreeApp : public CWinApp
{
public:
	CTestTreeApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CTestTreeApp theApp;