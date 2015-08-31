#pragma once

#include "VideoUserInfo.h"
namespace core {
namespace video {
namespace ezviz {
class CVideoUserInfoEzviz : public CVideoUserInfo
{
private:
	std::string _phone;
	std::string _accToken;
public:
	
	DECLARE_GETTER_SETTER(std::string, _phone);
	DECLARE_GETTER_SETTER(std::string, _accToken);

	CVideoUserInfoEzviz();
	virtual ~CVideoUserInfoEzviz();
};
NAMESPACE_END
NAMESPACE_END
NAMESPACE_END
