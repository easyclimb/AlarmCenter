#pragma once

#include "video.h"

namespace ado { class CDbOper; };
namespace core {
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
	void LoadUserInfoEzvizFromDB();
	int LoadDeviceInfoEzvizFromDB(ezviz::CVideoUserInfoEzviz* userInfo);
	void LoadEzvizPrivateCloudInfoFromDB();
	void LoadBindInfoFromDB();

	void GetVideoUserList(CVideoUserInfoList& list);
	void GetVideoDeviceList(CVideoDeviceInfoList& list);
	bool GetVideoDeviceInfo(int id, PRODUCTOR productor, CVideoDeviceInfo*& device);

	BOOL Execute(const CString& sql);
	int AddAutoIndexTableReturnID(const CString& query);
	BOOL UpdatePrivateCloudInfo(const std::string& ip, int port);

	DECLARE_UNCOPYABLE(CVideoManager)
	DECLARE_SINGLETON(CVideoManager)
};


NAMESPACE_END
NAMESPACE_END
