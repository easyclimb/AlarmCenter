#pragma once
#include <grpc++/grpc++.h>
#include "../rpc/alarm_center_video.grpc.pb.h"
#include "../video/video.h"
#include "core.h"

namespace ipc {

class alarm_center_video_service : public dp::singleton<alarm_center_video_service>
{
protected:
	alarm_center_video_service();

public:
	
	virtual ~alarm_center_video_service();

	void play_video(const video::zone_uuid_ptr& uuid, const core::alarm_text_ptr& at);
	void play_video(const video::video_device_info_ptr& device);

	video::ezviz::video_user_info_ezviz_ptr get_ezviz_user(int id);
	video::jovision::video_user_info_jovision_ptr get_jovision_user(int id);
	void get_user_list(video::video_user_info_list& list);

	void get_dev_list(video::video_device_info_list& list);
	void get_dev_list_of_ezviz_user(const video::ezviz::video_user_info_ezviz_ptr& user, video::ezviz::video_device_info_ezviz_list& list);
	video::video_device_info_ptr get_device(const video::video_device_identifier_ptr& data);
	video::video_device_info_ptr get_device(const video::video_device_identifier& data);
	video::video_device_info_ptr get_device(const video::video_device_identifier* data);

	video::bind_info get_bind_info(const video::zone_uuid& uuid);
	bool unbind(const video::zone_uuid& uuid);
	bool bind(const video::video_device_info_ptr& device, const video::zone_uuid& uuid);
	bool set_binded_zone_auto_popup(const video::zone_uuid& uuid, bool b = true);
};

}


