// AddVideoUserJovisionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AddVideoUserJovisionDlg.h"
#include "afxdialogex.h"
#include "VideoManager.h"
#include "VideoUserInfoJovision.h"

using namespace video;
using namespace video::jovision;

// CAddVideoUserJovisionDlg dialog

IMPLEMENT_DYNAMIC(CAddVideoUserJovisionDlg, CDialogEx)

CAddVideoUserJovisionDlg::CAddVideoUserJovisionDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_ADD_VIDEO_USER_JOVISION, pParent)
{

}

CAddVideoUserJovisionDlg::~CAddVideoUserJovisionDlg()
{
}

void CAddVideoUserJovisionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_NAME, m_name);
	DDX_Control(pDX, IDC_EDIT_USER_PHONE, m_phone);
	DDX_Control(pDX, IDC_EDIT_DEFAULT_USER_NAME, m_default_user_name);
	DDX_Control(pDX, IDC_EDIT_DEFAULT_USER_PASSWD, m_default_user_passwd);
}


BEGIN_MESSAGE_MAP(CAddVideoUserJovisionDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAddVideoUserJovisionDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CAddVideoUserJovisionDlg message handlers


void CAddVideoUserJovisionDlg::OnBnClickedOk()
{
	m_name.GetWindowTextW(m_strName);
	m_phone.GetWindowTextW(m_strPhone);
	m_default_user_name.GetWindowTextW(m_strDefaultName);
	m_default_user_passwd.GetWindowTextW(m_strDefaultPasswd);

	if (m_strName.IsEmpty()) {
		m_name.ShowBalloonTip(L"    ", GetStringFromAppResource(IDS_STRING_CANT_BE_EMPTY), TTI_INFO_LARGE);
		return;
	}

	auto vmgr = video_manager::get_instance();
	if (vmgr->CheckIfUserJovisionNameExists((LPCTSTR)m_strName)) {
		m_name.ShowBalloonTip(L"    ", GetStringFromAppResource(IDS_STRING_USER_ALREADY_EXISTS), TTI_INFO_LARGE);
		return;
	}

	CDialogEx::OnOK();
}
