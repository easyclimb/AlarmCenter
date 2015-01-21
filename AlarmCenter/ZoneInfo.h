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
	int _type;
	int _detector_id;
	wchar_t* _alias;
	CDetectorInfo* _detectorInfo;
public:
	DEALARE_GETTER_SETTER_INT(_id);
	DEALARE_GETTER_SETTER_INT(_zone_id);
	DEALARE_GETTER_SETTER_INT(_ademco_id);
	DEALARE_GETTER_SETTER_INT(_map_id);
	DEALARE_GETTER_SETTER_INT(_type);
	DEALARE_GETTER_SETTER_INT(_detector_id);
	CZoneInfo();
	~CZoneInfo();
	DECLARE_STRING_SETTER(_alias);
	DECLARE_STRING_GETTER(_alias);

	void SetDetectorInfo(CDetectorInfo* detectorInfo) {
		assert(detectorInfo);
		_detectorInfo = detectorInfo;
	}

	const CDetectorInfo* GetDetectorInfo() const {
		return _detectorInfo;
	}
};




NAMESPACE_END
