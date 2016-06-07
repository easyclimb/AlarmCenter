// AddVideoDeviceJovisionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AddVideoDeviceJovisionDlg.h"
#include "afxdialogex.h"
#include "VideoManager.h"
#include "VideoUserInfoJovision.h"
#include "VideoDeviceInfoJovision.h"

namespace detail {
int g_prev_check_by_sse = 1;
int g_prev_check_use_default_name = 0;
int g_prev_check_use_default_passwd = 0;

}

using namespace video;
using namespace jovision;
using namespace detail;

// CAddVideoDeviceJovisionDlg dialog

IMPLEMENT_DYNAMIC(CAddVideoDeviceJovisionDlg, CDialogEx)

CAddVideoDeviceJovisionDlg::CAddVideoDeviceJovisionDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_ADD_VIDEO_DEVICE_JOVISION, pParent)
{

}

CAddVideoDeviceJovisionDlg::~CAddVideoDeviceJovisionDlg()
{
}

void CAddVideoDeviceJovisionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_CLOUD_SSE_ID, m_cloud_sse_id);
	DDX_Control(pDX, IDC_EDIT_NAME, m_user_name);
	DDX_Control(pDX, IDC_EDIT_PASSWD, m_user_passwd);
	DDX_Control(pDX, IDC_IPADDRESS1, m_dev_ip);
	DDX_Control(pDX, IDC_EDIT_PASSWD2, m_dev_port);
	DDX_Control(pDX, IDC_CHECK1, m_chk_by_sse);
	DDX_Control(pDX, IDC_CHECK2, m_chk_use_default_user_name);
	DDX_Control(pDX, IDC_CHECK3, m_chk_use_default_user_passwd);
}


BEGIN_MESSAGE_MAP(CAddVideoDeviceJovisionDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK1, &CAddVideoDeviceJovisionDlg::OnBnClickedCheckUseCloudSse)
	ON_BN_CLICKED(IDC_CHECK2, &CAddVideoDeviceJovisionDlg::OnBnClickedCheckUseDefaultUserName)
	ON_BN_CLICKED(IDC_CHECK3, &CAddVideoDeviceJovisionDlg::OnBnClickedCheckUseDefaultUserPasswd)
	ON_EN_CHANGE(IDC_EDIT_NAME, &CAddVideoDeviceJovisionDlg::OnEnChangeEditName)
	ON_EN_CHANGE(IDC_EDIT_PASSWD, &CAddVideoDeviceJovisionDlg::OnEnChangeEditPasswd)
	ON_BN_CLICKED(IDOK, &CAddVideoDeviceJovisionDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CAddVideoDeviceJovisionDlg message handlers
BOOL CAddVideoDeviceJovisionDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	assert(user_);

	m_chk_by_sse.SetCheck(g_prev_check_by_sse);
	OnBnClickedCheckUseCloudSse();

	m_chk_use_default_user_name.SetCheck(g_prev_check_use_default_name);
	OnBnClickedCheckUseDefaultUserName();

	m_chk_use_default_user_passwd.SetCheck(g_prev_check_use_default_passwd);
	OnBnClickedCheckUseDefaultUserPasswd();


	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CAddVideoDeviceJovisionDlg::OnBnClickedCheckUseCloudSse()
{
	g_prev_check_by_sse = m_chk_by_sse.GetCheck();
	m_cloud_sse_id.EnableWindow(g_prev_check_by_sse);
	m_dev_ip.EnableWindow(!g_prev_check_by_sse);
	m_dev_port.EnableWindow(!g_prev_check_by_sse);
}


void CAddVideoDeviceJovisionDlg::OnBnClickedCheckUseDefaultUserName()
{
	g_prev_check_use_default_name = m_chk_use_default_user_name.GetCheck();
	if (g_prev_check_use_default_name && user_) {
		m_user_name.SetWindowTextW(user_->get_global_user_name().c_str());
	} else {
		m_user_name.SetWindowTextW(L"");
	}
}


void CAddVideoDeviceJovisionDlg::OnBnClickedCheckUseDefaultUserPasswd()
{
	g_prev_check_use_default_passwd = m_chk_use_default_user_passwd.GetCheck();
	if (g_prev_check_use_default_passwd && user_) {
		m_user_passwd.SetWindowTextW(utf8::a2w(user_->get_global_user_passwd()).c_str());
	} else {
		m_user_passwd.SetWindowTextW(L"");
	}
}


void CAddVideoDeviceJovisionDlg::OnEnChangeEditName()
{
	if (g_prev_check_use_default_name && user_) {
		CString txt;
		m_user_name.GetWindowTextW(txt);
		if (txt.Compare(user_->get_global_user_name().c_str()) != 0) {
			g_prev_check_use_default_name = 0;
			m_chk_use_default_user_name.SetCheck(0);
		}
	}
}


void CAddVideoDeviceJovisionDlg::OnEnChangeEditPasswd()
{
	if (g_prev_check_use_default_passwd && user_) {
		CString txt;
		m_user_passwd.GetWindowTextW(txt);
		if (txt.Compare(utf8::a2w(user_->get_global_user_passwd()).c_str()) != 0) {
			g_prev_check_use_default_passwd = 0;
			m_chk_use_default_user_passwd.SetCheck(0);
		}
	}
}


void CAddVideoDeviceJovisionDlg::OnBnClickedOk()
{


	CDialogEx::OnOK();
}
