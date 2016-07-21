#pragma once
#include "core.h"

class sub_process_mgr;

namespace grpc {
class Server;
}


namespace ipc {
class alarm_center_map_service : public dp::singleton<alarm_center_map_service>
{
	
protected:

	alarm_center_map_service(); 
	bool running_ = false;
	bool show_csr_map_ = false;
	bool shutdown_ok_ = false;
	std::mutex mutex_ = {};
	std::condition_variable cv_ = {};
	std::thread thread1_ = {};
	std::thread thread2_ = {};
	std::shared_ptr<::grpc::Server> server_ = {};
	std::shared_ptr<sub_process_mgr> sub_process_mgr_ = {};
	std::vector<core::MachineUuid> machine_uuids_ = {};
	std::mutex lock_4_machine_uuids_ = {};
	void daemon_baidu_map();

public:
	// must call shutdown before destruct service
	virtual ~alarm_center_map_service();
	void shutdown();
	
	void show_csr_map() { show_csr_map_ = true; }
	void show_map(int ademco_id, int zone_value);
	void show_map(const core::alarm_machine_ptr& machine);
	std::vector<core::MachineUuid> get_buffered_machines();

	class alarm_center_map_service_impl;
};











}
