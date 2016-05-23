#pragma once
#include "VideoDeviceInfo.h"

namespace video {
namespace normal {

class video_device_info_normal :
	public video_device_info
{
private:
	std::string _domain;
	std::string _ip;
	unsigned short _port;

public:
	
	DECLARE_GETTER_SETTER(std::string, _domain);
	DECLARE_GETTER_SETTER(std::string, _ip);
	DECLARE_GETTER_SETTER(unsigned short, _port);

	video_device_info_normal();
	virtual ~video_device_info_normal();
};




};};

