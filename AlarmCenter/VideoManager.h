#pragma once

#include "../video/video.h"

namespace SQLite { class Database; };


namespace video {	

class video_manager : public dp::singleton<video_manager>
{
public:
	typedef enum VideoEzvizResult
	{
		RESULT_OK,
		RESULT_USER_ALREADY_EXSIST,
		RESULT_PRIVATE_CLOUD_CONNECT_FAILED_OR_USER_NOT_EXIST,
		RESULT_INSERT_TO_DB_FAILED,
	}VideoEzvizResult;

private:
	std::shared_ptr<SQLite::Database> db_;
	user_list _userList;
	std::mutex _userListLock;
	device_list device_list_;
	ezviz::ezviz_device_list ezviz_device_list_;
	jovision::jovision_device_list jovision_device_list_;
	bind_map _bindMap;
	std::mutex _bindMapLock;
	
public:
	~video_manager();
	void LoadFromDB();
	productor ProductorEzviz;
	productor ProductorJovision;
protected:
	void LoadUserInfoFromDB();
	bool LoadUserInfoEzvizFromDB(const ezviz::ezviz_user_ptr& user);
	bool LoadUserInfoJovisinoFromDB(const jovision::jovision_user_ptr& user);
	int LoadDeviceInfoEzvizFromDB(ezviz::ezviz_user_ptr userInfo);
	int LoadDeviceInfoJovisionFromDB(jovision::jovision_user_ptr userInfo);
	void LoadBindInfoFromDB();
	const productor video_manager::GetProductorInfo(int productor);
	
public:


	void GetVideoUserList(user_list& list);
	void GetVideoDeviceList(device_list& list);
	bool GetVideoDeviceInfo(int id, productor_type productor, device_ptr& device);
	ezviz::ezviz_device_ptr GetVideoDeviceInfoEzviz(int id);
	jovision::jovision_device_ptr GetVideoDeviceInfoJovision(int id);
	video::device_ptr GetVideoDeviceInfo(video::video_device_identifier* data);
	
	bool BindZoneAndDevice(const zone_uuid& zoneUuid, video::device_ptr device);
	bool UnbindZoneAndDevice(const zone_uuid& zoneUuid);

	ezviz::ezviz_user_ptr GetVideoUserEzviz(int id);
	jovision::jovision_user_ptr GetVideoUserJovision(int id);
	bind_info GetBindInfo(const zone_uuid& zone);
	bool SetBindInfoAutoPlayVideoOnAlarm(const zone_uuid& zone, int auto_play_when_alarm);

	BOOL Execute(const CString& sql);
	int AddAutoIndexTableReturnID(const CString& query);

protected:

	video_manager();
};

};
