#include "stdafx.h"
#include "alarm_center_video_service.h"

namespace ipc {

alarm_center_video_service::alarm_center_video_service()
{
}


alarm_center_video_service::~alarm_center_video_service()
{
}

void alarm_center_video_service::play_video(const video::zone_uuid_ptr & uuid, const core::alarm_text_ptr & text)
{

}

void alarm_center_video_service::play_video(const video::video_device_info_ptr & device)
{
}

video::ezviz::video_user_info_ezviz_ptr alarm_center_video_service::get_ezviz_user(int id)
{
	return video::ezviz::video_user_info_ezviz_ptr();
}

video::jovision::video_user_info_jovision_ptr alarm_center_video_service::get_jovision_user(int id)
{
	return video::jovision::video_user_info_jovision_ptr();
}

void alarm_center_video_service::get_user_list(video::user_list & list)
{
}

void alarm_center_video_service::get_dev_list(video::video_device_info_list & list)
{
}

void alarm_center_video_service::get_dev_list_of_ezviz_user(const video::ezviz::video_user_info_ezviz_ptr & user, video::ezviz::video_device_info_ezviz_list & list)
{

}

video::video_device_info_ptr alarm_center_video_service::get_device(const video::video_device_identifier_ptr& data)
{
	return video::video_device_info_ptr();
}

video::video_device_info_ptr alarm_center_video_service::get_device(const video::video_device_identifier & data)
{
	return video::video_device_info_ptr();
}

video::video_device_info_ptr alarm_center_video_service::get_device(const video::video_device_identifier * data)
{
	return video::video_device_info_ptr();
}

video::bind_info alarm_center_video_service::get_bind_info(const video::zone_uuid & uuid)
{
	return video::bind_info();
}

bool alarm_center_video_service::unbind(const video::zone_uuid & uuid)
{
	return false;
}

bool alarm_center_video_service::bind(const video::video_device_info_ptr & device, const video::zone_uuid & uuid)
{
	return false;
}

bool alarm_center_video_service::set_binded_zone_auto_popup(const video::zone_uuid & uuid, bool b)
{
	return false;
}





}
