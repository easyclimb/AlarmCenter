#include "stdafx.h"
#include "alarm_center_video_service.h"
#include "sub_process_mgr.h"
#include <grpc++/grpc++.h>
#include "../rpc/alarm_center_video.grpc.pb.h"


namespace ipc {

class alarm_center_video_service::alarm_center_video_service_impl : public alarm_center_video::video_service::Service
{
public:
	virtual ~alarm_center_video_service_impl() {
		AUTO_LOG_FUNCTION;
	}

	virtual ::grpc::Status update_db(::grpc::ServerContext* context, 
									 const ::alarm_center_video::request* request, 
									 ::alarm_center_video::reply* response) override {
		AUTO_LOG_FUNCTION;
		return ::grpc::Status::OK;
	}

	virtual ::grpc::Status get_is_show_video_user_mgr_dlg(::grpc::ServerContext* context, 
														  const ::alarm_center_video::request* request, 
														  ::alarm_center_video::reply* response) override {
		AUTO_LOG_FUNCTION;
		auto service = alarm_center_video_service::get_instance();
		if (service->running_ && service->show_video_user_mgr_dlg_) {
			service->show_video_user_mgr_dlg_ = false;
			response->set_place_holder("show");
		}
		service->sub_process_mgr_->feed_watch_dog();
		return ::grpc::Status::OK;
	}

	virtual ::grpc::Status get_alarming_devs(::grpc::ServerContext* context, 
											 const ::alarm_center_video::request* request, 
											 ::grpc::ServerWriter< ::alarm_center_video::alarm_info>* writer) override {
		AUTO_LOG_FUNCTION;
		return ::grpc::Status::OK;
	}

	virtual ::grpc::Status get_updated_bind_infos(::grpc::ServerContext* context,
												  const ::alarm_center_video::request* request, 
												  ::grpc::ServerWriter< ::alarm_center_video::bind_info>* writer) override {
		AUTO_LOG_FUNCTION;
		return ::grpc::Status::OK;
	}

	virtual ::grpc::Status insert_history_record(::grpc::ServerContext* context, 
												 const ::alarm_center_video::hisroty_record* request, 
												 ::alarm_center_video::reply* response) override {
		AUTO_LOG_FUNCTION;
		return ::grpc::Status::OK;
	}

	virtual ::grpc::Status delete_camera_info(::grpc::ServerContext* context, 
											  const ::alarm_center_video::camera_info* request, 
											  ::alarm_center_video::reply* response) override {
		AUTO_LOG_FUNCTION;
		return ::grpc::Status::OK;
	}


};




alarm_center_video_service::alarm_center_video_service()
{
	running_ = true;

	auto alarm_center_video_exe = get_exe_path_a() + "\\AlarmCenterVideo.exe";
	sub_process_mgr_ = std::make_shared<sub_process_mgr>(alarm_center_video_exe);
	sub_process_mgr_->start();

	thread1_ = std::thread([this]() {
		AUTO_LOG_FUNCTION;
		try {
			alarm_center_video_service::alarm_center_video_service_impl service;
			std::string server_address("0.0.0.0:50052");
			::grpc::ServerBuilder builder;
			builder.AddListeningPort(server_address, ::grpc::InsecureServerCredentials());
			builder.RegisterService(&service);
			std::shared_ptr<grpc::Server> server = builder.BuildAndStart();
			server_ = server;
			server->Wait();

			{
				std::unique_lock<std::mutex> ul(mutex_);
				cv_.wait(ul, [this]() {return shutdown_ok_; });
			}
		} catch (...) {
			return;
		}
	})/*.detach()*/;
	//thread1_.detach();

	thread2_ = std::thread(&alarm_center_video_service::daemon_video_process, this);
}

alarm_center_video_service::~alarm_center_video_service()
{
	
}

void alarm_center_video_service::shutdown()
{
	AUTO_LOG_FUNCTION;
	try {
		running_ = false;
		thread2_.join();

		sub_process_mgr_->stop();
		sub_process_mgr_ = nullptr;

		//server_->completion_queue()->Shutdown();
		{
			std::lock_guard<std::mutex> lg(mutex_);
			server_->Shutdown();
			server_ = nullptr;
			shutdown_ok_ = true;
		}
		cv_.notify_one();

		JLOGA("before thread1.join");
		thread1_.join();
		JLOGA("after thread1.join");



	} catch (...) {

	}
}

void alarm_center_video_service::show_video_user_mgr_dlg()
{
	show_video_user_mgr_dlg_ = true;
}

void alarm_center_video_service::daemon_video_process()
{
	while (running_) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		if (sub_process_mgr_->get_elapsed_ms() > 5000) {
			sub_process_mgr_->restart();
		}
	}
}

void alarm_center_video_service::play_video(const video::zone_uuid_ptr & uuid, const core::alarm_text_ptr & text)
{

}

void alarm_center_video_service::play_video(const video::device_ptr & device)
{

}

video::ezviz::ezviz_user_ptr alarm_center_video_service::get_ezviz_user(int id)
{
	return video::ezviz::ezviz_user_ptr();
}

video::jovision::jovision_user_ptr alarm_center_video_service::get_jovision_user(int id)
{
	return video::jovision::jovision_user_ptr();
}

void alarm_center_video_service::get_user_list(video::user_list & list)
{
}

void alarm_center_video_service::get_dev_list(video::device_list & list)
{
}

void alarm_center_video_service::get_dev_list_of_ezviz_user(const video::ezviz::ezviz_user_ptr & user, video::ezviz::ezviz_device_list & list)
{

}

video::device_ptr alarm_center_video_service::get_device(const video::video_device_identifier_ptr& data)
{
	return video::device_ptr();
}

video::device_ptr alarm_center_video_service::get_device(const video::video_device_identifier & data)
{
	return video::device_ptr();
}

video::device_ptr alarm_center_video_service::get_device(const video::video_device_identifier * data)
{
	return video::device_ptr();
}

video::bind_info alarm_center_video_service::get_bind_info(const video::zone_uuid & uuid)
{
	return video::bind_info();
}

bool alarm_center_video_service::unbind(const video::zone_uuid & uuid)
{
	return false;
}

bool alarm_center_video_service::bind(const video::device_ptr & device, const video::zone_uuid & uuid)
{
	return false;
}

bool alarm_center_video_service::set_binded_zone_auto_popup(const video::zone_uuid & uuid, bool b)
{
	return false;
}





}
