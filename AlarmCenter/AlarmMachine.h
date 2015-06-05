#pragma once
#include <list>
#include "core.h"
#include "baidu.h"

namespace core {

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
	
class CZoneInfo;
typedef CZoneInfo* PZone;
typedef std::list<CZoneInfo*> CZoneInfoList;
typedef std::list<CZoneInfo*>::iterator CZoneInfoListIter;
class CMapInfo;
typedef std::list<CMapInfo*> CMapInfoList;
typedef std::list<CMapInfo*>::iterator CMapInfoListIter;
//typedef void(__stdcall *TraverseZoneOfMapCB)(void* udata, CZoneInfo* zone);

typedef void(__stdcall *OnOtherTryEnterBufferMode)(void* udata);
typedef struct OnOtherTryEnterBufferModeObj
{
private:
	OnOtherTryEnterBufferMode _cb;
	void* _udata;
public:
	OnOtherTryEnterBufferModeObj() : _cb(NULL), _udata(NULL) {}
	bool valid() const { return _cb && _udata; }
	void call() { if (valid())_cb(_udata); }
	void update(OnOtherTryEnterBufferMode cb, void* udata) { _cb = cb; _udata = udata; }
	void reset() { _cb = NULL; _udata = NULL; }
}OnOtherTryEnterBufferModeObj;




using namespace ademco;

class CAlarmMachine
{ 
private:
	int _id;
	int _ademco_id;
	int _group_id;
	MachineType _machine_type;
	bool _banned;
	char _device_id[64];
	char _ipv4[64];
	wchar_t _device_idW[64];
	wchar_t* _alias;
	wchar_t* _contact;
	wchar_t* _address;
	wchar_t* _phone;
	wchar_t* _phone_bk;
	bool _online;
	bool _armed;
	bool _alarming;
	bool _has_alarming_direct_zone;
	bool _buffer_mode;
	bool _is_submachine;
	bool _has_video;
	volatile int _submachine_zone;
	volatile int _submachine_count;
	CMapInfo* _unbindZoneMap;
	CMapInfoList _mapList;
	CMapInfoListIter _curMapListIter;
	std::list<ademco::AdemcoEvent*> _ademcoEventList;
	std::list<ademco::AdemcoEvent*> _ademcoEventFilter;
	CLock _lock4AdemcoEventList;
	PZone _zoneArray[MAX_MACHINE_ZONE];
	CZoneInfoList _validZoneList;
	ConnHangupObj _connHangupObj;
	EventLevel _highestEventLevel;
	volatile long _alarmingSubMachineCount;
	time_t _lastActionTime;
	bool _bChecking;
	OnOtherTryEnterBufferModeObj _ootebmOjb;
	COleDateTime _expire_time;
	DWORD _last_time_check_if_expire;
	web::BaiduCoordinate _coor;
protected:
	void HandleAdemcoEvent(const ademco::AdemcoEvent* ademcoEvent, BOOL bDeleteAfterHandled = TRUE);
	void inc_alarmingSubMachineCount();
	void dec_alarmingSubMachineCount();
	void set_highestEventLevel(EventLevel level);
	void NotifySubmachines(const ademco::AdemcoEvent* ademcoEvent);
	void HandleRetrieveResult(const ademco::AdemcoEvent* ademcoEvent);
	void UpdateLastActionTime() { AUTO_LOG_FUNCTION; LOG(L"subMachine %03d, %s", _submachine_zone, _alias); _lastActionTime = time(NULL); }
	void SetAllSubMachineOnOffLine(bool online = true);
public:
	CAlarmMachine();
	~CAlarmMachine();
	//bool IsOnline() const { return _online; }
	//bool IsArmed() const { return _armed; }
	void clear_ademco_event_list();

	// 2015-05-18 16:42:58
	const char* get_ipv4() const { return _ipv4; }
	void set_ipv4(const char* ipv4) { if (ipv4) strcpy_s(_ipv4, ipv4); else memset(_ipv4, 0, sizeof(_ipv4)); }
	// 2015年5月6日 15:58:07 分机超过16小时则自动检测相关
	time_t GetLastActionTime() const { return _lastActionTime; }

	// 2015年4月22日 16:55:04 按钮颜色相关。分别清除所有分机信息后清除主机按钮颜色
	EventLevel get_highestEventLevel() const { return _highestEventLevel; }
	long get_alarmingSubMachineCount() const { return _alarmingSubMachineCount; }

	// 2015年4月16日 15:45:06 链路挂起相关
	void SetConnHangupCallback(void* udata, ConnHangupCB cb) { _connHangupObj.udata = udata; _connHangupObj.cb = cb; }

	// 2015年3月24日 17:45:11 分机相关
	void inc_submachine_count();
	void dec_submachine_count();
	int get_submachine_count() { /*AUTO_LOG_FUNCTION;*/ return _submachine_count; }

