#pragma once
#include "VideoUserInfo.h"
namespace core {
namespace video {
namespace normal {
class CVideoUserInfoNormal :
	public CVideoUserInfo
{
private:
	std::string _user_acct;
	std::string _user_passwd;
public:

	DECLARE_GETTER_SETTER(std::string, _user_acct);
	DECLARE_GETTER_SETTER(std::string, _user_passwd);

	CVideoUserInfoNormal();
	virtual ~CVideoUserInfoNormal();
};




NAMESPACE_END
NAMESPACE_END
NAMESPACE_END
