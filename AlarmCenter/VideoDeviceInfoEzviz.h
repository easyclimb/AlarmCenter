#pragma once
#include "VideoDeviceInfo.h"
namespace core {
namespace video {
namespace ezviz {

class CVideoDeviceInfoEzviz :
	public CVideoDeviceInfo
{
private:
	std::string _cameraId;
	std::string _cameraName;
	int _cameraNo;
	int _defence;
	std::string _deviceId;
	std::string _deviceName;
	std::string _deviceSerial;
	int _isEncrypt;
	std::string _isShared;
	std::string _picUrl;
	int _status;
	std::string _secure_code;
	//std::wstring _device_note;
public:

	bool execute_update_info();


	DECLARE_GETTER_SETTER(std::string, _cameraId);
	DECLARE_GETTER_SETTER(std::string, _cameraName);
	DECLARE_GETTER_SETTER_INT(_cameraNo);
	DECLARE_GETTER_SETTER_INT(_defence);
	DECLARE_GETTER_SETTER(std::string, _deviceId);
	DECLARE_GETTER_SETTER(std::string, _deviceName);
	DECLARE_GETTER_SETTER(std::string, _deviceSerial);
	DECLARE_GETTER_SETTER_INT(_isEncrypt);
	DECLARE_GETTER_SETTER(std::string, _isShared);
	DECLARE_GETTER_SETTER(std::string, _picUrl);
	DECLARE_GETTER_SETTER_INT(_status);
	DECLARE_GETTER_SETTER(std::string, _secure_code);
	//DECLARE_GETTER_SETTER(std::wstring, _device_note);

	

	CVideoDeviceInfoEzviz();
	virtual ~CVideoDeviceInfoEzviz();
};



NAMESPACE_END
NAMESPACE_END
NAMESPACE_END

