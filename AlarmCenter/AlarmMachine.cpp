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
	, _statusCb(NULL)
	, _udata(NULL)
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
}








NAMESPACE_END
