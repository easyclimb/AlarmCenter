#pragma once

#include "video.h"
#include <list>
#include "video.h"

namespace video {

class video_user_info 
{
protected:
	int _id;
	int real_user_id_ = 0;
	std::wstring _user_name;
	std::string user_phone_ = "";
	productor_info _productorInfo;
	video_device_info_list _deviceList;
public:

	virtual DECLARE_GETTER_SETTER_INT(_id);

	virtual int get_real_user_id() const { return real_user_id_; }
	virtual void set_real_user_id(int id) { real_user_id_ = id; }

	virtual std::string get_user_phone() const { return user_phone_; }
	virtual void set_user_phone(const std::string& phone) { user_phone_ = phone; }

	virtual DECLARE_GETTER_SETTER(std::wstring, _user_name);
	virtual DECLARE_GETTER_SETTER(productor_info, _productorInfo);

	virtual void GetDeviceList(video_device_info_list& list);
	virtual int get_device_count() const { return _deviceList.size(); }
	virtual void AddDevice(video_device_info_ptr device) { _deviceList.push_back(device); }
	virtual bool DeleteVideoDevice(video_device_info_ptr) { return false; }
	
	// db oper
	virtual bool execute_set_user_name(const std::wstring& name);

	video_user_info();
	virtual ~video_user_info();
};
};
