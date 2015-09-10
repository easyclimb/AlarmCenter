#pragma once
#include "video.h"

namespace core {
namespace video {
class CVideoUserInfo;
class CVideoDeviceInfo
{
protected:
	int _id;
	std::wstring _device_note;
	CVideoUserInfo* _userInfo;
	ZoneUuid _zoneUuid;
public:

	DECLARE_GETTER_SETTER_INT(_id);
	DECLARE_GETTER_SETTER(std::wstring, _device_note);
	DECLARE_GETTER(CVideoUserInfo*, _userInfo);
	DECLARE_SETTER_NONE_CONST(CVideoUserInfo*, _userInfo);

	CVideoDeviceInfo();
	virtual ~CVideoDeviceInfo();
};


NAMESPACE_END
NAMESPACE_END
