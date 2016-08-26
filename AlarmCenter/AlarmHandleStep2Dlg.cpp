// AlarmHandleStep2Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AlarmHandleStep2Dlg.h"
#include "afxdialogex.h"
#include "alarm_handle_mgr.h"

using namespace core;


core::alarm_judgement CAlarmHandleStep2Dlg::prev_sel_alarm_judgement_ = core::alarm_judgement_by_video_image;
int CAlarmHandleStep2Dlg::prev_user_defined_ = alarm_judgement_min;

// CAlarmHandleStep2Dlg dialog

IMPLEMENT_DYNAMIC(CAlarmHandleStep2Dlg, CDialogEx)

CAlarmHandleStep2Dlg::CAlarmHandleStep2Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_ALARM_HANDLE_2, pParent)
{

}

CAlarmHandleStep2Dlg::~CAlarmHandleStep2Dlg()
{
}

void CAlarmHandleStep2Dlg::enable_windows()
{
	m_btn_video.EnableWindow(m_radio_video_image.GetCheck());
	m_btn_img.EnableWindow(m_radio_video_image.GetCheck());
	m_cmb_user_define.EnableWindow(m_radio_user_define.GetCheck());
}

void CAlarmHandleStep2Dlg::resolv_alarm_judgement()
{

}

void CAlarmHandleStep2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RADIO1, m_radio_video_image);
	DDX_Control(pDX, IDC_RADIO2, m_radio_comfirmed_with_consumer);
	DDX_Control(pDX, IDC_RADIO3, m_radio_platform);
	DDX_Control(pDX, IDC_RADIO4, m_radio_user_define);
	DDX_Control(pDX, IDC_COMBO1, m_cmb_user_define);
	DDX_Control(pDX, IDC_BUTTON_VIDEO, m_btn_video);
	DDX_Control(pDX, IDC_BUTTON_IMAGE, m_btn_img);
}


BEGIN_MESSAGE_MAP(CAlarmHandleStep2Dlg, CDialogEx)
	ON_BN_CLICKED(IDC_RADIO1, &CAlarmHandleStep2Dlg::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CAlarmHandleStep2Dlg::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO3, &CAlarmHandleStep2Dlg::OnBnClickedRadio3)
	ON_BN_CLICKED(IDC_RADIO4, &CAlarmHandleStep2Dlg::OnBnClickedRadio4)
	ON_BN_CLICKED(IDOK, &CAlarmHandleStep2Dlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CAlarmHandleStep2Dlg message handlers


BOOL CAlarmHandleStep2Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText(TR(IDS_STRING_ALARM_JUDGEMENT));

	SET_WINDOW_TEXT(IDC_RADIO1, IDS_STRING_BY_VIDEO_AND_IMAGE);
	SET_WINDOW_TEXT(IDC_RADIO2, IDS_STRING_BY_CONFIRM_WITH_OWNER);
	SET_WINDOW_TEXT(IDC_RADIO3, IDS_STRING_BY_PLATFORM_PROMPT);
	SET_WINDOW_TEXT(IDC_RADIO4, IDS_STRING_USER_DEFINED);

	SET_WINDOW_TEXT(IDC_BUTTON_VIDEO, IDS_STRING_VIDEO);
	SET_WINDOW_TEXT(IDC_BUTTON_IMAGE, IDS_STRING_IMAGE);

	SET_WINDOW_TEXT(IDC_STATIC_NOTE, IDS_STRING_NOTE);

	SET_WINDOW_TEXT(IDOK, IDS_OK);
	SET_WINDOW_TEXT(IDCANCEL, IDS_CANCEL);

	auto mgr = core::alarm_handle_mgr::get_instance();

	switch (prev_sel_alarm_judgement_) {
	case core::alarm_judgement_by_video_image:
		m_radio_video_image.SetCheck(1);
		break;
	case core::alarm_judgement_by_confirm_with_owner:
		m_radio_comfirmed_with_consumer.SetCheck(1);
		break;
	case core::alarm_judgement_by_platform_tip:
		m_radio_platform.SetCheck(1);
		break;
	case core::alarm_judgement_by_user_define:
	default:
	{
		m_radio_user_define.SetCheck(1);
		alarm_judgement_type_info judge;
		if (prev_user_defined_ == alarm_judgement_min) {
			judge = mgr->get_first_user_defined_judgement();
		} else {
			judge = mgr->get_alarm_judgement_type_info(prev_user_defined_);
		}

		if (judge.first != alarm_judgement_min) {
			prev_user_defined_ = judge.first;
			int ndx = m_cmb_user_define.AddString(judge.second.c_str());
			m_cmb_user_define.SetItemData(ndx, judge.first);
			m_cmb_user_define.SetCurSel(ndx);
		}
	}
		break;
	}

	enable_windows();


	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CAlarmHandleStep2Dlg::OnBnClickedRadio1()
{
	enable_windows();
}


void CAlarmHandleStep2Dlg::OnBnClickedRadio2()
{
	enable_windows();
}


void CAlarmHandleStep2Dlg::OnBnClickedRadio3()
{
	enable_windows();
}


void CAlarmHandleStep2Dlg::OnBnClickedRadio4()
{
	enable_windows();
}


void CAlarmHandleStep2Dlg::OnBnClickedOk()
{
	resolv_alarm_judgement();


	CDialogEx::OnOK();
}
