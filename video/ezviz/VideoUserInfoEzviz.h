#pragma once

#include "../VideoUserInfo.h"

namespace video {
namespace ezviz {

class ezviz_user : public user, public std::enable_shared_from_this<ezviz_user>
{
private:
	std::string acc_token_ = "";
	std::chrono::system_clock::time_point token_time_ = std::chrono::system_clock::now();

public:
	ezviz_user() {};
	virtual ~ezviz_user() {};

	std::string get_acc_token() const { return acc_token_; }
	void set_acc_token(const std::string& token) { acc_token_ = token; }

	auto get_token_time() const { return token_time_; }
	void set_token_time(const std::chrono::system_clock::time_point& tp) { token_time_ = tp; }
	
	
};
};};
