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

	void get_is_show_video_user_mgr_dlg() {
		AUTO_LOG_FUNCTION;

		alarm_center_video::reply reply;
		alarm_center_video::request request;
		grpc::ClientContext context;

		auto status = stub_->get_is_show_video_user_mgr_dlg(&context, request, &reply);
		if (status.ok()) {
			bool show = reply.place_holder() == "show";
			if (show) {
				auto app = AfxGetApp();
				if (app) {
					auto wnd = app->GetMainWnd();
					if (wnd) {
						wnd->PostMessageW(WM_SHOW_USER_MGR_DLG);
					}
				}
			}
		}
	}

};



alarm_center_video_client::alarm_center_video_client()
{
	AUTO_LOG_FUNCTION;
	client_ = std::make_unique<ipc_client>(grpc::CreateChannel("127.0.0.1:50052", grpc::InsecureChannelCredentials()));
	client_->get_is_show_video_user_mgr_dlg();

	running_ = true;
	thread_ = std::thread(&alarm_center_video_client::worker, this);
}

alarm_center_video_client::~alarm_center_video_client()
{
	running_ = false;
	thread_.join();

	client_ = nullptr;
}

void alarm_center_video_client::worker()
{
	AUTO_LOG_FUNCTION;
	auto last_time_get_is_show_video_user_mgr_dlg = std::chrono::steady_clock::now();
	auto last_time_get_alarm_devs = std::chrono::steady_clock::now();

	while (running_) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		// get is show video_user_mgr_dlg per 3s
		{
			auto now = std::chrono::steady_clock::now();
			auto diff = now - last_time_get_is_show_video_user_mgr_dlg;
			if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() >= 3) {
				{
					std::lock_guard<std::mutex> lg(mutex_);
					client_->get_is_show_video_user_mgr_dlg();
				}

				last_time_get_is_show_video_user_mgr_dlg = std::chrono::steady_clock::now();
			}
		}
	}
}

void alarm_center_video_client::insert_record(int ademco_id, int zone_value, const std::wstring & txt)
{
}















}
