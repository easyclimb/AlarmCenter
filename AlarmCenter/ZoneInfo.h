#pragma once
#include <list>
#include "core.h"
#include "DetectorBindInterface.h"

namespace core
{

typedef enum zone_status_or_property {
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
}zone_status_or_property;

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


typedef enum zone_type {
	ZT_ZONE,				// 主机防区
	ZT_SUB_MACHINE,			// 分机
	ZT_SUB_MACHINE_ZONE,	// 分机防区
}zone_type;

using namespace ademco;

class zone_info : public detector_bind_interface ,
	public std::enable_shared_from_this<zone_info>
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
	zone_type _type;
	int _detector_id;
	int _sub_machine_id;
	//int _property_id;
	CString _alias;
	
	alarm_machine_ptr _subMachineInfo;
	map_info_weak_ptr _mapInfo;
	bool _alarming;
	EventLevel _highestEventLevel;
	std::list<ADEMCO_EVENT> _eventList;
	
	static const detector_interface_type m_dit = DIT_ZONE_INFO;
public:
	virtual detector_interface_type GetInterfaceType() const override { return m_dit; }
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
	zone_type get_type() const { return _type; }
	void set_type(int type) { _type = Integer2ZoneType(type); }

	DECLARE_GETTER_SETTER_INT(_detector_id); 
	DECLARE_GETTER_SETTER_INT(_sub_machine_id);
	DECLARE_GETTER_SETTER_STRING(_alias);

	explicit zone_info();
	~zone_info();

	virtual std::wstring FormatTooltip() const override;
	virtual void DoClick() override;
	virtual void DoRClick() override;
	
	virtual bool get_alarming() const override { return _alarming; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	void SetSubMachineInfo(const core::alarm_machine_ptr& subMachine) {
		assert(subMachine);
		_subMachineInfo = subMachine;
	}

	alarm_machine_ptr GetSubMachineInfo() const { return _subMachineInfo; }

	void SetMapInfo(const core::map_info_ptr& mapInfo) { _mapInfo = mapInfo; }
	map_info_ptr GetMapInfo() const { return _mapInfo.lock(); }

	void HandleAdemcoEvent(const ademco::AdemcoEventPtr& ademcoEvent, const alarm_text_ptr& at = nullptr);

	// 2015年3月17日 20:57:08 真正操作下属分机的操作，考虑由zoneinfo操作比较合适
	bool execute_set_sub_machine(const core::alarm_machine_ptr& subMachine);
	bool execute_del_sub_machine();
	bool execute_update_alias(const wchar_t* alias);
	bool execute_update_contact(const wchar_t* contact);
	bool execute_update_address(const wchar_t* address);
	bool execute_update_phone(const wchar_t* phone);
	bool execute_update_phone_bk(const wchar_t* phone_bk);
	bool execute_set_detector_info(const detector_info_ptr& detInfo);
	//bool execute_rem_detector_info();
	bool execute_del_detector_info();
	bool execute_bind_detector_info_to_map_info(const core::map_info_ptr& mapInfo);
	//bool execute_unbind_detector_info_from_map_info();
	bool execute_create_detector_info_and_bind_map_info(const detector_info_ptr& detInfo,
														const core::map_info_ptr& mapInfo);
	bool execute_set_physical_addr(int addr);
	bool execute_set_status_or_property(char status);

	CString get_formatted_zone_id() const;

protected:
	static zone_type Integer2ZoneType(int type) {
		switch (type) {
			case ZT_SUB_MACHINE:		return ZT_SUB_MACHINE;		break;
			case ZT_SUB_MACHINE_ZONE:	return ZT_SUB_MACHINE_ZONE; break;
			case ZT_ZONE: default:		return ZT_ZONE;				break;
		}
	}

};

};

