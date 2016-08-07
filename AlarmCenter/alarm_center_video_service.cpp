#include "stdafx.h"
#include "alarm_center_video_service.h"
#include "sub_process_mgr.h"
#include <grpc++/grpc++.h>
#include "../rpc/alarm_center_video.grpc.pb.h"
#include "HistoryRecord.h"
#include "AlarmMachineManager.h"
#include "VideoManager.h"
#include "../video/ezviz/VideoDeviceInfoEzviz.h"
#include "../video/jovision/VideoDeviceInfoJovision.h"


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
		auto service = alarm_center_video_service::get_instance();
		if (service->running_) {
			video::video_manager::get_instance()->LoadFromDB();
			service->notify_observers(0);
		}
		return ::grpc::Status::OK;
	}

	virtual ::grpc::Status is_db_updated(::grpc::ServerContext* context,
										 const ::alarm_center_video::request* request,
										 ::alarm_center_video::reply* responce) override {

		auto service = alarm_center_video_service::get_instance();
		if (service->running_ && service->db_updated_) {
			service->db_updated_ = false;
			responce->set_place_holder("true");
		}
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
		auto service = alarm_center_video_service::get_instance();
		if (service->running_ && service->lock_for_devices_wait_to_paly_.try_lock()) {
			std::lock_guard<std::mutex> lg(service->lock_for_devices_wait_to_paly_, std::adopt_lock);

			bool ok = true;
			alarm_center_video::alarm_info alarm_info;

			auto dev = service->devices_wait_to_paly_.begin();
			while(dev != service->devices_wait_to_paly_.end()) {
				alarm_info.mutable_devinfo()->set_dev_id(dev->first->dev_id);
				alarm_info.mutable_devinfo()->set_productor_type(dev->first->productor_type);

				if (!dev->second.empty()) {
					alarm_info.mutable_zone_uuid()->set_ademco_id(dev->second.back()->_ademco_id);
					alarm_info.mutable_zone_uuid()->set_zone_value(dev->second.back()->_zone_value);
					alarm_info.mutable_zone_uuid()->set_gg(dev->second.back()->_gg);
				}

				if (!writer->Write(alarm_info)) {
					ok = false;
					break;
				}

				dev = service->devices_wait_to_paly_.erase(dev);
				service->sub_process_mgr_->feed_watch_dog();
			}

		}
		service->sub_process_mgr_->feed_watch_dog();

		return ::grpc::Status::OK;
	}

	virtual ::grpc::Status get_updated_bind_infos(::grpc::ServerContext* context,
												  const ::alarm_center_video::request* request, 
												  ::grpc::ServerWriter< ::alarm_center_video::bind_info>* writer) override {
		AUTO_LOG_FUNCTION;
		return ::grpc::Status::OK;
	}

	virtual ::grpc::Status insert_history_record(::grpc::ServerContext* context, 
												 ::grpc::ServerReader<::alarm_center_video::hisroty_record>* reader,
												 ::alarm_center_video::reply* response) override {
		AUTO_LOG_FUNCTION;
		auto service = alarm_center_video_service::get_instance();
		auto hr = core::history_record_manager::get_instance();
		alarm_center_video::hisroty_record record;
		while (service->running_ && reader->Read(&record)) {
			range_log log("while loop");
			service->sub_process_mgr_->feed_watch_dog();
			hr->InsertRecord(record.ademco_id(),
							 record.zone_value(),
							 utf8::a2w(record.record()).c_str(),
							 time(nullptr),
							 core::RECORD_LEVEL_VIDEO);
		}
		service->sub_process_mgr_->feed_watch_dog();
		return ::grpc::Status::OK;
	}

	virtual ::grpc::Status delete_camera_info(::grpc::ServerContext* context, 
											  ::grpc::ServerReader<::alarm_center_video::camera_info>* reader,
											  ::alarm_center_video::reply* response) override {
		AUTO_LOG_FUNCTION;
		auto mgr = core::alarm_machine_manager::get_instance();
		auto service = alarm_center_video_service::get_instance();
		alarm_center_video::camera_info camera;
		while (service->running_ && reader->Read(&camera)) {
			range_log log("while loop");
			service->sub_process_mgr_->feed_watch_dog();

			mgr->DeleteCameraInfo(camera.dev_id(), camera.productor_type());
		}

		service->sub_process_mgr_->feed_watch_dog();

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
		if (sub_process_mgr_->get_elapsed_ms() > 15000) {
			sub_process_mgr_->restart();
		}
	}
}

