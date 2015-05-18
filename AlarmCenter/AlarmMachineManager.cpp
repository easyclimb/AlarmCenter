#include "stdafx.h"
#include "AlarmMachineManager.h"
#include "ado2.h"
#include "AlarmMachine.h"
//#include "SubMachineInfo.h"
#include "ademco_func.h"
#include "MapInfo.h"
#include "ZoneInfo.h"
#include "DetectorInfo.h"
#include "DetectorLib.h"
#include "ConfigHelper.h"
#include "resource.h"
#include "NetworkConnector.h"
#include "InputDlg.h"
#include "UserInfo.h"
#include "HistoryRecord.h"
#include "GroupInfo.h"
#include "AlarmCenter.h"
#include "AlarmCenterDlg.h"
#include "baidu.h"
#include "CsrInfo.h"

namespace core {


static const int ONE_MINUTE = 60 * 1000;
static const int ONE_HOUR = 60 * ONE_MINUTE;
#ifdef _DEBUG
static const int MAX_SUBMACHINE_ACTION_TIME_OUT = 16 * ONE_MINUTE;
static const int CHECK_GAP = ONE_MINUTE;
static const int TRY_LOCK_RETRY_GAP = ONE_MINUTE;
static const int WAIT_TIME_FOR_RETRIEVE_RESPONCE = 3000;
#else
static const int MAX_SUBMACHINE_ACTION_TIME_OUT = 16 * ONE_HOUR; // 16 hour
static const int CHECK_GAP = ONE_HOUR;
static const int TRY_LOCK_RETRY_GAP = ONE_MINUTE;
static const int WAIT_TIME_FOR_RETRIEVE_RESPONCE = ONE_MINUTE;
#endif

IMPLEMENT_SINGLETON(CAlarmMachineManager)

CAlarmMachineManager::CAlarmMachineManager()
	:/* m_rootGroupInfo(NULL)
	, */m_pDatabase(NULL)
	, m_pPrevCallDisarmWnd(NULL)
	, m_prevCallDisarmAdemcoID(-1)
	, m_prevCallDisarmGG(-1)
	, m_prevCallDisarmZoneValue(-1)
#ifdef USE_ARRAY
	, m_curMachinePos(0)
	, m_validMachineCount(0)
#endif
	, m_hThread(INVALID_HANDLE_VALUE)
	, m_hThreadChecker(INVALID_HANDLE_VALUE)
	, m_hEventExit(INVALID_HANDLE_VALUE)
	, m_hEventOotebm(INVALID_HANDLE_VALUE)
{
	AUTO_LOG_FUNCTION;
#ifdef USE_ARRAY
	//size_t sss = sizeof(m_alarmMachines);
	memset(m_alarmMachines, 0, sizeof(m_alarmMachines));
#endif
	
	m_hEventExit = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hEventOotebm = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hThread = CreateThread(NULL, 0, ThreadCheckSubMachine, this, 0, NULL);
	
}


CAlarmMachineManager::~CAlarmMachineManager()
{
	AUTO_LOG_FUNCTION;
	SetEvent(m_hEventExit);
	if (INVALID_HANDLE_VALUE != m_hThread)
		WaitForSingleObject(m_hThread, INFINITE);
	CLOSEHANDLE(m_hThread);
	CLOSEHANDLE(m_hEventExit); 
	CLOSEHANDLE(m_hEventOotebm);
#ifdef USE_ARRAY
	for (int i = 0; i < MAX_MACHINE; i++) {
		CAlarmMachine* machine = m_alarmMachines[i];
		SAFEDELETEP(machine);
	}
#else
	std::list<CAlarmMachine*>::iterator iter = m_listAlarmMachine.begin();
	while (iter != m_listAlarmMachine.end()) {
		CAlarmMachine* machine = *iter++;
		delete machine;
	}
	m_listAlarmMachine.clear();
#endif
	

	if (m_pDatabase) {
		if (m_pDatabase->IsOpen()) {
			m_pDatabase->Close();
		}
		delete m_pDatabase;
	}
	CDetectorLib::ReleaseObject();
	CGroupManager::ReleaseObject();
}


void CAlarmMachineManager::LoadFromDB(void* udata, LoadDBProgressCB cb)
{
	AUTO_LOG_FUNCTION;
	InitDB();
	InitCsrInfo();
	InitDetectorLib();
	LoadDetectorLibFromDB();
	//LoadZonePropertyInfoFromDB();
	LoadGroupInfoFromDB();

	//TestLoadAlarmMachineFromDB(udata, cb);
	LoadAlarmMachineFromDB(udata, cb);


}


void CAlarmMachineManager::InitCsrInfo()
{
	CString query = L"select * from CsrInfo";
	ado::CADORecordset recordset(m_pDatabase);
	recordset.Open(m_pDatabase->m_pConnection, query);
	DWORD count = recordset.GetRecordCount();
	if (count == 1) {
		CString acct, addr; int city_code; double x, y;
		recordset.MoveFirst();
		recordset.GetFieldValue(L"CsrAcct", acct);
		recordset.GetFieldValue(L"CsrAddress", addr);
		recordset.GetFieldValue(L"CsrCitycode", city_code);
		recordset.GetFieldValue(L"CsrBaiduMapX", x);
		recordset.GetFieldValue(L"CsrBaiduMapY", y);
		CCsrInfo* csr = CCsrInfo::GetInstance();
		csr->set_acct(acct);
		csr->set_addr(addr);
		csr->set_city_code(city_code);
		csr->set_x(x);
		csr->set_y(y);
	}
	recordset.Close();
}


BOOL CAlarmMachineManager::ExecuteSql(const CString& query)
{
	AUTO_LOG_FUNCTION;
	if (m_pDatabase) {
		LOG(L"%s\n", query);
		return m_pDatabase->Execute(query);
	}
	return FALSE;
}


int CAlarmMachineManager::AddAutoIndexTableReturnID(const CString& query)
{
	AUTO_LOG_FUNCTION;
	if (!m_pDatabase)
		return -1;

	if (!ExecuteSql(query))
		return -1;

	ado::CADORecordset recordset(m_pDatabase);
	recordset.Open(m_pDatabase->m_pConnection, L"select @@identity as _id_");
	DWORD count = recordset.GetRecordCount();
	if (count == 1) {
		recordset.MoveFirst();
		int id;
		recordset.GetFieldValue(L"_id_", id);
		return id;
	}
	return -1;
}

//
//const char* CAlarmMachineManager::GetCsrAcctA() const
//{
//	return m_csr_acctA;
//}
//
//
//const wchar_t* CAlarmMachineManager::GetCsrAcctW() const
//{
//	return m_csr_acctW;
//}
//
//
//void CAlarmMachineManager::SetCsrAcct(const char* csr_acct)
//{
//	strcpy_s(m_csr_acctA, csr_acct);
//	USES_CONVERSION;
//	wcscpy_s(m_csr_acctW, A2W(csr_acct));
//	CString sql;
//	sql.Format(L"update CsrInfo set CsrAcct='%s'", m_csr_acctW);
//	VERIFY(ExecuteSql(sql));
//}
//
//
//void CAlarmMachineManager::SetCsrAcct(const wchar_t* csr_acct)
//{
//	wcscpy_s(m_csr_acctW, csr_acct);
//	USES_CONVERSION;
//	strcpy_s(m_csr_acctA, W2A(csr_acct));
//}

void CAlarmMachineManager::InitDB()
{
	try {
		m_pDatabase = new ado::CADODatabase();
		//pDataGridRecord = new CADORecordset(m_pDatabase);
		TRACE(_T("after new, m_pDatabase %x, pDataGridRecord %x"),
			  m_pDatabase);
		LPCTSTR pszMdb = L"AlarmCenter.mdb";
		/*if (CConfig::IsChinese())
		pszMdb = _T("AlarmCenter.mdb");
		else
		pszMdb = _T("AlarmCenter_en.mdb");*/
		TCHAR szMdbPath[1024];
		_tcscpy_s(szMdbPath, GetModuleFilePath());
		_tcscat_s(szMdbPath, _T("\\"));
		_tcscat_s(szMdbPath, pszMdb);
		TRACE(_T("before pathexists"));
		if (!CFileOper::PathExists(szMdbPath)) {
			MessageBox(NULL, L"File AlarmCenter.mdb missed or broken!", L"Error", MB_OK | MB_ICONERROR);
			ExitProcess(0);
			return;
		}
		TRACE(_T("after pathexists"));
		//连接数据库
		CString strConn = _T("");
		strConn.Format(_T("Provider=Microsoft.Jet.OLEDB.4.0; Data Source='%s';Jet OLEDB:Database"), szMdbPath);
		TRACE(strConn);
		if (!m_pDatabase->Open(strConn)) {
			TRACE(_T("m_pDatabase->Open() error"));
			MessageBox(NULL, L"File AlarmCenter.mdb missed or broken!", L"Error", MB_OK | MB_ICONERROR);
			ExitProcess(0);
		} else {
			TRACE(_T("m_pDatabase->Open() ok"));
			CString trace = _T("");
			trace.Format(_T("CDBOper ConnectDB %s success\n"), strConn);
			TRACE(trace);
		}
	} catch (...) {
		AfxMessageBox(_T("connect to access error!"));
		ExitProcess(0);
	}
	TRACE(_T("CDBOper::CDBOper() ok"));
}


static const TCHAR* TRIPLE_CONDITION(int condition, const TCHAR* a,
									 const TCHAR* b, const TCHAR* c)
{
	switch (condition) {
		case 0: return a; break;
		case 1: return b; break;
		case 2: return c; break;
		default: ASSERT(0); return _T(""); break;
	}
}


void CAlarmMachineManager::InitDetectorLib()
{
	CLog::WriteLog(_T("CDBOper::InitData()"));
	const TCHAR *query = _T("select * from DetectorLib order by id");
	std::auto_ptr<ado::CADORecordset> pDataGridRecord(new ado::CADORecordset(m_pDatabase));
	pDataGridRecord->Open(m_pDatabase->m_pConnection, query);
	CLog::WriteLog(_T("pDataGridRecord->Open(m_pDatabase->m_pConnection 0x%x, %s)"),
				   m_pDatabase->m_pConnection, query);
	CLog::WriteLog(_T("pDataGridRecord->Open() over, calling GetRecordCount"));
	ULONG count = pDataGridRecord->GetRecordCount();
	CLog::WriteLog(_T("GetRecordCount over, count is %d"), count);
	if (count == 0) {
		//BOOL bChinese = CConfig::IsChinese();
		int condition = 0;
		//USES_CONVERSION;
		util::ApplicationLanguage lang = util::CConfigHelper::GetInstance()->GetLanguage();
		switch (lang) {
			case util::AL_CHINESE:condition = 0;
				break;
			case util::AL_ENGLISH:condition = 2;
				break;
			case util::AL_TAIWANESE:condition = 1;
				break;
			default:ASSERT(0);
				break;
		}

		CString detPath = _T("");
		detPath.Format(_T("%s\\Detectors\\"), GetModuleFilePath());

		CString format, query;
		format = L"insert into DetectorLib ([type],[detector_name],[path],[path_pair],[antline_num],[antline_gap]) values(%d,'%s','%s','%s',%d,%d)";
		
		// A2
		query.Format(format, DT_DOUBLE, _T("A2"), detPath + _T("A2.bmp"), 
					 detPath + L"A2Receiver.bmp", ALN_2, ALG_12);
		VERIFY(m_pDatabase->Execute(query));
		
		// A4
		query.Format(format, DT_DOUBLE, _T("A4"), detPath + _T("A4.bmp"),
					 detPath + L"A4Receiver.bmp", ALN_4, ALG_12);
		VERIFY(m_pDatabase->Execute(query));

		// A8
		query.Format(format, DT_DOUBLE, _T("A8"), detPath + _T("A8.bmp"),
					 detPath + L"A8Receiver.bmp", ALN_8, ALG_12);
		VERIFY(m_pDatabase->Execute(query));

		// R2
		query.Format(format, DT_DOUBLE, _T("R2"), detPath + _T("R2.bmp"),
					 detPath + L"R2Receiver.bmp", ALN_2, ALG_12);
		VERIFY(m_pDatabase->Execute(query));

		// R3
		query.Format(format, DT_DOUBLE, _T("R3"), detPath + _T("R3.bmp"),
					 detPath + L"R3Receiver.bmp", ALN_3, ALG_12);
		VERIFY(m_pDatabase->Execute(query));

		// R4
		query.Format(format, DT_DOUBLE, _T("R4"), detPath + _T("R4.bmp"),
					 detPath + L"R4Receiver.bmp", ALN_4, ALG_12);
		VERIFY(m_pDatabase->Execute(query));

		// R6
		query.Format(format, DT_DOUBLE, _T("R6"), detPath + _T("R6.bmp"),
					 detPath + L"R6Receiver.bmp", ALN_6, ALG_12);
		VERIFY(m_pDatabase->Execute(query));

		// R8
		query.Format(format, DT_DOUBLE, _T("R8"), detPath + _T("R8.bmp"),
					 detPath + L"R8Receiver.bmp", ALN_8, ALG_16);
		VERIFY(m_pDatabase->Execute(query));

		// S4
		query.Format(format, DT_DOUBLE, _T("S4"), detPath + _T("S4.bmp"),
					 detPath + L"S4Receiver.bmp", ALN_4, ALG_12);
		VERIFY(m_pDatabase->Execute(query));

		// S4-D
		query.Format(format, DT_DOUBLE, _T("S4-D"), detPath + _T("S4-D.bmp"),
					 detPath + L"S4-DReceiver.bmp", ALN_4, ALG_12);
		VERIFY(m_pDatabase->Execute(query));

		// S8
		query.Format(format, DT_DOUBLE, _T("S8"), detPath + _T("S8.bmp"),
					 detPath + L"S8Receiver.bmp", ALN_8, ALG_14);
		VERIFY(m_pDatabase->Execute(query));

		// S8-D
		query.Format(format, DT_DOUBLE, _T("S8-D"), detPath + _T("S8-D.bmp"),
					 detPath + L"S8-DReceiver.bmp", ALN_8, ALG_14);
		VERIFY(m_pDatabase->Execute(query));

		// T205
		query.Format(format, DT_SINGLE, _T("T205"), detPath + _T("T205.bmp"),
					 L"", ALN_0, ALG_0);
		VERIFY(m_pDatabase->Execute(query));

		// JHD-2
		query.Format(format, DT_SINGLE, _T("JHD-2"), detPath + _T("JHD-2.bmp"),
					 L"", ALN_0, ALG_0);
		VERIFY(m_pDatabase->Execute(query));

		// T201
		query.Format(format, DT_SINGLE, _T("T201"), detPath + _T("T201.bmp"),
					 L"", ALN_0, ALG_0);
		VERIFY(m_pDatabase->Execute(query));

		// T601
		query.Format(format, DT_SINGLE, _T("T601"), detPath + _T("T601.bmp"),
					 L"", ALN_0, ALG_0);
		VERIFY(m_pDatabase->Execute(query));

		// 无线门磁
		query.Format(format, DT_SINGLE, 
					 TRIPLE_CONDITION(condition, _T("无线门磁"), _T("無線門磁"), _T("WirelessDoorSensor")), 
					 detPath + _T("WirelessDoorSensor.bmp"), L"", ALN_0, ALG_0);
		VERIFY(m_pDatabase->Execute(query));

		// 紧急按钮HB-A380
		query.Format(format, DT_SINGLE,
					 TRIPLE_CONDITION(condition, _T("紧急按钮HB-A380"), _T("緊急按鈕HB-A380"), _T("EmergencyButtonHB-A380")),
					 detPath + _T("EmergencyButtonHB-A380.bmp"), L"", ALN_0, ALG_0);
		VERIFY(m_pDatabase->Execute(query));

		// 卧室主机HB-3030C
		query.Format(format, DT_SUB_MACHINE,
					 TRIPLE_CONDITION(condition, _T("卧室主机HB-3030C"), _T("臥室主機HB-3030C"), _T("HB-3030C")),
					 detPath + _T("HB-3030C.bmp"), L"", ALN_0, ALG_0);
		VERIFY(m_pDatabase->Execute(query));

		// 液晶主机HB-BJQ-560
		query.Format(format, DT_SUB_MACHINE,
					 TRIPLE_CONDITION(condition, _T("液晶主机HB-BJQ-560"), _T("液晶主機HB-BJQ-560"), _T("HB-BJQ-560")),
					 detPath + _T("HB-BJQ-560.bmp"), L"", ALN_0, ALG_0);
		VERIFY(m_pDatabase->Execute(query));
	}
	CLog::WriteLog(_T("CDBOper::InitData() ok"));
}

#ifdef _DEBUG
void print_group_info(CGroupInfo* group) 
{
	LOG(L"%d %d %s", group->get_id(), group->get_parent_id(), group->get_name());
}
#endif

void CAlarmMachineManager::LoadGroupInfoFromDB()
{
	static const wchar_t* query = L"select * from GroupInfo order by parent_id and id";
	ado::CADORecordset recordset(m_pDatabase);
	recordset.Open(m_pDatabase->m_pConnection, query);
	DWORD count = recordset.GetRecordCount();
	if (count > 0) {
		recordset.MoveFirst();
		CGroupManager* mgr = CGroupManager::GetInstance();
		std::list<CGroupInfo*> unresolvedGroupList;
		for (DWORD i = 0; i < count; i++) {
			int id, parent_id;
			CString name;
			recordset.GetFieldValue(L"id", id);
			recordset.GetFieldValue(L"parent_id", parent_id);
			recordset.GetFieldValue(L"group_name", name);
			recordset.MoveNext();
			
			if (id == 1 && parent_id == 0) {
				mgr->_tree.set_id(1);
				mgr->_tree.set_parent_id(0);
				CString rootName;
				rootName.LoadStringW(IDS_STRING_GROUP_ROOT);
				mgr->_tree.set_name(rootName);
			} else {
				CGroupInfo* group = new CGroupInfo();
				group->set_id(id);
				group->set_parent_id(parent_id);
				group->set_name(name);
				//mgr->_groupList.push_back(group);
				//m_listGroupInfo.push_back(group);
				bool ok = mgr->_tree.AddChildGroup(group);
				if (!ok) {
					unresolvedGroupList.push_back(group);
				}
			}
		}

		std::list<CGroupInfo*>::iterator iter = unresolvedGroupList.begin();
		while (iter != unresolvedGroupList.end()) {
			CGroupInfo* group = *iter++;
			bool ok = mgr->_tree.AddChildGroup(group);
			VERIFY(ok);
		}
		
//#ifdef _DEBUG
//		mgr;
//#endif
//		CGroupInfo* group = &mgr->_tree;
//		print_group_info(group);
//		std::list<CGroupInfo*>::iterator iter = group->b

	}
	recordset.Close();
}


void CAlarmMachineManager::LoadAlarmMachineFromDB(void* udata, LoadDBProgressCB cb)
{
	AUTO_LOG_FUNCTION;
	static const wchar_t* query = L"select * from AlarmMachine order by ademco_id";
	ado::CADORecordset recordset(m_pDatabase);
	LOG(L"CADORecordset recordset %p\n", &recordset);
	BOOL ret = recordset.Open(m_pDatabase->m_pConnection, query);
	VERIFY(ret); LOG(L"recordset.Open() return %d\n", ret);
	DWORD count = recordset.GetRecordCount();
	ProgressEx progress;
	LOG(L"recordset.GetRecordCount() return %d\n", count);
	if (count > 0) {
		CGroupManager* mgr = CGroupManager::GetInstance();
		CString null;
		null.LoadStringW(IDS_STRING_NULL);
		recordset.MoveFirst();
		for (DWORD i = 0; i < count; i++) {
			CAlarmMachine *machine = new CAlarmMachine();
			int id, ademco_id, group_id, banned, type, has_video;
			CString device_id, alias, contact, address, phone, phone_bk;
			COleDateTime expire_time;
			recordset.GetFieldValue(L"id", id);
			recordset.GetFieldValue(L"ademco_id", ademco_id);
			recordset.GetFieldValue(L"device_id", device_id);
			recordset.GetFieldValue(L"machine_type", type);
			recordset.GetFieldValue(L"banned", banned); 
			recordset.GetFieldValue(L"banned", has_video);
			recordset.GetFieldValue(L"alias", alias);
			if (alias.IsEmpty()) { alias = null; }
			recordset.GetFieldValue(L"contact", contact);
			if (contact.IsEmpty()) { contact = null; }
			recordset.GetFieldValue(L"address", address);
			if (address.IsEmpty()) { address = null; }
			recordset.GetFieldValue(L"phone", phone);
			if (phone.IsEmpty()) { phone = null; }
			recordset.GetFieldValue(L"phone_bk", phone_bk);
			if (phone_bk.IsEmpty()) { phone_bk = null; }
			recordset.GetFieldValue(L"expire_time", expire_time);
			recordset.GetFieldValue(L"group_id", group_id);
			recordset.MoveNext();

			machine->set_id(id);
			machine->set_ademco_id(ademco_id);
			machine->set_device_id(device_id);
			machine->set_machine_type(Integer2MachineType(type));
			machine->set_banned(banned != 0);
			machine->set_has_video(has_video != 0);
			machine->set_alias(alias);
			machine->set_contact(contact);
			machine->set_address(address);
			machine->set_phone(phone);
			machine->set_phone_bk(phone_bk);
			machine->set_group_id(group_id);
			machine->set_expire_time(expire_time);
#ifdef USE_ARRAY
			m_alarmMachines[ademco_id] = machine;
			m_validMachineCount++;
#else
			m_listAlarmMachine.push_back(machine);
#endif

			if (cb && udata) {
				progress.progress = static_cast<int>(i * MAX_MACHINE / count);
				progress.value = ademco_id;
				progress.total = count;
				cb(udata, true, &progress);
			}

			LoadMapInfoFromDB(machine);
			//LoadUnbindZoneMapInfoFromDB(machine);
			LoadZoneInfoFromDB(machine, udata, cb, &progress);
			bool ok = mgr->_tree.AddChildMachine(machine);
			VERIFY(ok);
		}
	}
	recordset.Close();

	if (cb && udata) {
		progress.progress = MAX_MACHINE;
		progress.value = MAX_MACHINE;
		progress.total = MAX_MACHINE;
		cb(udata, true, &progress);
	}
}


#ifdef _DEBUG
void CAlarmMachineManager::TestLoadAlarmMachineFromDB(void* udata, LoadDBProgressCB cb)
{
	AUTO_LOG_FUNCTION;
	static const wchar_t* query = L"select * from AlarmMachine order by ademco_id";
	ado::CADORecordset recordset_machine(m_pDatabase);
	recordset_machine.Open(m_pDatabase->m_pConnection, query);
	DWORD machine_count = recordset_machine.GetRecordCount();
	ProgressEx progress;
	if (machine_count > 0) {
		CString null;
		null.LoadStringW(IDS_STRING_NULL);
		recordset_machine.MoveFirst();
		for (DWORD i = 0; i < machine_count; i++) {
			int id_machine, ademco_id, group_id, banned, type;
			CString device_id, alias, contact, address, phone, phone_bk;
			recordset_machine.GetFieldValue(L"id", id_machine);
			recordset_machine.GetFieldValue(L"ademco_id", ademco_id);
			recordset_machine.GetFieldValue(L"device_id", device_id);
			recordset_machine.GetFieldValue(L"machine_type", type);
			recordset_machine.GetFieldValue(L"banned", banned);
			recordset_machine.GetFieldValue(L"alias", alias);
			if (alias.IsEmpty()) { alias = null; }
			recordset_machine.GetFieldValue(L"contact", contact);
			if (contact.IsEmpty()) { contact = null; }
			recordset_machine.GetFieldValue(L"address", address);
			if (address.IsEmpty()) { address = null; }
			recordset_machine.GetFieldValue(L"phone", phone);
			if (phone.IsEmpty()) { phone = null; }
			recordset_machine.GetFieldValue(L"phone_bk", phone_bk);
			if (phone_bk.IsEmpty()) { phone_bk = null; }
			recordset_machine.GetFieldValue(L"group_id", group_id);
			recordset_machine.MoveNext();

			if (cb && udata) {
				progress.progress = static_cast<int>(i * MAX_MACHINE / machine_count);
				progress.value = ademco_id;
				progress.total = machine_count;
				cb(udata, true, &progress);
			}

			{
				CString query;
				query.Format(L"select * from MapInfo where type=%d and machine_id=%d order by id",
							 MAP_MACHINE, ademco_id);
				ado::CADORecordset recordset_map(m_pDatabase);
				recordset_map.Open(m_pDatabase->m_pConnection, query);
				DWORD map_count = recordset_map.GetRecordCount();
				if (map_count > 0) {
					CString null;
					null.LoadStringW(IDS_STRING_NULL);
					recordset_map.MoveFirst();
					for (DWORD i = 0; i < map_count; i++) {
						int id_map, type, machine_id;
						CString alias, path;
						recordset_map.GetFieldValue(L"id", id_map);
						recordset_map.GetFieldValue(L"type", type);
						recordset_map.GetFieldValue(L"machine_id", machine_id);
						recordset_map.GetFieldValue(L"alias", alias);
						if (alias.IsEmpty()) { alias = null; }
						recordset_map.GetFieldValue(L"path", path);
						recordset_map.MoveNext();

						{
							CString query;
							query.Format(L"select * from DetectorInfo where map_id=%d and zone_info_id=-1 order by id",
										 id_map);
							ado::CADORecordset recordset_det(m_pDatabase);
							recordset_det.Open(m_pDatabase->m_pConnection, query);
							DWORD detcount = recordset_det.GetRecordCount();
							if (detcount > 0) {
								recordset_det.MoveFirst();
								for (DWORD i = 0; i < detcount; i++) {
									int id, /*zone_info_id, map_id,*/ x, y, distance, angle, detector_lib_id;
									recordset_det.GetFieldValue(L"id", id);
									//recordset.GetFieldValue(L"map_id", map_id);
									recordset_det.GetFieldValue(L"x", x);
									recordset_det.GetFieldValue(L"y", y);
									recordset_det.GetFieldValue(L"distance", distance);
									recordset_det.GetFieldValue(L"angle", angle);
									recordset_det.GetFieldValue(L"detector_lib_id", detector_lib_id);
									recordset_det.MoveNext();
								}
							}
							recordset_det.Close();
						}
					}
				}
				recordset_map.Close();
			}
			//LoadUnbindZoneMapInfoFromDB(machine);
			//LoadZoneInfoFromDB(machine, udata, cb, &progress);
			ProgressEx subProgress;
			progress.subProgress = &subProgress;
			if (cb && udata) {
				subProgress.progress = 0;
				subProgress.value = 0;
				subProgress.total = 0;
				cb(udata, false, &subProgress);
			}

			CString query;
			query.Format(L"select * from ZoneInfo where ademco_id=%d order by zone_value",
						 ademco_id);
			ado::CADORecordset recordset_zone(m_pDatabase);
			recordset_zone.Open(m_pDatabase->m_pConnection, query);
			DWORD count_zone = recordset_zone.GetRecordCount();
			if (count_zone > 0) {
				CString null;
				null.LoadStringW(IDS_STRING_NULL);
				recordset_zone.MoveFirst();
				for (DWORD i = 0; i < count_zone; i++) {
					int id, ademco_id, zone_value, /*sub_zone_id, */type_zone,
						/*property_id, */detector_id, sub_machine_id;
					CString alias;
					recordset_zone.GetFieldValue(L"id", id);
					recordset_zone.GetFieldValue(L"ademco_id", ademco_id);
					recordset_zone.GetFieldValue(L"zone_value", zone_value);
					//recordset.GetFieldValue(L"sub_zone_id", sub_zone_id);
					recordset_zone.GetFieldValue(L"type", type_zone);
					recordset_zone.GetFieldValue(L"alias", alias);
					if (alias.GetLength() == 0) { alias = null; }
					//recordset.GetFieldValue(L"property_info_id", property_id);
					recordset_zone.GetFieldValue(L"detector_info_id", detector_id);
					recordset_zone.GetFieldValue(L"sub_machine_id", sub_machine_id);
					recordset_zone.MoveNext();

					//LoadDetectorInfoFromDB(zone);
					{
						CString query;
						query.Format(L"select * from DetectorInfo where id=%d",
									 detector_id);
						ado::CADORecordset recordset_det2(m_pDatabase);
						recordset_det2.Open(m_pDatabase->m_pConnection, query);
						DWORD count_det2 = recordset_det2.GetRecordCount();
						if (count_det2 == 1) {
							recordset_det2.MoveFirst();
							int id, /*zone_info_id, */map_id, x, y, distance, angle, detector_lib_id;
							recordset_det2.GetFieldValue(L"id", id);
							recordset_det2.GetFieldValue(L"map_id", map_id);
							recordset_det2.GetFieldValue(L"x", x);
							recordset_det2.GetFieldValue(L"y", y);
							recordset_det2.GetFieldValue(L"distance", distance);
							recordset_det2.GetFieldValue(L"angle", angle);
							recordset_det2.GetFieldValue(L"detector_lib_id", detector_lib_id);
						}
						recordset_det2.Close();
					}

					if (type_zone == ZT_SUB_MACHINE) {
						//LoadSubMachineInfoFromDB(zone);
						CString query;
						query.Format(L"select * from SubMachine where id=%d",
									 sub_machine_id);
						ado::CADORecordset recordset_sub(m_pDatabase);
						recordset_sub.Open(m_pDatabase->m_pConnection, query);
						DWORD count_sub = recordset_sub.GetRecordCount();
						if (count_sub == 1) {
							CString null;
							null.LoadStringW(IDS_STRING_NULL);
							recordset_sub.MoveFirst();
							CString /*alias, */contact, address, phone, phone_bk;
							//recordset.GetFieldValue(L"alias", alias);
							recordset_sub.GetFieldValue(L"contact", contact);
							if (contact.IsEmpty()) { contact = null; }
							recordset_sub.GetFieldValue(L"address", address);
							if (address.IsEmpty()) { address = null; }
							recordset_sub.GetFieldValue(L"phone", phone);
							if (phone.IsEmpty()) { phone = null; }
							recordset_sub.GetFieldValue(L"phone_bk", phone_bk);
							if (phone_bk.IsEmpty()) { phone_bk = null; }

							//LoadMapInfoFromDB(subMachine);
							{
								CString query;
								query.Format(L"select * from MapInfo where type=%d and machine_id=%d order by id",
											 MAP_SUB_MACHINE, sub_machine_id);
								ado::CADORecordset recordset_map2(m_pDatabase);
								recordset_map2.Open(m_pDatabase->m_pConnection, query);
								DWORD map_count2 = recordset_map2.GetRecordCount();
								if (map_count2 > 0) {
									CString null;
									null.LoadStringW(IDS_STRING_NULL);
									recordset_map2.MoveFirst();
									for (DWORD i = 0; i < map_count2; i++) {
										int id_map, type, machine_id;
										CString alias, path;
										recordset_map2.GetFieldValue(L"id", id_map);
										recordset_map2.GetFieldValue(L"type", type);
										recordset_map2.GetFieldValue(L"machine_id", machine_id);
										recordset_map2.GetFieldValue(L"alias", alias);
										if (alias.IsEmpty()) { alias = null; }
										recordset_map2.GetFieldValue(L"path", path);
										recordset_map2.MoveNext();
									}
								}
								recordset_map2.Close();
							}

							//LoadSubZoneInfoOfSubMachineFromDB(subMachine);
							{
								CString query;
								query.Format(L"select * from SubZone where sub_machine_id=%d",
											 sub_machine_id);
								ado::CADORecordset recordset_sub_zone(m_pDatabase);
								recordset_sub_zone.Open(m_pDatabase->m_pConnection, query);
								DWORD count = recordset_sub_zone.GetRecordCount();
								if (count > 0) {
									CString null;
									null.LoadStringW(IDS_STRING_NULL);
									recordset_sub_zone.MoveFirst();
									for (DWORD i = 0; i < count; i++) {
										CString alias;
										int id, sub_zone_value, detector_info_id/*, property_info_id*/;
										recordset_sub_zone.GetFieldValue(L"id", id);
										recordset_sub_zone.GetFieldValue(L"sub_zone", sub_zone_value);
										recordset_sub_zone.GetFieldValue(L"detector_info_id", detector_info_id);
										//recordset.GetFieldValue(L"property_info_id", property_info_id);
										recordset_sub_zone.GetFieldValue(L"alias", alias);
										if (alias.GetLength() == 0) { alias = null; }
										recordset_sub_zone.MoveNext();
									}
								}
								recordset_sub_zone.Close();
							}

						}
						recordset_sub.Close();
					}
					if (cb && udata) {
						subProgress.progress = static_cast<int>(i * MAX_MACHINE_ZONE / count_zone);
						subProgress.value = zone_value;
						subProgress.total = count_zone;
						cb(udata, false, &subProgress);
					}
				}
			}
			recordset_zone.Close();
		}
	}
	recordset_machine.Close();
}
#endif


void CAlarmMachineManager::LoadMapInfoFromDB(CAlarmMachine* machine)
{
	AUTO_LOG_FUNCTION;
	MapType mt = machine->get_is_submachine() ? MAP_SUB_MACHINE : MAP_MACHINE;
	CString query;
	query.Format(L"select * from MapInfo where type=%d and machine_id=%d order by id", mt, 
				 machine->get_is_submachine() ? machine->get_id() : machine->get_ademco_id());
	ado::CADORecordset recordset(m_pDatabase);
	recordset.Open(m_pDatabase->m_pConnection, query);
	DWORD count = recordset.GetRecordCount();
	if (count > 0) {
		CString null;
		null.LoadStringW(IDS_STRING_NULL);
		recordset.MoveFirst();
		for (DWORD i = 0; i < count; i++) {
			int id, type, machine_id;
			CString alias, path; 
			recordset.GetFieldValue(L"id", id);
			recordset.GetFieldValue(L"type", type);
			recordset.GetFieldValue(L"machine_id", machine_id);
			recordset.GetFieldValue(L"alias", alias);
			if (alias.IsEmpty()) { alias = null; }
			recordset.GetFieldValue(L"path", path);
			recordset.MoveNext();

			CMapInfo* mapInfo = new CMapInfo();
			mapInfo->set_id(id);
			mapInfo->set_type(type);
			mapInfo->set_machine_id(machine_id);
			mapInfo->set_alias(alias);
			mapInfo->set_path(path);
			//LoadZoneInfoFromDB(mapInfo);
			LoadNoZoneHasMapDetectorInfoFromDB(mapInfo);
			machine->AddMap(mapInfo);
		}
	}
	recordset.Close();
}


void CAlarmMachineManager::LoadNoZoneHasMapDetectorInfoFromDB(CMapInfo* mapInfo)
{
	CString query;
	query.Format(L"select * from DetectorInfo where map_id=%d and zone_info_id=-1 order by id",
				 mapInfo->get_id());
	ado::CADORecordset recordset(m_pDatabase);
	recordset.Open(m_pDatabase->m_pConnection, query);
	DWORD count = recordset.GetRecordCount();
	if (count > 0) {
		recordset.MoveFirst();
		for (DWORD i = 0; i < count; i++) {
			int id, /*zone_info_id, map_id,*/ x, y, distance, angle, detector_lib_id;
			recordset.GetFieldValue(L"id", id);
			//recordset.GetFieldValue(L"map_id", map_id);
			recordset.GetFieldValue(L"x", x);
			recordset.GetFieldValue(L"y", y);
			recordset.GetFieldValue(L"distance", distance);
			recordset.GetFieldValue(L"angle", angle);
			recordset.GetFieldValue(L"detector_lib_id", detector_lib_id);
			recordset.MoveNext();

			CDetectorInfo* detector = new CDetectorInfo();
			detector->set_id(id);
			detector->set_zone_info_id(-1);
			detector->set_map_id(mapInfo->get_id());
			detector->set_zone_value(-1);
			detector->set_x(x);
			detector->set_y(y);
			detector->set_distance(distance);
			detector->set_angle(angle);
			detector->set_detector_lib_id(detector_lib_id);
			mapInfo->AddNoZoneDetectorInfo(detector);
		}
	}
}


//void CAlarmMachineManager::LoadUnbindZoneMapInfoFromDB(CAlarmMachine* machine)
//{
//	CMapInfo* mapInfo = new CMapInfo();
//	mapInfo->set_id(-1);
//	CString fmAlias;
//	fmAlias.LoadStringW(IDS_STRING_NOZONEMAP);
//	mapInfo->set_alias(fmAlias);
//
//	CString query;
//	query.Format(L"select * from ZoneInfo where ademco_id=%d and map_id=-1 order by zone_id",
//				 machine->get_ademco_id());
//	ado::CADORecordset recordset(m_pDatabase);
//	recordset.Open(m_pDatabase->m_pConnection, query);
//	DWORD count = recordset.GetRecordCount();
//	if (count > 0) {
//		recordset.MoveFirst();
//		for (DWORD i = 0; i < count; i++) {
//			int id, zone_id, ademco_id, /*map_id, */detector_id, property_id;
//			CString alias;
//			recordset.GetFieldValue(L"id", id);
//			recordset.GetFieldValue(L"zone_id", zone_id);
//			recordset.GetFieldValue(L"ademco_id", ademco_id);
//			//recordset.GetFieldValue(L"map_id", map_id);
//			recordset.GetFieldValue(L"alias", alias);
//			recordset.GetFieldValue(L"detector_info_id", detector_id);
//			recordset.GetFieldValue(L"property_info_id", property_id);
//			recordset.MoveNext();
//
//			CZoneInfo* zone = new CZoneInfo();
//			zone->set_id(id);
//			zone->set_zone(zone_id);
//			zone->set_ademco_id(ademco_id);
//			//zone->set_map_id(map_id);
//			//zone->set_type(type);
//			zone->set_alias(alias);
//			zone->set_detector_id(detector_id);
//			zone->set_property_id(property_id);
//			LoadDetectorInfoFromDB(zone);
//
//			mapInfo->AddZone(zone);
//		}
//	}
//	machine->SetUnbindZoneMap(mapInfo);
//	recordset.Close();
//}


void CAlarmMachineManager::LoadZoneInfoFromDB(CAlarmMachine* machine, void* udata, LoadDBProgressCB cb, ProgressEx* progress)
{
	AUTO_LOG_FUNCTION;
	ProgressEx subProgress;
	progress->subProgress = &subProgress;
	if (cb && udata) {
		subProgress.progress = 0;
		subProgress.value = 0;
		subProgress.total = MAX_MACHINE_ZONE;
		cb(udata, false, progress);
		LOG(L"SUBPROGRESS reset 0 OK\n");
	}

	CString query;
	query.Format(L"select * from ZoneInfo where ademco_id=%d order by zone_value",
				 machine->get_ademco_id());
	ado::CADORecordset recordset(m_pDatabase);
	LOG(L"CADORecordset recordset %p\n", &recordset);
	BOOL ret = recordset.Open(m_pDatabase->m_pConnection, query); VERIFY(ret);
	LOG(L"recordset.Open() return %d\n", ret);
	DWORD count = recordset.GetRecordCount();
	LOG(L"recordset.GetRecordCount() return %d\n", count);
	if (count > 0) {
		CString null;
		null.LoadStringW(IDS_STRING_NULL);
		recordset.MoveFirst();
		for (DWORD i = 0; i < count; i++) {
			int id, ademco_id, zone_value, /*sub_zone_id, */type,
				status_or_property, detector_id, sub_machine_id, addr;
			CString alias;
			recordset.GetFieldValue(L"id", id);
			recordset.GetFieldValue(L"ademco_id", ademco_id);
			recordset.GetFieldValue(L"zone_value", zone_value);
			//recordset.GetFieldValue(L"sub_zone_id", sub_zone_id);
			recordset.GetFieldValue(L"type", type);
			recordset.GetFieldValue(L"alias", alias);
			if (alias.GetLength() == 0) { alias = null; }
			//recordset.GetFieldValue(L"property_info_id", property_id);
			recordset.GetFieldValue(L"detector_info_id", detector_id);
			recordset.GetFieldValue(L"sub_machine_id", sub_machine_id);
			recordset.GetFieldValue(L"status_or_property", status_or_property);
			recordset.GetFieldValue(L"physical_addr", addr);
			recordset.MoveNext();
			
			CZoneInfo* zone = new CZoneInfo();
			zone->set_id(id);
			zone->set_ademco_id(ademco_id);
			zone->set_zone_value(zone_value);
			//zone->set_sub_zone(sub_zone_id);
			//zone->set_map_id(map_id);
			zone->set_type(type);
			zone->set_alias(alias);
			zone->set_detector_id(detector_id);
			//zone->set_property_id(property_id);
			zone->set_sub_machine_id(sub_machine_id);
			zone->set_status_or_property(status_or_property);
			zone->set_physical_addr(addr);
			LoadDetectorInfoFromDB(zone);
			if (zone->get_type() == ZT_SUB_MACHINE)
				LoadSubMachineInfoFromDB(zone);
			machine->AddZone(zone);

			if (cb && udata) { 
				subProgress.progress = static_cast<int>(i * MAX_MACHINE_ZONE / count);
				subProgress.value = zone_value;
				subProgress.total = count;
				cb(udata, false, progress);
			}
		}

		/*for (int i = 0; i < MAX_MACHINE_ZONE; i++) {
			CZoneInfo* zone = machine->GetZone(i);
			if (zone) {
				LoadDetectorInfoFromDB(zone);
				if (zone->get_type() == ZT_SUB_MACHINE)
					LoadSubMachineInfoFromDB(zone);
				if (cb && udata) {
					subProgress.value = i + 1;
					subProgress.percent = static_cast<int>(i * MAX_MACHINE_ZONE / count);
					cb(udata, false, progress);
				}
			}
		}*/
	}
	/*if (cb && udata) {
		subProgress.progress = MAX_MACHINE_ZONE;
		subProgress.value = MAX_MACHINE_ZONE;
		subProgress.total = MAX_MACHINE_ZONE;
		cb(udata, false, progress);
	}*/
	recordset.Close();
}

//
//void CAlarmMachineManager::LoadZoneInfoFromDB(CMapInfo* mapInfo)
//{
//	CString query;
//	query.Format(L"select * from ZoneInfo where ademco_id=%d and map_id=%d order by zone_id",
//				 mapInfo->get_machine_id(), mapInfo->get_id());
//	ado::CADORecordset recordset(m_pDatabase);
//	recordset.Open(m_pDatabase->m_pConnection, query);
//	DWORD count = recordset.GetRecordCount();
//	if (count > 0) {
//		recordset.MoveFirst();
//		for (DWORD i = 0; i < count; i++) {
//			int id, zone_id, ademco_id, /*map_id, */detector_id, property_id;
//			CString alias;
//			recordset.GetFieldValue(L"id", id);
//			recordset.GetFieldValue(L"zone_id", zone_id);
//			recordset.GetFieldValue(L"ademco_id", ademco_id);
//			//recordset.GetFieldValue(L"map_id", map_id);
//			recordset.GetFieldValue(L"alias", alias);
//			recordset.GetFieldValue(L"detector_info_id", detector_id);
//			recordset.GetFieldValue(L"property_info_id", property_id);
//			recordset.MoveNext();
//
//			CZoneInfo* zone = new CZoneInfo();
//			zone->set_id(id);
//			zone->set_zone(zone_id);
//			zone->set_ademco_id(ademco_id);
//			//zone->set_map_id(map_id);
//			//zone->set_type(type);
//			zone->set_alias(alias);
//			zone->set_detector_id(detector_id);
//			zone->set_property_id(property_id);
//			LoadDetectorInfoFromDB(zone);
//			mapInfo->AddZone(zone);
//		}
//	}
//	recordset.Close();
//}


void CAlarmMachineManager::LoadDetectorInfoFromDB(CZoneInfo* zone)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"select * from DetectorInfo where id=%d",
				 zone->get_detector_id());
	ado::CADORecordset recordset(m_pDatabase);
	recordset.Open(m_pDatabase->m_pConnection, query);
	DWORD count = recordset.GetRecordCount();
	if (count == 1) {
		recordset.MoveFirst();
		int id, /*zone_info_id, */map_id, x, y, distance, angle, detector_lib_id;
		recordset.GetFieldValue(L"id", id);
		recordset.GetFieldValue(L"map_id", map_id);
		recordset.GetFieldValue(L"x", x);
		recordset.GetFieldValue(L"y", y);
		recordset.GetFieldValue(L"distance", distance);
		recordset.GetFieldValue(L"angle", angle);
		recordset.GetFieldValue(L"detector_lib_id", detector_lib_id);

		CDetectorInfo* detector = new CDetectorInfo();
		detector->set_id(id);
		detector->set_map_id(map_id);
		detector->set_zone_info_id(zone->get_id());
		bool bSubZone = (ZT_SUB_MACHINE_ZONE == zone->get_type());
		int zone_value = bSubZone ? zone->get_sub_zone() : zone->get_zone_value();
		detector->set_zone_value(zone_value);
		detector->set_x(x);
		detector->set_y(y);
		detector->set_distance(distance);
		detector->set_angle(angle);
		detector->set_detector_lib_id(detector_lib_id);
		zone->SetDetectorInfo(detector);
	}
	recordset.Close();
}


