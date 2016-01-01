#pragma once
#include <string>
#include <list>
#include <map>
#include "core.h"
#include "baidu.h"
#include "ademco_func.h"

namespace core {

using namespace ademco;
//namespace ademco { class PrivatePacket; };

static const int MAX_MACHINE_ZONE = 1000;
static const int MAX_SUBMACHINE_ZONE = 100;

typedef enum MachineType {
	MT_UNKNOWN = 0,
	MT_WIFI,		// wifi主机
	MT_NETMOD,		// 带网络模块的工程主机
	MT_GPRS,		// gprs主机
	MT_MAX,
}MachineType;

static MachineType Integer2MachineType(int type)
{
	switch (type) {
		case MT_WIFI:	return MT_WIFI;		break;
		case MT_NETMOD:	return MT_NETMOD;	break;
		case MT_GPRS: 	return MT_GPRS;		break;
		default:		return MT_UNKNOWN;	break;
	}
}
	

typedef void(__stdcall *OnOtherTryEnterBufferMode)(void* udata);
typedef struct OnOtherTryEnterBufferModeObj
{
private:
	OnOtherTryEnterBufferMode _cb;
	void* _udata;
public:
	OnOtherTryEnterBufferModeObj() : _cb(nullptr), _udata(nullptr) {}
	bool valid() const { return _cb && _udata; }
	void call() { if (valid())_cb(_udata); }
	void update(OnOtherTryEnterBufferMode cb, void* udata) { _cb = cb; _udata = udata; }
	void reset() { _cb = nullptr; _udata = nullptr; }
}OnOtherTryEnterBufferModeObj;


//typedef std::list<RemoteControlCommand*> RemoteControlCommandQueue;


class CAlarmMachine : public std::enable_shared_from_this<CAlarmMachine>
{ 
private:
	int _id;
	int _ademco_id;
	int _group_id;
	MachineType _machine_type;
	bool _banned;
	//char _device_id[64];
	char _ipv4[64];
	//wchar_t _device_idW[64];
	CString _alias = L"";
	CString _contact = L"";
	CString _address = L"";
	CString _phone = L"";
	CString _phone_bk = L"";
	bool _online;
	MachineStatus _machine_status;
	bool _alarming;
	bool _has_alarming_direct_zone;
	bool _buffer_mode;
	bool _is_submachine;
	bool _has_video;
	volatile int _submachine_zone;
	volatile int _submachine_count;
	CMapInfoPtr _unbindZoneMap;
	CMapInfoList _mapList;
	CMapInfoList::iterator _curMapListIter;
	std::list<AdemcoEventPtr> _ademcoEventList;
	std::list<AdemcoEventPtr> _ademcoEventFilter;
	CLock _lock4AdemcoEventList;
	std::map<int, CZoneInfoPtr> _zoneMap;
	CZoneInfoList _validZoneList;
	RemoteControlCommandConnObj _rcccObj;
	ademco::EventLevel _highestEventLevel;
	volatile long _alarmingSubMachineCount;
	time_t _lastActionTime;
	bool _bChecking;
	OnOtherTryEnterBufferModeObj _ootebmOjb;
	COleDateTime _expire_time;
	DWORD _last_time_check_if_expire;
	web::BaiduCoordinate _coor;
	int _zoomLevel;
	SmsConfigure _sms_cfg;
	//std::list<RemoteControlCommand*> _rccList;

	// 2015年8月1日 14:45:30 storaged in xml
	bool _auto_show_map_when_start_alarming;

	// 2015年8月18日 21:45:36 for qianfangming
	ademco::PrivatePacket* _privatePacket;

	// 2015-11-18 16:34:57 for show camera icon on mapview
	//CCameraInfoList _cameraList;
protected:
	void HandleAdemcoEvent(AdemcoEventPtr ademcoEvent);
	void inc_alarmingSubMachineCount();
	void dec_alarmingSubMachineCount();
	void set_highestEventLevel(ademco::EventLevel level);
	void NotifySubmachines(AdemcoEventPtr ademcoEvent);
	void HandleRetrieveResult(AdemcoEventPtr ademcoEvent);
	void UpdateLastActionTime() { AUTO_LOG_FUNCTION; JLOG(L"subMachine %03d, %s", _submachine_zone, _alias); _lastActionTime = time(nullptr); }
	void SetAllSubMachineOnOffLine(bool online = true);
	std::string get_xml_path();
	
public:
	CAlarmMachine();
	~CAlarmMachine();

	void clear_ademco_event_list();

	// 2015年8月18日 21:57:55 qianfangming
	void SetPrivatePacket(const ademco::PrivatePacket* privatePacket);
	const ademco::PrivatePacket* GetPrivatePacket() const;


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
	void SetConnHangupCallback(void* udata, RemoteControlCommandConnCB cb) { _rcccObj.udata = udata; _rcccObj.cb = cb; }

