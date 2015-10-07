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
	std::list<ZoneUuid> _zoneUuidList;
	bool _binded;
public:

	DECLARE_GETTER_SETTER_INT(_id);
	DECLARE_GETTER_SETTER(std::wstring, _device_note);
	DECLARE_GETTER(CVideoUserInfo*, _userInfo);
	DECLARE_SETTER_NONE_CONST(CVideoUserInfo*, _userInfo);
	//DECLARE_GETTER(ZoneUuid, _zoneUuid);
	//DECLARE_GETTER_SETTER(bool, _binded);
	//void set_zoneUuid(const ZoneUuid& zoneUuid) { _zoneUuid = zoneUuid; _binded = true; }
	void add_zoneUuid(const ZoneUuid& zoneUuid) { _zoneUuidList.push_back(zoneUuid); _zoneUuidList.unique(); }
	void get_zoneUuidList(std::list<ZoneUuid>& list) { std::copy(_zoneUuidList.begin(), _zoneUuidList.end(), std::back_inserter(list)); }
	ZoneUuid GetActiveZoneUuid() const { ZoneUuid zoneUuid; if (!_zoneUuidList.empty()) { zoneUuid = _zoneUuidList.front(); } return zoneUuid; }
	void SetActiveZoneUuid(const ZoneUuid& zoneUuid) { _zoneUuidList.remove(zoneUuid); _zoneUuidList.push_front(zoneUuid); }
	void del_zoneUuid(const ZoneUuid& zoneUuid) { _zoneUuidList.remove(zoneUuid); }

	CVideoDeviceInfo();
	virtual ~CVideoDeviceInfo();
};


NAMESPACE_END
