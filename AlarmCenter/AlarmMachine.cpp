#include "stdafx.h"
#include "AlarmMachine.h"
#include "ZoneInfo.h"
#include "DetectorInfo.h"
#include "MapInfo.h"
#include "ademco_event.h"
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

using namespace ademco;
namespace core {

IMPLEMENT_OBSERVER(CAlarmMachine)
	
CAlarmMachine::CAlarmMachine()
	: _id(0)
	, _ademco_id(0)
	, _group_id(0)
	, _alias(NULL)
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
{
	memset(_device_id, 0, sizeof(_device_id));
	memset(_device_idW, 0, sizeof(_device_idW));
	_alias = new wchar_t[1];
	_alias[0] = 0;

	memset(_zoneArray, 0, sizeof(_zoneArray));

	_unbindZoneMap = new CMapInfo();
	_unbindZoneMap->set_id(-1);
	CString fmAlias;
	fmAlias.LoadStringW(IDS_STRING_NOZONEMAP);
	_unbindZoneMap->set_alias(fmAlias);
}


CAlarmMachine::~CAlarmMachine()
{
	AdemcoEvent ademcoEvent(EVENT_IM_GONNA_DIE, 0, 0, time(NULL), NULL, 0);
	NotifyObservers(&ademcoEvent);
	DESTROY_OBSERVER;

	if (_alias) { delete[] _alias; }
	if (_contact) { delete[] _contact; }
	if (_address) { delete[] _address; }
	if (_phone) { delete[] _phone; }
	if (_phone_bk) { delete[] _phone_bk; }
	if (_unbindZoneMap) { delete _unbindZoneMap; }

	std::list<CMapInfo*>::iterator map_iter = _mapList.begin();
	while (map_iter != _mapList.end()) {
		CMapInfo* map = *map_iter++;
		delete map;
	}
	_mapList.clear();

	std::list<AdemcoEvent*>::iterator iter = _ademcoEventList.begin();
	while (iter != _ademcoEventList.end()) {
		AdemcoEvent* ademcoEvent = *iter++;
		delete ademcoEvent;
	}
	_ademcoEventList.clear();

	iter = _ademcoEventFilter.begin();
	while (iter != _ademcoEventFilter.end()) {
		AdemcoEvent* ademcoEvent = *iter++;
		delete ademcoEvent;
	}

	for (int i = 0; i < MAX_MACHINE_ZONE; i++) {
		CZoneInfo* zone = _zoneArray[i];
		SAFEDELETEP(zone);
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
		wnd->SendMessage(WM_ADEMCOEVENT, (WPARAM)this, 0);
	}

	std::list<AdemcoEvent*>::iterator iter = _ademcoEventList.begin();
	while (iter != _ademcoEventList.end()) {
		AdemcoEvent* ademcoEvent = *iter++;
		delete ademcoEvent;
	}
	_ademcoEventList.clear();

	AdemcoEvent* ademcoEvent = new AdemcoEvent(EVENT_CLEARMSG, 0, 0, time(NULL), NULL, 0); // default 0
	NotifyObservers(ademcoEvent);
	if (_unbindZoneMap) {
		_unbindZoneMap->InversionControl(ICMC_CLR_ALARM_TEXT);
	}
	std::list<PZone>::iterator zoneIter = _validZoneList.begin();
	while (zoneIter != _validZoneList.end()) {
		CZoneInfo* zoneInfo = *zoneIter++;
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
		std::list<AdemcoEvent*>::iterator iter = _ademcoEventList.begin();
		while (iter != _ademcoEventList.end()) {
			AdemcoEvent* ademcoEvent = *iter++;
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
	std::list<AdemcoEvent*>::iterator iter = _ademcoEventList.begin();
	while (iter != _ademcoEventList.end()) {
		AdemcoEvent* ademcoEvent = *iter++;
		if (udata && cb) {
			cb(udata, ademcoEvent);
		}
	}
	_lock4AdemcoEventList.UnLock();
}


CMapInfo* CAlarmMachine::GetMapInfo(int map_id)
{
	std::list<CMapInfo*>::iterator iter = _mapList.begin();
	while (iter != _mapList.end()) {
		CMapInfo* mapInfo = *iter++;
		if (mapInfo->get_id() == map_id)
			return mapInfo;
	}
	return NULL;
}


void CAlarmMachine::HandleAdemcoEvent(const ademco::AdemcoEvent* ademcoEvent, 
									  BOOL bDeleteAfterHandled)
{
	AUTO_LOG_FUNCTION;
	if (!_is_submachine) {
#pragma region define val
		bool bMachineStatus = false;
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
				_connHangupObj.reset();
				bMachineStatus = true; online = false; fmEvent.LoadStringW(IDS_STRING_OFFLINE);
				CSoundPlayer::GetInstance()->Play(CSoundPlayer::SI_OFFLINE); 
				break;
			case ademco::EVENT_ONLINE: bMachineStatus = true; fmEvent.LoadStringW(IDS_STRING_ONLINE);
				break;
			case ademco::EVENT_CONN_HANGUP:
				if (_connHangupObj.valid()) { _connHangupObj.cb(_connHangupObj.udata, true); }
				record.Format(L"%s%04d(%s) %s", fmMachine, _ademco_id, _alias, fmHangup);
				CHistoryRecord::GetInstance()->InsertRecord(_ademco_id, -1, record, 
															ademcoEvent->_time, 
															RECORD_LEVEL_ONOFFLINE);
				delete ademcoEvent;
				return;
				break;
			case ademco::EVENT_CONN_RESUME:
				if (_connHangupObj.valid()) { _connHangupObj.cb(_connHangupObj.udata, false); }
				record.Format(L"%s%04d(%s) %s", fmMachine, _ademco_id, _alias, fmResume);
				CHistoryRecord::GetInstance()->InsertRecord(_ademco_id, -1, record,
															ademcoEvent->_time,
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

#pragma region online or armed
		if ((ademcoEvent->_zone == 0) && (ademcoEvent->_sub_zone == INDEX_ZONE)) {
			_online = online;
			_armed = armed;
		}
#pragma endregion

		if (bMachineStatus) {	// 状态事件
#pragma region status event
			if (ademcoEvent->_zone == 0) { // 主机状态
				record.Format(L"%s%04d(%s) %s", fmMachine, _ademco_id, _alias,
							  fmEvent);
			} else { // 分机状态
				record.Format(L"%s%04d(%s) %s%03d(%s) %s",
							  fmMachine, _ademco_id, _alias,
							  fmSubMachine, ademcoEvent->_zone, aliasOfZoneOrSubMachine,
							  fmEvent);
				if (subMachine) {
					subMachine->_online = online;
					subMachine->_armed = armed;
					subMachine->SetAdemcoEvent(ademcoEvent->_event, ademcoEvent->_zone,
											   ademcoEvent->_sub_zone, ademcoEvent->_time,
											   ademcoEvent->_xdata, ademcoEvent->_xdata_len);
				}
			}
			CHistoryRecord::GetInstance()->InsertRecord(get_ademco_id(), ademcoEvent->_zone,
														record, ademcoEvent->_time,
														RECORD_LEVEL_ONOFFLINE);
#pragma endregion
		} else {				// 报警事件
#pragma region alarm event
			_alarming = true;
			// 格式化所需字符串
#pragma region format text
			CString smachine, szone, sevent;
			smachine.Format(L"%s%04d(%s) ", fmMachine, _ademco_id, _alias);

			if (ademcoEvent->_zone != 0) {
				if (ademcoEvent->_sub_zone == INDEX_ZONE) {
					szone.Format(L"%s%03d(%s)", fmZone, ademcoEvent->_zone, aliasOfZoneOrSubMachine);
				} else {
					szone.Format(L"%s%03d(%s)", fmSubMachine, ademcoEvent->_zone, aliasOfZoneOrSubMachine);
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
			sevent.Format(L" %s", res->AdemcoEventToString(ademcoEvent->_event));

			time_t event_time = ademcoEvent->_time;
			wchar_t wtime[32] = { 0 };
			struct tm tmtm;
			localtime_s(&tmtm, &event_time);
			if (event_time == -1) {
				event_time = time(NULL);
				localtime_s(&tmtm, &event_time);
			}
			wcsftime(wtime, 32, L"%H:%M:%S", &tmtm);

			AlarmText* at = new AlarmText();
			at->_zone = ademcoEvent->_zone;
			at->_subzone = ademcoEvent->_sub_zone;
			at->_event = ademcoEvent->_event;
			at->_txt.Format(L"%s %s%s", wtime, szone, sevent);
#pragma endregion
			// 写数据库
#pragma region write history recored
			CHistoryRecord *hr = CHistoryRecord::GetInstance();
			hr->InsertRecord(get_ademco_id(), ademcoEvent->_zone,
							 smachine + szone + sevent,
							 ademcoEvent->_time, RECORD_LEVEL_ALARM);
#pragma endregion

			// 界面响应
			// 1. 增加主界面 触警主机
			CWinApp* app = AfxGetApp(); ASSERT(app);
			CWnd* wnd = app->GetMainWnd(); ASSERT(wnd);
			wnd->SendMessage(WM_ADEMCOEVENT, (WPARAM)this, 1);

			// 2. 区分有无防区信息
			if (zone) {	// 2.1 有防区信息
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
			} else {	// 2.2 无防区信息
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

			set_highestEventLevel(GetEventLevel(ademcoEvent->_event));
#pragma endregion
		}
	}
	NotifyObservers(ademcoEvent);
	if (bDeleteAfterHandled)
		delete ademcoEvent;
}


void CAlarmMachine::HandleRetrieveResult(const ademco::AdemcoEvent* ademcoEvent)
{
	int gg = ademcoEvent->_sub_zone;
	ASSERT(ademcoEvent->_xdata && (ademcoEvent->_xdata_len == 3));
	if (!(ademcoEvent->_xdata && (ademcoEvent->_xdata_len == 3)))
		return;
	char status = ademcoEvent->_xdata[0];
	int addr = MAKEWORD(ademcoEvent->_xdata[2], ademcoEvent->_xdata[1]);

	CZoneInfo* zoneInfo = GetZone(ademcoEvent->_zone);
	if (!zoneInfo) { // 无数据，这是索要操作的回应
		// 交给 “查询所有主机”界面 CRetrieveProgressDlg 处理
		NotifyObservers(ademcoEvent);
	} else { // 已经有数据，这是恢复主机数据的回应
		CAlarmMachine* subMachine = zoneInfo->GetSubMachineInfo();
		if (status != zoneInfo->get_status_or_property()) {
			zoneInfo->execute_set_status_or_property(status);
		}
		if (addr != zoneInfo->get_physical_addr()) {
			zoneInfo->execute_set_physical_addr(addr);
		}

		bool ok = true;
		if ((gg == 0xEE) && (subMachine != NULL)) {
			ADEMCO_EVENT ademco_event = CZoneInfo::char_to_status(status);
			SetAdemcoEvent(ademco_event, zoneInfo->get_zone_value(), 0xEE,
						   time(NULL), NULL, 0);
		} else if ((gg == 0x00) && (subMachine == NULL)) {
			
		} else { ok = false; ASSERT(0); }

		if (ok) { // 交给“恢复主机数据”界面  CRestoreMachineDlg 处理
			NotifyObservers(ademcoEvent);
		}
	}
}


void CAlarmMachine::NotifySubmachines(const ademco::AdemcoEvent* ademcoEvent)
{
	std::list<PZone>::iterator zoneIter = _validZoneList.begin();
	while (zoneIter != _validZoneList.end()) {
		CZoneInfo* zoneInfo = *zoneIter++;
		if (zoneInfo->get_type() == ZT_SUB_MACHINE) {
			CAlarmMachine* subMachine = zoneInfo->GetSubMachineInfo();
			if (subMachine) {
				subMachine->set_machine_type(_machine_type);
				subMachine->HandleAdemcoEvent(ademcoEvent, FALSE);
			}
		}
	}
}


void CAlarmMachine::SetAdemcoEvent(int ademco_event, int zone, int subzone, 
								   const time_t& event_time,
								   const char* xdata, int xdata_len)
{
	AUTO_LOG_FUNCTION;
#ifdef _DEBUG
	wchar_t wtime[32] = { 0 };
	struct tm tmtm;
	localtime_s(&tmtm, &event_time);
	wcsftime(wtime, 32, L"%Y-%m-%d %H:%M:%S", &tmtm);
	LOG(L"param: %s\n", wtime);
#endif
	_lock4AdemcoEventList.Lock();
	time_t now = time(NULL);
	AdemcoEvent* ademcoEvent = new AdemcoEvent(ademco_event, zone, subzone, 
											   event_time, xdata, xdata_len);
	std::list<AdemcoEvent*>::iterator iter = _ademcoEventFilter.begin();
	while (iter != _ademcoEventFilter.end()) {
		AdemcoEvent* oldEvent = *iter;
#ifdef _DEBUG
		localtime_s(&tmtm, &now);
		wcsftime(wtime, 32, L"%Y-%m-%d %H:%M:%S", &tmtm);
		LOG(L"now: %s\n", wtime);
		localtime_s(&tmtm, &oldEvent->_time);
		wcsftime(wtime, 32, L"%Y-%m-%d %H:%M:%S", &tmtm);
		LOG(L"old: %s\n", wtime);
#endif
		if (now - oldEvent->_time >= 6) {
			delete oldEvent;
			_ademcoEventFilter.erase(iter);
			iter = _ademcoEventFilter.begin();
			continue;
		} else if (*oldEvent == *ademcoEvent) {
			delete oldEvent;
			_ademcoEventFilter.erase(iter);
			_ademcoEventFilter.push_back(ademcoEvent);
			_lock4AdemcoEventList.UnLock();
			return;
		}
		iter++;
	}
	_ademcoEventFilter.push_back(new AdemcoEvent(ademco_event, zone, subzone, 
		event_time, xdata, xdata_len));

	if (!_buffer_mode) {
		HandleAdemcoEvent(ademcoEvent);
	} else {
		_ademcoEventList.push_back(ademcoEvent);
	}
	_lock4AdemcoEventList.UnLock();

}

//
//void CAlarmMachine::SetAdemcoEvent(const ademco::AdemcoEvent* ademcoEventParam)
//{
//	AUTO_LOG_FUNCTION;
//
//	_lock4AdemcoEventList.Lock();
//	AdemcoEvent* ademcoEvent = new AdemcoEvent(*ademcoEventParam);
//
//	if (!_buffer_mode) {
//		HandleAdemcoEvent(ademcoEvent);
//	} else {
//		_ademcoEventList.push_back(ademcoEvent);
//	}
//	_lock4AdemcoEventList.UnLock();
//}


void CAlarmMachine::set_device_id(const wchar_t* device_id)
{
	wcscpy_s(_device_idW, device_id);
	USES_CONVERSION;
	strcpy_s(_device_id, W2A(_device_idW));
}


void CAlarmMachine::set_device_id(const char* device_id)
{
	strcpy_s(_device_id, device_id);
	USES_CONVERSION;
	wcscpy_s(_device_idW, A2W(device_id));
}


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
		static AdemcoEvent ademcoEvent(EVENT_MACHINE_ALIAS, 0, 0, time(NULL), NULL, 0);
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
	// 这时只要删除防区信息即可，不需要考虑防区有分机的情况；
	// 即使有分机，也已经在之前的步骤中删除了。
	CString query;
	if (_is_submachine) {
		query.Format(L"delete from SubZone where id=%d", zoneInfo->get_id());
	} else {
		query.Format(L"delete from ZoneInfo where id=%d", zoneInfo->get_id());
	}
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		// 删除探头信息
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
		CZoneInfoListIter iter = list.begin();
		while (iter != list.end()) {
			CZoneInfo* zoneInfo = *iter++;
			_unbindZoneMap->AddZone(zoneInfo);
			zoneInfo->SetMapInfo(_unbindZoneMap);
		}

		_mapList.remove(mapInfo);
		SAFEDELETEP(mapInfo);
		return true;
	} while (0);
	return false;
}


void CAlarmMachine::inc_submachine_count()
{ 
	AUTO_LOG_FUNCTION;
	_submachine_count++;
	static AdemcoEvent ademcoEvent(EVENT_SUBMACHINECNT, 0, 0, time(NULL), NULL, 0);
	NotifyObservers(&ademcoEvent);
}


void CAlarmMachine::dec_submachine_count()
{ 
	//AUTO_LOG_FUNCTION;
	_submachine_count--;
	static AdemcoEvent ademcoEvent(EVENT_SUBMACHINECNT, 0, 0, time(NULL), NULL, 0); 
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
