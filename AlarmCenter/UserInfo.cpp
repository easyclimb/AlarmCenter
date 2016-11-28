#include "stdafx.h"
#include "UserInfo.h"
#include "md5.h"
#include <algorithm>
#include "../contrib/sqlitecpp/SQLiteCpp.h"

using namespace SQLite;

namespace core {

//IMPLEMENT_SINGLETON(user_manager)

user_info::user_info()
{
}


user_info::~user_info()
{
}


//IMPLEMENT_OBSERVER(user_manager)

user_manager::user_manager()
{
	auto path = get_config_path_a() + "\\user.db3";
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
		user->set_id(user_id);
		user->set_priority(user_priority);
		user->set_name(utf8::a2w(user_name).c_str());
		user->set_passwd(utf8::a2w(user_passwd).c_str());
		user->set_phone(utf8::a2w(user_phone).c_str());
		user_list_.push_back(user);
	}
}


user_manager::~user_manager()
{

}


bool user_manager::user_exists(int user_id, std::wstring& user_name)
{
	for (auto user : user_list_) {
		if (user->get_id() == user_id) {
			user_name = user->get_name();
			return true;
		}
	}
	return false;
}


bool user_manager::user_exists(const wchar_t* user_name, int& user_id)
{
	for (auto user : user_list_) {
		if (user->get_name() == user_name) {
			user_id = user->get_id();
			return true;
		}
	}
	return false;
}



bool user_manager::login(int user_id, const wchar_t* user_passwd)
{
	lock_guard_type lock(lock_for_cur_user_);
	for (auto user : user_list_) {
		if (user->get_id() == user_id) {
			auto passwdW = encrypt(user_passwd);
			if (user->get_passwd() == passwdW) {
				cur_user_ = user;
				notify_observers(cur_user_);
				return true;
			} else {
				return false;
			}
		}
	}
	return false;
}


bool user_manager::login(const wchar_t* user_name, const wchar_t* user_passwd)
{
	lock_guard_type lock(lock_for_cur_user_);
	for (auto user : user_list_) {
		if (user->get_name() == user_name) {
			auto passwdW = encrypt(user_passwd);
			if (user->get_passwd() == passwdW) {
				cur_user_ = user;
				notify_observers(cur_user_);
				return true;
			} else {
				return false;
			}
		}
	}
	return false;
}


user_info_ptr user_manager::get_first_user_info()
{
	if (user_list_.size() == 0)
		return nullptr;

	cur_user_iter_ = user_list_.begin();
	if (cur_user_iter_ != user_list_.end()) {
		return *cur_user_iter_++;
	}
	return nullptr;
}


user_info_ptr user_manager::get_next_user_info()
{
	if (user_list_.size() == 0)
		return nullptr;

	if (cur_user_iter_ != user_list_.end()) {
		return *cur_user_iter_++;
	}
	return nullptr;
}


user_info_ptr user_manager::get_user_info(int user_id)
{
	for (auto user : user_list_) {
		if (user_id == user->get_id()) {
			return user;
		}
	}
	return nullptr;
}


int user_manager::distribute_user_id()
{
	Statement query(*db_, "select max(user_id) from table_user");
	if (query.executeStep()) {
		int id = query.getColumn(0).getInt() + 1;
		return id;
	}
	
	return -1;
}


bool user_manager::update_user_info(int user_id, const core::user_info_ptr& newUserInfo)
{
	CString query;
	query.Format(L"update table_user set user_priority=%d,user_name='%s',user_phone='%s' where user_id=%d",
				 newUserInfo->get_priority(), newUserInfo->get_name().c_str(),
				 newUserInfo->get_phone().c_str(), user_id);
	bool ok = db_->exec(utf8::w2a((LPCTSTR)query)) > 0;
	if (ok) {
		if (cur_user_->get_id() == user_id) {
			cur_user_->set_name(newUserInfo->get_name());
			cur_user_->set_phone(newUserInfo->get_phone());
			cur_user_->set_priority(newUserInfo->get_priority());
			notify_observers(cur_user_);
		} else {
			cur_user_iter_ = user_list_.begin();
			while (cur_user_iter_ != user_list_.end()) {
				user_info_ptr user = *cur_user_iter_++;
				if (user->get_id() == user_id) {
					user->set_name(newUserInfo->get_name());
					user->set_phone(newUserInfo->get_phone());
					user->set_priority(newUserInfo->get_priority());
					break;
				}
			}
		}
	}
	return ok;
}


bool user_manager::add_user(const core::user_info_ptr& newUserInfo)
{
	auto passwdW = encrypt(L"123456");

	CString query;
	query.Format(L"insert into table_user ([user_id],[user_priority],[user_name],[user_passwd],[user_phone]) values(%d,%d,'%s','%s','%s')",
				 newUserInfo->get_id(), newUserInfo->get_priority(),
				 newUserInfo->get_name().c_str(), passwdW.c_str(),
				 newUserInfo->get_phone().c_str());
	bool ok = db_->exec(utf8::w2a((LPCTSTR)query)) > 0;
	if (ok) {
		newUserInfo->set_passwd(passwdW);
		user_list_.push_back(newUserInfo);
	}

	return ok;
}


bool user_manager::delete_user(const core::user_info_ptr& user)
{
	assert(user);
	CString query;
	query.Format(L"delete from table_user where user_id=%d", user->get_id());
	bool ok = db_->exec(utf8::w2a((LPCTSTR)query)) > 0;
	if (ok) {
		cur_user_iter_ = user_list_.begin();
		while (cur_user_iter_ != user_list_.end()) {
			user_info_ptr tmp_user = *cur_user_iter_;
			if (user == tmp_user) {
				user_list_.erase(cur_user_iter_);
				break;
			}
			cur_user_iter_++;
		}
		cur_user_iter_ = user_list_.begin();
	}
	return ok;
}


bool user_manager::change_user_passwd(const core::user_info_ptr& user, const wchar_t* passwd)
{
	assert(user);

	auto passwdW = encrypt(passwd);

	CString query;
	query.Format(L"update table_user set user_passwd='%s' where user_id=%d",
				 passwdW.c_str(), user->get_id());
	bool ok = db_->exec(utf8::w2a((LPCTSTR)query)) > 0;
	if (ok) {
		cur_user_iter_ = user_list_.begin();
		while (cur_user_iter_ != user_list_.end()) {
			user_info_ptr tmp_user = *cur_user_iter_++;
			if (user == tmp_user) {
				tmp_user->set_passwd(passwdW);
				break;
			}
		}
		cur_user_iter_ = user_list_.begin();
	}
	return ok;
}

std::wstring user_manager::encrypt(const std::wstring & plain_text)
{
	auto passwdA = utf8::w2a(plain_text);
	util::MD5 md5;
	md5.update(passwdA);
	std::string smd5 = md5.toString();
	std::transform(smd5.begin(), smd5.end(), smd5.begin(), ::tolower);
	auto passwdW = utf8::a2w(smd5);
	return passwdW;
}

};
