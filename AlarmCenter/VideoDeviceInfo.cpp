#include "stdafx.h"
#include "VideoDeviceInfo.h"

namespace video {
CVideoDeviceInfo::CVideoDeviceInfo()
	: _id(0)
	, _device_note()
	, _userInfo(NULL)
	, _binded(false)
{}


CVideoDeviceInfo::~CVideoDeviceInfo()
{}

NAMESPACE_END
