#pragma once

#include "video.h"

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
	video_user_info_list _userList;
	std::mutex _userListLock;
	video_device_info_list device_list_;
	ezviz::video_device_info_ezviz_list ezviz_device_list_;
	jovision::video_device_info_jovision_list jovision_device_list_;
	bind_map _bindMap;
	std::mutex _bindMapLock;
	
	bool running_ = false;
	std::mutex mutex_ = {};
	std::condition_variable condvar_ = {};
	std::thread thread_ = {};
	void ThreadWorker();
public:
	~video_manager();
	void LoadFromDB();
	productor_info ProductorEzviz;
	productor_info ProductorJovision;
protected:
	void LoadUserInfoFromDB();
	bool LoadUserInfoEzvizFromDB(const ezviz::video_user_info_ezviz_ptr& user);
	bool LoadUserInfoJovisinoFromDB(const jovision::video_user_info_jovision_ptr& user);

	int LoadDeviceInfoEzvizFromDB(ezviz::video_user_info_ezviz_ptr userInfo);
	int LoadDeviceInfoJovisionFromDB(jovision::video_user_info_jovision_ptr userInfo);
	void LoadEzvizPrivateCloudInfoFromDB();
	void LoadBindInfoFromDB();
	const productor_info video_manager::GetProductorInfo(int productor);
	

public:
	bool AddVideoDeviceJovision(jovision::video_user_info_jovision_ptr user, jovision::video_device_info_jovision_ptr device);
	void GetVideoUserList(video_user_info_list& list);
	void GetVideoDeviceList(video_device_info_list& list);
	bool GetVideoDeviceInfo(int id, productor productor, video_device_info_ptr& device);
	ezviz::video_device_info_ezviz_ptr GetVideoDeviceInfoEzviz(int id);
	jovision::video_device_info_jovision_ptr GetVideoDeviceInfoJovision(int id);
	video::video_device_info_ptr GetVideoDeviceInfo(video::video_device_identifier* data);
	
	bool BindZoneAndDevice(const zone_uuid& zoneUuid, video::video_device_info_ptr device);
	bool UnbindZoneAndDevice(const zone_uuid& zoneUuid);

	bool DeleteVideoUserEzviz(ezviz::video_user_info_ezviz_ptr userInfo);
	bool DeleteVideoUserJovision(jovision::video_user_info_jovision_ptr userInfo);
	ezviz::video_user_info_ezviz_ptr GetVideoUserEzviz(int id);
	jovision::video_user_info_jovision_ptr GetVideoUserJovision(int id);
	VideoEzvizResult AddVideoUserEzviz(ezviz::video_user_info_ezviz_ptr user);
	VideoEzvizResult AddVideoUserJovision(jovision::video_user_info_jovision_ptr user);
	bool CheckIfUserEzvizPhoneExists(const std::string& user_phone);
	bool CheckIfUserJovisionNameExists(const std::wstring& user_name);
	VideoEzvizResult RefreshUserEzvizDeviceList(ezviz::video_user_info_ezviz_ptr userInfo);
	bind_info GetBindInfo(const zone_uuid& zone);
	bool SetBindInfoAutoPlayVideoOnAlarm(const zone_uuid& zone, int auto_play_when_alarm);
	void CheckUserAcctkenTimeout();

	BOOL Execute(const CString& sql);
	int AddAutoIndexTableReturnID(const CString& query);

protected:

	video_manager();
};

};
