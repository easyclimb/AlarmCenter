// AlarmHandleStep4Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AlarmHandleStep4Dlg.h"
#include "afxdialogex.h"


// CAlarmHandleStep4Dlg dialog

IMPLEMENT_DYNAMIC(CAlarmHandleStep4Dlg, CDialogEx)

CAlarmHandleStep4Dlg::CAlarmHandleStep4Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_ALARM_HANDLE_4, pParent)
{

}

CAlarmHandleStep4Dlg::~CAlarmHandleStep4Dlg()
{
}

void CAlarmHandleStep4Dlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_EDIT_AID, m_aid);
	DDX_Control(pDX, IDC_EDIT_NAME, m_name);
	DDX_Control(pDX, IDC_EDIT_CONTACT, m_contact);
	DDX_Control(pDX, IDC_EDIT_ADDR, m_addr);
	DDX_Control(pDX, IDC_EDIT_PHONE, m_phone);
	DDX_Control(pDX, IDC_EDIT_PHONE_BK, m_phone_bk);
	DDX_Control(pDX, IDC_EDIT_ALARM_INFO, m_alarm_text);
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_STATUS, m_cmb_status);
	DDX_Control(pDX, IDC_EDIT_ASSIGN_TIME, m_assign_time);
	DDX_Control(pDX, IDC_COMBO_GUARD, m_cmb_guard);
	DDX_Control(pDX, IDC_EDIT11, m_predict_minutes);
	DDX_Control(pDX, IDC_EDIT_PREDICT_HANDLE_TIME, m_handle_time);
	DDX_Control(pDX, IDC_COMBO_SHEET_MAKER, m_cmb_user);
	DDX_Control(pDX, IDC_COMBO_ALARM_REASON, m_cmb_alarm_reason);
	DDX_Control(pDX, IDC_EDIT_REASON_DETAIL, m_reason_detail);
	DDX_Control(pDX, IDC_EDIT_REASON_ATTACH, m_reason_attach);
	DDX_Control(pDX, IDC_COMBO_JUDGEMENT, m_cmb_judgement);
	DDX_Control(pDX, IDC_EDIT8, m_judgement_detail);
	DDX_Control(pDX, IDC_EDIT_JUDGEMENT_ATTACH1, m_judgement_attach1);
	DDX_Control(pDX, IDC_EDIT_JUDGEMENT_ATTACH2, m_judgement_attach2);
	DDX_Control(pDX, IDOK, m_btn_ok);
}


BEGIN_MESSAGE_MAP(CAlarmHandleStep4Dlg, CDialogEx)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_MGR_GUARD, &CAlarmHandleStep4Dlg::OnBnClickedButtonMgrGuard)
	ON_BN_CLICKED(IDC_BUTTON_SWITCH_USER, &CAlarmHandleStep4Dlg::OnBnClickedButtonSwitchUser)
	ON_BN_CLICKED(IDC_BUTTON_ADD_REASON_ATTACH, &CAlarmHandleStep4Dlg::OnBnClickedButtonAddReasonAttach)
	ON_BN_CLICKED(IDC_BUTTON_ADD_JUDGMENT_ATTACH1, &CAlarmHandleStep4Dlg::OnBnClickedButtonAddJudgmentAttach1)
	ON_BN_CLICKED(IDC_BUTTON_ADD_JUDGEMENT_ATTACH2, &CAlarmHandleStep4Dlg::OnBnClickedButtonAddJudgementAttach2)
END_MESSAGE_MAP()


// CAlarmHandleStep4Dlg message handlers


