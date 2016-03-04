// AutoRetrieveZoneInfoDlg.cpp : 实现文件
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
END_MESSAGE_MAP()


// CAutoRetrieveZoneInfoDlg 消息处理程序


void CAutoRetrieveZoneInfoDlg::OnBnClickedOk()
{
	return;
}


BOOL CAutoRetrieveZoneInfoDlg::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	assert(m_machine);

	if (m_machine->get_machine_type() == MT_IMPRESSED_GPRS_MACHINE_2050) {
		m_progress.SetRange32(0, 100);
		m_staticProgress.SetWindowTextW(L"0/100");
	} else {
		m_progress.SetRange32(0, 1000);
		m_staticProgress.SetWindowTextW(L"0/1000");
	}
	

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}


void CAutoRetrieveZoneInfoDlg::OnBnClickedButtonStart() 
{
	if (m_bRetrieving) {
		m_btnStart.SetWindowTextW(GetStringFromAppResource(IDS_STRING_START));
		KillTimer(1);
		m_progress.SetPos(0);
		m_staticProgress.SetWindowTextW(L"0/100"); // should be expressed_gprs_machine
		m_staticTime.SetWindowTextW(L"00:00");
		m_observer.reset();
		m_event_list.clear();
		m_bRetrieving = FALSE;
	} else {
		
		m_dwStartTime = GetTickCount();
		SetTimer(1, 100, nullptr);

		CString msg = L"", str = L"", fmok, fmfail, progress;
		fmok = GetStringFromAppResource(IDS_STRING_FM_RETRIEVE_OK);
		fmfail = GetStringFromAppResource(IDS_STRING_FM_RETRIEVE_FAILED);

		int max_machine_zone = 0;
		std::wstring max_progress = L"";
		if (m_machine->get_machine_type() == MT_IMPRESSED_GPRS_MACHINE_2050) {
			max_machine_zone = 100;
			max_progress = L"100";		
			m_progress.SetPos(1);
			m_staticProgress.SetWindowTextW(L"1/100");
			m_btnStart.SetWindowTextW(GetStringFromAppResource(IDS_STRING_STOP));
			m_bRetrieving = TRUE;

			if (!m_machine->get_online()) {
				m_listctrl.SetCurSel(m_listctrl.InsertString(-1, GetStringFromAppResource(IDS_STRING_STOP_RTRV_BY_OFFLINE)));
				OnBnClickedButtonStart();
				return;
			}

			m_observer = std::make_shared<ObserverType>(this);
			m_machine->register_observer(m_observer);

			// send enter set mode
			auto path = m_machine->get_last_time_event_source();
			switch (path)
			{
			case ademco::ES_TCP_CLIENT:
			{
				// direct mode, dont need to enter set mode
				//mgr->RemoteControlAlarmMachine(m_machine, EVENT_ENTER_SET_MODE, 0, 0, nullptr, nullptr, path, this);
				auto t = time(nullptr);
				m_event_list.push_back(std::make_shared<AdemcoEvent>(ES_TCP_CLIENT, EVENT_ENTER_SET_MODE, 0, 0, t, t, nullptr));
				break;
			}
			
			case ademco::ES_TCP_SERVER:
				//mgr->RemoteControlAlarmMachine(m_machine, EVENT_ENTER_SET_MODE, 0, 0, nullptr, nullptr, path, this);
				net::CNetworkConnector::GetInstance()->Send(m_machine->get_ademco_id(), EVENT_ENTER_SET_MODE, 
															0, 0, nullptr, nullptr, path);
				break;
			case ademco::ES_UNKNOWN:
			case ademco::ES_SMS:
			default:
				m_listctrl.SetCurSel(m_listctrl.InsertString(-1, GetStringFromAppResource(IDS_STRING_STOP_RTRV_BY_OFFLINE)));
				OnBnClickedButtonStart();
				return;
				break;
			}
			

		} else {
			max_machine_zone = MAX_MACHINE_ZONE;
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
		CZoneInfoPtr zoneInfo = m_machine->GetZone(zoneValue);
		if (zoneInfo) {
			CString fm; fm = GetStringFromAppResource(IDS_STRING_FM_ZONE_ALREADY_EXSISTS);
			msg.Format(fm, zoneInfo->get_alias());
			return true;
		}
		bool bNeedCreateSubMachine = false;
		bool bWireZone = WIRE_ZONE_RANGE_BEG <= zoneValue && zoneValue <= WIRE_ZONE_RANGE_END;
		if (!m_machine->get_is_submachine()) {
			if (zoneValue <= 0 || zoneValue >= MAX_MACHINE_ZONE) {
				msg = GetStringFromAppResource(IDS_STRING_E_ZONE_RANGE_FAILE);
				MessageBox(msg);
				break;
			}
			if (!bWireZone && MT_NETMOD == m_machine->get_machine_type()) {
				CRetrieveProgressDlg retrieveProgressDlg;
				retrieveProgressDlg.m_machine = m_machine;
				retrieveProgressDlg.m_zone = zoneValue;
				if (retrieveProgressDlg.DoModal() != IDOK) {
					msg = GetStringFromAppResource(IDS_STRING_USER_STOP_RESTRIEVE);
					break;
				}
				//int gg = retrieveProgressDlg.m_gg;
				CString alias, fmZone, fmSubMachine;
				fmZone = GetStringFromAppResource(IDS_STRING_ZONE);
				fmSubMachine = GetStringFromAppResource(IDS_STRING_SUBMACHINE);
				if (0xCC == retrieveProgressDlg.m_gg) { // not registered
					msg = GetStringFromAppResource(IDS_STRING_ZONE_NO_DUIMA);
					return true;
				} else if (0xEE == retrieveProgressDlg.m_gg) { // submachine
					zoneInfo = std::make_shared<CZoneInfo>();
					zoneInfo->set_ademco_id(m_machine->get_ademco_id());
					zoneInfo->set_zone_value(zoneValue);
					zoneInfo->set_type(ZT_SUB_MACHINE);
					zoneInfo->set_status_or_property(retrieveProgressDlg.m_status);
					zoneInfo->set_physical_addr(retrieveProgressDlg.m_addr);
					alias.Format(L"%s%03d", fmSubMachine, zoneValue);
					zoneInfo->set_alias(alias);
					bNeedCreateSubMachine = true;
				} else if (0x00 == retrieveProgressDlg.m_gg) { // direct
					zoneInfo = std::make_shared<CZoneInfo>();
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
				zoneInfo = std::make_shared<CZoneInfo>();
				zoneInfo->set_ademco_id(m_machine->get_ademco_id());
				zoneInfo->set_zone_value(zoneValue);
				zoneInfo->set_type(ZT_ZONE);
			}
		} 

		if (m_machine->execute_add_zone(zoneInfo)) {
			if (bNeedCreateSubMachine) {
				CString null;
				null = GetStringFromAppResource(IDS_STRING_NULL);
				CAlarmMachinePtr subMachine = std::make_shared<CAlarmMachine>();
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
				if (!zoneInfo->execute_set_sub_machine(subMachine)) {
					ASSERT(0); JLOG(L"execute_set_sub_machine failed.\n"); break;
				}
				//m_machine->inc_submachine_count();
				char status = zoneInfo->get_status_or_property() & 0xFF;
				ADEMCO_EVENT ademco_event = CZoneInfo::char_to_status(status);
				auto t = time(nullptr);
				m_machine->SetAdemcoEvent(ES_UNKNOWN, ademco_event, zoneValue, 0xEE, t, t);
			}
		} else {
			ASSERT(0); JLOG(L"m_machine->execute_add_zone(zoneInfo) failed.\n"); zoneInfo.reset(); break;
		}

		if (msg.IsEmpty()) {
			//CString szone, ssubmachine; szone = GetStringFromAppResource(IDS_STRING_ZONE);
			//ssubmachine = GetStringFromAppResource(IDS_STRING_SUBMACHINE);
			msg = zoneInfo->get_alias();
		}
		return true;
	} while (0);
	if (msg.IsEmpty())
		msg = GetStringFromAppResource(IDS_STRING_QUERY_FAILED);
	return false;
}


void CAutoRetrieveZoneInfoDlg::OnTimer(UINT_PTR nIDEvent)
{
	DWORD cnt = GetTickCount() - m_dwStartTime;
	if (cnt >= 1000) {
		cnt /= 1000;
		int min = cnt / 60;
		int sec = cnt % 60;
		CString t; t.Format(L"%02d:%02d", min, sec);
		m_staticTime.SetWindowTextW(t);
	}

	auto translate_serial_property_to_zone_property = [](int zone_property) {
		switch (zone_property)
		{
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

		default:
			break;
		}
		return ZSOP_INVALID;
	};

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
									zoneInfo = std::make_shared<CZoneInfo>();
									zoneInfo->set_ademco_id(m_machine->get_ademco_id());
									zoneInfo->set_zone_value(zone);
									zoneInfo->set_type(ZT_ZONE);
									zoneInfo->set_status_or_property(zp);
									txt.Format(L"%s%02d", GetStringFromAppResource(IDS_STRING_ZONE), zone);
									zoneInfo->set_alias(txt);
									m_machine->execute_add_zone(zoneInfo);
								}
								m_progress.SetPos(zone);
								txt.Format(L"%02d/100", zone);
								m_staticProgress.SetWindowTextW(txt);
								txt.Format(L"%s%02d", GetStringFromAppResource(IDS_STRING_ZONE), zone);
								m_listctrl.SetCurSel(m_listctrl.InsertString(-1, txt));
							}
						}
					}

					unsigned char status = ademcoEvent->_xdata->at(len - 2);
					if (status == 0xFF) { // over
						if (m_machine->get_zone_count() == 0) {
							m_listctrl.SetCurSel(m_listctrl.InsertString(-1, GetStringFromAppResource(IDS_STRING_NO_DUIMA_ZONE)));
						}
						m_listctrl.SetCurSel(m_listctrl.InsertString(-1, GetStringFromAppResource(IDS_STRING_RETRIEVE_OVER)));
						OnBnClickedButtonStart();

						// send leave set mode
						auto path = m_machine->get_last_time_event_source();
						switch (path)
						{
						case ademco::ES_TCP_CLIENT:
						{
							// direct mode, dont need to enter set mode
							//mgr->RemoteControlAlarmMachine(m_machine, EVENT_ENTER_SET_MODE, 0, 0, nullptr, nullptr, path, this);
							auto t = time(nullptr);
							m_event_list.push_back(std::make_shared<AdemcoEvent>(ES_TCP_CLIENT, EVENT_STOP_RETRIEVE, 
																				 0, 0, t, t, nullptr));
							break;
						}

						case ademco::ES_TCP_SERVER:
							//mgr->RemoteControlAlarmMachine(m_machine, EVENT_ENTER_SET_MODE, 0, 0, nullptr, nullptr, path, this);
							net::CNetworkConnector::GetInstance()->Send(m_machine->get_ademco_id(), EVENT_STOP_RETRIEVE,
																		0, 0, nullptr, nullptr, path);
							break;
						case ademco::ES_UNKNOWN:
						case ademco::ES_SMS:
						default:
							//m_listctrl.SetCurSel(m_listctrl.InsertString(-1, GetStringFromAppResource(IDS_STRING_STOP_RTRV_BY_OFFLINE)));
							//OnBnClickedButtonStart();
							//return;
							break;
						}
						return;
						
					} else { // continue, send A2
						unsigned char raw[5] = { 0xEB, 0xAB, 0x3F, 0xA2, 0x77 };
						auto cmd = std::make_shared<char_array>();
						std::copy(raw, raw + 5, std::back_inserter(*cmd));
						auto mgr = core::CAlarmMachineManager::GetInstance();
						auto path = m_machine->get_last_time_event_source();
						switch (path)
						{
						case ademco::ES_TCP_CLIENT:
							mgr->RemoteControlAlarmMachine(m_machine, EVENT_RETRIEVE_ZONE_OR_SUB_MACHINE, 0, 0, cmd, nullptr, path, this);
							break;
						case ademco::ES_TCP_SERVER:
							mgr->RemoteControlAlarmMachine(m_machine, EVENT_RETRIEVE_ZONE_OR_SUB_MACHINE, 0, 0, nullptr, cmd, path, this);
							break;
						case ademco::ES_UNKNOWN:
						case ademco::ES_SMS:
						default:
							m_listctrl.SetCurSel(m_listctrl.InsertString(-1, GetStringFromAppResource(IDS_STRING_STOP_RTRV_BY_OFFLINE)));
							OnBnClickedButtonStart();
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
				auto mgr = core::CAlarmMachineManager::GetInstance();
				auto path = m_machine->get_last_time_event_source();
				switch (path)
				{
				case ademco::ES_TCP_CLIENT:
					mgr->RemoteControlAlarmMachine(m_machine, EVENT_RETRIEVE_ZONE_OR_SUB_MACHINE, 0, 0, cmd, nullptr, path, this);
					break;
				case ademco::ES_TCP_SERVER:
					mgr->RemoteControlAlarmMachine(m_machine, EVENT_RETRIEVE_ZONE_OR_SUB_MACHINE, 0, 0, nullptr, cmd, path, this);
					break;
				case ademco::ES_UNKNOWN:
				case ademco::ES_SMS:
				default:
					m_listctrl.SetCurSel(m_listctrl.InsertString(-1, GetStringFromAppResource(IDS_STRING_STOP_RTRV_BY_OFFLINE)));
					OnBnClickedButtonStart();
					return;
					break;
				}
			}
				break;

			case EVENT_STOP_RETRIEVE: // cannot enter set mode, stop retrieve
				m_listctrl.SetCurSel(m_listctrl.InsertString(-1, GetStringFromAppResource(IDS_STRING_STOP_RTRV_BY_SET_MODE)));
				OnBnClickedButtonStart();
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
