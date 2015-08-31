#include "stdafx.h"
#include "VideoDeviceInfoEzviz.h"

namespace core {
namespace video {
namespace ezviz {

CProductorInfo CVideoDeviceInfoEzviz::_productorInfo(ProductorEzviz);

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
	, _secureCode()

{}


CVideoDeviceInfoEzviz::~CVideoDeviceInfoEzviz()
{}











NAMESPACE_END
NAMESPACE_END
NAMESPACE_END
