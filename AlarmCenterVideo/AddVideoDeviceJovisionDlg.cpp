// AddVideoDeviceJovisionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenterVideo.h"
#include "AddVideoDeviceJovisionDlg.h"
#include "afxdialogex.h"
#include "../AlarmCenter/VideoManager.h"
#include "../AlarmCenter/VideoUserInfoJovision.h"
#include "../AlarmCenter/VideoDeviceInfoJovision.h"
#include "JovisonSdkMgr.h"

namespace detail {
int g_prev_check_by_sse = 1;
int g_prev_check_use_default_name = 0;
int g_prev_check_use_default_passwd = 0;
int g_link_id = -1;
}

using namespace ::detail;
using namespace video;
using namespace jovision;


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
	DDX_Control(pDX, IDC_EDIT_NOTE, m_dev_note);
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
	device_ = std::make_shared<jovision::video_device_info_jovision>();

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

	device_->set_by_sse(g_prev_check_by_sse > 0);
}


void CAddVideoDeviceJovisionDlg::OnBnClickedCheckUseDefaultUserName()
{
	g_prev_check_use_default_name = m_chk_use_default_user_name.GetCheck();
	if (g_prev_check_use_default_name && user_) {
		m_user_name.SetWindowTextW(user_->get_global_user_name().c_str());
		device_->set_user_name(user_->get_global_user_name());
	} else {
		m_user_name.SetWindowTextW(L"");
		device_->set_user_name(L"");
	}
}


void CAddVideoDeviceJovisionDlg::OnBnClickedCheckUseDefaultUserPasswd()
{
	g_prev_check_use_default_passwd = m_chk_use_default_user_passwd.GetCheck();
	if (g_prev_check_use_default_passwd && user_) {
		m_user_passwd.SetWindowTextW(utf8::a2w(user_->get_global_user_passwd()).c_str());
		device_->set_user_passwd(user_->get_global_user_passwd());
	} else {
		m_user_passwd.SetWindowTextW(L"");
		device_->set_user_passwd("");
	}
}


void CAddVideoDeviceJovisionDlg::OnEnChangeEditName()
{
	CString txt;
	m_user_name.GetWindowTextW(txt);

	if (g_prev_check_use_default_name && user_) {
		if (txt.Compare(user_->get_global_user_name().c_str()) != 0) {
			g_prev_check_use_default_name = 0;
			m_chk_use_default_user_name.SetCheck(0);
		}
	}

	device_->set_user_name((LPCTSTR)txt);
}


void CAddVideoDeviceJovisionDlg::OnEnChangeEditPasswd()
{
	CString txt;
	m_user_passwd.GetWindowTextW(txt);
	
	if (g_prev_check_use_default_passwd && user_) {
		if (txt.Compare(utf8::a2w(user_->get_global_user_passwd()).c_str()) != 0) {
			g_prev_check_use_default_passwd = 0;
			m_chk_use_default_user_passwd.SetCheck(0);
		}
	}

	device_->set_user_passwd(utf8::w2a((LPCTSTR)txt));
}


