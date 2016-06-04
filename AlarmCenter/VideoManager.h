#pragma once

#include "video.h"

namespace SQLite { class Database; };


namespace video {	

class video_manager : private boost::noncopyable
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
	video_device_info_list _deviceList;
	ezviz::video_device_info_ezviz_list _ezvizDeviceList;
	bind_map _bindMap;
	std::mutex _bindMapLock;
	
	HANDLE m_hThread;
	HANDLE m_hEvent;
public:
	//video_manager();
	~video_manager();
	void LoadFromDB();
	productor_info ProductorEzviz;
	productor_info ProductorJovision;
protected:
	void LoadUserInfoFromDB();
	bool LoadUserInfoEzvizFromDB(const ezviz::video_user_info_ezviz_ptr& user);
	bool LoadUserInfoJovisinoFromDB(const jovision::video_user_info_jovision_ptr& user);

	int LoadDeviceInfoEzvizFromDB(ezviz::video_user_info_ezviz_ptr userInfo);
	void LoadEzvizPrivateCloudInfoFromDB();
	void LoadBindInfoFromDB();
	const productor_info video_manager::GetProductorInfo(int productor);
	static DWORD WINAPI ThreadWorker(LPVOID);

public:
	void GetVideoUserList(video_user_info_list& list);
	void GetVideoDeviceList(video_device_info_list& list);
	void GetVideoDeviceEzvizWithDetectorList(ezviz::video_device_info_ezviz_list& list);
	bool GetVideoDeviceInfo(int id, productor productor, video_device_info_ptr& device);
	ezviz::video_device_info_ezviz_ptr GetVideoDeviceInfoEzviz(int id);
	
	bool BindZoneAndDevice(const zone_uuid& zoneUuid, ezviz::video_device_info_ezviz_ptr device);
	bool UnbindZoneAndDevice(const zone_uuid& zoneUuid);

	bool DeleteVideoUser(ezviz::video_user_info_ezviz_ptr userInfo);
	ezviz::video_user_info_ezviz_ptr GetVideoUserEzviz(int id);
	VideoEzvizResult AddVideoUserEzviz(ezviz::video_user_info_ezviz_ptr user);
	bool CheckIfUserEzvizPhoneExists(const std::string& user_phone);
	bool CheckIfUserJovisionNameExists(const std::wstring& user_name);
	VideoEzvizResult RefreshUserEzvizDeviceList(ezviz::video_user_info_ezviz_ptr userInfo);
	bind_info GetBindInfo(const zone_uuid& zone);
	bool SetBindInfoAutoPlayVideoOnAlarm(const zone_uuid& zone, int auto_play_when_alarm);
	void CheckUserAcctkenTimeout();

	BOOL Execute(const CString& sql);
	int AddAutoIndexTableReturnID(const CString& query);

	DECLARE_SINGLETON(video_manager)
};

};
