#include "stdafx.h"
#include "AlarmMachine.h"
#include "ZoneInfo.h"
#include "DetectorInfo.h"
#include "MapInfo.h"
#include "ademco_event.h"
#include "resource.h"
#include "HistoryRecord.h"
#include "ZonePropertyInfo.h"
#include "AppResource.h"
#include "UserInfo.h"
#include "AlarmMachineManager.h"
#include "GroupInfo.h"

#include <algorithm>

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
	, _buffer_mode(false)
	, _unbindZoneMap(NULL)
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

	for (int i = 0; i < MAX_MACHINE_ZONE; i++) {
		CZoneInfo* zone = _zoneArray[i];
		SAFEDELETEP(zone);
	}
}


void CAlarmMachine::clear_ademco_event_list()
{
	_lock4AdemcoEventList.Lock();

	CWinApp* app = AfxGetApp(); ASSERT(app);
	CWnd* wnd = app->GetMainWnd(); ASSERT(wnd);
	wnd->SendMessage(WM_ADEMCOEVENT, (WPARAM)this, 0);

	std::list<AdemcoEvent*>::iterator iter = _ademcoEventList.begin();
	while (iter != _ademcoEventList.end()) {
		AdemcoEvent* ademcoEvent = *iter++;
		delete ademcoEvent;
	}
	_ademcoEventList.clear();

	AdemcoEvent* ademcoEvent = new AdemcoEvent(0, 0, EVENT_CLEARMSG, time(NULL)); // default 0
	NotifyObservers(ademcoEvent);
	delete ademcoEvent;

	// add a record
	CString srecord, suser, sfm, sop;
	suser.LoadStringW(IDS_STRING_USER);
	sfm.LoadStringW(IDS_STRING_LOCAL_OP);
	sop.LoadStringW(IDS_STRING_CLR_MSG);
			
	const CUserInfo* user = CUserManager::GetInstance()->GetCurUserInfo();
	srecord.Format(L"%s(ID:%d,%s)%s:%s(%04d:%s)", suser,
				   user->get_user_id(), user->get_user_name(),
				   sfm, sop, get_ademco_id(), get_alias());
	CHistoryRecord::GetInstance()->InsertRecord(get_ademco_id(),
												srecord, time(NULL),
												RECORD_LEVEL_USERCONTROL);
	_lock4AdemcoEventList.UnLock();
}


void CAlarmMachine::EnterBufferMode() 
{ 
	LOG_FUNCTION_AUTO;
	_lock4AdemcoEventList.Lock(); 
	_buffer_mode = true; 
	_lock4AdemcoEventList.UnLock();
}


void CAlarmMachine::LeaveBufferMode() 
{
	LOG_FUNCTION_AUTO;
	_lock4AdemcoEventList.Lock();

	_buffer_mode = false; 

	std::list<AdemcoEvent*>::iterator iter = _ademcoEventList.begin();
	while (iter != _ademcoEventList.end()) {
		AdemcoEvent* ademcoEvent = *iter++;
		HandleAdemcoEvent(ademcoEvent);
	}

	_lock4AdemcoEventList.UnLock(); 
}


