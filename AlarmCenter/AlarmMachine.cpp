#include "stdafx.h"
#include "AlarmMachine.h"
#include "ZoneInfo.h"
#include "MapInfo.h"
#include "ademco_event.h"
#include "resource.h"
#include "HistoryRecord.h"


using namespace ademco;
namespace core {

IMPLEMENT_OBSERVER(CAlarmMachine)
	
CAlarmMachine::CAlarmMachine()
	: _id(0)
	, _ademco_id(0)
	, _alias(NULL)
	, _online(false)
	, _noZoneMap(NULL)
{
	memset(_device_id, 0, sizeof(_device_id));
	memset(_device_idW, 0, sizeof(_device_idW));
	_alias = new wchar_t[1];
	_alias[0] = 0;
}


CAlarmMachine::~CAlarmMachine()
{
	if (_alias) { delete[] _alias; }
	if (_contact) { delete[] _contact; }
	if (_address) { delete[] _address; }
	if (_phone) { delete[] _phone; }
	if (_phone_bk) { delete[] _phone_bk; }
	if (_noZoneMap) { delete _noZoneMap; }

	/*std::list<CZoneInfo*>::iterator zone_iter = _zoneList.begin();
	while (zone_iter != _zoneList.end()) {
		CZoneInfo* zone = *zone_iter++;
		delete zone;
	}
	_zoneList.clear();*/

	std::list<CMapInfo*>::iterator map_iter = _mapList.begin();
	while (map_iter != _mapList.end()) {
		CMapInfo* map = *map_iter++;
		delete map;
	}
	_mapList.clear();

	/*std::list<AdemcoEventCallbackInfo*>::iterator iter = _observerList.begin();
	while (iter != _observerList.end()) {
		AdemcoEventCallbackInfo* observer = *iter++;
		delete observer;
	}
	_observerList.clear();*/

	DESTROY_OBSERVER;

	clear_ademco_event_list();
}

//
//void CAlarmMachine::TraverseZoneOfMap(int map_id, void* udata, TraverseZoneOfMapCB cb)
//{
//	if (udata == NULL || cb == NULL)
//		return;
//
//	std::list<CZoneInfo*>::iterator zone_iter = _zoneList.begin();
//	while (zone_iter != _zoneList.end()) {
//		CZoneInfo* zone = *zone_iter++;
//		if (zone->get_map_id() == map_id)	
//			cb(udata, zone);
//	}
//}


void CAlarmMachine::clear_ademco_event_list()
{
	_lock4AdemcoEventList.Lock();
	std::list<AdemcoEvent*>::iterator iter = _ademcoEventList.begin();
	while (iter != _ademcoEventList.end()) {
		AdemcoEvent* ademcoEvent = *iter++;
		delete ademcoEvent;
	}
	_ademcoEventList.clear();

	AdemcoEvent* ademcoEvent = new AdemcoEvent(0, EVENT_CLEARMSG, time(NULL)); // default 0
	NotifyObservers(ademcoEvent);
	delete ademcoEvent;

	_lock4AdemcoEventList.UnLock();

}


void CAlarmMachine::TraverseAdmecoEventList(void* udata, AdemcoEventCB cb)
{
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
//void CAlarmMachine::RegisterObserver(void* udata, AdemcoEventCB cb)
//{
//	std::list<AdemcoEventCallbackInfo*>::iterator iter = _observerList.begin();
//	while (iter != _observerList.end()) {
//		AdemcoEventCallbackInfo* observer = *iter;
//		if (observer->_udata == udata) {
//			return;
//		}
//		iter++;
//	}
//	AdemcoEventCallbackInfo *observer = new AdemcoEventCallbackInfo(cb, udata);
//	_observerList.insert(iter, observer);
//}
//
//
//void CAlarmMachine::UnregisterObserver(void* udata)
//{
//	std::list<AdemcoEventCallbackInfo*>::iterator iter = _observerList.begin();
//	while (iter != _observerList.end()) {
//		AdemcoEventCallbackInfo* observer = *iter;
//		if (observer->_udata == udata) {
//			delete observer;
//			_observerList.erase(iter);
//			break;
//		}
//		iter++;
//	}
//}
//
//
//void CAlarmMachine::NotifyObservers(AdemcoEvent* ademcoEvent)
//{
//	std::list<AdemcoEventCallbackInfo*>::iterator iter = _observerList.begin();
//	while (iter != _observerList.end()) {
//		AdemcoEventCallbackInfo* observer = *iter++;
//		observer->_on_result(observer->_udata, ademcoEvent);
//	}
//}


void CAlarmMachine::SetAdemcoEvent(int zone, int ademco_event, const time_t& event_time)
{
	bool online = (ademco_event == MS_OFFLINE) ? false : true;
	if (_online != online) {
		_online = online;
	}

	CString fmEvent;
	BOOL need2record = TRUE;
	// machine status
	switch (ademco_event) {
		case MS_OFFLINE:
			fmEvent.LoadStringW(IDS_STRING_OFFLINE);
			break;
		case MS_ONLINE:
			fmEvent.LoadStringW(IDS_STRING_ONLINE);
			break;
		case ademco::EVENT_DISARM:
			fmEvent.LoadStringW(IDS_STRING_DISARM);
			break;
		case ademco::EVENT_ARM:
			fmEvent.LoadStringW(IDS_STRING_ARM);
			break;
		default:
			need2record = FALSE;
			break;
	}

	if (need2record) {
		CString record, fmMachine;
		fmMachine.LoadStringW(IDS_STRING_MACHINE);
		record.Format(L"%s%04d(%s) %s", fmMachine, get_ademco_id(), get_alias(), fmEvent);
		CHistoryRecord::GetInstance()->InsertRecord(get_ademco_id(), record, event_time, 
													core::RECORD_LEVEL_ONOFFLINE);
	}

	_lock4AdemcoEventList.Lock();
	AdemcoEvent* ademcoEvent = new AdemcoEvent(zone, ademco_event, event_time);
	_ademcoEventList.push_back(ademcoEvent);
	NotifyObservers(ademcoEvent);
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

NAMESPACE_END
