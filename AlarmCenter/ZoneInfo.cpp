#include "stdafx.h"
#include "ZoneInfo.h"
#include "DetectorInfo.h"

namespace core
{

CZoneInfo::CZoneInfo()
	: _id(0)
	, _zone_id(0)
	, _ademco_id(0)
	, _map_id(0)
	//, _type(0)
	, _detector_id(0)
	, _property_id(0)
	, _alias(NULL)
	, _detectorInfo(NULL)
{
	_alias = new wchar_t[1];
	_alias[0] = 0;
}


CZoneInfo::~CZoneInfo()
{
	if (_alias) { delete[] _alias; }
	if (_detectorInfo) { delete _detectorInfo; }
}


//////***************CSubZoneInfo****************/////////

CSubZoneInfo::CSubZoneInfo()
	: _id(0)
	, _sub_zone(0)
	, _zone_id(0)
	, _sub_map_id(0)
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
}


NAMESPACE_END
