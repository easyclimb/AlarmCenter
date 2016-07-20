#pragma once

#include <list>
#include "video.h"

namespace video {

class user 
{
protected:
	// table user id
	int _id; 

	// table ezviz_user/jovision_user id
	int real_user_id_ = 0; 

	std::wstring _user_name = L"";
	std::string user_phone_ = "";
	productor productor_ = {};
	device_list device_list_ = {};

public:
	user() {};
	virtual ~user() {};

	virtual DECLARE_GETTER_SETTER_INT(_id);

	virtual int get_real_user_id() const { return real_user_id_; }
	virtual void set_real_user_id(int id) { real_user_id_ = id; }

	virtual std::string get_user_phone() const { return user_phone_; }
	virtual void set_user_phone(const std::string& phone) { user_phone_ = phone; }

	virtual DECLARE_GETTER_SETTER(std::wstring, _user_name);
	
	virtual productor get_productor() const { return productor_; }
	virtual void set_productor(const productor& productor) { productor_ = productor; }

	virtual device_list get_device_list() const { return device_list_; }
	virtual int get_device_count() const { return device_list_.size(); }
	virtual void add_device(const device_ptr& device) { device_list_.push_back(device); }
	virtual void rm_device(const device_ptr& device) { device_list_.remove(device); }

	
};

};
