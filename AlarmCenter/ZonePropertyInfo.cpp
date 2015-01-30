#include "stdafx.h"
#include "ZonePropertyInfo.h"

namespace core {

// CZonePropertyData

CZonePropertyData::CZonePropertyData()
{}


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


CZonePropertyData* CZonePropertyInfo::GetZonePropertyData(int id)
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


NAMESPACE_END
