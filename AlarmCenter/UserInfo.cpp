#include "stdafx.h"
#include "UserInfo.h"
#include "md5.h"
#include <algorithm>
#include "ado2.h"

namespace core {

IMPLEMENT_SINGLETON(CUserManager)

CUserInfo::CUserInfo()
	: /*_id(0), */_user_id(0), _user_priority(UP_OPERATOR)
	, _user_name(NULL), _user_passwd(NULL), _user_phone(NULL)
{
	_user_name = new wchar_t[1];
	_user_name[0] = 0;

	_user_passwd = new wchar_t[1];
	_user_passwd[0] = 0;

	_user_phone = new wchar_t[1];
	_user_phone[0] = 0;
}


CUserInfo::~CUserInfo()
{
	SAFEDELETEARR(_user_name);
	SAFEDELETEARR(_user_passwd);
	SAFEDELETEARR(_user_phone);
}


IMPLEMENT_OBSERVER(CUserManager)

CUserManager::CUserManager() 
	: _curUser(NULL)
	, _database(NULL)
{
	
	try {
		_database = new ado::CADODatabase();
		LOG(_T("CUserManager after new, _pDatabase %x"), _database);
		LPCTSTR pszMdb = L"user_info.mdb";
		TCHAR szMdbPath[1024];
		_tcscpy_s(szMdbPath, GetModuleFilePath());
		_tcscat_s(szMdbPath, _T("\\"));
		_tcscat_s(szMdbPath, pszMdb);
		CLog::WriteLog(_T("CUserManager before pathexists"));
		if (!CFileOper::PathExists(szMdbPath)) {
			MessageBox(NULL, L"File 'user_info.mdb' missed or broken!", L"Error", MB_OK | MB_ICONERROR);
			ExitProcess(0);
			return;
		}
		LOG(_T("after pathexists"));
		//连接数据库
		CString strConn = _T("");
		strConn.Format(_T("Provider=Microsoft.Jet.OLEDB.4.0; Data Source='%s';Jet OLEDB:Database"), szMdbPath);
		CLog::WriteLog(strConn);
		if (!_database->Open(strConn)) {
			TRACE(_T("CUserManager _database->Open() error"));
			MessageBox(NULL, L"File user_info.mdb missed or broken!", L"Error", MB_OK | MB_ICONERROR);
			ExitProcess(0);
		} else {
			CLog::WriteLog(_T("_database->Open() ok"));
			CString trace = _T("");
			trace.Format(_T("CUserManager ConnectDB %s success\n"), strConn);
			CLog::WriteLog(trace);
		}

		static const wchar_t* query = L"select * from UserInfo order by id";
		ado::CADORecordset recordset(_database);
		recordset.Open(_database->m_pConnection, query);
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

				CUserInfo* user = new CUserInfo();
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
	} catch (...) {
		AfxMessageBox(_T("connect to access error!"));
		ExitProcess(0);
	}

	CLog::WriteLog(_T("CUserManager::CUserManager() ok"));
}


CUserManager::~CUserManager()
{
	std::list<CUserInfo*>::iterator iter = _userList.begin();
	while (iter != _userList.end()) {
		CUserInfo* user = *iter++;
		delete user;
	}

	if (_database) {
		if (_database->IsOpen()) {
			_database->Close();
		}
		delete _database;
	}

	DESTROY_OBSERVER;
}


BOOL CUserManager::UserExists(int user_id, CString& user_name)
{
	std::list<CUserInfo*>::iterator iter = _userList.begin();
	while (iter != _userList.end()) {
		CUserInfo* user = *iter++;
		if (user->get_user_id() == user_id) {
			user_name = user->get_user_name();
			return TRUE;
		}
	}
	return FALSE;
}


BOOL CUserManager::UserExists(const wchar_t* user_name, int& user_id)
{
	std::list<CUserInfo*>::iterator iter = _userList.begin();
	while (iter != _userList.end()) {
		CUserInfo* user = *iter++;
		if (wcscmp(user->get_user_name(), user_name) == 0) {
			user_id = user->get_user_id();
			return TRUE;
		}
	}
	return FALSE;
}



BOOL CUserManager::Login(int user_id, const wchar_t* user_passwd)
{
	CLocalLock lock(_lock4CurUser.GetObject());
	std::list<CUserInfo*>::iterator iter = _userList.begin();
	while (iter != _userList.end()) {
		CUserInfo* user = *iter++;
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
				NotifyObservers((const CUserInfo*)_curUser);
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
	CLocalLock lock(_lock4CurUser.GetObject());
	std::list<CUserInfo*>::iterator iter = _userList.begin();
	while (iter != _userList.end()) {
		CUserInfo* user = *iter++;
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
				NotifyObservers((const CUserInfo*)_curUser);
				return TRUE;
			} else {
				return FALSE;
			}
		}
	}
	return FALSE;
}