void CAlarmMachineManager::LoadSubMachineInfoFromDB(CZoneInfo* zone)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"select * from SubMachine where id=%d",
				 zone->get_sub_machine_id());
	ado::CADORecordset recordset(m_pDatabase);
	recordset.Open(m_pDatabase->m_pConnection, query);
	DWORD count = recordset.GetRecordCount();
	if (count == 1) {
		CString null;
		null.LoadStringW(IDS_STRING_NULL);
		recordset.MoveFirst();
		CString /*alias, */contact, address, phone, phone_bk;
		//recordset.GetFieldValue(L"alias", alias);
		recordset.GetFieldValue(L"contact", contact);
		if (contact.IsEmpty()) { contact = null; }
		recordset.GetFieldValue(L"address", address);
		if (address.IsEmpty()) { address = null; }
		recordset.GetFieldValue(L"phone", phone);
		if (phone.IsEmpty()) { phone = null; }
		recordset.GetFieldValue(L"phone_bk", phone_bk);
		if (phone_bk.IsEmpty()) { phone_bk = null; }

		CAlarmMachine* subMachine = new CAlarmMachine();
		subMachine->set_is_submachine(true);
		subMachine->set_id(zone->get_sub_machine_id());
		subMachine->set_ademco_id(zone->get_ademco_id());
		subMachine->set_submachine_zone(zone->get_zone_value());
		subMachine->set_alias(zone->get_alias());
		subMachine->set_address(address);
		subMachine->set_contact(contact);
		subMachine->set_phone(phone);
		subMachine->set_phone_bk(phone_bk);
		CAlarmMachine* parentMachine = NULL;
		if (GetMachine(zone->get_ademco_id(), parentMachine) && parentMachine) {
			subMachine->set_machine_type(parentMachine->get_machine_type());
		}

		LoadMapInfoFromDB(subMachine);
		LoadSubZoneInfoOfSubMachineFromDB(subMachine);

		zone->SetSubMachineInfo(subMachine);
	}
	recordset.Close();
}


