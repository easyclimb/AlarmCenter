#include "stdafx.h"
#include "VideoDeviceInfoEzviz.h"
#include "VideoManager.h"
#include "VideoUserInfo.h"

namespace video {
namespace ezviz {

video_device_info_ezviz::video_device_info_ezviz()
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
	//, _detector_info_id(-1)
{}


video_device_info_ezviz::~video_device_info_ezviz()
{
}


bool video_device_info_ezviz::execute_update_info()
{
	AUTO_LOG_FUNCTION;
	CString sql;
	sql.Format(L"update table_device_info_ezviz set \
cameraId='%s',cameraName='%s',cameraNo=%d,defence=%d,deviceId='%s', \
deviceName='%s',deviceSerial='%s',isEncrypt=%d,isShared='%s',picUrl='%s',\
status=%d,secure_code='%s',device_note='%s',user_info_id=%d where ID=%d", // detector_info_id=%d 
			   utf8::a2w(get_cameraId()).c_str(), 
			   get_cameraName().c_str(),
			   get_cameraNo(),
			   get_defence(),
			   utf8::a2w(get_deviceId()).c_str(),
			   get_deviceName().c_str(),
			   utf8::a2w(get_deviceSerial()).c_str(),
			   get_isEncrypt(),
			   utf8::a2w(get_isShared()).c_str(),
			   utf8::a2w(get_picUrl()).c_str(),
			   get_status(),
			   utf8::a2w(get_secure_code()).c_str(),
			   get_device_note().c_str(),
			   get_userInfo()->get_id(),
			   _id);

	return video_manager::get_instance()->Execute(sql) ? true : false;
}







};};
