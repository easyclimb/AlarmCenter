// RemindQueryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "RemindQueryDlg.h"
#include "afxdialogex.h"


// CRemindQueryDlg dialog

IMPLEMENT_DYNAMIC(CRemindQueryDlg, CDialogEx)

CRemindQueryDlg::CRemindQueryDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CRemindQueryDlg::IDD, pParent)
	, m_strOk(L"")
	, m_counter(0)
{

}

CRemindQueryDlg::~CRemindQueryDlg()
{
}

void CRemindQueryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOk);
}


BEGIN_MESSAGE_MAP(CRemindQueryDlg, CDialogEx)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDOK, &CRemindQueryDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CRemindQueryDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CRemindQueryDlg message handlers

BOOL CRemindQueryDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_btnOk.GetWindowTextW(m_strOk);
	m_counter = 10;
	SetTimer(1, 1000, nullptr);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CRemindQueryDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (m_counter-- > 0) {
		CString s; s.Format(L"%s(%d)", m_strOk, m_counter);
		m_btnOk.SetWindowTextW(s);
	} else {
		KillTimer(1);
		OnBnClickedOk();
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CRemindQueryDlg::OnBnClickedOk()
{
	KillTimer(1);
	CDialogEx::OnOK();
}


void CRemindQueryDlg::OnBnClickedCancel()
{
	CDialogEx::OnCancel();
}
