#pragma once

namespace alarm_center_map {
class machine_info;
}

namespace ipc {

class alarm_center_map_client : public dp::singleton<alarm_center_map_client>
{
protected:
	alarm_center_map_client();

	class ipc_client;
	std::unique_ptr<ipc_client> client_ = {};

	bool running_ = false;
	std::thread thread_ = {};
	void worker();

	std::mutex mutex_ = {};

public:
	~alarm_center_map_client();

	void set_csr_info(double x, double y, int level);
	void set_machine_info(double x, double y, int level, int ademco_id, int zone_value, bool auto_popup);

	void get_csr_info(double& x, double& y, int& level) const;
	std::vector<std::shared_ptr<alarm_center_map::machine_info>> get_machines();
};



}
