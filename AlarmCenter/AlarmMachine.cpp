#include "stdafx.h"
#include "AlarmMachine.h"
#include "MapInfo.h"
#include "ZoneInfo.h"

namespace core {
	
CAlarmMachine::CAlarmMachine()
	: _id(0)
	, _ademco_id(0)
	, _alias(NULL)
	, _status(MS_OFFLINE)
	, _online(false)
	//, _statusCb(NULL)
	//, _udata(NULL)
{
	//memset(this, 0, sizeof(this));
	memset(_device_id, 0, sizeof(_device_id));
	memset(_device_idW, 0, sizeof(_device_idW));
	_alias = new wchar_t[1];
	_alias[0] = 0;
}


CAlarmMachine::~CAlarmMachine()
{
	if (_alias) { delete[] _alias; }

	std::list<CMapInfo*>::iterator map_iter = _mapList.begin();
	while (map_iter != _mapList.end()) {
		CMapInfo* map = *map_iter++;
		delete map;
	}
	_mapList.clear();

	std::list<CZoneInfo*>::iterator zone_iter = _zoneList.begin();
	while (zone_iter != _zoneList.end()) {
		CZoneInfo* zone = *zone_iter++;
		delete zone;
	}
	_zoneList.clear();

	std::list<MachineStatusCallbackInfo*>::iterator iter = _observerList.begin();
	while (iter != _observerList.end()) {
		MachineStatusCallbackInfo* observer = *iter++;
		delete observer;
	}
	_observerList.clear();
}


void CAlarmMachine::RegisterObserver(void* udata, MachineStatusCB cb)
{ /*_udata = udata; _statusCb = cb;*/
	std::list<MachineStatusCallbackInfo*>::iterator iter = _observerList.begin();
	while (iter != _observerList.end()) {
		MachineStatusCallbackInfo* observer = *iter;
		if (observer->_udata == udata) {
			return;
		}
		iter++;
	}
	MachineStatusCallbackInfo *observer = new MachineStatusCallbackInfo(cb, udata);
	_observerList.insert(iter, observer);
}


void CAlarmMachine::UnregisterObserver(void* udata)
{
	std::list<MachineStatusCallbackInfo*>::iterator iter = _observerList.begin();
	while (iter != _observerList.end()) {
		MachineStatusCallbackInfo* observer = *iter;
		if (observer->_udata == udata) {
			delete observer;
			_observerList.erase(iter);
			break;
		}
		iter++;
	}
}


void CAlarmMachine::NotifyObservers()
{
	std::list<MachineStatusCallbackInfo*>::iterator iter = _observerList.begin();
	while (iter != _observerList.end()) {
		MachineStatusCallbackInfo* observer = *iter++;
		observer->_on_result(observer->_udata, _status);
	}
}



NAMESPACE_END
