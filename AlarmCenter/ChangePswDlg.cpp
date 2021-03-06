// ChangePswDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "ChangePswDlg.h"
#include "afxdialogex.h"
#include "UserInfo.h"

// CChangePswDlg dialog

IMPLEMENT_DYNAMIC(CChangePswDlg, CDialogEx)

CChangePswDlg::CChangePswDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_CHANGE_PSW, pParent)
{

}

CChangePswDlg::~CChangePswDlg()
{
}

void CChangePswDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_OLD_PSW, m_old_psw);
	DDX_Control(pDX, IDC_EDIT_NEW_PSW, m_new_psw);
	DDX_Control(pDX, IDC_EDIT_NEW_PSW_2, m_new_psw_2);
}


BEGIN_MESSAGE_MAP(CChangePswDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CChangePswDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CChangePswDlg message handlers


BOOL CChangePswDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowTextW(TR(IDS_STRING_IDC_BUTTON_CHANGE_PASSWD));

	SET_WINDOW_TEXT(IDC_STATIC_INPUT_OLD_PSW, IDS_STRING_INPUT_OLD_PSW);
	SET_WINDOW_TEXT(IDC_STATIC_INPUT_NEW_PSW, IDS_STRING_INPUT_NEW_PSW);
	SET_WINDOW_TEXT(IDC_STATIC_INPUT_NEW_PSW_2, IDS_STRING_INPUT_NEW_PSW_AGAIN);

	SET_WINDOW_TEXT(IDOK, IDS_OK);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CChangePswDlg::OnBnClickedOk()
{
	auto mgr = core::user_manager::get_instance();
	auto user = mgr->get_cur_user_info();

	CString txt;
	m_old_psw.GetWindowTextW(txt);
	if (txt.IsEmpty() || (user->get_passwd() != mgr->encrypt((LPCTSTR)txt))) {
		m_old_psw.ShowBalloonTip(TR(IDS_STRING_ERROR), TR(IDS_STRING_USER_PASSWD_WRONG), TTI_ERROR);
		return;
	}

	CString t2;
	m_new_psw.GetWindowTextW(txt);
	m_new_psw_2.GetWindowTextW(t2);

	if (txt != t2 || t2.IsEmpty()) {
		m_new_psw_2.ShowBalloonTip(TR(IDS_STRING_ERROR), TR(IDS_STRING_PSWDS_NOT_SAME), TTI_ERROR);
		return;
	}

	if (mgr->change_user_passwd(user, txt)) {
		MessageBox(TR(IDS_STRING_SUCCESS), TR(IDS_STRING_IDC_BUTTON_CHANGE_PASSWD));
	} else {
		MessageBox(TR(IDS_STRING_FAILED), TR(IDS_STRING_IDC_BUTTON_CHANGE_PASSWD));
		return;
	}

	CDialogEx::OnOK();
}
