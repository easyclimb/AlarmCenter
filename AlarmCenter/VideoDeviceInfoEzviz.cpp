#include "stdafx.h"
#include "VideoDeviceInfoEzviz.h"
#include "VideoInfo.h"
#include "VideoUserInfo.h"

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


bool CVideoDeviceInfoEzviz::execute_update_info()
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	CString sql;
	sql.Format(L"update device_info_ezviz set \
cameraId='%s',cameraName='%s',cameraNo=%d,defence=%d,deviceId='%s', \
deviceName='%s',deviceSerial='%s',isEncrypt=%d,isShared='%s',picUrl='%s',\
status=%d,secure_code='%s',device_note='%s',user_info_id=%d where ID=%d",
				A2W(get_cameraId().c_str()),
				A2W(get_cameraName().c_str()),
				get_cameraNo(),
				get_defence(),
				A2W(get_deviceId().c_str()),
				A2W(get_deviceName().c_str()),
				A2W(get_deviceSerial().c_str()),
				get_isEncrypt(),
				A2W(get_isShared().c_str()),
				A2W(get_picUrl().c_str()),
				get_status(),
				A2W(get_secure_code().c_str()),
				get_device_note().c_str(),
				get_userInfo()->get_id(),
				_id);
	return CVideoManager::GetInstance()->Execute(sql) ? true : false;
}








NAMESPACE_END
NAMESPACE_END
