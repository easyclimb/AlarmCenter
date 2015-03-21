#include "stdafx.h"
#include "MapInfo.h"
#include "DetectorInfo.h"
#include "AlarmMachineManager.h"

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
		_cb(_udata, ICMC_DESTROY, NULL);
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


void CMapInfo::SetInversionControlCallBack(void* udata, OnInversionControlMapCB cb)
{ 
	_udata = udata; _cb = cb;
}


void CMapInfo::InversionControl(InversionControlMapCommand icmc, AlarmText* at)
{
	AUTO_LOG_FUNCTION;
	if ((ICMC_ADD_ALARM_TEXT == icmc) && at) {
		_alarming = true;
		if (_cb) { 
			_cb(_udata, ICMC_ADD_ALARM_TEXT, at); delete at; 
		} else {
			_lock4AlarmTextList.Lock();
			_alarmTextList.push_back(at);
			_lock4AlarmTextList.UnLock();
		}
	} else if(ICMC_CLR_ALARM_TEXT == icmc){
		_alarming = false;
		if (_cb) { _cb(_udata, ICMC_CLR_ALARM_TEXT, NULL); }
		clear_alarm_text_list();
	} else {
		if (_cb) { _cb(_udata, icmc, NULL); }
	}
}


void CMapInfo::TraverseAlarmText(void* udata, OnInversionControlMapCB cb)
{
	_lock4AlarmTextList.Lock();
	std::list<AlarmText*>::iterator iter = _alarmTextList.begin();
	while (iter != _alarmTextList.end()) {
		AlarmText* at = *iter++; 
		cb(udata, ICMC_ADD_ALARM_TEXT, at); 
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


bool CMapInfo::execute_delete_no_zone_detector_info(CDetectorInfo* detInfo)
{
	AUTO_LOG_FUNCTION;
	ASSERT(detInfo); ASSERT(_id == detInfo->get_id());
	CString query;
	query.Format(L"delete from DetectorInfo where id=%d", detInfo->get_id());
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (!mgr->ExecuteSql(query)) {
		ASSERT(0); LOG(L"delete DetectorInfo failed.\n");
		return false;
	}
	_noZoneDetectorList.remove(detInfo);
	delete detInfo;
	return true;
}

NAMESPACE_END
