// HistoryRecord.cpp: implementation of the history_record_manager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HistoryRecord.h"
#include "DbOper.h"
#include "UserInfo.h"
#include "AppResource.h"


namespace core {

IMPLEMENT_SINGLETON(history_record_manager)

void history_record_manager::OnCurUserChangedResult(const core::user_info_ptr& user)
{
	assert(user);
	if (m_curUserInfo == user)
		return;
	
	CString srecord, suser, slogin, slogout;
	suser = GetStringFromAppResource(IDS_STRING_USER);
	slogin = GetStringFromAppResource(IDS_STRING_LOGIN);
	slogout = GetStringFromAppResource(IDS_STRING_LOGOUT);

	if (m_curUserInfo) {
		srecord.Format(L"%s%s:(ID:%d, %s)", suser, slogout,
					   m_curUserInfo->get_user_id(),
					   m_curUserInfo->get_user_name());
		InsertRecord(-1, -1, srecord, time(nullptr), RECORD_LEVEL_USERLOG);
	}

	m_curUserInfo = user;
	srecord.Format(L"%s%s:(ID:%d, %s)", suser, slogin,
				   m_curUserInfo->get_user_id(),
				   m_curUserInfo->get_user_name());
	InsertRecord(-1, -1, srecord, time(nullptr), RECORD_LEVEL_USERLOG);
}

history_record_manager::history_record_manager()
	: m_db(nullptr)
	, m_curUserInfo(nullptr)
	, m_nRecordCounter(0)
	, m_nTotalRecord(0L)
	, m_csLock()
{
	AUTO_LOG_FUNCTION;
	m_db = std::make_shared<ado::CDbOper>();
	m_db->Open(L"HistoryRecord.mdb");
	m_nTotalRecord = GetRecordCountPro();

	user_manager* mgr = user_manager::GetInstance();
	auto user = mgr->GetCurUserInfo();
	OnCurUserChangedResult(user);
	m_cur_user_changed_observer = std::make_shared<CurUserChangedObserver>(this);
	mgr->register_observer(m_cur_user_changed_observer);
	m_hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	m_hThread = CreateThread(nullptr, 0, ThreadWorker, this, 0, nullptr);
}

history_record_manager::~history_record_manager()
{
	AUTO_LOG_FUNCTION;

	SetEvent(m_hEvent);
	WaitForSingleObject(m_hThread, INFINITE);

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
	//history_record_ptr HistoryRecord = std::make_shared<HistoryRecord>(-1, ademco_id, zone_value, user_manager::GetInstance()->GetCurUserID(), level, record, wtime);
	//m_bufferedRecordList.push_back(HistoryRecord);
	m_bufferedRecordList.push_back(std::make_shared<history_record>(-1, ademco_id, zone_value, user_manager::GetInstance()->GetCurUserID(), level, record, wtime));
}


void history_record_manager::InsertRecordPrivate(const history_record_ptr& hr)
{
	AUTO_LOG_FUNCTION;
	while (!m_csLock.try_lock()) { JLOG(L"m_csLock.TryLock() failed.\n"); std::this_thread::sleep_for(std::chrono::milliseconds(500)); }
	std::lock_guard<std::mutex> lock(m_csLock, std::adopt_lock);
	JLOG(L"m_csLock.Lock()\n");

	//user_manager* mgr = user_manager::GetInstance();
	CString query = _T("");
	query.Format(_T("insert into [HistoryRecord] ([ademco_id],[zone_value],[user_id],[level],[record],[time]) values(%d,%d,%d,%d,'%s','%s')"),
				 hr->ademco_id, hr->zone_value, m_curUserInfo->get_user_id(), hr->level, hr->record, hr->record_time);
	JLOG(L"%s\n", query);
	int id = m_db->AddAutoIndexTableReturnID(query);
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
			auto app = AfxGetApp();
			if (app && app->m_pMainWnd) {
				app->m_pMainWnd->PostMessageW(WM_NEED_TO_EXPORT_HR, m_nTotalRecord, MAX_HISTORY_RECORD);
				m_nRecordCounter = 0;
			}
		} else {
			m_nRecordCounter = 0;
		}
	}
	
	JLOG(L"m_csLock.UnLock()\n");
}


