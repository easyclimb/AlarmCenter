// Dialog222222.cpp : implementation file
//

#include "stdafx.h"
#include "TestBaiduMap.h"
#include "Dialog222222.h"
#include "afxdialogex.h"


// CDialog222222 dialog

IMPLEMENT_DYNAMIC(CDialog222222, CDialogEx)

CDialog222222::CDialog222222(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDialog222222::IDD, pParent)
{

}

CDialog222222::~CDialog222222()
{
}

void CDialog222222::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_EXPLORER1, m_web);
}


BEGIN_MESSAGE_MAP(CDialog222222, CDialogEx)
END_MESSAGE_MAP()


// CDialog222222 message handlers


BOOL CDialog222222::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_web.SetHwnd(m_hWnd);
	m_web.OpenWebBrowser();
	VARIANT url;
	url.vt = VT_LPWSTR;
	url.bstrVal = L"D:\\dev\\web\\test.html";
	m_web.OpenURL(&url);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
