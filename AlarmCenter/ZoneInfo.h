#pragma once
#include <list>

namespace core
{

// sub_zone: 
// 0 for zone of machine
// between 0 and 0xEE for zone of sub machine
// 0xee for sub machine
static const int INDEX_ZONE			= 0;
static const int INDEX_SUB_MACHINE	= 0xEE;

typedef void(__stdcall *OnAlarmCB)(void* udata, bool alarm);

enum ZoneType {
	ZT_ZONE,				// 主机防区
	ZT_SUB_MACHINE,			// 分机
	ZT_SUB_MACHINE_ZONE,	// 分机防区
};

using namespace ademco;
class CDetectorInfo;
//class CSubMachineInfo;
class CAlarmMachine;
class CMapInfo;

class CZoneInfo
{
	const char *__class_name;
private:
	int _id;
	int _ademco_id;
	int _zone_value;
	int _sub_zone;
	//int _map_id;
	//int _type;
	ZoneType _type;
	int _detector_id;
	int _sub_machine_id;
	//int _property_id;
	wchar_t* _alias;
	CDetectorInfo* _detectorInfo;
	CAlarmMachine* _subMachineInfo;
	CMapInfo* _mapInfo;
	void* _udata;
	OnAlarmCB _cb;
	bool _alarming;
public:
	DECLARE_GETTER_SETTER_INT(_id);
	DECLARE_GETTER_SETTER_INT(_zone_value);
	DECLARE_GETTER_SETTER_INT(_sub_zone);
	DECLARE_GETTER_SETTER_INT(_ademco_id);
	//DECLARE_GETTER_SETTER_INT(_map_id);
	//DECLARE_GETTER_SETTER_INT(_type);
	ZoneType get_type() const { return _type; }
	void set_type(int type) { _type = Integer2ZoneType(type); }

	DECLARE_GETTER_SETTER_INT(_detector_id); 
	DECLARE_GETTER_SETTER_INT(_sub_machine_id);
	DECLARE_GETTER_SETTER_STRING(_alias);

	CZoneInfo();
	~CZoneInfo();

	void SetDetectorInfo(CDetectorInfo* detectorInfo) {
		_detectorInfo = detectorInfo;
	}

	CDetectorInfo* GetDetectorInfo() const { return _detectorInfo; }

	void SetSubMachineInfo(CAlarmMachine* subMachine) {
		assert(subMachine);
		_subMachineInfo = subMachine;
	}

	CAlarmMachine* GetSubMachineInfo() const { return _subMachineInfo; }

	void SetMapInfo(CMapInfo* mapInfo) { _mapInfo = mapInfo; }
	CMapInfo* GetMapInfo() const { return _mapInfo; }

	void HandleAdemcoEvent(const ademco::AdemcoEvent* ademcoEvent);

	void SetAlarmCallback(void* udata, OnAlarmCB cb) { _udata = udata; _cb = cb; }

	bool get_alarming() const { return _alarming; }

	// 2015年3月17日 20:57:08 真正操作下属分机的操作，考虑由zoneinfo操作比较合适
	bool execute_set_sub_machine(CAlarmMachine* subMachine);
	bool execute_del_sub_machine();

protected:
	static ZoneType Integer2ZoneType(int type) {
		switch (type) {
			case ZT_SUB_MACHINE:		return ZT_SUB_MACHINE;		break;
			case ZT_SUB_MACHINE_ZONE:	return ZT_SUB_MACHINE_ZONE; break;
			case ZT_ZONE: default:		return ZT_ZONE;				break;
		}
	}

	
	DECLARE_UNCOPYABLE(CZoneInfo);
};


NAMESPACE_END
