#pragma once

#include "VideoUserInfo.h"

namespace video {
namespace ezviz {
class CVideoUserInfoEzviz : public CVideoUserInfo
{
private:
	std::string _user_phone;
	std::string _user_accToken;
	COleDateTime _user_tokenTime;
public:

	bool execute_set_user_accToken(const std::string& accToken);
	bool execute_add_device(CVideoDeviceInfoEzviz* device);
	bool execute_set_user_name(const std::wstring& name);
	bool execute_set_user_token_time(const COleDateTime& dt);
	virtual bool DeleteVideoDevice(CVideoDeviceInfo* device);

	DECLARE_GETTER_SETTER(std::string, _user_phone);
	DECLARE_GETTER_SETTER(std::string, _user_accToken);
	DECLARE_GETTER_SETTER(COleDateTime, _user_tokenTime);

	CVideoUserInfoEzviz();
	virtual ~CVideoUserInfoEzviz();
};
NAMESPACE_END
NAMESPACE_END
