#include "stdafx.h"
#include "AlarmMachine.h"
#include "ZoneInfo.h"
#include "DetectorInfo.h"
#include "MapInfo.h"
#include "ademco_event.h"
//#include "ademco_func.h"
#include "resource.h"
#include "HistoryRecord.h"
#include "AppResource.h"
#include "UserInfo.h"
#include "AlarmMachineManager.h"
#include "GroupInfo.h"
//#include "SubMachineInfo.h"
#include "SoundPlayer.h"
#include <algorithm>
#include <iterator>
#include "Gsm.h"
#include "tinyxml\\tinyxml.h"

using namespace ademco;
namespace core {

#ifdef _DEBUG
static const int CHECK_EXPIRE_GAP_TIME = 6 * 1000; // check machine if expire in every 6 seconds.
#else
static const int CHECK_EXPIRE_GAP_TIME = 60 * 1000; // check machine if expire in every minutes.
#endif

IMPLEMENT_OBSERVER(CAlarmMachine)
	
CAlarmMachine::CAlarmMachine()
	: _id(0)
	, _ademco_id(0)
	, _group_id(0)
	, _alias(NULL)
	, _contact(NULL)
	, _address(NULL)
	, _phone(NULL)
	, _phone_bk(NULL)
	, _online(false)
	, _armed(false)
	, _alarming(false)
	, _has_alarming_direct_zone(false)
	, _buffer_mode(false)
	, _is_submachine(false)
	, _has_video(false)
	, _submachine_zone(0)
	, _submachine_count(0)
	, _unbindZoneMap(NULL)
	, _highestEventLevel(EVENT_LEVEL_NULL)
	, _alarmingSubMachineCount(0)
	, _lastActionTime(time(NULL))
	, _bChecking(false)
	, _expire_time()
	, _last_time_check_if_expire(0)
	, _coor()

