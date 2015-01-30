#include "stdafx.h"
#include "MapInfo.h"
#include "ZoneInfo.h"

namespace core {

CMapInfo::CMapInfo()
	: _id(0)
	, _type(0)
	, _ademco_id(0)
	, _path(NULL)
{
	_path = new wchar_t[1];
	_path[0] = 0;
}


CMapInfo::~CMapInfo()
{
	if (_path) { delete[] _path; }

	std::list<CZoneInfo*>::iterator zone_iter = _zoneList.begin();
	while (zone_iter != _zoneList.end()) {
		CZoneInfo* zone = *zone_iter++;
		delete zone;
	}
	_zoneList.clear();
}


const CZoneInfo* CMapInfo::GetFirstZoneInfo()
{
	if (_zoneList.size() > 0) {
		_curZoneListIter = _zoneList.begin();
		return *_curZoneListIter++;
	}

	return NULL;
}


const CZoneInfo* CMapInfo::GetNextZoneInfo()
{
	if (_zoneList.size() > 0 && _curZoneListIter != _zoneList.end()) {
		return *_curZoneListIter++;
	}

	return NULL;
}








NAMESPACE_END
