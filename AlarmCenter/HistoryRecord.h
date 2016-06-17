// HistoryRecord.h: interface for the history_record_manager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HISTORYRECORD_H__DBF6AB91_29D7_41CB_8EF4_8CF871CA44AB__INCLUDED_)
#define AFX_HISTORYRECORD_H__DBF6AB91_29D7_41CB_8EF4_8CF871CA44AB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>
#include <list>
#include <memory>
#include "core.h"

namespace SQLite { class Database; };

namespace core
{

//#ifdef _DEBUG
//static const int MAX_HISTORY_RECORD = 10000;
//static const int WARNING_VAR		= 1000;
//static const int CHECK_POINT		= 100;
//#else
static const int MAX_HISTORY_RECORD = 10000;
//static const int WARNING_VAR		= 60000;
static const int CHECK_POINT		= 100;
//#endif

typedef enum record_level
{
	RECORD_LEVEL_STATUS,	
	RECORD_LEVEL_USERLOG,	
	RECORD_LEVEL_USEREDIT,	
	RECORD_LEVEL_USERCONTROL,
	RECORD_LEVEL_ALARM,		
	RECORD_LEVEL_EXCEPTION,
	RECORD_LEVEL_VIDEO,
	RECORD_LEVEL_SYSTEM,
	RECORD_LEVEL_CLEARHR,		// CLEAR ALL
}record_level;

static record_level Int2RecordLevel(int level) 
{
	switch (level) {
		case RECORD_LEVEL_STATUS:
			return RECORD_LEVEL_STATUS;
			break;
		case RECORD_LEVEL_USERLOG:
			return RECORD_LEVEL_USERLOG;
			break;
		case RECORD_LEVEL_USEREDIT:
			return RECORD_LEVEL_USEREDIT;
			break;
		case RECORD_LEVEL_USERCONTROL:
			return RECORD_LEVEL_USERCONTROL;
			break;
		case RECORD_LEVEL_ALARM:
			return RECORD_LEVEL_ALARM;
			break;
		case RECORD_LEVEL_EXCEPTION:
			return RECORD_LEVEL_EXCEPTION;
			break;
		case RECORD_LEVEL_VIDEO:
			return RECORD_LEVEL_VIDEO;
			break;
		case RECORD_LEVEL_SYSTEM:
			return RECORD_LEVEL_SYSTEM;
			break;
		default:
			return RECORD_LEVEL_CLEARHR;
			break;
	}
}

class history_record
{
public:
	history_record() : id(-1), ademco_id(0), zone_value(0), user_id(0), level(RECORD_LEVEL_SYSTEM),
		record(_T("")), record_time(_T(""))
	{}

	history_record(const history_record& rhs)
		: id(rhs.id), ademco_id(rhs.ademco_id), zone_value(rhs.zone_value),
		user_id(rhs.user_id), level(rhs.level), record(rhs.record), record_time(rhs.record_time)
	{}

	history_record(int IN_id, int In_ademco_id, int In_zone_value, int In_user_id, record_level IN_level,
				  const CString& IN_record, const CString& IN_record_time)
		: id(IN_id), level(IN_level), ademco_id(In_ademco_id), zone_value(In_zone_value), 
		user_id(In_user_id), record(IN_record), record_time(IN_record_time)
	{}

	int id;
	int ademco_id;
	int zone_value;
	int user_id;
	record_level level;
	CString record;
	CString record_time;
};


class history_record_manager  : public dp::observable<history_record_ptr>, public dp::singleton<history_record_manager>
{
	class CurUserChangedObserver : public dp::observer<user_info_ptr>
	{
	public:
		explicit CurUserChangedObserver(history_record_manager* hr) : _hr(hr) {}
		virtual void on_update(const user_info_ptr& ptr) {
			if (_hr) {
				_hr->OnCurUserChangedResult(ptr);
			}
		}
	private:
		history_record_manager* _hr;
	};

	std::shared_ptr<CurUserChangedObserver> m_cur_user_changed_observer;

public:
	void TraverseHistoryRecord(const observer_ptr& ptr);
	
	BOOL GetTopNumRecordsBasedOnID(const int baseID, const int nums, const observer_ptr& ptr);
	BOOL GetTopNumRecordsBasedOnIDByMachine(const int baseID, const int nums, int ademco_id, const observer_ptr& ptr);
	BOOL GetTopNumRecordsBasedOnIDByMachineAndZone(const int baseID, const int nums, int ademco_id, int zone_value, const observer_ptr& ptr);
	BOOL GetTopNumRecordByAdemcoID(int nums, int ademco_id, const observer_ptr& ptr, BOOL bAsc = TRUE);
	BOOL GetTopNumRecordByAdemcoIDAndZone(int nums, int ademco_id, int zone_value, const observer_ptr& ptr, BOOL bAsc = TRUE);
	BOOL DeleteHalfRecored(void);
	//BOOL DeleteRecord(int num);
	void InsertRecord(int ademco_id, int zone_value, const wchar_t* record,
					  const time_t& recored_time, record_level level);
	long GetRecordCount() const { return m_nTotalRecord; };
	long GetRecordConntByMachine(int ademco_id);
	long GetRecordConntByMachineAndZone(int ademco_id, int zone_value);
	
	virtual ~history_record_manager();
	void OnCurUserChangedResult(const core::user_info_ptr& user);
	long GetRecordMinimizeID();
	long GetRecordMinimizeIDByMachine(int ademco_id);
	long GetRecordMinimizeIDByMachineAndZone(int ademco_id, int zone_value);
	BOOL GetHistoryRecordByDate(const CString& beg, const CString& end, const observer_ptr& ptr);
	BOOL GetHistoryRecordByDateByRecordLevel(const CString& beg, const CString& end, record_level level, const observer_ptr& ptr);
	BOOL GetHistoryRecordByDateByUser(const CString& beg, const CString& end, int user_id, const observer_ptr& ptr);
	BOOL GetHistoryRecordByDateByMachine(int ademco_id, const CString& beg, const CString& end, const observer_ptr& ptr);
	history_record_ptr GetHisrotyRecordById(int id);
	//BOOL TryLockRecord() { return m_csLock.TryLock(); }
	//BOOL UnlockRecord() { m_csLock.UnLock(); }
protected:
	BOOL GetHistoryRecordBySql(const CString& query, const observer_ptr& ptr, BOOL bAsc = TRUE);
	long GetRecordCountPro();
	void InsertRecordPrivate(const history_record_ptr& hr);
private:
	//std::mutex m_csRecord;
	std::mutex m_csLock;
	std::shared_ptr<SQLite::Database> db_ = nullptr;
	core::user_info_ptr m_curUserInfo;
	int m_nRecordCounter;
	long m_nTotalRecord;
	std::list<history_record_ptr> m_bufferedRecordList;
	std::mutex m_lock4BufferedRecordList;
	std::map<int, history_record_ptr> m_recordMap;
	HANDLE m_hThread;
	HANDLE m_hEvent;
	static DWORD WINAPI ThreadWorker(LPVOID lp);
	
	//DECLARE_SINGLETON(history_record_manager)
protected:
	history_record_manager();
};

};


#endif // !defined(AFX_HISTORYRECORD_H__DBF6AB91_29D7_41CB_8EF4_8CF871CA44AB__INCLUDED_)
