#include "stdafx.h"
#include "VideoUserInfo.h"
#include "VideoDeviceInfoEzviz.h"
#include "VideoDeviceInfoNormal.h"
#include <iterator>

namespace video {

//productor_info video_user_info::_productorInfo(ProductorUnknown);

video_user_info::video_user_info()
	: _id(0)
	, _user_name()
	, _productorInfo(ProductorUnknown)
	, _deviceList()
{}


video_user_info::~video_user_info()
{	
	_deviceList.clear();
}


void video_user_info::GetDeviceList(video_device_info_list& list)
{
	std::copy(_deviceList.begin(), _deviceList.end(), std::back_inserter(list));
}
};