void CAlarmMachineManager::LoadSubZoneInfoOfSubMachineFromDB(CAlarmMachine* subMachine)
{
	CString query;
	query.Format(L"select * from SubZone where sub_machine_id=%d",
				 subMachine->get_id());
	ado::CADORecordset recordset(m_pDatabase);
	recordset.Open(m_pDatabase->m_pConnection, query);
	DWORD count = recordset.GetRecordCount();
	if (count > 0) {
		CString null;
		null.LoadStringW(IDS_STRING_NULL);
		recordset.MoveFirst();
		for (DWORD i = 0; i < count; i++) {
			CString alias;
			int id, sub_zone_value, detector_info_id/*, property_info_id*/;
			recordset.GetFieldValue(L"id", id);
			recordset.GetFieldValue(L"sub_zone", sub_zone_value);
			recordset.GetFieldValue(L"detector_info_id", detector_info_id);
			//recordset.GetFieldValue(L"property_info_id", property_info_id);
			recordset.GetFieldValue(L"alias", alias);
			if (alias.GetLength() == 0) { alias = null; }
			recordset.MoveNext();

			CZoneInfo* subZone = new CZoneInfo();
			subZone->set_id(id);
			subZone->set_ademco_id(subMachine->get_ademco_id());
			subZone->set_sub_zone(sub_zone_value);
			subZone->set_sub_machine_id(subMachine->get_id());
			subZone->set_alias(alias);
			subZone->set_detector_id(detector_info_id);
			//subZone->set_property_id(property_info_id);
			subZone->set_type(ZT_SUB_MACHINE_ZONE);

			LoadDetectorInfoFromDB(subZone);

			subMachine->AddZone(subZone);
		}
	}
	recordset.Close();
}


