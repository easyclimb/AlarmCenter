// EditZoneDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "EditZoneDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "ZoneInfo.h"
#include "MapInfo.h"
#include "AlarmMachineManager.h"
#include "AddZoneDlg.h"
#include "DetectorInfo.h"
#include "DetectorLib.h"
#include "ChooseDetDlg.h"
#include "AlarmMachineDlg.h"
#include "RetrieveProgressDlg.h"
#include "SubMachineExpireManagerDlg.h"
#include "AutoRetrieveZoneInfoDlg.h"
#include "UserInfo.h"
#include "MannualyAddZoneWrite2MachineDlg.h"
#include "ChooseVideoDeviceDlg.h"
#include "../video/ezviz/VideoUserInfoEzviz.h"
#include "../video/ezviz/VideoDeviceInfoEzviz.h"
#include "ConfigHelper.h"
#include "HistoryRecord.h"
#include "VideoManager.h"
#include "alarm_center_video_service.h"

using namespace core;


class CEditZoneDlg::db_updated_observer : public dp::observer<int>
{
public:
	virtual void on_update(const int&) override {
		if (dlg) {
			dlg->PostMessageW(WM_VIDEO_INFO_CHANGE);
		}
	}

	CEditZoneDlg* dlg;
};

//const int TIMER_ID_UPDATE_MACHINE_INFO = 1;


using namespace detail;

// CEditZoneDlg dialog
#ifdef _DEBUG
#define NEW_FEATURE_NET_MOD 1
#else
#define NEW_FEATURE_NET_MOD 1
#endif

IMPLEMENT_DYNAMIC(CEditZoneDlg, CDialogEx)

CEditZoneDlg::CEditZoneDlg(CWnd* pParent /*=nullptr*/)
: CDialogEx(CEditZoneDlg::IDD, pParent)
, m_machine(nullptr)
, m_rootItem(nullptr)
, m_bNeedReloadMaps(FALSE)
, m_machineDlg(nullptr)
{

}

CEditZoneDlg::~CEditZoneDlg()
{}

void CEditZoneDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE1, m_tree);
	DDX_Control(pDX, IDC_EDIT_ZONE, m_zone);
	DDX_Control(pDX, IDC_COMBO_TYPE, m_type);
	DDX_Control(pDX, IDC_EDIT_ALIAS, m_alias);
	DDX_Control(pDX, IDC_STATIC_SUBMACHINE, m_groupSubMachine);
	DDX_Control(pDX, IDC_EDIT_CONTACT, m_contact);
	DDX_Control(pDX, IDC_EDIT_ADDRESS, m_addr);
	DDX_Control(pDX, IDC_EDIT_PHONE, m_phone);
	DDX_Control(pDX, IDC_EDIT_PHONE_BK, m_phone_bk);
	DDX_Control(pDX, IDC_BUTTON_AUTO_RETRIEVE, m_btnAutoRetrieveZoneInfo);
	DDX_Control(pDX, IDC_BUTTON_DELZONE, m_btnDeleteZone);
	DDX_Control(pDX, IDC_CHECK1, m_chk_report_status);
	DDX_Control(pDX, IDC_CHECK4, m_chk_report_status_bk);
	DDX_Control(pDX, IDC_CHECK2, m_chk_report_exception);
	DDX_Control(pDX, IDC_CHECK5, m_chk_report_exception_bk);
	DDX_Control(pDX, IDC_CHECK3, m_chk_report_alarm);
	DDX_Control(pDX, IDC_CHECK6, m_chk_report_alarm_bk);
	DDX_Control(pDX, IDC_BUTTON_MANULLY_ADD_ZONE_WRITE_TO_MACHINE, m_btnManualyAddZoneWrite2Machine);
	DDX_Control(pDX, IDC_EDIT_PHYSIC_ADDR, m_pyisic_addr);
	DDX_Control(pDX, IDC_BUTTON_BIND_OR_UNBIND_VIDEO_DEVICE, m_btnBindOrUnbindVideoDevice);
	DDX_Control(pDX, IDC_CHECK_AUTO_PLAY_VIDEO_ON_ALARM, m_chkAutoPlayVideoOnAlarm);
	DDX_Control(pDX, IDC_BUTTON_PREVIEW, m_btnPreview);
	DDX_Control(pDX, IDC_EDIT_DEV_INFO, m_editDevInfo);
	DDX_Control(pDX, IDC_BUTTON_ADDZONE, m_btnAddZone);
	DDX_Control(pDX, IDC_BUTTON_SAVE, m_btn_save);
}


BEGIN_MESSAGE_MAP(CEditZoneDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_ADDZONE, &CEditZoneDlg::OnBnClickedButtonAddzone)
	ON_BN_CLICKED(IDC_BUTTON_DELZONE, &CEditZoneDlg::OnBnClickedButtonDelzone)
	ON_CBN_SELCHANGE(IDC_COMBO_TYPE, &CEditZoneDlg::OnCbnSelchangeComboZoneType)
	ON_EN_CHANGE(IDC_EDIT_ALIAS, &CEditZoneDlg::OnEnChangeEditAlias)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, &CEditZoneDlg::OnTvnSelchangedTreeZone)
	ON_EN_CHANGE(IDC_EDIT_CONTACT, &CEditZoneDlg::OnEnChangeEditContact)
	ON_EN_CHANGE(IDC_EDIT_ADDRESS, &CEditZoneDlg::OnEnChangeEditAddress)
	ON_EN_CHANGE(IDC_EDIT_PHONE, &CEditZoneDlg::OnEnChangeEditPhone)
	ON_EN_CHANGE(IDC_EDIT_PHONE_BK, &CEditZoneDlg::OnEnChangeEditPhoneBk)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_DETECTOR, &CEditZoneDlg::OnBnClickedButtonEditDetector)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_MANAGE_SUBMACHINE_EXPIRE_TIME, &CEditZoneDlg::OnBnClickedButtonManageSubmachineExpireTime)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_RETRIEVE, &CEditZoneDlg::OnBnClickedButtonAutoRetrieve)
	ON_BN_CLICKED(IDC_CHECK1, &CEditZoneDlg::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_CHECK2, &CEditZoneDlg::OnBnClickedCheck2)
	ON_BN_CLICKED(IDC_CHECK3, &CEditZoneDlg::OnBnClickedCheck3)
	ON_BN_CLICKED(IDC_CHECK4, &CEditZoneDlg::OnBnClickedCheck4)
	ON_BN_CLICKED(IDC_CHECK5, &CEditZoneDlg::OnBnClickedCheck5)
	ON_BN_CLICKED(IDC_CHECK6, &CEditZoneDlg::OnBnClickedCheck6)
	ON_BN_CLICKED(IDC_BUTTON_MANULLY_ADD_ZONE_WRITE_TO_MACHINE, &CEditZoneDlg::OnBnClickedButtonManullyAddZoneWriteToMachine)
	ON_BN_CLICKED(IDC_BUTTON_BIND_OR_UNBIND_VIDEO_DEVICE, &CEditZoneDlg::OnBnClickedButtonBindOrUnbindVideoDevice)
	ON_BN_CLICKED(IDC_CHECK_AUTO_PLAY_VIDEO_ON_ALARM, &CEditZoneDlg::OnBnClickedCheckAutoPlayVideoOnAlarm)
	ON_MESSAGE(WM_VIDEO_INFO_CHANGE, &CEditZoneDlg::OnVideoInfoChanged)
	ON_BN_CLICKED(IDC_BUTTON_PREVIEW, &CEditZoneDlg::OnBnClickedButtonPreview)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CEditZoneDlg::OnBnClickedButtonSave)
END_MESSAGE_MAP()


// CEditZoneDlg message handlers


