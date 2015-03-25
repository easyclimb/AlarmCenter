// HistoryRecord.cpp: implementation of the CHistoryRecord class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AlarmCenter.h"
#include "HistoryRecord.h"
#include <odbcinst.h>
#include <afxdb.h>
#include <comdef.h>
#include "ado2.h"
#include "UserInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

namespace core {

IMPLEMENT_SINGLETON(CHistoryRecord)
IMPLEMENT_OBSERVER(CHistoryRecord)

static void __stdcall OnCurUesrChanged(void* udata, const CUserInfo* user) 
{
	CHistoryRecord* hr = reinterpret_cast<CHistoryRecord*>(udata); assert(hr);
	hr->OnCurUserChandedResult(user);
}

void CHistoryRecord::OnCurUserChandedResult(const core::CUserInfo* user)
{
	assert(user);
	if (m_curUserInfo == user)
		return;
	
	CString srecord, suser, slogin, slogout;
	suser.LoadString(IDS_STRING_USER);
	slogin.LoadStringW(IDS_STRING_LOGIN);
	slogout.LoadStringW(IDS_STRING_LOGOUT);

	if (m_curUserInfo) {
		srecord.Format(L"%s%s:(ID:%d, %s)", suser, slogout,
					   m_curUserInfo->get_user_id(),
					   m_curUserInfo->get_user_name());
		InsertRecord(-1, -1, srecord, time(NULL), RECORD_LEVEL_USERLOG);
	}

	m_curUserInfo = user;
	srecord.Format(L"%s%s:(ID:%d, %s)", suser, slogin,
				   m_curUserInfo->get_user_id(),
				   m_curUserInfo->get_user_name());
	InsertRecord(-1, -1, srecord, time(NULL), RECORD_LEVEL_USERLOG);
}

CHistoryRecord::CHistoryRecord()
	: m_bUpdated(TRUE)
	, m_pDatabase(NULL)
	, m_curUserInfo(NULL)
{
	CLog::WriteLog(_T("CHistoryRecord::CHistoryRecord()"));
	::InitializeCriticalSection(&m_csRecord);
	try {
		m_pDatabase = new ado::CADODatabase();
		LOG(_T("CHistoryRecord after new, m_pDatabase %x"), m_pDatabase);
		LPCTSTR pszMdb = L"HistoryRecord.mdb";
		TCHAR szMdbPath[1024];
		_tcscpy_s(szMdbPath, GetModuleFilePath());
		_tcscat_s(szMdbPath, _T("\\"));
		_tcscat_s(szMdbPath, pszMdb);
		CLog::WriteLog(_T("CHistoryRecord before pathexists"));
		if (!CFileOper::PathExists(szMdbPath)) {
			MessageBox(NULL, L"File 'HistoryRecord.mdb' missed or broken!", L"Error", MB_OK | MB_ICONERROR);
			ExitProcess(0);
			return;
		}
		TRACE(_T("after pathexists"));

		CString strConn = _T("");
		strConn.Format(_T("Provider=Microsoft.Jet.OLEDB.4.0; Data Source='%s';Jet OLEDB:Database Password='888101'"), szMdbPath);
		CLog::WriteLog(strConn);
		if (!m_pDatabase->Open(strConn)) {
			TRACE(_T("CHistoryRecord m_pDatabase->Open() error"));
			MessageBox(NULL, L"File HistoryRecord.mdb missed or broken!", L"Error", MB_OK | MB_ICONERROR);
			ExitProcess(0);
		} else {
			CLog::WriteLog(_T("m_pDatabase->Open() ok"));
			CString trace = _T("");
			trace.Format(_T("CHistoryRecord ConnectDB %s success\n"), strConn);
			CLog::WriteLog(trace);
		}
	} catch (...) {
		AfxMessageBox(_T("connect to access error!"));
		ExitProcess(0);
	}
	CLog::WriteLog(_T("CHistoryRecord::CHistoryRecord() ok"));

	CUserManager* mgr = CUserManager::GetInstance();
	const CUserInfo* user = mgr->GetCurUserInfo();
	OnCurUserChandedResult(user);
	mgr->RegisterObserver(this, OnCurUesrChanged);
}

CHistoryRecord::~CHistoryRecord()
{
	CLog::WriteLog(_T("CHistoryRecord::~CHistoryRecord()"));
	if (m_pDatabase) {
		if (m_pDatabase->IsOpen()) {
			m_pDatabase->Close();
		}
		delete m_pDatabase;
	}
	::DeleteCriticalSection(&m_csRecord);

	DESTROY_OBSERVER;
	CLog::WriteLog(_T("CHistoryRecord::~CHistoryRecord() OVER."));
}


void CHistoryRecord::InsertRecord(int ademco_id, int zone_value, const wchar_t* record,
								  const time_t& recored_time, RecordLevel level)
{
	CLocalLock lock(&m_csRecord);
	wchar_t wtime[32] = { 0 };
	struct tm tmtm;
	time_t event_time = recored_time;
	localtime_s(&tmtm, &event_time);
	if (tmtm.tm_year == 1900) {
		event_time = time(NULL);
		localtime_s(&tmtm, &event_time);
	}
	wcsftime(wtime, 32, L"%Y-%m-%d %H:%M:%S", &tmtm);
	//CUserManager* mgr = CUserManager::GetInstance();
	CString query = _T("");
	query.Format(_T("insert into [HistoryRecord] ([ademco_id],[zone_value],[user_id],[level],[record],[time]) values(%d,%d,%d,%d,'%s','%s')"),
				 ademco_id, zone_value, m_curUserInfo->get_user_id(), level, record, wtime);
	BOOL ok = m_pDatabase->Execute(query);
	LOG(L"%s\n", query);
	VERIFY(ok);
	if (ok) {
		HistoryRecord record(0, ademco_id, zone_value, m_curUserInfo->get_user_id(),
							 level, record, wtime);
		NotifyObservers((const HistoryRecord*)&record);
	}
}

BOOL CHistoryRecord::IsUpdated()
{
	BOOL ret = m_bUpdated;
	m_bUpdated = FALSE;
	return ret;
}

BOOL CHistoryRecord::GetTopNumRecordsBasedOnID(const int baseID, 
											   const int nums,
											   void* udata, OnHistoryRecordCB cb)
{
	CString query = _T("");
	query.Format(_T("select top %d * from HistoryRecord where id >= %d order by id"),
				 nums, baseID);
	return GetHistoryRecordBySql(query, udata, cb, FALSE);
}

BOOL CHistoryRecord::DeleteAllRecored()
{
	CLocalLock lock(&m_csRecord);
	if (m_pDatabase->Execute(L"delete from HistoryRecord"))	{
		m_bUpdated = TRUE;
		return TRUE;
	}
	return FALSE;
}


BOOL CHistoryRecord::DeleteRecord(int num)
{
	CString query = _T("");
	query.Format(_T("delete from HistoryRecord where id in (select top %d id from record order by id asc)"), num);
	return m_pDatabase->Execute(query);
}


long CHistoryRecord::GetRecordCount()
{
	CLocalLock lock(&m_csRecord);
	const TCHAR* cCount = _T("count_of_record");
	CString query = _T("");
	query.Format(_T("select count(id) as %s from HistoryRecord"), cCount);
	ado::CADORecordset dataGridRecord(m_pDatabase);
	dataGridRecord.Open(m_pDatabase->m_pConnection, query);
	ULONG count = dataGridRecord.GetRecordCount();
	long uCount = 0;
	if (count > 0) {
		dataGridRecord.MoveFirst();
		dataGridRecord.GetFieldValue(cCount, uCount);
	}
	dataGridRecord.Close();
	return uCount;
}


long CHistoryRecord::GetRecordMinimizeID()
{
	CLocalLock lock(&m_csRecord);
	const TCHAR* cMinID = _T("minimize_id");
	CString query = _T("");
	query.Format(_T("select min(id) as %s from HistoryRecord"), cMinID);
	ado::CADORecordset dataGridRecord(m_pDatabase);
	dataGridRecord.Open(m_pDatabase->m_pConnection, query);
	ULONG count = dataGridRecord.GetRecordCount();
	long id = 0;
	if (count == 1) {
		dataGridRecord.MoveFirst();
		dataGridRecord.GetFieldValue(cMinID, id);
	}
	dataGridRecord.Close();
	return id;
}


void CHistoryRecord::TraverseHistoryRecord(void* udata, OnHistoryRecordCB cb)
{
	CString query = _T("");
	query.Format(_T("select * from HistoryRecord order by id"));
	GetHistoryRecordBySql(query, udata, cb);
}


BOOL CHistoryRecord::GetHistoryRecordByDate(const CString& beg, const CString& end,
											void* udata, OnHistoryRecordCB cb)
{
	CString query = _T("");
	query.Format(_T("select * from HistoryRecord where time between #%s# and #%s# order by id"),
				 beg, end);
	return GetHistoryRecordBySql(query, udata, cb, FALSE);
}


BOOL CHistoryRecord::GetHistoryRecordByDateByAlarm(const CString& beg, const CString& end,
												   void* udata, OnHistoryRecordCB cb)
{
	CString query = _T("");
	query.Format(_T("select * from HistoryRecord where level=%d and time between #%s# and #%s# order by id"),
				 RECORD_LEVEL_ALARM, beg, end);
	return GetHistoryRecordBySql(query, udata, cb, FALSE);
}


BOOL CHistoryRecord::GetHistoryRecordBySql(const CString& query, void* udata, 
										   OnHistoryRecordCB cb, BOOL bAsc)
{
	CLocalLock lock(&m_csRecord);
	ado::CADORecordset dataGridRecord(m_pDatabase);
	dataGridRecord.Open(m_pDatabase->m_pConnection, query);
	ULONG count = dataGridRecord.GetRecordCount();
	if (count > 0) {
		bAsc ? dataGridRecord.MoveFirst() : dataGridRecord.MoveLast();
		for (ULONG i = 0; i < count; i++) {
			int id = -1, ademco_id = -1, zone_value = -1, user_id = -1, level = -1;
			CString record_content = _T("");
			CString record_time = _T("");
			dataGridRecord.GetFieldValue(_T("id"), id);
			dataGridRecord.GetFieldValue(_T("ademco_id"), ademco_id);
			dataGridRecord.GetFieldValue(_T("zone_value"), zone_value);
			dataGridRecord.GetFieldValue(_T("user_id"), user_id);
			dataGridRecord.GetFieldValue(_T("record"), record_content);
			dataGridRecord.GetFieldValue(_T("time"), record_time);
			dataGridRecord.GetFieldValue(_T("level"), level);
			HistoryRecord record(id, ademco_id, zone_value,
								 user_id, level, record_content, record_time);
			if (cb) { cb(udata, &record); }
			bAsc ? dataGridRecord.MoveNext() : dataGridRecord.MovePrevious();
		}
	}
	dataGridRecord.Close();
	return count > 0;
}


NAMESPACE_END
