#include "stdafx.h"
#include "alarm_center_video_client.h"

namespace ipc {

class alarm_center_video_client::ipc_client
{
public:
	ipc_client(std::shared_ptr<grpc::Channel> channel)
		: stub_(alarm_center_video::video_service::NewStub(channel))
	{}

	std::unique_ptr<alarm_center_video::video_service::Stub> stub_ = {};

};



alarm_center_video_client::alarm_center_video_client()
{
}

alarm_center_video_client::~alarm_center_video_client()
{
}

void alarm_center_video_client::insert_record(int ademco_id, int zone_value, const std::wstring & txt)
{
}















}
