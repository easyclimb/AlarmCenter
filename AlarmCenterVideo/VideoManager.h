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
	
	bool running_ = false;
	std::mutex mutex_ = {};
	std::condition_variable condvar_ = {};
	std::thread thread_ = {};
	void ThreadWorker();
public:
	~video_manager();
	void LoadFromDB(bool refresh_ezviz_users_device_list = true);
	productor ProductorEzviz;
	productor ProductorJovision;
protected:
	void LoadUserInfoFromDB(bool refresh_ezviz_users_device_list = true);
	bool LoadUserInfoEzvizFromDB(const ezviz::ezviz_user_ptr& user, bool refresh_ezviz_users_device_list = true);
	bool LoadUserInfoJovisinoFromDB(const jovision::jovision_user_ptr& user);
	int LoadDeviceInfoEzvizFromDB(ezviz::ezviz_user_ptr userInfo);
	int LoadDeviceInfoJovisionFromDB(jovision::jovision_user_ptr userInfo);
	void LoadBindInfoFromDB();
	const productor video_manager::GetProductorInfo(int productor);
	
public:

	// common
	bool execute_set_user_name(const user_ptr& user, const std::wstring& name);
	bool execute_update_dev(const device_ptr& device);

	// ezviz
	// ezviz user
	bool execute_set_ezviz_users_acc_token(const video::ezviz::ezviz_user_ptr& user, const std::string& accToken);
	bool execute_set_ezviz_users_token_time(const video::ezviz::ezviz_user_ptr& user, const std::chrono::system_clock::time_point& tp);
	bool execute_add_device_for_ezviz_user(const video::ezviz::ezviz_user_ptr& user, const video::ezviz::ezviz_device_ptr& device);
	bool execute_del_ezviz_users_device(const video::ezviz::ezviz_user_ptr& user, const video::ezviz::ezviz_device_ptr& device);
	// ezviz device
	bool execute_update_ezviz_dev(const video::ezviz::ezviz_device_ptr& device);

	// jovision
	// jovision user
	bool execute_set_jovision_users_global_user_name(const video::jovision::jovision_user_ptr& user, const std::wstring& name);
	bool execute_set_jovision_users_global_user_passwd(const video::jovision::jovision_user_ptr& user, const std::string& passwd);
	bool execute_del_jovision_users_device(video::jovision::jovision_user_ptr& user, video::jovision::jovision_device_ptr& device);
	bool execute_add_device_for_jovision_user(const video::jovision::jovision_user_ptr& user, const video::jovision::jovision_device_ptr& dev);
	// jovision device
	bool execute_update_jovision_dev(const video::jovision::jovision_device_ptr& dev);

	

	bool AddVideoDeviceJovision(const jovision::jovision_user_ptr& user, const jovision::jovision_device_ptr& device);
	void GetVideoUserList(user_list& list);
	void GetVideoDeviceList(device_list& list);
	bool GetVideoDeviceInfo(int id, productor_type productor, device_ptr& device);
	ezviz::ezviz_device_ptr GetVideoDeviceInfoEzviz(int id);
	jovision::jovision_device_ptr GetVideoDeviceInfoJovision(int id);
	video::device_ptr GetVideoDeviceInfo(video::video_device_identifier* data);
	
	bool BindZoneAndDevice(const zone_uuid& zoneUuid, video::device_ptr device);
	bool UnbindZoneAndDevice(const zone_uuid& zoneUuid);

	bool DeleteVideoUserEzviz(ezviz::ezviz_user_ptr userInfo);
	bool DeleteVideoUserJovision(jovision::jovision_user_ptr userInfo);
	ezviz::ezviz_user_ptr GetVideoUserEzviz(int id);
	jovision::jovision_user_ptr GetVideoUserJovision(int id);
	VideoEzvizResult AddVideoUserEzviz(ezviz::ezviz_user_ptr user);
	VideoEzvizResult AddVideoUserJovision(jovision::jovision_user_ptr user);
	bool CheckIfUserEzvizPhoneExists(const std::string& user_phone);
	bool CheckIfUserJovisionNameExists(const std::wstring& user_name);
	VideoEzvizResult RefreshUserEzvizDeviceList(ezviz::ezviz_user_ptr userInfo);
	bind_info GetBindInfo(const zone_uuid& zone);
	bool SetBindInfoAutoPlayVideoOnAlarm(const zone_uuid& zone, int auto_play_when_alarm);
	void CheckUserAcctkenTimeout();

	BOOL Execute(const CString& sql);
	int AddAutoIndexTableReturnID(const CString& query);

protected:

	video_manager();
};


};
