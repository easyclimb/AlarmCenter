// HistoryRecord.cpp: implementation of the CHistoryRecord class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AlarmCenter.h"
#include "HistoryRecord.h"
#include "DBOper.h"
#include <odbcinst.h>
#include <afxdb.h>
#include <comdef.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CHistoryRecord *CHistoryRecord::m_pInst = NULL;
CLock CHistoryRecord::m_lock;

static const int WORKER_THREAD_NO = 2;

CHistoryRecord::CHistoryRecord()
	: m_bUpdated(TRUE)
	, m_recordLatest(0, 0, _T(""), _T(""))
	, m_hThread(NULL)
	, m_hEventShutdown(INVALID_HANDLE_VALUE)
{
	CLog::WriteLog(_T("CHistoryRecord::CHistoryRecord()"));
	::InitializeCriticalSection(&m_csRecord);
	
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
	if (!m_TempRecordList.IsEmpty()) {
		POSITION pos = m_TempRecordList.GetHeadPosition();
		while (pos) {
			PTEMP_RECORD tempRecord = m_TempRecordList.GetNext(pos);
			SAFEDELETEP(tempRecord);
		}
		m_TempRecordList.RemoveAll();
	}
	::DeleteCriticalSection(&m_csRecord);
	CLog::WriteLog(_T("CHistoryRecord::~CHistoryRecord() OVER."));
}

void CHistoryRecord::InsertRecord(int level, const CString &record)
{
	CLocalLock lock(&m_csRecord);
	if(record.Compare(m_recordLatest.record) == 0)
	{
		//CTime oldTime(FormatStringTime2SystemTime(m_recordLatest.record_time));
		//CTime nowTime = CTime::GetCurrentTime();
		//CTimeSpan ts = nowTime - oldTime;
		//if(ts.GetTotalMinutes() <= 2)
			return;
	}
	CTime now = CTime::GetCurrentTime();
	CString time = now.Format(GetStringTable(IDS_STRING_TIME_FORMAT));
	PTEMP_RECORD tempRecord = new TEMP_RECORD(level, record, time);
	m_TempRecordList.AddTail(tempRecord);

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
	/*POSITION pos = m_listRecord.GetTailPosition();
	while(pos && num--)
	{
		list.AddTail(m_listRecord.GetPrev(pos));
	}
	m_bUpdated = FALSE;
	
	return TRUE;
	*/
	CLocalLock lock(&m_csRecord);
	CDBOper *db = CDBOper::GetInstance();
	return db->GetRecords(baseID, nums, list);
}

BOOL CHistoryRecord::DeleteAllRecored()
{
	CLocalLock lock(&m_csRecord);
	CDBOper *db = CDBOper::GetInstance();
	if(db->DeleteAllHistoryRecord())
	{
		m_recordLatest.id = -1;
		m_recordLatest.record.Empty();
		m_recordLatest.record_time.Empty();
		m_bUpdated = TRUE;
		return TRUE;
	}
	return FALSE;
}

long CHistoryRecord::GetRecordCount()
{
	CLocalLock lock(&m_csRecord);
	CDBOper *db = CDBOper::GetInstance();
	return db->GetRecordsCount();
}

BOOL CHistoryRecord::GetTopNumRecords(int nums, CRecordList &list)
{
	CLocalLock lock(&m_csRecord);
	CDBOper *db = CDBOper::GetInstance();
	return db->GetRecords(nums, list);
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
		if (hr->m_TempRecordList.GetCount() > 0) {
			PTEMP_RECORD tempRecord = hr->m_TempRecordList.RemoveHead();
			//LeaveCriticalSection(&hr->m_csRecord);
			CDBOper *db = CDBOper::GetInstance();
			int count = db->GetRecordsCount();
			if (count >= MAX_HISTORY_RECORD) {
				if (db->DeleteRecord(1000))
					hr->m_TempRecordList.AddTail(tempRecord);
			} else {
				if (db->AddRecord(0, tempRecord->_level, tempRecord->_record, tempRecord->_time)) {
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
