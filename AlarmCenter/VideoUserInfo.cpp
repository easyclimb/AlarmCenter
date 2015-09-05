#include "stdafx.h"
#include "VideoUserInfo.h"
#include "VideoDeviceInfoEzviz.h"
#include "VideoDeviceInfoNormal.h"
#include <iterator>

namespace core {
namespace video {

CProductorInfo CVideoUserInfo::_productorInfo(ProductorEzviz);

CVideoUserInfo::CVideoUserInfo()
	: _id(0)
	, _user_name()
	, _deviceList()
{}


CVideoUserInfo::~CVideoUserInfo()
{
	CVideoDeviceInfoListIter iter = _deviceList.begin();
	if (_productorInfo.get_productor() == EZVIZ) {
		while (iter != _deviceList.end()) {
			ezviz::CVideoDeviceInfoEzviz* device = reinterpret_cast<ezviz::CVideoDeviceInfoEzviz*>(*iter++);
			SAFEDELETEP(device);
		}
	} else if (_productorInfo.get_productor() == NORMAL) {
		while (iter != _deviceList.end()) {
			normal::CVideoDeviceInfoNormal* device = reinterpret_cast<normal::CVideoDeviceInfoNormal*>(*iter++);
			SAFEDELETEP(device);
		}
	}
}


void CVideoUserInfo::GetDeviceList(CVideoDeviceInfoList& list)
{
	std::copy(_deviceList.begin(), _deviceList.end(), std::back_inserter(list));
}

NAMESPACE_END
NAMESPACE_END