DWORD WINAPI history_record_manager::ThreadWorker(LPVOID lp)
{
	AUTO_LOG_FUNCTION;
	history_record_manager* hr = reinterpret_cast<history_record_manager*>(lp);
	while (true) {
		if (WAIT_OBJECT_0 == WaitForSingleObject(hr->m_hEvent, 1000)) break;
		if (hr->m_lock4BufferedRecordList.try_lock()) {
			std::lock_guard<std::mutex> lock(hr->m_lock4BufferedRecordList, std::adopt_lock);
			for (auto record : hr->m_bufferedRecordList) {
				hr->InsertRecordPrivate(record);
			}
			hr->m_bufferedRecordList.clear();
		}

	}
	return 0;
}


BOOL history_record_manager::GetHistoryRecordBySql(const CString& query, const observer_ptr& ptr, BOOL bAsc)
{
	AUTO_LOG_FUNCTION;
	//std::lock_guard<std::mutex> lock(m_csRecord);
	std::shared_ptr<observer_type> obs(ptr.lock());
	if (!obs) return FALSE;
	while (!m_csLock.try_lock()) { JLOG(L"m_csLock.TryLock() failed.\n"); std::this_thread::sleep_for(std::chrono::milliseconds(500));; }
	std::lock_guard<std::mutex> lock(m_csLock, std::adopt_lock);
	JLOG(L"m_csLock.Lock()\n");
	ado::CADORecordset dataGridRecord(m_db->GetDatabase());
	dataGridRecord.Open(m_db->GetDatabase()->m_pConnection, query);
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
			history_record_ptr record = std::make_shared<history_record>(id, ademco_id, zone_value,
																		 user_id, Int2RecordLevel(level), 
																		 record_content, record_time);
			m_recordMap[id] = record;
			//if (cb) { cb(udata, record); }
			obs->on_update(record);
			bAsc ? dataGridRecord.MoveNext() : dataGridRecord.MovePrevious();
		}
	}
	dataGridRecord.Close();
	
	JLOG(L"m_csLock.UnLock()\n");
	return count > 0;
}


BOOL history_record_manager::GetTopNumRecordsBasedOnID(const int baseID, const int nums, const observer_ptr& ptr)
{
	AUTO_LOG_FUNCTION;
	CString query = _T("");
	query.Format(_T("select top %d * from HistoryRecord where id >= %d order by id"), nums, baseID);
	return GetHistoryRecordBySql(query, ptr, FALSE);
}


BOOL history_record_manager::GetTopNumRecordsBasedOnIDByMachine(const int baseID, const int nums, int ademco_id, const observer_ptr& ptr)
{
	AUTO_LOG_FUNCTION;
	CString query = _T("");
	query.Format(_T("select top %d * from HistoryRecord where ademco_id=%d and id >= %d order by id"),
				 nums, ademco_id, baseID);
	return GetHistoryRecordBySql(query, ptr, FALSE);
}


BOOL history_record_manager::GetTopNumRecordsBasedOnIDByMachineAndZone(const int baseID, const int nums, int ademco_id, int zone_value, const observer_ptr& ptr)
{
	AUTO_LOG_FUNCTION;
	CString query = _T("");
	query.Format(_T("select top %d * from HistoryRecord where ademco_id=%d and zone_value=%d and id >= %d order by id"),
				 nums, ademco_id, zone_value, baseID);
	return GetHistoryRecordBySql(query, ptr, FALSE);
}


BOOL history_record_manager::GetTopNumRecordByAdemcoID(int nums, int ademco_id, const observer_ptr& ptr, BOOL bAsc)
{
	AUTO_LOG_FUNCTION;
	CString query = _T("");
	query.Format(_T("select top %d * from HistoryRecord where ademco_id=%d order by id"),
				 nums, ademco_id);
	return GetHistoryRecordBySql(query, ptr, bAsc);
}


BOOL history_record_manager::GetTopNumRecordByAdemcoIDAndZone(int nums, int ademco_id, int zone_value, const observer_ptr& ptr, BOOL bAsc)
{
	AUTO_LOG_FUNCTION;
	CString query = _T("");
	query.Format(_T("select top %d * from HistoryRecord where ademco_id=%d and zone_value=%d order by id"),
				 nums, ademco_id, zone_value);
	return GetHistoryRecordBySql(query, ptr, bAsc);
}


