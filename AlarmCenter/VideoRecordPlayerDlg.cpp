// VideoRecordPlayerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "VideoRecordPlayerDlg.h"
#include "afxdialogex.h"
#include "VideoDeviceInfoJovision.h"
#include "VideoUserInfoJovision.h"
#include "VideoManager.h"
#include "JovisonSdkMgr.h"

using namespace video;
using namespace video::jovision;

// CVideoRecordPlayerDlg dialog

IMPLEMENT_DYNAMIC(CVideoRecordPlayerDlg, CDialogEx)

CVideoRecordPlayerDlg::CVideoRecordPlayerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_VIDEO_RECORD_PLAYER, pParent)
{

}

CVideoRecordPlayerDlg::~CVideoRecordPlayerDlg()
{
}

void CVideoRecordPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PLAYER, m_player);
	DDX_Control(pDX, IDC_STATIC_REC_LIST, m_group_rec_list);
	DDX_Control(pDX, IDC_LIST1, m_list_rec);
	DDX_Control(pDX, IDC_DATETIMEPICKER1, m_ctrl_data);
	DDX_Control(pDX, IDC_BUTTON_GET_REC_LIST, m_btn_get_rec_list);
	DDX_Control(pDX, IDC_STATIC_LOGS, m_group_logs);
	DDX_Control(pDX, IDC_LIST3, m_list_log);
}


BEGIN_MESSAGE_MAP(CVideoRecordPlayerDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CVideoRecordPlayerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CVideoRecordPlayerDlg::OnBnClickedCancel)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_GET_REC_LIST, &CVideoRecordPlayerDlg::OnBnClickedButtonGetRecList)
END_MESSAGE_MAP()


// CVideoRecordPlayerDlg message handlers


BOOL CVideoRecordPlayerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	CenterWindow();

	assert(device_); 
	SetWindowText(GetStringFromAppResource(IDS_DIALOG_VIDEO_RECORD_PLAYER) + device_->get_formatted_name().c_str());

	m_group_rec_list.SetWindowTextW(GetStringFromAppResource(IDS_REC_LIST));
	m_btn_get_rec_list.SetWindowTextW(GetStringFromAppResource(IDS_GET_REC_LIST));
	m_group_logs.SetWindowTextW(GetStringFromAppResource(IDS_OP_LOG));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CVideoRecordPlayerDlg::OnBnClickedOk()
{
	return;
}


void CVideoRecordPlayerDlg::OnBnClickedCancel()
{
	return;
}


void CVideoRecordPlayerDlg::OnClose()
{
	ShowWindow(SW_HIDE);
}


void CVideoRecordPlayerDlg::OnBnClickedButtonGetRecList()
{

}
