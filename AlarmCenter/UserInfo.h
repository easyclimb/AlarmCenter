#pragma once

#include <list>

namespace ado { class CADODatabase; };

namespace core {

class CUserInfo
{
private:
	int _id;
	int _user_id;
	int _user_level;
	wchar_t* _user_name;
	wchar_t* _user_passwd;
	wchar_t* _user_phone;
public:
	CUserInfo();
	~CUserInfo();
	DEALARE_GETTER_SETTER_INT(_id);
	DEALARE_GETTER_SETTER_INT(_user_id);
	DEALARE_GETTER_SETTER_INT(_user_level);

	DECLARE_GETTER_SETTER_STRING(_user_name);
	DECLARE_GETTER_SETTER_STRING(_user_passwd);
	DECLARE_GETTER_SETTER_STRING(_user_phone);


};


class CUserManager 
{
private:
	std::list<CUserInfo*> _userList;
	CUserInfo* _curUser;
	CLock _lock4CurUser;
	ado::CADODatabase* _database;
public:
	~CUserManager();
	//void AddUser(CUserInfo* user) { _userList.push_back(user); }
	BOOL UserExists(int user_id);
	BOOL UserExists(const wchar_t* user_name);
	BOOL CheckPasswd(int user_id, const wchar_t* user_passwd);
	BOOL CheckPasswd(const wchar_t* user_name, const wchar_t* user_passwd);
	CUserInfo* GetCurUserInfo() { CLocalLock lock(_lock4CurUser.GetObject()); return _curUser; }
private:
	DECLARE_SINGLETON(CUserManager)
	DECLARE_UNCOPYABLE(CUserManager)
};


NAMESPACE_END


