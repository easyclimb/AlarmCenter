#pragma once
#include "video.h"

namespace core {
namespace video {
class CVideoUserInfo;
class CVideoDeviceInfo
{
protected:
	int _id;
	std::string _note;
	CVideoUserInfo* _userInfo;
public:

	DECLARE_GETTER_SETTER_INT(_id);
	DECLARE_GETTER_SETTER(std::string, _note);
	DECLARE_GETTER(CVideoUserInfo*, _userInfo);
	DECLARE_SETTER_NONE_CONST(CVideoUserInfo*, _userInfo);

	CVideoDeviceInfo();
	virtual ~CVideoDeviceInfo();
};


NAMESPACE_END
NAMESPACE_END
