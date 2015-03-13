#pragma once

//#define USE_ZONE_PROPERTY
#ifdef USE_ZONE_PROPERTY
#include <list>

namespace core {

enum ZoneProperty {
	ZP_BURGLAR = 0,	// ����
	ZP_FIRE,		// ��
	ZP_DURESS,		// в��
	ZP_GAS,			// ú��
	ZP_WATER,		// ˮ��
	ZP_MAX,			
};

class CZonePropertyData
{
private:
	int _id;
	ZoneProperty _property;
	wchar_t* _property_text;
	wchar_t* _alarm_text;
public:
	CZonePropertyData();
	~CZonePropertyData();

	void set_property(int zone_property) { _property = IntegerToZoneProperty(zone_property); }
	ZoneProperty get_property() const { return _property; }

	DECLARE_GETTER_SETTER_INT(_id);
	DECLARE_GETTER_SETTER_STRING(_property_text);
	DECLARE_GETTER_SETTER_STRING(_alarm_text);

protected:
	static ZoneProperty IntegerToZoneProperty(int property);
};


class CZonePropertyInfo {
	DECLARE_UNCOPYABLE(CZonePropertyInfo)
private:
	std::list<CZonePropertyData*> _zonePropertyDataList;
public:
	~CZonePropertyInfo();

	void AddZonePropertyData(CZonePropertyData* data) { _zonePropertyDataList.push_back(data); }
	CZonePropertyData* GetZonePropertyDataById(int id);
	CZonePropertyData* GetZonePropertyDataByProperty(int property_value);
	DECLARE_SINGLETON(CZonePropertyInfo)
};




NAMESPACE_END
#endif
