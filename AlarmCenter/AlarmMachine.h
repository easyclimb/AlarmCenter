#pragma once
#include <list>
#include "core.h"

namespace core {

static const int MAX_MACHINE_ZONE = 1000;

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
//typedef void(_stdcall *TraverseZoneOfMapCB)(void* udata, CZoneInfo* zone);


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
	int _submachine_zone;
	int _submachine_count;
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
	long _alarmingSubMachineCount;
protected:
	void HandleAdemcoEvent(const ademco::AdemcoEvent* ademcoEvent, BOOL bDeleteAfterHandled = TRUE);
	void inc_alarmingSubMachineCount();
	void dec_alarmingSubMachineCount();
	void set_highestEventLevel(EventLevel level);
	void NotifySubmachines(const ademco::AdemcoEvent* ademcoEvent);
	void HandleQueryResult(const ademco::AdemcoEvent* ademcoEvent);
public:
	CAlarmMachine();
	~CAlarmMachine();
	//bool IsOnline() const { return _online; }
	//bool IsArmed() const { return _armed; }
	void clear_ademco_event_list();

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

	// 2015年2月25日 15:50:16 真正操作数据库的修改操作
	bool execute_set_banned(bool banned = true);
	bool execute_set_machine_type(MachineType type);
	bool execute_set_has_video(bool has);
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
	
	// 2015年3月16日 16:19:27 真正操作数据库的防区操作
	bool execute_add_zone(CZoneInfo* zoneInfo);
	bool execute_del_zone(CZoneInfo* zoneInfo);
	// 2015年2月12日 21:34:56
	// 当编辑某个主机时，该主机接收的所有事件都先缓存，退出编辑后再 notify observers.
	bool EnterBufferMode();
	bool LeaveBufferMode();

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
	DECLARE_GETTER_SETTER_INT(_submachine_zone);

	DECLARE_GETTER_SETTER_STRING(_alias);
	DECLARE_GETTER_SETTER_STRING(_contact);
	DECLARE_GETTER_SETTER_STRING(_address);
	DECLARE_GETTER_SETTER_STRING(_phone);
	DECLARE_GETTER_SETTER_STRING(_phone_bk);

	DECLARE_OBSERVER(AdemcoEventCB, AdemcoEvent*);
	DECLARE_UNCOPYABLE(CAlarmMachine);
};



NAMESPACE_END
