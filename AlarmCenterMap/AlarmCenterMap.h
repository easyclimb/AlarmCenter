
// AlarmCenterMap.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������
#include "../AlarmCenter/res.h"
#include "../Installer/lang/string_table.h"

#define SET_WINDOW_TEXT(IDC, IDS) {CWnd*p = GetDlgItem(IDC); if(p){p->SetWindowText(TR(IDS));}}
#define SET_CLASSIC_WINDOW_TEXT(IDC) SET_WINDOW_TEXT(IDC, "IDS_STRING_"#IDC)


// CAlarmCenterMapApp: 
// �йش����ʵ�֣������ AlarmCenterMap.cpp
//

class CAlarmCenterMapApp : public CWinApp
{
public:
	CAlarmCenterMapApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CAlarmCenterMapApp theApp;