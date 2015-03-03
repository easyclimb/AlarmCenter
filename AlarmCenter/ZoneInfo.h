#pragma once

namespace core
{

class CDetectorInfo;
class CZoneInfo
{
	DECLARE_UNCOPYABLE(CZoneInfo)
private:
	int _id;
	int _zone_id;
	int _ademco_id;
	int _map_id;
	//int _type;
	int _detector_id;
	int _property_id;
	wchar_t* _alias;
	CDetectorInfo* _detectorInfo;
public:
	DEALARE_GETTER_SETTER_INT(_id);
	DEALARE_GETTER_SETTER_INT(_zone_id);
	DEALARE_GETTER_SETTER_INT(_ademco_id);
	DEALARE_GETTER_SETTER_INT(_map_id);
	//DEALARE_GETTER_SETTER_INT(_type);
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
};


class CSubZoneInfo
{
private:
	int _id;
	int _sub_zone;
	int _zone_id;
	int _sub_map_id;
	//int _type;
	int _detector_id;
	int _property_id;
	wchar_t* _alias;
	CDetectorInfo* _detectorInfo;
public:
	DEALARE_GETTER_SETTER_INT(_id);
	DEALARE_GETTER_SETTER_INT(_sub_zone);
	DEALARE_GETTER_SETTER_INT(_zone_id);
	DEALARE_GETTER_SETTER_INT(_sub_map_id);
	//DEALARE_GETTER_SETTER_INT(_type);
	DEALARE_GETTER_SETTER_INT(_detector_id);
	DEALARE_GETTER_SETTER_INT(_property_id);

	DECLARE_GETTER_SETTER_STRING(_alias);

	CSubZoneInfo();
	~CSubZoneInfo();

	void SetDetectorInfo(CDetectorInfo* detectorInfo)
	{
		assert(detectorInfo);
		_detectorInfo = detectorInfo;
	}

	CDetectorInfo* GetDetectorInfo() const
	{
		return _detectorInfo;
	}

	DECLARE_UNCOPYABLE(CSubZoneInfo)
};



NAMESPACE_END
