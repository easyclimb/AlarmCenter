﻿// AutoRetrieveZoneInfoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AutoRetrieveZoneInfoDlg.h"
#include "afxdialogex.h"
#include "RetrieveProgressDlg.h"
#include "AlarmMachine.h"
#include "ZoneInfo.h"
#include "AlarmMachineManager.h"
#include "NetworkConnector.h"

using namespace core;


namespace detail {

auto translate_serial_property_to_zone_property = [](int zone_property) {
	switch (zone_property) {
		case 0x00: // buglar zone
			return ZP_GLOBAL;
			break;

		case 0x01: // emergency zone
			return ZP_EMERGE;
			break;

		case 0x02: // fire zone
			return ZP_FIRE;
			break;

		case 0x03: // duress zone
			return ZP_DURESS;
			break;

		case 0x04: // gas
			return ZP_GAS;
			break;

		case 0x05: // water
			return ZP_WATER;
			break;

		case 0x06: // submachine, ignore it
			break;

		case 0x07: // remote controller, ignore it
			break;

		case 0x08:
			return ZP_HALF;
			break;

		case 0x09:
			return ZP_SHIELD;
			break;

		case 0x0A:
			return ZP_DOOR;
			break;

		default:
			break;
	}
	return ZSOP_INVALID;
};


}

using namespace ::detail;

// CAutoRetrieveZoneInfoDlg 对话框

IMPLEMENT_DYNAMIC(CAutoRetrieveZoneInfoDlg, CDialogEx)

CAutoRetrieveZoneInfoDlg::CAutoRetrieveZoneInfoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CAutoRetrieveZoneInfoDlg::IDD, pParent)
	, m_machine(nullptr)
	, m_dwStartTime(0)
{

}

CAutoRetrieveZoneInfoDlg::~CAutoRetrieveZoneInfoDlg()
{
}

void CAutoRetrieveZoneInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_START, m_btnStart);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
	DDX_Control(pDX, IDC_STATIC_PROGRESS, m_staticProgress);
	DDX_Control(pDX, IDC_STATIC_TIME, m_staticTime);
	DDX_Control(pDX, IDC_LIST1, m_listctrl);
}


BEGIN_MESSAGE_MAP(CAutoRetrieveZoneInfoDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAutoRetrieveZoneInfoDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_START, &CAutoRetrieveZoneInfoDlg::OnBnClickedButtonStart)
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CAutoRetrieveZoneInfoDlg 消息处理程序


void CAutoRetrieveZoneInfoDlg::OnBnClickedOk()
{
	return;
}


BOOL CAutoRetrieveZoneInfoDlg::OnInitDialog() 
{
	CDialogEx::OnInitDialog();

	SetWindowText(TR(IDS_STRING_IDD_DIALOG_AUTO_RETRIEVE_ZONE_INFO));
	SET_WINDOW_TEXT(IDC_BUTTON_START, IDS_STRING_IDC_BUTTON_START);


	assert(m_machine);

	int max_zone = get_machine_max_zone_by_type(m_machine->get_machine_type()) + 1;
	CString txt;
	txt.Format(L"0/%d", max_zone);
	m_staticProgress.SetWindowTextW(txt);
	m_progress.SetRange32(0, max_zone);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}


void CAutoRetrieveZoneInfoDlg::OnBnClickedButtonStart() 
{
	AUTO_LOG_FUNCTION;
	if (m_bRetrieving) {
		Reset();
		LeaveSetMode();
	} else {
		
		m_dwStartTime = GetTickCount();
		SetTimer(1, 100, nullptr);

		CString msg = L"", str = L"", fmok, fmfail, progress;
		fmok = TR(IDS_STRING_FM_RETRIEVE_OK);
		fmfail = TR(IDS_STRING_FM_RETRIEVE_FAILED);
		
		int max_machine_zone = get_machine_max_zone_by_type(m_machine->get_machine_type()) + 1;
		

		if (is_machine_can_only_add_zone_by_retrieve(m_machine->get_machine_type())) {
			m_progress.SetPos(1);
			CString txt;
			txt.Format(L"1/%d", max_machine_zone);
			m_staticProgress.SetWindowTextW(txt);
			m_btnStart.SetWindowTextW(TR(IDS_STRING_IDC_BUTTON_STOP));
			m_bRetrieving = TRUE;

			if (!m_machine->get_online()) {
				m_listctrl.SetCurSel(m_listctrl.InsertString(-1, TR(IDS_STRING_STOP_RTRV_BY_OFFLINE)));
				Reset();
				return;
			}

			m_observer = std::make_shared<ObserverType>(this);
			m_machine->register_observer(m_observer);
			m_event_list.clear();
			m_zone_list.clear();

			// send enter set mode
			net::CNetworkConnector::get_instance()->Send(m_machine->get_ademco_id(), EVENT_ENTER_SET_MODE,
														0, 0, nullptr, nullptr, m_machine->get_last_time_event_source());

		} else {
			max_machine_zone = MAX_MACHINE_ZONE;
			std::wstring max_progress = L"";
			max_progress = L"1000";
			for (int i = 1; i < max_machine_zone; i++) {
				msg.Empty();
				if (RetrieveZoneInfo(i, msg)) {
					m_progress.SetPos(i);
					progress.Format(L"%d/%s", i, max_progress.c_str());
					m_staticProgress.SetWindowTextW(progress);
					str.Format(fmok, i);
					int ndx = m_listctrl.InsertString(-1, str + L": " + msg);
					m_listctrl.SetCurSel(ndx);
				} else {
					str.Format(fmfail, i, msg);
					int ndx = m_listctrl.InsertString(-1, str);
					m_listctrl.SetCurSel(ndx);
					break;
				}
			}

			KillTimer(1);
			m_progress.SetPos(0);
			msg.Format(L"0/%s", max_progress.c_str());
			m_staticProgress.SetWindowTextW(msg);
			m_staticTime.SetWindowTextW(L"00:00");
		}
	}
	
}


