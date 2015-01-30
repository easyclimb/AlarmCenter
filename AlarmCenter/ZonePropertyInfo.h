#pragma once

#include <list>

namespace core {

enum ZoneProperty {
	ZP_BURGLAR = 0,	// µÁ¾¯
	ZP_FIRE,		// »ð¾¯
	ZP_DURESS,		// Ð²ÆÈ
	ZP_GAS,			// ÃºÆø
	ZP_WATER,		// Ë®¾¯
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

	DEALARE_GETTER_SETTER_INT(_id);
	DECLARE_GETTER_SETTER_STRING(_property_text);
	DECLARE_GETTER_SETTER_STRING(_alarm_text);

protected:
	static ZoneProperty IntegerToZoneProperty(int property);
};


class CZonePropertyInfo {
private:
	std::list<CZonePropertyData*> _zonePropertyDataList;
public:
	~CZonePropertyInfo();

	void AddZonePropertyData(CZonePropertyData* data) { _zonePropertyDataList.push_back(data); }
	CZonePropertyData* GetZonePropertyData(int id);

	DECLARE_UNCOPYABLE(CZonePropertyInfo)
	DECLARE_SINGLETON(CZonePropertyInfo)
};




NAMESPACE_END

