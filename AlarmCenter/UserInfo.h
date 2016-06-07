#pragma once

#include <memory>
#include <list>
#include "core.h"

namespace SQLite { class Database; };

namespace core {

typedef enum user_priority {
	UP_SUPER = 0,
	UP_ADMIN,
	UP_OPERATOR,
}user_priority;

class user_info
{
private:
	//int _id;
	int _user_id;
	user_priority _user_priority;
	CString _user_name;
	CString _user_passwd;
	CString _user_phone;
public:
	user_info();
	~user_info();
	user_info(const user_info& rhs) { CopyFrom(rhs); }
	user_info& operator=(const user_info& rhs) { CopyFrom(rhs); }

	user_priority get_user_priority() const { return _user_priority; }
	void set_user_priority(int priority) { _user_priority = IntegerToUserPriority(priority); }
	void set_user_priority(user_priority priority) { _user_priority = priority; }

	DECLARE_GETTER_SETTER_INT(_user_id);
	DECLARE_GETTER_SETTER_STRING(_user_name);
	DECLARE_GETTER_SETTER_STRING(_user_passwd);
	DECLARE_GETTER_SETTER_STRING(_user_phone);

protected:

	void CopyFrom(const user_info& rhs) {
		set_user_id(rhs.get_user_id());
		set_user_priority(rhs.get_user_priority());
		set_user_name(rhs.get_user_name());
		set_user_phone(rhs.get_user_phone());
		set_user_passwd(rhs.get_user_passwd());
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
	std::list<user_info_ptr> _userList;
	user_info_ptr _curUser;
	std::mutex _lock4CurUser;
	std::shared_ptr<SQLite::Database> db_;
	std::list<user_info_ptr>::iterator _curUserIter;
public:
	
	~user_manager();
	BOOL UserExists(int user_id, CString& user_name);
	BOOL UserExists(const wchar_t* user_name, int& user_id);
	BOOL Login(int user_id, const wchar_t* user_passwd);
	BOOL Login(const wchar_t* user_name, const wchar_t* user_passwd);
	user_info_ptr GetCurUserInfo() { std::lock_guard<std::mutex> lock(_lock4CurUser); return _curUser; }
	user_info_ptr GetFirstUserInfo();
	user_info_ptr GetNextUserInfo();
	user_info_ptr GetUserInfo(int user_id);
	int DistributeUserID();
	BOOL UpdateUserInfo(int user_id, const core::user_info_ptr& newUserInfo);
	BOOL AddUser(const core::user_info_ptr& newUserInfo);
	BOOL DeleteUser(const core::user_info_ptr& user);
	BOOL ChangeUserPasswd(const core::user_info_ptr& user, const wchar_t* passwd);
	int GetCurUserID() { return _curUser->get_user_id(); }

protected:
	//DECLARE_SINGLETON(user_manager)
	user_manager();
};
};



