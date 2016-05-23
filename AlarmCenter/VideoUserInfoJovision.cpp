#include "stdafx.h"
#include "VideoUserInfoJovision.h"
#include "VideoManager.h"

namespace video {
namespace jovision {

video_user_info_jovision::video_user_info_jovision()
{}


video_user_info_jovision::~video_user_info_jovision()
{}


bool video_user_info_jovision::execute_set_global_user_name(const std::wstring& name)
{
	std::wstringstream ss;
	ss << L"update user_info_jovision set global_user_name='" << name << L"' where id=" << real_user_id_;
	if (video_manager::GetInstance()->Execute(ss.str().c_str())) {
		set_global_user_name(name);
		return true;
	}
	return false;
}


bool video_user_info_jovision::execute_set_global_user_passwd(const std::string& passwd)
{
	std::wstringstream ss;
	ss << L"update user_info_jovision set global_user_passwd='" << utf8::a2w(passwd) << L"' where id=" << real_user_id_;
	if (video_manager::GetInstance()->Execute(ss.str().c_str())) {
		set_global_user_passwd(passwd);
		return true;
	}
	return false;
}


};
};

