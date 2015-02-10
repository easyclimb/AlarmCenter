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

static void __stdcall OnCurUesrChanged(void* udata, CUserInfo* user) 
{
	CHistoryRecord* hr = reinterpret_cast<CHistoryRecord*>(udata); assert(hr);
	hr->OnCurUserChandedResult(user);
}

void CHistoryRecord::OnCurUserChandedResult(core::CUserInfo* user)
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
		InsertRecord(-1, srecord, time(NULL), RECORD_LEVEL_USERLOG);
	}

	m_curUserInfo = user;
	srecord.Format(L"%s%s:(ID:%d, %s)", suser, slogin,
				   m_curUserInfo->get_user_id(),
				   m_curUserInfo->get_user_name());
	InsertRecord(-1, srecord, time(NULL), RECORD_LEVEL_USERLOG);
}

CHistoryRecord::CHistoryRecord()
	: m_bUpdated(TRUE)
#ifdef USE_THREAD_TO_BUFF_RECORD
	, m_hThread(NULL)
	, m_hEventShutdown(INVALID_HANDLE_VALUE)
#endif
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

#ifdef USE_THREAD_TO_BUFF_RECORD
	if (m_hEventShutdown == INVALID_HANDLE_VALUE) {
		m_hEventShutdown = CreateEvent(NULL, TRUE, FALSE, NULL);
		m_hThread = new HANDLE[WORKER_THREAD_NO];
		for (int i = 0; i < WORKER_THREAD_NO; i++) {
			m_hThread[i] = CreateThread(NULL, 0, ThreadWorker, this, CREATE_SUSPENDED, NULL);
			SetThreadPriority(m_hThread[i], THREAD_PRIORITY_BELOW_NORMAL);
			ResumeThread(m_hThread[i]);
		}
	}
#endif

	CUserManager* mgr = CUserManager::GetInstance();
	CUserInfo* user = mgr->GetCurUserInfo();
	OnCurUserChandedResult(user);
	mgr->RegisterObserver(this, OnCurUesrChanged);
}

CHistoryRecord::~CHistoryRecord()
{
	CLog::WriteLog(_T("CHistoryRecord::~CHistoryRecord()"));
#ifdef USE_THREAD_TO_BUFF_RECORD
	if (m_hEventShutdown != INVALID_HANDLE_VALUE) {
		SetEvent(m_hEventShutdown);
		WaitForMultipleObjects(WORKER_THREAD_NO, m_hThread, TRUE, INFINITE);
		for (int i = 0; i < WORKER_THREAD_NO; i++) {
			CLOSEHANDLE(m_hThread[i]);
		}
		SAFEDELETEARR(m_hThread);
		CLOSEHANDLE(m_hEventShutdown);
	}
	if (m_TempRecordList.size() > 0) {
		std::list<PTEMP_RECORD>::iterator iter = m_TempRecordList.begin();
		while (iter != m_TempRecordList.end()) {
			PTEMP_RECORD tempRecord = *iter++;
			SAFEDELETEP(tempRecord);
		}
		m_TempRecordList.clear();
	}
#endif
	if (m_pDatabase) {
		if (m_pDatabase->IsOpen()) {
			m_pDatabase->Close();
		}
		delete m_pDatabase;
	}
	::DeleteCriticalSection(&m_csRecord);
	CLog::WriteLog(_T("CHistoryRecord::~CHistoryRecord() OVER."));
}

void CHistoryRecord::InsertRecord(int ademco_id, const wchar_t* record,
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
#ifdef USE_THREAD_TO_BUFF_RECORD
	PTEMP_RECORD tempRecord = new TEMP_RECORD(level, record, wtime);
	m_TempRecordList.push_back(tempRecord);
#else
	//CUserManager* mgr = CUserManager::GetInstance();
	CString query = _T("");
	query.Format(_T("insert into [HistoryRecord] ([ademco_id],[user_id],[level],[record],[time]) values(%d,%d,%d,'%s','%s')"),
				 ademco_id, m_curUserInfo->get_user_id(), level, record, wtime);
	BOOL ok = m_pDatabase->Execute(query);
	VERIFY(ok);
	if (ok) {
		HistoryRecord record(0, ademco_id, m_curUserInfo->get_user_id(), 
							 level, record, wtime);
		NotifyObservers(&record);
	}
#endif
}

BOOL CHistoryRecord::IsUpdated()
{
	BOOL ret = m_bUpdated;
	m_bUpdated = FALSE;
	return ret;
}

