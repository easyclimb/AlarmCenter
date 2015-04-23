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

enum InversionControlZoneCommand {
	ICZC_ALARM_START,	// 报警
	ICZC_ALARM_STOP,	// 消警
	ICZC_SET_FOCUS,		// 高亮
	ICZC_KILL_FOCUS,	// 取消高亮
	ICZC_ROTATE,		// 旋转
	ICZC_DISTANCE,		// 调整间距(仅针对对射探头)
	ICZC_MOVE,			// 移动
	ICZC_DESTROY,		// CZoneInfo已析构
};

typedef void(__stdcall *OnInversionControlZoneCB)(void* udata, 
												  InversionControlZoneCommand iczc,
												  DWORD dwExtra);

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
	//const char *__class_name;
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
	OnInversionControlZoneCB _cb;
	bool _alarming;
	EventLevel _highestEventLevel;
	std::list<ADEMCO_EVENT> _eventList;
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

	void SetAlarmCallback(void* udata, OnInversionControlZoneCB cb) { _udata = udata; _cb = cb; }

	bool get_alarming() const { return _alarming; }

	// 2015年3月17日 20:57:08 真正操作下属分机的操作，考虑由zoneinfo操作比较合适
	bool execute_set_sub_machine(CAlarmMachine* subMachine);
	bool execute_del_sub_machine();
	bool execute_update_alias(const wchar_t* alias);
	bool execute_update_contact(const wchar_t* contact);
	bool execute_update_address(const wchar_t* address);
	bool execute_update_phone(const wchar_t* phone);
	bool execute_update_phone_bk(const wchar_t* phone_bk);
	bool execute_set_detector_info(CDetectorInfo* detInfo);
	bool execute_rem_detector_info();
	bool execute_del_detector_info();
	bool execute_bind_detector_info_to_map_info(CMapInfo* mapInfo);
	bool execute_unbind_detector_info_from_map_info();
	bool execute_create_detector_info_and_bind_map_info(CDetectorInfo* detInfo,
														CMapInfo* mapInfo);
	enum DetectorInfoField {
		DIF_X,
		DIF_Y,
		DIF_DISTANCE,
		DIF_ANGLE,
	};
	bool execute_update_detector_info_field(DetectorInfoField dif, int value);

	void InversionControl(InversionControlZoneCommand iczc);

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