void CAlarmMachineManager::LoadDetectorLibFromDB()
{
	CDetectorLib* detectorLib = CDetectorLib::GetInstance();
	CString query;
	query.Format(L"select * from DetectorLib order by id");
	ado::CADORecordset recordset(m_pDatabase);
	recordset.Open(m_pDatabase->m_pConnection, query);
	DWORD count = recordset.GetRecordCount();
	if (count > 0) {
		recordset.MoveFirst();
		for (DWORD i = 0; i < count; i++) {
			int id, type, antline_num, antline_gap;
			CString detector_name, path, path_pair;
			recordset.GetFieldValue(L"id", id);
			recordset.GetFieldValue(L"type", type);
			recordset.GetFieldValue(L"detector_name", detector_name);
			recordset.GetFieldValue(L"path", path);
			recordset.GetFieldValue(L"path_pair", path_pair);
			recordset.GetFieldValue(L"antline_num", antline_num);
			recordset.GetFieldValue(L"antline_gap", antline_gap);
			recordset.MoveNext();

			CDetectorLibData* data = new CDetectorLibData();
			data->set_id(id);
			data->set_type(type);
			data->set_detector_name(detector_name);
			data->set_path(path);
			data->set_path_pair(path_pair);
			data->set_antline_num(antline_num);
			data->set_antline_gap(antline_gap);
			detectorLib->AddDetectorLibData(data);
		}
	}
	recordset.Close();
}

