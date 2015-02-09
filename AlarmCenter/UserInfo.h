#pragma once

#include <list>

namespace ado { class CADODatabase; };

namespace core {

enum UserPriority {
	UP_SUPER = 0,
	UP_ADMIN,
	UP_OPERATOR,
};

class CUserInfo
{
private:
	//int _id;
	int _user_id;
	UserPriority _user_priority;
	wchar_t* _user_name;
	wchar_t* _user_passwd;
	wchar_t* _user_phone;
public:
	CUserInfo();
	~CUserInfo();
	CUserInfo(const CUserInfo& rhs) { CopyFrom(rhs); }
	CUserInfo& operator=(const CUserInfo& rhs) { CopyFrom(rhs); }

	//DEALARE_GETTER_SETTER_INT(_user_level);
	UserPriority get_user_priority() const { return _user_priority; }
	void set_user_priority(int priority) { _user_priority = IntegerToUserPriority(priority); }
	void set_user_priority(UserPriority priority) { _user_priority = priority; }



	//DEALARE_GETTER_SETTER_INT(_id);
	DEALARE_GETTER_SETTER_INT(_user_id);

	DECLARE_GETTER_SETTER_STRING(_user_name);
	DECLARE_GETTER_SETTER_STRING(_user_passwd);
	DECLARE_GETTER_SETTER_STRING(_user_phone);

protected:

	void CopyFrom(const CUserInfo& rhs) {
		//set_id(rhs.get_id());
		set_user_id(rhs.get_user_id());
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

typedef void(_stdcall *OnCurUserChangedCB)(void* udata, CUserInfo* user);

class CUserManager 
{
	
private:
	std::list<CUserInfo*> _userList;
	CUserInfo* _curUser;
	CLock _lock4CurUser;
	ado::CADODatabase* _database;
	std::list<CUserInfo*>::iterator _curUserIter;
public:
	~CUserManager();
	//void AddUser(CUserInfo* user) { _userList.push_back(user); }
	BOOL UserExists(int user_id);
	BOOL UserExists(const wchar_t* user_name);
	BOOL Login(int user_id, const wchar_t* user_passwd);
	BOOL Login(const wchar_t* user_name, const wchar_t* user_passwd);
	CUserInfo* GetCurUserInfo() { CLocalLock lock(_lock4CurUser.GetObject()); return _curUser; }
	CUserInfo* GetFirstUserInfo();
	CUserInfo* GetNextUserInfo();
	int DistributeUserID();
	BOOL UpdateUserInfo(int user_id, const CUserInfo& newUserInfo);
	BOOL AddUser(const CUserInfo& newUserInfo);
	BOOL DeleteUser(const CUserInfo* user);
	BOOL ChangeUserPasswd(const CUserInfo* user, const wchar_t* passwd);
private:
	DECLARE_SINGLETON(CUserManager)
	DECLARE_UNCOPYABLE(CUserManager)
	DECLARE_OBSERVER(OnCurUserChangedCB, CUserInfo*)
};


NAMESPACE_END


