#pragma once

#include "video.h"

namespace ado { class CDbOper; };
namespace video {

	
	

class CVideoManager
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
	ado::CDbOper* m_db;
	CVideoUserInfoList _userList;
	CLock _userListLock;
	CVideoDeviceInfoList _deviceList;
	ezviz::CVideoDeviceInfoEzvizList _ezvizDeviceList;
	CBindMap _bindMap;
	CProductorInfo ProductorEzviz;
public:
	//CVideoManager();
	~CVideoManager();
	void LoadFromDB();
protected:
	
	void LoadUserInfoEzvizFromDB();
	int LoadDeviceInfoEzvizFromDB(ezviz::CVideoUserInfoEzviz* userInfo);
	void LoadEzvizPrivateCloudInfoFromDB();
	void LoadBindInfoFromDB();
	const CProductorInfo CVideoManager::GetProductorInfo(int productor);
public:
	void GetVideoUserList(CVideoUserInfoList& list);
	void GetVideoDeviceList(CVideoDeviceInfoList& list);
	bool GetVideoDeviceInfo(int id, PRODUCTOR productor, CVideoDeviceInfo*& device);
	bool DeleteVideoUser(ezviz::CVideoUserInfoEzviz* userInfo);
	bool BindZoneAndDevice(ZoneUuid zoneUuid, ezviz::CVideoDeviceInfoEzviz* device);
	bool UnbindZoneAndDevice(ZoneUuid zoneUuid);
	VideoEzvizResult AddVideoUserEzviz(const std::wstring& user_name, const std::string& user_phone);
	bool CheckIfUserEzvizPhoneExists(const std::string& user_phone);
	VideoEzvizResult RefreshUserEzvizDeviceList(ezviz::CVideoUserInfoEzviz* userInfo);
	BindInfo GetBindInfo(const ZoneUuid& zone);
	bool SetBindInfoAutoPlayVideoOnAlarm(const ZoneUuid& zone, int auto_play_video);
	void CheckUserAcctkenTimeout();

	BOOL Execute(const CString& sql);
	int AddAutoIndexTableReturnID(const CString& query);
	BOOL UpdatePrivateCloudInfo(const std::string& ip, int port);

	DECLARE_UNCOPYABLE(CVideoManager)
	DECLARE_SINGLETON(CVideoManager)
};


NAMESPACE_END