BOOL history_record_manager::DeleteAllRecored()
{
	AUTO_LOG_FUNCTION;
	//EnterCriticalSection(&m_csRecord);
	while (!m_csLock.try_lock()) { JLOG(L"m_csLock.TryLock() failed.\n"); std::this_thread::sleep_for(std::chrono::milliseconds(500));; }
	std::lock_guard<std::mutex> lock(m_csLock, std::adopt_lock);
	JLOG(L"m_csLock.Lock()\n");
	if (m_db->Execute(L"delete from HistoryRecord"))	{
		m_db->Execute(L"alter table HistoryRecord alter column id counter(1,1)");
		m_nRecordCounter = 0;
		m_nTotalRecord = 0;
		//CString s, fm;
		//fm = GetStringFromAppResource(IDS_STRING_FM_USER_EXPORT_HR);
		//s.Format(fm, m_curUserInfo->get_user_id(), m_curUserInfo->get_user_name());
		//AfxMessageBox(s, MB_ICONINFORMATION);
		//LeaveCriticalSection(&m_csRecord);
		m_recordMap.clear();
		JLOG(L"m_csLock.UnLock()\n");
		auto record = std::make_shared<history_record>(-1, -1, -1, m_curUserInfo->get_user_id(), RECORD_LEVEL_CLEARHR, L"", L"");
		notify_observers(record);
		//InsertRecord(-1, -1, s, time(nullptr), RECORD_LEVEL_USERCONTROL);
		return TRUE;
	}
	//LeaveCriticalSection(&m_csRecord);
	JLOG(L"m_csLock.UnLock()\n");
	return FALSE;
}

//
//BOOL history_record_manager::DeleteRecord(int num)
//{
//	CString query = _T("");
//	query.Format(_T("delete from HistoryRecord where id in (select top %d id from record order by id asc)"), num);
//	return m_db->GetDatabase()->Execute(query);
//}


long history_record_manager::GetRecordCountPro()
{
	AUTO_LOG_FUNCTION;
	//std::lock_guard<std::mutex> lock(m_csRecord);
	while (!m_csLock.try_lock()) { JLOG(L"m_csLock.TryLock() failed.\n"); std::this_thread::sleep_for(std::chrono::milliseconds(500));; }
	std::lock_guard<std::mutex> lock(m_csLock, std::adopt_lock); 
	JLOG(L"m_csLock.Lock()\n");
	const TCHAR* cCount = _T("count_of_record");
	CString query = _T("");
	query.Format(_T("select count(id) as %s from HistoryRecord"), cCount);
	ado::CADORecordset dataGridRecord(m_db->GetDatabase());
	dataGridRecord.Open(m_db->GetDatabase()->m_pConnection, query);
	ULONG count = dataGridRecord.GetRecordCount();
	long uCount = 0;
	if (count == 1) {
		dataGridRecord.MoveFirst();
		dataGridRecord.GetFieldValue(cCount, uCount);
	}
	dataGridRecord.Close();
	
	JLOG(L"m_csLock.UnLock()\n");
	return uCount;
}


long history_record_manager::GetRecordConntByMachine(int ademco_id)
{
	AUTO_LOG_FUNCTION;
	//std::lock_guard<std::mutex> lock(m_csRecord);
	while (!m_csLock.try_lock()) { JLOG(L"m_csLock.TryLock() failed.\n"); std::this_thread::sleep_for(std::chrono::milliseconds(500)); }
	std::lock_guard<std::mutex> lock(m_csLock, std::adopt_lock); JLOG(L"m_csLock.Lock()\n");
	const TCHAR* cCount = _T("count_of_record");
	CString query = _T("");
	query.Format(_T("select count(id) as %s from HistoryRecord where ademco_id=%d"), 
				 cCount, ademco_id);
	ado::CADORecordset dataGridRecord(m_db->GetDatabase());
	dataGridRecord.Open(m_db->GetDatabase()->m_pConnection, query);
	ULONG count = dataGridRecord.GetRecordCount();
	long uCount = 0;
	if (count == 1) {
		dataGridRecord.MoveFirst();
		dataGridRecord.GetFieldValue(cCount, uCount);
	}
	dataGridRecord.Close();
	JLOG(L"m_csLock.UnLock()\n");
	return uCount;
}


