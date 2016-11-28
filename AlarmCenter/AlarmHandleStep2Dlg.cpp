// AlarmHandleStep2Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AlarmHandleStep2Dlg.h"
#include "afxdialogex.h"
#include "alarm_handle_mgr.h"
#include "C:/dev/Global/win32/file_op.h"
#include "AlarmHandleStep1Dlg.h"

using namespace core;
using namespace ::detail;


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

bool CAlarmHandleStep2Dlg::resolv_alarm_judgement()
{
	switch (prev_sel_alarm_judgement_) {
	case core::alarm_judgement_by_video_image:
	{
		CString text;
		m_edit_video_path.GetWindowTextW(text);
		video_ = (LPCTSTR)text;

		m_edit_image_path.GetWindowTextW(text);
		image_ = (LPCTSTR)text;
	}
		break;
	case core::alarm_judgement_by_confirm_with_owner:
		break;
	case core::alarm_judgement_by_platform_tip:
		break;
	case core::alarm_judgement_by_user_define:
	{
		CString text;
		m_cmb_user_define.GetWindowTextW(text);
		if (text.IsEmpty()) {
			text.Format(L"%s %s", TR(IDS_STRING_USER_DEFINE), TR(IDS_STRING_CANT_BE_EMPTY));
			MessageBoxW(text, 0, MB_ICONERROR);
			return false;
		}

		std::wstring txt = (LPCTSTR)text;
		auto info = core::alarm_handle_mgr::get_instance()->execute_add_judgement_type(txt);
		prev_user_defined_ = info.first;
	}
		break;
	case core::alarm_judgement_max:
	default:
		return false;
		break;
	}

	CString text;
	m_edit_note.GetWindowTextW(text);
	note_ = (LPCTSTR)text;
	

	return true;
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
	DDX_Control(pDX, IDC_EDIT1, m_edit_video_path);
	DDX_Control(pDX, IDC_EDIT2, m_edit_image_path);
	DDX_Control(pDX, IDC_EDIT3, m_edit_note);
}


BEGIN_MESSAGE_MAP(CAlarmHandleStep2Dlg, CDialogEx)
	ON_BN_CLICKED(IDC_RADIO1, &CAlarmHandleStep2Dlg::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CAlarmHandleStep2Dlg::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO3, &CAlarmHandleStep2Dlg::OnBnClickedRadio3)
	ON_BN_CLICKED(IDC_RADIO4, &CAlarmHandleStep2Dlg::OnBnClickedRadio4)
	ON_BN_CLICKED(IDOK, &CAlarmHandleStep2Dlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_VIDEO, &CAlarmHandleStep2Dlg::OnBnClickedButtonVideo)
	ON_BN_CLICKED(IDC_BUTTON_IMAGE, &CAlarmHandleStep2Dlg::OnBnClickedButtonImage)
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

	auto user_defined_judgement_types = mgr->get_all_user_defined_judgements();
	for (auto judge : user_defined_judgement_types) {
		int ndx = m_cmb_user_define.AddString(judge.second.c_str());
		m_cmb_user_define.SetItemData(ndx, judge.first);
		m_cmb_user_define.SetCurSel(ndx);
	}

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
			if (!user_defined_judgement_types.empty()) {
				judge = user_defined_judgement_types.front();
			}
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
	prev_sel_alarm_judgement_ = alarm_judgement_by_video_image;
}


void CAlarmHandleStep2Dlg::OnBnClickedRadio2()
{
	enable_windows();
	prev_sel_alarm_judgement_ = alarm_judgement_by_confirm_with_owner;
}


void CAlarmHandleStep2Dlg::OnBnClickedRadio3()
{
	enable_windows();
	prev_sel_alarm_judgement_ = alarm_judgement_by_platform_tip;
}


void CAlarmHandleStep2Dlg::OnBnClickedRadio4()
{
	enable_windows();
	prev_sel_alarm_judgement_ = alarm_judgement_by_user_define;
}


void CAlarmHandleStep2Dlg::OnBnClickedOk()
{
	if (!resolv_alarm_judgement()) {
		return;
	}


	CDialogEx::OnOK();
}


void CAlarmHandleStep2Dlg::OnBnClickedButtonVideo()
{
	std::wstring dest = {};
	if (open_file_and_upload_to_data_attach(GetSafeHwnd(), dest)) {
		video_ = dest;
		m_edit_video_path.SetWindowTextW(dest.c_str());
	} else {
		video_.clear();
		m_edit_video_path.SetWindowTextW(L"");
		MessageBox(TR(IDS_STRING_FAILED), TR(IDS_STRING_ERROR), MB_ICONERROR);
	}
}


void CAlarmHandleStep2Dlg::OnBnClickedButtonImage()
{
	std::wstring dest = {};
	if (open_file_and_upload_to_data_attach(GetSafeHwnd(), dest)) {
		video_ = dest;
		m_edit_video_path.SetWindowTextW(dest.c_str());
	} else {
		video_.clear();
		m_edit_image_path.SetWindowTextW(L"");
		MessageBox(TR(IDS_STRING_FAILED), TR(IDS_STRING_ERROR), MB_ICONERROR);
	}
}
