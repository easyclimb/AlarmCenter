#include "stdafx.h"
#include "VideoUserInfo.h"
#include "VideoDeviceInfoEzviz.h"
#include "VideoDeviceInfoNormal.h"
#include <iterator>

namespace video {

//CProductorInfo CVideoUserInfo::_productorInfo(ProductorUnknown);

CVideoUserInfo::CVideoUserInfo()
	: _id(0)
	, _user_name()
	, _productorInfo(ProductorUnknown)
	, _deviceList()
{}


CVideoUserInfo::~CVideoUserInfo()
{	
	_deviceList.clear();
}


void CVideoUserInfo::GetDeviceList(CVideoDeviceInfoList& list)
{
	std::copy(_deviceList.begin(), _deviceList.end(), std::back_inserter(list));
}
};