//
//void CAlarmMachineManager::LoadZonePropertyInfoFromDB()
//{
//	CString query;
//	util::ApplicationLanguage lang = util::CConfigHelper::GetInstance()->GetLanguage();
//	switch (lang) {
//		case util::AL_CHINESE:
//			query.Format(L"select id,zone_property,zone_property_text_ch as zone_property_text,zone_alarm_text_ch as zone_alarm_text from ZonePropertyInfo order by id");
//			break;
//		case util::AL_ENGLISH:
//			query.Format(L"select id,zone_property,zone_property_text_en as zone_property_text,zone_alarm_text_en as zone_alarm_text from ZonePropertyInfo order by id");
//			break;
//		case util::AL_TAIWANESE:
//			query.Format(L"select id,zone_property,zone_property_text_tw as zone_property_text,zone_alarm_text_tw as zone_alarm_text from ZonePropertyInfo order by id");
//			break;
//		default:
//			ASSERT(0);
//			break;
//	}
//	
//	ado::CADORecordset recordset(m_pDatabase);
//	recordset.Open(m_pDatabase->m_pConnection, query);
//	DWORD count = recordset.GetRecordCount();
//	if (count > 0) {
//		CZonePropertyInfo* zonePropertyInfo = CZonePropertyInfo::GetInstance();
//		recordset.MoveFirst();
//		for (DWORD i = 0; i < count; i++) {
//			int id, zone_property;
//			CString zone_property_text, zone_alarm_text;
//			recordset.GetFieldValue(L"id", id);
//			recordset.GetFieldValue(L"zone_property", zone_property);
//			recordset.GetFieldValue(L"zone_property_text", zone_property_text);
//			recordset.GetFieldValue(L"zone_alarm_text", zone_alarm_text);
//			recordset.MoveNext();
//
//			CZonePropertyData* data = new CZonePropertyData();
//			data->set_id(id);
//			data->set_property(zone_property);
//			data->set_property_text(zone_property_text);
//			data->set_alarm_text(zone_alarm_text);
//			zonePropertyInfo->AddZonePropertyData(data);
//		}
//	}
//	recordset.Close();
//}


