#pragma once
#include "video.h"
#include "VideoUserInfo.h"

namespace video {
class user;
class device
{
protected:
	int _id = 0;
	std::wstring _device_note = L"";
	user_weak_ptr _userInfo = {};
	std::list<zone_uuid> _zoneUuidList = {};
	bool _binded = false;

public:
	device() {};
	virtual ~device() {};

	virtual std::wstring get_formatted_name(const std::wstring& seperator = L"--") const = 0;

	virtual video_device_identifier* create_identifier() const {
		video_device_identifier* data = new video_device_identifier();
		data->dev_id = _id;
		auto user = _userInfo.lock();
		if (user) {
			data->productor_type = user->get_productor().get_productor_type();
		} else {
			assert(0);
			data->productor_type = video::UNKNOWN;
		}

		return data;
	}

	DECLARE_GETTER_SETTER_INT(_id);
	DECLARE_GETTER_SETTER(std::wstring, _device_note);
	user_ptr get_userInfo() { return _userInfo.lock(); }
	DECLARE_SETTER_NONE_CONST(user_ptr, _userInfo);

	void add_zoneUuid(const zone_uuid& zoneUuid) { _zoneUuidList.push_back(zoneUuid); _zoneUuidList.unique(); }
	void get_zoneUuidList(std::list<zone_uuid>& list) { std::copy(_zoneUuidList.begin(), _zoneUuidList.end(), std::back_inserter(list)); }
	zone_uuid GetActiveZoneUuid() const { zone_uuid zoneUuid(0, 0, 0); if (!_zoneUuidList.empty()) { zoneUuid = _zoneUuidList.front(); } return zoneUuid; }
	void SetActiveZoneUuid(const zone_uuid& zoneUuid) { _zoneUuidList.remove(zoneUuid); _zoneUuidList.push_front(zoneUuid); }
	void del_zoneUuid(const zone_uuid& zoneUuid) { _zoneUuidList.remove(zoneUuid); }

	
};

};

