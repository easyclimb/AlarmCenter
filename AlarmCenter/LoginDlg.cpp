// LoginDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "LoginDlg.h"
#include "afxdialogex.h"
#include "UserInfo.h"

// CLoginDlg dialog

IMPLEMENT_DYNAMIC(CLoginDlg, CDialogEx)

CLoginDlg::CLoginDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CLoginDlg::IDD, pParent)
	, m_prev_user_id(-1)
	, m_prev_user_name(L"")
{

}

CLoginDlg::~CLoginDlg()
{
}

void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//  DDX_Control(pDX, IDC_COMBO_USER_ID, m_user_id);
	//  DDX_Control(pDX, IDC_COMBO_USER_NAME, m_user_name);
	DDX_Control(pDX, IDC_EDIT_USER_PASSWD, m_user_passwd);
	DDX_Control(pDX, IDC_CHECK_LOGBY_USERID, m_chkLogByID);
	DDX_Control(pDX, IDC_EDIT_USERID, m_user_id);
	DDX_Control(pDX, IDC_EDIT_USER_NAME, m_user_name);
	DDX_Control(pDX, IDC_STATIC_NOTE_ID, m_note_id);
	DDX_Control(pDX, IDC_STATIC_NOTE_NAME, m_note_name);
	DDX_Control(pDX, IDC_STATIC_NOTE_PASSWD, m_note_passwd);
}


BEGIN_MESSAGE_MAP(CLoginDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK_LOGBY_USERID, &CLoginDlg::OnBnClickedCheckLogbyUserid)
	ON_BN_CLICKED(IDOK, &CLoginDlg::OnBnClickedOk)
	ON_EN_CHANGE(IDC_EDIT_USERID, &CLoginDlg::OnEnChangeEditUserid)
	ON_EN_CHANGE(IDC_EDIT_USER_NAME, &CLoginDlg::OnEnChangeEditUserName)
END_MESSAGE_MAP()


// CLoginDlg message handlers


void CLoginDlg::OnBnClickedCheckLogbyUserid()
{
	BOOL byID = m_chkLogByID.GetCheck();
	if (byID) {
		m_user_id.EnableWindow();
		m_user_name.EnableWindow(0);
	} else {
		m_user_id.EnableWindow(0);
		m_user_name.EnableWindow();
	}
}


void CLoginDlg::OnBnClickedOk()
{
	core::CUserManager* mgr = core::CUserManager::GetInstance();
	CString passwd;
	m_user_passwd.GetWindowTextW(passwd);
	BOOL ok = FALSE;
	BOOL byID = m_chkLogByID.GetCheck();
	if (byID) {
		CString suser_id;
		m_user_id.GetWindowTextW(suser_id);
		int user_id = _wtoi(suser_id);
		ok = mgr->Login(user_id, passwd);
	} else {
		CString user_name;
		m_user_name.GetWindowTextW(user_name);
		ok = mgr->Login(user_name, passwd);
	}

	if (!ok) {
		CString note;
		note.LoadStringW(IDS_STRING_USER_PASSWD_WRONG);
		MessageBox(note, L"", MB_ICONERROR);
		return;
	}

	CDialogEx::OnOK();
}


void CLoginDlg::OnEnChangeEditUserid()
{
	CString suser_id;
	m_user_id.GetWindowTextW(suser_id);
	int user_id = _wtoi(suser_id);
	if (m_prev_user_id == user_id) {
		return;
	}
	m_prev_user_id = user_id;
	core::CUserManager* mgr = core::CUserManager::GetInstance();
	CString user_name;
	if (!mgr->UserExists(user_id, user_name)) {
		CString note;
		note.LoadStringW(IDS_STRING_USERID_NOT_EXISTS);
		m_note_id.SetWindowTextW(note);
	} else {
		m_user_name.SetWindowTextW(user_name);
		m_note_id.SetWindowTextW(L"");
	}
}


void CLoginDlg::OnEnChangeEditUserName()
{
	CString user_name;
	int user_id;
	m_user_name.GetWindowTextW(user_name);
	if (m_prev_user_name.Compare(user_name) == 0) {
		return;
	}
	m_prev_user_name = user_name;
	core::CUserManager* mgr = core::CUserManager::GetInstance();
	if (!mgr->UserExists(user_name, user_id)) {
		CString note;
		note.LoadStringW(IDS_STRING_USER_NAME_NOT_EXISTS);
		m_note_name.SetWindowTextW(note);
	} else {
		user_name.Format(L"%d", user_id);
		m_user_id.SetWindowTextW(user_name);
		m_note_name.SetWindowTextW(L"");
	}
}


BOOL CLoginDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_user_id.EnableWindow(0);
	m_user_name.EnableWindow();
#ifdef _DEBUG
	m_user_name.SetWindowTextW(L"admin");
	m_user_passwd.SetWindowTextW(L"123456");
#endif

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