	// 2015年8月1日 14:46:21 storaged in xml
	, _auto_show_map_when_start_alarming(true)
	, _privatePacket(NULL)
{
	//memset(_device_id, 0, sizeof(_device_id));
	//memset(_device_idW, 0, sizeof(_device_idW));
	memset(_ipv4, 0, sizeof(_ipv4));

	_alias = new wchar_t[1];
	_alias[0] = 0; 
	
	_contact = new wchar_t[1];
	_contact[0] = 0;
	
	_address = new wchar_t[1];
	_address[0] = 0;
	
	_phone = new wchar_t[1];
	_phone[0] = 0;
	
	_phone_bk = new wchar_t[1];
	_phone_bk[0] = 0;

	//_expire_time = new wchar_t[1];
	//_expire_time[0] = 0;

	memset(_zoneArray, 0, sizeof(_zoneArray));

	_unbindZoneMap = new CMapInfo();
	_unbindZoneMap->set_id(-1);
	CString fmAlias;
	fmAlias.LoadStringW(IDS_STRING_NOZONEMAP);
	_unbindZoneMap->set_alias(fmAlias);

	//LoadXmlConfig();
}


CAlarmMachine::~CAlarmMachine()
{
	AdemcoEvent ademcoEvent(ES_UNKNOWN, EVENT_IM_GONNA_DIE, 0, 0, time(NULL), time(NULL), NULL, 0);
	NotifyObservers(&ademcoEvent);
	DESTROY_OBSERVER;

	if (_alias) { delete[] _alias; }
	if (_contact) { delete[] _contact; }
	if (_address) { delete[] _address; }
	if (_phone) { delete[] _phone; }
	if (_phone_bk) { delete[] _phone_bk; }
	//if (_expire_time) { delete[] _expire_time; }
	if (_unbindZoneMap) { delete _unbindZoneMap; }

	for (auto map : _mapList) {
		delete map;
	}
	_mapList.clear();

	for (auto ademcoEvent : _ademcoEventList) {
		delete ademcoEvent;
	}
	_ademcoEventList.clear();

	for (auto ademcoEvent : _ademcoEventFilter) {
		delete ademcoEvent;
	}
	_ademcoEventFilter.clear();

	for (int i = 0; i < MAX_MACHINE_ZONE; i++) {
		CZoneInfo* zone = _zoneArray[i];
		SAFEDELETEP(zone);
	}

	SAFEDELETEP(_privatePacket);
}


void CAlarmMachine::SetPrivatePacket(const ademco::PrivatePacket* privatePacket)
{
	if (privatePacket == NULL) {
		SAFEDELETEP(_privatePacket);
		return;
	}

	if (_privatePacket == NULL)
		_privatePacket = new PrivatePacket();
	_privatePacket->Copy(privatePacket);
}


const ademco::PrivatePacket* CAlarmMachine::GetPrivatePacket() const
{
	return _privatePacket;
}


std::string CAlarmMachine::get_xml_path()
{
	USES_CONVERSION;
	CString dir = L"", path = L"";
	dir.Format(L"%s\\config", GetModuleFilePath());
	CreateDirectory(dir, NULL);
	dir += L"\\AlarmMachine";
	CreateDirectory(dir, NULL);
	if (_is_submachine) {
		path.Format(L"%s\\%04d-%03d.xml", dir, _ademco_id, _submachine_zone);
	} else {
		path.Format(L"%s\\%04d.xml", dir, _ademco_id);
	}
	return W2A(path);
}


void CAlarmMachine::LoadXmlConfig()
{
	using namespace tinyxml;
	std::string path = get_xml_path();
	TiXmlDocument doc(path.c_str());
	bool ok = false;
	do {
		if (!doc.LoadFile()) break;
		TiXmlElement* root = doc.FirstChildElement();
		if (!root)break;
		TiXmlElement* auto_show_map_when_start_alarming = root->FirstChildElement("auto_show_map_when_start_alarming");
		if (!auto_show_map_when_start_alarming)break;
		const char* text = auto_show_map_when_start_alarming->Attribute("value");
		if (!text || strlen(text) == 0) break;
		_auto_show_map_when_start_alarming = atoi(text) == 1;
		ok = true;
	} while (0);

	if (!ok) {
		SaveXmlConfig();
	}
}


void CAlarmMachine::SaveXmlConfig()
{
	using namespace tinyxml;
	std::string path = get_xml_path();
	TiXmlDocument doc;
	TiXmlDeclaration* dec = new TiXmlDeclaration("1.0", "", "");
	doc.LinkEndChild(dec);
	TiXmlElement* root = new TiXmlElement("AlarmMachineConfig");
	doc.LinkEndChild(root);
	TiXmlElement* auto_show_map_when_start_alarming = new TiXmlElement("auto_show_map_when_start_alarming");
	auto_show_map_when_start_alarming->SetAttribute("value", _auto_show_map_when_start_alarming);
	root->LinkEndChild(auto_show_map_when_start_alarming);
	doc.SaveFile(path.c_str());
}


void CAlarmMachine::set_auto_show_map_when_start_alarming(bool b)
{
	if (b != _auto_show_map_when_start_alarming) {
		_auto_show_map_when_start_alarming = b;
		SaveXmlConfig();
	}
}


void CAlarmMachine::clear_ademco_event_list()
{
	_lock4AdemcoEventList.Lock();
	_alarming = false;
	_has_alarming_direct_zone = false;
	_highestEventLevel = EVENT_LEVEL_STATUS;
	_alarmingSubMachineCount = 0;

	if (!_is_submachine) {
		CWinApp* app = AfxGetApp(); ASSERT(app);
		CWnd* wnd = app->GetMainWnd(); ASSERT(wnd);
		wnd->PostMessage(WM_ADEMCOEVENT, (WPARAM)this, 0);
	}

	for (auto ademcoEvent : _ademcoEventList) {
		delete ademcoEvent;
	}
	_ademcoEventList.clear();

	AdemcoEvent* ademcoEvent = new AdemcoEvent(ES_UNKNOWN, EVENT_CLEARMSG, 0, 0, time(NULL), time(NULL), NULL, 0); // default 0
	NotifyObservers(ademcoEvent);
	if (_unbindZoneMap) {
		_unbindZoneMap->InversionControl(ICMC_CLR_ALARM_TEXT);
	}

	for (auto zoneInfo : _validZoneList) {
		CMapInfo* mapInfo = zoneInfo->GetMapInfo();
		if (mapInfo) {
			mapInfo->InversionControl(ICMC_CLR_ALARM_TEXT);
		}
		if (zoneInfo->get_type() == ZT_SUB_MACHINE) {
			CAlarmMachine* subMachine = zoneInfo->GetSubMachineInfo();
			if (subMachine) {
				subMachine->clear_ademco_event_list();
			}
		}
		zoneInfo->HandleAdemcoEvent(ademcoEvent);
	}
	delete ademcoEvent;

	// add a record
	CString srecord, suser, sfm, sop, spost, fmSubmachine;
	suser.LoadStringW(IDS_STRING_USER);
	sfm.LoadStringW(IDS_STRING_LOCAL_OP);
	sop.LoadStringW(IDS_STRING_CLR_MSG);
	fmSubmachine.LoadStringW(IDS_STRING_SUBMACHINE);
	const CUserInfo* user = CUserManager::GetInstance()->GetCurUserInfo();
	srecord.Format(L"%s(ID:%d,%s)%s:%s", suser,
				   user->get_user_id(), user->get_user_name(),
				   sfm, sop);
	if (_is_submachine) {
		CAlarmMachine* netMachine = NULL;
		CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
		if (mgr->GetMachine(_ademco_id, netMachine)) {
			spost.Format(L"%04d(%s)%s%03d(%s)", _ademco_id, netMachine->get_alias(),
						 fmSubmachine, _submachine_zone, _alias);
			netMachine->dec_alarmingSubMachineCount();
		}
	} else {
		spost.Format(L"%04d(%s)", _ademco_id, _alias);
	}
	srecord += spost;
	CHistoryRecord::GetInstance()->InsertRecord(get_ademco_id(),
												_is_submachine ? _submachine_zone : 0,
												srecord, time(NULL),
												RECORD_LEVEL_USERCONTROL);
	_lock4AdemcoEventList.UnLock();
}


bool CAlarmMachine::EnterBufferMode()
{ 
	AUTO_LOG_FUNCTION;
	_ootebmOjb.call();
	if (_lock4AdemcoEventList.TryLock()) {
		_buffer_mode = true;
		_lock4AdemcoEventList.UnLock();
		return true;
	}
	return false;
}


bool CAlarmMachine::LeaveBufferMode()
{
	AUTO_LOG_FUNCTION;
	if (_lock4AdemcoEventList.TryLock()) {
		_buffer_mode = false;
		for (auto ademcoEvent: _ademcoEventList) {
			HandleAdemcoEvent(ademcoEvent);
		}
		_ademcoEventList.clear();
		_lock4AdemcoEventList.UnLock();
		return true;
	}
	return false;
}


void CAlarmMachine::TraverseAdmecoEventList(void* udata, AdemcoEventCB cb)
{
	AUTO_LOG_FUNCTION;
	_lock4AdemcoEventList.Lock();
	for (auto ademcoEvent : _ademcoEventList) {
		if (udata && cb) {
			cb(udata, ademcoEvent);
		}
	}
	_lock4AdemcoEventList.UnLock();
}


CMapInfo* CAlarmMachine::GetMapInfo(int map_id)
{
	for (auto mapInfo : _mapList) {
		if (mapInfo->get_id() == map_id)
			return mapInfo;
	}
	return NULL;
}


void CAlarmMachine::HandleAdemcoEvent(const ademco::AdemcoEvent* ademcoEvent, 
									  BOOL bDeleteAfterHandled)
{
	AUTO_LOG_FUNCTION;

	// check if expire
	if (GetTickCount() - _last_time_check_if_expire > CHECK_EXPIRE_GAP_TIME) {
		if (get_left_service_time() <= 0) {
			CString rec, fmmachine, fmsubmachine, fmexpire;
			fmmachine.LoadStringW(IDS_STRING_MACHINE);
			fmsubmachine.LoadStringW(IDS_STRING_SUBMACHINE);
			fmexpire.LoadStringW(IDS_STRING_EXPIRE);
			int zoneValue = 0;
			if (_is_submachine) {
				CString parentAlias; parentAlias.LoadStringW(IDS_STRING_NULL);
				CAlarmMachine* parentMachine = NULL;
				if (CAlarmMachineManager::GetInstance()->GetMachine(_ademco_id, parentMachine) && parentMachine) {
					parentAlias = parentMachine->get_alias();
				}
				rec.Format(L"%s%04d(%s)%s%03d(%s) %s", fmmachine, _ademco_id, parentAlias, fmsubmachine, _submachine_zone, _alias, fmexpire);
				zoneValue = _submachine_zone;
			} else {
				rec.Format(L"%s%04d(%s) %s", fmmachine, _ademco_id, _alias, fmexpire);
			}
			CHistoryRecord::GetInstance()->InsertRecord(_ademco_id, zoneValue, rec, 
														ademcoEvent->_recv_time, 
														RECORD_LEVEL_EXCEPTION);
		}
		_last_time_check_if_expire = GetTickCount();
	}

	// handle ademco event
	if (!_is_submachine) {
		if (_banned && bDeleteAfterHandled) {
			delete ademcoEvent;
			return;
		}
#pragma region define val
		bool bMachineStatus = false;
		bool bOnofflineStatus = false;
		CString fmEvent, fmNull, record, fmMachine, fmSubMachine, fmZone, fmHangup, fmResume;
		fmNull.LoadStringW(IDS_STRING_NULL);
		fmMachine.LoadStringW(IDS_STRING_MACHINE);
		fmSubMachine.LoadStringW(IDS_STRING_SUBMACHINE);
		fmZone.LoadStringW(IDS_STRING_ZONE);
		fmHangup.LoadStringW(IDS_STRING_CONN_HANGUP);
		fmResume.LoadStringW(IDS_STRING_CONN_RESUME);
		bool online = true;
		bool armed = true;
		CZoneInfo* zone = GetZone(ademcoEvent->_zone);
		CAlarmMachine* subMachine = NULL;
		CString aliasOfZoneOrSubMachine = fmNull;
		if (zone) {
			subMachine = zone->GetSubMachineInfo();
			if (subMachine) { aliasOfZoneOrSubMachine = subMachine->get_alias(); } 
			else { aliasOfZoneOrSubMachine = zone->get_alias(); }
		}
#pragma endregion

#pragma region switch event
		switch (ademcoEvent->_event) {
			case ademco::EVENT_OFFLINE:
				bOnofflineStatus = true; _rcccObj.reset();
				bMachineStatus = true; online = false; fmEvent.LoadStringW(IDS_STRING_OFFLINE);
				//CSoundPlayer::GetInstance()->Play(CSoundPlayer::SI_OFFLINE); 
				CSoundPlayer::GetInstance()->IncOffLineMachineNum();
				break;
			case ademco::EVENT_ONLINE: bOnofflineStatus = true; 
				bMachineStatus = true; fmEvent.LoadStringW(IDS_STRING_ONLINE);
				CSoundPlayer::GetInstance()->DecOffLineMachineNum();
				break;
			case ademco::EVENT_CONN_HANGUP:
				if (_rcccObj.valid()) { _rcccObj.cb(_rcccObj.udata, RCCC_HANGUP); }
				record.Format(L"%s%04d(%s) %s", fmMachine, _ademco_id, _alias, fmHangup);
				CHistoryRecord::GetInstance()->InsertRecord(_ademco_id, -1, record, 
															ademcoEvent->_recv_time,
															RECORD_LEVEL_ONOFFLINE);
				delete ademcoEvent;
				return;
				break;
			case ademco::EVENT_CONN_RESUME:
				if (_rcccObj.valid()) { _rcccObj.cb(_rcccObj.udata, RCCC_RESUME); }
				record.Format(L"%s%04d(%s) %s", fmMachine, _ademco_id, _alias, fmResume);
				CHistoryRecord::GetInstance()->InsertRecord(_ademco_id, -1, record,
															ademcoEvent->_recv_time,
															RECORD_LEVEL_ONOFFLINE); 
				delete ademcoEvent;
				return;
				break;
			case ademco::EVENT_RETRIEVE_SUB_MACHINE:
				HandleRetrieveResult(ademcoEvent);
				delete ademcoEvent;
				return;
				break;
			case ademco::EVENT_QUERY_SUB_MACHINE:
				delete ademcoEvent;
				return;
				break;
			case ademco::EVENT_I_AM_NET_MODULE:
				execute_set_machine_type(MT_NETMOD);
				NotifyObservers(ademcoEvent);
				NotifySubmachines(ademcoEvent);
				delete ademcoEvent;
				return;
				break;
			case ademco::EVENT_DISARM_PWD_ERR:
				CAlarmMachineManager::GetInstance()->DisarmPasswdWrong(_ademco_id);
				delete ademcoEvent;
				return;
				break;
			case ademco::EVENT_DISARM: bMachineStatus = true; armed = false; fmEvent.LoadStringW(IDS_STRING_DISARM);
				break;
			case ademco::EVENT_ARM: bMachineStatus = true; armed = true; fmEvent.LoadStringW(IDS_STRING_ARM);
				break;
			case ademco::EVENT_RECONNECT:
			case ademco::EVENT_SERIAL485CONN:
			case ademco::EVENT_SUB_MACHINE_SENSOR_RESUME:
			case ademco::EVENT_SUB_MACHINE_POWER_RESUME:
				bMachineStatus = false;
				break;
			case ademco::EVENT_EMERGENCY:
			case ademco::EVENT_BADBATTERY:
			case ademco::EVENT_LOWBATTERY:
			//	bMachineStatus = true;
			case ademco::EVENT_BURGLAR:
			case ademco::EVENT_DURESS:
			case ademco::EVENT_DISCONNECT:
			case ademco::EVENT_SOLARDISTURB:
			case ademco::EVENT_SUB_MACHINE_SENSOR_EXCEPTION:
			case ademco::EVENT_SUB_MACHINE_POWER_EXCEPTION:
				CSoundPlayer::GetInstance()->Play(CSoundPlayer::SI_BUGLAR);
				break;
			case ademco::EVENT_SERIAL485DIS:
				CSoundPlayer::GetInstance()->Play(CSoundPlayer::SI_OFFLINE);
				break;
			case ademco::EVENT_DOORRINGING:
				CSoundPlayer::GetInstance()->Play(CSoundPlayer::SI_DOORRING);
				break;
			case ademco::EVENT_FIRE:
				CSoundPlayer::GetInstance()->Play(CSoundPlayer::SI_FIRE);
				break;
			case ademco::EVENT_GAS:
				CSoundPlayer::GetInstance()->Play(CSoundPlayer::SI_GAS);
				break;
			case ademco::EVENT_TEMPER:
				CSoundPlayer::GetInstance()->Play(CSoundPlayer::SI_PLEASE_HELP);
				break;
			case ademco::EVENT_WATER:
				CSoundPlayer::GetInstance()->Play(CSoundPlayer::SI_WATER);
				break;
			default: bMachineStatus = false;
				break;
		}
#pragma endregion

		// define AdemcoDataSegment for sending sms
		AdemcoDataSegment dataSegment;
		dataSegment.Make(_ademco_id, ademcoEvent->_sub_zone, ademcoEvent->_event, ademcoEvent->_zone);

		if (bMachineStatus) {	// status of machine
			bool bStatusChanged = false;
#pragma region online or armed
			if ((ademcoEvent->_zone == 0) && (ademcoEvent->_sub_zone == INDEX_ZONE)) {
				_online = online;
				if (!bOnofflineStatus && (_armed != armed)) {
					bStatusChanged = true;
					execute_set_armd(armed);
				}
			}
#pragma endregion

#pragma region status event
			CGsm* gsm = CGsm::GetInstance();
			if (ademcoEvent->_zone == 0) { // netmachine
				record.Format(L"%s%04d(%s) %s", fmMachine, _ademco_id, _alias,
							  fmEvent);
				// 2015-06-05 16:35:49 submachine on/off line status follow machine on/off line status
				if (bOnofflineStatus && !_is_submachine) {
					SetAllSubMachineOnOffLine(online);
				} 
			} else { // submachine
				record.Format(L"%s%04d(%s) %s%03d(%s) %s",
							  fmMachine, _ademco_id, _alias,
							  fmSubMachine, ademcoEvent->_zone, aliasOfZoneOrSubMachine,
							  fmEvent);
				if (subMachine) {
					//subMachine->_online = online;
					if (!bOnofflineStatus) {
						if (subMachine->get_armed() != armed)
							bStatusChanged = true;

						if (subMachine->execute_set_armd(armed)) {
							subMachine->SetAdemcoEvent(ademcoEvent->_resource,
													   ademcoEvent->_event,
													   ademcoEvent->_zone,
													   ademcoEvent->_sub_zone, 
													   ademcoEvent->_timestamp,
													   ademcoEvent->_recv_time,
													   ademcoEvent->_xdata, 
													   ademcoEvent->_xdata_len);
						}
					}
						
					if (bStatusChanged) {
						SmsConfigure cfg = subMachine->get_sms_cfg();
						if (_tcslen(subMachine->get_phone()) != 0) {
							if (cfg.report_status) {
								gsm->SendSms(subMachine->get_phone(), &dataSegment, fmEvent);
							}
						}

						if (_tcslen(subMachine->get_phone_bk()) != 0) {
							if (cfg.report_status_bk) {
								gsm->SendSms(subMachine->get_phone_bk(), &dataSegment, fmEvent);
							}
						}
					}
					
				}
			}

			if (!bOnofflineStatus && bStatusChanged) {
				if (_tcslen(_phone) != 0) {
					if (_sms_cfg.report_status) {
						gsm->SendSms(_phone, &dataSegment, record);
					}
				}

				if (_tcslen(_phone_bk) != 0) {
					if (_sms_cfg.report_status_bk) {
						gsm->SendSms(_phone_bk, &dataSegment, record);
					}
				}
			}
			CHistoryRecord::GetInstance()->InsertRecord(get_ademco_id(), 
														ademcoEvent->_zone,
														record, 
														ademcoEvent->_recv_time,
														RECORD_LEVEL_ONOFFLINE);
#pragma endregion
		} else {				// alarm or exception event
#pragma region alarm event
			_alarming = true;

#pragma region format text
			CString smachine(L""), szone(L""), sevent(L""), stmp(L"");
			smachine.Format(L"%s%04d(%s) ", fmMachine, _ademco_id, _alias);

			if (ademcoEvent->_zone != 0) {
				if (ademcoEvent->_sub_zone == INDEX_ZONE) {
					szone.Format(L"%s%03d(%s)", fmZone, ademcoEvent->_zone, aliasOfZoneOrSubMachine);
				} else {
					stmp.Format(L"%s%03d(%s)", fmSubMachine, ademcoEvent->_zone, aliasOfZoneOrSubMachine);
					smachine += stmp; 
					if (ademcoEvent->_sub_zone != INDEX_SUB_MACHINE) {
						CString ssubzone, ssubzone_alias = fmNull;
						if (subMachine) {
							CZoneInfo* subZone = subMachine->GetZone(ademcoEvent->_sub_zone);
							if (subZone) { ssubzone_alias = subZone->get_alias(); }
						}
						ssubzone.Format(L" %s%02d(%s)", fmZone, ademcoEvent->_sub_zone, ssubzone_alias);
						szone += ssubzone;
					}
				}
			} else {
				szone = smachine;
				smachine.Empty();
			}

			CAppResource* res = CAppResource::GetInstance();
			sevent.Format(L"%s", res->AdemcoEventToString(ademcoEvent->_event));

			time_t timestamp = ademcoEvent->_recv_time;
			wchar_t wtime[32] = { 0 };
			struct tm tmtm;
			localtime_s(&tmtm, &timestamp);
			if (timestamp == -1) {
				timestamp = time(NULL);
				localtime_s(&tmtm, &timestamp);
			}
			wcsftime(wtime, 32, L"%H:%M:%S", &tmtm);

			AlarmText* at = new AlarmText();
			at->_zone = ademcoEvent->_zone;
			at->_subzone = ademcoEvent->_sub_zone;
			at->_event = ademcoEvent->_event;
			at->_txt.Format(L"%s %s %s", wtime, szone, sevent);
#pragma endregion

			EventLevel eventLevel = GetEventLevel(ademcoEvent->_event);
			set_highestEventLevel(eventLevel);

#pragma region write history recored
			CHistoryRecord *hr = CHistoryRecord::GetInstance();
			RecordLevel recordLevel = RECORD_LEVEL_ALARM;
			if (eventLevel == EVENT_LEVEL_EXCEPTION || eventLevel == EVENT_LEVEL_EXCEPTION_RESUME) {
				recordLevel = RECORD_LEVEL_EXCEPTION;
			}
			hr->InsertRecord(get_ademco_id(), ademcoEvent->_zone,
							 smachine + szone + L" " + sevent,
							 ademcoEvent->_recv_time, recordLevel);
#pragma endregion

			// ui
			// 1. main view btn flash
			CWinApp* app = AfxGetApp(); ASSERT(app);
			CWnd* wnd = app->GetMainWnd(); ASSERT(wnd);
			wnd->PostMessage(WM_ADEMCOEVENT, (WPARAM)this, 1);

			// 2. alarm text
			if (zone) {	// 2.1 ÓÐ·ÀÇøÐÅÏ¢
				CMapInfo* mapInfo = zone->GetMapInfo();
				AlarmText* dupAt = new AlarmText(*at);
				if (subMachine) {
					CZoneInfo* subZone = subMachine->GetZone(ademcoEvent->_sub_zone);
					if (subZone) {
						subZone->HandleAdemcoEvent(ademcoEvent);
						CMapInfo* subMap = subZone->GetMapInfo();
						if (subMap) {
							subMap->InversionControl(ICMC_ADD_ALARM_TEXT, dupAt);
						}
					} else {
						subMachine->_unbindZoneMap->InversionControl(ICMC_ADD_ALARM_TEXT, dupAt);
					}
				} else {
					mapInfo->InversionControl(ICMC_ADD_ALARM_TEXT, at);
					zone->HandleAdemcoEvent(ademcoEvent);
					delete dupAt;
				}
			} else {	// 2.2 no zone alarm map
				_unbindZoneMap->InversionControl(ICMC_ADD_ALARM_TEXT, at);
			}

			if (subMachine) {
				if (!subMachine->get_alarming()) {
					subMachine->set_alarming(true);
					_alarmingSubMachineCount++;
				}
				subMachine->set_highestEventLevel(GetEventLevel(ademcoEvent->_event));
				subMachine->HandleAdemcoEvent(ademcoEvent, FALSE);
			} else {
				_has_alarming_direct_zone = true;
			}

			

			// send sms
			CGsm* gsm = CGsm::GetInstance();
			if (_tcslen(_phone) != 0) {
				if ((_sms_cfg.report_alarm && (eventLevel == EVENT_LEVEL_ALARM))
					|| (_sms_cfg.report_exception && (eventLevel == EVENT_LEVEL_EXCEPTION || eventLevel == EVENT_LEVEL_EXCEPTION_RESUME))) {
					gsm->SendSms(_phone, &dataSegment, szone + sevent);
				}
			}

			if (_tcslen(_phone_bk) != 0) {
				if ((_sms_cfg.report_alarm_bk && eventLevel == EVENT_LEVEL_ALARM)
					|| (_sms_cfg.report_exception_bk && (eventLevel == EVENT_LEVEL_EXCEPTION || eventLevel == EVENT_LEVEL_EXCEPTION_RESUME))) {
					gsm->SendSms(_phone_bk, &dataSegment, szone + sevent);
				}
			}

			if (subMachine) {
				SmsConfigure cfg = subMachine->get_sms_cfg();
				if (_tcslen(subMachine->get_phone()) != 0) {
					if ((cfg.report_alarm && (eventLevel == EVENT_LEVEL_ALARM))
						|| (cfg.report_exception && (eventLevel == EVENT_LEVEL_EXCEPTION || eventLevel == EVENT_LEVEL_EXCEPTION_RESUME))) {
						gsm->SendSms(subMachine->get_phone(), &dataSegment, szone + sevent);
					}
				}

				if (_tcslen(subMachine->get_phone_bk()) != 0) {
					if ((cfg.report_alarm_bk && eventLevel == EVENT_LEVEL_ALARM)
						|| (cfg.report_exception_bk && (eventLevel == EVENT_LEVEL_EXCEPTION || eventLevel == EVENT_LEVEL_EXCEPTION_RESUME))) {
						gsm->SendSms(subMachine->get_phone_bk(), &dataSegment, szone + sevent);
					}
				}
			}
#pragma endregion
		}
	} else { // _is_submachine
		UpdateLastActionTime();
	}
	NotifyObservers(ademcoEvent);
	if (bDeleteAfterHandled)
		delete ademcoEvent;
}


void CAlarmMachine::SetAllSubMachineOnOffLine(bool online)
{
	for (auto zoneInfo : _validZoneList) {
		CAlarmMachine* subMachine = zoneInfo->GetSubMachineInfo();
		if (subMachine) {
			subMachine->set_online(online);
			subMachine->SetAdemcoEvent(ES_UNKNOWN, online ? (subMachine->get_armed() ? EVENT_ARM : EVENT_DISARM) : EVENT_OFFLINE,
									   subMachine->get_submachine_zone(), 
									   INDEX_SUB_MACHINE, time(NULL), time(NULL), NULL, 0);
		}
	}
}


void CAlarmMachine::HandleRetrieveResult(const ademco::AdemcoEvent* ademcoEvent)
{
	AUTO_LOG_FUNCTION;
	int gg = ademcoEvent->_sub_zone;
	if (!(ademcoEvent->_xdata && (ademcoEvent->_xdata_len == 3))) {
		ASSERT(0); return;
	}
	char status = ademcoEvent->_xdata[0];
	int addr = MAKEWORD(ademcoEvent->_xdata[2], ademcoEvent->_xdata[1]);
	LOG(L"gg %d, zone %d, status %02X, addr %04X\n", 
		gg, ademcoEvent->_zone, status, addr & 0xFFFF);

	CZoneInfo* zoneInfo = GetZone(ademcoEvent->_zone);
	if (!zoneInfo) { // ÎÞÊý¾Ý£¬ÕâÊÇË÷Òª²Ù×÷µÄ»ØÓ¦
		LOG(L"no zoneInfo for %d\n", ademcoEvent->_zone);
		// ½»¸ø ¡°²éÑ¯ËùÓÐÖ÷»ú¡±½çÃæ CRetrieveProgressDlg ´¦Àí
		NotifyObservers(ademcoEvent);
	} else { // ÒÑ¾­ÓÐÊý¾Ý£¬ÕâÊÇ»Ö¸´Ö÷»úÊý¾ÝµÄ»ØÓ¦
		LOG(L"has zoneInfo for %d\n", ademcoEvent->_zone);
		CAlarmMachine* subMachine = zoneInfo->GetSubMachineInfo();
		if (subMachine) {
			LOG(L"has submachine info\n");
			subMachine->UpdateLastActionTime();
		} else {
			LOG(L"no submachine info\n");
		}

		bool ok = true;


		if (is_zone_status(static_cast<unsigned char>(status & 0xFF)) != 
			is_zone_status(static_cast<unsigned char>(zoneInfo->get_status_or_property() & 0xFF))) {
			//zoneInfo->execute_set_status_or_property(status);
			LOG(L"status %02X != zoneInfo->get_status_or_property() %02X\n", 
				status, zoneInfo->get_status_or_property());
			ok = false;
		}
		if (addr != zoneInfo->get_physical_addr()) {
			//zoneInfo->execute_set_physical_addr(addr);
			LOG(L"addr %04X != zoneInfo->get_physical_addr() %04X\n",
				addr, zoneInfo->get_physical_addr());
			ok = false;
		}

		if (ok) {
			LOG(L"ok\n");
			if ((gg == 0xEE) && (subMachine != NULL)) {
				LOG(L"(gg == 0xEE) && (subMachine != NULL)\n");
				ADEMCO_EVENT ademco_event = CZoneInfo::char_to_status(status);
				SetAdemcoEvent(ademcoEvent->_resource, ademco_event, zoneInfo->get_zone_value(), 0xEE,
							   time(NULL), time(NULL), NULL, 0);
			} else if ((gg == 0x00) && (subMachine == NULL)) {
				LOG(L"(gg == 0x00) && (subMachine == NULL)\n");
			} else { ok = false; ASSERT(0); }
			
			if (ok) { // ½»¸ø¡°»Ö¸´Ö÷»úÊý¾Ý¡±½çÃæ  CRestoreMachineDlg ´¦Àí
				LOG(L"ok\n"); 
				NotifyObservers(ademcoEvent);
			} else {
				LOG(L"failed.\n");
			}
		}
	}
}


void CAlarmMachine::NotifySubmachines(const ademco::AdemcoEvent* ademcoEvent)
{
	for (auto zoneInfo : _validZoneList) {
		if (zoneInfo->get_type() == ZT_SUB_MACHINE) {
			CAlarmMachine* subMachine = zoneInfo->GetSubMachineInfo();
			if (subMachine) {
				subMachine->set_machine_type(_machine_type);
				subMachine->HandleAdemcoEvent(ademcoEvent, FALSE);
			}
		}
	}
}


void CAlarmMachine::SetAdemcoEvent(EventSource resource, 
								   int ademco_event, int zone, int subzone,
								   const time_t& timestamp, const time_t& recv_time,
								   const char* xdata, int xdata_len)
{
	AUTO_LOG_FUNCTION;
	_lock4AdemcoEventList.Lock();
	AdemcoEvent* ademcoEvent = new AdemcoEvent(resource, ademco_event, zone, subzone, timestamp, recv_time, xdata, xdata_len);
	if (EVENT_PRIVATE_EVENT_MIN <= ademco_event && ademco_event <= EVENT_PRIVATE_EVENT_MAX) {
		// 内部事件立即处理
	} else {
#ifdef _DEBUG
		wchar_t wtime[32] = { 0 };
		struct tm tmtm;
		localtime_s(&tmtm, &recv_time);
		wcsftime(wtime, 32, L"%Y-%m-%d %H:%M:%S", &tmtm);
		LOG(L"param: %s\n", wtime);
#endif
		time_t now = time(NULL);
		auto iter = _ademcoEventFilter.begin();
		while (iter != _ademcoEventFilter.end()) {
			AdemcoEvent* oldEvent = *iter;
#ifdef _DEBUG
			localtime_s(&tmtm, &now);
			wcsftime(wtime, 32, L"%Y-%m-%d %H:%M:%S", &tmtm);
			LOG(L"now: %s\n", wtime);
			localtime_s(&tmtm, &oldEvent->_recv_time);
			wcsftime(wtime, 32, L"%Y-%m-%d %H:%M:%S", &tmtm);
			LOG(L"old: %s\n", wtime);
#endif
			if (now - oldEvent->_recv_time >= 6) {
				delete oldEvent;
				_ademcoEventFilter.erase(iter);
				iter = _ademcoEventFilter.begin();
				continue;
			} else if (oldEvent->operator== (*ademcoEvent)) {
				delete oldEvent;
				_ademcoEventFilter.erase(iter);
				_ademcoEventFilter.push_back(ademcoEvent);
				_lock4AdemcoEventList.UnLock();
				return;
			}
			iter++;
		}
		_ademcoEventFilter.push_back(new AdemcoEvent(resource, ademco_event, zone, subzone, timestamp, recv_time, xdata, xdata_len));
	}

	if (_buffer_mode) {
		_ademcoEventList.push_back(ademcoEvent);
	} else {
		HandleAdemcoEvent(ademcoEvent);
	}
	_lock4AdemcoEventList.UnLock();

}


//void CAlarmMachine::set_device_id(const wchar_t* device_id)
//{
//	wcscpy_s(_device_idW, device_id);
//	USES_CONVERSION;
//	strcpy_s(_device_id, W2A(_device_idW));
//}
//
//
//void CAlarmMachine::set_device_id(const char* device_id)
//{
//	strcpy_s(_device_id, device_id);
//	USES_CONVERSION;
//	wcscpy_s(_device_idW, A2W(device_id));
//}


bool CAlarmMachine::execute_set_banned(bool banned)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update AlarmMachine set banned=%d where id=%d and ademco_id=%d",
				 banned, _id, _ademco_id);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		_banned = banned;
		return true;
	}

	return false;
}

