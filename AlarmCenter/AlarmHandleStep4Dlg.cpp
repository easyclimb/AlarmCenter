// AlarmHandleStep4Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AlarmHandleStep4Dlg.h"
#include "SecurityGuardMgrDlg.h"
#include "LoginDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "UserInfo.h"

using namespace core;

namespace detail {
const int c_timer_id_update_date = 1;

}

using namespace detail;

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
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_AID, m_aid);
	DDX_Control(pDX, IDC_EDIT_NAME, m_name);
	DDX_Control(pDX, IDC_EDIT_CONTACT, m_contact);
	DDX_Control(pDX, IDC_EDIT_ADDR, m_addr);
	DDX_Control(pDX, IDC_EDIT_PHONE, m_phone);
	DDX_Control(pDX, IDC_EDIT_PHONE_BK, m_phone_bk);
	DDX_Control(pDX, IDC_EDIT_ALARM_INFO, m_alarm_text);
	DDX_Control(pDX, IDC_COMBO_STATUS, m_cmb_status);
	DDX_Control(pDX, IDC_EDIT_ASSIGN_TIME, m_assign_time);
	DDX_Control(pDX, IDC_COMBO_GUARD, m_cmb_guard);
	DDX_Control(pDX, IDC_EDIT11, m_predict_minutes);
	DDX_Control(pDX, IDC_EDIT_PREDICT_HANDLE_TIME, m_handle_time);
	DDX_Control(pDX, IDC_COMBO_ALARM_REASON, m_cmb_alarm_reason);
	DDX_Control(pDX, IDC_EDIT_REASON_DETAIL, m_reason_detail);
	DDX_Control(pDX, IDC_EDIT_REASON_ATTACH, m_reason_attach);
	DDX_Control(pDX, IDC_COMBO_JUDGEMENT, m_cmb_judgement);
	DDX_Control(pDX, IDC_EDIT8, m_judgement_detail);
	DDX_Control(pDX, IDC_EDIT_JUDGEMENT_ATTACH1, m_judgement_attach1);
	DDX_Control(pDX, IDC_EDIT_JUDGEMENT_ATTACH2, m_judgement_attach2);
	DDX_Control(pDX, IDOK, m_btn_ok);
	DDX_Control(pDX, IDC_EDIT_USER, m_user);
	DDX_Control(pDX, IDC_BUTTON_ADD_JUDGMENT_ATTACH1, m_btn_add_judgment_attach_1);
	DDX_Control(pDX, IDC_BUTTON_ADD_JUDGEMENT_ATTACH2, m_btn_add_judgment_attach_2);
	DDX_Control(pDX, IDC_EDIT_USER_DEFINE_JUDGE, m_judgment_user_define);
	DDX_Control(pDX, IDC_BUTTON_PRINT, m_btn_print);
	DDX_Control(pDX, IDC_BUTTON_SWITCH_USER, m_btn_switch_user);
	DDX_Control(pDX, IDC_BUTTON_ADD_REASON_ATTACH, m_btn_add_reason_attach);
}