int CAlarmMachineManager::GetMachineCount() const
{
#ifdef USE_ARRAY
	return m_validMachineCount;
#else
	return m_listAlarmMachine.size();
#endif
}


BOOL CAlarmMachineManager::GetMachine(int ademco_id, CAlarmMachine*& machine)
{
#ifdef USE_ARRAY
	if (0 <= ademco_id && ademco_id < MAX_MACHINE) {
		machine = m_alarmMachines[ademco_id];
		return (NULL != machine);
	}
#else
	std::list<CAlarmMachine*>::iterator iter = m_listAlarmMachine.begin();
	while (iter != m_listAlarmMachine.end()) {
		CAlarmMachine* local_machine = *iter++;
		if (local_machine->get_ademco_id() == ademco_id) {
			machine = local_machine;
			return TRUE;
		}
	}
#endif
	
	return FALSE;
}

BOOL CAlarmMachineManager::GetFirstMachine(CAlarmMachine*& machine)
{
#ifdef USE_ARRAY
	if (0 < m_validMachineCount) {
		m_curMachinePos = 0;
		machine = m_alarmMachines[m_curMachinePos++];
		return TRUE;
	}
#else
	if (0 < m_listAlarmMachine.size()) {
		m_curMachinePos = m_listAlarmMachine.begin();
		machine = *m_curMachinePos++;
		return TRUE;
	}
#endif
	
	return FALSE;
}


BOOL CAlarmMachineManager::GetNextMachine(CAlarmMachine*& machine)
{
#ifdef USE_ARRAY
	if (0 < m_validMachineCount && m_validMachineCount < MAX_MACHINE) {
		m_curMachinePos = 0;
		machine = m_alarmMachines[m_curMachinePos++];
		return TRUE;
	}
#else
	if (0 < m_listAlarmMachine.size() && m_curMachinePos != m_listAlarmMachine.end()) {
		machine = *m_curMachinePos++;
		return TRUE;
	}
#endif
	
	return FALSE;
}


BOOL CAlarmMachineManager::CheckIsValidMachine(int ademco_id, const char* device_id, int zone)
{
	if (ademco_id < 0 || MAX_MACHINE <= ademco_id) {
		return FALSE;
	}

	if (zone < 0 || MAX_MACHINE_ZONE < zone) {
		return FALSE;
	}

#ifdef USE_ARRAY
	CAlarmMachine* machine = m_alarmMachines[ademco_id];
	if (NULL != machine) {
		if (strcmp(machine->GetDeviceIDA(), device_id) == 0) {
			if (!machine->get_banned()) {
				return TRUE;
			}
		}
	}
#else
	std::list<CAlarmMachine*>::iterator iter = m_listAlarmMachine.begin();
	while (iter != m_listAlarmMachine.end()) {
		CAlarmMachine* machine = *iter++;
		if ((machine->get_ademco_id() == ademco_id)) {
			if (strcmp(machine->GetDeviceIDA(), device_id) == 0) {
				if (machine->get_banned()) {

					break;
				}
				return TRUE;
			}
			break;
		}
	}
#endif
	

	return FALSE;
}


BOOL CAlarmMachineManager::CheckIfMachineAdemcoIdCanUse(int ademco_id)
{
	if (ademco_id < 0 || MAX_MACHINE <= ademco_id) {
		return FALSE;
	}

	CAlarmMachine* machine = m_alarmMachines[ademco_id];
	if (NULL != machine) {
		return FALSE;
	}

	return TRUE;
}


BOOL CAlarmMachineManager::CheckIfMachineAcctAlreadyInuse(const char* device_id)
{
#ifdef USE_ARRAY
	for (int i = 0; i < MAX_MACHINE; i++) {
		CAlarmMachine* machine = m_alarmMachines[i];
		if(machine){
			if (strcmp(machine->GetDeviceIDA(), device_id) == 0) {
				return TRUE;
			}
		}
	}
#else
	std::list<CAlarmMachine*>::iterator iter = m_listAlarmMachine.begin();
	while (iter != m_listAlarmMachine.end()) {
		CAlarmMachine* machine = *iter++;
		if (strcmp(machine->GetDeviceIDA(), device_id) == 0) {
			return TRUE;
		}
	}
#endif
	
	return FALSE;
}


BOOL CAlarmMachineManager::CheckIfMachineAcctAlreadyInuse(const wchar_t* device_id)
{
#ifdef USE_ARRAY
	for (int i = 0; i < MAX_MACHINE; i++) {
		CAlarmMachine* machine = m_alarmMachines[i];
		if (machine) {
			if (wcscmp(machine->GetDeviceIDW(), device_id) == 0) {
				return TRUE;
			}
		}
	}
#else
	std::list<CAlarmMachine*>::iterator iter = m_listAlarmMachine.begin();
	while (iter != m_listAlarmMachine.end()) {
		CAlarmMachine* machine = *iter++;
		if (wcscmp(machine->GetDeviceIDW(), device_id) == 0) {
			return TRUE;
		}
	}
#endif

	return FALSE;
}


BOOL CAlarmMachineManager::DistributeAdemcoID(int& ademco_id)
{
	BOOL ok = FALSE;
#ifdef USE_ARRAY
	if (m_validMachineCount >= MAX_MACHINE) {
		return FALSE;
	}
	
	for (int i = 0; i < MAX_MACHINE; i++) {
		CAlarmMachine* machine = m_alarmMachines[i];
		if (NULL == machine) {
			ok = TRUE;
			ademco_id = i;
			break;
		}
	}
#else
	if (m_listAlarmMachine.size() >= MAX_MACHINE) {
		return FALSE;
	}
	
	CAlarmMachine* machine = m_listAlarmMachine.back();
	if (machine->get_ademco_id() >= MAX_MACHINE - 1) {
		int temp_id = 0;
		std::list<CAlarmMachine*>::iterator iter = m_listAlarmMachine.begin();
		while (iter != m_listAlarmMachine.end()) {
			CAlarmMachine* machine = *iter++;
			if (machine->get_ademco_id() != temp_id++) {
				ademco_id = temp_id;
				ok = TRUE;
				break;
			}
		}
	} else {
		ademco_id = machine->get_ademco_id() + 1;
		ok = TRUE;
	}
#endif
	

	return ok;
}


BOOL CAlarmMachineManager::AddMachine(int ademco_id, 
									  const char* device_id, 
									  const wchar_t* alias)
{
	CAlarmMachine* machine = new CAlarmMachine();
	machine->set_ademco_id(ademco_id);
	machine->set_device_id(device_id);
	machine->set_alias(alias);


	// todo: 写数据库

	return TRUE;
}


