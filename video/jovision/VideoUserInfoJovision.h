#pragma once
#include "../VideoUserInfo.h"

namespace video {
namespace jovision {

class jovision_user : public user
{
private:
	std::wstring global_user_name_ = L"";
	std::string global_user_passwd_ = "";

public:
	jovision_user() {};
	virtual ~jovision_user() {};

	std::wstring get_global_user_name() const { return global_user_name_; }
	void set_global_user_name(const std::wstring& name) { global_user_name_ = name; }

	std::string get_global_user_passwd() const { return global_user_passwd_; }
	void set_global_user_passwd(const std::string& passwd) { global_user_passwd_ = passwd; }
	

};




};
};