bool CAlarmMachine::execute_set_has_video(bool has)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update AlarmMachine set has_video=%d where id=%d and ademco_id=%d",
				 has, _id, _ademco_id);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		_has_video = has;
		return true;
	}

	return false;
}


bool CAlarmMachine::execute_set_armd(bool arm)
{
	AUTO_LOG_FUNCTION;
	CString query;
	if (_is_submachine) {
		query.Format(L"update SubMachine set armed=%d where id=%d", arm, _id);
	} else {
		query.Format(L"update AlarmMachine set armed=%d where id=%d", arm, _id);
	}
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		_armed = arm;
		return true;
	}

	return false;
}


bool CAlarmMachine::execute_set_machine_type(MachineType type)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update AlarmMachine set machine_type=%d where id=%d and ademco_id=%d",
				 type, _id, _ademco_id);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		_machine_type = type;
		return true;
	}

	return false;
}


bool CAlarmMachine::execute_set_alias(const wchar_t* alias)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update AlarmMachine set alias='%s' where id=%d and ademco_id=%d",
				 alias, _id, _ademco_id);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		set_alias(alias);
		static AdemcoEvent ademcoEvent(ES_UNKNOWN, EVENT_MACHINE_ALIAS, 0, 0, time(NULL), time(NULL), NULL, 0);
		NotifyObservers(&ademcoEvent);
		return true;
	}

	return false;
}


