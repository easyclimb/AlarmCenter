#pragma once

#include "video.h"

namespace ado { class CDbOper; };
namespace video {	

class CVideoManager : private boost::noncopyable
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
	std::unique_ptr<ado::CDbOper> m_db;
	CVideoUserInfoList _userList;
	std::mutex _userListLock;
	CVideoDeviceInfoList _deviceList;
	ezviz::CVideoDeviceInfoEzvizList _ezvizDeviceList;
	CBindMap _bindMap;
	std::mutex _bindMapLock;
	CProductorInfo ProductorEzviz;
	HANDLE m_hThread;
	HANDLE m_hEvent;
public:
	//CVideoManager();
	~CVideoManager();
	void LoadFromDB();

protected:
	void LoadUserInfoEzvizFromDB();
	int LoadDeviceInfoEzvizFromDB(ezviz::CVideoUserInfoEzvizPtr userInfo);
	void LoadEzvizPrivateCloudInfoFromDB();
	void LoadBindInfoFromDB();
	const CProductorInfo CVideoManager::GetProductorInfo(int productor);
	static DWORD WINAPI ThreadWorker(LPVOID);

public:
	void GetVideoUserList(CVideoUserInfoList& list);
	void GetVideoDeviceList(CVideoDeviceInfoList& list);
	void GetVideoDeviceEzvizWithDetectorList(ezviz::CVideoDeviceInfoEzvizList& list);
	bool GetVideoDeviceInfo(int id, PRODUCTOR productor, CVideoDeviceInfoPtr& device);
	ezviz::CVideoDeviceInfoEzvizPtr GetVideoDeviceInfoEzviz(int id);
	
	bool BindZoneAndDevice(const ZoneUuid& zoneUuid, ezviz::CVideoDeviceInfoEzvizPtr device);
	bool UnbindZoneAndDevice(const ZoneUuid& zoneUuid);

	bool DeleteVideoUser(ezviz::CVideoUserInfoEzvizPtr userInfo);
	ezviz::CVideoUserInfoEzvizPtr GetVideoUserEzviz(int id);
	VideoEzvizResult AddVideoUserEzviz(ezviz::CVideoUserInfoEzvizPtr user);
	bool CheckIfUserEzvizPhoneExists(const std::string& user_phone);
	VideoEzvizResult RefreshUserEzvizDeviceList(ezviz::CVideoUserInfoEzvizPtr userInfo);
	BindInfo GetBindInfo(const ZoneUuid& zone);
	bool SetBindInfoAutoPlayVideoOnAlarm(const ZoneUuid& zone, int auto_play_video);
	void CheckUserAcctkenTimeout();

	BOOL Execute(const CString& sql);
	int AddAutoIndexTableReturnID(const CString& query);

	DECLARE_SINGLETON(CVideoManager)
};

};
