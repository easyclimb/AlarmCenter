#pragma once

#include <memory>
#include <list>
#include "observer.h"
#include "core.h"

namespace ado { class CDbOper; };

namespace core {

typedef enum UserPriority {
	UP_SUPER = 0,
	UP_ADMIN,
	UP_OPERATOR,
}UserPriority;

class CUserInfo
{
private:
	//int _id;
	int _user_id;
	UserPriority _user_priority;
	CString _user_name;
	CString _user_passwd;
	CString _user_phone;
public:
	CUserInfo();
	~CUserInfo();
	CUserInfo(const CUserInfo& rhs) { CopyFrom(rhs); }
	CUserInfo& operator=(const CUserInfo& rhs) { CopyFrom(rhs); }

	UserPriority get_user_priority() const { return _user_priority; }
	void set_user_priority(int priority) { _user_priority = IntegerToUserPriority(priority); }
	void set_user_priority(UserPriority priority) { _user_priority = priority; }

	DECLARE_GETTER_SETTER_INT(_user_id);
	DECLARE_GETTER_SETTER_STRING(_user_name);
	DECLARE_GETTER_SETTER_STRING(_user_passwd);
	DECLARE_GETTER_SETTER_STRING(_user_phone);

protected:

	void CopyFrom(const CUserInfo& rhs) {
		set_user_id(rhs.get_user_id());
		set_user_priority(rhs.get_user_priority());
		set_user_name(rhs.get_user_name());
		set_user_phone(rhs.get_user_phone());
		set_user_passwd(rhs.get_user_passwd());
	}

	static UserPriority IntegerToUserPriority(int priority) {
		switch (priority) {
			case UP_SUPER:				return UP_SUPER;	break;
			case UP_ADMIN:				return UP_ADMIN;	break;
			case UP_OPERATOR:default:	return UP_OPERATOR;	break;
		}
	}
};



//typedef void(__stdcall *OnCurUserChangedCB)(void* udata, CUserInfoPtr user);

class CUserManager : public dp::observable<CUserInfoPtr>
{
	
private:
	std::list<CUserInfoPtr> _userList;
	CUserInfoPtr _curUser;
	CLock _lock4CurUser;
	std::shared_ptr<ado::CDbOper> _db;
	std::list<CUserInfoPtr>::iterator _curUserIter;
public:
	~CUserManager();
	BOOL UserExists(int user_id, CString& user_name);
	BOOL UserExists(const wchar_t* user_name, int& user_id);
	BOOL Login(int user_id, const wchar_t* user_passwd);
	BOOL Login(const wchar_t* user_name, const wchar_t* user_passwd);
	CUserInfoPtr GetCurUserInfo() { CLocalLock lock(_lock4CurUser.GetLockObject()); return _curUser; }
	CUserInfoPtr GetFirstUserInfo();
	CUserInfoPtr GetNextUserInfo();
	CUserInfoPtr GetUserInfo(int user_id);
	int DistributeUserID();
	BOOL UpdateUserInfo(int user_id, CUserInfoPtr newUserInfo);
	BOOL AddUser(CUserInfoPtr newUserInfo);
	BOOL DeleteUser(CUserInfoPtr user);
	BOOL ChangeUserPasswd(CUserInfoPtr user, const wchar_t* passwd);
	int GetCurUserID() { CLocalLock lock(_lock4CurUser.GetLockObject()); return _curUser->get_user_id(); }
private:
	DECLARE_SINGLETON(CUserManager)
	DECLARE_UNCOPYABLE(CUserManager)
	//DECLARE_OBSERVER(OnCurUserChangedCB, CUserInfoPtr)
};


NAMESPACE_END


