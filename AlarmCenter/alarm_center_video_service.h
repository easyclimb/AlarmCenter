#pragma once
#include <grpc++/grpc++.h>
#include "../rpc/alarm_center_video.grpc.pb.h"
#include "video.h"
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
	void get_dev_list_of_ezviz_user(const video::ezviz::video_user_info_ezviz_ptr& user, video::ezviz::video_device_info_ezviz_list& list);
};

}


