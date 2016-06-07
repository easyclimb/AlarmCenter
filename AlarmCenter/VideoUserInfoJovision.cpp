#include "stdafx.h"
#include "VideoUserInfoJovision.h"
#include "VideoManager.h"
#include "VideoDeviceInfoJovision.h"

namespace video {
namespace jovision {

video_user_info_jovision::video_user_info_jovision()
{}


video_user_info_jovision::~video_user_info_jovision()
{}


bool video_user_info_jovision::execute_set_global_user_name(const std::wstring& name)
{
	std::wstringstream ss;
	ss << L"update table_user_info_jovision set global_user_name='" << name << L"' where id=" << real_user_id_;
	if (video_manager::get_instance()->Execute(ss.str().c_str())) {
		set_global_user_name(name);
		video::video_device_info_list list;
		GetDeviceList(list);
		for (auto device : list) {
			auto dev = std::dynamic_pointer_cast<jovision::video_device_info_jovision>(device);
			dev->set_user_name(name);
			dev->execute_update_info();
		}
		return true;
	}
	return false;
}


bool video_user_info_jovision::execute_set_global_user_passwd(const std::string& passwd)
{
	std::wstringstream ss;
	ss << L"update table_user_info_jovision set global_user_passwd='" << utf8::a2w(passwd) << L"' where id=" << real_user_id_;
	if (video_manager::get_instance()->Execute(ss.str().c_str())) {
		set_global_user_passwd(passwd);
		video::video_device_info_list list;
		GetDeviceList(list);
		for (auto device : list) {
			auto dev = std::dynamic_pointer_cast<jovision::video_device_info_jovision>(device);
			dev->set_user_passwd(passwd);
			dev->execute_update_info();
		}
		return true;
	}
	return false;
}


};
};

