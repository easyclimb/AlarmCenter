#include "stdafx.h"
#include "ZoneInfo.h"
#include "DetectorInfo.h"
#include "ZonePropertyInfo.h"
#include "AlarmMachine.h"
#include "AlarmMachineManager.h"

namespace core
{

CZoneInfo::CZoneInfo()
	: _id(-1)
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
	if (_alias) { delete[] _alias; }
	if (_detectorInfo) { delete _detectorInfo; }
	if (_subMachineInfo) { delete _subMachineInfo; }
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
			_cb(_udata, *alarm);
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


NAMESPACE_END
