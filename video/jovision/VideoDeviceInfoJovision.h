#pragma once
#include "../VideoDeviceInfo.h"

namespace video {
namespace jovision {

class jovision_device : public device
{
private:
	bool connect_by_sse_or_ip_ = false;
	std::string cloud_sse_id_ = "";
	std::string device_ipv4_ = "";
	int device_port_ = 0;
	int channel_num_ = 0;
	std::wstring user_name_ = L"";
	std::string user_passwd_ = "";

public:

	jovision_device() {};
	virtual ~jovision_device() {};

	virtual std::wstring get_formatted_name(const std::wstring& seperator = L"--") const override {
		std::wstringstream ss;
		ss << _id << seperator << _device_note << seperator;
		if (connect_by_sse_or_ip_) {
			ss << utf8::a2w(cloud_sse_id_);
		} else {
			ss << utf8::a2w(device_ipv4_) << L":" << device_port_;
		}

		return ss.str();
	}

	JCLink_t link_id_ = -1;
	
	bool get_by_sse() const { return connect_by_sse_or_ip_; }
	void set_by_sse(bool b) { connect_by_sse_or_ip_ = b; }

	std::string get_sse() const { return cloud_sse_id_; }
	void set_sse(const std::string& sse) { cloud_sse_id_ = sse; }

	std::string get_ip() const { return device_ipv4_; }
	void set_ip(const std::string& ip) { device_ipv4_ = ip; }

	int get_port() const { return device_port_; }
	void set_port(int port) { device_port_ = port; }

	int get_channel_num() const { return channel_num_; }
	void set_channel_num(int num) { channel_num_ = num; }

	std::wstring get_user_name() const { return user_name_; }
	void set_user_name(const std::wstring& name) { user_name_ = name; }

	std::string get_user_passwd() const { return user_passwd_; }
	void set_user_passwd(const std::string& passwd) { user_passwd_ = passwd; }

	
};




};};

