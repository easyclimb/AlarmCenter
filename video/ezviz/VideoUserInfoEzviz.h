#pragma once

#include "../VideoUserInfo.h"

namespace video {
namespace ezviz {

class video_user_info_ezviz : public video_user_info, public std::enable_shared_from_this<video_user_info_ezviz>
{
private:
	std::string acc_token_ = "";
	std::chrono::system_clock::time_point token_time_ = std::chrono::system_clock::now();

protected:
	bool execute_set_token_time(const std::chrono::system_clock::time_point& tp);

public:

	std::string get_acc_token() const { return acc_token_; }
	void set_acc_token(const std::string& token) { acc_token_ = token; }

	auto get_token_time() const { return token_time_; }
	void set_token_time(const std::chrono::system_clock::time_point& tp) { token_time_ = tp; }

	bool execute_set_acc_token(const std::string& token);
	

	bool execute_add_device(video_device_info_ezviz_ptr device);
	virtual bool DeleteVideoDevice(video_device_info_ezviz_ptr device);

	video_user_info_ezviz();
	virtual ~video_user_info_ezviz();
};};};