	// 2015年3月4日 14:29:34 防区操作
	void AddZone(CZoneInfo* zoneInfo);
	CZoneInfo* GetZone(int zone);
	// 2015年3月3日 14:16:10 获取所有防区信息
	void GetAllZoneInfo(CZoneInfoList& list);
	int get_zone_count() const { return _validZoneList.size(); }

	bool execute_set_coor(const web::BaiduCoordinate& coor);
	// 2015年2月25日 15:50:16 真正操作数据库的修改操作
	bool execute_set_banned(bool banned = true);
	bool execute_set_machine_type(MachineType type);
	bool execute_set_has_video(bool has);
	bool execute_set_armd(bool arm);
	bool execute_set_alias(const wchar_t* alias);
	bool execute_set_contact(const wchar_t* contact);
	bool execute_set_address(const wchar_t* address);
	bool execute_set_phone(const wchar_t* phone);
	bool execute_set_phone_bk(const wchar_t* phone_bk);
	bool execute_set_group_id(int group_id);
	bool execute_add_map(CMapInfo* mapInfo);
	bool execute_update_map_alias(CMapInfo* mapInfo, const wchar_t* alias);
	bool execute_update_map_path(CMapInfo* mapInfo, const wchar_t* path);
	bool execute_delete_map(CMapInfo* mapInfo);
	bool execute_update_expire_time(const COleDateTime& datetime);
	
	// 2015年3月16日 16:19:27 真正操作数据库的防区操作
	bool execute_add_zone(CZoneInfo* zoneInfo);
	bool execute_del_zone(CZoneInfo* zoneInfo);
	// 2015年2月12日 21:34:56
	// 当编辑某个主机时，该主机接收的所有事件都先缓存，退出编辑后再 notify observers.
	bool EnterBufferMode();
	bool LeaveBufferMode();
	// 2015年5月6日 21:03:22
	// 当EnterBufferMode时，设置此obj，以便其他地方调用EnterBufferMode时LeaveBufferMode
	void SetOotebmObj(OnOtherTryEnterBufferMode cb, void* udata) { _ootebmOjb.update(cb, udata); }

	void AddMap(CMapInfo* map) { _mapList.push_back(map); }
	//CMapInfo* GetFirstMap();
	//CMapInfo* GetNextMap();
	//void SetUnbindZoneMap(CMapInfo* map) { _unbindZoneMap = map; }
	CMapInfo* GetUnbindZoneMap() { return _unbindZoneMap; }
	void GetAllMapInfo(CMapInfoList& list);
	CMapInfo* GetMapInfo(int map_id);
	
	void SetAdemcoEvent(int ademco_event, int zone, int subzone, const time_t& event_time,
						const char* xdata, int xdata_len);
	//void SetAdemcoEvent(const ademco::AdemcoEvent* ademcoEvent);
	void TraverseAdmecoEventList(void* udata, ademco::AdemcoEventCB cb);

	const char* GetDeviceIDA() const { return _device_id; }
	const wchar_t* GetDeviceIDW() const { return _device_idW; }

	void set_device_id(const wchar_t* device_id);
	void set_device_id(const char* device_id);

	MachineType get_machine_type() const { return _machine_type; }
	void set_machine_type(MachineType type) { _machine_type = type; }

	DECLARE_GETTER_SETTER_INT(_id);
	DECLARE_GETTER_SETTER_INT(_ademco_id); 
	DECLARE_GETTER_SETTER_INT(_group_id);
	DECLARE_GETTER_SETTER(bool, _banned); 
	DECLARE_GETTER_SETTER(bool, _is_submachine); 
	DECLARE_GETTER_SETTER(bool, _alarming);
	DECLARE_GETTER_SETTER(bool, _online);
	DECLARE_GETTER_SETTER(bool, _armed);
	DECLARE_GETTER_SETTER(bool, _has_video); 
	DECLARE_GETTER_SETTER(bool, _bChecking);
	DECLARE_GETTER_SETTER_INT(_submachine_zone);

	DECLARE_GETTER_SETTER_STRING(_alias);
	DECLARE_GETTER_SETTER_STRING(_contact);
	DECLARE_GETTER_SETTER_STRING(_address);
	DECLARE_GETTER_SETTER_STRING(_phone);
	DECLARE_GETTER_SETTER_STRING(_phone_bk);
	DECLARE_GETTER_SETTER(COleDateTime, _expire_time);
	double get_left_service_time() const {
		COleDateTimeSpan span = _expire_time - COleDateTime::GetCurrentTime();
		return span.GetTotalMinutes();
	}
	DECLARE_GETTER_SETTER(web::BaiduCoordinate, _coor);

	DECLARE_OBSERVER(AdemcoEventCB, AdemcoEvent*);
	DECLARE_UNCOPYABLE(CAlarmMachine);
};



NAMESPACE_END
