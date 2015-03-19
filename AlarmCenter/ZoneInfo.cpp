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
		_cb(_udata, ALARM_QUIT);
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
			_cb(_udata, *alarm ? ALARM_START : ALARM_STOP);
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
		return true;
	} else {
		ASSERT(0); LOG(L"update zoneInfo alias failed: %s\n", query);
		return false;
	}
}


bool CZoneInfo::execute_update_contact(const wchar_t* alias)
{
	return false;
}


bool CZoneInfo::execute_update_address(const wchar_t* alias)
{
	return false;
}


bool CZoneInfo::execute_update_phone(const wchar_t* alias)
{
	return false;
}


bool CZoneInfo::execute_update_phone_bk(const wchar_t* alias)
{
	return false;
}



NAMESPACE_END
