#include "stdafx.h"
#include "VideoDeviceInfo.h"
#include "VideoUserInfo.h"
#include "ezviz/VideoUserInfoEzviz.h"
#include "jovision/VideoUserInfoJovision.h"

namespace video {

video_device_identifier * device::create_identifier() const


device::device()
	: _id(0)
	, _device_note()
	, _userInfo()
	, _binded(false)
{}


device::~device()
{}

};