BOOL CAlarmMachineManager::AddMachine(CAlarmMachine* machine)
{
	int ademco_id = machine->get_ademco_id();
	if (ademco_id < 0 || MAX_MACHINE <= ademco_id) {
		return FALSE;
	}

	m_lock4Machines.Lock();
	CString query;
	query.Format(L"insert into [AlarmMachine] ([ademco_id],[device_id],[banned],[machine_type],[has_video],[alias],[contact],[address],[phone],[phone_bk],[group_id],[expire_time]) values(%d,'%s',%d,%d,%d,'%s','%s','%s','%s','%s',%d,'%s')",
				 ademco_id, machine->GetDeviceIDW(), machine->get_banned(),
				 machine->get_machine_type(), machine->get_has_video(), 
				 machine->get_alias(), machine->get_contact(),
				 machine->get_address(), machine->get_phone(), 
				 machine->get_phone_bk(), machine->get_group_id(),
				 machine->get_expire_time().Format(L"%Y-%m-%d %H:%M:%S"));
	int id = AddAutoIndexTableReturnID(query);
	if (-1 == id) {
		m_lock4Machines.UnLock();
		return FALSE;
	}

	machine->set_id(id);
	//CMapInfo* mapInfo = new CMapInfo();
	//mapInfo->set_id(-1);
	//CString fmAlias;
	//fmAlias.LoadStringW(IDS_STRING_NOZONEMAP);
	//mapInfo->set_alias(fmAlias);
	//machine->SetUnbindZoneMap(mapInfo);

#ifdef USE_ARRAY
	m_alarmMachines[ademco_id] = machine;
	m_validMachineCount++;
#else
	m_listAlarmMachine.push_back(machine);
#endif
	m_lock4Machines.UnLock();
	return TRUE;
}


BOOL CAlarmMachineManager::DeleteMachine(CAlarmMachine* machine)
{
	int ademco_id = machine->get_ademco_id();
	if (ademco_id < 0 || MAX_MACHINE <= ademco_id) {
		return FALSE;
	}

	m_lock4Machines.Lock();
	CString query;
	query.Format(L"delete from AlarmMachine where id=%d and ademco_id=%d",
				 machine->get_id(), machine->get_ademco_id());
	if (m_pDatabase->Execute(query)) {
		// delete all zone & detector info of machine
		std::list<CZoneInfo*> zoneList;
		machine->GetAllZoneInfo(zoneList);
		std::list<CZoneInfo*>::iterator zoneIter = zoneList.begin();
		while (zoneIter != zoneList.end()) {
			CZoneInfo* zone = *zoneIter++;
			int detector_id = zone->get_detector_id();
			if (-1 != detector_id) {
				query.Format(L"delete from DetectorInfo where id=%d", detector_id);
				VERIFY(m_pDatabase->Execute(query));
			}
		}

		query.Format(L"delete from ZoneInfo where ademco_id=%d", machine->get_ademco_id());
		VERIFY(m_pDatabase->Execute(query));

		query.Format(L"delete from MapInfo where machine_id=%d and type=%d", 
					 machine->get_ademco_id(), MAP_MACHINE);
		VERIFY(m_pDatabase->Execute(query));

		CGroupInfo* group = CGroupManager::GetInstance()->GetGroupInfo(machine->get_group_id());
		group->RemoveChildMachine(machine); delete machine;
		m_alarmMachines[ademco_id] = NULL; m_validMachineCount--;
		m_lock4Machines.UnLock();
		return TRUE;
	}
	m_lock4Machines.UnLock();
	return FALSE;
}


BOOL CAlarmMachineManager::DeleteSubMachine(CZoneInfo* zoneInfo)
{
	ASSERT(zoneInfo);
	CAlarmMachine* subMachine = zoneInfo->GetSubMachineInfo();
	ASSERT(subMachine);

	CString query;
	query.Format(L"delete from SubMachine where id=%d",
				 subMachine->get_id());
	LOG(L"%s\n", query);
	VERIFY(m_pDatabase->Execute(query));

	// delete all zone & detector info of machine
	std::list<CZoneInfo*> zoneList;
	subMachine->GetAllZoneInfo(zoneList);
	std::list<CZoneInfo*>::iterator zoneIter = zoneList.begin();
	while (zoneIter != zoneList.end()) {
		CZoneInfo* zone = *zoneIter++;
		int detector_id = zone->get_detector_id();
		if (-1 != detector_id) {
			query.Format(L"delete from DetectorInfo where id=%d", detector_id);
			LOG(L"%s\n", query);
			VERIFY(m_pDatabase->Execute(query));
		}
	}

	query.Format(L"delete from SubZone where sub_machine_id=%d",
				 subMachine->get_id());
	LOG(L"%s\n", query);
	VERIFY(m_pDatabase->Execute(query));

	query.Format(L"delete from MapInfo where machine_id=%d and type=%d",
				 subMachine->get_id(), MAP_SUB_MACHINE);
	LOG(L"%s\n", query);
	VERIFY(m_pDatabase->Execute(query));

	query.Format(L"update ZoneInfo set type=%d,sub_machine_id=-1 where id=%d",
				 ZT_ZONE, zoneInfo->get_id());
	LOG(L"%s\n", query);
	VERIFY(m_pDatabase->Execute(query));

	return TRUE;
}


void CAlarmMachineManager::MachineEventHandler(int ademco_id, int ademco_event, 
											   int zone, int subzone, 
											   const time_t& event_time,
											   const char* xdata, int xdata_len)
{
	AUTO_LOG_FUNCTION;
	CAlarmMachine* machine = NULL;
	if (GetMachine(ademco_id, machine) && machine) {
		machine->SetAdemcoEvent(ademco_event, zone, subzone, event_time, xdata, xdata_len);
	}
		/*switch (ademco_event) {	
			case ademco::EVENT_ARM:
				machine->SetStatus(zone, ademco_event);
				break;
			case ademco::EVENT_DISARM:
				machine->SetStatus(zone, MS_DISARM);
				break;
			case ademco::EVENT_HALFARM:
				machine->SetStatus(zone, MS_HALFARM);
				break;
			case ademco::EVENT_EMERGENCY:
				machine->SetStatus(zone, MS_EMERGENCY);
				break;
			case ademco::EVENT_BURGLAR:
				machine->SetStatus(zone, MS_BUGLAR);
				break;
			case ademco::EVENT_FIRE:
				machine->SetStatus(zone, MS_FIRE);
				break;
			case ademco::EVENT_DURESS:
				machine->SetStatus(zone, MS_DURESS);
				break;
			case ademco::EVENT_GAS:
				machine->SetStatus(zone, MS_GAS);
				break;
			case ademco::EVENT_WATER:
				machine->SetStatus(zone, MS_WATER);
				break;
			case ademco::EVENT_TEMPER:
				machine->SetStatus(zone, MS_TEMPER);
				break;
			case ademco::EVENT_LOWBATTERY:
				machine->SetStatus(zone, MS_LOWBATTERY);
				break;
			case ademco::EVENT_SOLARDISTURB:
				machine->SetStatus(zone, MS_SOLARDISTURB);
				break;
			case ademco::EVENT_DISCONNECT:
				machine->SetStatus(zone, MS_DISCONNECT);
				break;
			case ademco::EVENT_SERIAL485DIS:
				machine->SetStatus(zone, MS_SERIAL485DIS);
				break;
			case ademco::EVENT_SERIAL485CONN:
				machine->SetStatus(zone, MS_SERIAL485CONN);
				break;
			case ademco::EVENT_DOORRINGING:
				machine->SetStatus(zone, MS_DOORRINGING);
				break;
			default:
				machine->SetStatus(zone, MS_ONLINE);
				break;
		}
	}
	//zone;
	*/
}


void CAlarmMachineManager::MachineOnline(int ademco_id, BOOL online, const char* ipv4, 
										 void* udata, ConnHangupCB cb)
{
	AUTO_LOG_FUNCTION;
	CAlarmMachine* machine = NULL;
	if (GetMachine(ademco_id, machine) && machine) {
		time_t event_time = time(NULL);
		machine->SetAdemcoEvent(online ? EVENT_ONLINE : EVENT_OFFLINE, 0, 0, event_time, NULL, 0);
		if (online && udata && cb) {
			machine->SetConnHangupCallback(udata, cb);
		}
		machine->set_ipv4(ipv4);
	}
}


BOOL CAlarmMachineManager::RemoteControlAlarmMachine(const CAlarmMachine* machine, 
													 int ademco_event, int gg, int zone, 
													 const char* xdata, int xdata_len, 
													 CWnd* pWnd)
{
	assert(machine);
	//char xdata[64] = { 0 };
	//int xdata_len = 0;
	if (ademco_event == ademco::EVENT_DISARM) {
		m_pPrevCallDisarmWnd = pWnd;
		m_prevCallDisarmAdemcoID = machine->get_ademco_id();
		m_prevCallDisarmGG = gg;
		m_prevCallDisarmZoneValue = zone;
	}

	CString srecord, suser, sfm, sop, spost, fmMachine, fmSubmachine;
	suser.LoadStringW(IDS_STRING_USER);
	sfm.LoadStringW(IDS_STRING_LOCAL_OP);
	fmMachine.LoadStringW(IDS_STRING_MACHINE);
	fmSubmachine.LoadStringW(IDS_STRING_SUBMACHINE);
	switch (ademco_event) {
		case EVENT_ARM:
			sop.LoadStringW(IDS_STRING_ARM);
			break;
		case EVENT_DISARM:
			sop.LoadStringW(IDS_STRING_DISARM);
			break;
		case EVENT_EMERGENCY:
			sop.LoadStringW(IDS_STRING_EMERGENCY);
			break;
		case EVENT_QUERY_SUB_MACHINE:
			sop.LoadStringW(IDS_STRING_QUERY);
			break;
		case EVENT_RETRIEVE_SUB_MACHINE:
			sop.LoadStringW(IDS_STRING_RETRIEVE);
			break;
		case EVENT_WRITE_TO_MACHINE: {
			sop.LoadStringW(IDS_STRING_WRITE2MACHINE);
			CString s, szone; szone.LoadStringW(IDS_STRING_ZONE);
			s.Format(L"(%s%03d)", szone, zone);
			sop += s;
		}
			break;
		default:
			assert(0);
			break;
	}
	const CUserInfo* user = CUserManager::GetInstance()->GetCurUserInfo();
	srecord.Format(L"%s(ID:%d,%s)%s:%s", suser,
				   user->get_user_id(), user->get_user_name(),
				   sfm, sop);

	if (machine->get_is_submachine()) {
		CAlarmMachine* netMachine = NULL;
		if (GetMachine(machine->get_ademco_id(), netMachine)) {
			spost.Format(L" %s%04d(%s)%s%03d(%s)", fmMachine, 
						 machine->get_ademco_id(),
						 netMachine->get_alias(),
						 fmSubmachine, 
						 machine->get_submachine_zone(), 
						 machine->get_alias());
		}
	} else {
		spost.Format(L" %s%04d(%s)", fmMachine, machine->get_ademco_id(), 
					 machine->get_alias());
	}
	srecord += spost;
	CHistoryRecord::GetInstance()->InsertRecord(machine->get_ademco_id(), 
												zone, srecord, time(NULL),
												RECORD_LEVEL_USERCONTROL);

	return net::CNetworkConnector::GetInstance()->Send(machine->get_ademco_id(),
													   ademco_event, gg, zone,
													   xdata, xdata_len);
}