BOOL CEditZoneDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetWindowText(TR(IDS_STRING_IDD_DIALOG_EDIT_ZONE));
	SET_WINDOW_TEXT(IDC_BUTTON_MANULLY_ADD_ZONE_WRITE_TO_MACHINE, IDS_STRING_IDC_BUTTON_MANULLY_ADD_ZONE_WRITE_TO_MACHINE);
	SET_WINDOW_TEXT(IDC_BUTTON_AUTO_RETRIEVE, IDS_STRING_IDC_BUTTON_AUTO_RETRIEVE);
	SET_WINDOW_TEXT(IDC_STATIC, IDS_STRING_IDC_STATIC_075);
	SET_WINDOW_TEXT(IDC_STATIC_2, IDS_STRING_ZONE);
	SET_WINDOW_TEXT(IDC_STATIC_3, IDS_STRING_HRLV);
	SET_WINDOW_TEXT(IDC_STATIC_4, IDS_STRING_IDC_STATIC_079);
	SET_WINDOW_TEXT(IDC_STATIC_5, IDS_STRING_ALIAS);
	SET_WINDOW_TEXT(IDC_STATIC_6, IDS_STRING_IDC_STATIC_080);
	SET_WINDOW_TEXT(IDC_BUTTON_BIND_OR_UNBIND_VIDEO_DEVICE, IDS_STRING_BIND_VIDEO_DEVICE);
	SET_WINDOW_TEXT(IDC_BUTTON_PREVIEW, IDS_STRING_IDC_BUTTON_PLAY);
	//SET_WINDOW_TEXT(IDC_CHECK_AUTO_PLAY_VIDEO_ON_ALARM, IDS_STRING_IDC_BUTTON_PLAY);	//报警时自动播放
	SET_WINDOW_TEXT(IDC_STATIC_SUBMACHINE, IDS_STRING_IDC_STATIC_SUBMACHINE);
	SET_WINDOW_TEXT(IDC_STATIC_7, IDS_STRING_CONTACT);
	SET_WINDOW_TEXT(IDC_STATIC_8, IDS_STRING_IDC_STATIC_013);
	SET_WINDOW_TEXT(IDC_STATIC_9, IDS_STRING_PHONE);
	SET_WINDOW_TEXT(IDC_CHECK1, IDS_STRING_REPORT_CHANGE);
	SET_WINDOW_TEXT(IDC_CHECK2, IDS_STRING_ERPORT_ABNORMAL_EVENTS);
	SET_WINDOW_TEXT(IDC_CHECK3, IDS_STRING_ALARM_EVENTS);
	SET_WINDOW_TEXT(IDC_STATIC_10, IDS_STRING_IDC_STATIC_015);
	SET_WINDOW_TEXT(IDC_CHECK4, IDS_STRING_REPORT_CHANGE);
	SET_WINDOW_TEXT(IDC_CHECK5, IDS_STRING_ERPORT_ABNORMAL_EVENTS);
	SET_WINDOW_TEXT(IDC_CHECK6, IDS_STRING_ALARM_EVENTS);
	SET_WINDOW_TEXT(IDC_CHECK_AUTO_PLAY_VIDEO_ON_ALARM, IDS_STRING_WHEN_ALARM_AUTOMATICALLY);
	SET_WINDOW_TEXT(IDC_BUTTON_SAVE, IDS_STRING_IDC_BUTTON_SAVE_CHANGE);


	assert(m_machine);

	CString txt, ssensor, ssubmachine, sssubmachine;
	ssensor = TR(IDS_STRING_SENSOR);
	ssubmachine = TR(IDS_STRING_SUBMACHINE);
	sssubmachine = TR(IDS_STRING_SSUBMACHINE);

	int ndx = m_type.InsertString(ZT_ZONE, ssensor);
	VERIFY(ndx == ZT_ZONE);
	if (!m_machine->get_is_submachine()) {
		ndx = m_type.InsertString(ZT_SUB_MACHINE, ssubmachine);
		VERIFY(ndx == ZT_SUB_MACHINE);
		m_btnAutoRetrieveZoneInfo.ShowWindow(SW_SHOW);
		m_btnManualyAddZoneWrite2Machine.ShowWindow(SW_SHOW);
	} else {
		m_btnAutoRetrieveZoneInfo.ShowWindow(SW_HIDE);
		m_btnManualyAddZoneWrite2Machine.ShowWindow(SW_HIDE);
	}

	if (NEW_FEATURE_NET_MOD) {
		m_type.EnableWindow(0);//2015-10-16 17:32:47 disable function of change zone type
		if (MT_NETMOD == m_machine->get_machine_type()) {
				
		} else {
			m_btnManualyAddZoneWrite2Machine.ShowWindow(SW_HIDE);
			m_btnAutoRetrieveZoneInfo.ShowWindow(SW_HIDE);
		}
	}

	if (m_machine->get_machine_type() == MT_IMPRESSED_GPRS_MACHINE_2050 || m_machine->get_machine_type() == MT_LCD) {
		m_btnAddZone.ShowWindow(SW_HIDE);
		m_btnDeleteZone.ShowWindow(SW_HIDE);
		m_btnAutoRetrieveZoneInfo.ShowWindow(SW_SHOW);
	}

	Init();

	auto userMgr = user_manager::get_instance();
	user_info_ptr user = userMgr->get_cur_user_info();
	core::user_priority user_priority = user->get_priority();
	switch (user_priority) {
		case core::UP_SUPER:
		case core::UP_ADMIN:
			m_btnDeleteZone.EnableWindow(1);
			m_btnBindOrUnbindVideoDevice.EnableWindow(1);
			break;
		case core::UP_OPERATOR:
		default:
			m_btnDeleteZone.EnableWindow(0);
			m_type.EnableWindow(0);
			m_btnBindOrUnbindVideoDevice.EnableWindow(0);
			m_chkAutoPlayVideoOnAlarm.EnableWindow(0);
			m_btnPreview.EnableWindow(0);
			break;
	}

	db_updated_observer_ = std::make_shared<db_updated_observer>();
	db_updated_observer_->dlg = this;
	ipc::alarm_center_video_service::get_instance()->register_observer(db_updated_observer_);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CEditZoneDlg::Init()
{
	m_tree.DeleteAllItems();
	CString sroot;
	sroot = TR(IDS_STRING_ZONE_INFO);
	HTREEITEM hRoot = m_tree.GetRootItem();
	m_rootItem = m_tree.InsertItem(sroot, hRoot);
	m_tree.SetItemData(m_rootItem, 0);
	m_tree.Expand(m_rootItem, TVE_EXPAND);

	CString txt = L"";

	txt.Format(L"%s-%s", m_machine->get_is_submachine() ? TR(IDS_STRING_SUBMACHINE) : TR(IDS_STRING_MACHINE), TR(IDS_STRING_SELF));
	m_tree.SetItemData(m_tree.InsertItem(txt, m_rootItem), ZONE_VALUE_FOR_MACHINE_SELF);

	zone_info_list list;
	m_machine->GetAllZoneInfo(list);
	for (auto zoneInfo : list) {
		FormatZoneInfoText(m_machine, zoneInfo, txt);
		HTREEITEM hChild = m_tree.InsertItem(txt, m_rootItem);
		m_tree.SetItemData(hChild, m_machine->get_is_submachine() ? zoneInfo->get_sub_zone() : zoneInfo->get_zone_value());
	}

	m_tree.Expand(m_rootItem, TVE_EXPAND);
	ExpandWindow(false);
}


