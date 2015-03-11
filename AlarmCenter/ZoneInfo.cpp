#include "stdafx.h"
#include "ZoneInfo.h"
#include "DetectorInfo.h"
//#include "SubMachineInfo.h"
#include "ZonePropertyInfo.h"
#include "AlarmMachine.h"


namespace core
{

CZoneInfo::CZoneInfo()
	: _id(0)
	, _ademco_id(0)
	, _zone_value(0)
	, _sub_zone(0)
	//, _map_id(0)
	, _type(ZT_ZONE)
	, _detector_id(0)
	, _sub_machine_id(0)
	, _property_id(0)
	, _alias(NULL)
	, _detectorInfo(NULL)
	, _subMachineInfo(NULL)
	, _mapInfo(NULL)
	, _udata(NULL)
	, _cb(NULL)
	, _alarming(false)
{
	_alias = new wchar_t[1];
	_alias[0] = 0;
}


CZoneInfo::~CZoneInfo()
{
	if (_alias) { delete[] _alias; }
	if (_detectorInfo) { delete _detectorInfo; }
	if (_subMachineInfo) { delete _subMachineInfo; }
}


void CZoneInfo::HandleAdemcoEvent(const ademco::AdemcoEvent* ademcoEvent)
{
	switch (ademcoEvent->_event) {
		case MS_OFFLINE: 
			break;
		case MS_ONLINE:
			break;
		case ademco::EVENT_DISARM:
			break;
		case ademco::EVENT_ARM:
			break;
		default: 
			break;
	}
	// 1. 调用探头的回调函数，报警或消警(如果存在)

	// 2. 传递给subMachineInfo(如果存在)
}


//////***************CSubZoneInfo****************/////////
#ifdef USE_SUB_ZONE
CSubZoneInfo::CSubZoneInfo()
	: _id(0)
	, _sub_zone(0)
	, _sub_machine_id(0)
	//, _sub_map_id(0)
	, _detector_id(0)
	, _property_id(0)
	, _alias(NULL)
	, _detectorInfo(NULL)
{
	INITIALIZE_STRING(_alias);
}


CSubZoneInfo::~CSubZoneInfo()
{
	SAFEDELETEARR(_alias);
	SAFEDELETEP(_detectorInfo);
}
#endif


NAMESPACE_END
