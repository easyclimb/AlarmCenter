#include "stdafx.h"
#include "AlarmCenterVideo.h"
#include "alarm_center_video_client.h"
#include "VideoManager.h"
#include "VideoPlayerDlg.h"
#include "../video/VideoDeviceInfo.h"
#include "ConfigHelper.h"

namespace ipc {

class alarm_center_video_client::ipc_client
{
public:
	ipc_client(std::shared_ptr<grpc::Channel> channel)
		: stub_(alarm_center_video::video_service::NewStub(channel))
	{}

	std::unique_ptr<alarm_center_video::video_service::Stub> stub_ = {};
	
	std::vector<history> histories_ = {};

	std::vector<alarm_center_video::camera_info> cameras_waiting_to_delete_ = {};



	void get_is_show_video_user_mgr_dlg() {
		AUTO_LOG_FUNCTION;

		alarm_center_video::is_show_mgr_dlg reply;
		alarm_center_video::request request;
		grpc::ClientContext context;

		auto status = stub_->get_is_show_video_user_mgr_dlg(&context, request, &reply);
		if (status.ok()) {
			bool show = reply.show() == "show";
			//if (show) {
				auto app = AfxGetApp();
				if (app) {
					auto wnd = app->GetMainWnd();
					if (wnd) {
						wnd->PostMessageW(WM_SHOW_USER_MGR_DLG, show, reply.user_level());
					}
				}
			//}
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

	bool get_wait_to_play_devices() {
		alarm_center_video::request request;
		alarm_center_video::alarm_info alarm_info;
		grpc::ClientContext context;

		auto mgr = video::video_manager::get_instance();
		auto reader = stub_->get_alarming_devs(&context, request);
		while (reader->Read(&alarm_info)) {
			video::video_device_identifier id;
			id.dev_id = alarm_info.devinfo().dev_id();
			id.productor_type = video::Integer2ProductorType(alarm_info.devinfo().productor_type());
			video::device_ptr device = mgr->GetVideoDeviceInfo(&id);
			int speed = util::CConfigHelper::get_instance()->get_default_video_level();

			if (device && g_videoPlayerDlg) {
				//if (alarm_info.alarm_msg_size() == 0) {
				//	g_videoPlayerDlg->PlayVideoByDevice(device, speed);
				//} else {
				//	for (auto txt : alarm_info.alarm_msg()) {
				//		//g_videoPlayerDlg->PlayVideoByDevice(device, speed, )
				//	}
				//}

				//video::zone_uuid_ptr zone = nullptr;
				if (alarm_info.has_zone_uuid()) {
					auto zone = std::make_shared<video::zone_uuid>();
					zone->_ademco_id = alarm_info.zone_uuid().ademco_id();
					zone->_zone_value = alarm_info.zone_uuid().zone_value();
					zone->_gg = alarm_info.zone_uuid().gg();

					core::alarm_text_ptr at = nullptr;
					if (alarm_info.has_alarm_txt()) {
						at = std::make_shared<core::alarm_text>();
						at->_zone = alarm_info.alarm_txt().zone_value();
						at->_subzone = alarm_info.alarm_txt().sub_zone();
						at->_event = alarm_info.alarm_txt().event_code();
						at->_txt = utf8::a2w(alarm_info.alarm_txt().alarm_txt()).c_str();
					}

					g_videoPlayerDlg->PlayVideo(zone, at);
				} else {
					g_videoPlayerDlg->PlayVideo(device);
				}

				
			}
		}

		auto status = reader->Finish();
		return status.ok();
	}

	bool delete_camera() {
		AUTO_LOG_FUNCTION;
		alarm_center_video::reply reply;
		grpc::ClientContext context;

		auto writer = stub_->delete_camera_info(&context, &reply);

		for (auto iter = cameras_waiting_to_delete_.begin(); iter != cameras_waiting_to_delete_.end();) {
			if (!writer->Write(*iter)) {
				break;
			} else {
				iter = cameras_waiting_to_delete_.erase(iter);
			}
		}

		writer->WritesDone();
		auto ok = writer->Finish().ok();
		return ok;
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
	auto last_time_check_has_device_waiting_to_play = std::chrono::steady_clock::now();
	auto last_time_check_delete_camera = std::chrono::steady_clock::now();

	while (running_) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		if (!running_)break;

		// insert buffered histories per 1s
		{
			auto now = std::chrono::steady_clock::now();
			auto diff = now - last_time_insert_histories;
			if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() >= 1) {

				range_log log("insert buffered histories per 1s");

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

		// check if alarm center updated db per 1s
		{
			auto now = std::chrono::steady_clock::now();
			auto diff = now - last_time_check_is_db_updated;
			if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() >= 1) {

				range_log log("insert buffered histories per 1s");

				if (client_->is_db_updated()) {
					video::video_manager::get_instance()->LoadFromDB(false);
					notify_observers(0);
				}

				last_time_check_is_db_updated = std::chrono::steady_clock::now();
			}
			
		}

		// check if there are devices waiting to play per 1s
		{
			auto now = std::chrono::steady_clock::now();
			auto diff = now - last_time_check_has_device_waiting_to_play;
			if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() >= 1) {

				range_log log("get_wait_to_play_devices per 1s");

				client_->get_wait_to_play_devices();

				last_time_check_has_device_waiting_to_play = std::chrono::steady_clock::now();
			}
		}


		// check if there are cameras waiting for been deleted per 1s
		{
			auto now = std::chrono::steady_clock::now();
			auto diff = now - last_time_check_delete_camera;
			if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() >= 1) {
				range_log log("delete_camera per 1s");

				std::vector<alarm_center_video::camera_info> cameras_waiting_to_delete;

				{
					std::lock_guard<std::mutex> lg(mutex_for_cameras_waiting_to_delete_);
					if (!cameras_waiting_to_delete_.empty()) {
						cameras_waiting_to_delete = cameras_waiting_to_delete_;
						cameras_waiting_to_delete_.clear();
					}
				}

				if (!cameras_waiting_to_delete.empty()) {
					client_->cameras_waiting_to_delete_ = cameras_waiting_to_delete;
					client_->delete_camera();
				}
				
				last_time_check_delete_camera = std::chrono::steady_clock::now();
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

		// get is show video_user_mgr_dlg per 1s
		{
			auto now = std::chrono::steady_clock::now();
			auto diff = now - last_time_get_is_show_video_user_mgr_dlg;
			if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() >= 1) {
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

void alarm_center_video_client::delete_camera_info(int dev_id, int productor_type)
{
	std::lock_guard<std::mutex> lg(mutex_for_cameras_waiting_to_delete_);
	alarm_center_video::camera_info camera;
	camera.set_dev_id(dev_id);
	camera.set_productor_type(productor_type);
	cameras_waiting_to_delete_.push_back(camera);
}















}
