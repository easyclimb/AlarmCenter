
// TestGsmDtu.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CTestGsmDtuApp: 
// �йش����ʵ�֣������ TestGsmDtu.cpp
//

class CTestGsmDtuApp : public CWinApp
{
public:
	CTestGsmDtuApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CTestGsmDtuApp theApp;