#pragma once
#include <list>

namespace core {

static const int MAX_MACHINE_ZONE = 1000;

enum MachineType {
	MT_NORMAL = 0,	// 普通主机，显示地图
	MT_VEDIO,		// 视频主机，显示视频
	MT_MAX,
};
	
class CZoneInfo;
typedef CZoneInfo* PZone;
typedef std::list<CZoneInfo*> CZoneInfoList;
typedef std::list<CZoneInfo*>::iterator CZoneInfoListIter;
//typedef void(_stdcall *TraverseZoneOfMapCB)(void* udata, CZoneInfo* zone);

using namespace ademco;
class CMapInfo;
class CAlarmMachine
{
private:
	int _id;
	int _ademco_id;
	int _group_id;
	MachineType _type;
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
	bool _buffer_mode;
	bool _is_submachine;
	int _submachine_zone;
	CMapInfo* _unbindZoneMap;
	std::list<CMapInfo*> _mapList;
	std::list<CMapInfo*>::iterator _curMapListIter;
	//CZoneInfoList _zoneList;
	std::list<ademco::AdemcoEvent*> _ademcoEventList;
	CLock _lock4AdemcoEventList;
	PZone _zoneArray[MAX_MACHINE_ZONE];
	std::list<PZone> _validZoneList;
protected:
	//CZoneInfo* GetZoneInfo(int zone_id);
	void HandleAdemcoEvent(const ademco::AdemcoEvent* ademcoEvent);
	CMapInfo* GetMapInfo(int map_id);
public:
	CAlarmMachine();
	~CAlarmMachine();
	bool IsOnline() const { return _online; }
	bool IsArmed() const { return _armed; }
	void clear_ademco_event_list();

	// 2015年3月4日 14:29:34 防区操作
	void AddZone(CZoneInfo* zoneInfo);
	CZoneInfo* GetZone(int zone);

	// 2015年3月3日 14:16:10 获取所有防区信息
	void GetAllZoneInfo(CZoneInfoList& list);

	// 2015年2月25日 15:50:16 真正操作数据库的修改操作
	bool execute_set_banned(bool banned = true);
	bool execute_set_type(int type);
	bool execute_set_alias(const wchar_t* alias);
	bool execute_set_contact(const wchar_t* contact);
	bool execute_set_address(const wchar_t* address);
	bool execute_set_phone(const wchar_t* phone);
	bool execute_set_phone_bk(const wchar_t* phone_bk);
	bool execute_set_group_id(int group_id);
	
	// 2015年3月16日 16:19:27 真正操作数据库的防区操作
	bool execute_add_zone(CZoneInfo* zoneInfo);

	// 2015年2月12日 21:34:56
	// 当编辑某个主机时，该主机接收的所有事件都先缓存，退出编辑后再 notify observers.
	void EnterBufferMode();
	void LeaveBufferMode();

	void AddMap(CMapInfo* map) { _mapList.push_back(map); }
	CMapInfo* GetFirstMap();
	CMapInfo* GetNextMap();
	//void SetUnbindZoneMap(CMapInfo* map) { _unbindZoneMap = map; }
	CMapInfo* GetUnbindZoneMap() { return _unbindZoneMap; }
	
	void SetAdemcoEvent(int ademco_event, int zone, int subzone, const time_t& event_time);
	void SetAdemcoEvent(const ademco::AdemcoEvent* ademcoEvent);
	void TraverseAdmecoEventList(void* udata, ademco::AdemcoEventCB cb);

	const char* GetDeviceIDA() const { return _device_id; }
	const wchar_t* GetDeviceIDW() const { return _device_idW; }

	void set_device_id(const wchar_t* device_id);
	void set_device_id(const char* device_id);

	MachineType get_type() const { return _type; }
	void set_type(int type) { _type = Integer2MachineType(type); }

	static MachineType Integer2MachineType(int type) {
		switch (type) {
			case MT_VEDIO:				return MT_VEDIO;	break;
			case MT_NORMAL: default:	return MT_NORMAL;	break;
		}
	}

	DECLARE_GETTER_SETTER_INT(_id);
	DECLARE_GETTER_SETTER_INT(_ademco_id); 
	DECLARE_GETTER_SETTER_INT(_group_id);
	DECLARE_GETTER_SETTER(bool, _banned); 
	DECLARE_GETTER_SETTER(bool, _is_submachine);
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
