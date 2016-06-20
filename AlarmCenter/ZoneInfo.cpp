#include "stdafx.h"
#include "ZoneInfo.h"
#include "DetectorInfo.h"
#include "AlarmMachine.h"
#include "AlarmMachineManager.h"
#include "MapInfo.h"
#include "AppResource.h"
#include "VideoPlayerDlg.h"
#include "HistoryRecord.h"
#include "AppResource.h"

namespace core
{

zone_info::zone_info()
	: detector_bind_interface()
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
	, _subMachineInfo()
	, _mapInfo()
	, _alarming(false)
	, _highestEventLevel(EVENT_LEVEL_STATUS)
{
}


zone_info::~zone_info()
{
}


int zone_info::char_to_status(char val)
{
	if (ZS_ARM == val)
		return EVENT_ARM;
	else if (ZS_DISARM == val)
		return EVENT_DISARM;
	else
		return EVENT_INVALID_EVENT;
}


char zone_info::status_to_char(int val)
{
	zone_status_or_property zsop;
	if (EVENT_ARM == val)
		zsop = ZS_ARM;
	else if (EVENT_DISARM == val)
		zsop = ZS_DISARM;
	else
		zsop = ZSOP_INVALID;
	return zsop & 0xFF;
}


void zone_info::HandleAdemcoEvent(const ademco::AdemcoEventPtr& ademcoEvent, const alarm_text_ptr& at)
{
	AUTO_LOG_FUNCTION;
	std::unique_ptr<bool> alarm = nullptr;
	switch (ademcoEvent->_event) {
		case ademco::EVENT_OFFLINE:
		case ademco::EVENT_ONLINE:
		case ademco::EVENT_DISARM:
		case ademco::EVENT_ARM:
			break;
		case EVENT_CLEARMSG:
			alarm = std::make_unique<bool>();
			*alarm = false;
			break;
		default: 
			alarm = std::make_unique<bool>();
			*alarm = true;
			break;
	}

	if (alarm) {
		_alarming = *alarm;
		if (_alarming) {
			EventLevel level = GetEventLevel(ademcoEvent->_event);
			bool bNeedPushBack = false; // 2016-6-2 09:55:13 never show a resume event to user
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

			//if (bNeedPushBack) {
				_highestEventLevel = EVENT_LEVEL_STATUS;
				//_eventList.push_back(ademcoEvent->_event);
				for (auto old_event : _eventList) {
					level = GetEventLevel(old_event);
					if (_highestEventLevel < level) {
						_highestEventLevel = level;
					}
				}
			//}

			_alarming = _eventList.size() > 0; // Is it still alarming?

			if (_alarming) {
				COLORREF clr = GetEventLevelColor(_highestEventLevel);
				if (_cb && !_udata.expired()) {
					_cb(_udata.lock(), std::make_shared<iczc_buffer>(ICZC_ALARM_START, clr));
				}
				
				// 2015-9-22 22:56:53 play video
				if (_type == ZT_ZONE) {
					g_videoPlayerDlg->PlayVideo(video::zone_uuid(_ademco_id, _zone_value, 0), at);
				} else if (_type == ZT_SUB_MACHINE_ZONE) {
					g_videoPlayerDlg->PlayVideo(video::zone_uuid(_ademco_id, _zone_value, _sub_zone), at);
				}
				
			} else {
				_highestEventLevel = EVENT_LEVEL_STATUS;
				if (_cb && !_udata.expired()) {
					_cb(_udata.lock(), std::make_shared<iczc_buffer>(ICZC_ALARM_STOP, 0));
				}
			}

		} else {
			_eventList.clear();
			_highestEventLevel = EVENT_LEVEL_STATUS;
			if (_cb && !_udata.expired()) {
				_cb(_udata.lock(), std::make_shared<iczc_buffer>(ICZC_ALARM_STOP, 0));
			}
		}
	}
}


bool zone_info::execute_set_sub_machine(const core::alarm_machine_ptr& subMachine)
{
	AUTO_LOG_FUNCTION;
	// 1.创建分机信息
	CString query;
	query.Format(L"insert into table_sub_machine ([contact],[address],[phone],[phone_bk],[expire_time]) values('%s','%s','%s','%s','%s')",
				 subMachine->get_contact(), subMachine->get_address(), 
				 subMachine->get_phone(), subMachine->get_phone_bk(),
				 utf8::a2w(jlib::time_point_to_string(subMachine->get_expire_time())).c_str());
	auto mgr = alarm_machine_manager::get_instance();
	int id = mgr->AddAutoIndexTableReturnID(query);
	if (-1 == id) {
		JLOG(L"add submachine failed: %s\n", query);
		ASSERT(0); return false;
	}
	subMachine->set_id(id);

	auto consumer_mgr = consumer_manager::get_instance();
	auto type = consumer_mgr->get_consumer_type_by_id(1); assert(type);
	auto a_consumer = consumer_mgr->execute_add_consumer(subMachine->get_ademco_id(), _zone_value,
														 type, 0, 0, subMachine->get_expire_time()); 
	assert(a_consumer);
	subMachine->set_consumer(a_consumer);

	// 2.更新防区信息
	query.Format(L"update table_zone set type=%d,sub_machine_id=%d where id=%d",
				 ZT_SUB_MACHINE, id, _id);
	if (!mgr->ExecuteSql(query)) {
		JLOG(L"update table_zone type failed: %s\n", query);
		ASSERT(0); return false;
	}

	// 3.set sms config
	mgr->CreateSmsConfigForMachine(subMachine);

	_type = ZT_SUB_MACHINE;
	_sub_machine_id = id;
	_subMachineInfo = subMachine;
	return true;
}


bool zone_info::execute_del_sub_machine()
{
	AUTO_LOG_FUNCTION;
	if (_subMachineInfo) {
		auto mgr = alarm_machine_manager::get_instance();
		if (mgr->DeleteSubMachine(shared_from_this())) {
			_subMachineInfo.reset();
			_sub_machine_id = -1;
			_type = ZT_ZONE;
			return true;
		}
	}
	return false;
}


bool zone_info::execute_update_alias(const wchar_t* alias)
{
	AUTO_LOG_FUNCTION;
	CString query;
	if (_type == ZT_SUB_MACHINE_ZONE) {
		query.Format(L"update table_sub_zone set zone_name='%s' where id=%d",
					 alias, _id);
	} else {
		query.Format(L"update table_zone set zone_name='%s' where id=%d",
					 alias, _id);
	}
	auto mgr = alarm_machine_manager::get_instance();
	if (mgr->ExecuteSql(query)) {
		auto t = time(nullptr);
		CString rec, smachine, ssubmachine, sfield;
		smachine = GetStringFromAppResource(IDS_STRING_MACHINE);
		ssubmachine = GetStringFromAppResource(IDS_STRING_SUBMACHINE);
		sfield = GetStringFromAppResource(IDS_STRING_ALIAS);
		rec.Format(L"%s(" + GetStringFromAppResource(IDS_STRING_FM_ADEMCO_ID) + L") %s(%03d) %s: %s --> %s",
				   smachine, _ademco_id, ssubmachine, _zone_value, sfield, _alias, alias);
		history_record_manager::get_instance()->InsertRecord(_ademco_id, _zone_value, rec, t, RECORD_LEVEL_USEREDIT);
		set_alias(alias);
		if (_subMachineInfo) {
			_subMachineInfo->set_alias(alias);
			_subMachineInfo->SetAdemcoEvent(ES_UNKNOWN, EVENT_MACHINE_ALIAS, 0, INDEX_SUB_MACHINE, t, t);
		}
		return true;
	} else {
		ASSERT(0); JLOG(L"update table_sub_machine alias failed\n");
		return false;
	}
}


bool zone_info::execute_update_contact(const wchar_t* contact)
{
	AUTO_LOG_FUNCTION;
	if (_subMachineInfo == nullptr)
		return false;

	CString query;
	query.Format(L"update table_sub_machine set contact='%s' where id=%d",
				 contact, _subMachineInfo->get_id());
	auto mgr = alarm_machine_manager::get_instance();
	if (mgr->ExecuteSql(query)) {
		auto t = time(nullptr);
		CString rec, smachine, ssubmachine, sfield;
		smachine = GetStringFromAppResource(IDS_STRING_MACHINE);
		ssubmachine = GetStringFromAppResource(IDS_STRING_SUBMACHINE);
		sfield = GetStringFromAppResource(IDS_STRING_CONTACT);
		rec.Format(L"%s(" + GetStringFromAppResource(IDS_STRING_FM_ADEMCO_ID) + L") %s(%03d) %s: %s --> %s",
				   smachine, _ademco_id, ssubmachine, _zone_value, sfield, _subMachineInfo->get_contact(), contact);
		history_record_manager::get_instance()->InsertRecord(_ademco_id, _zone_value, rec, t, RECORD_LEVEL_USEREDIT);
		_subMachineInfo->set_contact(contact);
		_subMachineInfo->SetAdemcoEvent(ES_UNKNOWN, EVENT_MACHINE_ALIAS, 0, INDEX_SUB_MACHINE, t, t);
		return true;
	} else {
		ASSERT(0); JLOG(L"update table_sub_machine contact failed.\n");
		return false;
	}
	return false;
}


bool zone_info::execute_update_address(const wchar_t* address)
{
	AUTO_LOG_FUNCTION;
	if (_subMachineInfo == nullptr)
		return false;

	CString query;
	query.Format(L"update table_sub_machine set address='%s' where id=%d",
				 address, _subMachineInfo->get_id());
	auto mgr = alarm_machine_manager::get_instance();
	if (mgr->ExecuteSql(query)) {
		auto t = time(nullptr);
		CString rec, smachine, ssubmachine, sfield;
		smachine = GetStringFromAppResource(IDS_STRING_MACHINE);
		ssubmachine = GetStringFromAppResource(IDS_STRING_SUBMACHINE);
		sfield = GetStringFromAppResource(IDS_STRING_ADDRESS);
		rec.Format(L"%s(" + GetStringFromAppResource(IDS_STRING_FM_ADEMCO_ID) + L") %s(%03d) %s: %s --> %s",
				   smachine, _ademco_id, ssubmachine, _zone_value, sfield, _subMachineInfo->get_address(), address);
		history_record_manager::get_instance()->InsertRecord(_ademco_id, _zone_value, rec, t, RECORD_LEVEL_USEREDIT);
		_subMachineInfo->set_address(address);
		_subMachineInfo->SetAdemcoEvent(ES_UNKNOWN, EVENT_MACHINE_ALIAS, 0, INDEX_SUB_MACHINE, t, t);
		return true;
	} else {
		ASSERT(0); JLOG(L"update table_sub_machine address failed.\n");
		return false;
	}
	return false;
}


bool zone_info::execute_update_phone(const wchar_t* phone)
{
	AUTO_LOG_FUNCTION;
	if (_subMachineInfo == nullptr)
		return false;

	CString query;
	query.Format(L"update table_sub_machine set phone='%s' where id=%d",
				 phone, _subMachineInfo->get_id());
	auto mgr = alarm_machine_manager::get_instance();
	if (mgr->ExecuteSql(query)) {
		auto t = time(nullptr);
		CString rec, smachine, ssubmachine, sfield;
		smachine = GetStringFromAppResource(IDS_STRING_MACHINE);
		ssubmachine = GetStringFromAppResource(IDS_STRING_SUBMACHINE);
		sfield = GetStringFromAppResource(IDS_STRING_PHONE);
		rec.Format(L"%s(" + GetStringFromAppResource(IDS_STRING_FM_ADEMCO_ID) + L") %s(%03d) %s: %s --> %s",
				   smachine, _ademco_id, ssubmachine, _zone_value, sfield, _subMachineInfo->get_phone(), phone);
		history_record_manager::get_instance()->InsertRecord(_ademco_id, _zone_value, rec, t, RECORD_LEVEL_USEREDIT);
		_subMachineInfo->set_phone(phone);
		_subMachineInfo->SetAdemcoEvent(ES_UNKNOWN, EVENT_MACHINE_ALIAS, 0, INDEX_SUB_MACHINE, t, t);
		return true;
	} else {
		ASSERT(0); JLOG(L"update table_sub_machine phone failed.\n");
		return false;
	}
	return false;
}


bool zone_info::execute_update_phone_bk(const wchar_t* phone_bk)
{
	AUTO_LOG_FUNCTION;
	if (_subMachineInfo == nullptr)
		return false;

	CString query;
	query.Format(L"update table_sub_machine set phone_bk='%s' where id=%d",
				 phone_bk, _subMachineInfo->get_id());
	auto mgr = alarm_machine_manager::get_instance();
	if (mgr->ExecuteSql(query)) {
		auto t = time(nullptr);
		CString rec, smachine, ssubmachine, sfield;
		smachine = GetStringFromAppResource(IDS_STRING_MACHINE);
		ssubmachine = GetStringFromAppResource(IDS_STRING_SUBMACHINE);
		sfield = GetStringFromAppResource(IDS_STRING_PHONE_BK);
		rec.Format(L"%s(" + GetStringFromAppResource(IDS_STRING_FM_ADEMCO_ID) + L") %s(%03d) %s: %s --> %s",
				   smachine, _ademco_id, ssubmachine, _zone_value, sfield, _subMachineInfo->get_phone_bk(), phone_bk);
		history_record_manager::get_instance()->InsertRecord(_ademco_id, _zone_value, rec, t, RECORD_LEVEL_USEREDIT);
		_subMachineInfo->set_phone_bk(phone_bk);
		_subMachineInfo->SetAdemcoEvent(ES_UNKNOWN, EVENT_MACHINE_ALIAS, 0, INDEX_SUB_MACHINE, t, t);
		return true;
	} else {
		ASSERT(0); JLOG(L"update table_sub_machine phone_bk failed.\n");
		return false;
	}
	return false;
}


bool zone_info::execute_set_detector_info(const detector_info_ptr& detInfo)
{
	AUTO_LOG_FUNCTION;
	ASSERT(_detectorInfo == nullptr); ASSERT(detInfo);
	CString query;
	if (ZT_SUB_MACHINE_ZONE == _type) {
		query.Format(L"update table_sub_zone set detector_info_id=%d where id=%d",
					 detInfo->get_id(), _id);
	} else {
		query.Format(L"update table_zone set detector_info_id=%d where id=%d",
					 detInfo->get_id(), _id);
	}
	auto mgr = alarm_machine_manager::get_instance();
	if (!mgr->ExecuteSql(query)) {
		ASSERT(0); JLOG(L"update zoneInfo failed.\n");
		return false;
	}
	_detectorInfo = detInfo;
	query.Format(L"update table_detector set zone_info_id=%d where id=%d",
					_id, detInfo->get_id());
	if (!mgr->ExecuteSql(query)) {
		ASSERT(0); JLOG(L"update table_detector failed.\n");
		return false;
	} 
	detInfo->set_zone_info_id(_id);
	detInfo->set_zone_value(_zone_value);
	return true;
}


bool zone_info::execute_del_detector_info()
{
	AUTO_LOG_FUNCTION;
	ASSERT(_detectorInfo);
	CString query;
	if (ZT_SUB_MACHINE_ZONE == _type) {
		query.Format(L"update table_sub_zone set detector_info_id=-1 where id=%d", _id);
	} else {
		query.Format(L"update table_zone set detector_info_id=-1 where id=%d", _id);
	}
	auto mgr = alarm_machine_manager::get_instance();
	if (!mgr->ExecuteSql(query)) {
		ASSERT(0); JLOG(L"update zoneInfo failed.\n");
		return false;
	}
	query.Format(L"delete from table_detector where id=%d",
				  _detectorInfo->get_id());
	if (!mgr->ExecuteSql(query)) {
		ASSERT(0); JLOG(L"delete table_detector failed.\n");
		return false;
	}
	mgr->DeleteDetector(_detectorInfo);
	_detectorInfo = nullptr;
	_detector_id = -1;
	if (!_mapInfo.expired()) {
		auto mapInfo = _mapInfo.lock();
		mapInfo->RemoveInterface(shared_from_this());
		mapInfo = nullptr;
	}
	return true;
}


bool zone_info::execute_bind_detector_info_to_map_info(const core::map_info_ptr& mapInfo)
{
	AUTO_LOG_FUNCTION;
	ASSERT(_detectorInfo); ASSERT(mapInfo);
	CString query;
	query.Format(L"update table_detector set map_id=%d where id=%d", 
				 mapInfo->get_id(), _detectorInfo->get_id());
	auto mgr = alarm_machine_manager::get_instance();
	if (!mgr->ExecuteSql(query)) {
		ASSERT(0); JLOG(L"update table_detector failed.\n");
		return false;
	}
	_detectorInfo->set_map_id(mapInfo->get_id());
	return true;
}


bool zone_info::execute_create_detector_info_and_bind_map_info(const detector_info_ptr& detInfo,
															   const core::map_info_ptr& mapInfo)
{
	AUTO_LOG_FUNCTION;
	ASSERT(_detectorInfo == nullptr); 
	ASSERT(detInfo); ASSERT(mapInfo);
	CString query;
	query.Format(L"insert into table_detector ([map_id],[zone_info_id],[x],[y],[distance],[angle],[detector_lib_id]) values(%d,%d,%d,%d,%d,%d,%d)",
				 mapInfo->get_id(), _id, detInfo->get_x(), detInfo->get_y(),
				 detInfo->get_distance(), detInfo->get_angle(),
				 detInfo->get_detector_lib_id());
	auto mgr = alarm_machine_manager::get_instance();
	int id = mgr->AddAutoIndexTableReturnID(query);
	if (-1 == id) {
		ASSERT(0); JLOG(L"insert detector info failed.\n"); return false;
	}
	if (ZT_SUB_MACHINE_ZONE == _type) {
		query.Format(L"update table_sub_zone set detector_info_id=%d where id=%d",
					 id, _id);
	} else {
		query.Format(L"update table_zone set detector_info_id=%d where id=%d",
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
	mgr->AddDetector(detInfo);
	_detector_id = id;
	if (!_mapInfo.expired()) {
		_mapInfo.lock()->RemoveInterface(shared_from_this());
	}
	_mapInfo = mapInfo;
	mapInfo->AddInterface(shared_from_this());
	return true;
}


bool zone_info::execute_set_physical_addr(int addr)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update table_zone set physical_addr=%d where id=%d",
				 addr, _id);
	auto mgr = alarm_machine_manager::get_instance();
	if (!mgr->ExecuteSql(query)) {
		ASSERT(0); JLOG(L"update physical_addr failed.\n"); return false;
	}
	_physical_addr = addr;
	return true;
}


bool zone_info::execute_set_status_or_property(char status)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update table_zone set status_or_property=%d where id=%d",
				 status, _id);
	auto mgr = alarm_machine_manager::get_instance();
	if (!mgr->ExecuteSql(query)) {
		ASSERT(0); JLOG(L"update _status_or_property failed.\n"); return false;
	}
	_status_or_property = status;
	return true;
}


void zone_info::DoClick()
{
	InversionControl(ICZC_CLICK);
}


void zone_info::DoRClick()
{
	InversionControl(ICZC_RCLICK);
}
};
