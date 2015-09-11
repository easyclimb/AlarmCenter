#pragma once

#include "video.h"

namespace ado { class CDbOper; };
namespace video {

	

class CVideoManager
{
private:
	ado::CDbOper* m_db;
	CVideoUserInfoList _userList;
	CVideoDeviceInfoList _deviceList;
	CBindMap _bindMap;
public:
	//CVideoManager();
	~CVideoManager();
	void LoadFromDB();
protected:
	
	void LoadUserInfoEzvizFromDB();
	int LoadDeviceInfoEzvizFromDB(ezviz::CVideoUserInfoEzviz* userInfo);
	void LoadEzvizPrivateCloudInfoFromDB();
	void LoadBindInfoFromDB();

public:
	void GetVideoUserList(CVideoUserInfoList& list);
	void GetVideoDeviceList(CVideoDeviceInfoList& list);
	bool GetVideoDeviceInfo(int id, PRODUCTOR productor, CVideoDeviceInfo*& device);
	bool DeleteVideoUser(ezviz::CVideoUserInfoEzviz* userInfo);
	bool BindZoneAndDevice(ZoneUuid zoneUuid, ezviz::CVideoDeviceInfoEzviz* device);
	bool UnbindZoneAndDevice(ZoneUuid zoneUuid);
	bool DeleteVideoDevice(ezviz::CVideoDeviceInfoEzviz* device);
	bool AddVideoUserEzviz(const std::wstring& user_name, const std::string& user_phone);
	bool CheckIfUserEzvizPhoneExists(const std::string& user_phone);

	BOOL Execute(const CString& sql);
	int AddAutoIndexTableReturnID(const CString& query);
	BOOL UpdatePrivateCloudInfo(const std::string& ip, int port);

	DECLARE_UNCOPYABLE(CVideoManager)
	DECLARE_SINGLETON(CVideoManager)
};


NAMESPACE_END
