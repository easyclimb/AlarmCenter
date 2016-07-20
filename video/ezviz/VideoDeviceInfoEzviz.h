#pragma once
#include "../VideoDeviceInfo.h"

namespace video {
namespace ezviz {

class ezviz_device :
	public device
{
private:
	std::string _cameraId = "";
	std::wstring _cameraName = L"";
	int _cameraNo = 0;
	int _defence = 0;
	std::string _deviceId = "";
	std::wstring _deviceName = L"";
	std::string _deviceSerial = "";
	int _isEncrypt = 0;
	std::string _isShared = "";
	std::string _picUrl = "";
	int _status = 0;
	std::string _secure_code = "";
	//std::wstring _device_note;

	// 2015-11-17 15:58:27 show device icon on machine's map
	//int _detector_info_id;
public:
	ezviz_device() {};
	virtual ~ezviz_device() {};

	virtual std::wstring get_formatted_name(const std::wstring& seperator = L"--") const override {
		std::wstringstream ss;
		ss << _id << seperator << _device_note << seperator << utf8::a2w(_deviceSerial);
		return ss.str();
	}


	DECLARE_GETTER_SETTER(std::string, _cameraId);
	DECLARE_GETTER_SETTER(std::wstring, _cameraName);
	DECLARE_GETTER_SETTER_INT(_cameraNo);
	DECLARE_GETTER_SETTER_INT(_defence);
	DECLARE_GETTER_SETTER(std::string, _deviceId);
	DECLARE_GETTER_SETTER(std::wstring, _deviceName);
	DECLARE_GETTER_SETTER(std::string, _deviceSerial);
	DECLARE_GETTER_SETTER_INT(_isEncrypt);
	DECLARE_GETTER_SETTER(std::string, _isShared);
	DECLARE_GETTER_SETTER(std::string, _picUrl);
	DECLARE_GETTER_SETTER_INT(_status);
	DECLARE_GETTER_SETTER(std::string, _secure_code);
	//DECLARE_GETTER_SETTER(std::wstring, _device_note);
	//DECLARE_GETTER_SETTER_INT(_detector_info_id);

	static bool IsValidVerifyCode(const std::string& code)
	{
		bool valid = true;
		if (code.size() != 6)
			valid = false;
		for (auto a : code) {
			if (a <'A' || a >'Z') {
				valid = false;
				break;
			}
		}
		return valid;
	}

	
};



};};

