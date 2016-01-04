#include "stdafx.h"
#include "UserInfo.h"
#include "md5.h"
#include <algorithm>
#include "DbOper.h"

namespace core {

IMPLEMENT_SINGLETON(CUserManager)

CUserInfo::CUserInfo()
	: /*_id(0), */_user_id(0), _user_priority(UP_OPERATOR)
	, _user_name(), _user_passwd(), _user_phone()
{
}


CUserInfo::~CUserInfo()
{
}


IMPLEMENT_OBSERVER(CUserManager)

CUserManager::CUserManager() 
	: _curUser(nullptr)
	, _db(nullptr)
{
	_db = std::make_shared<ado::CDbOper>();
	_db->Open(L"user_info.mdb");

	static const wchar_t* query = L"select * from UserInfo order by id";
	ado::CADORecordset recordset(_db->GetDatabase());
	recordset.Open(_db->GetDatabase()->m_pConnection, query);
	DWORD count = recordset.GetRecordCount();
	if (count > 0) {
		recordset.MoveFirst();
		for (DWORD i = 0; i < count; i++) {
			int /*id, */user_id, user_priority;
			CString user_name, user_passwd, user_phone;
			//recordset.GetFieldValue(L"id", id);
			recordset.GetFieldValue(L"user_id", user_id);
			recordset.GetFieldValue(L"user_priority", user_priority);
			recordset.GetFieldValue(L"user_name", user_name);
			recordset.GetFieldValue(L"user_passwd", user_passwd);
			recordset.GetFieldValue(L"user_phone", user_phone);
			recordset.MoveNext();

			CUserInfoPtr user = std::make_shared<CUserInfo>();
			//user->set_id(id);
			user->set_user_id(user_id);
			user->set_user_priority(user_priority);
			user->set_user_name(user_name);
			user->set_user_passwd(user_passwd);
			user->set_user_phone(user_phone);
			_userList.push_back(user);
		}
	}
	recordset.Close();
}


CUserManager::~CUserManager()
{
	_userList.clear();

	DESTROY_OBSERVER;
}


BOOL CUserManager::UserExists(int user_id, CString& user_name)
{
	for (auto user : _userList) {
		if (user->get_user_id() == user_id) {
			user_name = user->get_user_name();
			return TRUE;
		}
	}
	return FALSE;
}


BOOL CUserManager::UserExists(const wchar_t* user_name, int& user_id)
{
	for (auto user : _userList) {
		if (wcscmp(user->get_user_name(), user_name) == 0) {
			user_id = user->get_user_id();
			return TRUE;
		}
	}
	return FALSE;
}



BOOL CUserManager::Login(int user_id, const wchar_t* user_passwd)
{
	CLocalLock lock(_lock4CurUser.GetLockObject());
	for (auto user : _userList) {
		if (user->get_user_id() == user_id) {
			USES_CONVERSION;
			const char* passwdA = W2A(user_passwd);
			util::MD5 md5;
			md5.update(passwdA, strnlen_s(passwdA, 1024));
			std::string smd5 = md5.toString();
			std::transform(smd5.begin(), smd5.end(), smd5.begin(), ::tolower);
			const wchar_t* passwdW = A2W(smd5.c_str());
			if (wcscmp(user->get_user_passwd(), passwdW) == 0) {
				_curUser = user;
				NotifyObservers(_curUser);
				return TRUE;
			} else {
				return FALSE;
			}
		}
	}
	return FALSE;
}



BOOL CUserManager::Login(const wchar_t* user_name, const wchar_t* user_passwd)
{
	CLocalLock lock(_lock4CurUser.GetLockObject());
	for (auto user : _userList) {
		if (wcscmp(user->get_user_name(), user_name) == 0) {
			USES_CONVERSION;
			const char* passwdA = W2A(user_passwd);
			util::MD5 md5;
			md5.update(passwdA, strnlen_s(passwdA, 1024));
			std::string smd5 = md5.toString();
			std::transform(smd5.begin(), smd5.end(), smd5.begin(), ::tolower);
			const wchar_t* passwdW = A2W(smd5.c_str());
			if (wcscmp(user->get_user_passwd(), passwdW) == 0) {
				_curUser = user;
				NotifyObservers(_curUser);
				return TRUE;
			} else {
				return FALSE;
			}
		}
	}
	return FALSE;
}


CUserInfoPtr CUserManager::GetFirstUserInfo()
{
	if (_userList.size() == 0)
		return nullptr;

	_curUserIter = _userList.begin();
	if (_curUserIter != _userList.end()) {
		return *_curUserIter++;
	}
	return nullptr;
}


CUserInfoPtr CUserManager::GetNextUserInfo()
{
	if (_userList.size() == 0)
		return nullptr;

	if (_curUserIter != _userList.end()) {
		return *_curUserIter++;
	}
	return nullptr;
}


CUserInfoPtr CUserManager::GetUserInfo(int user_id)
{
	for (auto user : _userList) {
		if (user_id == user->get_user_id()) {
			return user;
		}
	}
	return nullptr;
}


int CUserManager::DistributeUserID()
{
	static const wchar_t* query = L"select max(user_id) as max_user_id from UserInfo";
	ado::CADORecordset recordset(_db->GetDatabase());
	recordset.Open(_db->GetDatabase()->m_pConnection, query);
	DWORD count = recordset.GetRecordCount();
	if (count == 1) {
		recordset.MoveFirst();
		int id;
		recordset.GetFieldValue(L"max_user_id", id);
		recordset.Close();
		return ++id;
	}
	return -1;
}


BOOL CUserManager::UpdateUserInfo(int user_id, CUserInfoPtr newUserInfo)
{
	CString query;
	query.Format(L"update UserInfo set user_priority=%d,user_name='%s',user_phone='%s' where user_id=%d",
				 newUserInfo->get_user_priority(), newUserInfo->get_user_name(),
				 newUserInfo->get_user_phone(), user_id);
	BOOL ok = _db->Execute(query);
	if (ok) {
		if (_curUser->get_user_id() == user_id) {
			_curUser->set_user_name(newUserInfo->get_user_name());
			_curUser->set_user_phone(newUserInfo->get_user_phone());
			_curUser->set_user_priority(newUserInfo->get_user_priority());
			NotifyObservers(_curUser);
		} else {
			_curUserIter = _userList.begin();
			while (_curUserIter != _userList.end()) {
				CUserInfoPtr user = *_curUserIter++;
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


BOOL CUserManager::AddUser(CUserInfoPtr newUserInfo)
{
	USES_CONVERSION;
	const char* passwdA = "123456";
	util::MD5 md5;
	md5.update(passwdA, strnlen_s(passwdA, 1024));
	std::string smd5 = md5.toString();
	std::transform(smd5.begin(), smd5.end(), smd5.begin(), ::tolower);
	const wchar_t* passwdW = A2W(smd5.c_str());

	CString query;
	query.Format(L"insert into [UserInfo] ([user_id],[user_priority],[user_name],[user_passwd],[user_phone]) values(%d,%d,'%s','%s','%s')",
				 newUserInfo->get_user_id(), newUserInfo->get_user_priority(),
				 newUserInfo->get_user_name(), passwdW,
				 newUserInfo->get_user_phone());
	BOOL ok = _db->Execute(query);
	if (ok) {
		newUserInfo->set_user_passwd(passwdW);
		_userList.push_back(newUserInfo);
	}

	return ok;
}


BOOL CUserManager::DeleteUser(CUserInfoPtr user)
{
	assert(user);
	CString query;
	query.Format(L"delete from UserInfo where user_id=%d", user->get_user_id());
	BOOL ok = _db->Execute(query);
	if (ok) {
		_curUserIter = _userList.begin();
		while (_curUserIter != _userList.end()) {
			CUserInfoPtr tmp_user = *_curUserIter;
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


BOOL CUserManager::ChangeUserPasswd(CUserInfoPtr user, const wchar_t* passwd)
{
	assert(user);

	USES_CONVERSION;
	const char* passwdA = W2A(passwd);
	util::MD5 md5;
	md5.update(passwdA, strnlen_s(passwdA, 1024));
	std::string smd5 = md5.toString();
	std::transform(smd5.begin(), smd5.end(), smd5.begin(), ::tolower);
	const wchar_t* passwdW = A2W(smd5.c_str());

	CString query;
	query.Format(L"update UserInfo set user_passwd='%s' where user_id=%d", 
				 passwdW, user->get_user_id());
	BOOL ok = _db->Execute(query);
	if (ok) {
		_curUserIter = _userList.begin();
		while (_curUserIter != _userList.end()) {
			CUserInfoPtr tmp_user = *_curUserIter++;
			if (user == tmp_user) {
				tmp_user->set_user_passwd(passwdW);
				break;
			}
		}
		_curUserIter = _userList.begin();
	}
	return ok;
}


NAMESPACE_END
