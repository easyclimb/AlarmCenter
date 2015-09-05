#pragma once

#include "video.h"
#include <list>
#include "video.h"

namespace core {
namespace video {
class CVideoDeviceInfo;
class CVideoUserInfo
{
protected:
	int _id;
	std::wstring _user_name;
	static CProductorInfo _productorInfo;
	CVideoDeviceInfoList _deviceList;
public:

	DECLARE_GETTER_SETTER_INT(_id);
	DECLARE_GETTER_SETTER(std::wstring, _user_name);
	DECLARE_GETTER(CProductorInfo, _productorInfo);
	//DECLARE_GETTER(std::list<CVideoDeviceInfo*>, _deviceList);
	void GetDeviceList(CVideoDeviceInfoList& list);
	int get_device_count() const { return _deviceList.size(); }
	void AddDevice(CVideoDeviceInfo* device) { _deviceList.push_back(device); }

	CVideoUserInfo();
	virtual ~CVideoUserInfo();
};

NAMESPACE_END
NAMESPACE_END
