#include "stdafx.h"
#include "alarm_center_map_service.h"
#include <grpc++/grpc++.h>
#include "../rpc/alarm_center_map.grpc.pb.h"
#include "CsrInfo.h"
#include "AlarmMachine.h"
#include "AlarmMachineManager.h"
#include "ZoneInfo.h"
#include "sub_process_mgr.h"

namespace ipc {


class alarm_center_map_service::alarm_center_map_service_impl : public alarm_center_map::map_service::Service
{
	virtual ::grpc::Status get_csr_info(::grpc::ServerContext* context, 
										const ::alarm_center_map::csr_info* request, 
										::alarm_center_map::csr_info* response) override {
		auto csr = core::csr_manager::get_instance();
		response->mutable_pt()->set_x(csr->get_coor().x);
		response->mutable_pt()->set_y(csr->get_coor().y);
		response->mutable_pt()->set_level(csr->get_level());
		response->set_show(alarm_center_map_service::get_instance()->show_csr_map_);

		alarm_center_map_service::get_instance()->sub_process_mgr_->feed_watch_dog();
		return ::grpc::Status::OK;
	}

	virtual ::grpc::Status get_alarming_machines_info(::grpc::ServerContext* context,
													  const ::alarm_center_map::csr_info* request,
													  ::grpc::ServerWriter< ::alarm_center_map::machine_info>* writer) override {
		bool ok = true;
		::alarm_center_map::machine_info info;
		auto mgr = core::alarm_machine_manager::get_instance();
		auto v = alarm_center_map_service::get_instance()->get_buffered_machines();
		for (auto uuid : v) {
			auto machine = mgr->GetMachineByUuid(uuid);
			if (machine) {
				info.set_ademco_id(uuid.first);
				info.set_zone_value(uuid.second);
				info.set_title(utf8::w2a((LPCTSTR)machine->get_formatted_name()));
				info.set_info(utf8::w2a((LPCTSTR)machine->get_formatted_info(L"<p/>")));
				info.set_auto_popup(machine->get_auto_show_map_when_start_alarming());
				info.mutable_pt()->set_x(machine->get_coor().x);
				info.mutable_pt()->set_y(machine->get_coor().y);
				info.mutable_pt()->set_level(machine->get_zoomLevel());

				if (!writer->Write(info)) {
					ok = false;
					break;
				}
			}
		}

		if (ok) {
			alarm_center_map_service::get_instance()->sub_process_mgr_->feed_watch_dog();
		}

		return ::grpc::Status::OK;
	}
	
	virtual ::grpc::Status set_csr_info(::grpc::ServerContext* context, const ::alarm_center_map::csr_info* request, ::alarm_center_map::csr_info* response) override {
		auto csr = core::csr_manager::get_instance();
		csr->execute_set_coor(web::BaiduCoordinate(request->pt().x(), request->pt().y()));
		csr->execute_set_zoom_level(request->pt().level());

		alarm_center_map_service::get_instance()->sub_process_mgr_->feed_watch_dog();
		return ::grpc::Status::OK;
	}
	
	virtual ::grpc::Status set_machine_info(::grpc::ServerContext* context, const ::alarm_center_map::machine_info* request, ::alarm_center_map::machine_info* response) override {
		auto mgr = core::alarm_machine_manager::get_instance();
		auto machine = mgr->GetMachineByUuid(core::MachineUuid(request->ademco_id(), request->zone_value()));
		if (machine) {
			machine->execute_set_coor(web::BaiduCoordinate(request->pt().x(), request->pt().y()));
			machine->execute_set_zoomLevel(request->pt().level());
			machine->execute_set_auto_show_map_when_start_alarming(request->auto_popup());
		}

		alarm_center_map_service::get_instance()->sub_process_mgr_->feed_watch_dog();
		return ::grpc::Status::OK;
	}
};

void alarm_center_map_service::daemon_baidu_map() {
	while (running_) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		if (sub_process_mgr_->get_elapsed_ms() > 5000) {
			sub_process_mgr_->restart();
		}
	}
}

alarm_center_map_service::alarm_center_map_service()
{
	running_ = true;
	
	auto baidu_map_exe = get_exe_path_a() + "\\AlarmCenterMap.exe";
	sub_process_mgr_ = std::make_shared<sub_process_mgr>(baidu_map_exe);
	sub_process_mgr_->start();

	

	thread1_ = std::thread([this]() {
		alarm_center_map_service::alarm_center_map_service_impl service;
		std::string server_address("0.0.0.0:50051");
		::grpc::ServerBuilder builder;
		builder.AddListeningPort(server_address, ::grpc::InsecureServerCredentials());
		builder.RegisterService(&service);
		server_ = builder.BuildAndStart();
		auto cp = server_;
		cp->Wait(); 
	})/*.detach()*/;
	//thread1_.detach();

	thread2_ = std::thread(&alarm_center_map_service::daemon_baidu_map, this);
}

alarm_center_map_service::~alarm_center_map_service()
{
	running_ = false;
	thread2_.join();
	sub_process_mgr_ = nullptr;
	
	server_->Shutdown();
	thread1_.join();
	server_ = nullptr;
}

void alarm_center_map_service::show_map(int ademco_id, int zone_value)
{
	std::lock_guard<std::mutex> lock(lock_4_machine_uuids_);
	show_csr_map_ = false;
	machine_uuids_.push_back(core::MachineUuid(ademco_id, zone_value));
}

void alarm_center_map_service::show_map(const core::alarm_machine_ptr & machine)
{
	show_map(machine->get_ademco_id(), machine->get_is_submachine() ? machine->get_submachine_zone() : 0);
}

std::vector<core::MachineUuid> alarm_center_map_service::get_buffered_machines()
{
	std::lock_guard<std::mutex> lock(lock_4_machine_uuids_);
	auto v = machine_uuids_;
	machine_uuids_.clear();
	return v;
}



















}