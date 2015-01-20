// HistoryRecord.h: interface for the CHistoryRecord class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HISTORYRECORD_H__DBF6AB91_29D7_41CB_8EF4_8CF871CA44AB__INCLUDED_)
#define AFX_HISTORYRECORD_H__DBF6AB91_29D7_41CB_8EF4_8CF871CA44AB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <list>

class CRecord
{
public:
	CRecord() : id(-1), level(0), record(_T("")), record_time(_T("")) {}
	CRecord(int IN_id, int IN_level, const CString& IN_record, const CString& IN_record_time)
		: id(IN_id), level(IN_level), record(IN_record), record_time(IN_record_time)
	{}
	inline BOOL CompareRecord(const CString& c_record)
	{
		return record.Compare(c_record) == 0;
	}
public:
	int id;
	int level;
	CString record;
	CString record_time;
};
typedef std::list<CRecord*> CRecordList;

class CHistoryRecord  
{
	typedef struct _TempRecord
	{
		int _level;
		CString _record;
		CString _time;
		_TempRecord() : _level(0), _record(_T("")), _time(_T("")) {}
		_TempRecord(int level, const CString& record, const CString& time)
			: _level(level), _record(record), _time(time)
		{}
	}TEMP_RECORD, *PTEMP_RECORD;
	typedef std::list<PTEMP_RECORD> TEMP_RECORD_LIST;
public:
	BOOL GetTopNumRecords(int num, CRecordList& list);
	long GetRecordCount();
	BOOL GetTopNumRecordsBasedOnID(const int baseID, const int nums, CRecordList& list);
	BOOL DeleteAllRecored(void);
	BOOL IsUpdated();
	void InsertRecord(int level, const CString& record);
	
	static CHistoryRecord *GetInstance()
	{
		m_lock.Lock();
		if(m_pInst == NULL)
		{
			static CHistoryRecord hr;
			m_pInst = &hr;
		}
		m_lock.UnLock();
		return m_pInst;
	}
	virtual ~CHistoryRecord();
private:
	CHistoryRecord();
	//CStringList m_listRecord;
	TEMP_RECORD_LIST m_TempRecordList;
	CRecord m_recordLatest;
	volatile BOOL m_bUpdated;
	CRITICAL_SECTION m_csRecord;
	static CHistoryRecord *m_pInst;
	static CLock m_lock;
	HANDLE *m_hThread;
	HANDLE m_hEventShutdown;
protected:
	static DWORD WINAPI ThreadWorker(LPVOID lp);
};

#endif // !defined(AFX_HISTORYRECORD_H__DBF6AB91_29D7_41CB_8EF4_8CF871CA44AB__INCLUDED_)