void CAlarmMachine::TraverseAdmecoEventList(void* udata, AdemcoEventCB cb)
{
	LOG_FUNCTION_AUTO;
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
//
//CZoneInfo* CAlarmMachine::GetZoneInfo(int zone)
//{
//	/*CZoneInfo* zone = NULL;
//	do {
//		if (_unbindZoneMap) {
//			zone = _unbindZoneMap->GetZoneInfo(zone_id);
//			if (zone)
//				return zone;
//		}
//
//		std::list<CMapInfo*>::iterator map_iter = _mapList.begin();
//		while (map_iter != _mapList.end()) {
//			CMapInfo* map = *map_iter++;
//			zone = map->GetZoneInfo(zone_id);
//			if (zone)
//				return zone;
//		}
//
//	} while (0);*/
//
//	if (0 <= zone && zone < MAX_MACHINE_ZONE) {
//		return _zoneArray[zone];
//	}
//
//	return NULL;
//}


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


void CAlarmMachine::HandleAdemcoEvent(const ademco::AdemcoEvent* ademcoEvent)
{
	LOG_FUNCTION_AUTO;

	// 主机事件
	if (ademcoEvent->_sub_zone == INDEX_ZONE) {	
		// 主机状态事件
		if (ademcoEvent->_zone == 0) {	
			_online = true;

			bool bMachineStatus = true;;
			CString fmEvent;
			switch (ademcoEvent->_event) {
				case MS_OFFLINE:
					_online = false;
					fmEvent.LoadStringW(IDS_STRING_OFFLINE);
					break;
				case MS_ONLINE:
					fmEvent.LoadStringW(IDS_STRING_ONLINE);
					break;
				case ademco::EVENT_DISARM:
					_armed = false;
					fmEvent.LoadStringW(IDS_STRING_DISARM);
					break;
				case ademco::EVENT_ARM:
					_armed = true;
					fmEvent.LoadStringW(IDS_STRING_ARM);
					break;
				default:
					bMachineStatus = false;
					break;
			}

			if (bMachineStatus) {
				CString record, fmMachine;
				fmMachine.LoadStringW(IDS_STRING_MACHINE);
				record.Format(L"%s%04d(%s) %s", fmMachine, get_ademco_id(), 
							  get_alias(), fmEvent);
				CHistoryRecord::GetInstance()->InsertRecord(get_ademco_id(), record,
															ademcoEvent->_time,
															RECORD_LEVEL_ONOFFLINE);
			}
		// 主机防区事件或主机报警事件
		} else {						
			CWinApp* app = AfxGetApp(); ASSERT(app);
			CWnd* wnd = app->GetMainWnd(); ASSERT(wnd);
			wnd->SendMessage(WM_ADEMCOEVENT, (WPARAM)this, 1);

			CZoneInfo* zone = GetZone(ademcoEvent->_zone);
			if (zone) {
				zone->HandleAdemcoEvent(ademcoEvent);
			} else {
				
			}
		}
		
		//CString text, alarmText;
		//// text.LoadStringW(IDS_STRING_MACHINE);
		//text = _alias;
		//if (ademcoEvent->_zone != 0) {
		//	CString fmZone, prefix;
		//	fmZone.LoadStringW(IDS_STRING_ZONE);
		//	prefix.Format(L" %s%03d", fmZone, ademcoEvent->_zone);
		//	text += prefix;
		//}
		//CString strEvent = L"";
		//CString alias = L"";
		//CZoneInfo* zoneInfo = GetZoneInfo(ademcoEvent->_zone);
		//CZonePropertyData* data = NULL;
		//if (zoneInfo) {
		//	CZonePropertyInfo* info = CZonePropertyInfo::GetInstance();
		//	data = info->GetZonePropertyDataById(zoneInfo->get_property_id());
		//	alias = zoneInfo->get_alias();
		//}
		//if (alias.IsEmpty()) {
		//	CString fmNull;
		//	fmNull.LoadStringW(IDS_STRING_NULL);
		//	alias = fmNull;
		//}
		//if (ademco::IsExceptionEvent(ademcoEvent->_event) || (data == NULL)) { // 异常信息，按照 event 显示文字
		//	CAppResource* res = CAppResource::GetInstance();
		//	CString strEvent = res->AdemcoEventToString(ademcoEvent->_event);
		//	alarmText.Format(L"%s(%s)", strEvent, alias);
		//} else { // 报警信息，按照 手动设置的报警文字 或 event 显示文字
		//	alarmText.Format(L"%s(%s)", data->get_alarm_text(), alias);
		//}
		//text += L" " + alarmText;
		//CHistoryRecord *hr = CHistoryRecord::GetInstance();
		//hr->InsertRecord(get_ademco_id(), text, ademcoEvent->_time,
		//				RECORD_LEVEL_ALARM);



	// 分机状态事件
	} else if (ademcoEvent->_sub_zone == INDEX_SUB_MACHINE) {
	
	// 分机分防区事件
	} else {
		
	}

	NotifyObservers(ademcoEvent);
}


void CAlarmMachine::SetAdemcoEvent(int zone, int subzone, int ademco_event, const time_t& event_time)
{
	LOG_FUNCTION_AUTO;

	_lock4AdemcoEventList.Lock();
	AdemcoEvent* ademcoEvent = new AdemcoEvent(zone, subzone, ademco_event, event_time);
	_ademcoEventList.push_back(ademcoEvent);
	if (!_buffer_mode) {
		HandleAdemcoEvent(ademcoEvent);
	}
	_lock4AdemcoEventList.UnLock();

}


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


CMapInfo* CAlarmMachine::GetFirstMap()
{
	if (_mapList.size() == 0)
		return NULL;
	_curMapListIter = _mapList.begin();
	return *_curMapListIter++;
}


CMapInfo* CAlarmMachine::GetNextMap()
{
	if (_mapList.size() == 0)
		return NULL;
	if (_curMapListIter == _mapList.end()) 
		return NULL;
	return *_curMapListIter++;
}


bool CAlarmMachine::execute_set_banned(bool banned)
{
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


bool CAlarmMachine::execute_set_type(int type)
{
	MachineType mt = Integer2MachineType(type);
	if (mt >= MT_MAX)
		return false;

	CString query;
	query.Format(L"update AlarmMachine set machine_type=%d where id=%d and ademco_id=%d",
				 mt, _id, _ademco_id);

	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		_type = mt;
		return true;
	}

	return false;
}


bool CAlarmMachine::execute_set_alias(const wchar_t* alias)
{
	CString query;
	query.Format(L"update AlarmMachine set alias='%s' where id=%d and ademco_id=%d",
				 alias, _id, _ademco_id);

	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		set_alias(alias);
		return true;
	}

	return false;
}


