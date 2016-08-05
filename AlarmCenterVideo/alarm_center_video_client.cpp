#include "stdafx.h"
#include "alarm_center_video_client.h"
#include "VideoManager.h"

namespace ipc {

class alarm_center_video_client::ipc_client
{
public:
	ipc_client(std::shared_ptr<grpc::Channel> channel)
		: stub_(alarm_center_video::video_service::NewStub(channel))
	{}

	std::unique_ptr<alarm_center_video::video_service::Stub> stub_ = {};
	
	std::vector<history> histories_ = {};

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

	bool insert_histories() {
		AUTO_LOG_FUNCTION;

		alarm_center_video::hisroty_record record;
		alarm_center_video::reply reply;
		grpc::ClientContext context;

		auto writer = stub_->insert_history_record(&context, &reply);
		auto iter = histories_.begin();
		while (iter != histories_.end()) {
			range_log log("while loop");
			record.set_ademco_id(iter->first.first);
			record.set_zone_value(iter->first.second);
			record.set_record(utf8::w2a(iter->second).c_str());
			if (writer->Write(record)) {
				iter = histories_.erase(iter);
			} else {
				break;
			}
		}
		writer->WritesDone();
		auto status = writer->Finish();
		return status.ok();
	}

	void refresh_db() {
		AUTO_LOG_FUNCTION;
		alarm_center_video::request request;
		alarm_center_video::reply reply;
		grpc::ClientContext context;

		stub_->update_db(&context, request, &reply);


	}

	bool is_db_updated() {
		AUTO_LOG_FUNCTION;
		alarm_center_video::request request;
		alarm_center_video::reply reply;
		grpc::ClientContext context;

		auto status = stub_->is_db_updated(&context, request, &reply);
		if (status.ok()) {
			JLOGA("is_db_updated: ", reply.place_holder().c_str());
			return reply.place_holder() == "true";
		}

		return false;
	}

};



alarm_center_video_client::alarm_center_video_client()
{
	AUTO_LOG_FUNCTION;
	client_ = std::make_unique<ipc_client>(grpc::CreateChannel("127.0.0.1:50052", grpc::InsecureChannelCredentials()));
	client_->get_is_show_video_user_mgr_dlg();

	running_ = true;
	thread_ = std::thread(&alarm_center_video_client::worker, this);
	thread_heart_beet_ = std::thread(&alarm_center_video_client::heart_beet_worker, this);
}

alarm_center_video_client::~alarm_center_video_client()
{
	running_ = false;
	thread_.join();
	thread_heart_beet_.join();

	client_ = nullptr;
}

void alarm_center_video_client::worker()
{
	AUTO_LOG_FUNCTION;
	auto last_time_insert_histories = std::chrono::steady_clock::now();
	auto last_time_get_alarm_devs = std::chrono::steady_clock::now();
	auto last_time_check_is_db_updated = std::chrono::steady_clock::now();

	while (running_) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		if (!running_)break;

		// insert buffered histories per 3s
		{
			
			auto now = std::chrono::steady_clock::now();
			auto diff = now - last_time_insert_histories;
			if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() >= 3) {

				range_log log("insert buffered histories per 3s");

				std::vector<history> histories;
				{
					std::lock_guard<std::mutex> lg(mutex_);

					JLOGA("histories_.size = %d", histories_.size());
					if (!histories_.empty()) {
						std::copy(histories_.begin(), histories_.end(), std::back_inserter(histories));
						histories_.clear();
					}
				}

				JLOGA("histories.size = %d", histories.size());
				if (!histories.empty()) {
					client_->histories_ = histories;
					client_->insert_histories();
				}

				last_time_insert_histories = std::chrono::steady_clock::now();
			}
		}

		// check need update db per 1s
		{
			if (db_refreshed_) {
				client_->refresh_db();
				db_refreshed_ = false;
			}
		}

		// check if alarm center updated db per 3s
		{
			auto now = std::chrono::steady_clock::now();
			auto diff = now - last_time_check_is_db_updated;
			if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() >= 3) {

				range_log log("insert buffered histories per 3s");

				if (client_->is_db_updated()) {
					video::video_manager::get_instance()->LoadFromDB(false);
					notify_observers(0);
				}

				last_time_check_is_db_updated = std::chrono::steady_clock::now();
			}
			
		}
	}
}

void alarm_center_video_client::heart_beet_worker()
{
	AUTO_LOG_FUNCTION;
	auto last_time_get_is_show_video_user_mgr_dlg = std::chrono::steady_clock::now();
	while (running_) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		if (!running_)break;

		// get is show video_user_mgr_dlg per 2s
		{
			auto now = std::chrono::steady_clock::now();
			auto diff = now - last_time_get_is_show_video_user_mgr_dlg;
			if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() >= 3) {
				{
					std::lock_guard<std::mutex> lg(mutex_for_heart_beet_);
					client_->get_is_show_video_user_mgr_dlg();
				}

				last_time_get_is_show_video_user_mgr_dlg = std::chrono::steady_clock::now();
			}
		}
	}
}

void alarm_center_video_client::insert_record(int ademco_id, int zone_value, const std::wstring & txt)
{
	AUTO_LOG_FUNCTION;
	std::lock_guard<std::mutex> lg(mutex_);
	histories_.push_back(alarm_center_video_client::history(alarm_center_video_client::machine_uuid(ademco_id, zone_value), txt));
}















}
