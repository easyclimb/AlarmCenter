// HistoryRecord.h: interface for the CHistoryRecord class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HISTORYRECORD_H__DBF6AB91_29D7_41CB_8EF4_8CF871CA44AB__INCLUDED_)
#define AFX_HISTORYRECORD_H__DBF6AB91_29D7_41CB_8EF4_8CF871CA44AB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <list>

namespace ado { class CADODatabase; };

namespace core {

class CUserInfo;
static const int MAX_HISTORY_RECORD = 1000000;
//#define USE_THREAD_TO_BUFF_RECORD


typedef enum RecordLevel
{
	RECORD_LEVEL_ONOFFLINE, // 上下线，踢非法，
	RECORD_LEVEL_USERLOG, // 用户登录与切换
	RECORD_LEVEL_USEREDIT, // 用户编辑(主机编辑、地图编辑、探头编辑、视频编辑)
	RECORD_LEVEL_USERCONTROL, // 用户远程控制主机: 布防、撤防、紧急报警
	RECORD_LEVEL_ALARM, // 报警与异常
}RecordLevel;

typedef struct HistoryRecord
{
	HistoryRecord() : id(-1), ademco_id(0), user_id(0),level(0), 
		record(_T("")), record_time(_T(""))
	{}

	HistoryRecord(int IN_id, int In_ademco_id, int In_user_id,int IN_level, 
			const CString& IN_record, const CString& IN_record_time)
			: id(IN_id), level(IN_level), ademco_id(In_ademco_id), 
			user_id(In_user_id), record(IN_record), record_time(IN_record_time)
	{}

	int id;
	int ademco_id;
	int user_id;
	int level;
	CString record;
	CString record_time;
}HistoryRecord;

typedef std::list<HistoryRecord*> CRecordList;


class CHistoryRecord  
{
public:
	BOOL GetTopNumRecords(int num, CRecordList& list);
	long GetRecordCount();
	BOOL GetTopNumRecordsBasedOnID(const int baseID, const int nums, CRecordList& list);
	BOOL DeleteAllRecored(void);
	BOOL DeleteRecord(int num);
	BOOL IsUpdated();
	void InsertRecord(int ademco_id, const wchar_t* record, 
					  const time_t& recored_time, RecordLevel level);
	
	virtual ~CHistoryRecord();
	void OnCurUserChandedResult(core::CUserInfo* user);
private:
	volatile BOOL m_bUpdated;
	CRITICAL_SECTION m_csRecord;
	ado::CADODatabase* m_pDatabase;
	core::CUserInfo* m_curUserInfo;

#ifdef USE_THREAD_TO_BUFF_RECORD
	static const int WORKER_THREAD_NO = 1;
	HANDLE *m_hThread;
	HANDLE m_hEventShutdown;
	CRecordList m_TempRecordList;
	static DWORD WINAPI ThreadWorker(LPVOID lp);
	BOOL AddRecord(int id, int level, const CString& record, const CString& time);
#endif
	
	
	DECLARE_UNCOPYABLE(CHistoryRecord)
	DECLARE_SINGLETON(CHistoryRecord)
};

NAMESPACE_END

#endif // !defined(AFX_HISTORYRECORD_H__DBF6AB91_29D7_41CB_8EF4_8CF871CA44AB__INCLUDED_)
