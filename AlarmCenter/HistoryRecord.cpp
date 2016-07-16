// HistoryRecord.cpp: implementation of the history_record_manager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AlarmCenter.h"
#include "HistoryRecord.h"
#include "UserInfo.h"
#include "AlarmCenter.h"
#include "sqlitecpp/SQLiteCpp.h"


using namespace SQLite;


namespace core {

//IMPLEMENT_SINGLETON(history_record_manager)

void history_record_manager::OnCurUserChangedResult(const core::user_info_ptr& user)
{
	assert(user);
	if (m_curUserInfo == user)
		return;
	
	CString srecord, suser, slogin, slogout;
	suser = TR(IDS_STRING_USER);
	slogin = TR(IDS_STRING_LOGIN);
	slogout = TR(IDS_STRING_LOGOUT);

	if (m_curUserInfo) {
		srecord.Format(L"%s%s:(ID:%d, %s)", suser, slogout,
					   m_curUserInfo->get_user_id(),
					   m_curUserInfo->get_user_name().c_str());
		InsertRecord(-1, -1, srecord, time(nullptr), RECORD_LEVEL_USERLOG);
	}

	m_curUserInfo = user;
	srecord.Format(L"%s%s:(ID:%d, %s)", suser, slogin,
				   m_curUserInfo->get_user_id(),
				   m_curUserInfo->get_user_name().c_str());
	InsertRecord(-1, -1, srecord, time(nullptr), RECORD_LEVEL_USERLOG);
}

history_record_manager::history_record_manager()
	: m_curUserInfo(nullptr)
	, m_nRecordCounter(0)
	, m_nTotalRecord(0L)
	, m_csLock()
{
	AUTO_LOG_FUNCTION;
	auto path = get_config_path() + "\\history.db3";
	db_ = std::make_shared<Database>(path, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
	assert(db_);
	if (!db_) { return; }

	try {
		// check if db empty
		{
			Statement query(*db_, "select name from sqlite_master where type='table'");
			if (!query.executeStep()) {
				// init tables
				db_->exec("drop table if exists table_history_record");
				db_->exec("create table table_history_record (id integer primary key AUTOINCREMENT, \
ademco_id integer, \
zone_value integer, \
user_id integer, \
level integer, \
record text, \
time text)");
				
				
			} else {
				std::string name = query.getColumn(0);
				JLOGA(name.c_str());
				while (query.executeStep()) {
					name = query.getColumn(0).getText();
					JLOGA(name.c_str());
				}
			}
		}




	} catch (std::exception& e) {
		JLOGA(e.what());
	}


	m_nTotalRecord = GetRecordCountPro();

	auto mgr = user_manager::get_instance();
	auto user = mgr->GetCurUserInfo();
	OnCurUserChangedResult(user);
	m_cur_user_changed_observer = std::make_shared<CurUserChangedObserver>(this);
	mgr->register_observer(m_cur_user_changed_observer);

	thread_ = std::thread(&history_record_manager::ThreadWorker, this);
}

history_record_manager::~history_record_manager()
{
	AUTO_LOG_FUNCTION;

	running_ = false;
	thread_.join();

	for (auto record : m_bufferedRecordList) {
		InsertRecordPrivate(record);
	}
	m_bufferedRecordList.clear();

	//DESTROY_OBSERVER;
}


void history_record_manager::InsertRecord(int ademco_id, int zone_value, const wchar_t* record,
								  const time_t& recored_time, record_level level)
{
	AUTO_LOG_FUNCTION;
	std::lock_guard<std::mutex> lock(m_lock4BufferedRecordList);
	wchar_t wtime[32] = { 0 };
	struct tm tmtm;
	time_t event_time = recored_time;
	localtime_s(&tmtm, &event_time);
	wcsftime(wtime, 32, L"%Y-%m-%d %H:%M:%S", &tmtm);
	//history_record_ptr HistoryRecord = std::make_shared<HistoryRecord>(-1, ademco_id, zone_value, user_manager::get_instance()->GetCurUserID(), level, record, wtime);
	//m_bufferedRecordList.push_back(HistoryRecord);
	m_bufferedRecordList.push_back(std::make_shared<history_record>(-1, ademco_id, zone_value, user_manager::get_instance()->GetCurUserID(), level, record, wtime));
}


void history_record_manager::InsertRecordPrivate(const history_record_ptr& hr)
{
	AUTO_LOG_FUNCTION;
	while (!m_csLock.try_lock()) { JLOG(L"m_csLock.TryLock() failed.\n"); std::this_thread::sleep_for(std::chrono::milliseconds(500)); }
	std::lock_guard<std::mutex> lock(m_csLock, std::adopt_lock);
	JLOG(L"m_csLock.Lock()\n");

	//auto mgr = user_manager::get_instance();
	CString sql = _T("");
	sql.Format(_T("insert into [table_history_record] ([ademco_id],[zone_value],[user_id],[level],[record],[time]) values(%d,%d,%d,%d,'%s','%s')"),
			   hr->ademco_id, hr->zone_value, m_curUserInfo->get_user_id(), hr->level, double_quotes(hr->record).c_str(), hr->record_time);
	JLOG(L"%s\n", sql);
	db_->exec(utf8::w2a((LPCTSTR)sql));
	int id = static_cast<int>(db_->getLastInsertRowid());
	JLOG(L"execute ret %d\n", id);
	if (id >= 0) {
		m_nTotalRecord++;
		m_recordMap[id] = hr;
		notify_observers(hr);
	}

	if (++m_nRecordCounter >= CHECK_POINT) {
		if (/*(WARNING_VAR <= m_nTotalRecord)
			|| */((MAX_HISTORY_RECORD - m_nTotalRecord) <= CHECK_POINT)
			|| (m_nTotalRecord >= MAX_HISTORY_RECORD)) {
			m_nRecordCounter -= CHECK_POINT;
			PostMessageToMainWnd(WM_NEED_TO_EXPORT_HR, m_nTotalRecord, MAX_HISTORY_RECORD);
			m_nRecordCounter = 0;
			
		} else {
			m_nRecordCounter = 0;
		}
	}
	
	JLOG(L"m_csLock.UnLock()\n");
}


void history_record_manager::ThreadWorker()
{
	AUTO_LOG_FUNCTION;
	while (running_) {
		if (m_lock4BufferedRecordList.try_lock()) {
			std::lock_guard<std::mutex> lock(m_lock4BufferedRecordList, std::adopt_lock);
			for (auto record : m_bufferedRecordList) {
				InsertRecordPrivate(record);
			}
			m_bufferedRecordList.clear();
		}
	}
}


BOOL history_record_manager::GetHistoryRecordBySql(const CString& sql, const observer_ptr& ptr, BOOL bAsc)
{
	AUTO_LOG_FUNCTION;
	//std::lock_guard<std::mutex> lock(m_csRecord);
	std::shared_ptr<observer_type> obs(ptr.lock());
	if (!obs) return FALSE;
	while (!m_csLock.try_lock()) { JLOG(L"m_csLock.TryLock() failed.\n"); std::this_thread::sleep_for(std::chrono::milliseconds(500));; }
	std::lock_guard<std::mutex> lock(m_csLock, std::adopt_lock);
	JLOG(L"m_csLock.Lock()\n");
	
	Statement query(*db_, utf8::w2a((LPCTSTR)sql));

	std::list<history_record_ptr> tmp_list;

	while (query.executeStep()) {
		int id = -1, ademco_id = -1, zone_value = -1, user_id = -1, level = -1, index = 0;
		std::string record_content, record_time;

		id = static_cast<int>(query.getColumn(index++));
		ademco_id = query.getColumn(index++);
		zone_value = query.getColumn(index++);
		user_id = query.getColumn(index++);
		level = query.getColumn(index++);
		record_content = query.getColumn(index++).getText();
		record_time = query.getColumn(index++).getText();
		
		history_record_ptr record = std::make_shared<history_record>(id, ademco_id, 
																	 zone_value,
																	 user_id, 
																	 Int2RecordLevel(level),
																	 utf8::a2w(record_content).c_str(), 
																	 utf8::a2w(record_time).c_str());
		m_recordMap[id] = record;
		tmp_list.push_back(record);
		
	}

	if (!bAsc) {
		tmp_list.reverse();
	}

	for (auto hr : tmp_list) {
		obs->on_update(hr);
	}
	
	JLOG(L"m_csLock.UnLock()\n");
	return TRUE;
}


BOOL history_record_manager::GetTopNumRecordsBasedOnID(const int baseID, const int nums, const observer_ptr& ptr)
{
	AUTO_LOG_FUNCTION;
	CString query = _T("");
	query.Format(_T("select * from table_history_record where id >= %d order by id limit %d"), baseID, nums);
	return GetHistoryRecordBySql(query, ptr, FALSE);
}


BOOL history_record_manager::GetTopNumRecordsBasedOnIDByMachine(const int baseID, const int nums, int ademco_id, const observer_ptr& ptr)
{
	AUTO_LOG_FUNCTION;
	CString query = _T("");
	query.Format(_T("select * from table_history_record where ademco_id=%d and id >= %d order by id limit %d"),
				 ademco_id, baseID, nums);
	return GetHistoryRecordBySql(query, ptr, FALSE);
}


BOOL history_record_manager::GetTopNumRecordsBasedOnIDByMachineAndZone(const int baseID, const int nums, int ademco_id, int zone_value, const observer_ptr& ptr)
{
	AUTO_LOG_FUNCTION;
	CString query = _T("");
	query.Format(_T("select * from table_history_record where ademco_id=%d and zone_value=%d and id >= %d order by id limit %d"),
				 ademco_id, zone_value, baseID, nums);
	return GetHistoryRecordBySql(query, ptr, FALSE);
}


BOOL history_record_manager::GetTopNumRecordByAdemcoID(int nums, int ademco_id, const observer_ptr& ptr, BOOL bAsc)
{
	AUTO_LOG_FUNCTION;
	CString query = _T("");
	query.Format(_T("select * from table_history_record where ademco_id=%d order by id limit %d"),
				 ademco_id, nums);
	return GetHistoryRecordBySql(query, ptr, bAsc);
}


BOOL history_record_manager::GetTopNumRecordByAdemcoIDAndZone(int nums, int ademco_id, int zone_value, const observer_ptr& ptr, BOOL bAsc)
{
	AUTO_LOG_FUNCTION;
	CString query = _T("");
	query.Format(_T("select * from table_history_record where ademco_id=%d and zone_value=%d order by id limit %d"),
				 ademco_id, zone_value, nums);
	return GetHistoryRecordBySql(query, ptr, bAsc);
}


BOOL history_record_manager::DeleteHalfRecored()
{
	AUTO_LOG_FUNCTION;
	//EnterCriticalSection(&m_csRecord);
	while (!m_csLock.try_lock()) { JLOG(L"m_csLock.TryLock() failed.\n"); std::this_thread::sleep_for(std::chrono::milliseconds(500));; }
	std::lock_guard<std::mutex> lock(m_csLock, std::adopt_lock);
	JLOG(L"m_csLock.Lock()\n");
	/*if (db_->exec("delete from table_history_record"))	{*/
	CString sql;
	int num = m_nTotalRecord / 2;
	sql.Format(L"delete from table_history_record where id in (select id from table_history_record order by id limit %d)", num);
	if (db_->exec(utf8::w2a((LPCTSTR)sql)))	{
		//db_->exec("update sqlite_sequence set seq=0 where name='table_history_record'");
		m_nRecordCounter = 0;
		m_nTotalRecord -= num;
		m_recordMap.clear();
		JLOG(L"m_csLock.UnLock()\n");
		auto record = std::make_shared<history_record>(-1, -1, -1, m_curUserInfo->get_user_id(), RECORD_LEVEL_CLEARHR, L"", L"");
		notify_observers(record);
		//InsertRecord(-1, -1, s, time(nullptr), RECORD_LEVEL_USERCONTROL);
		return TRUE;
	}
	JLOG(L"m_csLock.UnLock()\n");
	return FALSE;
}


long history_record_manager::GetRecordCountPro()
{
	AUTO_LOG_FUNCTION;
	while (!m_csLock.try_lock()) { JLOG(L"m_csLock.TryLock() failed.\n"); std::this_thread::sleep_for(std::chrono::milliseconds(500));; }
	std::lock_guard<std::mutex> lock(m_csLock, std::adopt_lock); 
	JLOG(L"m_csLock.Lock()\n");
	try {
		Statement query(*db_, "select count(*) from table_history_record");
		if (query.executeStep()) {
			return query.getColumn(0).getInt();
		}
	} catch (std::exception& e) {
		JLOGA(e.what());
	}
	JLOG(L"m_csLock.UnLock()\n");
	return 0;
}


long history_record_manager::GetRecordConntByMachine(int ademco_id)
{
	AUTO_LOG_FUNCTION;
	//std::lock_guard<std::mutex> lock(m_csRecord);
	while (!m_csLock.try_lock()) { JLOG(L"m_csLock.TryLock() failed.\n"); std::this_thread::sleep_for(std::chrono::milliseconds(500)); }
	std::lock_guard<std::mutex> lock(m_csLock, std::adopt_lock); JLOG(L"m_csLock.Lock()\n");
	
	CString sql;
	sql.Format(_T("select count(*) from table_history_record where ademco_id=%d"), ademco_id);
	Statement query(*db_, utf8::w2a((LPCTSTR)sql));
	if (query.executeStep()) {
		return query.getColumn(0).getInt();
	}

	JLOG(L"m_csLock.UnLock()\n");
	return 0;
}


long history_record_manager::GetRecordConntByMachineAndZone(int ademco_id, int zone_value)
{
	AUTO_LOG_FUNCTION;
	//std::lock_guard<std::mutex> lock(m_csRecord);
	while (!m_csLock.try_lock()) { JLOG(L"m_csLock.TryLock() failed.\n"); std::this_thread::sleep_for(std::chrono::milliseconds(500)); }
	std::lock_guard<std::mutex> lock(m_csLock, std::adopt_lock); JLOG(L"m_csLock.Lock()\n");

	CString sql = _T("");
	sql.Format(_T("select count(*) from table_history_record where ademco_id=%d and zone_value=%d"),
			   ademco_id, zone_value);
	Statement query(*db_, utf8::w2a((LPCTSTR)sql));
	if (query.executeStep()) {
		return query.getColumn(0).getInt();
	}

	JLOG(L"m_csLock.UnLock()\n");
	return 0;
}


long history_record_manager::GetRecordMinimizeID()
{
	AUTO_LOG_FUNCTION;
	//std::lock_guard<std::mutex> lock(m_csRecord);
	while (!m_csLock.try_lock()) { JLOG(L"m_csLock.TryLock() failed.\n"); std::this_thread::sleep_for(std::chrono::milliseconds(500)); }
	std::lock_guard<std::mutex> lock(m_csLock, std::adopt_lock);
	JLOG(L"m_csLock.Lock()\n");

	Statement query(*db_, "select min(id) from table_history_record");
	if (query.executeStep()) {
		return query.getColumn(0).getInt();
	}

	JLOG(L"m_csLock.UnLock()\n");
	return 0;
}


long history_record_manager::GetRecordMinimizeIDByMachine(int ademco_id)
{
	AUTO_LOG_FUNCTION;
	while (!m_csLock.try_lock()) { 
		JLOG(L"m_csLock.TryLock() failed.\n"); 
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	std::lock_guard<std::mutex> lock(m_csLock, std::adopt_lock); JLOG(L"m_csLock.Lock()\n");

	CString sql = _T("");
	sql.Format(_T("select min(id) from table_history_record where ademco_id=%d"), ademco_id);
	Statement query(*db_, utf8::w2a((LPCTSTR)sql));
	if (query.executeStep()) {
		return query.getColumn(0).getInt();
	}

	JLOG(L"m_csLock.UnLock()\n");
	return 0;
}


long history_record_manager::GetRecordMinimizeIDByMachineAndZone(int ademco_id, int zone_value)
{
	AUTO_LOG_FUNCTION;
	while (!m_csLock.try_lock()) { JLOG(L"m_csLock.TryLock() failed.\n"); std::this_thread::sleep_for(std::chrono::milliseconds(500)); }
	std::lock_guard<std::mutex> lock(m_csLock, std::adopt_lock);
	JLOG(L"m_csLock.Lock()\n");

	CString sql = _T("");
	sql.Format(_T("select min(id) from table_history_record where ademco_id=%d and zone_value=%d"),
				 ademco_id, zone_value);
	Statement query(*db_, utf8::w2a((LPCTSTR)sql));
	if (query.executeStep()) {
		return query.getColumn(0).getInt();
	}

	JLOG(L"m_csLock.UnLock()\n");
	return 0;
}


void history_record_manager::TraverseHistoryRecord(const observer_ptr& ptr)
{
	AUTO_LOG_FUNCTION;
	CString query = _T("");
	query.Format(_T("select * from table_history_record order by id"));
	GetHistoryRecordBySql(query, ptr);
}


BOOL history_record_manager::GetHistoryRecordByDate(int ademco_id, int zone_value, const CString& beg, const CString& end, const observer_ptr& ptr)
{
	AUTO_LOG_FUNCTION;
	CString query = _T("select * from table_history_record where ");
	if (ademco_id != -1) {
		query.AppendFormat(L"ademco_id=%d and ", ademco_id);
	}

	if (zone_value != -1) {
		query.AppendFormat(L"zone_value=%d and ", zone_value);
	}

	query.AppendFormat(_T("time between \"%s\" and \"%s\" order by id"), beg, end);
	return GetHistoryRecordBySql(query, ptr, FALSE);
}


BOOL history_record_manager::GetHistoryRecordByDateByRecordLevel(int ademco_id, int zone_value, const CString& beg, const CString& end, record_level level, const observer_ptr& ptr)
{
	AUTO_LOG_FUNCTION;
	CString query = _T("select * from table_history_record where ");
	if (ademco_id != -1) {
		query.AppendFormat(L"ademco_id=%d and ", ademco_id);
	}

	if (zone_value != -1) {
		query.AppendFormat(L"zone_value=%d and ", zone_value);
	}

	query.AppendFormat(L"level=%d and ", level);

	query.AppendFormat(_T("time between \"%s\" and \"%s\" order by id"), beg, end);
	return GetHistoryRecordBySql(query, ptr, FALSE);
}


BOOL history_record_manager::GetHistoryRecordByDateByUser(int ademco_id, int zone_value, const CString& beg, const CString& end, int user_id, const observer_ptr& ptr)
{
	AUTO_LOG_FUNCTION;
	CString query = _T("select * from table_history_record where ");
	if (ademco_id != -1) {
		query.AppendFormat(L"ademco_id=%d and ", ademco_id);
	}

	if (zone_value != -1) {
		query.AppendFormat(L"zone_value=%d and ", zone_value);
	}

	query.AppendFormat(L"user_id=%d and ", user_id);

	query.AppendFormat(_T("time between \"%s\" and \"%s\" order by id"), beg, end);
	return GetHistoryRecordBySql(query, ptr, FALSE);
}


//BOOL history_record_manager::GetHistoryRecordByDateByMachine(int ademco_id, 
//													 const CString& beg,
//													 const CString& end,
//													 const observer_ptr& ptr)
//{
//	AUTO_LOG_FUNCTION;
//	CString query = _T("");
//	query.Format(_T("select * from table_history_record where ademco_id=%d and time between \"%s\" and \"%s\" order by id"),
//				 ademco_id, beg, end);
//	return GetHistoryRecordBySql(query, ptr, FALSE);
//}


history_record_ptr history_record_manager::GetHisrotyRecordById(int id)
{
	AUTO_LOG_FUNCTION;
	while (!m_csLock.try_lock()) { JLOG(L"m_csLock.TryLock() failed.\n"); std::this_thread::sleep_for(std::chrono::milliseconds(500)); }
	std::lock_guard<std::mutex> lock(m_csLock, std::adopt_lock);
	history_record_ptr hr;
	JLOG(L"m_csLock.Lock()\n");
	auto iter = m_recordMap.find(id);
	if (iter != m_recordMap.end()) {
		hr = iter->second;
	} else {
		CString sql;
		sql.Format(L"select * from table_history_record where id=%d", id);
		Statement query(*db_, utf8::w2a((LPCTSTR)sql));
		while (query.executeStep())	{
			int ademco_id = -1, zone_value = -1, user_id = -1, level = -1, index = 1;
			std::string record_content, record_time;

			ademco_id = query.getColumn(index++);
			zone_value = query.getColumn(index++);
			user_id = query.getColumn(index++);
			level = query.getColumn(index++);
			record_content = query.getColumn(index++).getText();
			record_time = query.getColumn(index++).getText();
			

			hr = std::make_shared<history_record>(id, ademco_id,
												  zone_value,
												  user_id,
												  Int2RecordLevel(level),
												  utf8::a2w(record_content).c_str(),
												  utf8::a2w(record_time).c_str());
			m_recordMap[id] = hr;
		}
	}

	JLOG(L"m_csLock.UnLock()\n");
	return hr;
}

};

