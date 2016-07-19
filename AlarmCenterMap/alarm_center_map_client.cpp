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
			if (/*csr_in_.pt().x() !=  reply.pt().x() ||
				csr_in_.pt().y() != reply.pt().y() ||
				csr_in_.pt().level() != reply.pt().level() ||*/
				csr_in_.show() != reply.show()) {
				csr_in_ = reply;

				JLOGA("reply.show: %s", reply.show() ? "true" : "false");
				JLOGA("csr_in.show: %s", csr_in_.show() ? "true" : "false");
				
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

	bool set_csr() {
		alarm_center_map::csr_info request, reply;
		grpc::ClientContext context;
		request = csr_out_;
		auto status = stub_->set_csr_info(&context, request, &reply);
		if (status.ok()) {
			csr_out_updated_ = false;
			return true;
		}

		return false;
	}

	bool get_machines() {
		alarm_center_map::csr_info request;
		alarm_center_map::machine_info reply;
		grpc::ClientContext context;
		auto reader = stub_->get_alarming_machines_info(&context, request);
		while (reader->Read(&reply)) {
			auto info = std::make_shared<alarm_center_map::machine_info>();
			info->set_ademco_id(reply.ademco_id());
			info->set_zone_value(reply.zone_value());
			info->set_auto_popup(reply.auto_popup());
			info->set_title(reply.title());
			info->set_info(reply.info());
			info->mutable_pt()->set_x(reply.pt().x());
			info->mutable_pt()->set_y(reply.pt().y());
			info->mutable_pt()->set_level(reply.pt().level());
			updated_in_machines_.push_back(info);
		}

		auto status = reader->Finish();
		if (status.ok()) {
			auto app = AfxGetApp();
			if (app) {
				auto wnd = app->GetMainWnd();
				if (wnd) {
					wnd->PostMessageW(WM_SHOW_MACHINE_MAP);
				}
			}
			return true;
		}

		return false;
	}

	bool set_machines() {
		//alarm_center_map::csr_info reply;
		alarm_center_map::machine_info reply;
		grpc::ClientContext context;

		auto iter = updated_out_machines_.begin();
		while(iter != updated_out_machines_.end()) {
			auto status = stub_->set_machine_info(&context, *(iter->second), &reply);
			if (status.ok()) {
				iter = updated_out_machines_.erase(iter);
			} else {
				break;
			}
		}

		return updated_out_machines_.empty();
	}

//private:
	std::unique_ptr<alarm_center_map::map_service::Stub> stub_ = {};

	// send to server
	alarm_center_map::csr_info csr_out_ = {};
	bool csr_out_updated_ = false;
	std::map<core::MachineUuid, std::shared_ptr<alarm_center_map::machine_info>> updated_out_machines_ = {};

	// recv from server
	alarm_center_map::csr_info csr_in_ = {};
	std::vector<std::shared_ptr<alarm_center_map::machine_info>> updated_in_machines_ = {};
	
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
				{
					std::lock_guard<std::mutex> lg(mutex_);
					client_->get_csr();
				}

				last_time_get_csr_info = std::chrono::steady_clock::now();
			}
		}

		// get alarm info per 2s
		{
			auto now = std::chrono::steady_clock::now();
			auto diff = now - last_time_get_alarm_info;
			if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() >= 2) {
				{
					std::lock_guard<std::mutex> lg(mutex_);
					client_->get_machines();
				}
				last_time_get_alarm_info = std::chrono::steady_clock::now();
			}
		}

		// set csr info 
		{
			std::lock_guard<std::mutex> lg(mutex_);
			if (client_->csr_out_updated_) {
				client_->set_csr();
			}
		}

		// set machine info
		{
			std::lock_guard<std::mutex> lg(mutex_);
			if (!client_->updated_out_machines_.empty()) {
				client_->set_machines();
			}
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

std::vector<std::shared_ptr<alarm_center_map::machine_info>> alarm_center_map_client::get_machines()
{
	std::lock_guard<std::mutex> lg(mutex_);
	auto ret = client_->updated_in_machines_;
	client_->updated_in_machines_.clear();
	return ret;
}


}