BOOL CAlarmHandleStep4Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowTextW(TR(IDS_STRING_ALARM_HANDLE));

	SET_WINDOW_TEXT(IDC_STATIC_AID, IDS_STRING_IDC_STATIC_008);
	SET_WINDOW_TEXT(IDC_STATIC_NAME, IDS_STRING_IDC_STATIC_011);
	SET_WINDOW_TEXT(IDC_STATIC_CONTACT, IDS_STRING_IDC_STATIC_012);
	SET_WINDOW_TEXT(IDC_STATIC_ADDR, IDS_STRING_IDC_STATIC_013);
	SET_WINDOW_TEXT(IDC_STATIC_PHONE, IDS_STRING_IDC_STATIC_014);
	SET_WINDOW_TEXT(IDC_STATIC_PHONE_BK, IDS_STRING_IDC_STATIC_015);
	SET_WINDOW_TEXT(IDC_STATIC_GROUP_ALARM_INFO, IDS_STRING_HRLV_ALARM);
	
	SET_WINDOW_TEXT(IDC_STATIC_STATUS, IDS_STRING_MACHINE_STATUS);
	SET_WINDOW_TEXT(IDC_STATIC_ASSIGN_TIME, IDS_STRING_ASSIGN_TIME);
	SET_WINDOW_TEXT(IDC_STATIC_GUARD, IDS_STRING_SECURITY_GUARD);
	SET_WINDOW_TEXT(IDC_BUTTON_MGR_GUARD, IDS_STRING_SECURITY_MANAGEMENT);
	SET_WINDOW_TEXT(IDC_STATIC_PREDICT_MINUTE, IDS_STRING_PREDICT_PROCESS_TIME);
	SET_WINDOW_TEXT(IDC_STATIC_PREDICT_HANDLE_TIME, IDS_STRING_PREDICT_HANDLE_TIME);
	SET_WINDOW_TEXT(IDC_STATIC_SHEET_MAKER, IDS_STRING_SHEET_MAKER);
	SET_WINDOW_TEXT(IDC_BUTTON_SWITCH_USER, IDS_STRING_IDC_BUTTON_SWITCH_USER);

	SET_WINDOW_TEXT(IDC_STATIC_ALARM_REASON, IDS_STRING_ALARM_REASON);
	SET_WINDOW_TEXT(IDC_STATIC_REASON_DETAIL, IDS_STRING_DETAIL);
	SET_WINDOW_TEXT(IDC_STATIC_REASON_ATTACH, IDS_STRING_ATTACHMENT);
	SET_WINDOW_TEXT(IDC_BUTTON_ADD_REASON_ATTACH, IDS_STRING_ADD_ATTACHMENT);

	SET_WINDOW_TEXT(IDC_STATIC_JUDGEMENT, IDS_STRING_JUDGMENT);
	SET_WINDOW_TEXT(IDC_STATIC_JUDGEMENT_DETAIL, IDS_STRING_DETAIL);
	SET_WINDOW_TEXT(IDC_STATIC_JUDGEMENT_ATTACH1, IDS_STRING_ATTACHMENT);
	SET_WINDOW_TEXT(IDC_BUTTON_ADD_JUDGMENT_ATTACH1, IDS_STRING_ADD_ATTACHMENT);
	SET_WINDOW_TEXT(IDC_BUTTON_ADD_JUDGEMENT_ATTACH2, IDS_STRING_ADD_ATTACHMENT);

	SET_WINDOW_TEXT(IDOK, IDS_OK);
	SET_WINDOW_TEXT(IDCANCEL, IDS_CANCEL);


	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CAlarmHandleStep4Dlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	CDialogEx::OnTimer(nIDEvent);
}


void CAlarmHandleStep4Dlg::OnBnClickedButtonMgrGuard()
{
	// TODO: Add your control notification handler code here
}


void CAlarmHandleStep4Dlg::OnBnClickedButtonSwitchUser()
{
	// TODO: Add your control notification handler code here
}


void CAlarmHandleStep4Dlg::OnBnClickedButtonAddReasonAttach()
{
	// TODO: Add your control notification handler code here
}


void CAlarmHandleStep4Dlg::OnBnClickedButtonAddJudgmentAttach1()
{
	// TODO: Add your control notification handler code here
}


void CAlarmHandleStep4Dlg::OnBnClickedButtonAddJudgementAttach2()
{
	// TODO: Add your control notification handler code here
}
