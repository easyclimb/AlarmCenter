#include "stdafx.h"
#include "SubMachineInfo.h"
#include "ZoneInfo.h"

namespace core {
/////////////************CSubMachine***********/////////////

CSubMachineInfo::CSubMachineInfo()
	: _id(0)
	, _ademco_id(0)
	, _zone_id(0)
	, _alias(NULL)
	, _contact(NULL)
	, _address(NULL)
	, _phone(NULL)
	, _phone_bk(NULL)
	, _armed(false)
{
	INITIALIZE_STRING(_alias);
	INITIALIZE_STRING(_contact);
	INITIALIZE_STRING(_address);
	INITIALIZE_STRING(_phone);
	INITIALIZE_STRING(_phone_bk);

	memset(_sub_zones, 0, sizeof(_sub_zones));
}

CSubMachineInfo::~CSubMachineInfo()
{
	SAFEDELETEARR(_alias);
	SAFEDELETEARR(_contact);
	SAFEDELETEARR(_address);
	SAFEDELETEARR(_phone);
	SAFEDELETEARR(_phone_bk);

	for (int i = 0; i < MAX_SUB_MACHINE_ZONE; i++) {
		CZoneInfo* zone = _sub_zones[i];
		SAFEDELETEP(zone);
	}
}


void CSubMachineInfo::AddSubZone(CZoneInfo* zoneInfo)
{
	int sub_zone = zoneInfo->get_sub_zone();
	if (0 <= sub_zone && sub_zone < MAX_SUB_MACHINE_ZONE) {
		_sub_zones[sub_zone] = zoneInfo;
	}
}


CZoneInfo* CSubMachineInfo::GetSubZone(int sub_zone)
{
	if (0 <= sub_zone && sub_zone < MAX_SUB_MACHINE_ZONE) {
		return _sub_zones[sub_zone];
	}
	return NULL;
}

NAMESPACE_END;
