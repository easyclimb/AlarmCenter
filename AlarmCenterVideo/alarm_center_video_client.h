#pragma once
#include <grpc++/grpc++.h>
#include "../rpc/alarm_center_video.pb.h"
#include "../rpc/alarm_center_video.grpc.pb.h"


namespace ipc {


class alarm_center_video_client : public dp::singleton<alarm_center_video_client>
{
protected:
	alarm_center_video_client();

public:
	~alarm_center_video_client();

private:
	class ipc_client;
	std::unique_ptr<ipc_client> client_ = {};

	bool running_ = false;
	std::thread thread_ = {};
	void worker();

	std::mutex mutex_ = {};

public:

	void get_video_user_list(video::video_user_info_list& list);

};











}

