#pragma once
#include "VideoDeviceInfo.h"

namespace video {
namespace jovision {

class video_device_info_jovision :
	public video_device_info
{
private:
	bool connect_by_sse_or_ip_ = false;
	std::string cloud_sse_id_ = "";
	std::string device_ipv4_ = "";
	int device_port_ = 0;
	std::wstring user_name_ = L"";
	std::string user_passwd_ = "";

public:
	
	bool get_by_sse() const { return connect_by_sse_or_ip_; }
	void set_by_sse(bool b) { connect_by_sse_or_ip_ = b; }

	std::string get_sse() const { return cloud_sse_id_; }
	void set_sse(const std::string& sse) { cloud_sse_id_ = sse; }

	std::string get_ip() const { return device_ipv4_; }
	void set_ip(const std::string& ip) { device_ipv4_ = ip; }

	int get_port() const { return device_port_; }
	void set_port(int port) { device_port_ = port; }

	std::wstring get_user_name() const { return user_name_; }
	void set_user_name(const std::wstring& name) { user_name_ = name; }

	std::string get_user_passwd() const { return user_passwd_; }
	void set_user_passwd(const std::string& passwd) { user_passwd_ = passwd; }

	bool execute_update_info();

	video_device_info_jovision();
	virtual ~video_device_info_jovision();
};




};};

