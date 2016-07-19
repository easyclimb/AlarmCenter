#pragma once
#include "../video/VideoDeviceInfo.h"

namespace video {
namespace ezviz {

class video_device_info_ezviz :
	public video_device_info
{
private:
	std::string _cameraId;
	std::wstring _cameraName;
	int _cameraNo;
	int _defence;
	std::string _deviceId;
	std::wstring _deviceName;
	std::string _deviceSerial;
	int _isEncrypt;
	std::string _isShared;
	std::string _picUrl;
	int _status;
	std::string _secure_code;
	//std::wstring _device_note;

	// 2015-11-17 15:58:27 show device icon on machine's map
	//int _detector_info_id;
public:

	bool execute_update_info();

	virtual std::wstring get_formatted_name(const std::wstring& seperator = L"--") const override;


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

	video_device_info_ezviz();
	virtual ~video_device_info_ezviz();
};



};};

