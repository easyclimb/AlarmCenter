
// BaiduMap.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CBaiduMapApp: 
// �йش����ʵ�֣������ BaiduMap.cpp
//

class CBaiduMapApp : public CWinApp
{
public:
	CBaiduMapApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CBaiduMapApp theApp;