bool CAlarmMachine::execute_set_contact(const wchar_t* contact)
{
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


//bool myfunc(CZoneInfo* a, CZoneInfo* b)
//{
//	return a->get_zone_value() < b->get_zone_value();
//}


void CAlarmMachine::GetAllZoneInfo(CZoneInfoList& list)
{
	/*std::list<CMapInfo*>::iterator map_iter = _mapList.begin();
	while (map_iter != _mapList.end()) {
		CMapInfo* map = *map_iter++;
		CZoneInfo* zone = map->GetFirstZoneInfo();
		while (zone) {
			list.push_back(zone);
			zone = map->GetNextZoneInfo();
		}
	}

	if (_unbindZoneMap) {
		CZoneInfo* zone = _unbindZoneMap->GetFirstZoneInfo();
		while (zone) {
			list.push_back(zone);
			zone = _unbindZoneMap->GetNextZoneInfo();
		}
	}*/

	//std::sort(list.begin(), list.end(), myfunc);
	//list.sort(myfunc);

	for (int i = 0; i < MAX_MACHINE_ZONE; i++) {
		CZoneInfo* zone = _zoneArray[i];
		if (zone) {
			list.push_back(zone);
		}
	}
}


void CAlarmMachine::AddZone(CZoneInfo* zoneInfo)
{
	assert(zoneInfo);
	int zone = zoneInfo->get_zone_value();
	if (0 <= zone && zone < MAX_MACHINE_ZONE) {
		_zoneArray[zone] = zoneInfo;

		CDetectorInfo* detector = zoneInfo->GetDetectorInfo();
		if (detector) {
			int map_id = detector->get_map_id();
			CMapInfo* mapInfo = GetMapInfo(map_id);
			if (mapInfo)
				mapInfo->AddZone(zoneInfo);
		} else {
			_unbindZoneMap->AddZone(zoneInfo);
		}

	} else {
		ASSERT(0);
	}
}


CZoneInfo* CAlarmMachine::GetZone(int zone)
{
	if (0 <= zone && zone < MAX_MACHINE_ZONE) {
		return _zoneArray[zone];
	} else {
		ASSERT(0);
	}

	return NULL;
}
NAMESPACE_END
