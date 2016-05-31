#include "stdafx.h"
#include "VideoUserInfoEzviz.h"
#include "VideoManager.h"
#include "VideoDeviceInfoEzviz.h"
#include "AlarmMachineManager.h"

namespace video {
namespace ezviz {

video_user_info_ezviz::video_user_info_ezviz()
{}


video_user_info_ezviz::~video_user_info_ezviz()
{

}


bool video_user_info_ezviz::execute_set_acc_token(const std::string& accToken)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	CString sql; 
	sql.Format(L"update table_user_info_ezviz set access_token='%s' where ID=%d",
			   A2W(accToken.c_str()), real_user_id_);
	if (video_manager::GetInstance()->Execute(sql)) {
		set_acc_token(accToken);
		return execute_set_token_time(std::chrono::system_clock::now());;
	}
	return false;
}


bool video_user_info_ezviz::execute_add_device(video_device_info_ezviz_ptr device)
{
	AUTO_LOG_FUNCTION;
	CString sql;
	sql.Format(L"insert into table_device_info_ezviz \
([cameraId],[cameraName],[cameraNo],[defence],[deviceId],[deviceName],[deviceSerial],\
[isEncrypt],[isShared],[picUrl],[status],[secure_code],[device_note],[user_info_id]) \
values('%s','%s',%d,%d,'%s','%s','%s',%d,'%s','%s',%d,'%s','%s',%d)",
			   utf8::a2w(device->get_cameraId()).c_str(),
			   device->get_cameraName().c_str(),
			   device->get_cameraNo(),
			   device->get_defence(),
			   utf8::a2w(device->get_deviceId()).c_str(),
			   device->get_deviceName().c_str(),
			   utf8::a2w(device->get_deviceSerial()).c_str(),
			   device->get_isEncrypt(),
			   utf8::a2w(device->get_isShared()).c_str(),
			   utf8::a2w(device->get_picUrl()).c_str(),
			   device->get_status(),
			   utf8::a2w(device->get_secure_code()).c_str(),
			   device->get_device_note().c_str(),
			   real_user_id_);

	int id = video_manager::GetInstance()->AddAutoIndexTableReturnID(sql);
	if (id != -1) {
		device->set_id(id);
		device->set_userInfo(shared_from_this());
		AddDevice(device);
		return true;
	}
	return false;
}


bool video_user_info_ezviz::DeleteVideoDevice(video_device_info_ezviz_ptr device)
{
	assert(device);
	bool ok = true;
	std::list<zone_uuid> zoneList;
	device->get_zoneUuidList(zoneList);
	for(auto zone : zoneList) {
		ok = video_manager::GetInstance()->UnbindZoneAndDevice(zone);
		if (!ok) {
			return ok;
		}
	}
	if (ok) {
		CString sql;
		sql.Format(L"delete from table_device_info_ezviz where ID=%d", device->get_id());
		ok = video_manager::GetInstance()->Execute(sql) ? true : false;
	}
	if (ok) {
		core::alarm_machine_manager::GetInstance()->DeleteCameraInfo(device->get_id(), device->get_userInfo()->get_productorInfo().get_productor());
		_deviceList.remove(device);
	}
	return ok;
}


bool video_user_info_ezviz::execute_set_token_time(const std::chrono::system_clock::time_point& tp)
{
	AUTO_LOG_FUNCTION;
	CString sql;
	sql.Format(L"update table_user_info_ezviz set token_time='%s' where ID=%d", time_point_to_wstring(tp).c_str(), real_user_id_);
	if (video_manager::GetInstance()->Execute(sql)) {
		set_token_time(tp);
		return true;
	}
	return false;
}

};};