void CEditZoneDlg::FormatZoneInfoText(const core::alarm_machine_ptr& machine,
									  zone_info_ptr zoneInfo,
									  CString& txt)
{
	AUTO_LOG_FUNCTION;
	CString szone, ssensor, sssubmachine, salias;
	ssensor = TR(IDS_STRING_SENSOR);
	sssubmachine = TR(IDS_STRING_SSUBMACHINE);

	if (machine->get_is_submachine()) {
		szone.Format(L"%02d", zoneInfo->get_sub_zone());
	}
	else {
		if (machine->get_machine_type() == MT_IMPRESSED_GPRS_MACHINE_2050) {
			szone.Format(L"%02d", zoneInfo->get_zone_value());
		} else {
			szone.Format(L"%03d", zoneInfo->get_zone_value());
		}
	}

	salias = zoneInfo->get_alias();
	if (salias.IsEmpty())
		salias = TR(IDS_STRING_NULL);
	txt.Format(L"%s--%s--%s", szone,
			   zoneInfo->get_type() == ZT_SUB_MACHINE ? sssubmachine : ssensor,
			   salias);
}


void CEditZoneDlg::ExpandWindow(bool expand)
{
	AUTO_LOG_FUNCTION;
	CRect rc, rcSub;
	GetWindowRect(rc);
	m_groupSubMachine.GetWindowRect(rcSub);
	m_groupSubMachine.SetWindowTextW(m_machine->get_is_submachine() ? TR(IDS_STRING_SUBMACHINE) : TR(IDS_STRING_MACHINE));

	if (expand) {
		rc.right = rcSub.right + 5;
	} else {
		rc.right = rcSub.left;
	}

	MoveWindow(rc);
}


void CEditZoneDlg::OnTvnSelchangedTreeZone(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	AUTO_LOG_FUNCTION;
	if (pResult)
		*pResult = 0;

	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (!hItem)
		return;

	DWORD data = m_tree.GetItemData(hItem);

	if (data == ZONE_VALUE_FOR_MACHINE_SELF) {
		ExpandWindow();

		CString txt; 
		txt.Format(L"%s-%s", m_machine->get_is_submachine() ? TR(IDS_STRING_SUBMACHINE) : TR(IDS_STRING_MACHINE), TR(IDS_STRING_SELF));
		m_zone.SetWindowTextW(txt);

		CString sBind, sUnbind; sBind = TR(IDS_STRING_BIND_VIDEO_DEVICE); sUnbind = TR(IDS_STRING_IDC_BUTTON_UNBIND);
		m_btnBindOrUnbindVideoDevice.SetWindowTextW(sBind);

		video::zone_uuid zoneUuid;
		zoneUuid._ademco_id = m_machine->get_ademco_id();
		if (m_machine->get_is_submachine()) {
			zoneUuid._zone_value = m_machine->get_submachine_zone();
			zoneUuid._gg = ZONE_VALUE_FOR_MACHINE_SELF;
		} else {
			zoneUuid._zone_value = ZONE_VALUE_FOR_MACHINE_SELF;
			zoneUuid._gg = 0;
		}

		auto bi = video::video_manager::get_instance()->GetBindInfo(zoneUuid);
		if (bi._device) {
			m_btnBindOrUnbindVideoDevice.SetWindowTextW(sUnbind);
			m_chkAutoPlayVideoOnAlarm.SetCheck(bi.auto_play_when_alarm_);
			m_btnPreview.EnableWindow();
			txt.Format(L"%s[%d,%s]", bi._device->get_userInfo()->get_user_name().c_str(), bi._device->get_id(), bi._device->get_device_note().c_str());
			m_editDevInfo.SetWindowTextW(txt);
		} else {
			m_btnBindOrUnbindVideoDevice.SetWindowTextW(sBind);
			m_chkAutoPlayVideoOnAlarm.SetCheck(0);
			m_btnPreview.EnableWindow(0);
			m_editDevInfo.SetWindowTextW(L"");
		}

		if (user_manager::get_instance()->get_cur_user_priority() == UP_OPERATOR) {
			m_btnBindOrUnbindVideoDevice.EnableWindow(0);
			m_chkAutoPlayVideoOnAlarm.EnableWindow(0);
		} else {
			m_btnBindOrUnbindVideoDevice.EnableWindow(1);
			m_chkAutoPlayVideoOnAlarm.EnableWindow(1);
		}

		m_type.SetCurSel(-1);
		m_type.EnableWindow(0);
		m_alias.SetWindowTextW(m_machine->get_machine_name());
		m_pyisic_addr.SetWindowTextW(L"");

		m_contact.SetWindowTextW(m_machine->get_contact());
		m_addr.SetWindowTextW(m_machine->get_address());
		m_phone.SetWindowTextW(m_machine->get_phone());
		m_phone_bk.SetWindowTextW(m_machine->get_phone_bk());
		sms_config cfg = m_machine->get_sms_cfg();
		m_chk_report_alarm.SetCheck(cfg.report_alarm);
		m_chk_report_status.SetCheck(cfg.report_status);
		m_chk_report_exception.SetCheck(cfg.report_exception);
		m_chk_report_alarm_bk.SetCheck(cfg.report_alarm_bk);
		m_chk_report_status_bk.SetCheck(cfg.report_status_bk);
		m_chk_report_exception_bk.SetCheck(cfg.report_exception_bk);


	} else {
		zone_info_ptr zoneInfo = m_machine->GetZone(data);
		if (!zoneInfo) {
			m_zone.SetWindowTextW(L"");
			m_type.SetCurSel(-1);
			m_alias.SetWindowTextW(L"");
			ExpandWindow(false);
			return;
		}
		bool bsub = (ZT_SUB_MACHINE == zoneInfo->get_type());
		ExpandWindow(bsub);

		CString sBind, sUnbind; sBind = TR(IDS_STRING_BIND_VIDEO_DEVICE); sUnbind = TR(IDS_STRING_IDC_BUTTON_UNBIND);
		m_btnBindOrUnbindVideoDevice.SetWindowTextW(sBind);
		//if (bsub) {
		//	/*m_btnBindOrUnbindVideoDevice.EnableWindow(0);
		//	m_chkAutoPlayVideoOnAlarm.SetCheck(0);
		//	m_chkAutoPlayVideoOnAlarm.EnableWindow(0);
		//	m_btnPreview.EnableWindow(0);
		//	m_editDevInfo.SetWindowTextW(L"");*/
		//} else {
			video::zone_uuid zoneUuid(m_machine->get_ademco_id(), zoneInfo->get_zone_value(), 0);
			if (m_machine->get_is_submachine()) {
				zoneUuid._gg = zoneInfo->get_sub_zone();
			} else if (bsub) {
				zoneUuid._gg = ZONE_VALUE_FOR_MACHINE_SELF;
			}
			//video::bind_info bi = ipc::alarm_center_video_service::get_instance()->get_bind_info(zoneUuid);
			auto bi = video::video_manager::get_instance()->GetBindInfo(zoneUuid);
			if (bi._device) {
				m_btnBindOrUnbindVideoDevice.SetWindowTextW(sUnbind);
				m_chkAutoPlayVideoOnAlarm.SetCheck(bi.auto_play_when_alarm_);
				m_btnPreview.EnableWindow();
				CString txt;
				txt.Format(L"%s[%d,%s]", bi._device->get_userInfo()->get_user_name().c_str(), bi._device->get_id(), bi._device->get_device_note().c_str());
				m_editDevInfo.SetWindowTextW(txt);
			} else {
				m_btnBindOrUnbindVideoDevice.SetWindowTextW(sBind);
				m_chkAutoPlayVideoOnAlarm.SetCheck(0);
				m_btnPreview.EnableWindow(0);
				m_editDevInfo.SetWindowTextW(L"");
			}

			if (user_manager::get_instance()->get_cur_user_priority() == UP_OPERATOR) {
				m_btnBindOrUnbindVideoDevice.EnableWindow(0);
				m_chkAutoPlayVideoOnAlarm.EnableWindow(0);
			} else {
				m_btnBindOrUnbindVideoDevice.EnableWindow(1);
				m_chkAutoPlayVideoOnAlarm.EnableWindow(1);
			}

		CString spysic_addr, szone, salias, scontact, saddr;
		if (m_machine->get_is_submachine()) {
			szone.Format(L"%02d", zoneInfo->get_sub_zone());
		} else {
			szone.Format(L"%03d", zoneInfo->get_zone_value());
		}

		m_zone.SetWindowTextW(szone);
		m_type.SetCurSel(bsub ? ZT_SUB_MACHINE : ZT_ZONE);
		m_alias.SetWindowTextW(zoneInfo->get_alias());
		spysic_addr.Format(L"%04X", zoneInfo->get_physical_addr() & 0xFFFF);
		m_pyisic_addr.SetWindowTextW(spysic_addr);
		if (bsub) {
			alarm_machine_ptr subMachine = zoneInfo->GetSubMachineInfo();
			if (subMachine) {
				m_contact.SetWindowTextW(subMachine->get_contact());
				m_addr.SetWindowTextW(subMachine->get_address());
				m_phone.SetWindowTextW(subMachine->get_phone());
				m_phone_bk.SetWindowTextW(subMachine->get_phone_bk());
				sms_config cfg = subMachine->get_sms_cfg();
				m_chk_report_alarm.SetCheck(cfg.report_alarm);
				m_chk_report_status.SetCheck(cfg.report_status);
				m_chk_report_exception.SetCheck(cfg.report_exception);
				m_chk_report_alarm_bk.SetCheck(cfg.report_alarm_bk);
				m_chk_report_status_bk.SetCheck(cfg.report_status_bk);
				m_chk_report_exception_bk.SetCheck(cfg.report_exception_bk);
			}
		}
	}

	
}


