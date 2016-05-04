#pragma once
#include <string>
#include <list>
#include <map>
#include "core.h"
#include "baidu.h"
#include "ademco_func.h"
#include "observer.h"

namespace core {

using namespace ademco;
//namespace ademco { class PrivatePacket; };

static const int MAX_MACHINE_ZONE = 1000;
static const int MAX_SUBMACHINE_ZONE = 100;

typedef enum machine_type {
	MT_UNKNOWN = 0,
	MT_WIFI,		// wifi主机
	MT_NETMOD,		// 带网络模块的工程主机
	MT_IMPRESSED_GPRS_MACHINE_2050,		// 改进型卧式主机2050型
	MT_MAX,
}machine_type;

static machine_type Integer2MachineType(int type)
{
	switch (type) {
		case MT_WIFI:	return MT_WIFI;		break;
		case MT_NETMOD:	return MT_NETMOD;	break;
		case MT_IMPRESSED_GPRS_MACHINE_2050: 	return MT_IMPRESSED_GPRS_MACHINE_2050;		break;
		default:		return MT_UNKNOWN;	break;
	}
}

typedef enum signal_strangth{
	SIGNAL_STRENGTH_0,
	SIGNAL_STRENGTH_1,
	SIGNAL_STRENGTH_2,
	SIGNAL_STRENGTH_3,
	SIGNAL_STRENGTH_4,
	SIGNAL_STRENGTH_5,
}signal_strangth;

static signal_strangth Integer2SignalStrength(int strength) {
	if (0 <= strength && strength <= 5) {
		return SIGNAL_STRENGTH_0;
	} else if (6 <= strength && strength <= 10) {
		return SIGNAL_STRENGTH_1;
	} else if (11 <= strength && strength <= 15) {
		return SIGNAL_STRENGTH_2;
	} else if (16 <= strength && strength <= 20) {
		return SIGNAL_STRENGTH_3;
	} else if (21 <= strength && strength <= 25) {
		return SIGNAL_STRENGTH_4;
	} else {
		return SIGNAL_STRENGTH_5;
	}
}
	

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


struct consumer_type {
	int id;
	CString name;
};


class consumer_type_manager
{

};

//typedef std::shared_ptr<ConsumerType> ConsumerTypePtr;

struct consumer {
	int id;
	consumer_type type;
	int receivable_amount;
	int paid_amount;
	
	int get_owed_amount() const { return receivable_amount - paid_amount; }
};


//typedef std::list<RemoteControlCommand*> RemoteControlCommandQueue;


class alarm_machine : public std::enable_shared_from_this<alarm_machine>, public dp::observable<AdemcoEventPtr>
{ 
private:
	int _id;
	int _ademco_id;
	int _group_id;
	machine_type _machine_type;
	bool _banned;
	char _ipv4[64];
	CString alias_ = L"";
	CString _contact = L"";
	CString _address = L"";
	CString _phone = L"";
	CString _phone_bk = L"";
	bool _online_by_direct_mode = false;
	bool _online_by_transmit_mode1 = false;
	bool _online_by_transmit_mode2 = false;
	machine_status _machine_status;
	bool _alarming;
	bool _has_alarming_direct_zone;
	bool _buffer_mode;
	bool _is_submachine;
	bool _has_video;
	volatile int _submachine_zone;
	volatile int _submachine_count;
	map_info_ptr _unbindZoneMap;
	map_info_list _mapList;
	map_info_list::iterator _curMapListIter;
	std::list<AdemcoEventPtr> _ademcoEventList;
	std::list<AdemcoEventPtr> _ademcoEventFilter;
	std::recursive_mutex _lock4AdemcoEventList;
	std::map<int, zone_info_ptr> _zoneMap;
	remote_control_command_conn_obj _rcccObj;
	ademco::EventLevel _highestEventLevel;
	volatile long _alarmingSubMachineCount;
	time_t _lastActionTime;
	bool _bChecking;
	on_other_try_enter_buffer_mode_obj _ootebmOjb;
	COleDateTime _expire_time;
	DWORD _last_time_check_if_expire;
	web::BaiduCoordinate _coor;
	int _zoomLevel;
	sms_config _sms_cfg;

	// 2015年8月1日 14:45:30 storaged in xml
	bool _auto_show_map_when_start_alarming;

	// 2015年8月18日 21:45:36 for qianfangming
	std::shared_ptr<ademco::PrivatePacket> _privatePacket;

	// 2016-1-23 15:46:05 for qianfangming sms mode
	bool _sms_mode = false;

	// 2016-3-3 17:52:12 for qianfangming retrieve zone info
	EventSource _last_time_event_source = ES_UNKNOWN;

