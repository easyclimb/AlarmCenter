#pragma once

namespace core
{

// sub_zone: 
// 0 for zone of machine
// between 0 and 0xEE for zone of sub machine
// 0xee for sub machine
static const int INDEX_ZONE			= 0;
static const int INDEX_SUB_MACHINE	= 0xEE;

enum ZoneType {
	ZT_ZONE,				// 主机防区
	ZT_SUB_MACHINE,			// 分机
	ZT_SUB_MACHINE_ZONE,	// 分机防区
};

class CDetectorInfo;
class CZoneInfo
{
private:
	int _id;
	int _zone;
	int _sub_zone;
	int _ademco_id;
	//int _map_id;
	//int _type;
	ZoneType _type;
	int _detector_id;
	int _property_id;
	wchar_t* _alias;
	CDetectorInfo* _detectorInfo;
public:
	DEALARE_GETTER_SETTER_INT(_id);
	DEALARE_GETTER_SETTER_INT(_zone);
	int get_sub_zone() const { return _sub_zone; }
	void set_sub_zone(int sub_zone) { 
		_sub_zone = sub_zone; 
		if (sub_zone == INDEX_ZONE) _type = ZT_ZONE;
		else if (sub_zone == INDEX_SUB_MACHINE) _type = ZT_SUB_MACHINE;
		else _type = ZT_SUB_MACHINE_ZONE;
	}
	
	DEALARE_GETTER_SETTER_INT(_ademco_id);
	//DEALARE_GETTER_SETTER_INT(_map_id);
	//DEALARE_GETTER_SETTER_INT(_type);
	ZoneType get_type() const { return _type; }

	DEALARE_GETTER_SETTER_INT(_detector_id);
	DEALARE_GETTER_SETTER_INT(_property_id);

	DECLARE_GETTER_SETTER_STRING(_alias);

	CZoneInfo();
	~CZoneInfo();

	void SetDetectorInfo(CDetectorInfo* detectorInfo) {
		assert(detectorInfo);
		_detectorInfo = detectorInfo;
	}

	CDetectorInfo* GetDetectorInfo() const {
		return _detectorInfo;
	}

	DECLARE_UNCOPYABLE(CZoneInfo)
};

//
//class CSubZoneInfo
//{
//private:
//	int _id;
//	int _sub_zone;
//	int _zone_id;
//	int _sub_map_id;
//	//int _type;
//	int _detector_id;
//	int _property_id;
//	wchar_t* _alias;
//	CDetectorInfo* _detectorInfo;
//public:
//	DEALARE_GETTER_SETTER_INT(_id);
//	DEALARE_GETTER_SETTER_INT(_sub_zone);
//	DEALARE_GETTER_SETTER_INT(_zone_id);
//	DEALARE_GETTER_SETTER_INT(_sub_map_id);
//	//DEALARE_GETTER_SETTER_INT(_type);
//	DEALARE_GETTER_SETTER_INT(_detector_id);
//	DEALARE_GETTER_SETTER_INT(_property_id);
//
//	DECLARE_GETTER_SETTER_STRING(_alias);
//
//	CSubZoneInfo();
//	~CSubZoneInfo();
//
//	void SetDetectorInfo(CDetectorInfo* detectorInfo)
//	{
//		assert(detectorInfo);
//		_detectorInfo = detectorInfo;
//	}
//
//	CDetectorInfo* GetDetectorInfo() const
//	{
//		return _detectorInfo;
//	}
//
//	DECLARE_UNCOPYABLE(CSubZoneInfo)
//};
//


NAMESPACE_END