void CEditZoneDlg::SelectItem(int zone_value)
{
	AUTO_LOG_FUNCTION;
	HTREEITEM hItem = m_tree.GetChildItem(m_rootItem);
	while (hItem) {
		if (m_tree.GetItemData(hItem) == static_cast<DWORD>(zone_value)) {
			m_tree.SelectItem(hItem); break;
		}
		hItem = m_tree.GetNextSiblingItem(hItem);
	}
}


int __stdcall CEditZoneDlg::MyTreeCompareProc(LPARAM lp1, LPARAM lp2, LPARAM lpSort)
{
	CEditZoneDlg* dlg = reinterpret_cast<CEditZoneDlg*>(lpSort);
	zone_info_ptr zoneInfo1 = dlg->m_machine->GetZone(lp1);
	zone_info_ptr zoneInfo2 = dlg->m_machine->GetZone(lp2);

	if (dlg && zoneInfo1 && zoneInfo2) {
		if (dlg->m_machine->get_is_submachine())
			return zoneInfo1->get_sub_zone() - zoneInfo2->get_sub_zone();
		else
			return zoneInfo1->get_zone_value() - zoneInfo2->get_zone_value();
	}

	return 0;
}


bool CEditZoneDlg::CreateSubMachine(const core::zone_info_ptr& zoneInfo, bool let_machine_online)
{
	CString null;
	null = TR(IDS_STRING_NULL);
	alarm_machine_ptr subMachine = std::make_shared<alarm_machine>();
	subMachine->set_is_submachine(true);
	subMachine->set_ademco_id(m_machine->get_ademco_id());
	subMachine->set_submachine_zone(zoneInfo->get_zone_value());
	subMachine->set_alias(zoneInfo->get_alias());
	subMachine->set_address(null);
	subMachine->set_contact(null);
	subMachine->set_phone(null);
	subMachine->set_phone_bk(null);
	subMachine->set_machine_type(m_machine->get_machine_type());
	subMachine->set_online(true);
	auto expire_time = std::chrono::system_clock::now();
	// add a year
	{
		expire_time += std::chrono::hours(24) * 365;
	}
	subMachine->set_expire_time(expire_time);
	if (!zoneInfo->execute_set_sub_machine(subMachine)) {
		ASSERT(0); JLOG(L"execute_set_sub_machine failed.\n"); return false;
	}
	char status = zoneInfo->get_status_or_property() & 0xFF;
	ADEMCO_EVENT ademco_event = static_cast<ADEMCO_EVENT>(zone_info::char_to_status(status));
	if (let_machine_online) {
		m_machine->SetAdemcoEvent(ES_UNKNOWN, EVENT_ONLINE, zoneInfo->get_zone_value(), 0xEE, time(nullptr), time(nullptr));
	}
	m_machine->SetAdemcoEvent(ES_UNKNOWN, ademco_event, zoneInfo->get_zone_value(), 0xEE, time(nullptr), time(nullptr));
	m_bNeedReloadMaps = TRUE;

	return true;
}


void CEditZoneDlg::AddZone(int zoneValue)
{
	zone_info_ptr zoneInfo = m_machine->GetZone(zoneValue);
	if (zoneInfo) {
		SelectItem(zoneValue);
	} else {
		bool bNeedCreateSubMachine = false;
		bool bWireZone = WIRE_ZONE_RANGE_BEG <= zoneValue && zoneValue <= WIRE_ZONE_RANGE_END;
		if (!m_machine->get_is_submachine()) {
			if (zoneValue < MIN_MACHINE_ZONE || zoneValue >= MAX_MACHINE_ZONE) {
				CString e; e = TR(IDS_STRING_E_ZONE_RANGE_FAILE);
				MessageBox(e);
				return;
			}
			if (!bWireZone && NEW_FEATURE_NET_MOD && MT_NETMOD == m_machine->get_machine_type()) {
				CRetrieveProgressDlg retrieveProgressDlg(this);
				retrieveProgressDlg.m_machine = m_machine;
				retrieveProgressDlg.m_zone = zoneValue;
				if (retrieveProgressDlg.DoModal() != IDOK)
					return;
				//int gg = retrieveProgressDlg.m_gg;
				CString alias, fmZone, fmSubMachine;
				fmZone = TR(IDS_STRING_ZONE);
				fmSubMachine = TR(IDS_STRING_SUBMACHINE);
				if (0xCC == retrieveProgressDlg.m_gg) { // not registered
					CString e; e = TR(IDS_STRING_ZONE_NO_DUIMA);
					MessageBox(e, L"", MB_ICONERROR);
					return;
				} else if (0xEE == retrieveProgressDlg.m_gg) { // submachine
					zoneInfo = std::make_shared<zone_info>();
					zoneInfo->set_ademco_id(m_machine->get_ademco_id());
					zoneInfo->set_zone_value(zoneValue);
					zoneInfo->set_type(ZT_SUB_MACHINE);
					zoneInfo->set_status_or_property(retrieveProgressDlg.m_status);
					zoneInfo->set_physical_addr(retrieveProgressDlg.m_addr);
					alias.Format(L"%s%03d", fmSubMachine, zoneValue);
					zoneInfo->set_alias(alias);
					m_type.SetCurSel(ZT_SUB_MACHINE);
					bNeedCreateSubMachine = true;
				} else if (0x00 == retrieveProgressDlg.m_gg) { // direct
					zoneInfo = std::make_shared<zone_info>();
					zoneInfo->set_ademco_id(m_machine->get_ademco_id());
					zoneInfo->set_zone_value(zoneValue);
					zoneInfo->set_type(ZT_ZONE);
					zoneInfo->set_status_or_property(retrieveProgressDlg.m_status);
					zoneInfo->set_physical_addr(retrieveProgressDlg.m_addr);
					alias.Format(L"%s%03d", fmZone, zoneValue);
					zoneInfo->set_alias(alias);
					m_type.SetCurSel(ZT_ZONE);
				} else {
					ASSERT(0);
					return;
				}
			} else {
				zoneInfo = std::make_shared<zone_info>();
				zoneInfo->set_ademco_id(m_machine->get_ademco_id());
				zoneInfo->set_zone_value(zoneValue);
				zoneInfo->set_type(ZT_ZONE);
				m_type.SetCurSel(ZT_ZONE);
			}
		} else { // sub-machine
			if (zoneValue <= 0 || zoneValue >= MAX_SUBMACHINE_ZONE) {
				CString e; e = TR(IDS_STRING_E_SUBZONE_RANGE_FAILE);
				MessageBox(e);
				return;
			}
			zoneInfo = std::make_shared<zone_info>();
			zoneInfo->set_zone_value(m_machine->get_submachine_zone());
			zoneInfo->set_sub_zone(zoneValue);
			zoneInfo->set_type(ZT_SUB_MACHINE_ZONE);
			m_type.SetCurSel(ZT_ZONE);
		}

		if (m_machine->execute_add_zone(zoneInfo)) {
			if (bNeedCreateSubMachine) {
				CreateSubMachine(zoneInfo);
			}
			CString txt;
			FormatZoneInfoText(m_machine, zoneInfo, txt);
			HTREEITEM hItem = m_tree.InsertItem(txt, m_rootItem);
			m_tree.SetItemData(hItem, m_machine->get_is_submachine() ? zoneInfo->get_sub_zone() : zoneInfo->get_zone_value());

			TVSORTCB tvs;
			tvs.hParent = TVI_ROOT;
			tvs.lpfnCompare = MyTreeCompareProc;
			tvs.lParam = reinterpret_cast<LPARAM>(this);
			m_tree.SortChildrenCB(&tvs);
			m_tree.SelectItem(hItem);
			
		} else {
			zoneInfo.reset();
		}
	}
}


