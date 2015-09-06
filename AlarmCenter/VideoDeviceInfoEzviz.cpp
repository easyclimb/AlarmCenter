#include "stdafx.h"
#include "VideoDeviceInfoEzviz.h"
#include "VideoInfo.h"

namespace core {
namespace video {
namespace ezviz {

CVideoDeviceInfoEzviz::CVideoDeviceInfoEzviz()
	: _cameraId()
	, _cameraName()
	, _cameraNo(0)
	, _defence(0)
	, _deviceId()
	, _deviceName()
	, _deviceSerial()
	, _isEncrypt(0)
	, _isShared()
	, _picUrl()
	, _status(0)
	, _secure_code()
{}


CVideoDeviceInfoEzviz::~CVideoDeviceInfoEzviz()
{}


bool execute_update_info()
{
	return false;
}








NAMESPACE_END
NAMESPACE_END
NAMESPACE_END
