#pragma once

namespace core
{

static const int DETECTORWIDTH = 180;

class CDetectorInfo
{
	DECLARE_UNCOPYABLE(CDetectorInfo)
private:
	int _id;
	//int _zone_info_id;
	int _x;
	int _y;
	int _distance;
	int _angle;
	int _detector_lib_id;
public:
	CDetectorInfo();
	~CDetectorInfo();
	DEALARE_GETTER_SETTER_INT(_id);
	//DEALARE_GETTER_SETTER_INT(_zone_info_id);
	DEALARE_GETTER_SETTER_INT(_x);
	DEALARE_GETTER_SETTER_INT(_y);
	DEALARE_GETTER_SETTER_INT(_distance);
	DEALARE_GETTER_SETTER_INT(_angle);
	DEALARE_GETTER_SETTER_INT(_detector_lib_id);
};







NAMESPACE_END