long history_record_manager::GetRecordConntByMachineAndZone(int ademco_id, int zone_value)
{
	AUTO_LOG_FUNCTION;
	//std::lock_guard<std::mutex> lock(m_csRecord);
	while (!m_csLock.try_lock()) { JLOG(L"m_csLock.TryLock() failed.\n"); std::this_thread::sleep_for(std::chrono::milliseconds(500)); }
	std::lock_guard<std::mutex> lock(m_csLock, std::adopt_lock); JLOG(L"m_csLock.Lock()\n");
	const TCHAR* cCount = _T("count_of_record");
	CString query = _T("");
	query.Format(_T("select count(id) as %s from HistoryRecord where ademco_id=%d and zone_value=%d"),
				 cCount, ademco_id, zone_value);
	ado::CADORecordset dataGridRecord(m_db->GetDatabase());
	dataGridRecord.Open(m_db->GetDatabase()->m_pConnection, query);
	ULONG count = dataGridRecord.GetRecordCount();
	long uCount = 0;
	if (count == 1) {
		dataGridRecord.MoveFirst();
		dataGridRecord.GetFieldValue(cCount, uCount);
	}
	dataGridRecord.Close();
	JLOG(L"m_csLock.UnLock()\n");
	return uCount;
}


long history_record_manager::GetRecordMinimizeID()
{
	AUTO_LOG_FUNCTION;
	//std::lock_guard<std::mutex> lock(m_csRecord);
	while (!m_csLock.try_lock()) { JLOG(L"m_csLock.TryLock() failed.\n"); std::this_thread::sleep_for(std::chrono::milliseconds(500)); }
	std::lock_guard<std::mutex> lock(m_csLock, std::adopt_lock);
	JLOG(L"m_csLock.Lock()\n");
	const TCHAR* cMinID = _T("minimize_id");
	CString query = _T("");
	query.Format(_T("select min(id) as %s from HistoryRecord"), cMinID);
	ado::CADORecordset dataGridRecord(m_db->GetDatabase());
	dataGridRecord.Open(m_db->GetDatabase()->m_pConnection, query);
	ULONG count = dataGridRecord.GetRecordCount();
	long id = 0;
	if (count == 1) {
		dataGridRecord.MoveFirst();
		dataGridRecord.GetFieldValue(cMinID, id);
	}
	dataGridRecord.Close();
	JLOG(L"m_csLock.UnLock()\n");
	return id;
}