bool CAlarmMachine::execute_set_contact(const wchar_t* contact)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update AlarmMachine set contact='%s' where id=%d and ademco_id=%d",
				 contact, _id, _ademco_id);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		set_contact(contact);
		return true;
	}

	return false;
}


bool CAlarmMachine::execute_set_address(const wchar_t* address)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update AlarmMachine set address='%s' where id=%d and ademco_id=%d",
				 address, _id, _ademco_id);

	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		set_address(address);
		return true;
	}

	return false;
}


bool CAlarmMachine::execute_set_phone(const wchar_t* phone)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update AlarmMachine set phone='%s' where id=%d and ademco_id=%d",
				 phone, _id, _ademco_id);

	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		set_phone(phone);
		return true;
	}

	return false;
}


bool CAlarmMachine::execute_set_phone_bk(const wchar_t* phone_bk)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update AlarmMachine set phone_bk='%s' where id=%d and ademco_id=%d",
				 phone_bk, _id, _ademco_id);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		set_phone_bk(phone_bk);
		return true;
	}

	return false;
}


bool CAlarmMachine::execute_set_group_id(int group_id)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update AlarmMachine set group_id=%d where id=%d and ademco_id=%d",
				 group_id, _id, _ademco_id);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		CGroupManager* mgr = CGroupManager::GetInstance();
		CGroupInfo* old_group = mgr->GetGroupInfo(_group_id);
		CGroupInfo* new_group = mgr->GetGroupInfo(group_id);
		old_group->RemoveChildMachine(this);
		set_group_id(group_id);
		new_group->AddChildMachine(this);
		return true;
	}

	return false;
}


