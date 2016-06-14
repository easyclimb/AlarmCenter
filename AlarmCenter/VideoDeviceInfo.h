#pragma once
#include "video.h"

namespace video {
class video_user_info;
class video_device_info
{
protected:
	int _id;
	std::wstring _device_note;
	video_user_info_weak_ptr _userInfo;
	std::list<zone_uuid> _zoneUuidList;
	bool _binded;

public:

	virtual std::wstring get_formatted_name(const std::wstring& seperator = L"--") const = 0;
	virtual video_device_identifier* create_identifier() const;

	DECLARE_GETTER_SETTER_INT(_id);
	DECLARE_GETTER_SETTER(std::wstring, _device_note);
	video_user_info_ptr get_userInfo() { return _userInfo.lock(); }
	DECLARE_SETTER_NONE_CONST(video_user_info_ptr, _userInfo);

	void add_zoneUuid(const zone_uuid& zoneUuid) { _zoneUuidList.push_back(zoneUuid); _zoneUuidList.unique(); }
	void get_zoneUuidList(std::list<zone_uuid>& list) { std::copy(_zoneUuidList.begin(), _zoneUuidList.end(), std::back_inserter(list)); }
	zone_uuid GetActiveZoneUuid() const { zone_uuid zoneUuid(0, 0, 0); if (!_zoneUuidList.empty()) { zoneUuid = _zoneUuidList.front(); } return zoneUuid; }
	void SetActiveZoneUuid(const zone_uuid& zoneUuid) { _zoneUuidList.remove(zoneUuid); _zoneUuidList.push_front(zoneUuid); }
	void del_zoneUuid(const zone_uuid& zoneUuid) { _zoneUuidList.remove(zoneUuid); }

	video_device_info();
	virtual ~video_device_info();
};

};

