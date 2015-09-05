#pragma once
#include <list>
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
	CBindInfoList _bindInfoList;
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

	void InsertInfoDeviceInfoEzviz(ezviz::CVideoDeviceInfoEzviz* device);

	DECLARE_UNCOPYABLE(CVideoManager)
	DECLARE_SINGLETON(CVideoManager)
};


NAMESPACE_END
NAMESPACE_END
