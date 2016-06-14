#include "stdafx.h"
#include "VideoDeviceInfoJovision.h"
#include "VideoManager.h"
#include "VideoUserInfo.h"

namespace video {
namespace jovision {

video_device_info_jovision::video_device_info_jovision()
	: video_device_info()
{}


video_device_info_jovision::~video_device_info_jovision()
{}


std::wstring video_device_info_jovision::get_formatted_name(const std::wstring& seperator) const
{
	std::wstringstream ss;
	ss << _id << seperator << _device_note << seperator;
	if (connect_by_sse_or_ip_) {
		ss << utf8::a2w(cloud_sse_id_);
	} else {
		ss << utf8::a2w(device_ipv4_) << L":" << device_port_;
	}
	
	return ss.str();
}

bool video_device_info_jovision::execute_update_info()
{
	auto vmgr = video::video_manager::get_instance();
	CString sql;
	sql.Format(L"update table_device_info_jovision set \
connect_by_sse_or_ip=%d,\
cloud_sse_id='%s',\
device_ipv4='%s',\
device_port=%d,\
user_name='%s',\
user_passwd='%s',\
user_info_id=%d,\
device_note='%s' where id=%d", 
			   connect_by_sse_or_ip_ ? 1 : 0, 
			   utf8::a2w(cloud_sse_id_).c_str(),
			   utf8::a2w(device_ipv4_).c_str(),
			   device_port_,
			   user_name_.c_str(),
			   utf8::a2w(user_passwd_).c_str(),
			   get_userInfo()->get_id(),
			   _device_note.c_str(),
			   _id);

	return vmgr->Execute(sql) ? true : false;
}





};
};

