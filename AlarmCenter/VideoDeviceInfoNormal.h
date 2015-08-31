#pragma once
#include "VideoDeviceInfo.h"

namespace core {
namespace video {
namespace normal {

class CVideoDeviceInfoNormal :
	public CVideoDeviceInfo
{
private:
	std::string _domain;
	std::string _ip;
	unsigned short _port;

public:
	
	DECLARE_GETTER_SETTER(std::string, _domain);
	DECLARE_GETTER_SETTER(std::string, _ip);
	DECLARE_GETTER_SETTER(unsigned short, _port);

	CVideoDeviceInfoNormal();
	virtual ~CVideoDeviceInfoNormal();
};




NAMESPACE_END
NAMESPACE_END
NAMESPACE_END

