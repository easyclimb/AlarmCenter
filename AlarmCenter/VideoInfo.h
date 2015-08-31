#pragma once
#include <list>
namespace ado { class CADODatabase; };
namespace core {
namespace video {

class CVideoUserInfo;
typedef std::list<CVideoUserInfo*> CVideoUserInfoList;

class CVideoDeviceInfo;
typedef std::list<CVideoDeviceInfo*> CVideoDeviceInfoList;

class CVideoManager
{
private:
	ado::CADODatabase* m_pDatabase;
	CVideoUserInfoList _userList;
	CVideoDeviceInfoList _deviceList;
public:
	//CVideoManager();
	~CVideoManager();

	void LoadFromDB();
	void LoadDeviceInfoFromDB();
	void LoadUserInfoFromDB();

	DECLARE_UNCOPYABLE(CVideoManager)
	DECLARE_SINGLETON(CVideoManager)
};


NAMESPACE_END
NAMESPACE_END