bool CAlarmMachine::execute_add_zone(CZoneInfo* zoneInfo)
{
	CString query;
	if (_is_submachine) {
		zoneInfo->set_sub_machine_id(_id);
		query.Format(L"insert into SubZone ([sub_zone],[sub_machine_id],[alias],[detector_info_id]) values(%d,%d,'%s',%d)",
					 zoneInfo->get_sub_zone(), _id, zoneInfo->get_alias(), 
					 zoneInfo->get_detector_id());
	} else {
		query.Format(L"insert into ZoneInfo ([ademco_id],[zone_value],[type],[detector_info_id],[sub_machine_id],[alias],[status_or_property],[physical_addr]) values(%d,%d,%d,%d,%d,'%s',%d,%d)",
					 _ademco_id, zoneInfo->get_zone_value(),
					 zoneInfo->get_type(), zoneInfo->get_detector_id(), -1,
					 zoneInfo->get_alias(), zoneInfo->get_status_or_property(), 
					 zoneInfo->get_physical_addr());
	}
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	int id = mgr->AddAutoIndexTableReturnID(query);
	if (-1 != id) {
		zoneInfo->set_id(id);
		zoneInfo->set_ademco_id(_ademco_id);
		if (wcslen(zoneInfo->get_alias()) == 0) {
			CString null;
			null.LoadStringW(IDS_STRING_NULL);
			zoneInfo->set_alias(null);
		}
		AddZone(zoneInfo);
		return true;
	}

	return false;
}


