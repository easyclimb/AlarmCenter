#include "stdafx.h"
#include "ZoneInfo.h"
#include "DetectorInfo.h"
#include "ZonePropertyInfo.h"
#include "AlarmMachine.h"
#include "AlarmMachineManager.h"

namespace core
{

CZoneInfo::CZoneInfo()
	: __class_name("class name is CZoneInfo")
	, _id(-1)
	, _ademco_id(-1)
	, _zone_value(-1)
	, _sub_zone(-1)
	//, _map_id(0)
	, _type(ZT_ZONE)
	, _detector_id(-1)
	, _sub_machine_id(-1)
	//, _property_id(0)
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
	SAFEDELETEARR(_alias);
	SAFEDELETEP(_detectorInfo); 
	SAFEDELETEP(_subMachineInfo);

	if (_cb) {
		_cb(_udata, ICZC_DESTROY);
	}
}


void CZoneInfo::HandleAdemcoEvent(const ademco::AdemcoEvent* ademcoEvent)
{
	bool *alarm = NULL;
	switch (ademcoEvent->_event) {
		case MS_OFFLINE: 
		case MS_ONLINE:
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

		// 调用探头的回调函数，报警(如果存在)
		if (_cb) {
			_cb(_udata, *alarm ? ICZC_ALARM_START : ICZC_ALARM_STOP);
		}

		// 若为消警，则清除MapInfo的AlarmTextList
		/*if (!(*alarm) && _mapInfo) {
			_mapInfo->
		}*/

		delete alarm;
	}
}


bool CZoneInfo::execute_set_sub_machine(CAlarmMachine* subMachine)
{
	AUTO_LOG_FUNCTION;
	// 1.创建分机信息
#pragma region create submachine
	CString query;
	query.Format(L"insert into SubMachine ([contact],[address],[phone],[phone_bk]) values('%s','%s','%s','%s')",
				 subMachine->get_contact(), subMachine->get_address(), 
				 subMachine->get_phone(), subMachine->get_phone_bk());
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	int id = mgr->AddAutoIndexTableReturnID(query);
	if (-1 == id) {
		LOG(L"add submachine failed: %s\n", query);
		ASSERT(0); return false;
	}
	subMachine->set_id(id);
#pragma endregion

	// 2.更新防区信息
#pragma region update zone info
	query.Format(L"update ZoneInfo set type=%d,sub_machine_id=%d where id=%d",
				 ZT_SUB_MACHINE, id, _id);
	if (!mgr->ExecuteSql(query)) {
		LOG(L"update ZoneInfo type failed: %s\n", query);
		ASSERT(0); return false;
	}
	_type = ZT_SUB_MACHINE;
	_sub_machine_id = id;
	_subMachineInfo = subMachine;
	return true;
#pragma endregion
}


