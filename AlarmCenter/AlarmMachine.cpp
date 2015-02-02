#include "stdafx.h"
#include "AlarmMachine.h"
#include "MapInfo.h"
#include "ademco_event.h"
using namespace ademco;

namespace core {
	
CAlarmMachine::CAlarmMachine()
	: _id(0)
	, _ademco_id(0)
	, _alias(NULL)
	, _ademco_zone(0)
	, _ademco_event(MS_OFFLINE)
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
	if (_contact) { delete[] _contact; }
	if (_address) { delete[] _address; }
	if (_phone) { delete[] _phone; }
	if (_phone_bk) { delete[] _phone_bk; }

	std::list<CMapInfo*>::iterator map_iter = _mapList.begin();
	while (map_iter != _mapList.end()) {
		CMapInfo* map = *map_iter++;
		delete map;
	}
	_mapList.clear();

	std::list<AdemcoEventCallbackInfo*>::iterator iter = _observerList.begin();
	while (iter != _observerList.end()) {
		AdemcoEventCallbackInfo* observer = *iter++;
		delete observer;
	}
	_observerList.clear();
}


void CAlarmMachine::RegisterObserver(void* udata, AdemcoEventCB cb)
{ /*_udata = udata; _statusCb = cb;*/
	std::list<AdemcoEventCallbackInfo*>::iterator iter = _observerList.begin();
	while (iter != _observerList.end()) {
		AdemcoEventCallbackInfo* observer = *iter;
		if (observer->_udata == udata) {
			return;
		}
		iter++;
	}
	AdemcoEventCallbackInfo *observer = new AdemcoEventCallbackInfo(cb, udata);
	_observerList.insert(iter, observer);
}


void CAlarmMachine::UnregisterObserver(void* udata)
{
	std::list<AdemcoEventCallbackInfo*>::iterator iter = _observerList.begin();
	while (iter != _observerList.end()) {
		AdemcoEventCallbackInfo* observer = *iter;
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
	std::list<AdemcoEventCallbackInfo*>::iterator iter = _observerList.begin();
	while (iter != _observerList.end()) {
		AdemcoEventCallbackInfo* observer = *iter++;
		observer->_on_result(observer->_udata, _ademco_zone, _ademco_event);
	}
}


void CAlarmMachine::SetAdemcoEvent(int zone, int ademco_event)
{
	if (_ademco_zone != zone && _ademco_event != ademco_event) {
		_ademco_zone = zone;
		_ademco_event = ademco_event;
		_online = ademco_event > MS_OFFLINE;
		NotifyObservers();
	}
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


CMapInfo* CAlarmMachine::GetFirstMap() const
{
	if (_mapList.size() == 0)
		return NULL;

	return _mapList.front();
}

NAMESPACE_END
