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


bool video_user_info_jovision::execute_add_device(video_device_info_jovision_ptr dev)
{
	CString sql;
	sql.Format(L"insert into table_device_info_jovision values(NULL,%d,'%s','%s',%d,'%s','%s',%d,'%s')",
			   dev->get_by_sse(), utf8::a2w(dev->get_sse()).c_str(), utf8::a2w(dev->get_ip()).c_str(),
			   dev->get_port(), dev->get_user_name().c_str(), utf8::a2w(dev->get_user_passwd()).c_str(),
			   _id, dev->get_device_note().c_str());
	int id = video_manager::get_instance()->AddAutoIndexTableReturnID(sql);
	if (id < 0) return false;
	dev->set_id(id);
	AddDevice(dev);
	return true;
}


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

