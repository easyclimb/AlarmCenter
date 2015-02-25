#include "stdafx.h"
#include "AlarmMachine.h"
#include "ZoneInfo.h"
#include "MapInfo.h"
#include "ademco_event.h"
#include "resource.h"
#include "HistoryRecord.h"
#include "ZonePropertyInfo.h"
#include "AppResource.h"
#include "UserInfo.h"
#include "AlarmMachineManager.h"

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
	, _noZoneMap(NULL)
{
	memset(_device_id, 0, sizeof(_device_id));
	memset(_device_idW, 0, sizeof(_device_idW));
	_alias = new wchar_t[1];
	_alias[0] = 0;
}


CAlarmMachine::~CAlarmMachine()
{
	DESTROY_OBSERVER;

	if (_alias) { delete[] _alias; }
	if (_contact) { delete[] _contact; }
	if (_address) { delete[] _address; }
	if (_phone) { delete[] _phone; }
	if (_phone_bk) { delete[] _phone_bk; }
	if (_noZoneMap) { delete _noZoneMap; }

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

	AdemcoEvent* ademcoEvent = new AdemcoEvent(0, EVENT_CLEARMSG, time(NULL)); // default 0
	NotifyObservers(ademcoEvent);
	delete ademcoEvent;

	// add a record
	CString srecord, suser, sfm, sop;
	suser.LoadStringW(IDS_STRING_USER);
	sfm.LoadStringW(IDS_STRING_LOCAL_OP);
	sop.LoadStringW(IDS_STRING_CLR_MSG);
			
	CUserInfo* user = CUserManager::GetInstance()->GetCurUserInfo();
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

CZoneInfo* CAlarmMachine::GetZoneInfo(int zone_id)
{
	CZoneInfo* zone = NULL;
	do {
		if (_noZoneMap) {
			zone = _noZoneMap->GetZoneInfo(zone_id);
			if (zone)
				return zone;
		}

		std::list<CMapInfo*>::iterator map_iter = _mapList.begin();
		while (map_iter != _mapList.end()) {
			CMapInfo* map = *map_iter++;
			zone = map->GetZoneInfo(zone_id);
			if (zone)
				return zone;
		}

	} while (0);

	return NULL;
}


void CAlarmMachine::HandleAdemcoEvent(ademco::AdemcoEvent* ademcoEvent)
{
	LOG_FUNCTION_AUTO;
	bool online = (ademcoEvent->_event == MS_OFFLINE) ? false : true;
	if (_online != online) {
		_online = online;
	}

	CString fmEvent;
	BOOL bMachineStatus = TRUE;
	// machine status
	switch (ademcoEvent->_event) {
		case MS_OFFLINE:
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
		{
				   bMachineStatus = FALSE;

				   CWinApp* app = AfxGetApp(); ASSERT(app);
				   CWnd* wnd = app->GetMainWnd(); ASSERT(wnd);
				   wnd->SendMessage(WM_ADEMCOEVENT, (WPARAM)this, 1);

				   CString text, alarmText;
				   // text.LoadStringW(IDS_STRING_MACHINE);

				   text = _alias;

				   if (ademcoEvent->_zone != 0) {
					   CString fmZone, prefix;
					   fmZone.LoadStringW(IDS_STRING_ZONE);
					   prefix.Format(L" %s%03d", fmZone, ademcoEvent->_zone);
					   text += prefix;
				   }

				   CString strEvent = L"";
				   CString alias = L"";
				   CZoneInfo* zoneInfo = GetZoneInfo(ademcoEvent->_zone);
				   CZonePropertyData* data = NULL;
				   if (zoneInfo) {
					   CZonePropertyInfo* info = CZonePropertyInfo::GetInstance();
					   data = info->GetZonePropertyData(zoneInfo->get_detector_property_id());
					   alias = zoneInfo->get_alias();
				   }

				   if (alias.IsEmpty()) {
					   CString fmNull;
					   fmNull.LoadStringW(IDS_STRING_NULL);
					   alias = fmNull;
				   }

				   if (ademco::IsExceptionEvent(ademcoEvent->_event) || (data == NULL)) { // �쳣��Ϣ������ event ��ʾ����
					   CAppResource* res = CAppResource::GetInstance();
					   CString strEvent = res->AdemcoEventToString(ademcoEvent->_event);
					   alarmText.Format(L"%s(%s)", strEvent, alias);
				   } else { // ������Ϣ������ �ֶ����õı������� �� event ��ʾ����
					   alarmText.Format(L"%s(%s)", data->get_alarm_text(), alias);
				   }

				   text += L" " + alarmText;

				   CHistoryRecord *hr = CHistoryRecord::GetInstance();
				   hr->InsertRecord(get_ademco_id(), text, ademcoEvent->_time,
									RECORD_LEVEL_ALARM);
		}
			break;
	}

	if (bMachineStatus) {
		CString record, fmMachine;
		fmMachine.LoadStringW(IDS_STRING_MACHINE);
		record.Format(L"%s%04d(%s) %s", fmMachine, get_ademco_id(), get_alias(), fmEvent);
		CHistoryRecord::GetInstance()->InsertRecord(get_ademco_id(), record, 
													ademcoEvent->_time,
													RECORD_LEVEL_ONOFFLINE);
	}

	NotifyObservers(ademcoEvent);
}


void CAlarmMachine::SetAdemcoEvent(int zone, int ademco_event, const time_t& event_time)
{
	LOG_FUNCTION_AUTO;

	_lock4AdemcoEventList.Lock();
	AdemcoEvent* ademcoEvent = new AdemcoEvent(zone, ademco_event, event_time);
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
	query.Format(L"update AlarmMachine set Banned=%d where id=%d and AdemcoID=%d",
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
	query.Format(L"update AlarmMachine set MachineType=%d where id=%d and AdemcoID=%d",
				 mt, _id, _ademco_id);

	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		_type = mt;
		return true;
	}

	return false;
}

NAMESPACE_END
