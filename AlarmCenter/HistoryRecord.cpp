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

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

namespace core {

CHistoryRecord *CHistoryRecord::m_pInst = NULL;
CLock CHistoryRecord::m_lock;

static const int WORKER_THREAD_NO = 2;

CHistoryRecord::CHistoryRecord()
	: m_bUpdated(TRUE)
	, m_recordLatest(0, 0, _T(""), _T(""))
	, m_hThread(NULL)
	, m_hEventShutdown(INVALID_HANDLE_VALUE)
	, m_pDatabase(NULL)
{
	CLog::WriteLog(_T("CHistoryRecord::CHistoryRecord()"));
	::InitializeCriticalSection(&m_csRecord);
	try {
		m_pDatabase = new ado::CADODatabase();
		//pDataGridRecord = new CADORecordset(m_pDatabase);
		CLog::WriteLog(_T("CHistoryRecord after new, m_pDatabase %x, pDataGridRecord %x"),
			  m_pDatabase);
		LPCTSTR pszMdb = L"HistoryRecord.mdb";
		/*if (CConfig::IsChinese())
		pszMdb = _T("AlarmCenter.mdb");
		else
		pszMdb = _T("AlarmCenter_en.mdb");*/
		TCHAR szMdbPath[1024];
		_tcscpy_s(szMdbPath, GetModuleFilePath());
		_tcscat_s(szMdbPath, _T("\\"));
		_tcscat_s(szMdbPath, pszMdb);
		CLog::WriteLog(_T("CHistoryRecord before pathexists"));
		if (!CFileOper::PathExists(szMdbPath)) {
			MessageBox(NULL, L"File HistoryRecord.mdb missed or broken!", L"Error", MB_OK | MB_ICONERROR);
			ExitProcess(0);
			return;
		}
		TRACE(_T("after pathexists"));
		//连接数据库
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
	CLog::WriteLog(_T("CDBOper::CDBOper() ok"));
}

CHistoryRecord::~CHistoryRecord()
{
	CLog::WriteLog(_T("CHistoryRecord::~CHistoryRecord()"));
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
	if (m_pDatabase) {
		if (m_pDatabase->IsOpen()) {
			m_pDatabase->Close();
		}
		delete m_pDatabase;
	}
	::DeleteCriticalSection(&m_csRecord);
	CLog::WriteLog(_T("CHistoryRecord::~CHistoryRecord() OVER."));
}

void CHistoryRecord::InsertRecord(int level, const CString &record)
{
	CLocalLock lock(&m_csRecord);
	//if(record.Compare(m_recordLatest.record) == 0)
	//{
	//	//CTime oldTime(FormatStringTime2SystemTime(m_recordLatest.record_time));
	//	//CTime nowTime = CTime::GetCurrentTime();
	//	//CTimeSpan ts = nowTime - oldTime;
	//	//if(ts.GetTotalMinutes() <= 2)
	//		return;
	//}
	CTime now = CTime::GetCurrentTime();
	CString fm;
	fm.LoadStringW(IDS_STRING_TIME_FORMAT);
	CString time = now.Format(fm);
	PTEMP_RECORD tempRecord = new TEMP_RECORD(level, record, time);
	m_TempRecordList.push_back(tempRecord);

	if (m_hEventShutdown == INVALID_HANDLE_VALUE) {
		m_hEventShutdown = CreateEvent(NULL, TRUE, FALSE, NULL);
		m_hThread = new HANDLE[WORKER_THREAD_NO];
		for (int i = 0; i < WORKER_THREAD_NO; i++) {
			m_hThread[i] = CreateThread(NULL, 0, ThreadWorker, this, CREATE_SUSPENDED, NULL);
			SetThreadPriority(m_hThread[i], THREAD_PRIORITY_BELOW_NORMAL);
			ResumeThread(m_hThread[i]);
		}
	}
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
	query.Format(_T("select top %d * from record where id >= %d order by id desc"), nums, baseID);
	dataGridRecord.Open(m_pDatabase->m_pConnection, query);
	ULONG count = dataGridRecord.GetRecordCount();
	if (count > 0) {
		dataGridRecord.MoveFirst();
		for (ULONG i = 0; i < count; i++) {
			int id = -1, level = -1;
			CString record = _T("");
			CString record_time = _T("");
			dataGridRecord.GetFieldValue(_T("id"), id);
			dataGridRecord.GetFieldValue(_T("record"), record);
			dataGridRecord.GetFieldValue(_T("time"), record_time);
			dataGridRecord.GetFieldValue(_T("level"), level);
			CRecord *pRecord = new CRecord(id, level, record, record_time);
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
	if (m_pDatabase->Execute(L"delete from record"))
	{
		m_recordLatest.id = -1;
		m_recordLatest.record.Empty();
		m_recordLatest.record_time.Empty();
		m_bUpdated = TRUE;
		return TRUE;
	}
	return FALSE;
}


BOOL CHistoryRecord::DeleteRecord(int num)
{
	CString query = _T("");
	query.Format(_T("delete from record where id in (select top %d id from record order by id asc)"), num);
	return m_pDatabase->Execute(query);
}


long CHistoryRecord::GetRecordCount()
{
	CLocalLock lock(&m_csRecord);
	const TCHAR* cCount = _T("count_of_record");
	CString query = _T("");
	query.Format(_T("select count(id) as %s from record"), cCount);
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
	query.Format(_T("select top %d * from record order by id desc"), nums);
	dataGridRecord.Open(m_pDatabase->m_pConnection, query);
	ULONG count = dataGridRecord.GetRecordCount();
	if (count > 0) {
		dataGridRecord.MoveFirst();
		for (ULONG i = 0; i < count; i++) {
			int id = -1, level = -1;
			CString record = _T("");
			CString record_time = _T("");
			dataGridRecord.GetFieldValue(_T("id"), id);
			dataGridRecord.GetFieldValue(_T("record"), record);
			dataGridRecord.GetFieldValue(_T("time"), record_time);
			dataGridRecord.GetFieldValue(_T("level"), level);
			CRecord *pRecord = new CRecord(id, level, record, record_time);
			list.push_back(pRecord);
			dataGridRecord.MoveNext();
		}
		dataGridRecord.Close();
		return TRUE;
	}
	dataGridRecord.Close();
	return FALSE;
}


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
			int count = hr->GetRecordCount();
			if (count >= MAX_HISTORY_RECORD) {
				if (hr->DeleteRecord(1000))
					hr->m_TempRecordList.push_back(tempRecord);
			} else {
				if (hr->AddRecord(0, tempRecord->_level, tempRecord->_record, tempRecord->_time)) {
					//CRecord *pRecord = new CRecord(id, record, time);
					//m_listRecord.AddTail(pRecord);
					hr->m_recordLatest.id = 0;
					hr->m_recordLatest.record = tempRecord->_record;
					hr->m_recordLatest.record_time = tempRecord->_time;
					bbb = true;
				}
				//CLog::WriteLog(_T("CHistoryRecord::InsertRecord(const CString &record) %s\n"),
				//	tempRecord->_record);
				SAFEDELETEP(tempRecord);
			}
		} else {
			//LeaveCriticalSection(&hr->m_csRecord);
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
	query.Format(_T("insert into [record] ([level],[record],[time]) values('%d','%s','%s')"),
				 level, record, time);
	return m_pDatabase->Execute(query);
}

NAMESPACE_END