void CAutoRetrieveZoneInfoDlg::OnAdemcoEventResult(const ademco::AdemcoEventPtr& ademcoEvent)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_event_list.push_back(ademcoEvent);
}


// return true for continue, false for stop
bool CAutoRetrieveZoneInfoDlg::RetrieveZoneInfo(int zoneValue, CString& msg)
{
	do {
		zone_info_ptr zoneInfo = m_machine->GetZone(zoneValue);
		if (zoneInfo) {
			CString fm; fm = TR(IDS_STRING_FM_ZONE_ALREADY_EXSISTS);
			msg.Format(fm, zoneInfo->get_alias());
			return true;
		}
		bool bNeedCreateSubMachine = false;
		bool bWireZone = WIRE_ZONE_RANGE_BEG <= zoneValue && zoneValue <= WIRE_ZONE_RANGE_END;
		if (!m_machine->get_is_submachine()) {
			if (zoneValue <= 0 || zoneValue >= MAX_MACHINE_ZONE) {
				msg = TR(IDS_STRING_E_ZONE_RANGE_FAILE);
				MessageBox(msg);
				break;
			}
			if (!bWireZone && MT_NETMOD == m_machine->get_machine_type()) {
				CRetrieveProgressDlg retrieveProgressDlg(this);
				retrieveProgressDlg.m_machine = m_machine;
				retrieveProgressDlg.m_zone = zoneValue;
				if (retrieveProgressDlg.DoModal() != IDOK) {
					msg = TR(IDS_STRING_USER_STOP_RESTRIEVE);
					break;
				}
				//int gg = retrieveProgressDlg.m_gg;
				CString alias, fmZone, fmSubMachine;
				fmZone = TR(IDS_STRING_ZONE);
				fmSubMachine = TR(IDS_STRING_SUBMACHINE);
				if (0xCC == retrieveProgressDlg.m_gg) { // not registered
					msg = TR(IDS_STRING_ZONE_NO_DUIMA);
					return true;
				} else if (0xEE == retrieveProgressDlg.m_gg) { // submachine
					zoneInfo = std::make_shared<zone_info>();
					zoneInfo->set_ademco_id(m_machine->get_ademco_id());
					zoneInfo->set_zone_value(zoneValue);
					zoneInfo->set_type(ZT_SUB_MACHINE);
					zoneInfo->set_status_or_property(retrieveProgressDlg.m_status);
					zoneInfo->set_physical_addr(retrieveProgressDlg.m_addr);
					alias.Format(L"%s%03d", fmSubMachine, zoneValue);
					zoneInfo->set_alias(alias);
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
				} else {
					ASSERT(0);
					break;
				}
			} else {
				zoneInfo = std::make_shared<zone_info>();
				zoneInfo->set_ademco_id(m_machine->get_ademco_id());
				zoneInfo->set_zone_value(zoneValue);
				zoneInfo->set_type(ZT_ZONE);
			}
		} 

		if (m_machine->execute_add_zone(zoneInfo)) {
			if (bNeedCreateSubMachine) {
				CString null;
				null = TR(IDS_STRING_NULL);
				alarm_machine_ptr subMachine = std::make_shared<alarm_machine>();
				subMachine->set_is_submachine(true);
				subMachine->set_ademco_id(m_machine->get_ademco_id());
				subMachine->set_submachine_zone(zoneValue);
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
					ASSERT(0); JLOG(L"execute_set_sub_machine failed.\n"); break;
				}
				//m_machine->inc_submachine_count();
				char status = zoneInfo->get_status_or_property() & 0xFF;
				ADEMCO_EVENT ademco_event = static_cast<ADEMCO_EVENT>(zone_info::char_to_status(status));
				auto t = time(nullptr);
				m_machine->SetAdemcoEvent(ES_UNKNOWN, ademco_event, zoneValue, 0xEE, t, t);
			}
		} else {
			ASSERT(0); JLOG(L"m_machine->execute_add_zone(zoneInfo) failed.\n"); zoneInfo.reset(); break;
		}

		if (msg.IsEmpty()) {
			//CString szone, ssubmachine; szone = TR(IDS_STRING_ZONE);
			//ssubmachine = TR(IDS_STRING_SUBMACHINE);
			msg = zoneInfo->get_alias();
		}
		return true;
	} while (0);
	if (msg.IsEmpty())
		msg = TR(IDS_STRING_QUERY_FAILED);
	return false;
}


