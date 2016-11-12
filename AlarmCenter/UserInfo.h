#pragma once

#include <memory>
#include <list>
#include "core.h"

namespace SQLite { class Database; };

namespace core {



class user_info
{
private:
	int id_ = 0;
	user_priority priority_ = UP_OPERATOR;
	std::wstring name_ = {};
	std::wstring passwd_ = {};
	std::wstring phone_ = {};
public:
	user_info();
	~user_info();
	user_info(const user_info& rhs) { copy_from(rhs); }
	user_info& operator=(const user_info& rhs) { copy_from(rhs); }

	user_priority get_priority() const { return priority_; }
	void set_priority(int priority) { priority_ = IntegerToUserPriority(priority); }
	void set_priority(user_priority priority) { priority_ = priority; }

	int get_id() const { return id_; }
	void set_id(int id) { id_ = id; }

	auto get_name() const { return name_; }
	void set_name(const std::wstring& name) { name_ = name; }

	auto get_passwd() const { return passwd_; }
	void set_passwd(const std::wstring& psw) { passwd_ = psw; }

	auto get_phone() const { return phone_; }
	void set_phone(const std::wstring& phone) { phone_ = phone; }

	std::wstring get_formmated_name() const {
		std::wstringstream ss;
		ss << id_ << L" " << name_ << L" " << phone_;
		return ss.str();
	}

protected:

	void copy_from(const user_info& rhs) {
		set_id(rhs.get_id());
		set_priority(rhs.get_priority());
		set_name(rhs.get_name());
		set_phone(rhs.get_phone());
		set_passwd(rhs.get_passwd());
	}

	static user_priority IntegerToUserPriority(int priority) {
		switch (priority) {
			case UP_SUPER:				return UP_SUPER;	break;
			case UP_ADMIN:				return UP_ADMIN;	break;
			case UP_OPERATOR:default:	return UP_OPERATOR;	break;
		}
	}
};


class user_manager : public dp::observable<user_info_ptr>, public dp::singleton<user_manager>
{
private:
	std::list<user_info_ptr> user_list_ = {};
	user_info_ptr cur_user_ = nullptr;
	std::recursive_mutex lock_for_cur_user_ = {};
	std::shared_ptr<SQLite::Database> db_ = nullptr;
	std::list<user_info_ptr>::iterator cur_user_iter_ = {};

	typedef std::lock_guard<std::recursive_mutex> lock_guard_type;

public:
	
	~user_manager();

	bool user_exists(int user_id, std::wstring& user_name);
	bool user_exists(const wchar_t* user_name, int& user_id);

	bool login(int user_id, const wchar_t* user_passwd);
	bool login(const wchar_t* user_name, const wchar_t* user_passwd);

	user_info_ptr get_cur_user_info() { lock_guard_type lock(lock_for_cur_user_); return cur_user_; }
	int get_cur_user_id() { lock_guard_type lock(lock_for_cur_user_); return cur_user_->get_id(); }
	auto get_cur_user_priority() { lock_guard_type lock(lock_for_cur_user_); return cur_user_->get_priority(); }

	user_info_ptr get_first_user_info();
	user_info_ptr get_next_user_info();
	user_info_ptr get_user_info(int user_id);

	int distribute_user_id();

	bool update_user_info(int user_id, const core::user_info_ptr& newUserInfo);
	bool add_user(const core::user_info_ptr& newUserInfo);
	bool delete_user(const core::user_info_ptr& user);
	bool change_user_passwd(const core::user_info_ptr& user, const wchar_t* passwd);

	std::wstring encrypt(const std::wstring& plain_text);
	
protected:
	user_manager();
};
};



