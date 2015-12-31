#include "stdafx.h"
#include "ZoneInfo.h"
#include "DetectorInfo.h"
#include "AlarmMachine.h"
#include "AlarmMachineManager.h"
#include "MapInfo.h"
#include "AlarmCenter.h"
#include "VideoPlayerDlg.h"

namespace core
{

CZoneInfo::CZoneInfo()
	: CDetectorBindInterface()
	, _id(-1)
	, _ademco_id(-1)
	, _zone_value(-1)
	, _sub_zone(-1)
	, _status_or_property(-1)
	, _physical_addr(-1)
	//, _map_id(0)
	, _type(ZT_ZONE)
	, _detector_id(-1)
	, _sub_machine_id(-1)
	//, _property_id(0)
	, _alias()
	, _subMachineInfo(nullptr)
	, _mapInfo(nullptr)
	, _alarming(false)
	, _highestEventLevel(EVENT_LEVEL_STATUS)
{
}


CZoneInfo::~CZoneInfo()
{
	SAFEDELETEP(_subMachineInfo);
}


int CZoneInfo::char_to_status(char val)
{
	if (ZS_ARM == val)
		return EVENT_ARM;
	else if (ZS_DISARM == val)
		return EVENT_DISARM;
	else
		return EVENT_INVALID_EVENT;
}


char CZoneInfo::status_to_char(int val)
{
	ZoneStatusOrProperty zsop;
	if (EVENT_ARM == val)
		zsop = ZS_ARM;
	else if (EVENT_DISARM == val)
		zsop = ZS_DISARM;
	else
		zsop = ZSOP_INVALID;
	return zsop & 0xFF;
}


void CZoneInfo::HandleAdemcoEvent(const ademco::AdemcoEvent* ademcoEvent)
{
	AUTO_LOG_FUNCTION;
	bool *alarm = nullptr;
	switch (ademcoEvent->_event) {
		case ademco::EVENT_OFFLINE:
		case ademco::EVENT_ONLINE:
		case ademco::EVENT_DISARM:
		case ademco::EVENT_ARM:
			break;
		case EVENT_CLEARMSG:
			alarm = new bool();
			*alarm = false;
			break;
		default: 
			alarm = new bool();
			*alarm = true;
			break;
	}

	if (alarm) {
		_alarming = *alarm;
		if (_alarming) {
			EventLevel level = GetEventLevel(ademcoEvent->_event);
			bool bNeedPushBack = true;
			if (level == EVENT_LEVEL_EXCEPTION_RESUME) {
				ADEMCO_EVENT exception_event = GetExceptionEventByResumeEvent(ademcoEvent->_event);
				auto iter = _eventList.begin();
				while (iter != _eventList.end()) {
					ADEMCO_EVENT old = *iter;
					if (old == exception_event) {
						_eventList.erase(iter);
						bNeedPushBack = false;
						break;
					}
					iter++;
				}
			}
			if (bNeedPushBack) {
				_highestEventLevel = EVENT_LEVEL_STATUS;
				_eventList.push_back(ademcoEvent->_event);
				for (auto old_event : _eventList) {
					level = GetEventLevel(old_event);
					if (_highestEventLevel < level) {
						_highestEventLevel = level;
					}
				}
			}
			_alarming = _eventList.size() > 0;
			if (_alarming) {
				COLORREF clr = GetEventLevelColor(_highestEventLevel);
				if (_cb && _udata) {
					_cb(_udata, ICZC_ALARM_START, clr);
				}
				
				// 2015-9-22 22:56:53 play video
				if (_type == ZT_ZONE) {
					g_videoPlayerDlg->PlayVideo(video::ZoneUuid(_ademco_id, _zone_value, 0));
				} else if (_type == ZT_SUB_MACHINE_ZONE) {
					g_videoPlayerDlg->PlayVideo(video::ZoneUuid(_ademco_id, _zone_value, _sub_zone));
				}
				
			} else {
				_highestEventLevel = EVENT_LEVEL_STATUS;
				if (_cb && _udata) {
					_cb(_udata, ICZC_ALARM_STOP, 0);
				}
			}
		} else {
			_eventList.clear();
			_highestEventLevel = EVENT_LEVEL_STATUS;
			if (_cb && _udata) {
				_cb(_udata, ICZC_ALARM_STOP, 0);
			}
		}

		// 调用探头的回调函数，报警(如果存在)
		/*if (_cb) {
			_cb(_udata, *alarm ? ICZC_ALARM_START : ICZC_ALARM_STOP);
		}*/

		// 若为消警，则清除MapInfo的AlarmTextList
		/*if (!(*alarm) && _mapInfo) {
			_mapInfo->
		}*/

		delete alarm;
	}
}


//void CZoneInfo::InversionControl(InversionControlZoneCommand iczc)
//{
//	AUTO_LOG_FUNCTION;
//	if (_cb) {
//		_cb(_udata, iczc, 0);
//	} else {
//		_iczcCommandList.push_back(iczc);
//	}
//}


bool CZoneInfo::execute_set_sub_machine(CAlarmMachine* subMachine)
{
	AUTO_LOG_FUNCTION;
	// 1.创建分机信息
	CString query;
	query.Format(L"insert into SubMachine ([contact],[address],[phone],[phone_bk]) values('%s','%s','%s','%s')",
				 subMachine->get_contact(), subMachine->get_address(), 
				 subMachine->get_phone(), subMachine->get_phone_bk());
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	int id = mgr->AddAutoIndexTableReturnID(query);
	if (-1 == id) {
		JLOG(L"add submachine failed: %s\n", query);
		ASSERT(0); return false;
	}
	subMachine->set_id(id);

	// 2.更新防区信息
	query.Format(L"update ZoneInfo set type=%d,sub_machine_id=%d where id=%d",
				 ZT_SUB_MACHINE, id, _id);
	if (!mgr->ExecuteSql(query)) {
		JLOG(L"update ZoneInfo type failed: %s\n", query);
		ASSERT(0); return false;
	}
	_type = ZT_SUB_MACHINE;
	_sub_machine_id = id;
	_subMachineInfo = subMachine;
	return true;
}


bool CZoneInfo::execute_del_sub_machine()
{
	AUTO_LOG_FUNCTION;
	if (_subMachineInfo) {
		CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
		if (mgr->DeleteSubMachine(this)) {
			delete _subMachineInfo;
			_subMachineInfo = nullptr;
			_sub_machine_id = -1;
			_type = ZT_ZONE;
			return true;
		}
	}
	return false;
}


bool CZoneInfo::execute_update_alias(const wchar_t* alias)
{
	AUTO_LOG_FUNCTION;
	CString query;
	if (_type == ZT_SUB_MACHINE_ZONE) {
		query.Format(L"update SubZone set alias='%s' where id=%d",
					 alias, _id);
	} else {
		query.Format(L"update ZoneInfo set alias='%s' where id=%d",
					 alias, _id);
	}
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (mgr->ExecuteSql(query)) {
		set_alias(alias);
		if (_subMachineInfo) {
			_subMachineInfo->set_alias(alias);
			static std::vector<char> xdata;
			_subMachineInfo->SetAdemcoEvent(ES_UNKNOWN, EVENT_MACHINE_ALIAS, 0, INDEX_SUB_MACHINE, time(nullptr), time(nullptr), xdata);
		}
		return true;
	} else {
		ASSERT(0); JLOG(L"update SubMachine alias failed\n");
		return false;
	}
}


bool CZoneInfo::execute_update_contact(const wchar_t* contact)
{
	AUTO_LOG_FUNCTION;
	if (_subMachineInfo == nullptr)
		return false;

	CString query;
	query.Format(L"update SubMachine set contact='%s' where id=%d",
				 contact, _subMachineInfo->get_id());
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (mgr->ExecuteSql(query)) {
		_subMachineInfo->set_contact(contact);
		static std::vector<char> xdata;
		_subMachineInfo->SetAdemcoEvent(ES_UNKNOWN, EVENT_MACHINE_ALIAS, 0, INDEX_SUB_MACHINE, time(nullptr), time(nullptr), xdata);
		return true;
	} else {
		ASSERT(0); JLOG(L"update SubMachine contact failed.\n");
		return false;
	}
	return false;
}


bool CZoneInfo::execute_update_address(const wchar_t* address)
{
	AUTO_LOG_FUNCTION;
	if (_subMachineInfo == nullptr)
		return false;

	CString query;
	query.Format(L"update SubMachine set address='%s' where id=%d",
				 address, _subMachineInfo->get_id());
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (mgr->ExecuteSql(query)) {
		_subMachineInfo->set_address(address);
		static std::vector<char> xdata;
		_subMachineInfo->SetAdemcoEvent(ES_UNKNOWN, EVENT_MACHINE_ALIAS, 0, INDEX_SUB_MACHINE, time(nullptr), time(nullptr), xdata);
		return true;
	} else {
		ASSERT(0); JLOG(L"update SubMachine address failed.\n");
		return false;
	}
	return false;
}


bool CZoneInfo::execute_update_phone(const wchar_t* phone)
{
	AUTO_LOG_FUNCTION;
	if (_subMachineInfo == nullptr)
		return false;

	CString query;
	query.Format(L"update SubMachine set phone='%s' where id=%d",
				 phone, _subMachineInfo->get_id());
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (mgr->ExecuteSql(query)) {
		_subMachineInfo->set_phone(phone);
		static std::vector<char> xdata;
		_subMachineInfo->SetAdemcoEvent(ES_UNKNOWN, EVENT_MACHINE_ALIAS, 0, INDEX_SUB_MACHINE, time(nullptr), time(nullptr), xdata);
		return true;
	} else {
		ASSERT(0); JLOG(L"update SubMachine phone failed.\n");
		return false;
	}
	return false;
}


bool CZoneInfo::execute_update_phone_bk(const wchar_t* phone_bk)
{
	AUTO_LOG_FUNCTION;
	if (_subMachineInfo == nullptr)
		return false;

	CString query;
	query.Format(L"update SubMachine set phone_bk='%s' where id=%d",
				 phone_bk, _subMachineInfo->get_id());
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (mgr->ExecuteSql(query)) {
		_subMachineInfo->set_phone_bk(phone_bk);
		static std::vector<char> xdata;
		_subMachineInfo->SetAdemcoEvent(ES_UNKNOWN, EVENT_MACHINE_ALIAS, 0, INDEX_SUB_MACHINE, time(nullptr), time(nullptr), xdata);
		return true;
	} else {
		ASSERT(0); JLOG(L"update SubMachine phone_bk failed.\n");
		return false;
	}
	return false;
}


bool CZoneInfo::execute_set_detector_info(CDetectorInfo* detInfo)
{
	AUTO_LOG_FUNCTION;
	ASSERT(_detectorInfo == nullptr); ASSERT(detInfo);
	CString query;
	if (ZT_SUB_MACHINE_ZONE == _type) {
		query.Format(L"update SubZone set detector_info_id=%d where id=%d",
					 detInfo->get_id(), _id);
	} else {
		query.Format(L"update ZoneInfo set detector_info_id=%d where id=%d",
					 detInfo->get_id(), _id);
	}
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (!mgr->ExecuteSql(query)) {
		ASSERT(0); JLOG(L"update zoneInfo failed.\n");
		return false;
	}
	_detectorInfo = detInfo;
	query.Format(L"update DetectorInfo set zone_info_id=%d where id=%d",
					_id, detInfo->get_id());
	if (!mgr->ExecuteSql(query)) {
		ASSERT(0); JLOG(L"update DetectorInfo failed.\n");
		return false;
	} 
	detInfo->set_zone_info_id(_id);
	detInfo->set_zone_value(_zone_value);
	return true;
}


bool CZoneInfo::execute_rem_detector_info()
{
	AUTO_LOG_FUNCTION;
	ASSERT(_detectorInfo);
	CString query;
	if (ZT_SUB_MACHINE_ZONE == _type) {
		query.Format(L"update SubZone set detector_info_id=-1 where id=%d", _id);
	} else {
		query.Format(L"update ZoneInfo set detector_info_id=-1 where id=%d", _id);
	}
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (!mgr->ExecuteSql(query)) {
		ASSERT(0); JLOG(L"update zoneInfo failed.\n");
		return false;
	}
	_detector_id = -1;
	query.Format(L"update DetectorInfo set zone_info_id=-1 where id=%d", 
				 _detectorInfo->get_id());
	if (!mgr->ExecuteSql(query)) {
		ASSERT(0); JLOG(L"update zoneInfo failed.\n");
		return false;
	}
	_detectorInfo->set_zone_info_id(-1);
	_detectorInfo->set_zone_value(-1);
	if (_mapInfo) {
		_mapInfo->AddNoZoneDetectorInfo(_detectorInfo);
		_mapInfo->RemoveInterface(this);
		_mapInfo = nullptr;
	}
	_detectorInfo = nullptr;
	return true;
}


bool CZoneInfo::execute_del_detector_info()
{
	AUTO_LOG_FUNCTION;
	ASSERT(_detectorInfo);
	CString query;
	if (ZT_SUB_MACHINE_ZONE == _type) {
		query.Format(L"update SubZone set detector_info_id=-1 where id=%d", _id);
	} else {
		query.Format(L"update ZoneInfo set detector_info_id=-1 where id=%d", _id);
	}
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (!mgr->ExecuteSql(query)) {
		ASSERT(0); JLOG(L"update zoneInfo failed.\n");
		return false;
	}
	query.Format(L"delete from DetectorInfo where id=%d",
				  _detectorInfo->get_id());
	if (!mgr->ExecuteSql(query)) {
		ASSERT(0); JLOG(L"delete DetectorInfo failed.\n");
		return false;
	}
	delete _detectorInfo;
	_detectorInfo = nullptr;
	_detector_id = -1;
	if (_mapInfo) {
		_mapInfo->RemoveInterface(this);
		_mapInfo = nullptr;
	}
	return true;
}


bool CZoneInfo::execute_bind_detector_info_to_map_info(CMapInfo* mapInfo)
{
	AUTO_LOG_FUNCTION;
	ASSERT(_detectorInfo); ASSERT(mapInfo);
	CString query;
	query.Format(L"update DetectorInfo set map_id=%d where id=%d", 
				 mapInfo->get_id(), _detectorInfo->get_id());
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (!mgr->ExecuteSql(query)) {
		ASSERT(0); JLOG(L"update DetectorInfo failed.\n");
		return false;
	}
	_detectorInfo->set_map_id(mapInfo->get_id());
	return true;
}


bool CZoneInfo::execute_unbind_detector_info_from_map_info()
{
	AUTO_LOG_FUNCTION;
	ASSERT(_detectorInfo); ASSERT(_mapInfo);
	CString query;
	query.Format(L"update DetectorInfo set map_id=-1 where id=%d",
				 _detectorInfo->get_id());
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (!mgr->ExecuteSql(query)) {
		ASSERT(0); JLOG(L"update DetectorInfo failed.\n");
		return false;
	}
	_detectorInfo->set_map_id(-1);
	_mapInfo->RemoveInterface(this);
	_mapInfo = nullptr;
	return true;
}


bool CZoneInfo::execute_create_detector_info_and_bind_map_info(CDetectorInfo* detInfo,
															   CMapInfo* mapInfo)
{
	AUTO_LOG_FUNCTION;
	ASSERT(_detectorInfo == nullptr); 
	ASSERT(detInfo); ASSERT(mapInfo);
	CString query;
	query.Format(L"insert into DetectorInfo ([map_id],[zone_info_id],[x],[y],[distance],[angle],[detector_lib_id]) values(%d,%d,%d,%d,%d,%d,%d)",
				 mapInfo->get_id(), _id, detInfo->get_x(), detInfo->get_y(),
				 detInfo->get_distance(), detInfo->get_angle(),
				 detInfo->get_detector_lib_id());
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	int id = mgr->AddAutoIndexTableReturnID(query);
	if (-1 == id) {
		ASSERT(0); JLOG(L"insert detector info failed.\n"); return false;
	}
	if (ZT_SUB_MACHINE_ZONE == _type) {
		query.Format(L"update SubZone set detector_info_id=%d where id=%d",
					 id, _id);
	} else {
		query.Format(L"update ZoneInfo set detector_info_id=%d where id=%d",
					 id, _id);
	}
	
	if (!mgr->ExecuteSql(query)) {
		ASSERT(0); JLOG(L"update zoneinfo failed.\n"); return false;
	}
	detInfo->set_id(id);
	detInfo->set_map_id(mapInfo->get_id());
	detInfo->set_zone_info_id(_id);
	bool bSubZone = (ZT_SUB_MACHINE_ZONE == _type);
	detInfo->set_zone_value(bSubZone ? _sub_zone : _zone_value);
	_detectorInfo = detInfo;
	_detector_id = id;
	if (_mapInfo) {
		_mapInfo->RemoveInterface(this);
	}
	_mapInfo = mapInfo;
	mapInfo->AddInterface(this);
	return true;
}


bool CZoneInfo::execute_set_physical_addr(int addr)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update ZoneInfo set physical_addr=%d where id=%d",
				 addr, _id);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (!mgr->ExecuteSql(query)) {
		ASSERT(0); JLOG(L"update physical_addr failed.\n"); return false;
	}
	_physical_addr = addr;
	return true;
}


bool CZoneInfo::execute_set_status_or_property(char status)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update ZoneInfo set status_or_property=%d where id=%d",
				 status, _id);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (!mgr->ExecuteSql(query)) {
		ASSERT(0); JLOG(L"update _status_or_property failed.\n"); return false;
	}
	_status_or_property = status;
	return true;
}


void CZoneInfo::DoClick()
{
	InversionControl(ICZC_CLICK);
}


void CZoneInfo::DoRClick()
{
	InversionControl(ICZC_RCLICK);
}

NAMESPACE_END