CUserInfo* CUserManager::GetFirstUserInfo()
{
	if (_userList.size() == 0)
		return NULL;

	_curUserIter = _userList.begin();
	if (_curUserIter != _userList.end()) {
		return *_curUserIter++;
	}
	return NULL;
}


CUserInfo* CUserManager::GetNextUserInfo()
{
	if (_userList.size() == 0)
		return NULL;

	if (_curUserIter != _userList.end()) {
		return *_curUserIter++;
	}
	return NULL;
}


int CUserManager::DistributeUserID()
{
	static const wchar_t* query = L"select max(user_id) as max_user_id from UserInfo";
	ado::CADORecordset recordset(_database);
	recordset.Open(_database->m_pConnection, query);
	DWORD count = recordset.GetRecordCount();
	if (count == 1) {
		recordset.MoveFirst();
		int id;
		recordset.GetFieldValue(L"max_user_id", id);
		recordset.Close();
		return ++id;
	}
	return -1;

	/*int id = 0;
	std::list<CUserInfo*>::iterator iter = _userList.begin();
	while (iter != _userList.end()) {
		CUserInfo* user = *iter++;
		if (id++ != user->get_user_id()) {
			return id;
		}
	}
	iter = _userList.end();
	iter--;
	if ()*/

}


BOOL CUserManager::UpdateUserInfo(int user_id, const CUserInfo& newUserInfo)
{
	CString query;
	query.Format(L"update UserInfo set user_priority=%d,user_name='%s',user_phone='%s' where user_id=%d",
				 newUserInfo.get_user_priority(), newUserInfo.get_user_name(),
				 newUserInfo.get_user_phone(), user_id);
	BOOL ok = _database->Execute(query);
	if (ok) {
		if (_curUser->get_user_id() == user_id) {
			_curUser->set_user_name(newUserInfo.get_user_name());
			_curUser->set_user_phone(newUserInfo.get_user_phone());
			_curUser->set_user_priority(newUserInfo.get_user_priority());
			NotifyObservers((const CUserInfo*)_curUser);
		} else {
			_curUserIter = _userList.begin();
			while (_curUserIter != _userList.end()) {
				CUserInfo* user = *_curUserIter++;
				if (user->get_user_id() == user_id) {
					user->set_user_name(newUserInfo.get_user_name());
					user->set_user_phone(newUserInfo.get_user_phone());
					user->set_user_priority(newUserInfo.get_user_priority());
					break;
				}
			}
		}
	}
	return ok;
}


BOOL CUserManager::AddUser(const CUserInfo& newUserInfo)
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
				 newUserInfo.get_user_id(), newUserInfo.get_user_priority(),
				 newUserInfo.get_user_name(), passwdW,
				 newUserInfo.get_user_phone());
	BOOL ok = _database->Execute(query);
	if (ok) {
		CUserInfo *user = new CUserInfo(newUserInfo);
		user->set_user_passwd(passwdW);
		_userList.push_back(user);
	}

	return ok;
}


BOOL CUserManager::DeleteUser(const CUserInfo* user)
{
	assert(user);
	CString query;
	query.Format(L"delete from UserInfo where user_id=%d", user->get_user_id());
	BOOL ok = _database->Execute(query);
	if (ok) {
		_curUserIter = _userList.begin();
		while (_curUserIter != _userList.end()) {
			CUserInfo* tmp_user = *_curUserIter;
			if (user == tmp_user) {
				delete tmp_user;
				_userList.erase(_curUserIter);
				break;
			}
			_curUserIter++;
		}
		_curUserIter = _userList.begin();
	}
	return ok;
}


BOOL CUserManager::ChangeUserPasswd(const CUserInfo* user, const wchar_t* passwd)
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
	BOOL ok = _database->Execute(query);
	if (ok) {
		_curUserIter = _userList.begin();
		while (_curUserIter != _userList.end()) {
			CUserInfo* tmp_user = *_curUserIter++;
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
