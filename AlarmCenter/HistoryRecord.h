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

namespace core
{

class CUserInfo;
//#ifdef _DEBUG
//static const int MAX_HISTORY_RECORD = 10000;
//static const int WARNING_VAR		= 1000;
//static const int CHECK_POINT		= 100;
//#else
static const int MAX_HISTORY_RECORD = 60000;
//static const int WARNING_VAR		= 60000;
static const int CHECK_POINT		= 100;
//#endif

typedef enum RecordLevel
{
	RECORD_LEVEL_ONOFFLINE,	
	RECORD_LEVEL_USERLOG,	
	RECORD_LEVEL_USEREDIT,	
	RECORD_LEVEL_USERCONTROL,
	RECORD_LEVEL_ALARM,		
	RECORD_LEVEL_CLEARHR,		// CLEAR ALL
}RecordLevel;

class HistoryRecord
{
public:
	HistoryRecord() : id(-1), ademco_id(0), zone_value(0), user_id(0), level(0),
		record(_T("")), record_time(_T(""))
	{}

	HistoryRecord(const HistoryRecord& rhs) 
		: id(rhs.id), ademco_id(rhs.ademco_id), zone_value(rhs.zone_value),
		user_id(rhs.user_id), level(rhs.level), record(rhs.record), record_time(rhs.record_time)
	{}

	HistoryRecord(int IN_id, int In_ademco_id, int In_zone_value, int In_user_id, int IN_level,
				  const CString& IN_record, const CString& IN_record_time)
		: id(IN_id), level(IN_level), ademco_id(In_ademco_id), zone_value(In_zone_value), 
		user_id(In_user_id), record(IN_record), record_time(IN_record_time)
	{}

	int id;
	int ademco_id;
	int zone_value;
	int user_id;
	int level;
	CString record;
	CString record_time;
};

//typedef std::list<HistoryRecord*> CRecordList;
//typedef std::list<HistoryRecord*>::iterator CRecordListIter;
//typedef std::list<HistoryRecord*>::reverse_iterator CRecordListReverseIter;

typedef void(__stdcall *OnHistoryRecordCB)(void* udata, const HistoryRecord* record);

class CHistoryRecord  
{
public:
	void TraverseHistoryRecord(void* udata, OnHistoryRecordCB cb);
	
	BOOL GetTopNumRecordsBasedOnID(const int baseID, const int nums, 
								   void* udata, OnHistoryRecordCB cb);
	BOOL GetTopNumRecordsBasedOnIDByMachine(const int baseID, const int nums,
											int ademco_id, void* udata, 
											OnHistoryRecordCB cb);
	BOOL GetTopNumRecordsBasedOnIDByMachineAndZone(const int baseID, const int nums,
												   int ademco_id, int zone_value, 
												   void* udata,
												   OnHistoryRecordCB cb);
	BOOL GetTopNumRecordByAdemcoID(int nums, int ademco_id, void* udata, OnHistoryRecordCB cb);
	BOOL GetTopNumRecordByAdemcoIDAndZone(int nums, int ademco_id, int zone_value, void* udata, OnHistoryRecordCB cb);
	BOOL DeleteAllRecored(void);
	//BOOL DeleteRecord(int num);
	void InsertRecord(int ademco_id, int zone_value, const wchar_t* record,
					  const time_t& recored_time, RecordLevel level);
	long GetRecordCount() const { return m_nTotalRecord; };
	long GetRecordConntByMachine(int ademco_id);
	long GetRecordConntByMachineAndZone(int ademco_id, int zone_value);
	virtual ~CHistoryRecord();
	void OnCurUserChandedResult(const core::CUserInfo* user);
	long GetRecordMinimizeID();
	long GetRecordMinimizeIDByMachine(int ademco_id);
	long GetRecordMinimizeIDByMachineAndZone(int ademco_id, int zone_value);
	BOOL GetHistoryRecordByDate(const CString& beg, const CString& end, 
									 void* udata, OnHistoryRecordCB cb);
	BOOL GetHistoryRecordByDateByAlarm(const CString& beg, const CString& end,
									   void* udata, OnHistoryRecordCB cb);
	BOOL GetHistoryRecordByDateByUser(const CString& beg, const CString& end,
									  void* udata, OnHistoryRecordCB cb);
	BOOL GetHistoryRecordByDateByMachine(int ademco_id, const CString& beg, const CString& end,
										 void* udata, OnHistoryRecordCB cb);
	//BOOL TryLockRecord() { return m_csLock.TryLock(); }
	//BOOL UnlockRecord() { m_csLock.UnLock(); }
protected:
	BOOL GetHistoryRecordBySql(const CString& query, void* udata, 
							   OnHistoryRecordCB cb, BOOL bAsc = TRUE);
	long GetRecordCountPro();
	
private:
	//CRITICAL_SECTION m_csRecord;
	CLock m_csLock;
	ado::CADODatabase* m_pDatabase;
	const core::CUserInfo* m_curUserInfo;
	int m_nRecordCounter;
	long m_nTotalRecord;
	DECLARE_UNCOPYABLE(CHistoryRecord)
	DECLARE_SINGLETON(CHistoryRecord)
	DECLARE_OBSERVER(OnHistoryRecordCB, HistoryRecord*)
};

NAMESPACE_END

#endif // !defined(AFX_HISTORYRECORD_H__DBF6AB91_29D7_41CB_8EF4_8CF871CA44AB__INCLUDED_)