void CAutoRetrieveZoneInfoDlg::OnTimer(UINT_PTR nIDEvent)
{
	//AUTO_LOG_FUNCTION;
	DWORD cnt = GetTickCount() - m_dwStartTime;
	if (cnt >= 1000) {
		cnt /= 1000;
		int min = cnt / 60;
		int sec = cnt % 60;
		CString t; t.Format(L"%02d:%02d", min, sec);
		m_staticTime.SetWindowTextW(t);
	}


	if(m_mutex.try_lock()) {
		std::lock_guard<std::mutex> lock(m_mutex, std::adopt_lock);
		CString txt = L"";
		for (auto ademcoEvent : m_event_list) {
			switch (ademcoEvent->_event) {
			case EVENT_RETRIEVE_ZONE_OR_SUB_MACHINE:
			{
				size_t len = ademcoEvent->_xdata->at(3);
				assert(len == ademcoEvent->_xdata->size());
				if (len == ademcoEvent->_xdata->size()) {
					if (len > 8) {
						size_t count = (len - 8) / 2;
						for (size_t i = 0; i < count; i++) {
							int zone = ademcoEvent->_xdata->at(6 + i * 2); assert(1 <= zone && zone <= 99);
							int zone_property = ademcoEvent->_xdata->at(7 + i * 2);
							auto zp = translate_serial_property_to_zone_property(zone_property);
							if (zp != ZSOP_INVALID) {
								auto zoneInfo = m_machine->GetZone(zone);
								if (zoneInfo) {
									zoneInfo->execute_set_status_or_property(zp & 0xFF);
								} else {
									zoneInfo = std::make_shared<zone_info>();
									zoneInfo->set_ademco_id(m_machine->get_ademco_id());
									zoneInfo->set_zone_value(zone);
									zoneInfo->set_type(ZT_ZONE);
									zoneInfo->set_status_or_property(zp);
									txt.Format(L"%s%02d", TR(IDS_STRING_ZONE), zone);
									zoneInfo->set_alias(txt);
								}
								m_zone_list.push_back(zoneInfo);
								m_progress.SetPos(zone);
								txt.Format(L"%02d/", zone);
								txt.AppendFormat(get_format_string_of_machine_zone_count_figure_by_type(m_machine->get_machine_type()), 
												 get_machine_max_zone_by_type(m_machine->get_machine_type()));
								m_staticProgress.SetWindowTextW(txt);
								txt.Format(L"%s%02d", TR(IDS_STRING_ZONE), zone);
								m_listctrl.SetCurSel(m_listctrl.InsertString(-1, txt));
							}
						}
					}

					unsigned char status = ademcoEvent->_xdata->at(len - 2);
					if (status == 0xFF) { // over
						// check local zone, they are old, maybe exist, maybe not.
						zone_info_list local_list;
						m_machine->GetAllZoneInfo(local_list);
						auto local_iter = local_list.begin();
						while (local_iter != local_list.end()) {
							auto zoneInfo = *local_iter;
							bool found = false;
							for (auto remote_iter = m_zone_list.begin(); remote_iter != m_zone_list.end(); remote_iter++) {
								if (zoneInfo->get_zone_value() == (*remote_iter)->get_zone_value()) {
									// found, update local, delete it from remote list
									zoneInfo->execute_set_status_or_property((*remote_iter)->get_status_or_property() & 0xFF);
									found = true;
									m_zone_list.erase(remote_iter);
									break;
								} 
							}

							if (!found) {
								// not found, delete local zone 
								m_machine->execute_del_zone(zoneInfo);
								local_iter = local_list.erase(local_iter);
							} else {
								local_iter++;
							}
							
						}

						// resolv not deleted remote zone, they are new zone, add them to machine
						for (auto remote_zone : m_zone_list) {
							m_machine->execute_add_zone(remote_zone);
						}

						if (m_machine->get_zone_count() == 0) {
							m_listctrl.SetCurSel(m_listctrl.InsertString(-1, TR(IDS_STRING_NO_DUIMA_ZONE)));
						}
						m_listctrl.SetCurSel(m_listctrl.InsertString(-1, TR(IDS_STRING_RETRIEVE_OVER)));
						Reset();
						LeaveSetMode();
						return;
						
					} else { // continue, send A2
						unsigned char raw[5] = { 0xEB, 0xAB, 0x3F, 0xA2, 0x77 };
						auto cmd = std::make_shared<char_array>();
						std::copy(raw, raw + 5, std::back_inserter(*cmd));
						auto mgr = core::alarm_machine_manager::get_instance();
						auto path = m_machine->get_last_time_event_source();
						switch (path)
						{
						case ademco::ES_TCP_CLIENT:
							mgr->RemoteControlAlarmMachine(m_machine, EVENT_RETRIEVE_ZONE_OR_SUB_MACHINE, 0, 0, cmd, nullptr, path, this);
							break;
						case ademco::ES_TCP_SERVER1:
						case ademco::ES_TCP_SERVER2:
							mgr->RemoteControlAlarmMachine(m_machine, EVENT_RETRIEVE_ZONE_OR_SUB_MACHINE, 0, 0, nullptr, cmd, path, this);
							break;
						case ademco::ES_UNKNOWN:
						case ademco::ES_SMS:
						default:
							m_listctrl.SetCurSel(m_listctrl.InsertString(-1, TR(IDS_STRING_STOP_RTRV_BY_OFFLINE)));
							Reset();
							return;
							break;
						}
					}
				}
			}
			break;

			case EVENT_ENTER_SET_MODE: // enter set mode ok
			{
				unsigned char raw[5] = { 0xEB, 0xAB, 0x3F, 0xA1, 0x76 };
				auto cmd = std::make_shared<char_array>();
				std::copy(raw, raw + 5, std::back_inserter(*cmd));
				auto mgr = core::alarm_machine_manager::get_instance();
				auto path = m_machine->get_last_time_event_source();
				switch (path)
				{
				case ademco::ES_TCP_CLIENT:
					mgr->RemoteControlAlarmMachine(m_machine, EVENT_RETRIEVE_ZONE_OR_SUB_MACHINE, 0, 0, cmd, nullptr, path, this);
					break;
				case ademco::ES_TCP_SERVER1:
				case ademco::ES_TCP_SERVER2:
					mgr->RemoteControlAlarmMachine(m_machine, EVENT_RETRIEVE_ZONE_OR_SUB_MACHINE, 0, 0, nullptr, cmd, path, this);
					break;
				case ademco::ES_UNKNOWN:
				case ademco::ES_SMS:
				default:
					m_listctrl.SetCurSel(m_listctrl.InsertString(-1, TR(IDS_STRING_STOP_RTRV_BY_OFFLINE)));
					Reset();
					return;
					break;
				}
			}
				break;

			case EVENT_STOP_RETRIEVE: // cannot enter set mode, stop retrieve
				m_listctrl.SetCurSel(m_listctrl.InsertString(-1, TR(IDS_STRING_STOP_RTRV_BY_SET_MODE)));
				Reset();
				return;
				break;

			default:
				break;
			}
			
		}
		m_event_list.clear();
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CAutoRetrieveZoneInfoDlg::Reset()
{
	m_btnStart.SetWindowTextW(TR(IDS_STRING_IDC_BUTTON_START));
	KillTimer(1);

	int max_zone = get_machine_max_zone_by_type(m_machine->get_machine_type()) + 1;
	CString txt;
	txt.Format(L"0/%d", max_zone);
	m_staticProgress.SetWindowTextW(txt);
	m_progress.SetRange32(0, max_zone);
	m_progress.SetPos(0);

	m_staticTime.SetWindowTextW(L"00:00");
	m_observer.reset();
	m_event_list.clear();

	
	m_bRetrieving = FALSE;
}


void CAutoRetrieveZoneInfoDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	if (m_bRetrieving) {
		Reset();
		LeaveSetMode();
	}
}


void CAutoRetrieveZoneInfoDlg::LeaveSetMode()
{
	// send leave set mode
	auto path = m_machine->get_last_time_event_source();
	switch (path)
	{
	case ademco::ES_TCP_CLIENT:
		net::CNetworkConnector::get_instance()->Send(m_machine->get_ademco_id(), EVENT_STOP_RETRIEVE,
													0, 0, nullptr, nullptr, path);
		break;
	case ademco::ES_TCP_SERVER1:
	case ademco::ES_TCP_SERVER2:
		net::CNetworkConnector::get_instance()->Send(m_machine->get_ademco_id(), EVENT_STOP_RETRIEVE,
													0, 0, nullptr, nullptr, path);
		break;
	case ademco::ES_UNKNOWN:
	case ademco::ES_SMS:
	default:
		break;
	}
}