	// 2015年3月24日 17:45:11 分机相关
	void inc_submachine_count();
	void dec_submachine_count();
	int get_submachine_count() { /*AUTO_LOG_FUNCTION;*/ return _submachine_count; }

	// 2015年3月4日 14:29:34 防区操作
	void AddZone(CZoneInfoPtr zoneInfo);
	CZoneInfoPtr GetZone(int zone);

	// 2015年3月3日 14:16:10 获取所有防区信息
	void GetAllZoneInfo(CZoneInfoList& list);
	int get_zone_count() const { return _validZoneList.size(); }

	bool execute_set_coor(const web::BaiduCoordinate& coor);
	// 2015年2月25日 15:50:16 真正操作数据库的修改操作
	bool execute_set_banned(bool banned = true);
	bool execute_set_machine_type(MachineType type);
	bool execute_set_has_video(bool has);
	bool execute_set_machine_status(MachineStatus status);
	bool execute_set_alias(const wchar_t* alias);
	bool execute_set_contact(const wchar_t* contact);
	bool execute_set_address(const wchar_t* address);
	bool execute_set_phone(const wchar_t* phone);
	bool execute_set_phone_bk(const wchar_t* phone_bk);
	bool execute_set_group_id(int group_id);
	bool execute_add_map(CMapInfoPtr mapInfo);
	bool execute_update_map_alias(CMapInfoPtr mapInfo, const wchar_t* alias);
	bool execute_update_map_path(CMapInfoPtr mapInfo, const wchar_t* path);
	bool execute_delete_map(CMapInfoPtr mapInfo);
	bool execute_update_expire_time(const COleDateTime& datetime);
	
	// 2015年3月16日 16:19:27 真正操作数据库的防区操作
	bool execute_add_zone(CZoneInfoPtr zoneInfo);
	bool execute_del_zone(CZoneInfoPtr zoneInfo);
	// 2015年2月12日 21:34:56
	// 当编辑某个主机时，该主机接收的所有事件都先缓存，退出编辑后再 notify observers.
	bool EnterBufferMode();
	bool LeaveBufferMode();
	// 2015年5月6日 21:03:22
	// 当EnterBufferMode时，设置此obj，以便其他地方调用EnterBufferMode时LeaveBufferMode
	void SetOotebmObj(OnOtherTryEnterBufferMode cb, void* udata) { _ootebmOjb.update(cb, udata); }

	void AddMap(CMapInfoPtr map) { _mapList.push_back(map); }
	CMapInfoPtr GetUnbindZoneMap() { return _unbindZoneMap; }
	void GetAllMapInfo(CMapInfoList& list);
	CMapInfoPtr GetMapInfo(int map_id);
	
	void SetAdemcoEvent(ademco::EventSource resource,
						int ademco_event,
						int zone, int subzone,
						const time_t& timestamp,
						const time_t& recv_time,
						const std::vector<char>& xdata);

	void TraverseAdmecoEventList(void* udata, ademco::AdemcoEventCB cb);

	//const char* GetDeviceIDA() const { return _device_id; }
	//const wchar_t* GetDeviceIDW() const { return _device_idW; }

	//void set_device_id(const wchar_t* device_id);
	//void set_device_id(const char* device_id);

	MachineType get_machine_type() const { return _machine_type; }
	void set_machine_type(MachineType type) { _machine_type = type; }

	DECLARE_GETTER_SETTER_INT(_id);
	DECLARE_GETTER_SETTER_INT(_ademco_id); 
	DECLARE_GETTER_SETTER_INT(_group_id);
	DECLARE_GETTER_SETTER(bool, _banned); 
	DECLARE_GETTER_SETTER(bool, _is_submachine); 
	DECLARE_GETTER_SETTER(bool, _alarming);
	DECLARE_GETTER_SETTER(bool, _online);
	DECLARE_GETTER_SETTER(MachineStatus, _machine_status);
	DECLARE_GETTER_SETTER(bool, _has_video); 
	DECLARE_GETTER_SETTER(bool, _bChecking);
	DECLARE_GETTER_SETTER_INT(_submachine_zone);

	DECLARE_GETTER_SETTER(CString, _alias);
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
	DECLARE_GETTER_SETTER(SmsConfigure, _sms_cfg);
	DECLARE_GETTER(bool, _auto_show_map_when_start_alarming);
	void set_auto_show_map_when_start_alarming(bool b);

	DECLARE_OBSERVER(AdemcoEventCB, AdemcoEventPtr);
	DECLARE_UNCOPYABLE(CAlarmMachine);
};



NAMESPACE_END