long history_record_manager::GetRecordMinimizeIDByMachine(int ademco_id)
{
	AUTO_LOG_FUNCTION;
	//std::lock_guard<std::mutex> lock(m_csRecord);
	while (!m_csLock.try_lock()) { 
		JLOG(L"m_csLock.TryLock() failed.\n"); 
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	std::lock_guard<std::mutex> lock(m_csLock, std::adopt_lock); JLOG(L"m_csLock.Lock()\n");
	const TCHAR* cMinID = _T("minimize_id");
	CString query = _T("");
	query.Format(_T("select min(id) as %s from HistoryRecord where ademco_id=%d"), 
				 cMinID, ademco_id);
	ado::CADORecordset dataGridRecord(m_db->GetDatabase());
	dataGridRecord.Open(m_db->GetDatabase()->m_pConnection, query);
	ULONG count = dataGridRecord.GetRecordCount();
	long id = 0;
	if (count == 1) {
		dataGridRecord.MoveFirst();
		dataGridRecord.GetFieldValue(cMinID, id);
	}
	dataGridRecord.Close();
	JLOG(L"m_csLock.UnLock()\n");
	return id;
}


long history_record_manager::GetRecordMinimizeIDByMachineAndZone(int ademco_id, int zone_value)
{
	AUTO_LOG_FUNCTION;
	//std::lock_guard<std::mutex> lock(m_csRecord);
	while (!m_csLock.try_lock()) { JLOG(L"m_csLock.TryLock() failed.\n"); std::this_thread::sleep_for(std::chrono::milliseconds(500)); }
	std::lock_guard<std::mutex> lock(m_csLock, std::adopt_lock);
	JLOG(L"m_csLock.Lock()\n");
	const TCHAR* cMinID = _T("minimize_id");
	CString query = _T("");
	query.Format(_T("select min(id) as %s from HistoryRecord where ademco_id=%d and zone_value=%d"),
				 cMinID, ademco_id, zone_value);
	ado::CADORecordset dataGridRecord(m_db->GetDatabase());
	dataGridRecord.Open(m_db->GetDatabase()->m_pConnection, query);
	ULONG count = dataGridRecord.GetRecordCount();
	long id = 0;
	if (count == 1) {
		dataGridRecord.MoveFirst();
		dataGridRecord.GetFieldValue(cMinID, id);
	}
	dataGridRecord.Close();
	JLOG(L"m_csLock.UnLock()\n");
	return id;
}


void history_record_manager::TraverseHistoryRecord(const observer_ptr& ptr)
{
	AUTO_LOG_FUNCTION;
	CString query = _T("");
	query.Format(_T("select * from HistoryRecord order by id"));
	GetHistoryRecordBySql(query, ptr);
}


BOOL history_record_manager::GetHistoryRecordByDate(const CString& beg, const CString& end, const observer_ptr& ptr)
{
	AUTO_LOG_FUNCTION;
	CString query = _T("");
	query.Format(_T("select * from HistoryRecord where time between #%s# and #%s# order by id"),
				 beg, end);
	return GetHistoryRecordBySql(query, ptr, FALSE);
}


BOOL history_record_manager::GetHistoryRecordByDateByRecordLevel(const CString& beg, const CString& end, record_level level, const observer_ptr& ptr)
{
	AUTO_LOG_FUNCTION;
	CString query = _T("");
	query.Format(_T("select * from HistoryRecord where level=%d and time between #%s# and #%s# order by id"),
				 level, beg, end);
	return GetHistoryRecordBySql(query, ptr, FALSE);
}


BOOL history_record_manager::GetHistoryRecordByDateByUser(const CString& beg, const CString& end, int user_id, const observer_ptr& ptr)
{
	AUTO_LOG_FUNCTION;
	CString query = _T("");
	query.Format(_T("select * from HistoryRecord where user_id=%d and time between #%s# and #%s# order by id"),
				 user_id, beg, end);
	return GetHistoryRecordBySql(query, ptr, FALSE);
}


BOOL history_record_manager::GetHistoryRecordByDateByMachine(int ademco_id, 
													 const CString& beg,
													 const CString& end,
													 const observer_ptr& ptr)
{
	AUTO_LOG_FUNCTION;
	CString query = _T("");
	query.Format(_T("select * from HistoryRecord where ademco_id=%d and time between #%s# and #%s# order by id"),
				 ademco_id, beg, end);
	return GetHistoryRecordBySql(query, ptr, FALSE);
}


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
		CString query;
		query.Format(L"select * from HistoryRecord where id=%d", id);
		ado::CADORecordset dataGridRecord(m_db->GetDatabase());
		dataGridRecord.Open(m_db->GetDatabase()->m_pConnection, query);
		ULONG count = dataGridRecord.GetRecordCount();
		if (count  == 1) {
			dataGridRecord.MoveFirst();
			int ademco_id = -1, zone_value = -1, user_id = -1, level = -1;
			CString record_content = _T("");
			CString record_time = _T("");
			dataGridRecord.GetFieldValue(_T("ademco_id"), ademco_id);
			dataGridRecord.GetFieldValue(_T("zone_value"), zone_value);
			dataGridRecord.GetFieldValue(_T("user_id"), user_id);
			dataGridRecord.GetFieldValue(_T("record"), record_content);
			dataGridRecord.GetFieldValue(_T("time"), record_time);
			dataGridRecord.GetFieldValue(_T("level"), level);
			hr = std::make_shared<history_record>(id, ademco_id, zone_value,
												  user_id, Int2RecordLevel(level), 
												  record_content, record_time);
			m_recordMap[id] = hr;
		}
		dataGridRecord.Close();
	}

	JLOG(L"m_csLock.UnLock()\n");
	return hr;
}

};

