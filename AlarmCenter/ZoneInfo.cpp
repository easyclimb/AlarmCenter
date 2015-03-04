#include "stdafx.h"
#include "ZoneInfo.h"
#include "DetectorInfo.h"
#include "SubMachineInfo.h"

namespace core
{

CZoneInfo::CZoneInfo()
	: _id(0)
	, _ademco_id(0)
	, _zone(0)
	//, _sub_zone(0)
	//, _map_id(0)
	, _type(ZT_ZONE)
	, _detector_id(0)
	, _sub_machine_id(0)
	, _property_id(0)
	, _alias(NULL)
	, _detectorInfo(NULL)
	, _subMachineInfo(NULL)
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


//////***************CSubZoneInfo****************/////////

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


NAMESPACE_END