bool CAlarmMachine::execute_del_zone(CZoneInfo* zoneInfo)
{
	AUTO_LOG_FUNCTION;
	CString query;
	if (_is_submachine) {
		query.Format(L"delete from SubZone where id=%d", zoneInfo->get_id());
	} else {
		query.Format(L"delete from ZoneInfo where id=%d", zoneInfo->get_id());
	}
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		CDetectorInfo* detInfo = zoneInfo->GetDetectorInfo();
		if (detInfo) {
			query.Format(L"delete from DetectorInfo where id=%d", detInfo->get_id());
			VERIFY(mgr->ExecuteSql(query));
		}

		CMapInfo* mapInfo = zoneInfo->GetMapInfo();
		if (mapInfo) {
			mapInfo->RemoveZone(zoneInfo);
		}

		_validZoneList.remove(zoneInfo);

		if (_is_submachine) {
			_zoneArray[zoneInfo->get_sub_zone()] = NULL;
		} else {
			_zoneArray[zoneInfo->get_zone_value()] = NULL;
		}

		SAFEDELETEP(zoneInfo);
		return true;
	}

	return false;
}


void CAlarmMachine::GetAllZoneInfo(CZoneInfoList& list)
{
	std::copy(_validZoneList.begin(), _validZoneList.end(), std::back_inserter(list));
}


