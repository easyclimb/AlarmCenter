// AlarmHandleStep1Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AlarmHandleStep1Dlg.h"
#include "AlarmHandleStep2Dlg.h"
#include "AlarmHandleStep3Dlg.h"
#include "AlarmHandleStep4Dlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "AlarmMachineManager.h"
#include "MapInfo.h"
#include "ZoneInfo.h"
#include "alarm_handle_mgr.h"

using namespace core;


class CAlarmHandleStep1Dlg::alarm_text_observer : public dp::observer<core::icmc_buffer_ptr>
{
public:
	virtual void on_update(const core::icmc_buffer_ptr& ptr) override {
		if (dlg) {
			dlg->add_alarm_text(ptr->_at);
		}
	}

	CAlarmHandleStep1Dlg* dlg = nullptr;
};

// CAlarmHandleStep1Dlg dialog

IMPLEMENT_DYNAMIC(CAlarmHandleStep1Dlg, CDialogEx)

CAlarmHandleStep1Dlg::CAlarmHandleStep1Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_ALARM_HANDLE_1, pParent)
{

}

CAlarmHandleStep1Dlg::~CAlarmHandleStep1Dlg()
{
}


core::alarm_type CAlarmHandleStep1Dlg::get_alarm_type()
{
	if (m_radio_true_alarm.GetCheck()) {
		return alarm_type_true;
	} else if (m_radio_device_false_positive.GetCheck()) {
		return alarm_type_device_false_positive;
	} else if (m_radio_test_device.GetCheck()) {
		return alarm_type_test_device;
	} else if (m_radio_man_made_false_posotive.GetCheck()) {
		return alarm_type_man_made_false_positive;
	}

	return alarm_type_cannot_determine;
}

void CAlarmHandleStep1Dlg::add_alarm_text(const core::alarm_text_ptr & at)
{
	if (!machine_->get_is_submachine()) {
		if (at->_subzone == 0) {
			alarm_texts_.push_back(at);
		}
	} else {
		alarm_texts_.push_back(at);
	}
	
}

void CAlarmHandleStep1Dlg::prepair()
{
	//cur_handling_alarm_text_ = nullptr;
	alarm_texts_.clear();

	auto no_zone_map = machine_->GetUnbindZoneMap();
	if (no_zone_map) {
		no_zone_map->TraverseAlarmText(alarm_text_observer_);
	}

	core::map_info_list map_list;
	machine_->GetAllMapInfo(map_list);
	for (auto map : map_list) {
		map->TraverseAlarmText(alarm_text_observer_);
	}

	CString txt;
	txt.Format(L"%06d", machine_->get_ademco_id());
	m_aid.SetWindowTextW(txt);
	m_name.SetWindowTextW(machine_->get_machine_name());
	m_contact.SetWindowTextW(machine_->get_contact());
	m_addr.SetWindowTextW(machine_->get_address());
	m_phone.SetWindowTextW(machine_->get_phone());
	m_phone_bk.SetWindowTextW(machine_->get_phone_bk());
}

void CAlarmHandleStep1Dlg::show_one()
{
	if (!alarm_texts_.empty()) {
		//auto iter = alarm_texts_.begin();
		//cur_handling_alarm_text_ = *iter;
		//alarm_texts_.erase(iter);



		//m_alarm_text.SetWindowTextW(cur_handling_alarm_text_->_txt);

		auto mgr = alarm_handle_mgr::get_instance();
		cur_handling_alarm_info_ = mgr->execute_add_alarm(machine_->get_ademco_id(),
														  machine_->get_is_submachine() ? machine_->get_submachine_zone() : 0,
														  machine_->get_is_submachine(),
														  /*(LPCTSTR)(cur_handling_alarm_text_->_txt),
														  std::chrono::system_clock::from_time_t(cur_handling_alarm_text_->_time),*/
														  0, 0, 0);

		cur_handling_alarm_info_ = mgr->execute_add_alarm_texts(cur_handling_alarm_info_->get_id(), alarm_texts_);

		m_alarm_text.SetWindowTextW(cur_handling_alarm_info_->get_text().c_str());

	}
}

