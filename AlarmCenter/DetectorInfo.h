#pragma once

namespace core
{

static const int DETECTORWIDTH = 180;

class CDetectorInfo
{
private:
	int _id;
	//int _zone_info_id;
	int _map_id;
	int _x;
	int _y;
	int _distance;
	int _angle;
	int _detector_lib_id;
public:
	CDetectorInfo();
	~CDetectorInfo();
	DECLARE_GETTER_SETTER_INT(_id);
	DECLARE_GETTER_SETTER_INT(_map_id);
	DECLARE_GETTER_SETTER_INT(_x);
	DECLARE_GETTER_SETTER_INT(_y);
	DECLARE_GETTER_SETTER_INT(_distance);
	DECLARE_GETTER_SETTER_INT(_angle);
	DECLARE_GETTER_SETTER_INT(_detector_lib_id);

	DECLARE_UNCOPYABLE(CDetectorInfo)
};







NAMESPACE_END