bool CZoneInfo::execute_del_sub_machine()
{
	AUTO_LOG_FUNCTION;
	if (_subMachineInfo) {
		CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
		if (mgr->DeleteSubMachine(this)) {
			delete _subMachineInfo;
			_subMachineInfo = NULL;
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
		if (_subMachineInfo)
			_subMachineInfo->set_alias(alias);
		return true;
	} else {
		ASSERT(0); LOG(L"update zoneInfo alias failed\n");
		return false;
	}
}


bool CZoneInfo::execute_update_contact(const wchar_t* contact)
{
	AUTO_LOG_FUNCTION;
	if (_subMachineInfo == NULL)
		return false;

	CString query;
	query.Format(L"update SubMachine set contact='%s' where id=%d",
				 contact, _subMachineInfo->get_id());
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (mgr->ExecuteSql(query)) {
		_subMachineInfo->set_contact(contact);
		return true;
	} else {
		ASSERT(0); LOG(L"update zoneInfo contact failed.\n");
		return false;
	}
	return false;
}


bool CZoneInfo::execute_update_address(const wchar_t* address)
{
	AUTO_LOG_FUNCTION;
	if (_subMachineInfo == NULL)
		return false;

	CString query;
	query.Format(L"update SubMachine set address='%s' where id=%d",
				 address, _subMachineInfo->get_id());
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (mgr->ExecuteSql(query)) {
		_subMachineInfo->set_address(address);
		return true;
	} else {
		ASSERT(0); LOG(L"update zoneInfo address failed.\n");
		return false;
	}
	return false;
}


bool CZoneInfo::execute_update_phone(const wchar_t* phone)
{
	AUTO_LOG_FUNCTION;
	if (_subMachineInfo == NULL)
		return false;

	CString query;
	query.Format(L"update SubMachine set phone='%s' where id=%d",
				 phone, _subMachineInfo->get_id());
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (mgr->ExecuteSql(query)) {
		_subMachineInfo->set_phone(phone);
		return true;
	} else {
		ASSERT(0); LOG(L"update zoneInfo phone failed.\n");
		return false;
	}
	return false;
}


bool CZoneInfo::execute_update_phone_bk(const wchar_t* phone_bk)
{
	AUTO_LOG_FUNCTION;
	if (_subMachineInfo == NULL)
		return false;

	CString query;
	query.Format(L"update SubMachine set phone_bk='%s' where id=%d",
				 phone_bk, _subMachineInfo->get_id());
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (mgr->ExecuteSql(query)) {
		_subMachineInfo->set_phone_bk(phone_bk);
		return true;
	} else {
		ASSERT(0); LOG(L"update zoneInfo phone_bk failed.\n");
		return false;
	}
	return false;
}


bool CZoneInfo::execute_set_detector_info(CDetectorInfo* detInfo)
{
	AUTO_LOG_FUNCTION;
	ASSERT(_detectorInfo == NULL); ASSERT(detInfo);
	CString query;
	query.Format(L"update ZoneInfo set detector_info_id=%d where id=%d", 
				 detInfo->get_id(), _id);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (!mgr->ExecuteSql(query)) {
		ASSERT(0); LOG(L"update zoneInfo failed.\n");
		return false;
	}
	_detectorInfo = detInfo;
	query.Format(L"update DetectorInfo set zone_info_id=%d where id=%d",
					_id, detInfo->get_id());
	if (!mgr->ExecuteSql(query)) {
		ASSERT(0); LOG(L"update DetectorInfo failed.\n");
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
	query.Format(L"update ZoneInfo set detector_info_id=-1 where id=%d", _id);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (!mgr->ExecuteSql(query)) {
		ASSERT(0); LOG(L"update zoneInfo failed.\n");
		return false;
	}
	_detector_id = -1;
	query.Format(L"update DetectorInfo set zone_info_id=-1 where id=%d", 
				 _detectorInfo->get_id());
	if (!mgr->ExecuteSql(query)) {
		ASSERT(0); LOG(L"update zoneInfo failed.\n");
		return false;
	}
	_detectorInfo->set_zone_info_id(-1);
	_detectorInfo->set_zone_value(-1);
	_detectorInfo = NULL;
	return true;
}


bool CZoneInfo::execute_del_detector_info()
{
	AUTO_LOG_FUNCTION;
	ASSERT(_detectorInfo);
	CString query;
	query.Format(L"update ZoneInfo set detector_info_id=-1 where id=%d", _id);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (!mgr->ExecuteSql(query)) {
		ASSERT(0); LOG(L"update zoneInfo failed.\n");
		return false;
	}
	query.Format(L"delete from DetectorInfo where id=%d",
				  _detectorInfo->get_id());
	if (!mgr->ExecuteSql(query)) {
		ASSERT(0); LOG(L"delete zoneInfo failed.\n");
		return false;
	}
	delete _detectorInfo;
	_detectorInfo = NULL;
	_detector_id = -1;
	return true;
}

NAMESPACE_END