void CEditZoneDlg::AddZone(int zoneValue, int gg, int sp, WORD addr)
{
	zone_info_ptr zoneInfo = nullptr;
	CString alias, fmZone, fmSubMachine;
	fmZone = TR(IDS_STRING_ZONE);
	fmSubMachine = TR(IDS_STRING_SUBMACHINE);
	bool bNeedCreateSubMachine = false;
	if (0xCC == gg) { // not registered
		CString e; e = TR(IDS_STRING_ZONE_NO_DUIMA);
		MessageBox(e, L"", MB_ICONERROR);
		return;
	} else if (0xEE == gg) { // submachine
		zoneInfo = std::make_shared<zone_info>();
		zoneInfo->set_ademco_id(m_machine->get_ademco_id());
		zoneInfo->set_zone_value(zoneValue);
		zoneInfo->set_type(ZT_SUB_MACHINE);
		zoneInfo->set_status_or_property(sp);
		zoneInfo->set_physical_addr(addr);
		alias.Format(L"%s%03d", fmSubMachine, zoneValue);
		zoneInfo->set_alias(alias);
		m_type.SetCurSel(ZT_SUB_MACHINE);
		bNeedCreateSubMachine = true;
	} else if (0x00 == gg) { // direct
		zoneInfo = std::make_shared<zone_info>();
		zoneInfo->set_ademco_id(m_machine->get_ademco_id());
		zoneInfo->set_zone_value(zoneValue);
		zoneInfo->set_type(ZT_ZONE);
		zoneInfo->set_status_or_property(sp);
		zoneInfo->set_physical_addr(addr);
		alias.Format(L"%s%03d", fmZone, zoneValue);
		zoneInfo->set_alias(alias);
		m_type.SetCurSel(ZT_ZONE);
	} else {
		ASSERT(0);
		return;
	}

	if (m_machine->execute_add_zone(zoneInfo)) {
		if (bNeedCreateSubMachine) {
			CreateSubMachine(zoneInfo, true);
		}
		CString txt;
		FormatZoneInfoText(m_machine, zoneInfo, txt);
		HTREEITEM hItem = m_tree.InsertItem(txt, m_rootItem);
		m_tree.SetItemData(hItem, zoneInfo->get_zone_value());

		TVSORTCB tvs;
		tvs.hParent = TVI_ROOT;
		tvs.lpfnCompare = MyTreeCompareProc;
		tvs.lParam = reinterpret_cast<LPARAM>(this);
		m_tree.SortChildrenCB(&tvs);
		m_tree.SelectItem(hItem);
		
	} else {
		zoneInfo.reset();
	}
}


void CEditZoneDlg::OnBnClickedButtonAddzone()
{
	AUTO_LOG_FUNCTION;
	int default_zone_value = 0;
	for (int i = MIN_MACHINE_ZONE; i < MAX_MACHINE_ZONE; i++) {
		zone_info_ptr zoneInfo = m_machine->GetZone(i);
		if (!zoneInfo) {
			default_zone_value = i;
			break;
		}
	}
	CAddZoneDlg dlg(this);
	dlg.m_value = default_zone_value;
	if (dlg.DoModal() != IDOK)
		return;

	AddZone(dlg.m_value);
}


void CEditZoneDlg::OnBnClickedButtonDelzone()
{
	AUTO_LOG_FUNCTION;
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (!hItem)
		return;

	DWORD data = m_tree.GetItemData(hItem);
	zone_info_ptr zoneInfo = m_machine->GetZone(data);
	if (!zoneInfo)
		return;

	bool ok = true;
	if (ZT_SUB_MACHINE == zoneInfo->get_type()) { 
		m_machine->dec_submachine_count();
		if (!DeleteSubMachine(zoneInfo)) {
			ok = false;
		}
	}

	if (ok) {
		bool hasDet = (zoneInfo->GetDetectorInfo() != nullptr);
		if (hasDet) {
			CString q; q = TR(IDS_STRING_Q_CONFIRM_DEL_DET);
			int ret = MessageBox(q, nullptr, MB_OKCANCEL | MB_ICONWARNING);
			if (IDOK != ret) {
				JLOG(L"user canceled delete zone\n");
				ok = false;
			}

			if (ok) {
				m_bNeedReloadMaps = TRUE;
			}
		} else {// IDS_STRING_Q_CONFIRM_DEL_DET_UNBIND
			CString q; q = TR(IDS_STRING_Q_CONFIRM_DEL_DET_UNBIND);
			int ret = MessageBox(q, nullptr, MB_OKCANCEL | MB_ICONWARNING);
			if (IDOK != ret) {
				JLOG(L"user canceled delete zone\n");
				ok = false;
			}
		}

		if (ok)
			ok = m_machine->execute_del_zone(zoneInfo);

		
	}

	if (ok) {
		HTREEITEM hNext = m_tree.GetNextSiblingItem(hItem);
		m_tree.DeleteItem(hItem);
		m_tree.SelectItem(hNext ? hNext : m_rootItem);
		//m_tree.SelectItem(m_rootItem);
	}
}