BEGIN_MESSAGE_MAP(CAlarmHandleStep4Dlg, CDialogEx)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_MGR_GUARD, &CAlarmHandleStep4Dlg::OnBnClickedButtonMgrGuard)
	ON_BN_CLICKED(IDC_BUTTON_SWITCH_USER, &CAlarmHandleStep4Dlg::OnBnClickedButtonSwitchUser)
	ON_BN_CLICKED(IDC_BUTTON_ADD_REASON_ATTACH, &CAlarmHandleStep4Dlg::OnBnClickedButtonAddReasonAttach)
	ON_BN_CLICKED(IDC_BUTTON_ADD_JUDGMENT_ATTACH1, &CAlarmHandleStep4Dlg::OnBnClickedButtonAddJudgmentAttach1)
	ON_BN_CLICKED(IDC_BUTTON_ADD_JUDGEMENT_ATTACH2, &CAlarmHandleStep4Dlg::OnBnClickedButtonAddJudgementAttach2)
	ON_EN_KILLFOCUS(IDC_EDIT11, &CAlarmHandleStep4Dlg::OnEnKillfocusEditPredictMinutes)
	ON_CBN_SELCHANGE(IDC_COMBO_STATUS, &CAlarmHandleStep4Dlg::OnCbnSelchangeComboStatus)
	ON_BN_CLICKED(IDOK, &CAlarmHandleStep4Dlg::OnBnClickedOk)
	ON_CBN_SELCHANGE(IDC_COMBO_GUARD, &CAlarmHandleStep4Dlg::OnCbnSelchangeComboGuard)
	ON_EN_CHANGE(IDC_EDIT11, &CAlarmHandleStep4Dlg::OnEnChangeEditPredictMinutes)
	ON_CBN_SELCHANGE(IDC_COMBO_JUDGEMENT, &CAlarmHandleStep4Dlg::OnCbnSelchangeComboJudgement)
	ON_BN_CLICKED(IDC_BUTTON_PRINT, &CAlarmHandleStep4Dlg::OnBnClickedButtonPrint)
END_MESSAGE_MAP()


// CAlarmHandleStep4Dlg message handlers

void CAlarmHandleStep4Dlg::update_guard()
{
	KillTimer(c_timer_id_update_date);
	m_cmb_guard.ResetContent();
	int guard_ndx = 0;
	auto mgr = alarm_handle_mgr::get_instance();
	m_cmb_guard.SetItemData(m_cmb_guard.AddString(L"-----------------------"), 0);
	for (auto id : mgr->get_security_guard_ids()) {
		auto guard = mgr->get_security_guard(id);
		int ndx = m_cmb_guard.AddString(guard->get_formatted_name().c_str());
		m_cmb_guard.SetItemData(ndx, id);
		if (handle_ && id == handle_->get_guard_id()) {
			guard_ndx = ndx;
		}
	}
	m_cmb_guard.SetCurSel(guard_ndx);
	OnCbnSelchangeComboGuard();
	if (cur_handling_alarm_info_->get_status() != alarm_status::alarm_status_cleared) {
		//SetTimer(c_timer_id_update_date, 1000, nullptr);
	}
}

