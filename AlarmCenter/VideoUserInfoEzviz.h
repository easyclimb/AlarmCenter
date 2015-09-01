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
	
	DECLARE_GETTER_SETTER(std::string, _user_phone);
	DECLARE_GETTER_SETTER(std::string, _user_accToken);

	CVideoUserInfoEzviz();
	virtual ~CVideoUserInfoEzviz();
};
NAMESPACE_END
NAMESPACE_END
NAMESPACE_END