void CEditZoneDlg::OnCbnSelchangeComboZoneType()
{
	AUTO_LOG_FUNCTION;
#pragma region test integrity
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (!hItem)
		return;

	DWORD data = m_tree.GetItemData(hItem);
	zone_info_ptr zoneInfo = m_machine->GetZone(data);
	if (!zoneInfo)
		return;

	int ndx = m_type.GetCurSel();
	if (ndx < 0)
		return;

	if (ndx == zoneInfo->get_type())
		return;
#pragma endregion

	bool ok = true;
	do {
		if (ndx == ZT_ZONE) { 
			if (!DeleteSubMachine(zoneInfo)) {
				JLOG(L"ChangeDetectorImage failed.\n"); ok = false; break;
			}
			m_machine->dec_submachine_count();
		} else if (ndx == ZT_SUB_MACHINE) { 
			CString null;
			null = TR(IDS_STRING_NULL);
			alarm_machine_ptr subMachine = std::make_shared<alarm_machine>();
			subMachine->set_is_submachine(true);
			subMachine->set_ademco_id(zoneInfo->get_ademco_id());
			subMachine->set_submachine_zone(zoneInfo->get_zone_value());
			subMachine->set_alias(zoneInfo->get_alias());
			subMachine->set_address(null);
			subMachine->set_contact(null);
			subMachine->set_phone(null);
			subMachine->set_phone_bk(null);
			subMachine->set_machine_type(m_machine->get_machine_type());
			auto expire_time = std::chrono::system_clock::now();
			// add a year
			{
				expire_time += std::chrono::hours(24) * 365;
			}

			subMachine->set_expire_time(expire_time);
			subMachine->set_coor(web::BaiduCoordinate(0, 0));
			if (!zoneInfo->execute_set_sub_machine(subMachine)) {
				ASSERT(0); JLOG(L"execute_set_sub_machine failed.\n"); ok = false; break;
			}
			m_machine->inc_submachine_count();

			map_info_ptr mapInfo = zoneInfo->GetMapInfo();
			detector_info_ptr detInfo = zoneInfo->GetDetectorInfo();
			if (mapInfo && detInfo) {
				mapInfo->SetActiveInterfaceInfo(zoneInfo);
				mapInfo->InversionControl(ICMC_DEL_DETECTOR);
				mapInfo->RemoveInterface(zoneInfo);
			}

			if (detInfo)
				zoneInfo->execute_del_detector_info();
			m_bNeedReloadMaps = TRUE;
		}
	} while (0);

	if (!ok) {
		m_type.SetCurSel(zoneInfo->get_type());
	} else {
		CString txt;
		FormatZoneInfoText(m_machine, zoneInfo, txt);
		m_tree.SetItemText(hItem, txt);
		m_tree.SelectItem(m_rootItem);
		m_tree.SelectItem(hItem);
		
	}
}


bool CEditZoneDlg::ChangeDetectorImage(const core::zone_info_ptr& zoneInfo, int newType)
{
	AUTO_LOG_FUNCTION;
	detector_info_ptr detInfo = zoneInfo->GetDetectorInfo();
	if (!detInfo) {
		JLOG(L"this zone has no detector.\n");
		return true;
	}

	if (newType >= DT_MAX) {
		JLOG(L"invalid newType %d.\n", newType);
		return false;
	}

	auto lib = detector_lib_manager::get_instance();
	const detector_lib_data_ptr libData = lib->GetDetectorLibData(detInfo->get_detector_lib_id());
	if (libData->get_type() & newType) {
		JLOG(L"newType is the same as old type.\n");
		return true;
	}

	CString q;
	q = TR((ZT_SUB_MACHINE == newType) ? IDS_STRING_Q_CHANGE_DET : IDS_STRING_Q_CHANGE_SUBMACHINE);
	int ret = MessageBox(q, nullptr, MB_OKCANCEL | MB_ICONQUESTION);
	if (ret != IDOK) {
		JLOG(L"user canceled change det type from sensor to submachine\n");
		return true;
	}

	CChooseDetDlg dlg(this);
	dlg.m_detType2Show = newType;
	if (IDOK != dlg.DoModal()) {
		JLOG(L"user canceled choose det type\n");
		return true;
	}

	CString query;
	query.Format(L"update table_detector set detector_lib_id=%d where id=%d",
				 dlg.m_chosenDetectorID, detInfo->get_id());
	auto mgr = alarm_machine_manager::get_instance();
	if (mgr->ExecuteSql(query))
		detInfo->set_detector_lib_id(dlg.m_chosenDetectorID);
	else {
		JLOG(L"update table_detector failed: %s\n", query);
		ASSERT(0); return false;
	}

	m_bNeedReloadMaps = TRUE;
	return true;
}


bool CEditZoneDlg::DeleteSubMachine(const core::zone_info_ptr& zoneInfo)
{
	AUTO_LOG_FUNCTION;
	alarm_machine_ptr subMachine = zoneInfo->GetSubMachineInfo();
	if (subMachine) {
		CString q; q = TR(IDS_STRING_Q_CONFIRM_DEL_SUBMACHINE);
		int ret = MessageBox(q, nullptr, MB_OKCANCEL | MB_ICONWARNING);
		if (IDOK != ret) {
			JLOG(L"user canceled change submachine to zone\n");
			return false;
		}

		if (!zoneInfo->execute_del_sub_machine()) {
			JLOG(L"delete submachine failed\n");
			ASSERT(0); return false;
		}
	}

	m_bNeedReloadMaps = TRUE;
	return true;
}


void CEditZoneDlg::OnEnChangeEditAlias()
{
	//auto_timer timer(m_hWnd, 1, 1000);
}


void CEditZoneDlg::OnEnChangeEditContact()
{
	//auto_timer timer(m_hWnd, 1, 1000);
}


void CEditZoneDlg::OnEnChangeEditAddress()
{
	//auto_timer timer(m_hWnd, 1, 1000);	
}


void CEditZoneDlg::OnEnChangeEditPhone()
{
	//auto_timer timer(m_hWnd, 1, 1000);
}


void CEditZoneDlg::OnEnChangeEditPhoneBk()
{
	//auto_timer timer(m_hWnd, 1, 1000);
}


void CEditZoneDlg::OnBnClickedButtonEditDetector()
{
	AUTO_LOG_FUNCTION;
}


void CEditZoneDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	db_updated_observer_->dlg = nullptr;
	db_updated_observer_ = nullptr;
}


void CEditZoneDlg::OnBnClickedButtonManageSubmachineExpireTime()
{
	AUTO_LOG_FUNCTION;
	CMachineExpireManagerDlg dlg(this);
	//dlg.m_machine = m_machine;
	zone_info_list list;
	m_machine->GetAllZoneInfo(list);
	std::list<alarm_machine_ptr> machineList;
	for (auto zoneInfo : list) {
		alarm_machine_ptr subMachine = zoneInfo->GetSubMachineInfo();
		if (subMachine) {
			machineList.push_back(subMachine);
		}
	}
	dlg.m_bSubMachine = true;
	dlg.m_machine = m_machine;
	dlg.SetExpiredMachineList(machineList);
	dlg.DoModal();
}


void CEditZoneDlg::OnBnClickedButtonAutoRetrieve()
{
	CAutoRetrieveZoneInfoDlg dlg(this);
	dlg.m_machine = m_machine;
	dlg.DoModal();
	Init();
	m_bNeedReloadMaps = TRUE;
}


void CEditZoneDlg::OnBnClickedCheck1()
{
	AUTO_LOG_FUNCTION;
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (!hItem)
		return;

	DWORD data = m_tree.GetItemData(hItem);
	alarm_machine_ptr machine = nullptr;

	if (data == ZONE_VALUE_FOR_MACHINE_SELF) {
		machine = m_machine;
	} else {
		zone_info_ptr zoneInfo = m_machine->GetZone(data);
		if (!zoneInfo)
			return;

		machine = zoneInfo->GetSubMachineInfo();
	}

	if (!machine) return;

	BOOL b = m_chk_report_status.GetCheck();
	sms_config cfg = machine->get_sms_cfg();
	cfg.report_status = b ? true : false;
	machine->execute_set_sms_cfg(cfg);
}


