#pragma once
#include "../video/VideoUserInfo.h"

namespace video {
namespace jovision {

class video_user_info_jovision :
	public video_user_info
{
private:
	std::wstring global_user_name_ = L"";
	std::string global_user_passwd_ = "";
public:

	bool execute_add_device(video_device_info_jovision_ptr dev);

	std::wstring get_global_user_name() const { return global_user_name_; }
	void set_global_user_name(const std::wstring& name) { global_user_name_ = name; }
	bool execute_set_global_user_name(const std::wstring& name);

	std::string get_global_user_passwd() const { return global_user_passwd_; }
	void set_global_user_passwd(const std::string& passwd) { global_user_passwd_ = passwd; }
	bool execute_set_global_user_passwd(const std::string& passwd);
	
	virtual bool DeleteVideoDevice(video_device_info_jovision_ptr device);

	video_user_info_jovision();
	virtual ~video_user_info_jovision();
};




};
};

