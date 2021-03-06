﻿#pragma once
#include <string>
#include <list>
#include <map>
#include "core.h"
#include "baidu.h"
#include "ademco_func.h"


//namespace SQLite { class Database; }

namespace core {

using namespace ademco;


typedef void(__stdcall *on_other_try_enter_buffer_mode)(void* udata);
typedef struct on_other_try_enter_buffer_mode_obj
{
private:
	on_other_try_enter_buffer_mode _cb;
	void* _udata;
public:
	on_other_try_enter_buffer_mode_obj() : _cb(nullptr), _udata(nullptr) {}
	bool valid() const { return _cb && _udata; }
	void call() { if (valid())_cb(_udata); }
	void update(on_other_try_enter_buffer_mode cb, void* udata) { _cb = cb; _udata = udata; }
	void reset() { _cb = nullptr; _udata = nullptr; }
}on_other_try_enter_buffer_mode_obj;


class alarm_machine :
	public std::enable_shared_from_this<alarm_machine>, 
	public dp::observable<AdemcoEventPtr>
{ 

private:
	int _id = 0;
	int _ademco_id = 0;
	int _group_id = 0;
	machine_type _machine_type = MT_UNKNOWN;
	bool _banned = false;
	char _ipv4[64] = { 0 };
	CString alias_ = L"";
	CString _contact = L"";
	CString _address = L"";
	CString _phone = L"";
	CString _phone_bk = L"";
	bool _online_by_direct_mode = false;
	bool _online_by_transmit_mode1 = false;
	bool _online_by_transmit_mode2 = false;
	machine_status _machine_status = machine_status::MACHINE_STATUS_UNKNOWN;
	bool _alarming = false;
	bool _has_alarming_direct_zone = false;
	bool _buffer_mode = false;
	bool _is_submachine = false;

	// 2016-11-26 14:52:01
	bool setting_mode_ = false;

	volatile int _submachine_zone = 0;
	volatile int _submachine_count = 0;
	map_info_ptr _unbindZoneMap = nullptr;
	map_info_list _mapList = {};
	map_info_list::iterator _curMapListIter = {};
	std::list<AdemcoEventPtr> _ademcoEventList = {};
	//std::list<AdemcoEventPtr> _ademcoEventFilter = {};
	AdemcoEventPtr ademco_event_filter_ = nullptr;
	std::recursive_mutex _lock4AdemcoEventList = {};
	std::map<int, zone_info_ptr> _zoneMap = {};
	remote_control_command_conn_obj _rcccObj = {};
	ademco::EventLevel _highestEventLevel = ademco::EventLevel::EVENT_LEVEL_STATUS;
	volatile long _alarmingSubMachineCount = 0;
	time_t _lastActionTime = {};
	bool _bChecking = {};
	on_other_try_enter_buffer_mode_obj _ootebmOjb = {};
	std::chrono::system_clock::time_point expire_time_ = {};
	// 2016-5-5 18:28:59 for service expire
	std::chrono::system_clock::time_point remind_time_ = {};

	std::chrono::steady_clock::time_point last_check_if_expire_time_ = {};
	web::BaiduCoordinate _coor = {};
	int _zoomLevel = 14;
	sms_config _sms_cfg = {};

	// 2015年8月1日 14:45:30 storaged in xml
	bool _auto_show_map_when_start_alarming = {};

	// 2015年8月18日 21:45:36 for qianfangming
	std::shared_ptr<ademco::PrivatePacket> privatePacket_from_server1_ = nullptr;
	std::shared_ptr<ademco::PrivatePacket> privatePacket_from_server2_ = nullptr;

	// 2016-1-23 15:46:05 for qianfangming sms mode
	bool _sms_mode = false;

	// 2016-3-3 17:52:12 for qianfangming retrieve zone info
	EventSource _last_time_event_source = ES_UNKNOWN;

	// 2016-4-9 18:15:40 for signal strength
	signal_strength signal_strength_ = SIGNAL_STRENGTH_5;
	int real_signal_strength_ = 0;

	// 2016-5-4 23:09:00 for service expire management
	consumer_ptr consumer_ = {};

	//2016-11-14 15:31:50 for alarm handle
	int alarm_id_ = 0;
	
protected:
	void HandleAdemcoEvent(const ademco::AdemcoEventPtr& ademcoEvent);
	void inc_alarmingSubMachineCount();
	void dec_alarmingSubMachineCount();
	void set_highestEventLevel(ademco::EventLevel level);
	void NotifySubmachines(const ademco::AdemcoEventPtr& ademcoEvent);
	void HandleRetrieveResult(const ademco::AdemcoEventPtr& ademcoEvent);
	void UpdateLastActionTime() { /*AUTO_LOG_FUNCTION; JLOG(L"subMachine %03d, %s", _submachine_zone, alias_); */_lastActionTime = time(nullptr); }
	void SetAllSubMachineOnOffLine(bool online = true);
	void handle_setting_mode();
	void handle_restore_factory_settings();
	void notify_observers_with_event(ademco::ADEMCO_EVENT evnt = ademco::EVENT_MACHINE_INFO_CHANGED);

public:
	alarm_machine();
	~alarm_machine();

	machine_uuid get_uuid() const;

	void clear_ademco_event_list(bool alarm_handled_over = false, bool clear_sub_machine = true);

	// 2016-5-5 10:41:37 for service management
	void set_consumer(const consumer_ptr& consumer);
	consumer_ptr get_consumer() const { return consumer_; }

	// 2016-3-3 17:53:12 for qianfangming retrieve
	ademco::EventSource get_last_time_event_source() const { return _last_time_event_source; }

	// 2015年8月18日 21:57:55 qianfangming
	void SetPrivatePacketFromServer1(const ademco::PrivatePacket* privatePacket);
	void SetPrivatePacketFromServer2(const ademco::PrivatePacket* privatePacket);
	const ademco::PrivatePacketPtr GetPrivatePacketFromServer1() const;
	const ademco::PrivatePacketPtr GetPrivatePacketFromServer2() const;

	// 2015年7月13日 14:20:38 kill connection
	void kill_connction() { if (_rcccObj.valid()) { _rcccObj.cb(_rcccObj.udata, RCCC_DISCONN); } }

	// 2015-05-18 16:42:58
	const char* get_ipv4() const { return _ipv4; }
	void set_ipv4(const char* ipv4) { if (ipv4) strcpy_s(_ipv4, ipv4); else memset(_ipv4, 0, sizeof(_ipv4)); }
	// 2015年5月6日 15:58:07 分机超过16小时则自动检测相关 // 2016年12月8日17:32:24修改，主机也用此值记录上次动作的事件
	time_t GetLastActionTime() const { return _lastActionTime; }

	// 2015年4月22日 16:55:04 按钮颜色相关。分别清除所有分机信息后清除主机按钮颜色
	ademco::EventLevel get_highestEventLevel() const { return _highestEventLevel; }
	long get_alarmingSubMachineCount() const { return _alarmingSubMachineCount; }

	// 2015年4月16日 15:45:06 链路挂起相关
	void SetConnHangupCallback(const net::server::CClientDataPtr& udata, remote_control_command_conn_call_back cb) { _rcccObj.udata = udata; _rcccObj.cb = cb; }

	// 2015年3月24日 17:45:11 分机相关
	void inc_submachine_count();
	void dec_submachine_count();
	int get_submachine_count() { /*AUTO_LOG_FUNCTION;*/ return _submachine_count; }

	// 2015年3月4日 14:29:34 防区操作
	void AddZone(const zone_info_ptr& zoneInfo);
	zone_info_ptr GetZone(int zone);

	// 2015年3月3日 14:16:10 获取所有防区信息
	void GetAllZoneInfo(zone_info_list& list);
	int get_zone_count() const { return _zoneMap.size(); }

	bool execute_set_coor(const web::BaiduCoordinate& coor);
	// 2015年2月25日 15:50:16 真正操作数据库的修改操作
	bool execute_set_banned(bool banned = true);
	bool execute_set_machine_type(machine_type type);
	bool execute_set_machine_status(machine_status status);
	bool execute_set_alias(const wchar_t* alias);
	bool execute_set_contact(const wchar_t* contact);
	bool execute_set_address(const wchar_t* address);
	bool execute_set_phone(const wchar_t* phone);
	bool execute_set_phone_bk(const wchar_t* phone_bk);
	bool execute_set_group_id(int group_id);
	bool execute_add_map(const core::map_info_ptr& mapInfo);
	bool execute_update_map_alias(const core::map_info_ptr& mapInfo, const wchar_t* alias);
	bool execute_update_map_path(const core::map_info_ptr& mapInfo, const wchar_t* path);
	bool execute_delete_map(const core::map_info_ptr& mapInfo);
	bool execute_update_expire_time(const std::chrono::system_clock::time_point& datetime);
	
	// 2015年3月16日 16:19:27 真正操作数据库的防区操作
	bool execute_add_zone(const zone_info_ptr& zoneInfo);
	bool execute_del_zone(const zone_info_ptr& zoneInfo);
	// 2015年2月12日 21:34:56
	// 当编辑某个主机时，该主机接收的所有事件都先缓存，退出编辑后再 notify observers.
	bool EnterBufferMode();
	bool LeaveBufferMode();
	// 2015年5月6日 21:03:22
	// 当EnterBufferMode时，设置此obj，以便其他地方调用EnterBufferMode时LeaveBufferMode
	void SetOotebmObj(on_other_try_enter_buffer_mode cb, void* udata) { _ootebmOjb.update(cb, udata); }

	void AddMap(map_info_ptr map) { _mapList.push_back(map); }
	map_info_ptr GetUnbindZoneMap() { return _unbindZoneMap; }
	void GetAllMapInfo(map_info_list& list);
	map_info_ptr GetMapInfo(int map_id);
	
	void SetAdemcoEvent(ademco::EventSource source,
						ademco::ADEMCO_EVENT ademco_event,
						int zone, int subzone,
						const time_t& timestamp,
						const time_t& recv_time,
						const ademco::char_array_ptr& xdata = nullptr);

	void TraverseAdmecoEventList(const observer_ptr& obj);

	machine_type get_machine_type() const { return _machine_type; }
	void set_machine_type(machine_type type) { _machine_type = type; }

	DECLARE_GETTER_SETTER_INT(_id);
	DECLARE_GETTER_SETTER_INT(_ademco_id); 
	DECLARE_GETTER_SETTER_INT(_group_id);
	DECLARE_GETTER_SETTER(bool, _banned); 
	DECLARE_GETTER_SETTER(bool, _is_submachine); 
	DECLARE_GETTER_SETTER(bool, _alarming);
	//DECLARE_GETTER_SETTER(bool, _online);
	bool get_online_by_direct_mode() const { return _online_by_direct_mode; }
	bool get_online_by_transmit_mode1() const { return _online_by_transmit_mode1; }
	bool get_online_by_transmit_mode2() const { return _online_by_transmit_mode2; }
	bool get_online() const { return _online_by_direct_mode || _online_by_transmit_mode1 || _online_by_transmit_mode2; }
	void set_online(bool online) { _online_by_direct_mode = _online_by_transmit_mode1 = _online_by_transmit_mode2 = online; }
	void set_online(const alarm_machine_ptr& rhs) {
		_online_by_direct_mode = rhs->_online_by_direct_mode;
		_online_by_transmit_mode1 = rhs->_online_by_transmit_mode1;
		_online_by_transmit_mode2 = rhs->_online_by_transmit_mode2;
	}
	DECLARE_GETTER_SETTER(machine_status, _machine_status);
	DECLARE_GETTER_SETTER(bool, _bChecking);
	DECLARE_GETTER_SETTER_INT(_submachine_zone);

	void set_alias(const CString& alias) { alias_ = alias; }
	CString get_machine_name() const { return alias_; }
	CString get_formatted_name(bool show_parent_name_if_has_parent = true) const;
	CString get_formatted_info(const CString& seperator) const;

	DECLARE_GETTER_SETTER(CString, _contact);
	DECLARE_GETTER_SETTER(CString, _address);
	DECLARE_GETTER_SETTER(CString, _phone);
	DECLARE_GETTER_SETTER(CString, _phone_bk);

	std::chrono::system_clock::time_point get_expire_time() const { return expire_time_; }
	void set_expire_time(const std::chrono::system_clock::time_point& tp) { expire_time_ = tp; }

	int get_left_service_time_in_minutes() const {
		auto now = std::chrono::system_clock::now();
		auto diff = expire_time_ - now;
		auto minutes = std::chrono::duration_cast<std::chrono::minutes>(diff);
		return minutes.count();
	}

	DECLARE_GETTER_SETTER(web::BaiduCoordinate, _coor);
	DECLARE_GETTER(int, _zoomLevel);
	void set_zoomLevel(int level) {
		if (level != _zoomLevel) {
			if (19 < level || level < 1) level = 14;
			_zoomLevel = level;
		}
	}
	bool execute_set_zoomLevel(int zoomLevel);
	DECLARE_GETTER_SETTER(bool, _auto_show_map_when_start_alarming);
	bool execute_set_auto_show_map_when_start_alarming(bool b);

	DECLARE_GETTER_SETTER(sms_config, _sms_cfg);
	bool execute_set_sms_cfg(const sms_config& cfg);

	DECLARE_GETTER_SETTER(bool, _sms_mode);

	int get_real_signal_strength() const { return real_signal_strength_; }
	signal_strength get_signal_strength() const { return signal_strength_; }
	void set_signal_strength(signal_strength strength) { signal_strength_ = strength; }
	void set_signal_strength(const alarm_machine_ptr& rhs) {
		real_signal_strength_ = rhs->real_signal_strength_;
		signal_strength_ = rhs->signal_strength_;
	}

	// 2016-11-14 15:32:27 for alarm handle. 
	// alarm_id == 0 means this machine is not handled or hanlded over, no matter its alarming or not.
	// other values means it is alarming and is handling.
	// after is handled, alarm_id needs to be reset to 0.
	int get_alarm_id() const { return alarm_id_; }
	void set_alarm_id(int id);


	// 2016-11-26 17:39:04 
	bool get_set_mode() const { return setting_mode_; }
};



};

