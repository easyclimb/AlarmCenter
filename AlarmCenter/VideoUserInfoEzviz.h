#pragma once

#include "VideoUserInfo.h"
namespace core {
namespace video {
namespace ezviz {
class CVideoUserInfoEzviz : public CVideoUserInfo
{
private:
	std::string _user_phone;
	std::string _user_accToken;
public:

	bool execute_set_user_accToken(const std::string& accToken);
	bool execute_add_device(CVideoDeviceInfoEzviz* device);
	
	DECLARE_GETTER_SETTER(std::string, _user_phone);
	DECLARE_GETTER_SETTER(std::string, _user_accToken);

	CVideoUserInfoEzviz();
	virtual ~CVideoUserInfoEzviz();
};
NAMESPACE_END
NAMESPACE_END
NAMESPACE_END
