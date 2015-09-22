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
	if (_productorInfo.get_productor() == EZVIZ) {
		for (auto& iter : _deviceList) {
			ezviz::CVideoDeviceInfoEzviz* device = reinterpret_cast<ezviz::CVideoDeviceInfoEzviz*>(iter);
			SAFEDELETEP(device);
		}
	} else if (_productorInfo.get_productor() == NORMAL) {
		for (auto& iter : _deviceList) {
			normal::CVideoDeviceInfoNormal* device = reinterpret_cast<normal::CVideoDeviceInfoNormal*>(iter);
			SAFEDELETEP(device);
		}
	}
}


void CVideoUserInfo::GetDeviceList(CVideoDeviceInfoList& list)
{
	std::copy(_deviceList.begin(), _deviceList.end(), std::back_inserter(list));
}

NAMESPACE_END
