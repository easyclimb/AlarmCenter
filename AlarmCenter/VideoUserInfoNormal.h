#pragma once
#include "VideoUserInfo.h"
namespace core {
namespace video {
namespace normal {
class CVideoUserInfoNormal :
	public CVideoUserInfo
{
private:
	std::string _acct;
	std::string _passwd;
public:

	DECLARE_GETTER_SETTER(std::string, _acct);
	DECLARE_GETTER_SETTER(std::string, _passwd);

	CVideoUserInfoNormal();
	virtual ~CVideoUserInfoNormal();
};




NAMESPACE_END
NAMESPACE_END
NAMESPACE_END
