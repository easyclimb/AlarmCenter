#include "stdafx.h"
#include "UserInfo.h"
#include "md5.h"
#include <algorithm>
#include "ado2.h"

namespace core {

IMPLEMENT_SINGLETON(CUserManager)

CUserInfo::CUserInfo()
	: _id(0), _user_id(0), _user_level(0)
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
		strConn.Format(_T("Provider=Microsoft.Jet.OLEDB.4.0; Data Source='%s';Jet OLEDB:Database Password='888101'"), szMdbPath);
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
				int id, user_id, user_level;
				CString user_name, user_passwd, user_phone;
				recordset.GetFieldValue(L"id", id);
				recordset.GetFieldValue(L"user_id", user_id);
				recordset.GetFieldValue(L"user_level", user_level);
				recordset.GetFieldValue(L"user_name", user_name);
				recordset.GetFieldValue(L"user_passwd", user_passwd);
				recordset.GetFieldValue(L"user_phone", user_phone);
				recordset.MoveNext();

				CUserInfo* user = new CUserInfo();
				user->set_id(id);
				user->set_user_id(user_id);
				user->set_user_level(user_level);
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


BOOL CUserManager::UserExists(int user_id)
{
	std::list<CUserInfo*>::iterator iter = _userList.begin();
	while (iter != _userList.end()) {
		CUserInfo* user = *iter++;
		if (user->get_user_id() == user_id) {
			return TRUE;
		}
	}
	return FALSE;
}


BOOL CUserManager::UserExists(const wchar_t* user_name)
{
	std::list<CUserInfo*>::iterator iter = _userList.begin();
	while (iter != _userList.end()) {
		CUserInfo* user = *iter++;
		if (wcscmp(user->get_user_name(), user_name) == 0) {
			return TRUE;
		}
	}
	return FALSE;
}



BOOL CUserManager::CheckPasswd(int user_id, const wchar_t* user_passwd)
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
				return TRUE;
			} else {
				return FALSE;
			}
		}
	}
	return FALSE;
}



BOOL CUserManager::CheckPasswd(const wchar_t* user_name, const wchar_t* user_passwd)
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
				return TRUE;
			} else {
				return FALSE;
			}
		}
	}
	return FALSE;
}










NAMESPACE_END
