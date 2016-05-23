#pragma once
#include "VideoUserInfo.h"

namespace video {
namespace normal {
class video_user_info_normal :
	public video_user_info
{
private:
	std::string _user_acct;
	std::string _user_passwd;
public:

	DECLARE_GETTER_SETTER(std::string, _user_acct);
	DECLARE_GETTER_SETTER(std::string, _user_passwd);

	video_user_info_normal();
	virtual ~video_user_info_normal();
};




};
};