void CAlarmHandleStep1Dlg::handle_one()
{
	bool handled = false;
	bool need_security_guard = false;

	auto alarm_type = get_alarm_type();
	alarm_judgement_ptr judgement = nullptr;
	alarm_handle_ptr handle = nullptr;

	auto mgr = core::alarm_handle_mgr::get_instance();

	// resolve alarm reason
	auto reason = mgr->execute_add_alarm_reason(alarm_type, L"", L"");
	cur_handling_alarm_info_ = mgr->execute_update_alarm_reason(cur_handling_alarm_info_->get_id(), reason);
	cur_handling_alarm_info_ = mgr->execute_update_alarm_status(cur_handling_alarm_info_->get_id(), alarm_status::alarm_status_not_judged);

	switch (alarm_type) {
	case core::alarm_type_true:
	case core::alarm_type_device_false_positive:
	case core::alarm_type_man_made_false_positive:
	case core::alarm_type_test_device:
	{
		// resolve alarm judgment
		CAlarmHandleStep2Dlg dlg;
		if (IDOK != dlg.DoModal()) {
			break;
		}

		int judgement_id = dlg.prev_sel_alarm_judgement_;
		if (judgement_id == alarm_judgement_by_user_define) {
			judgement_id = dlg.prev_user_defined_;
		}

		judgement = mgr->execute_add_judgment(judgement_id, dlg.note_, dlg.video_, dlg.image_);
		cur_handling_alarm_info_ = mgr->execute_update_alarm_judgment(cur_handling_alarm_info_->get_id(), judgement);

		if (alarm_type == alarm_type_true) {
			cur_handling_alarm_info_ = mgr->execute_update_alarm_status(cur_handling_alarm_info_->get_id(), alarm_status::alarm_status_not_handled);
			need_security_guard = true;
		} else {
			cur_handling_alarm_info_ = mgr->execute_update_alarm_status(cur_handling_alarm_info_->get_id(), alarm_status::alarm_status_cleared);
			handled = true;
		}
	}
		break;
	
	case core::alarm_type_cannot_determine:
		need_security_guard = true;
		break;

	default:
		break;
	}

	if (need_security_guard) {
		// reolve alarm handle

		CAlarmHandleStep3Dlg dlg;
		if (IDOK == dlg.DoModal()) {
			handle = create_alarm_handle(dlg.cur_editting_guard_id_,
										 wstring_to_time_point(cur_handling_alarm_info_->get_assign_time()),
										 std::chrono::minutes(dlg.cur_editting_handle_time_),
										 dlg.cur_editting_note_);

			cur_handling_alarm_info_ = mgr->execute_update_alarm_handle(cur_handling_alarm_info_->get_id(), handle);
			cur_handling_alarm_info_ = mgr->execute_update_alarm_status(cur_handling_alarm_info_->get_id(), alarm_status::alarm_status_not_cleared);
			handled = true;
		} else {
			handled = false;
		}
	}

	if (handled) {
		//show_result();

		CAlarmHandleStep4Dlg dlg;
		dlg.cur_handling_alarm_info_ = cur_handling_alarm_info_;
		dlg.judgment_ = judgement;
		dlg.handle_ = handle;
		dlg.reason_ = reason;
		dlg.machine_ = machine_;
		if (IDOK != dlg.DoModal()) {
			handled = false;
		}

		if (handled && !alarm_texts_.empty()) {
			//show_one();
		}
	}
}


void CAlarmHandleStep1Dlg::show_result()
{
	
}

void CAlarmHandleStep1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_AID, m_aid);
	DDX_Control(pDX, IDC_EDIT_NAME, m_name);
	DDX_Control(pDX, IDC_EDIT_CONTACT, m_contact);
	DDX_Control(pDX, IDC_EDIT_ADDR, m_addr);
	DDX_Control(pDX, IDC_EDIT_PHONE, m_phone);
	DDX_Control(pDX, IDC_EDIT_PHONE_BK, m_phone_bk);
	DDX_Control(pDX, IDC_EDIT_ALARM_INFO, m_alarm_text);
	DDX_Control(pDX, IDC_RADIO1, m_radio_true_alarm);
	DDX_Control(pDX, IDC_RADIO2, m_radio_device_false_positive);
	DDX_Control(pDX, IDC_RADIO3, m_radio_test_device);
	DDX_Control(pDX, IDC_RADIO4, m_radio_man_made_false_posotive);
	DDX_Control(pDX, IDC_RADIO5, m_radio_cannot_determine);
}


BEGIN_MESSAGE_MAP(CAlarmHandleStep1Dlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAlarmHandleStep1Dlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CAlarmHandleStep1Dlg message handlers


BOOL CAlarmHandleStep1Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText(TR(IDS_STRING_ALARM_HANDLE_STEP_1));

	SET_WINDOW_TEXT(IDC_STATIC_AID, IDS_STRING_IDC_STATIC_008);
	SET_WINDOW_TEXT(IDC_STATIC_NAME, IDS_STRING_IDC_STATIC_011);
	SET_WINDOW_TEXT(IDC_STATIC_CONTACT, IDS_STRING_IDC_STATIC_012);
	SET_WINDOW_TEXT(IDC_STATIC_ADDR, IDS_STRING_IDC_STATIC_013);
	SET_WINDOW_TEXT(IDC_STATIC_PHONE, IDS_STRING_IDC_STATIC_014);
	SET_WINDOW_TEXT(IDC_STATIC_PHONE_BK, IDS_STRING_IDC_STATIC_015);
	SET_WINDOW_TEXT(IDC_STATIC_GROUP_ALARM_INFO, IDS_STRING_HRLV_ALARM);

	SET_WINDOW_TEXT(IDC_RADIO1, IDS_STRING_ALARM_TYPE_TRUE);
	SET_WINDOW_TEXT(IDC_RADIO2, IDS_STRING_ALARM_TYPE_DEVICE_FALSE_POSITIVE);
	SET_WINDOW_TEXT(IDC_RADIO3, IDS_STRING_ALARM_TYPE_TEST_DEVICE);
	SET_WINDOW_TEXT(IDC_RADIO4, IDS_STRING_ALARM_TYPE_MAN_MADE_FALSE_POSITIVE);
	SET_WINDOW_TEXT(IDC_RADIO5, IDS_STRING_ALARM_TYPE_UNABLE_TO_DETERMINE);

	SET_WINDOW_TEXT(IDOK, IDS_OK);
	SET_WINDOW_TEXT(IDCANCEL, IDS_CANCEL);


	m_radio_cannot_determine.SetCheck(1);

	alarm_text_observer_ = std::make_shared<alarm_text_observer>();
	alarm_text_observer_->dlg = this;

	if (machine_) {
		prepair();
		show_one();
	}



	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CAlarmHandleStep1Dlg::OnBnClickedOk()
{
	handle_one();

	

	CDialogEx::OnOK();
}