void CAlarmMachine::GetAllMapInfo(CMapInfoList& list)
{
	std::copy(_mapList.begin(), _mapList.end(), std::back_inserter(list));
}


void CAlarmMachine::AddZone(CZoneInfo* zoneInfo)
{
	assert(zoneInfo);
	int zone = zoneInfo->get_zone_value();
	if (ZT_SUB_MACHINE_ZONE == zoneInfo->get_type()) {
		zone = zoneInfo->get_sub_zone();
	} else if (ZT_SUB_MACHINE == zoneInfo->get_type()) {
		inc_submachine_count();
	}
	if (0 <= zone && zone < MAX_MACHINE_ZONE) {
		_zoneArray[zone] = zoneInfo;
		_validZoneList.push_back(zoneInfo);

		CDetectorInfo* detector = zoneInfo->GetDetectorInfo();
		if (detector) {
			int map_id = detector->get_map_id();
			CMapInfo* mapInfo = GetMapInfo(map_id);
			if (mapInfo) {
				mapInfo->AddZone(zoneInfo);
				zoneInfo->SetMapInfo(mapInfo);
			}
		} else {
			_unbindZoneMap->AddZone(zoneInfo);
			zoneInfo->SetMapInfo(_unbindZoneMap);
		}

	} else {
		ASSERT(0);
	}
}


