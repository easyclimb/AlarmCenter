#pragma once
#include <list>
#include <algorithm>


namespace core {

enum MachineType {
	MT_NORMAL = 0,	// 普通主机，显示地图
	MT_VEDIO,		// 视频主机，显示视频
	MT_MAX,
};
	
class CZoneInfo;

typedef void(_stdcall *TraverseZoneOfMapCB)(void* udata, CZoneInfo* zone);

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
	CMapInfo* _noZoneMap;
	std::list<CMapInfo*> _mapList;
	std::list<CMapInfo*>::iterator _curMapListIter;
	std::list<ademco::AdemcoEvent*> _ademcoEventList;
	CLock _lock4AdemcoEventList;
protected:
	CZoneInfo* GetZoneInfo(int zone_id);
	void HandleAdemcoEvent(ademco::AdemcoEvent* ademcoEvent);
public:
	CAlarmMachine();
	~CAlarmMachine();
	bool IsOnline() const { return _online; }
	bool IsArmed() const { return _armed; }
	void clear_ademco_event_list();

	// 2015年2月25日 15:50:16 真正操作数据库的修改操作
	bool execute_set_banned(bool banned = true);
	bool execute_set_type(int type);
	bool execute_set_alias(const wchar_t* alias);
	bool execute_set_contact(const wchar_t* contact);
	bool execute_set_address(const wchar_t* address);
	bool execute_set_phone(const wchar_t* phone);
	bool execute_set_phone_bk(const wchar_t* phone_bk);
	bool execute_set_group_id(int group_id);

	// 2015年2月12日 21:34:56
	// 当编辑某个主机时，该主机接收的所有事件都先缓存，退出编辑后再 notify observers.
	void EnterBufferMode();
	void LeaveBufferMode();

	void AddMap(CMapInfo* map) { _mapList.push_back(map); }
	CMapInfo* GetFirstMap();
	CMapInfo* GetNextMap();
	void SetNoZoneMap(CMapInfo* map) { _noZoneMap = map; }
	CMapInfo* GetNoZoneMap() { return _noZoneMap; }
	
	void SetAdemcoEvent(int zone, int ademco_event, const time_t& event_time);
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

	DEALARE_GETTER_SETTER_INT(_id);
	DEALARE_GETTER_SETTER_INT(_ademco_id); 
	DEALARE_GETTER_SETTER_INT(_group_id);
	DEALARE_GETTER_SETTER(bool, _banned);

	DECLARE_GETTER_SETTER_STRING(_alias);
	DECLARE_GETTER_SETTER_STRING(_contact);
	DECLARE_GETTER_SETTER_STRING(_address);
	DECLARE_GETTER_SETTER_STRING(_phone);
	DECLARE_GETTER_SETTER_STRING(_phone_bk);

	DECLARE_OBSERVER(AdemcoEventCB, AdemcoEvent*);
	DECLARE_UNCOPYABLE(CAlarmMachine);
};

NAMESPACE_END
