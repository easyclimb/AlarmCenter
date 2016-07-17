#include "stdafx.h"
#include "alarm_center_map_client.h"

#include <grpc++/grpc++.h>
#include "../rpc/alarm_center_map.grpc.pb.h"
#include "../AlarmCenter/core.h"

namespace ipc {

class alarm_center_map_client::ipc_client
{
public:
	ipc_client(std::shared_ptr<grpc::Channel> channel) 
		: stub_(alarm_center_map::map_service::NewStub(channel))
	{}


	bool get_csr() {
		alarm_center_map::csr_info request, reply;
		grpc::ClientContext context;
		auto status = stub_->get_csr_info(&context, request, &reply);
		if (status.ok()) {
			if (csr_in_.pt().x() !=  reply.pt().x() ||
				csr_in_.pt().y() != reply.pt().y() ||
				csr_in_.pt().level() != reply.pt().level() ||
				csr_in_.show() != reply.show()) {
				csr_in_ = reply;
				
				if (reply.show()) {
					auto app = AfxGetApp();
					if (app) {
						auto wnd = app->GetMainWnd();
						if (wnd) {
							wnd->PostMessageW(WM_SHOW_CSR_MAP);
						}
					}
				}
			}
			return true;
		}
		return false;
	}


//private:
	std::unique_ptr<alarm_center_map::map_service::Stub> stub_ = {};

	// send to server
	alarm_center_map::csr_info csr_out_ = {};
	bool csr_out_updated_ = false;
	std::map<core::MachineUuid, std::shared_ptr<alarm_center_map::machine_info>> updated_out_machines_ = {};

	// recv from server
	alarm_center_map::csr_info csr_in_ = {};
	std::map<core::MachineUuid, std::shared_ptr<alarm_center_map::machine_info>> updated_in_machines_ = {};
	
};


alarm_center_map_client::alarm_center_map_client()
{
	client_ = std::make_unique<ipc_client>(grpc::CreateChannel("127.0.0.1:50051", grpc::InsecureChannelCredentials()));
	client_->get_csr();

	running_ = true;
	thread_ = std::thread(&alarm_center_map_client::worker, this);
}

alarm_center_map_client::~alarm_center_map_client()
{
	running_ = false;
	thread_.join();

	client_ = nullptr;
}

void alarm_center_map_client::worker()
{
	auto last_time_get_csr_info = std::chrono::steady_clock::now();
	auto last_time_get_alarm_info = std::chrono::steady_clock::now();

	while (running_) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		// get csr info per 3s
		{
			auto now = std::chrono::steady_clock::now();
			auto diff = now - last_time_get_csr_info;
			if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() >= 3) {
				client_->get_csr();
				last_time_get_csr_info = std::chrono::steady_clock::now();
			}
		}

		// get alarm info per 4s
		{
			auto now = std::chrono::steady_clock::now();
			auto diff = now - last_time_get_alarm_info;
			if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() >= 4) {

				last_time_get_alarm_info = std::chrono::steady_clock::now();
			}
		}

		// set csr info
		{

		}

		// set machine info
		{

		}
	}
}

void alarm_center_map_client::set_csr_info(double x, double y, int level)
{
	std::lock_guard<std::mutex> lg(mutex_);

	client_->csr_out_.mutable_pt()->set_x(x);
	client_->csr_out_.mutable_pt()->set_y(y);
	client_->csr_out_.mutable_pt()->set_level(level);
	client_->csr_out_updated_ = true;
}

void alarm_center_map_client::set_machine_info(double x, double y, int level, int ademco_id, int zone_value, bool auto_popup)
{
	std::lock_guard<std::mutex> lg(mutex_);

	core::MachineUuid uuid(ademco_id, zone_value);

	std::shared_ptr<alarm_center_map::machine_info> info = nullptr;

	auto iter = client_->updated_out_machines_.find(uuid);
	if (iter == client_->updated_out_machines_.end()) {
		info = std::make_shared<alarm_center_map::machine_info>();
		client_->updated_out_machines_[uuid] = info;
	} else {
		info = iter->second;
	}

	info->mutable_pt()->set_x(x);
	info->mutable_pt()->set_y(y);
	info->mutable_pt()->set_level(level);

	info->set_ademco_id(ademco_id);
	info->set_zone_value(zone_value);
	info->set_auto_popup(auto_popup);
}

void alarm_center_map_client::get_csr_info(double & x, double & y, int & level) const
{
	x = client_->csr_in_.pt().x();
	y = client_->csr_in_.pt().y();
	level = client_->csr_in_.pt().level();
}

void alarm_center_map_client::get_machines()
{
	std::lock_guard<std::mutex> lg(mutex_);

}


}