bool CAddVideoDeviceJovisionDlg::CAddVideoDeviceJovisionDlg::TestInput()
{
	CString txt, title = TR(IDS_STRING_ERROR);
	if (g_prev_check_by_sse) {
		m_cloud_sse_id.GetWindowTextW(txt);
		if (txt.IsEmpty()) {
			txt.Format(L"%s %s", TR(IDS_STRING_CLOUD_SSE_ID), TR(IDS_STRING_CANT_BE_EMPTY));
			m_cloud_sse_id.ShowBalloonTip(title, txt, TTI_ERROR);
			return false;
		}		
		device_->set_sse(utf8::w2a((LPCTSTR)txt));
		device_->set_ip("");
		device_->set_port(0);
	} else {
		m_dev_ip.GetWindowTextW(txt);
		if (txt.IsEmpty() || txt == L"0.0.0.0") {
			txt.Format(L"%s %s", TR(IDS_STRING_IDC_STATIC_023), TR(IDS_STRING_CANT_BE_EMPTY));
			MessageBox(txt, title, MB_ICONERROR);
			return false;
		}
		device_->set_ip(utf8::w2a(LPCTSTR(txt)));

		m_dev_port.GetWindowTextW(txt);
		if (txt.IsEmpty()) {
			txt.Format(L"%s %s", TR(IDS_STRING_DEVICE_PORT), TR(IDS_STRING_CANT_BE_EMPTY));
			m_dev_port.ShowBalloonTip(title, txt, TTI_ERROR);
			return false;
		}
		int port = _ttoi(txt);
		if (65535 < port || port < 1024) {
			txt = TR(IDS_STRING_INVALID_PORT);
			m_dev_port.ShowBalloonTip(title, txt, TTI_ERROR);
			return false;
		}
		device_->set_port(port);

		device_->set_sse("");
	}

	m_user_name.GetWindowTextW(txt);
	if (txt.IsEmpty()) {
		txt.Format(L"%s %s", TR(IDS_STRING_IDC_STATIC_003), TR(IDS_STRING_CANT_BE_EMPTY));
		m_user_name.ShowBalloonTip(title, txt, TTI_ERROR);
		return false;
	}
	device_->set_user_name((LPCTSTR)txt);

	m_user_passwd.GetWindowTextW(txt);
	if (txt.IsEmpty()) {
		txt.Format(L"%s %s", TR(IDS_STRING_IDC_STATIC_022), TR(IDS_STRING_CANT_BE_EMPTY));
		m_user_passwd.ShowBalloonTip(title, txt, TTI_ERROR);
		return false;
	}
	device_->set_user_passwd(utf8::w2a((LPCTSTR)txt));

	m_dev_note.GetWindowTextW(txt);
	device_->set_device_note((LPCTSTR)txt);

	return true;
}


void CAddVideoDeviceJovisionDlg::OnBnClickedOk()
{
	if (!TestInput()) {
		return;
	}

	CDialogEx::OnOK();
}
//
//
//void CAddVideoDeviceJovisionDlg::OnBnClickedButtonPreview()
//{
//	if (!TestInput()) {
//		return;
//	}
//
//	auto jov = jovision::sdk_mgr_jovision::get_instance();
//	jovision::JCLink_t link_id = -1; 
//
//	if (device_->get_by_sse()) {
//		link_id = jov->connect(const_cast<char*>(device_->get_ip().c_str()), device_->get_port(), 1,
//							   const_cast<char*>(utf8::w2a(device_->get_user_name()).c_str()),
//							   const_cast<char*>(device_->get_user_passwd().c_str()),
//							   1, nullptr);
//	} else {
//		link_id = jov->connect(const_cast<char*>(device_->get_sse().c_str()), 0, 1,
//							   const_cast<char*>(utf8::w2a(device_->get_user_name()).c_str()),
//							   const_cast<char*>(device_->get_user_passwd().c_str()),
//							   1, nullptr);
//	}
//
//	CString txt, title = TR(IDS_STRING_ERROR);
//
//	do {
//		if (link_id == -1) {
//			txt = TR(IDS_STRING_CONN_FAIL);
//			MessageBox(txt, title, MB_ICONERROR);
//			break;
//		}
//
//		if (!jov->enable_decoder(link_id, TRUE)) {
//			txt = TR(IDS_STRING_ENABLE_DECODE_FAIL);
//			MessageBox(txt, title, MB_ICONERROR);
//			break;
//		}
//
//		CRect rc;
//		m_player.GetClientRect(rc);
//		m_player.ClientToScreen(rc);
//		if (!jov->set_video_preview(link_id, m_player.GetSafeHwnd(), &rc)) {
//			txt = TR(IDS_STRING_ENABLE_PREVIEW_FAIL);
//			MessageBox(txt, title, MB_ICONERROR);
//			break;
//		}
//
//		g_link_id = link_id;
//		return;
//	} while (0);
//
//	if (link_id != -1) {
//		jov->disconnect(link_id);
//		g_link_id = -1;
//	}
//
//}
