#pragma once
#include "video.h"

namespace video {
class CVideoUserInfo;
class CVideoDeviceInfo
{
protected:
	int _id;
	std::wstring _device_note;
	CVideoUserInfo* _userInfo;
	ZoneUuid _zoneUuid;
	bool _binded;
public:

	DECLARE_GETTER_SETTER_INT(_id);
	DECLARE_GETTER_SETTER(std::wstring, _device_note);
	DECLARE_GETTER(CVideoUserInfo*, _userInfo);
	DECLARE_SETTER_NONE_CONST(CVideoUserInfo*, _userInfo);
	DECLARE_GETTER(ZoneUuid, _zoneUuid);
	DECLARE_GETTER_SETTER(bool, _binded);
	void set_zoneUuid(const ZoneUuid& zoneUuid) { _zoneUuid = zoneUuid; _binded = true; }
	

	CVideoDeviceInfo();
	virtual ~CVideoDeviceInfo();
};


NAMESPACE_END
