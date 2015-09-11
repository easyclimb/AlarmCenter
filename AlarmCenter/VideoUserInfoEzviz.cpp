#include "stdafx.h"
#include "VideoUserInfoEzviz.h"
#include "VideoInfo.h"
#include "VideoDeviceInfoEzviz.h"

namespace video {
namespace ezviz {
CVideoUserInfoEzviz::CVideoUserInfoEzviz()
	: _user_phone()
	, _user_accToken()
{}


CVideoUserInfoEzviz::~CVideoUserInfoEzviz()
{}


bool CVideoUserInfoEzviz::execute_set_user_accToken(const std::string& accToken)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	CString sql; 
	sql.Format(L"update user_info set user_accToken='%s' where ID=%d",
			   A2W(accToken.c_str()), _id);
	if (CVideoManager::GetInstance()->Execute(sql)) {
		set_user_accToken(accToken);
		return true;
	}
	return false;
}


bool CVideoUserInfoEzviz::execute_add_device(CVideoDeviceInfoEzviz* device)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	CString sql;
	sql.Format(L"insert into device_info_ezviz \
([cameraId],[cameraName],[cameraNo],[defence],[deviceId],[deviceName],[deviceSerial],\
[isEncrypt],[isShared],[picUrl],[status],[secure_code],[device_note],[user_info_id]) \
values('%s','%s',%d,%d,'%s','%s','%s',%d,'%s','%s',%d,'%s','%s',%d)",
			   A2W(device->get_cameraId().c_str()),
			   A2W(device->get_cameraName().c_str()),
			   device->get_cameraNo(),
			   device->get_defence(),
			   A2W(device->get_deviceId().c_str()),
			   A2W(device->get_deviceName().c_str()),
			   A2W(device->get_deviceSerial().c_str()),
			   device->get_isEncrypt(),
			   A2W(device->get_isShared().c_str()),
			   A2W(device->get_picUrl().c_str()),
			   device->get_status(),
			   A2W(device->get_secure_code().c_str()),
			   device->get_device_note().c_str(),
			   _id);

	int id = CVideoManager::GetInstance()->AddAutoIndexTableReturnID(sql);
	if (id != -1) {
		device->set_id(id);
		device->set_userInfo(this);
		AddDevice(device);
		return true;
	}
	return false;
}


bool CVideoUserInfoEzviz::execute_set_user_name(const std::wstring& name)
{
	AUTO_LOG_FUNCTION;
	CString sql;
	sql.Format(L"update user_info set user_name='%s' where ID=%d",
			   name.c_str(), _id);
	if (CVideoManager::GetInstance()->Execute(sql)) {
		set_user_name(name);
		return true;
	}
	return false;
}


bool CVideoUserInfoEzviz::DeleteVideoDevice(CVideoDeviceInfo* device)
{
	assert(device);
	bool ok = true;
	if (device->get_binded()) {
		ZoneUuid zoneUuid = device->get_zoneUuid();
		ok = CVideoManager::GetInstance()->UnbindZoneAndDevice(zoneUuid);
	}
	if (ok) {
		CString sql;
		sql.Format(L"delete from device_info_ezviz where ID=%d", device->get_id());
		ok = CVideoManager::GetInstance()->Execute(sql) ? true : false;
	}
	if (ok) {
		_deviceList.remove(device);
		SAFEDELETEP(device);
	}
	return ok;
}


NAMESPACE_END
NAMESPACE_END
