#pragma once

#include "video.h"
#include <list>
#include "video.h"

namespace video {
class video_device_info;
class video_user_info 
{
protected:
	int _id;
	std::wstring _user_name;
	productor_info _productorInfo;
	video_device_info_list _deviceList;
public:

	DECLARE_GETTER_SETTER_INT(_id);
	DECLARE_GETTER_SETTER(std::wstring, _user_name);
	DECLARE_GETTER_SETTER(productor_info, _productorInfo);
	//DECLARE_GETTER(std::list<video_device_info_ptr>, _deviceList);
	void GetDeviceList(video_device_info_list& list);
	int get_device_count() const { return _deviceList.size(); }
	void AddDevice(video_device_info_ptr device) { _deviceList.push_back(device); }
	virtual bool DeleteVideoDevice(video_device_info_ptr) { return false; }
	

	video_user_info();
	virtual ~video_user_info();
};
};