	// 2016-4-9 18:15:40 for signal strength
	signal_strangth signal_strength_ = SIGNAL_STRENGTH_5;
	int real_signal_strength_ = 0;
	
protected:
	void HandleAdemcoEvent(const ademco::AdemcoEventPtr& ademcoEvent);
	void inc_alarmingSubMachineCount();
	void dec_alarmingSubMachineCount();
	void set_highestEventLevel(ademco::EventLevel level);
	void NotifySubmachines(const ademco::AdemcoEventPtr& ademcoEvent);
	void HandleRetrieveResult(const ademco::AdemcoEventPtr& ademcoEvent);
	void UpdateLastActionTime() { AUTO_LOG_FUNCTION; JLOG(L"subMachine %03d, %s", _submachine_zone, alias_); _lastActionTime = time(nullptr); }
	void SetAllSubMachineOnOffLine(bool online = true);
	std::string get_xml_path();
	
public:
	alarm_machine();
	~alarm_machine();

	void clear_ademco_event_list();

	// 2016-3-3 17:53:12 for qianfangming retrieve
	ademco::EventSource get_last_time_event_source() const { return _last_time_event_source; }

	// 2015年8月18日 21:57:55 qianfangming
	void SetPrivatePacket(const ademco::PrivatePacket* privatePacket);
	const ademco::PrivatePacketPtr GetPrivatePacket() const;


	void LoadXmlConfig();
	void SaveXmlConfig();
	// 2015-06-11 17:31:57 remote control 
	//void RemoteControl(int ademco_id, int ademco_event, int gg, int zone, const char* xdata, size_t xdata_len);

	// 2015年7月13日 14:20:38 kill connection
	void kill_connction() { if (_rcccObj.valid()) { _rcccObj.cb(_rcccObj.udata, RCCC_DISCONN); } }

	// 2015-05-18 16:42:58
	const char* get_ipv4() const { return _ipv4; }
	void set_ipv4(const char* ipv4) { if (ipv4) strcpy_s(_ipv4, ipv4); else memset(_ipv4, 0, sizeof(_ipv4)); }
	// 2015年5月6日 15:58:07 分机超过16小时则自动检测相关
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
	bool execute_set_has_video(bool has);
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
	bool execute_update_expire_time(const COleDateTime& datetime);
	
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
						int ademco_event,
						int zone, int subzone,
						const time_t& timestamp,
						const time_t& recv_time,
						const ademco::char_array_ptr& xdata = nullptr);

	void TraverseAdmecoEventList(const observer_ptr& obj);

	//const char* GetDeviceIDA() const { return _device_id; }
	//const wchar_t* GetDeviceIDW() const { return _device_idW; }

	//void set_device_id(const wchar_t* device_id);
	//void set_device_id(const char* device_id);

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
	DECLARE_GETTER_SETTER(machine_status, _machine_status);
	DECLARE_GETTER_SETTER(bool, _has_video); 
	DECLARE_GETTER_SETTER(bool, _bChecking);
	DECLARE_GETTER_SETTER_INT(_submachine_zone);

	//DECLARE_GETTER_SETTER(CString, _alias);
	void set_alias(const CString& alias) { alias_ = alias; }
	CString get_machine_name() const { return alias_; }
	CString get_formatted_machine_name() const {
		CString txt;
		if(_is_submachine)
			txt.Format(L"%03d(%s)", _submachine_zone, alias_); 
		else
			txt.Format(L"%06d(%s)", _ademco_id, alias_);
		return txt;
	}
	DECLARE_GETTER_SETTER(CString, _contact);
	DECLARE_GETTER_SETTER(CString, _address);
	DECLARE_GETTER_SETTER(CString, _phone);
	DECLARE_GETTER_SETTER(CString, _phone_bk);
	DECLARE_GETTER_SETTER(COleDateTime, _expire_time);
	double get_left_service_time() const {
		COleDateTimeSpan span = _expire_time - COleDateTime::GetCurrentTime();
		return span.GetTotalMinutes();
	}
	DECLARE_GETTER_SETTER(web::BaiduCoordinate, _coor);
	DECLARE_GETTER(int, _zoomLevel);
	void set_zoomLevel(int zoomLevel);
	DECLARE_GETTER_SETTER(sms_config, _sms_cfg);
	DECLARE_GETTER(bool, _auto_show_map_when_start_alarming);
	void set_auto_show_map_when_start_alarming(bool b);

	DECLARE_GETTER_SETTER(bool, _sms_mode);

	int get_real_signal_strength() const { return real_signal_strength_; }
	signal_strangth get_signal_strength() const { return signal_strength_; }
	void set_signal_strength(signal_strangth strength) { signal_strength_ = strength; }

	//DECLARE_OBSERVER(AdemcoEventCB, AdemcoEventPtr);
	DECLARE_UNCOPYABLE(alarm_machine);
};



NAMESPACE_END