void alarm_center_video_service::play_video(const video::zone_uuid_ptr & uuid, const core::alarm_text_ptr & /*text*/)
{
	std::lock_guard<std::mutex> lg(lock_for_devices_wait_to_paly_);
	auto mgr = video::video_manager::get_instance();
	auto bi = mgr->GetBindInfo(*uuid);
	if (bi._device) {
		bool found = false;
		for (auto dev : devices_wait_to_paly_) {
			if (dev.first->dev_id == bi._device->get_id() 
				&& dev.first->productor_type == bi._device->get_userInfo()->get_productor().get_productor_type()) {
				dev.second.push_back(uuid);
				found = true;
				break;
			}
		}

		if (!found) {
			video::video_device_identifier_ptr id(bi._device->create_identifier());
			device_text_pair pair;
			pair.first = id;
			pair.second.push_back(uuid);
			devices_wait_to_paly_.push_back(pair);
		}
	}
}

void alarm_center_video_service::play_video(const video::device_ptr & device)
{
	std::lock_guard<std::mutex> lg(lock_for_devices_wait_to_paly_);
	auto mgr = video::video_manager::get_instance();
	bool found = false;

	for (auto dev : devices_wait_to_paly_) {
		if (dev.first->dev_id == device->get_id()
			&& dev.first->productor_type == device->get_userInfo()->get_productor().get_productor_type()) {
			found = true;
			break;
		}
	}

	if (!found) {
		video::video_device_identifier_ptr id(device->create_identifier());
		device_text_pair pair;
		pair.first = id;
		devices_wait_to_paly_.push_back(pair);
	}

}
//
//video::ezviz::ezviz_user_ptr alarm_center_video_service::get_ezviz_user(int id)
//{
//	return video::ezviz::ezviz_user_ptr();
//}
//
//video::jovision::jovision_user_ptr alarm_center_video_service::get_jovision_user(int id)
//{
//	return video::jovision::jovision_user_ptr();
//}
//
//void alarm_center_video_service::get_user_list(video::user_list & list)
//{
//}
//
//void alarm_center_video_service::get_dev_list(video::device_list & list)
//{
//}
//
//void alarm_center_video_service::get_dev_list_of_ezviz_user(const video::ezviz::ezviz_user_ptr & user, video::ezviz::ezviz_device_list & list)
//{
//
//}
//
//video::device_ptr alarm_center_video_service::get_device(const video::video_device_identifier_ptr& data)
//{
//	return video::device_ptr();
//}
//
//video::device_ptr alarm_center_video_service::get_device(const video::video_device_identifier & data)
//{
//	return video::device_ptr();
//}
//
//video::device_ptr alarm_center_video_service::get_device(const video::video_device_identifier * data)
//{
//	return video::device_ptr();
//}
//
//video::bind_info alarm_center_video_service::get_bind_info(const video::zone_uuid & uuid)
//{
//	return video::bind_info();
//}
//
//bool alarm_center_video_service::unbind(const video::zone_uuid & uuid)
//{
//	return false;
//}
//
//bool alarm_center_video_service::bind(const video::device_ptr & device, const video::zone_uuid & uuid)
//{
//	return false;
//}
//
//bool alarm_center_video_service::set_binded_zone_auto_popup(const video::zone_uuid & uuid, bool b)
//{
//	return false;
//}





}
