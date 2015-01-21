#include "stdafx.h"
#include "MapInfo.h"


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
}












NAMESPACE_END
