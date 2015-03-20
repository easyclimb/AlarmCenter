#include "stdafx.h"
#include "MapInfo.h"
//#include "ZoneInfo.h"
#include "DetectorInfo.h"

#include <iterator>

namespace core {

CMapInfo::CMapInfo()
	: __class_name("class name is CMapInfo")
	, _id(0)
	, _type(MAP_MACHINE)
	, _machine_id(0)
	, _path(NULL)
	, _udata(NULL)
	, _cb(NULL)
	, _alarming(false)
	, _activeZoneInfo(NULL)
{
	_alias = new wchar_t[1];
	_alias[0] = 0;
	_path = new wchar_t[1];
	_path[0] = 0;
}


CMapInfo::~CMapInfo()
{
	if (_alias) { delete[] _alias; }
	if (_path) { delete[] _path; }
	clear_alarm_text_list();
	if (_cb) {
		_cb(_udata, ICC_DESTROY, NULL);
	}

	CDetectorInfoListIter iter = _noZoneDetectorList.begin();
	while (iter != _noZoneDetectorList.end()) {
		CDetectorInfo* detInfo = *iter++;
		delete detInfo;
	}
	_noZoneDetectorList.clear();
}


void CMapInfo::GetAllZoneInfo(std::list<CZoneInfo*>& list)
{
	std::copy(_zoneList.begin(), _zoneList.end(), std::back_inserter(list));
}


void CMapInfo::SetInversionControlCallBack(void* udata, OnInversionControlCB cb)
{ 
	_udata = udata; _cb = cb;
}


void CMapInfo::InversionControl(InversionControlCommand icc, AlarmText* at)
{
	if ((ICC_ADD_ALARM_TEXT == icc) && at) {
		_alarming = true;
		if (_cb) { 
			_cb(_udata, ICC_ADD_ALARM_TEXT, at); delete at; 
		} else {
			_lock4AlarmTextList.Lock();
			_alarmTextList.push_back(at);
			_lock4AlarmTextList.UnLock();
		}
	} else if(ICC_CLR_ALARM_TEXT == icc){
		_alarming = false;
		if (_cb) { _cb(_udata, ICC_CLR_ALARM_TEXT, NULL); }
		clear_alarm_text_list();
	} else {
		if (_cb) { _cb(_udata, icc, NULL); }
	}
}


void CMapInfo::TraverseAlarmText(void* udata, OnInversionControlCB cb)
{
	_lock4AlarmTextList.Lock();
	std::list<AlarmText*>::iterator iter = _alarmTextList.begin();
	while (iter != _alarmTextList.end()) {
		AlarmText* at = *iter++; 
		cb(udata, ICC_ADD_ALARM_TEXT, at); 
		delete at;
	}
	_alarmTextList.clear();
	_lock4AlarmTextList.UnLock();
}


void CMapInfo::clear_alarm_text_list()
{
	_lock4AlarmTextList.Lock();
	std::list<AlarmText*>::iterator iter = _alarmTextList.begin();
	while (iter != _alarmTextList.end()) {
		AlarmText* at = *iter++;
		delete at;
	}
	_alarmTextList.clear();
	_lock4AlarmTextList.UnLock();
}


void CMapInfo::GetNoZoneDetectorInfo(CDetectorInfoList& list)
{
	std::copy(_noZoneDetectorList.begin(), _noZoneDetectorList.end(), 
			  std::back_inserter(list));
}

NAMESPACE_END
