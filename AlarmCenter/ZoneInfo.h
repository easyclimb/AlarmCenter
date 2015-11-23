#pragma once
#include <list>

namespace core
{

typedef enum ZoneStatusOrProperty {
	// 状态
	ZS_ARM		= 0x01, // 布防
	ZS_DISARM	= 0x02, // 撤防
	// 属性
	ZP_GLOBAL	= 0xD0, // 全局
	ZP_HALF		= 0xD1, // 半局
	ZP_EMERGE	= 0xD2, // 紧急
	ZP_SHIELD	= 0xD3, // 屏蔽
	ZP_DOOR		= 0xD4, // 门铃
	ZP_FIRE		= 0xD5, // 火警
	ZP_DURESS	= 0xD6, // 胁迫
	ZP_GAS		= 0xD7, // 煤气
	ZP_WATER	= 0xD8, // 水警

	ZSOP_INVALID = 0xFF, // 哨兵
}ZoneStatusOrProperty;

static bool is_zone_status(unsigned char zsp)
{
	return ZS_ARM == zsp ||
		ZS_DISARM == zsp;
}

// sub_zone: 
// 0 for zone of machine
// between 0 and 0xEE for zone of sub machine
// 0xee for sub machine
static const int INDEX_ZONE			= 0;
static const int INDEX_SUB_MACHINE	= 0xEE;

// wire zone range
static const int WIRE_ZONE_RANGE_BEG = 0;
static const int WIRE_ZONE_RANGE_END = 7;

typedef enum InversionControlZoneCommand {
	ICZC_ALARM_START,	// 报警
	ICZC_ALARM_STOP,	// 消警
	ICZC_SET_FOCUS,		// 高亮
	ICZC_KILL_FOCUS,	// 取消高亮
	ICZC_ROTATE,		// 旋转
	ICZC_DISTANCE,		// 调整间距(仅针对对射探头)
	ICZC_MOVE,			// 移动
	ICZC_CLICK,			// 单击
	ICZC_RCLICK,		// 右击
	//ICZC_ALIAS_CHANGED, // 别名已修改
	ICZC_DESTROY,		// CZoneInfo已析构
}InversionControlZoneCommand;

typedef void(__stdcall *OnInversionControlZoneCB)(void* udata, 
												  InversionControlZoneCommand iczc,
												  DWORD dwExtra);

typedef enum ZoneType {
	ZT_ZONE,				// 主机防区
	ZT_SUB_MACHINE,			// 分机
	ZT_SUB_MACHINE_ZONE,	// 分机防区
}ZoneType;

using namespace ademco;
class CDetectorInfo;
//class CSubMachineInfo;
class CAlarmMachine;
class CMapInfo;


typedef enum DetectorInterfaceType
{
	DIT_ZONE_INFO = 0,
	DIT_CAMERA_INFO = 1,
}DetectorInterfaceType;

class CDetectorBindInterface
{
public:
	CDetectorBindInterface() {}
	virtual ~CDetectorBindInterface() {}
	virtual CDetectorInfo* GetDetectorInfo() const = 0;
	virtual void SetInversionControlCallback(void* udata, OnInversionControlZoneCB cb) = 0;
	virtual bool get_alarming() const = 0;
	virtual std::wstring FormatTooltip() const = 0;
	virtual void DoClick() = 0;
	virtual void DoRClick() = 0;
	virtual DetectorInterfaceType GetInterfaceType() const = 0;

	DECLARE_UNCOPYABLE(CDetectorBindInterface)
};



class CZoneInfo : public CDetectorBindInterface
{
	//const char *__class_name;
private:
	int _id;
	int _ademco_id;
	int _zone_value;
	int _sub_zone;
	int _status_or_property;
	int _physical_addr;
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
	std::list<InversionControlZoneCommand> _iczcCommandList;
	static const DetectorInterfaceType m_dit = DIT_ZONE_INFO;
public:
	virtual DetectorInterfaceType GetInterfaceType() const override { return m_dit; }
	static int char_to_status(char val);
	static char status_to_char(int val);
	DECLARE_GETTER_SETTER_INT(_id);
	DECLARE_GETTER_SETTER_INT(_zone_value);
	DECLARE_GETTER_SETTER_INT(_sub_zone);
	DECLARE_GETTER_SETTER_INT(_ademco_id); 
	DECLARE_GETTER_SETTER_INT(_status_or_property);
	DECLARE_GETTER_SETTER_INT(_physical_addr);
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

	// CDetectorBindInterface methods:
	virtual CDetectorInfo* GetDetectorInfo() const override { return _detectorInfo; }
	virtual std::wstring FormatTooltip() const override;
	virtual void DoClick() override;
	virtual void DoRClick() override;
	virtual void SetInversionControlCallback(void* udata, OnInversionControlZoneCB cb) override {
		_udata = udata; _cb = cb;
		if (udata && cb && _iczcCommandList.size() > 0) {
			for (auto iczc : _iczcCommandList) {
				cb(udata, iczc, 0);
			}
			_iczcCommandList.clear();
		}
	}
	virtual bool get_alarming() const override { return _alarming; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	void SetSubMachineInfo(CAlarmMachine* subMachine) {
		assert(subMachine);
		_subMachineInfo = subMachine;
	}

	CAlarmMachine* GetSubMachineInfo() const { return _subMachineInfo; }

	void SetMapInfo(CMapInfo* mapInfo) { _mapInfo = mapInfo; }
	CMapInfo* GetMapInfo() const { return _mapInfo; }

	void HandleAdemcoEvent(const ademco::AdemcoEvent* ademcoEvent);
	void InversionControl(InversionControlZoneCommand iczc);

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
	bool execute_set_physical_addr(int addr);
	bool execute_set_status_or_property(char status);

	typedef enum DetectorInfoField {
		DIF_X,
		DIF_Y,
		DIF_DISTANCE,
		DIF_ANGLE,
	}DetectorInfoField;
	bool execute_update_detector_info_field(DetectorInfoField dif, int value);

	

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


class CCameraInfo : public CDetectorBindInterface
{
public:
	explicit CCameraInfo() {}
	virtual ~CCameraInfo() {}

	void SetDetectorInfo(CDetectorInfo* detectorInfo) {
		_detectorInfo = detectorInfo;
	}
	// CDetectorBindInterface methods:
	virtual CDetectorInfo* GetDetectorInfo() const override { return _detectorInfo; }
	virtual std::wstring FormatTooltip() const override ;
	virtual void DoClick() override {};
	virtual void DoRClick() override {};
	virtual void SetInversionControlCallback(void*, OnInversionControlZoneCB) override {}
	virtual bool get_alarming() const override { return false; }
	virtual DetectorInterfaceType GetInterfaceType() const override { return m_dit; }

	DECLARE_GETTER_SETTER_INT(_ademco_id);
	DECLARE_GETTER_SETTER_INT(_sub_machine_id);
	DECLARE_GETTER_SETTER_INT(_device_info_id);
	DECLARE_GETTER_SETTER_INT(_productor);
private:
	int _ademco_id = -1;
	int _sub_machine_id = -1;
	CDetectorInfo* _detectorInfo = nullptr;
	int _device_info_id = -1;
	int _productor = -1;
	static const DetectorInterfaceType m_dit = DIT_CAMERA_INFO;
};


NAMESPACE_END