void CEditZoneDlg::OnBnClickedCheck2()
{
	AUTO_LOG_FUNCTION;
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (!hItem)
		return;

	DWORD data = m_tree.GetItemData(hItem);
	alarm_machine_ptr machine = nullptr;

	if (data == ZONE_VALUE_FOR_MACHINE_SELF) {
		machine = m_machine;
	} else {
		zone_info_ptr zoneInfo = m_machine->GetZone(data);
		if (!zoneInfo)
			return;

		machine = zoneInfo->GetSubMachineInfo();
	}

	if (!machine) return;

	BOOL b = m_chk_report_exception.GetCheck();
	sms_config cfg = machine->get_sms_cfg();
	cfg.report_exception = b ? true : false;
	machine->execute_set_sms_cfg(cfg);
}


void CEditZoneDlg::OnBnClickedCheck3()
{
	AUTO_LOG_FUNCTION;
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (!hItem)
		return;

	DWORD data = m_tree.GetItemData(hItem);
	alarm_machine_ptr machine = nullptr;

	if (data == ZONE_VALUE_FOR_MACHINE_SELF) {
		machine = m_machine;
	} else {
		zone_info_ptr zoneInfo = m_machine->GetZone(data);
		if (!zoneInfo)
			return;

		machine = zoneInfo->GetSubMachineInfo();
	}

	if (!machine) return;

	BOOL b = m_chk_report_alarm.GetCheck();
	sms_config cfg = machine->get_sms_cfg();
	cfg.report_alarm = b ? true : false;
	machine->execute_set_sms_cfg(cfg);
}


void CEditZoneDlg::OnBnClickedCheck4()
{
	AUTO_LOG_FUNCTION;
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (!hItem)
		return;

	DWORD data = m_tree.GetItemData(hItem);
	alarm_machine_ptr machine = nullptr;

	if (data == ZONE_VALUE_FOR_MACHINE_SELF) {
		machine = m_machine;
	} else {
		zone_info_ptr zoneInfo = m_machine->GetZone(data);
		if (!zoneInfo)
			return;

		machine = zoneInfo->GetSubMachineInfo();
	}

	if (!machine) return;

	BOOL b = m_chk_report_status_bk.GetCheck();
	sms_config cfg = machine->get_sms_cfg();
	cfg.report_status_bk = b ? true : false;
	machine->execute_set_sms_cfg(cfg);
}


void CEditZoneDlg::OnBnClickedCheck5()
{
	AUTO_LOG_FUNCTION;
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (!hItem)
		return;

	DWORD data = m_tree.GetItemData(hItem);
	alarm_machine_ptr machine = nullptr;

	if (data == ZONE_VALUE_FOR_MACHINE_SELF) {
		machine = m_machine;
	} else {
		zone_info_ptr zoneInfo = m_machine->GetZone(data);
		if (!zoneInfo)
			return;

		machine = zoneInfo->GetSubMachineInfo();
	}

	if (!machine) return;

	BOOL b = m_chk_report_exception_bk.GetCheck();
	sms_config cfg = machine->get_sms_cfg();
	cfg.report_exception_bk = b ? true : false;
	machine->execute_set_sms_cfg(cfg);
}


void CEditZoneDlg::OnBnClickedCheck6()
{
	AUTO_LOG_FUNCTION;
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (!hItem)
		return;

	DWORD data = m_tree.GetItemData(hItem);
	alarm_machine_ptr machine = nullptr;

	if (data == ZONE_VALUE_FOR_MACHINE_SELF) {
		machine = m_machine;
	} else {
		zone_info_ptr zoneInfo = m_machine->GetZone(data);
		if (!zoneInfo)
			return;

		machine = zoneInfo->GetSubMachineInfo();
	}

	if (!machine) return;

	BOOL b = m_chk_report_alarm_bk.GetCheck();
	sms_config cfg = machine->get_sms_cfg();
	cfg.report_alarm_bk = b ? true : false;
	machine->execute_set_sms_cfg(cfg);
}


void CEditZoneDlg::OnBnClickedButtonManullyAddZoneWriteToMachine()
{
	CMannualyAddZoneWrite2MachineDlg dlg(this);
	dlg.m_machine = m_machine;
	if (IDOK == dlg.DoModal()) {
		AddZone(dlg.m_zone, dlg.m_gg, dlg.m_zs, dlg.m_waddr);
	}
}


void CEditZoneDlg::OnBnClickedButtonBindOrUnbindVideoDevice()
{
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (!hItem)
		return;

	DWORD data = m_tree.GetItemData(hItem);
	if (data == ZONE_VALUE_FOR_MACHINE_SELF) {
		video::zone_uuid zoneUuid;
		zoneUuid._ademco_id = m_machine->get_ademco_id();
		if (m_machine->get_is_submachine()) {
			zoneUuid._zone_value = m_machine->get_submachine_zone();
			zoneUuid._gg = ZONE_VALUE_FOR_MACHINE_SELF;
		} else {
			zoneUuid._zone_value = ZONE_VALUE_FOR_MACHINE_SELF;
			zoneUuid._gg = 0;
		}

		auto mgr = video::video_manager::get_instance();
		auto bi = video::video_manager::get_instance()->GetBindInfo(zoneUuid);
		if (bi._device) {
			if (mgr->UnbindZoneAndDevice(zoneUuid)) {
				CString txt; txt = TR(IDS_STRING_BIND_VIDEO_DEVICE);
				m_btnBindOrUnbindVideoDevice.SetWindowTextW(txt);
				m_chkAutoPlayVideoOnAlarm.SetCheck(0);
				m_btnPreview.EnableWindow(0);
				m_editDevInfo.SetWindowTextW(L"");

				CString rec;
				rec.Format(L"%s %s %s", m_machine->get_formatted_name(), TR(IDS_STRING_UNBINDED_VIDEO), bi._device->get_formatted_name().c_str());
				core::history_record_manager::get_instance()->InsertRecord(m_machine->get_ademco_id(), zoneUuid._zone_value, rec, time(nullptr), core::RECORD_LEVEL_USEREDIT);
			}
		} else {
			CChooseVideoDeviceDlg dlg(this);
			if (IDOK != dlg.DoModal())
				return;

			auto device = dlg.m_dev; assert(dlg.m_dev);
			if (mgr->BindZoneAndDevice(zoneUuid, device)) {
				CString txt; txt = TR(IDS_STRING_IDC_BUTTON_UNBIND);
				m_btnBindOrUnbindVideoDevice.SetWindowTextW(txt);
				m_chkAutoPlayVideoOnAlarm.SetCheck(1);
				m_btnPreview.EnableWindow(1);
				txt.Format(L"%s[%d,%s]", device->get_userInfo()->get_user_name().c_str(), device->get_id(), device->get_device_note().c_str());
				m_editDevInfo.SetWindowTextW(txt);

				CString rec;
				rec.Format(L"%s %s %s", m_machine->get_formatted_name(), TR(IDS_STRING_BINDED_VIDEO), device->get_formatted_name().c_str());
				core::history_record_manager::get_instance()->InsertRecord(m_machine->get_ademco_id(), zoneUuid._zone_value, rec, time(nullptr), core::RECORD_LEVEL_USEREDIT);
			}
		}

	} else {
		zone_info_ptr zoneInfo = m_machine->GetZone(data);
		if (!zoneInfo)
			return;

		bool bsub = zoneInfo->get_type() == ZT_SUB_MACHINE;

		video::zone_uuid zoneUuid(m_machine->get_ademco_id(), zoneInfo->get_zone_value(), 0);
		if (m_machine->get_is_submachine()) {
			zoneUuid._gg = zoneInfo->get_sub_zone();
		} else if (bsub) {
			zoneUuid._gg = ZONE_VALUE_FOR_MACHINE_SELF;
		}

		auto mgr = video::video_manager::get_instance();
		auto bi = mgr->GetBindInfo(zoneUuid);
		if (bi._device) {
			if (mgr->UnbindZoneAndDevice(zoneUuid)) {

				CString txt; txt = TR(IDS_STRING_BIND_VIDEO_DEVICE);
				m_btnBindOrUnbindVideoDevice.SetWindowTextW(txt);
				m_chkAutoPlayVideoOnAlarm.SetCheck(0);
				m_btnPreview.EnableWindow(0);
				m_editDevInfo.SetWindowTextW(L"");

				CString rec;
				rec.Format(L"%s %s %s", zoneInfo->get_formatted_zone_id(), TR(IDS_STRING_UNBINDED_VIDEO), bi._device->get_formatted_name().c_str());
				core::history_record_manager::get_instance()->InsertRecord(m_machine->get_ademco_id(), zoneInfo->get_zone_value(), rec, time(nullptr), core::RECORD_LEVEL_USEREDIT);
			}
		} else {
			CChooseVideoDeviceDlg dlg(this);
			if (IDOK != dlg.DoModal())
				return;

			auto device = dlg.m_dev; assert(dlg.m_dev);
			if (mgr->BindZoneAndDevice(zoneUuid, device)) {
				CString txt; txt = TR(IDS_STRING_IDC_BUTTON_UNBIND);
				m_btnBindOrUnbindVideoDevice.SetWindowTextW(txt);
				m_chkAutoPlayVideoOnAlarm.SetCheck(1);
				m_btnPreview.EnableWindow(1);
				txt.Format(L"%s[%d,%s]", device->get_userInfo()->get_user_name().c_str(), device->get_id(), device->get_device_note().c_str());
				m_editDevInfo.SetWindowTextW(txt);

				CString rec;
				rec.Format(L"%s %s %s", zoneInfo->get_formatted_zone_id(), TR(IDS_STRING_BINDED_VIDEO), device->get_formatted_name().c_str());
				core::history_record_manager::get_instance()->InsertRecord(m_machine->get_ademco_id(), zoneInfo->get_zone_value(), rec, time(nullptr), core::RECORD_LEVEL_USEREDIT);
			}
		}
	}
}