void CAlarmMachineManager::DisarmPasswdWrong(int ademco_id)
{
	CString spasswdwrong;
	spasswdwrong.LoadStringW(IDS_STRING_USER_PASSWD_WRONG);
	CHistoryRecord::GetInstance()->InsertRecord(ademco_id, m_prevCallDisarmZoneValue,
												spasswdwrong, time(NULL),
												RECORD_LEVEL_USERCONTROL);
	if (m_prevCallDisarmAdemcoID != ademco_id)
		return;

	char xdata[64] = { 0 };
	int xdata_len = 0;
	CInputDlg dlg(m_pPrevCallDisarmWnd);
	if (dlg.DoModal() != IDOK)
		return;
	if (dlg.m_edit.GetLength() != 6)
		return;

	CString srecord, suser, sfm, sop, snull;
	suser.LoadStringW(IDS_STRING_USER);
	sfm.LoadStringW(IDS_STRING_LOCAL_OP);
	sop.LoadStringW(IDS_STRING_DISARM);
	snull.LoadStringW(IDS_STRING_NULL);
	
	const CUserInfo* user = CUserManager::GetInstance()->GetCurUserInfo();
	CAlarmMachine* machine = NULL;
	GetMachine(ademco_id, machine);
	srecord.Format(L"%s(ID:%d,%s)%s:%s%04d(%s)", suser,
				   user->get_user_id(), user->get_user_name(),
				   sfm, sop, ademco_id, machine ? machine->get_alias() : snull);
	CHistoryRecord::GetInstance()->InsertRecord(machine->get_ademco_id(),
												m_prevCallDisarmZoneValue,
												srecord, time(NULL),
												RECORD_LEVEL_USERCONTROL);

	USES_CONVERSION;
	strcpy_s(xdata, W2A(dlg.m_edit));
	xdata_len = strlen(xdata);
	net::CNetworkConnector::GetInstance()->Send(ademco_id, 
												ademco::EVENT_DISARM, 
												m_prevCallDisarmGG, 
												m_prevCallDisarmZoneValue, 
												xdata, xdata_len);
}


void CAlarmMachineManager::EnterEditMode()
{
	AUTO_LOG_FUNCTION;
#ifdef USE_ARRAY
	for (int i = 0; i < MAX_MACHINE; i++) {
		CAlarmMachine* machine = m_alarmMachines[i];
		if (NULL != machine) {
			machine->EnterBufferMode();
		}
	}
#else
	std::list<CAlarmMachine*>::iterator iter = m_listAlarmMachine.begin();
	while (iter != m_listAlarmMachine.end()) {
		CAlarmMachine* machine = *iter++;
		machine->EnterBufferMode();
	}
#endif
	
}


void CAlarmMachineManager::LeaveEditMode()
{
	AUTO_LOG_FUNCTION;
#ifdef USE_ARRAY
	for (int i = 0; i < MAX_MACHINE; i++) {
		CAlarmMachine* machine = m_alarmMachines[i];
		if (NULL != machine) {
			machine->LeaveBufferMode();
		}
	}
#else
	std::list<CAlarmMachine*>::iterator iter = m_listAlarmMachine.begin();
	while (iter != m_listAlarmMachine.end()) {
		CAlarmMachine* machine = *iter++;
		machine->LeaveBufferMode();
	}
#endif
}


void __stdcall CAlarmMachineManager::OnOtherCallEnterBufferMode(void* udata)
{
	AUTO_LOG_FUNCTION;
	CAlarmMachineManager* mgr = reinterpret_cast<CAlarmMachineManager*>(udata);
	SetEvent(mgr->m_hEventOotebm);
}
//m_hEventOotebm

DWORD WINAPI CAlarmMachineManager::ThreadCheckSubMachine(LPVOID lp)
{
	AUTO_LOG_FUNCTION;
	CAlarmMachineManager* mgr = reinterpret_cast<CAlarmMachineManager*>(lp);
	while (1) {
		if (WAIT_OBJECT_0 == WaitForSingleObject(mgr->m_hEventExit, CHECK_GAP))
			break;
		if (mgr->m_validMachineCount == 0)
			continue;
		
		CLocalLock lock(mgr->m_lock4Machines.GetLockObject());
		CAlarmMachineList *subMachineList = NULL;
		for (int i = 0; i < MAX_MACHINE; i++) {
			if (WAIT_OBJECT_0 == WaitForSingleObject(mgr->m_hEventExit, 0))
				break;
			CAlarmMachine* machine = mgr->m_alarmMachines[i];
			if (machine && machine->get_online() && machine->get_submachine_count() > 0) {
				if (!machine->EnterBufferMode()) {
					machine->SetOotebmObj(NULL, NULL);
					continue;
				}
				machine->SetOotebmObj(OnOtherCallEnterBufferMode, mgr);
				CZoneInfoList list;
				machine->GetAllZoneInfo(list);
				CZoneInfoListIter iter = list.begin();
				bool bAlreadyLeaveBuffMode = false;
				while (iter != list.end()) {
					if (WAIT_OBJECT_0 == WaitForSingleObject(mgr->m_hEventExit, 0))
						break;
					if (WAIT_OBJECT_0 == WaitForSingleObject(mgr->m_hEventOotebm, 0)) {
						machine->SetOotebmObj(NULL, NULL); 
						machine->LeaveBufferMode();
						bAlreadyLeaveBuffMode = true;
						break;
					}
					CZoneInfo* zoneInfo = *iter++;
					CAlarmMachine* subMachine = zoneInfo->GetSubMachineInfo();
					if (subMachine) {
						time_t lastActionTime = subMachine->GetLastActionTime();
						time_t check_time = time(NULL);
						if ((check_time - lastActionTime) * 1000 >= MAX_SUBMACHINE_ACTION_TIME_OUT) {
							if (subMachineList == NULL) {
								subMachineList = new CAlarmMachineList();
							}
							subMachineList->push_back(subMachine);
						}
					}
				}
				if (!bAlreadyLeaveBuffMode) {
					machine->SetOotebmObj(NULL, NULL);
					machine->LeaveBufferMode();
				}
			}
		}
		if (subMachineList && subMachineList->size() > 0) {
			CAlarmCenterApp* app = reinterpret_cast<CAlarmCenterApp*>(AfxGetApp());
			ASSERT(app);
			CAlarmCenterDlg* dlg = reinterpret_cast<CAlarmCenterDlg*>(app->GetMainWnd());
			ASSERT(dlg);
			dlg->SendMessage(WM_NEEDQUERYSUBMACHINE, 
							 (WPARAM)subMachineList, subMachineList->size());
		}
	}
	return 0;
}


DWORD WINAPI CAlarmMachineManager::ThreadChecker(LPVOID lp)
{
	AUTO_LOG_FUNCTION;
	CHECKER_PARAM* cp = reinterpret_cast<CHECKER_PARAM*>(lp); ASSERT(cp);
	CAlarmMachineManager* mgr = cp->mgr;
	int ademco_id = cp->ademco_id;
	int zone_value = cp->zone_value;
	delete cp;

	LOG(L"ademco_id %04d, zone_value %03d\n", ademco_id, zone_value);

	while (1) {
		if (WAIT_OBJECT_0 == WaitForSingleObject(mgr->m_hEventExit, 0))
			break;

	}
	/*if (subMachine->get_bChecking()) {
		lastActionTime = subMachine->GetLastActionTime();
		if ((time(NULL) - lastActionTime) * 1000 < MAX_SUBMACHINE_ACTION_TIME_OUT) {
			break;
		}
		if ((time(NULL) - check_time) == 20) {
			mgr->RemoteControlAlarmMachine(subMachine,
										   EVENT_QUERY_SUB_MACHINE,
										   INDEX_SUB_MACHINE,
										   subMachine->get_submachine_zone(),
										   NULL, 0, NULL);
		} else if ((time(NULL) - check_time) == 40) {
			mgr->RemoteControlAlarmMachine(subMachine,
										   EVENT_QUERY_SUB_MACHINE,
										   INDEX_SUB_MACHINE,
										   subMachine->get_submachine_zone(),
										   NULL, 0, NULL);
		} else if ((time(NULL) - check_time) * 1000 > ONE_MINUTE) {
			subMachine->set_bChecking(false);
			subMachine->set_online(false);
			subMachine->SetAdemcoEvent(EVENT_OFFLINE,
									   subMachine->get_submachine_zone(),
									   INDEX_SUB_MACHINE,
									   time(NULL), NULL, 0);
			break;
		}
	} else {
		subMachine->set_bChecking(true);
		mgr->RemoteControlAlarmMachine(subMachine,
									   EVENT_QUERY_SUB_MACHINE,
									   INDEX_SUB_MACHINE,
									   subMachine->get_submachine_zone(),
									   NULL, 0, NULL);
	}*/
	return 0;
}



NAMESPACE_END