CZoneInfo* CAlarmMachine::GetZone(int zone)
{
	if (0 <= zone && zone < MAX_MACHINE_ZONE) {
		return _zoneArray[zone];
	} 

	return NULL;
}


bool CAlarmMachine::execute_add_map(CMapInfo* mapInfo)
{
	MapType mt = _is_submachine ? MAP_SUB_MACHINE : MAP_MACHINE;
	mapInfo->set_type(mt);
	mapInfo->set_machine_id(_is_submachine ? _id : _ademco_id);

	CString query;
	query.Format(L"insert into MapInfo ([type],[machine_id],[alias],[path]) values(%d,%d,'%s','%s')",
				 mt, mapInfo->get_machine_id(), mapInfo->get_alias(),
				 mapInfo->get_path());
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	int id = mgr->AddAutoIndexTableReturnID(query);
	if (-1 != id) {
		mapInfo->set_id(id);
		AddMap(mapInfo);
		return true;
	} else {
		ASSERT(0); LOG(L"add map failed.\n"); 
		return false;
	}
}


bool CAlarmMachine::execute_update_map_alias(CMapInfo* mapInfo, const wchar_t* alias)
{
	AUTO_LOG_FUNCTION;
	ASSERT(mapInfo);
	CString query;
	query.Format(L"update MapInfo set alias='%s' where id=%d", alias, mapInfo->get_id());
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (mgr->ExecuteSql(query)) {
		mapInfo->set_alias(alias);
		return true;
	} else {
		ASSERT(0); LOG(L"update map alias failed.\n");
		return false;
	}
}


bool CAlarmMachine::execute_update_map_path(CMapInfo* mapInfo, const wchar_t* path)
{
	AUTO_LOG_FUNCTION;
	ASSERT(mapInfo);
	CString query;
	query.Format(L"update MapInfo set path='%s' where id=%d", path, mapInfo->get_id());
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (mgr->ExecuteSql(query)) {
		mapInfo->set_path(path);
		return true;
	} else {
		ASSERT(0); LOG(L"update map alias failed.\n");
		return false;
	}
}


bool CAlarmMachine::execute_delete_map(CMapInfo* mapInfo)
{
	AUTO_LOG_FUNCTION;
	ASSERT(mapInfo && (-1 != mapInfo->get_id()));
	CString query;
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	do {
		query.Format(L"delete from MapInfo where id=%d", mapInfo->get_id());
		if (!mgr->ExecuteSql(query)) {
			LOG(L"delete map failed.\n"); break;
		}

		query.Format(L"update DetectorInfo set map_id=-1 where map_id=%d",
					 mapInfo->get_id());
		if (!mgr->ExecuteSql(query)) {
			LOG(L"update DetectorInfo failed.\n"); break;
		}

		CZoneInfoList list;
		mapInfo->GetAllZoneInfo(list);
		for (auto zoneInfo : list) {
			_unbindZoneMap->AddZone(zoneInfo);
			zoneInfo->SetMapInfo(_unbindZoneMap);
		}

		_mapList.remove(mapInfo);
		SAFEDELETEP(mapInfo);
		return true;
	} while (0);
	return false;
}


bool CAlarmMachine::execute_update_expire_time(const COleDateTime& datetime)
{
	AUTO_LOG_FUNCTION;
	CString query;
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	do {
		if (_is_submachine) {
			query.Format(L"update SubMachine set expire_time='%s' where id=%d",
						 datetime.Format(L"%Y-%m-%d %H:%M:%S"), _id);
		} else {
			query.Format(L"update AlarmMachine set expire_time='%s' where id=%d",
						 datetime.Format(L"%Y-%m-%d %H:%M:%S"), _id);
		}
		if (!mgr->ExecuteSql(query)) {
			LOG(L"update expire_time failed.\n"); break;
		}

		_expire_time = datetime;
		return true;
	} while (0);
	return false;
}


bool CAlarmMachine::execute_set_coor(const web::BaiduCoordinate& coor)
{
	AUTO_LOG_FUNCTION;
	CString query;
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	do {
		if (_is_submachine) {
			query.Format(L"update SubMachine set baidu_x=%f,baidu_y=%f where id=%d",
						 coor.x, coor.y, _id);
		} else {
			query.Format(L"update AlarmMachine set baidu_x=%f,baidu_y=%f where id=%d",
						 coor.x, coor.y, _id);
		}
		if (!mgr->ExecuteSql(query)) {
			LOG(L"update baidu coor failed.\n"); break;
		}

		_coor = coor;
		return true;
	} while (0);
	return false;
}


void CAlarmMachine::inc_submachine_count()
{ 
	AUTO_LOG_FUNCTION;
	_submachine_count++;
	static AdemcoEvent ademcoEvent(ES_UNKNOWN, EVENT_SUBMACHINECNT, 0, 0, time(NULL), time(NULL), NULL, 0);
	NotifyObservers(&ademcoEvent);
}


void CAlarmMachine::dec_submachine_count()
{ 
	//AUTO_LOG_FUNCTION;
	_submachine_count--;
	static AdemcoEvent ademcoEvent(ES_UNKNOWN, EVENT_SUBMACHINECNT, 0, 0, time(NULL), time(NULL), NULL, 0);
	NotifyObservers(&ademcoEvent);
}


void CAlarmMachine::inc_alarmingSubMachineCount()
{
	_alarmingSubMachineCount++;
}


void CAlarmMachine::dec_alarmingSubMachineCount()
{
	if (_alarmingSubMachineCount == 0)
		return;

	if (--_alarmingSubMachineCount == 0 && !_has_alarming_direct_zone) {
		clear_ademco_event_list();
	}
}


void CAlarmMachine::set_highestEventLevel(EventLevel level)
{
	if (level > _highestEventLevel)
		_highestEventLevel = level;
}




NAMESPACE_END
