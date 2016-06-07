// AddVideoDeviceJovisionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AddVideoDeviceJovisionDlg.h"
#include "afxdialogex.h"
#include "VideoManager.h"
#include "VideoUserInfoJovision.h"
#include "VideoDeviceInfoJovision.h"

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
}


BEGIN_MESSAGE_MAP(CAddVideoDeviceJovisionDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK1, &CAddVideoDeviceJovisionDlg::OnBnClickedCheckUseCloudSse)
	ON_BN_CLICKED(IDC_CHECK2, &CAddVideoDeviceJovisionDlg::OnBnClickedCheckUseDefaultUserName)
	ON_BN_CLICKED(IDC_CHECK3, &CAddVideoDeviceJovisionDlg::OnBnClickedCheckUseDefaultUserPasswd)
END_MESSAGE_MAP()


// CAddVideoDeviceJovisionDlg message handlers


void CAddVideoDeviceJovisionDlg::OnBnClickedCheckUseCloudSse()
{
	int b = m_chk_by_sse.GetCheck();
	m_cloud_sse_id.EnableWindow(b);
	m_dev_ip.EnableWindow(!b);
	m_dev_port.EnableWindow(!b);
}


void CAddVideoDeviceJovisionDlg::OnBnClickedCheckUseDefaultUserName()
{
	int b = m_chk_use_default_user_name.GetCheck();
	if (b) {
		auto vmgr = video_manager::get_instance();

	}
}


void CAddVideoDeviceJovisionDlg::OnBnClickedCheckUseDefaultUserPasswd()
{

}


BOOL CAddVideoDeviceJovisionDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_chk_by_sse.SetCheck(0);
	OnBnClickedCheckUseCloudSse();


	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}