BOOL CHistoryRecord::GetTopNumRecordsBasedOnID(const int baseID, const int nums, CRecordList &list)
{
	CLocalLock lock(&m_csRecord);
	ado::CADORecordset dataGridRecord(m_pDatabase);
	CString query = _T("");
	query.Format(_T("select top %d * from HistoryRecord where id >= %d order by id desc"), nums, baseID);
	dataGridRecord.Open(m_pDatabase->m_pConnection, query);
	ULONG count = dataGridRecord.GetRecordCount();
	if (count > 0) {
		dataGridRecord.MoveFirst();
		for (ULONG i = 0; i < count; i++) {
			int id = -1, ademco_id = -1, user_id = -1, level = -1;
			CString record = _T("");
			CString record_time = _T("");
			dataGridRecord.GetFieldValue(_T("id"), id);
			dataGridRecord.GetFieldValue(_T("ademco_id"), ademco_id);
			dataGridRecord.GetFieldValue(_T("user_id"), user_id);
			dataGridRecord.GetFieldValue(_T("record"), record);
			dataGridRecord.GetFieldValue(_T("time"), record_time);
			dataGridRecord.GetFieldValue(_T("level"), level);
			HistoryRecord *pRecord = new HistoryRecord(id, ademco_id, user_id, level, record, record_time);
			list.push_back(pRecord);
			dataGridRecord.MoveNext();
		}
		dataGridRecord.Close();
		return TRUE;
	}
	dataGridRecord.Close();
	return FALSE;
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

BOOL CHistoryRecord::GetTopNumRecords(int nums, CRecordList &list)
{
	CLocalLock lock(&m_csRecord);
	ado::CADORecordset dataGridRecord(m_pDatabase);
	CString query = _T("");
	query.Format(_T("select top %d * from HistoryRecord order by id desc"), nums);
	dataGridRecord.Open(m_pDatabase->m_pConnection, query);
	ULONG count = dataGridRecord.GetRecordCount();
	if (count > 0) {
		dataGridRecord.MoveFirst();
		for (ULONG i = 0; i < count; i++) {
			int id = -1, ademco_id = -1, user_id = -1, level = -1;
			CString record = _T("");
			CString record_time = _T("");
			dataGridRecord.GetFieldValue(_T("id"), id);
			dataGridRecord.GetFieldValue(_T("ademco_id"), ademco_id);
			dataGridRecord.GetFieldValue(_T("user_id"), user_id);
			dataGridRecord.GetFieldValue(_T("record"), record);
			dataGridRecord.GetFieldValue(_T("time"), record_time);
			dataGridRecord.GetFieldValue(_T("level"), level);
			HistoryRecord *pRecord = new HistoryRecord(id, ademco_id, user_id, level, record, record_time);
			list.push_back(pRecord);
			dataGridRecord.MoveNext();
		}
		dataGridRecord.Close();
		return TRUE;
	}
	dataGridRecord.Close();
	return FALSE;
}

#ifdef USE_THREAD_TO_BUFF_RECORD
DWORD WINAPI CHistoryRecord::ThreadWorker(LPVOID lp)
{
	CHistoryRecord *hr = reinterpret_cast<CHistoryRecord*>(lp);
	bool bbb = false;
	
	for (;;) {
		if (WAIT_OBJECT_0 == WaitForSingleObject(hr->m_hEventShutdown, 1000))
			break;
		CLocalLock lock(&hr->m_csRecord);
		//EnterCriticalSection(&hr->m_csRecord);
		if (hr->m_TempRecordList.size() > 0) {
			PTEMP_RECORD tempRecord = hr->m_TempRecordList.front();
			hr->m_TempRecordList.pop_front();
			//LeaveCriticalSection(&hr->m_csRecord);
			//int count = hr->GetRecordCount();
			long count = 0;
			{
				const TCHAR* cCount = _T("count_of_record");
				CString query = _T("");
				query.Format(_T("select count(id) as %s from HistoryRecord"), cCount);
				ado::CADORecordset dataGridRecord(hr->m_pDatabase);
				dataGridRecord.Open(hr->m_pDatabase->m_pConnection, query);
				ULONG res = dataGridRecord.GetRecordCount();
				long uCount = 0;
				if (res > 0) {
					dataGridRecord.MoveFirst();
					dataGridRecord.GetFieldValue(cCount, uCount);
					count = uCount;
				}
				dataGridRecord.Close();
			}
			if (count >= MAX_HISTORY_RECORD) {
				if (hr->DeleteRecord(1000))
					hr->m_TempRecordList.push_back(tempRecord);
			} else {
				if (hr->AddRecord(0, tempRecord->_level, tempRecord->_record, tempRecord->_time)) {
					bbb = true;
				}
				SAFEDELETEP(tempRecord);
			}
		} else {
			if (bbb) {
				bbb = false;
				hr->m_bUpdated = TRUE;
			}
		}
	}
	return 0;
}


BOOL CHistoryRecord::AddRecord(int /*id*/, int level, const CString& record, const CString& time)
{
	CString query = _T("");
	query.Format(_T("insert into [HistoryRecord] ([level],[record],[time]) values('%d','%s','%s')"),
				 level, record, time);
	return m_pDatabase->Execute(query);
}
#endif

NAMESPACE_END
