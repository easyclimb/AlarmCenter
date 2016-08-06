#pragma once
#include "../video/video.h"
#include "core.h"

class sub_process_mgr;

namespace grpc {
class Server;
}

namespace ipc {

class alarm_center_video_service : public dp::singleton<alarm_center_video_service>, public dp::observable<int>
{
protected:
	alarm_center_video_service();
	class alarm_center_video_service_impl;
	bool running_ = false;
	bool show_video_user_mgr_dlg_ = false;
	bool shutdown_ok_ = false;
	std::mutex mutex_ = {};
	std::condition_variable cv_ = {};
	std::thread thread1_ = {};
	std::thread thread2_ = {};
	std::shared_ptr<::grpc::Server> server_ = {};
	std::shared_ptr<sub_process_mgr> sub_process_mgr_ = {};
	void daemon_video_process();

	bool db_updated_ = false;

	typedef std::pair<video::video_device_identifier_ptr, std::vector<video::zone_uuid_ptr>> device_text_pair;
	std::vector<device_text_pair> devices_wait_to_paly_ = {};
	std::mutex lock_for_devices_wait_to_paly_ = {};

public:
	
	// must call shutdown before destruct service
	virtual ~alarm_center_video_service();

	void shutdown();

	void show_video_user_mgr_dlg();

	void play_video(const video::zone_uuid_ptr& uuid, const core::alarm_text_ptr& at);
	void play_video(const video::device_ptr& device);

	void update_db() { db_updated_ = true; }

	//video::ezviz::ezviz_user_ptr get_ezviz_user(int id);
	//video::jovision::jovision_user_ptr get_jovision_user(int id);
	//void get_user_list(video::user_list& list);

	//void get_dev_list(video::device_list& list);
	//void get_dev_list_of_ezviz_user(const video::ezviz::ezviz_user_ptr& user, video::ezviz::ezviz_device_list& list);
	//video::device_ptr get_device(const video::video_device_identifier_ptr& data);
	//video::device_ptr get_device(const video::video_device_identifier& data);
	//video::device_ptr get_device(const video::video_device_identifier* data);

	//video::bind_info get_bind_info(const video::zone_uuid& uuid);
	//bool unbind(const video::zone_uuid& uuid);
	//bool bind(const video::device_ptr& device, const video::zone_uuid& uuid);
	//bool set_binded_zone_auto_popup(const video::zone_uuid& uuid, bool b = true);
};

}


