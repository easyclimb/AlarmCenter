#pragma once

#include "VideoUserInfo.h"

namespace video {
namespace ezviz {
class video_user_info_ezviz : public video_user_info, public std::enable_shared_from_this<video_user_info_ezviz>
{
private:
	std::string _user_phone;
	std::string _user_accToken;
	COleDateTime _user_tokenTime;
public:

	bool execute_set_user_accToken(const std::string& accToken);
	bool execute_add_device(video_device_info_ezviz_ptr device);
	bool execute_set_user_name(const std::wstring& name);
	bool execute_set_user_token_time(const COleDateTime& dt);
	virtual bool DeleteVideoDevice(video_device_info_ezviz_ptr device);

	DECLARE_GETTER_SETTER(std::string, _user_phone);
	DECLARE_GETTER_SETTER(std::string, _user_accToken);
	DECLARE_GETTER_SETTER(COleDateTime, _user_tokenTime);

	video_user_info_ezviz();
	virtual ~video_user_info_ezviz();
};};};