void CEditZoneDlg::OnBnClickedCheckAutoPlayVideoOnAlarm()
{
	AUTO_LOG_FUNCTION;
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (!hItem)
		return;

	DWORD data = m_tree.GetItemData(hItem);

	video::zone_uuid zoneUuid;
	zoneUuid._ademco_id = m_machine->get_ademco_id();

	if (data == ZONE_VALUE_FOR_MACHINE_SELF) {
		if (m_machine->get_is_submachine()) {
			zoneUuid._zone_value = m_machine->get_submachine_zone();
			zoneUuid._gg = ZONE_VALUE_FOR_MACHINE_SELF;
		} else {
			zoneUuid._zone_value = ZONE_VALUE_FOR_MACHINE_SELF;
			zoneUuid._gg = 0;
		}
	} else {
		zone_info_ptr zoneInfo = m_machine->GetZone(data);
		if (!zoneInfo)
			return;

		zoneUuid._zone_value = zoneInfo->get_zone_value();
		if (m_machine->get_is_submachine()) {
			zoneUuid._gg = zoneInfo->get_sub_zone();
		} else if (zoneInfo->get_type() == ZT_SUB_MACHINE) {
			zoneUuid._gg = ZONE_VALUE_FOR_MACHINE_SELF;
		}
	}

	auto mgr = video::video_manager::get_instance();
	video::bind_info bi = mgr->GetBindInfo(zoneUuid);
	if (bi._device) {
		BOOL bCheck = m_chkAutoPlayVideoOnAlarm.GetCheck();
		if (bCheck != bi.auto_play_when_alarm_) {
			if (!mgr->SetBindInfoAutoPlayVideoOnAlarm(zoneUuid, bCheck ? true : false)) {
				m_chkAutoPlayVideoOnAlarm.SetCheck(!bCheck);
			}
		}
	}
}


afx_msg LRESULT CEditZoneDlg::OnVideoInfoChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	OnTvnSelchangedTreeZone(nullptr, nullptr);
	return 0;
}


void CEditZoneDlg::OnBnClickedButtonPreview()
{
	AUTO_LOG_FUNCTION;
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (!hItem)
		return;

	DWORD data = m_tree.GetItemData(hItem);

	video::zone_uuid zoneUuid;
	zoneUuid._ademco_id = m_machine->get_ademco_id();

	if (data == ZONE_VALUE_FOR_MACHINE_SELF) {
		if (m_machine->get_is_submachine()) {
			zoneUuid._zone_value = m_machine->get_submachine_zone();
			zoneUuid._gg = ZONE_VALUE_FOR_MACHINE_SELF;
		} else {
			zoneUuid._zone_value = ZONE_VALUE_FOR_MACHINE_SELF;
			zoneUuid._gg = 0;
		}
	} else {
		zone_info_ptr zoneInfo = m_machine->GetZone(data);
		if (!zoneInfo || zoneInfo->get_type() == ZT_SUB_MACHINE)
			return;

		zoneUuid._zone_value = zoneInfo->get_zone_value();
		if (m_machine->get_is_submachine()) {
			zoneUuid._gg = zoneInfo->get_sub_zone();
		} else {
			zoneUuid._gg = 0;
		}
	}

	auto mgr = video::video_manager::get_instance();
	video::bind_info bi = mgr->GetBindInfo(zoneUuid);
	if (bi._device) {
		ipc::alarm_center_video_service::get_instance()->play_video(bi._device);
	}
}


void CEditZoneDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1) { // en changed

		//auto_timer timer(m_hWnd, 1, 1000);
		KillTimer(1);

		
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CEditZoneDlg::OnBnClickedButtonSave()
{
	do {
		HTREEITEM hItem = m_tree.GetSelectedItem();
		if (!hItem)
			break;

		CString alias;
		m_alias.GetWindowTextW(alias);

		CString contact;
		m_contact.GetWindowTextW(contact);

		CString address;
		m_addr.GetWindowTextW(address);

		CString phone;
		m_phone.GetWindowTextW(phone);

		CString phone_bk;
		m_phone_bk.GetWindowTextW(phone_bk);

		DWORD data = m_tree.GetItemData(hItem);
		if (data == ZONE_VALUE_FOR_MACHINE_SELF) {

			if (m_machine->get_is_submachine()) {
				auto mgr = core::alarm_machine_manager::get_instance();
				auto parent = mgr->GetMachine(m_machine->get_ademco_id());
				auto zoneInfo = parent->GetZone(m_machine->get_submachine_zone());
				if (zoneInfo) {
					zoneInfo->execute_update_alias(alias);
					zoneInfo->execute_update_contact(contact);
					zoneInfo->execute_update_address(address);
					zoneInfo->execute_update_phone(phone);
					zoneInfo->execute_update_phone_bk(phone_bk);
				}
			} else {
				m_machine->execute_set_alias(alias);
				m_machine->execute_set_contact(contact);
				m_machine->execute_set_address(address);
				m_machine->execute_set_phone(phone);
				m_machine->execute_set_phone_bk(phone_bk);
			}


		} else {
			zone_info_ptr zoneInfo = m_machine->GetZone(data);
			if (!zoneInfo)
				break;

			if (zoneInfo->execute_update_alias(alias)) {
				CString txt;
				FormatZoneInfoText(m_machine, zoneInfo, txt);
				m_tree.SetItemText(hItem, txt);
			}
			zoneInfo->execute_update_contact(contact);
			zoneInfo->execute_update_address(address);
			zoneInfo->execute_update_phone(phone);
			zoneInfo->execute_update_phone_bk(phone_bk);
		}
	} while (0);
}