int CAlarmHandleStep4Dlg::get_predict_min()
{
	int min = alarm_handle::handle_time_default;
	CString txt;
	m_predict_minutes.GetWindowTextW(txt);

	do {
		if (txt.IsEmpty())
			break;

		int minutes = std::stoi((LPCTSTR)txt);
		if (minutes < alarm_handle::handle_time_min) {
			break;
		}

		if (minutes > alarm_handle::handle_time_max) {
			break;
		}

		min = minutes;

	} while (0);

	return min;
}


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

	SET_WINDOW_TEXT(IDC_BUTTON_PRINT, IDS_STRING_PRINT);
	SET_WINDOW_TEXT(IDOK, IDS_OK);
	SET_WINDOW_TEXT(IDCANCEL, IDS_CANCEL);

	{
		CString txt;
		txt.Format(L"%06d", machine_->get_ademco_id());
		m_aid.SetWindowTextW(txt);
		m_name.SetWindowTextW(machine_->get_machine_name());
		m_contact.SetWindowTextW(machine_->get_contact());
		m_addr.SetWindowTextW(machine_->get_address());
		m_phone.SetWindowTextW(machine_->get_phone());
		m_phone_bk.SetWindowTextW(machine_->get_phone_bk());
		m_alarm_text.SetWindowTextW(cur_handling_alarm_info_->get_text().c_str());
	}

	auto mgr = alarm_handle_mgr::get_instance();

	int status_ndx = -1;
	for (int status = alarm_status::alarm_status_min + 1; status < alarm_status::alarm_status_max; status++) {
		auto txt = alarm_info::get_alarm_status_text(status);
		int ndx = m_cmb_status.AddString(txt.c_str());
		m_cmb_status.SetItemData(ndx, status);
		if (status == cur_handling_alarm_info_->get_status()) {
			status_ndx = ndx;
		}
	}
	m_cmb_status.SetCurSel(status_ndx);

	update_guard();

	if (handle_) {
		m_assign_time.SetWindowTextW(time_point_to_wstring(handle_->get_assigned_time_point()).c_str());

		std::wstringstream ss;
		ss << handle_->get_predict_minutes_to_handle();
		m_predict_minutes.SetWindowTextW(ss.str().c_str());

		m_handle_time.SetWindowTextW(time_point_to_wstring(handle_->get_assigned_time_point() + std::chrono::minutes(handle_->get_predict_minutes_to_handle())).c_str());
	}

	auto user = user_manager::get_instance()->get_cur_user_info();
	m_user.SetWindowTextW(user->get_formmated_name().c_str());
	if (!read_only_ && user->get_id() != cur_handling_alarm_info_->get_user_id()) {
		cur_handling_alarm_info_ = mgr->execute_update_alarm_user(cur_handling_alarm_info_->get_id(), user->get_id());
	}

	int reason_ndx = -1;
	for (int reason = alarm_reason::by::real_alarm; reason <= alarm_reason::by::other_reasons; reason++) {
		auto txt = alarm_reason::get_reason_text(reason);
		int ndx = m_cmb_alarm_reason.AddString(txt.c_str());
		m_cmb_alarm_reason.SetItemData(ndx, reason);
		if (reason_ && reason == reason_->get_reason()) {
			reason_ndx = ndx;
		}
	}
	m_cmb_alarm_reason.SetCurSel(reason_ndx);

	if (reason_) {
		m_reason_detail.SetWindowTextW(reason_->get_detail().c_str());
		m_reason_attach.SetWindowTextW(reason_->get_attach().c_str());
	}

	int judgment_ndx = -1;
	for (int judgment = alarm_judgement::alarm_judgement_min + 1; judgment < alarm_judgement::alarm_judgement_by_user_define; judgment++) {
		auto txt = alarm_judgement_info::get_alarm_judgement_type_text(judgment);
		int ndx = m_cmb_judgement.AddString(txt.c_str());
		m_cmb_judgement.SetItemData(ndx, judgment);
		if (judgment_ && judgment == judgment_->get_judgement_type_id()) {
			judgment_ndx = ndx;
			int able = (judgment == alarm_judgement::alarm_judgement_by_video_image);
			m_btn_add_judgment_attach_1.EnableWindow(able);
			m_btn_add_judgment_attach_2.EnableWindow(able);
		}
	}

	auto user_defined_judgement_types = mgr->get_all_user_defined_judgements();
	for (auto judge : user_defined_judgement_types) {
		int ndx = m_cmb_judgement.AddString(judge.second.c_str());
		m_cmb_judgement.SetItemData(ndx, judge.first);
		if (judgment_ && judge.first == judgment_->get_judgement_type_id()) {
			judgment_ndx = ndx;
		}
	}

	{
		auto txt = alarm_judgement_info::get_alarm_judgement_type_text(alarm_judgement::alarm_judgement_by_user_define);
		int ndx = m_cmb_judgement.AddString(txt.c_str());
		m_cmb_judgement.SetItemData(ndx, 0); // 0 for user define 
	}

	m_cmb_judgement.SetCurSel(judgment_ndx);

	if (judgment_) {
		m_judgement_detail.SetWindowTextW(judgment_->get_note().c_str());
		m_judgement_attach1.SetWindowTextW(judgment_->get_note1().c_str());
		m_judgement_attach2.SetWindowTextW(judgment_->get_note2().c_str());
	}	

	if (read_only_) {
		m_cmb_status.EnableWindow(0);
		m_cmb_guard.EnableWindow(0);
		m_predict_minutes.EnableWindow(0);

		m_btn_switch_user.EnableWindow(0);
		m_cmb_alarm_reason.EnableWindow(0);
		m_reason_detail.EnableWindow(0);
		m_reason_attach.EnableWindow(0);
		m_btn_add_reason_attach.EnableWindow(0);
		m_cmb_judgement.EnableWindow(0);
		m_judgment_user_define.EnableWindow(0);
		m_judgement_detail.EnableWindow(0);
		m_btn_add_judgment_attach_1.EnableWindow(0);
		m_btn_add_judgment_attach_2.EnableWindow(0);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CAlarmHandleStep4Dlg::OnTimer(UINT_PTR nIDEvent)
{
	if (!read_only_ && m_cmb_guard.GetCurSel() != 0) {
		//KillTimer(c_timer_id_update_date);
		m_assign_time.SetWindowTextW(now_to_wstring().c_str());
		OnEnKillfocusEditPredictMinutes();
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CAlarmHandleStep4Dlg::OnBnClickedButtonMgrGuard()
{
	CSecurityGuardMgrDlg dlg;
	dlg.DoModal();

	update_guard();
}


void CAlarmHandleStep4Dlg::OnBnClickedButtonSwitchUser()
{
	CLoginDlg dlg;
	dlg.DoModal();

	auto user = user_manager::get_instance()->get_cur_user_info();
	m_user.SetWindowTextW(user->get_formmated_name().c_str());
}


void CAlarmHandleStep4Dlg::OnBnClickedButtonAddReasonAttach()
{
	std::wstring path;
	if (!jlib::get_file_open_dialog_result(path, m_hWnd)) {
		return;
	}
	m_reason_attach.SetWindowTextW(path.c_str());
}


void CAlarmHandleStep4Dlg::OnBnClickedButtonAddJudgmentAttach1()
{
	std::wstring path;
	if (!jlib::get_file_open_dialog_result(path, m_hWnd)) {
		return;
	}
	m_judgement_attach1.SetWindowTextW(path.c_str());
}


void CAlarmHandleStep4Dlg::OnBnClickedButtonAddJudgementAttach2()
{
	std::wstring path;
	if (!jlib::get_file_open_dialog_result(path, m_hWnd)) {
		return;
	}
	m_judgement_attach2.SetWindowTextW(path.c_str());
}


void CAlarmHandleStep4Dlg::OnEnChangeEditPredictMinutes()
{
	AUTO_LOG_FUNCTION;
	//KillTimer(c_timer_id_update_date);
}


void CAlarmHandleStep4Dlg::OnEnKillfocusEditPredictMinutes()
{
	AUTO_LOG_FUNCTION;

	if (m_cmb_guard.GetCurSel() != 0) {
		m_handle_time.SetWindowTextW(time_point_to_wstring(std::chrono::system_clock::now() + std::chrono::minutes(get_predict_min())).c_str());
	}
}


void CAlarmHandleStep4Dlg::OnCbnSelchangeComboStatus()
{
	
}


void CAlarmHandleStep4Dlg::OnCbnSelchangeComboGuard()
{
	int ndx = m_cmb_guard.GetCurSel();
	int data = m_cmb_guard.GetItemData(ndx);
	bool enable_guard = (ndx != -1 && data != 0);
	m_predict_minutes.EnableWindow(!read_only_ && enable_guard);

	if (enable_guard) {
		m_assign_time.SetWindowTextW(cur_handling_alarm_info_->get_assign_time().c_str());
		CString txt;
		int minutes = alarm_handle::handle_time_default;
		if (handle_) {
			minutes = handle_->get_predict_minutes_to_handle();
		}
		txt.Format(L"%d", minutes);
		m_predict_minutes.SetWindowTextW(txt);
		OnEnKillfocusEditPredictMinutes();
	} else {
		//KillTimer(c_timer_id_update_date);
		m_assign_time.SetWindowTextW(L"");
		m_predict_minutes.SetWindowTextW(L"");
		m_handle_time.SetWindowTextW(L"");
	}
}


void CAlarmHandleStep4Dlg::OnBnClickedOk()
{
	KillTimer(c_timer_id_update_date);

	if (read_only_) {
		CDialogEx::OnOK();
		return;
	}
	
	CString txt;
	auto mgr = alarm_handle_mgr::get_instance();

	// update status
	int ndx = m_cmb_status.GetCurSel();
	int data = m_cmb_status.GetItemData(ndx);
	auto status = alarm_info::integer_to_alarm_status(data);
	cur_handling_alarm_info_ = mgr->execute_update_alarm_status(cur_handling_alarm_info_->get_id(), status);

	// update handle
	ndx = m_cmb_guard.GetCurSel();
	if (ndx == 0) {
		handle_ = nullptr;
		cur_handling_alarm_info_ = mgr->execute_update_alarm_handle(cur_handling_alarm_info_->get_id(), handle_);
	} else {
		int min = get_predict_min();
		int guard_id = m_cmb_guard.GetItemData(ndx);
		auto guard = mgr->get_security_guard(guard_id);

		handle_ = create_alarm_handle(guard_id, wstring_to_time_point(cur_handling_alarm_info_->get_assign_time()),
									  std::chrono::minutes(min), handle_ ? handle_->get_note() : std::wstring());
		cur_handling_alarm_info_ = mgr->execute_update_alarm_handle(cur_handling_alarm_info_->get_id(), handle_);
	}

	// update reason
	ndx = m_cmb_alarm_reason.GetCurSel();
	int reason = m_cmb_alarm_reason.GetItemData(ndx);
	/*if (reason == 0) {
		reason_ = nullptr;
		cur_handling_alarm_info_ = mgr->execute_update_alarm_reason(cur_handling_alarm_info_->get_id(), reason_);
	} else */{
		m_reason_detail.GetWindowTextW(txt);
		std::wstring detail = (LPCTSTR)txt;
		m_reason_attach.GetWindowTextW(txt);
		std::wstring attatch = (LPCTSTR)txt;

		reason_ = create_alarm_reason(alarm_reason::integer_to_by_what(reason), detail, attatch);
		cur_handling_alarm_info_ = mgr->execute_update_alarm_reason(cur_handling_alarm_info_->get_id(), reason_);
	}

	// update judgment
	ndx = m_cmb_judgement.GetCurSel();
	int judgment = m_cmb_judgement.GetItemData(ndx);
	alarm_judgement_type_info judge_type;
	if (judgment == 0) { // user define
		m_judgment_user_define.GetWindowTextW(txt);
		if (txt.IsEmpty()) {
			return; // can't be empty
		}
		judge_type = mgr->execute_add_judgement_type((LPCTSTR)txt); // add a new judgment type
	} else { // several default types
		judge_type = mgr->get_alarm_judgement_type_info(judgment);
	}

	if (judge_type.first == alarm_judgement::alarm_judgement_min) { // not a valid judgement type
		return;
	}

	m_judgement_detail.GetWindowTextW(txt);
	std::wstring judge_note = (LPCTSTR)txt;
	m_judgement_attach1.GetWindowTextW(txt);
	std::wstring judge_note1 = (LPCTSTR)txt;
	m_judgement_attach2.GetWindowTextW(txt);
	std::wstring judge_note2 = (LPCTSTR)txt;
	judgment_ = create_alarm_judgement_ptr(judge_type.first, judge_note, judge_note1, judge_note2);
	cur_handling_alarm_info_ = mgr->execute_update_alarm_judgment(cur_handling_alarm_info_->get_id(), judgment_);

	if (alarm_status::alarm_status_cleared == status) {
		machine_->clear_ademco_event_list();
		machine_->set_alarm_id(0);
	} else {
		machine_->set_alarm_id(cur_handling_alarm_info_->get_id());
	}

	CDialogEx::OnOK();
}


void CAlarmHandleStep4Dlg::OnCbnSelchangeComboJudgement()
{
	int ndx = m_cmb_judgement.GetCurSel();
	if (ndx < 0)return;
	int judgment = m_cmb_judgement.GetItemData(ndx);
	int able = (judgment == alarm_judgement::alarm_judgement_by_video_image);
	m_btn_add_judgment_attach_1.EnableWindow(able);
	m_btn_add_judgment_attach_2.EnableWindow(able);
	able = (judgment == 0); // 0 for user define
	m_judgment_user_define.EnableWindow(able);
}


void CAlarmHandleStep4Dlg::OnBnClickedButtonPrint()
{
	CPrintDialog dlg(FALSE);
	//dlg.GetDefaults();
	int ret = dlg.DoModal();
	if (ret != IDOK) {
		return;
	}

	//DEVMODE devmode = { 0 };
	//devmode.dmOrientation = DMORIENT_PORTRAIT;

	//PRINTDLG   pd;
	//pd.lStructSize = sizeof(PRINTDLG);
	//pd.Flags = PD_RETURNDC;
	//pd.hDC = nullptr;
	//pd.hwndOwner = nullptr;
	//pd.hInstance = nullptr;
	//pd.nMaxPage = 2;
	//pd.nMinPage = 1;
	//pd.nFromPage = 1;
	//pd.nToPage = 1;
	//pd.nCopies = 1;
	//pd.hDevMode = nullptr;
	//pd.hDevNames = nullptr;

	///////////////////////////////////////////////////////////
	////显示打印对话框，由用户来设定纸张大小等.
	//if (!PrintDlg(&pd))   return;
	//ASSERT(pd.hDC != nullptr);/*断言获取的句柄不为空.*/



	//auto pDevMode = dlg.GetDevMode();
	//GlobalLock(pDevMode);
	//pDevMode->dmOrientation = DMORIENT_PORTRAIT;
	//GlobalUnlock(pDevMode);


	auto winDC = GetDC();
	CRect rc;
	GetClientRect(rc);

	if (0) {
		CRect rc_btn;
		m_btn_print.GetWindowRect(rc_btn);
		rc.bottom += rc_btn.Height() + 25;
	}

	// is a default printer set up?
	HDC hdcPrinter = dlg.GetPrinterDC();
	//HDC hdcPrinter = pd.hDC;
	if (hdcPrinter == NULL) {
		MessageBox(_T("Buy a printer!"));
	} else {
		// create a CDC and attach it to the default printer
		CDC dcPrinter;
		dcPrinter.Attach(hdcPrinter);

		// call StartDoc() to begin printing
		DOCINFO docinfo;
		memset(&docinfo, 0, sizeof(docinfo));
		docinfo.cbSize = sizeof(docinfo);
		docinfo.lpszDocName = _T("CDC::StartDoc() Code Fragment");

		// if it fails, complain and exit gracefully
		if (dcPrinter.StartDoc(&docinfo) < 0) {
			MessageBox(_T("Printer wouldn't initalize"));
		} else {
			// start a page
			if (dcPrinter.StartPage() < 0) {
				//MessageBox(_T("Could not start page"));
				dcPrinter.AbortDoc();
			} else {
				int   nHorRes = dcPrinter.GetDeviceCaps(HORZRES);
				int   nVerRes = dcPrinter.GetDeviceCaps(VERTRES);
				int   nXMargin = 20;//页边的空白   
				int   nYMargin = 5;

				dcPrinter.StretchBlt(nXMargin, nYMargin, nHorRes - 2 * nXMargin, nVerRes - 2 * nYMargin, winDC,
									 0, 0, rc.Width(), rc.Height(), SRCCOPY);

				dcPrinter.EndPage();
				dcPrinter.EndDoc();
			}
		}
	}
}
