#include "stdafx.h"
#include "VideoDeviceInfo.h"
#include "VideoUserInfo.h"
#include "VideoUserInfoEzviz.h"
#include "VideoUserInfoJovision.h"

namespace video {

video_device_identifier * video_device_info::create_identifier() const
{
	video_device_identifier* data = new video_device_identifier();
	data->dev_id = _id;
	auto user = _userInfo.lock();
	if (user) {
		data->productor = user->get_productorInfo().get_productor();
	} else {
		assert(0);
		data->productor = video::UNKNOWN;
	}
	
	return data;
}

video_device_info::video_device_info()
	: _id(0)
	, _device_note()
	, _userInfo()
	, _binded(false)
{}


video_device_info::~video_device_info()
{}
};
