#include "stdafx.h"
#include "ZonePropertyInfo.h"

#ifdef USE_ZONE_PROPERTY
namespace core {

// CZonePropertyData

CZonePropertyData::CZonePropertyData()
	: _id(0)
	, _property(ZP_BURGLAR)
	, _property_text(NULL)
	, _alarm_text(NULL)
{
	_property_text = new wchar_t[1];
	_property_text[0] = 0;

	_alarm_text = new wchar_t[1];
	_alarm_text[0] = 0;
}


CZonePropertyData::~CZonePropertyData()
{
	SAFEDELETEARR(_property_text);
	SAFEDELETEARR(_alarm_text);
}


ZoneProperty CZonePropertyData::IntegerToZoneProperty(int property)
{
	switch (property) {
		case ZP_BURGLAR:	return ZP_BURGLAR;		break;
		case ZP_DURESS:		return ZP_DURESS;		break;
		case ZP_FIRE:		return ZP_FIRE;			break;
		case ZP_GAS:		return ZP_GAS;			break;
		case ZP_WATER:		return ZP_WATER;		break;
		default:			return ZP_MAX;			break;
	}
}

// CZonePropertyInfo

IMPLEMENT_SINGLETON(CZonePropertyInfo)

CZonePropertyInfo::CZonePropertyInfo() 
	: _zonePropertyDataList()
{}


CZonePropertyInfo::~CZonePropertyInfo() 
{
	std::list<CZonePropertyData*>::iterator iter = _zonePropertyDataList.begin();
	while (iter != _zonePropertyDataList.end()) {
		CZonePropertyData* data = *iter++;
		delete data;
	}
}


CZonePropertyData* CZonePropertyInfo::GetZonePropertyDataById(int id)
{
	std::list<CZonePropertyData*>::iterator iter = _zonePropertyDataList.begin();
	while (iter != _zonePropertyDataList.end()) {
		CZonePropertyData* data = *iter++;
		if (data->get_id() == id) {
			return data;
		}
	}

	return NULL;
}


CZonePropertyData* CZonePropertyInfo::GetZonePropertyDataByProperty(int property_value)
{
	std::list<CZonePropertyData*>::iterator iter = _zonePropertyDataList.begin();
	while (iter != _zonePropertyDataList.end()) {
		CZonePropertyData* data = *iter++;
		if (data->get_property() == property_value) {
			return data;
		}
	}

	return NULL;
}


NAMESPACE_END
#endif
