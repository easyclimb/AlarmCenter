#pragma once
#include <list>
namespace ado { class CADODatabase; };
namespace core {
namespace video {

class CVideoUserInfo;
typedef std::list<CVideoUserInfo*> CVideoUserInfoList;

class CVideoDeviceInfo;
typedef std::list<CVideoDeviceInfo*> CVideoDeviceInfoList;


typedef struct ZoneUuid {
	int _ademco_id;
	int _zone_value;
	int _gg;
}ZoneUuid;

typedef struct BindInfo {
	int _device_info_id;
	int _productor_info_id;
	int _auto_play_video;
}BindInfo;

typedef std::list<BindInfo*> CBindInfoList;

class CVideoManager
{
private:
	ado::CADODatabase* m_pDatabase;
	CVideoUserInfoList _userList;
	CVideoDeviceInfoList _deviceList;
	CBindInfoList _bindInfoList;
public:
	//CVideoManager();
	~CVideoManager();

	void LoadFromDB();
	void LoadDeviceInfoEzvizFromDB();
	bool LoadUserInfoEzvizFromDB(int user_id, CVideoUserInfo** ppUserInfo);
	void LoadEzvizPrivateCloudInfoFromDB();
	void LoadBindInfoFromDB();

	DECLARE_UNCOPYABLE(CVideoManager)
	DECLARE_SINGLETON(CVideoManager)
};


NAMESPACE_END
NAMESPACE_END
