// AddSecurityGuardDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AddSecurityGuardDlg.h"
#include "afxdialogex.h"
#include "alarm_handle_mgr.h"

// CAddSecurityGuardDlg dialog

IMPLEMENT_DYNAMIC(CAddSecurityGuardDlg, CDialogEx)

CAddSecurityGuardDlg::CAddSecurityGuardDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_ADD_GUARD, pParent)
{

}

CAddSecurityGuardDlg::~CAddSecurityGuardDlg()
{
}

void CAddSecurityGuardDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_name);
	DDX_Control(pDX, IDC_EDIT2, m_phone);
	DDX_Control(pDX, IDOK, m_btn_ok);
}


BEGIN_MESSAGE_MAP(CAddSecurityGuardDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAddSecurityGuardDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CAddSecurityGuardDlg message handlers


BOOL CAddSecurityGuardDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowTextW(TR(IDS_STRING_ADD_SECURITY_GUARD));

	SET_WINDOW_TEXT(IDC_STATIC_NAME, IDS_STRING_NAME);
	SET_WINDOW_TEXT(IDC_STATIC_PHONE, IDS_STRING_PHONE);
	SET_WINDOW_TEXT(IDOK, IDS_OK);
	SET_WINDOW_TEXT(IDCANCEL, IDS_CANCEL);




	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CAddSecurityGuardDlg::OnBnClickedOk()
{
	using namespace core;

	CString name, phone;
	m_name.GetWindowTextW(name);
	m_phone.GetWindowTextW(phone);

	if (name.IsEmpty()) {
		m_name.ShowBalloonTip(TR(IDS_STRING_ERROR), TR(IDS_STRING_NAME), TTI_ERROR);
		return;
	}

	if (phone.IsEmpty()) {
		m_phone.ShowBalloonTip(TR(IDS_STRING_ERROR), TR(IDS_STRING_PHONE), TTI_ERROR);
		return;
	}

	auto mgr = alarm_handle_mgr::get_instance();
	auto guard = mgr->execute_add_security_guard((LPCTSTR)name, (LPCTSTR)phone);
	if (guard) {
		guard_id_ = guard->get_id();
		CDialogEx::OnOK();
	}

	
}
