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
	typedef std::pair<int, int> machine_uuid;
	typedef std::pair<machine_uuid, std::wstring> history;
	class ipc_client;
	std::unique_ptr<ipc_client> client_ = {};
	std::vector<history> histories_ = {};

	bool running_ = false;
	std::thread thread_ = {};
	std::thread thread_heart_beet_ = {};
	void worker();
	void heart_beet_worker();

	std::mutex mutex_ = {};
	std::mutex mutex_for_heart_beet_ = {};

public:

	void insert_record(int ademco_id, int zone_value, const std::wstring& txt);
};











}

