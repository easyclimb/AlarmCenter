#include "stdafx.h"
#include "UserInfo.h"
#include "md5.h"
#include <algorithm>
#include "sqlitecpp/SQLiteCpp.h"

using namespace SQLite;

namespace core {

//IMPLEMENT_SINGLETON(user_manager)

user_info::user_info()
	: /*_id(0), */_user_id(0), _user_priority(UP_OPERATOR)
	, _user_name(), _user_passwd(), _user_phone()
{
}


user_info::~user_info()
{
}


//IMPLEMENT_OBSERVER(user_manager)

user_manager::user_manager()
	: _curUser(nullptr)
	, db_(nullptr)
{
	auto path = get_config_path() + "\\user.db3";
	db_ = std::make_shared<Database>(path, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
	assert(db_);
	if (!db_) { return; }

	try {
		// check if db empty
		{
			Statement query(*db_, "select name from sqlite_master where type='table'");
			if (!query.executeStep()) {
				// init tables
				db_->exec("drop table if exists table_user");
				db_->exec("create table table_user (id integer primary key AUTOINCREMENT, \
user_id integer, \
user_priority integer, \
user_name text, \
user_passwd text, \
user_phone text)");
				db_->exec("insert into table_user values(NULL, 0, 0, \"admin\", \"e10adc3949ba59abbe56e057f20f883e\", \"\")");
			} else {
				std::string name = query.getColumn(0);
				JLOGA(name.c_str());
				while (query.executeStep()) {
					name = query.getColumn(0).getText();
					JLOGA(name.c_str());
				}
			}
		}
	} catch (std::exception& e) {
		JLOGA(e.what());
	}

	int user_id, user_priority;
	std::string user_name, user_passwd, user_phone;
	Statement query(*db_, "select user_id,user_priority,user_name,user_passwd,user_phone from table_user order by id");
	while (query.executeStep()) {
		int ndx = 0;
		user_id = query.getColumn(ndx++);
		user_priority = query.getColumn(ndx++);
		user_name = query.getColumn(ndx++).getText();
		user_passwd = query.getColumn(ndx++).getText();
		user_phone = query.getColumn(ndx++).getText();

		user_info_ptr user = std::make_shared<user_info>();
		user->set_user_id(user_id);
		user->set_user_priority(user_priority);
		user->set_user_name(utf8::a2w(user_name).c_str());
		user->set_user_passwd(utf8::a2w(user_passwd).c_str());
		user->set_user_phone(utf8::a2w(user_phone).c_str());
		_userList.push_back(user);
	}
}


user_manager::~user_manager()
{

}


BOOL user_manager::UserExists(int user_id, std::wstring& user_name)
{
	for (auto user : _userList) {
		if (user->get_user_id() == user_id) {
			user_name = user->get_user_name();
			return TRUE;
		}
	}
	return FALSE;
}


BOOL user_manager::UserExists(const wchar_t* user_name, int& user_id)
{
	for (auto user : _userList) {
		if (user->get_user_name() == user_name) {
			user_id = user->get_user_id();
			return TRUE;
		}
	}
	return FALSE;
}



BOOL user_manager::Login(int user_id, const wchar_t* user_passwd)
{
	std::lock_guard<std::mutex> lock(_lock4CurUser);
	for (auto user : _userList) {
		if (user->get_user_id() == user_id) {
			std::string passwdA = utf8::w2a(user_passwd);
			util::MD5 md5;
			md5.update(passwdA);
			std::string smd5 = md5.toString();
			std::transform(smd5.begin(), smd5.end(), smd5.begin(), ::tolower);
			std::wstring passwdW = utf8::a2w(smd5);
			if (user->get_user_passwd() == passwdW) {
				_curUser = user;
				notify_observers(_curUser);
				return TRUE;
			} else {
				return FALSE;
			}
		}
	}
	return FALSE;
}



BOOL user_manager::Login(const wchar_t* user_name, const wchar_t* user_passwd)
{
	std::lock_guard<std::mutex> lock(_lock4CurUser);
	for (auto user : _userList) {
		if (user->get_user_name() == user_name) {
			std::string passwdA = utf8::w2a(user_passwd);
			util::MD5 md5;
			md5.update(passwdA);
			std::string smd5 = md5.toString();
			std::transform(smd5.begin(), smd5.end(), smd5.begin(), ::tolower);
			std::wstring passwdW = utf8::a2w(smd5);
			if (user->get_user_passwd() == passwdW) {
				_curUser = user;
				notify_observers(_curUser);
				return TRUE;
			} else {
				return FALSE;
			}
		}
	}
	return FALSE;
}


user_info_ptr user_manager::GetFirstUserInfo()
{
	if (_userList.size() == 0)
		return nullptr;

	_curUserIter = _userList.begin();
	if (_curUserIter != _userList.end()) {
		return *_curUserIter++;
	}
	return nullptr;
}


user_info_ptr user_manager::GetNextUserInfo()
{
	if (_userList.size() == 0)
		return nullptr;

	if (_curUserIter != _userList.end()) {
		return *_curUserIter++;
	}
	return nullptr;
}


user_info_ptr user_manager::GetUserInfo(int user_id)
{
	for (auto user : _userList) {
		if (user_id == user->get_user_id()) {
			return user;
		}
	}
	return nullptr;
}


int user_manager::DistributeUserID()
{
	Statement query(*db_, "select max(user_id) from table_user");
	if (query.executeStep()) {
		int id = query.getColumn(0).getInt() + 1;
		return id;
	}
	
	return -1;
}


BOOL user_manager::UpdateUserInfo(int user_id, const core::user_info_ptr& newUserInfo)
{
	CString query;
	query.Format(L"update table_user set user_priority=%d,user_name='%s',user_phone='%s' where user_id=%d",
				 newUserInfo->get_user_priority(), newUserInfo->get_user_name().c_str(),
				 newUserInfo->get_user_phone().c_str(), user_id);
	BOOL ok = db_->exec(utf8::w2a((LPCTSTR)query)) > 0;
	if (ok) {
		if (_curUser->get_user_id() == user_id) {
			_curUser->set_user_name(newUserInfo->get_user_name());
			_curUser->set_user_phone(newUserInfo->get_user_phone());
			_curUser->set_user_priority(newUserInfo->get_user_priority());
			notify_observers(_curUser);
		} else {
			_curUserIter = _userList.begin();
			while (_curUserIter != _userList.end()) {
				user_info_ptr user = *_curUserIter++;
				if (user->get_user_id() == user_id) {
					user->set_user_name(newUserInfo->get_user_name());
					user->set_user_phone(newUserInfo->get_user_phone());
					user->set_user_priority(newUserInfo->get_user_priority());
					break;
				}
			}
		}
	}
	return ok;
}


BOOL user_manager::AddUser(const core::user_info_ptr& newUserInfo)
{
	const char* passwdA = "123456";
	util::MD5 md5;
	md5.update(passwdA, strnlen_s(passwdA, 1024));
	std::string smd5 = md5.toString();
	std::transform(smd5.begin(), smd5.end(), smd5.begin(), ::tolower);
	auto passwdW = utf8::a2w(smd5);

	CString query;
	query.Format(L"insert into table_user ([user_id],[user_priority],[user_name],[user_passwd],[user_phone]) values(%d,%d,'%s','%s','%s')",
				 newUserInfo->get_user_id(), newUserInfo->get_user_priority(),
				 newUserInfo->get_user_name().c_str(), passwdW.c_str(),
				 newUserInfo->get_user_phone().c_str());
	BOOL ok = db_->exec(utf8::w2a((LPCTSTR)query)) > 0;
	if (ok) {
		newUserInfo->set_user_passwd(passwdW);
		_userList.push_back(newUserInfo);
	}

	return ok;
}


BOOL user_manager::DeleteUser(const core::user_info_ptr& user)
{
	assert(user);
	CString query;
	query.Format(L"delete from table_user where user_id=%d", user->get_user_id());
	BOOL ok = db_->exec(utf8::w2a((LPCTSTR)query)) > 0;
	if (ok) {
		_curUserIter = _userList.begin();
		while (_curUserIter != _userList.end()) {
			user_info_ptr tmp_user = *_curUserIter;
			if (user == tmp_user) {
				_userList.erase(_curUserIter);
				break;
			}
			_curUserIter++;
		}
		_curUserIter = _userList.begin();
	}
	return ok;
}


BOOL user_manager::ChangeUserPasswd(const core::user_info_ptr& user, const wchar_t* passwd)
{
	assert(user);

	auto passwdA = utf8::w2a(passwd);
	util::MD5 md5;
	md5.update(passwdA);
	std::string smd5 = md5.toString();
	std::transform(smd5.begin(), smd5.end(), smd5.begin(), ::tolower);
	auto passwdW = utf8::a2w(smd5);

	CString query;
	query.Format(L"update table_user set user_passwd='%s' where user_id=%d",
				 passwdW.c_str(), user->get_user_id());
	BOOL ok = db_->exec(utf8::w2a((LPCTSTR)query)) > 0;
	if (ok) {
		_curUserIter = _userList.begin();
		while (_curUserIter != _userList.end()) {
			user_info_ptr tmp_user = *_curUserIter++;
			if (user == tmp_user) {
				tmp_user->set_user_passwd(passwdW);
				break;
			}
		}
		_curUserIter = _userList.begin();
	}
	return ok;
}

};
