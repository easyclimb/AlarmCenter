﻿#include "stdafx.h"
#include "AlarmMachineManager.h"
#include "DbOper.h"
#include "AlarmMachine.h"
#include "AppResource.h"

//#include "SubMachineInfo.h"
#include "ademco_func.h"
#include "MapInfo.h"
#include "ZoneInfo.h"
#include "CameraInfo.h"
#include "DetectorInfo.h"
#include "DetectorLib.h"
#include "ConfigHelper.h"
#include "NetworkConnector.h"
//#include "InputDlg.h"
#include "UserInfo.h"
#include "HistoryRecord.h"
#include "GroupInfo.h"
#include "AlarmCenter.h"
#include "AlarmCenterDlg.h"
#include "baidu.h"
#include "CsrInfo.h"
#include "Sms.h"
#include "VideoManager.h"

#include <memory> // for std::shared_ptr
#include <algorithm>
#include "InputDlg.h" // todo: remove this

namespace core {


static const int ONE_MINUTE = 60 * 1000;
static const int ONE_HOUR = 60 * ONE_MINUTE;
//#ifdef _DEBUG
//static const int MAX_SUBMACHINE_ACTION_TIME_OUT = 16 * ONE_MINUTE;
//static const int CHECK_GAP = ONE_MINUTE;
//static const int TRY_LOCK_RETRY_GAP = ONE_MINUTE;
//static const int WAIT_TIME_FOR_RETRIEVE_RESPONCE = 3000;
//#else
static const int MAX_SUBMACHINE_ACTION_TIME_OUT = 16 * ONE_HOUR; // 16 hour
static const int CHECK_GAP = 16 * ONE_HOUR;
static const int TRY_LOCK_RETRY_GAP = ONE_MINUTE;
static const int WAIT_TIME_FOR_RETRIEVE_RESPONCE = ONE_MINUTE;
//#endif

IMPLEMENT_SINGLETON(CAlarmMachineManager)

CAlarmMachineManager::CAlarmMachineManager()
	:/* m_rootGroupInfo(nullptr)
	 , */m_db(nullptr)
	, m_pPrevCallDisarmWnd(nullptr)
	, m_prevCallDisarmAdemcoID(-1)
	, m_prevCallDisarmGG(-1)
	, m_prevCallDisarmZoneValue(-1)
#ifdef USE_ARRAY
	, m_curMachinePos(0)
	, m_validMachineCount(0)
#endif
	, m_hThread(INVALID_HANDLE_VALUE)
	, m_hEventExit(INVALID_HANDLE_VALUE)
	, m_hEventOotebm(INVALID_HANDLE_VALUE)
{
	AUTO_LOG_FUNCTION;
#ifdef USE_ARRAY
	//size_t sss = sizeof(m_alarmMachines);
	memset(m_alarmMachines, 0, sizeof(m_alarmMachines));
#endif
	
	m_hEventExit = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	m_hEventOotebm = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	m_hThread = CreateThread(nullptr, 0, ThreadCheckSubMachine, this, 0, nullptr);
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

	m_machineMap.clear();
	for (auto iter : m_cameraMap) {
		iter.second.clear();
	}
	m_cameraMap.clear();

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

	LoadCameraInfoFromDB();
}


void CAlarmMachineManager::InitCsrInfo()
{
	CString query = L"select * from CsrInfo";
	ado::CADORecordset recordset(m_db->GetDatabase());
	recordset.Open(m_db->GetDatabase()->m_pConnection, query);
	DWORD count = recordset.GetRecordCount();
	if (count == 1) {
		CString acct, addr; int city_code, zoomLevel; double x, y;
		recordset.MoveFirst();
		recordset.GetFieldValue(L"CsrAcct", acct);
		recordset.GetFieldValue(L"CsrAddress", addr);
		recordset.GetFieldValue(L"CsrCitycode", city_code);
		recordset.GetFieldValue(L"CsrBaiduMapX", x);
		recordset.GetFieldValue(L"CsrBaiduMapY", y);
		recordset.GetFieldValue(L"ZoomLevel", zoomLevel);
		CCsrInfo* csr = CCsrInfo::GetInstance();
		//csr->set_acct(acct);
		csr->set_addr(addr);
		csr->set_city_code(city_code);
		csr->set_coor(web::BaiduCoordinate(x, y));
		csr->set_level(zoomLevel);
	}
	recordset.Close();
}


BOOL CAlarmMachineManager::ExecuteSql(const CString& query)
{
	return m_db->Execute(query);
}


int CAlarmMachineManager::AddAutoIndexTableReturnID(const CString& query)
{
	return m_db->AddAutoIndexTableReturnID(query);
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
	m_db = std::make_shared<ado::CDbOper>();
	m_db->Open(L"AlarmCenter.mdb");		
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
	CString query = _T("select * from DetectorLib order by id");
	std::shared_ptr<ado::CADORecordset> pDataGridRecord(new ado::CADORecordset(m_db->GetDatabase()));
	pDataGridRecord->Open(m_db->GetDatabase()->m_pConnection, query);
	CLog::WriteLog(_T("pDataGridRecord->Open(m_db->GetDatabase()->m_pConnection 0x%x, %s)"),
				   m_db->GetDatabase()->m_pConnection, query);
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

		CString format;
		format = L"insert into DetectorLib ([type],[detector_name],[path],[path_pair],[antline_num],[antline_gap]) values(%d,'%s','%s','%s',%d,%d)";
		
		// A2
		query.Format(format, DT_DOUBLE, _T("A2"), detPath + _T("A2.bmp"), 
					 detPath + L"A2Receiver.bmp", ALN_2, ALG_12);
		VERIFY(m_db->GetDatabase()->Execute(query));
		
		// A4
		query.Format(format, DT_DOUBLE, _T("A4"), detPath + _T("A4.bmp"),
					 detPath + L"A4Receiver.bmp", ALN_4, ALG_12);
		VERIFY(m_db->GetDatabase()->Execute(query));

		// A8
		query.Format(format, DT_DOUBLE, _T("A8"), detPath + _T("A8.bmp"),
					 detPath + L"A8Receiver.bmp", ALN_8, ALG_12);
		VERIFY(m_db->GetDatabase()->Execute(query));

		// R2
		query.Format(format, DT_DOUBLE, _T("R2"), detPath + _T("R2.bmp"),
					 detPath + L"R2Receiver.bmp", ALN_2, ALG_12);
		VERIFY(m_db->GetDatabase()->Execute(query));

		// R3
		query.Format(format, DT_DOUBLE, _T("R3"), detPath + _T("R3.bmp"),
					 detPath + L"R3Receiver.bmp", ALN_3, ALG_12);
		VERIFY(m_db->GetDatabase()->Execute(query));

		// R4
		query.Format(format, DT_DOUBLE, _T("R4"), detPath + _T("R4.bmp"),
					 detPath + L"R4Receiver.bmp", ALN_4, ALG_12);
		VERIFY(m_db->GetDatabase()->Execute(query));

		// R6
		query.Format(format, DT_DOUBLE, _T("R6"), detPath + _T("R6.bmp"),
					 detPath + L"R6Receiver.bmp", ALN_6, ALG_12);
		VERIFY(m_db->GetDatabase()->Execute(query));

		// R8
		query.Format(format, DT_DOUBLE, _T("R8"), detPath + _T("R8.bmp"),
					 detPath + L"R8Receiver.bmp", ALN_8, ALG_16);
		VERIFY(m_db->GetDatabase()->Execute(query));

		// S4
		query.Format(format, DT_DOUBLE, _T("S4"), detPath + _T("S4.bmp"),
					 detPath + L"S4Receiver.bmp", ALN_4, ALG_12);
		VERIFY(m_db->GetDatabase()->Execute(query));

		// S4-D
		query.Format(format, DT_DOUBLE, _T("S4-D"), detPath + _T("S4-D.bmp"),
					 detPath + L"S4-DReceiver.bmp", ALN_4, ALG_12);
		VERIFY(m_db->GetDatabase()->Execute(query));

		// S8
		query.Format(format, DT_DOUBLE, _T("S8"), detPath + _T("S8.bmp"),
					 detPath + L"S8Receiver.bmp", ALN_8, ALG_14);
		VERIFY(m_db->GetDatabase()->Execute(query));

		// S8-D
		query.Format(format, DT_DOUBLE, _T("S8-D"), detPath + _T("S8-D.bmp"),
					 detPath + L"S8-DReceiver.bmp", ALN_8, ALG_14);
		VERIFY(m_db->GetDatabase()->Execute(query));

		// T205
		query.Format(format, DT_SINGLE, _T("T205"), detPath + _T("T205.bmp"),
					 L"", ALN_0, ALG_0);
		VERIFY(m_db->GetDatabase()->Execute(query));

		// JHD-2
		query.Format(format, DT_SINGLE, _T("JHD-2"), detPath + _T("JHD-2.bmp"),
					 L"", ALN_0, ALG_0);
		VERIFY(m_db->GetDatabase()->Execute(query));

		// T201
		query.Format(format, DT_SINGLE, _T("T201"), detPath + _T("T201.bmp"),
					 L"", ALN_0, ALG_0);
		VERIFY(m_db->GetDatabase()->Execute(query));

		// T601
		query.Format(format, DT_SINGLE, _T("T601"), detPath + _T("T601.bmp"),
					 L"", ALN_0, ALG_0);
		VERIFY(m_db->GetDatabase()->Execute(query));

		// 无线门磁
		query.Format(format, DT_SINGLE, 
					 TRIPLE_CONDITION(condition, _T("无线门磁"), _T("無線門磁"), _T("WirelessDoorSensor")), 
					 detPath + _T("WirelessDoorSensor.bmp"), L"", ALN_0, ALG_0);
		VERIFY(m_db->GetDatabase()->Execute(query));

		// 紧急按钮HB-A380
		query.Format(format, DT_SINGLE,
					 TRIPLE_CONDITION(condition, _T("紧急按钮HB-A380"), _T("緊急按鈕HB-A380"), _T("EmergencyButtonHB-A380")),
					 detPath + _T("EmergencyButtonHB-A380.bmp"), L"", ALN_0, ALG_0);
		VERIFY(m_db->GetDatabase()->Execute(query));

		// 卧室主机HB-3030C
		query.Format(format, DT_SUB_MACHINE,
					 TRIPLE_CONDITION(condition, _T("卧室主机HB-3030C"), _T("臥室主機HB-3030C"), _T("HB-3030C")),
					 detPath + _T("HB-3030C.bmp"), L"", ALN_0, ALG_0);
		VERIFY(m_db->GetDatabase()->Execute(query));

		// 液晶主机HB-BJQ-560
		query.Format(format, DT_SUB_MACHINE,
					 TRIPLE_CONDITION(condition, _T("液晶主机HB-BJQ-560"), _T("液晶主機HB-BJQ-560"), _T("HB-BJQ-560")),
					 detPath + _T("HB-BJQ-560.bmp"), L"", ALN_0, ALG_0);
		VERIFY(m_db->GetDatabase()->Execute(query));

		// 摄像机
		query.Format(format, DT_CAMERA,
					 TRIPLE_CONDITION(condition, _T("摄像机"), _T("摄像机"), _T("Camera")),
					 detPath + _T("camera_72px.bmp"), L"", ALN_0, ALG_0);
		VERIFY(m_db->GetDatabase()->Execute(query));
	}
	CLog::WriteLog(_T("CDBOper::InitData() ok"));
}


void CAlarmMachineManager::LoadGroupInfoFromDB()
{
	static const wchar_t* query = L"select * from GroupInfo order by parent_id";
	ado::CADORecordset recordset(m_db->GetDatabase());
	recordset.Open(m_db->GetDatabase()->m_pConnection, query);
	DWORD count = recordset.GetRecordCount();
	if (count > 0) {
		recordset.MoveFirst();
		CGroupManager* mgr = CGroupManager::GetInstance();
		std::list<CGroupInfoPtr> unresolvedGroupList;
		for (DWORD i = 0; i < count; i++) {
			int id, parent_id;
			CString name;
			recordset.GetFieldValue(L"id", id);
			recordset.GetFieldValue(L"parent_id", parent_id);
			recordset.GetFieldValue(L"group_name", name);
			recordset.MoveNext();
			
			if (id == 1 && parent_id == 0) {
				mgr->_tree->set_id(1);
				mgr->_tree->set_parent_id(0);
				CString rootName;
				rootName = GetStringFromAppResource(IDS_STRING_GROUP_ROOT);
				mgr->_tree->set_name(rootName);
			} else {
				auto group = std::make_shared<CGroupInfo>();
				group->set_id(id);
				group->set_parent_id(parent_id);
				group->set_name(name);
				//mgr->_groupList.push_back(group);
				//m_listGroupInfo.push_back(group);
				bool ok = mgr->_tree->AddChildGroup(group);
				if (!ok) {
					unresolvedGroupList.push_back(group);
				}
			}
		}

		for (auto group : unresolvedGroupList) {
			bool ok = mgr->_tree->AddChildGroup(group);
			VERIFY(ok);
		}
	}
	recordset.Close();
}


void CAlarmMachineManager::LoadAlarmMachineFromDB(void* udata, LoadDBProgressCB cb)
{
	AUTO_LOG_FUNCTION;
	static const wchar_t* query = L"select * from AlarmMachine order by ademco_id";
	ado::CADORecordset recordset(m_db->GetDatabase());
	JLOG(L"CADORecordset recordset %p\n", &recordset);
	BOOL ret = recordset.Open(m_db->GetDatabase()->m_pConnection, query);
	VERIFY(ret); JLOG(L"recordset.Open() return %d\n", ret);
	DWORD count = recordset.GetRecordCount();
	
	JLOG(L"recordset.GetRecordCount() return %d\n", count);
	CSms* sms = CSms::GetInstance();
	if (count > 0) {
		CGroupManager* mgr = CGroupManager::GetInstance();
		CString null;
		null = GetStringFromAppResource(IDS_STRING_NULL);
		recordset.MoveFirst();
		for (DWORD i = 0; i < count; i++) {
			int id, ademco_id, group_id, banned, type, has_video, status;
			CString /*device_id, */alias, contact, address, phone, phone_bk;
			COleDateTime expire_time;
			double x, y;
			recordset.GetFieldValue(L"id", id);
			recordset.GetFieldValue(L"ademco_id", ademco_id);
			if (ademco_id < 0 || MAX_MACHINE <= ademco_id)
				continue;
			//recordset.GetFieldValue(L"device_id", device_id);
			recordset.GetFieldValue(L"machine_type", type);
			recordset.GetFieldValue(L"banned", banned); 
			recordset.GetFieldValue(L"banned", has_video);
			recordset.GetFieldValue(L"machine_status", status);
			recordset.GetFieldValue(L"alias", alias);
			if (alias == null) { alias.Empty(); }
			recordset.GetFieldValue(L"contact", contact);
			if (contact == null) { contact.Empty(); }
			recordset.GetFieldValue(L"address", address);
			if (address == null) { address.Empty(); }
			recordset.GetFieldValue(L"phone", phone);
			if (phone == null) { phone.Empty(); }
			recordset.GetFieldValue(L"phone_bk", phone_bk);
			if (phone_bk == null) { phone_bk.Empty(); }
			recordset.GetFieldValue(L"expire_time", expire_time);
			recordset.GetFieldValue(L"group_id", group_id);
			recordset.GetFieldValue(L"baidu_x", x);
			recordset.GetFieldValue(L"baidu_y", y);
			recordset.MoveNext();

			auto machine = std::make_shared<CAlarmMachine>();
			machine->set_id(id);
			machine->set_ademco_id(ademco_id);
			//machine->set_device_id(device_id);
			machine->set_machine_type(Integer2MachineType(type));
			machine->set_banned(banned != 0);
			machine->set_has_video(has_video != 0);
			machine->set_machine_status(Integer2MachineStatus(status));
			machine->set_alias((LPCTSTR)alias);
			machine->set_contact((LPCTSTR)contact);
			machine->set_address((LPCTSTR)address);
			machine->set_phone((LPCTSTR)phone);
			machine->set_phone_bk((LPCTSTR)phone_bk);
			machine->set_group_id(group_id);
			if (expire_time.GetStatus() == COleDateTime::invalid) {
				expire_time = COleDateTime::GetCurrentTime();
			}
			machine->set_expire_time(expire_time);
			machine->set_coor(web::BaiduCoordinate(x, y));
			
			SmsConfigure sms_cfg;
			if (sms->get_sms_config(machine->get_is_submachine(), ademco_id, machine->get_submachine_zone(), sms_cfg)) {
				machine->set_sms_cfg(sms_cfg);
			}else{
				sms->add_sms_config(machine->get_is_submachine(), ademco_id, machine->get_submachine_zone(), sms_cfg);
				machine->set_sms_cfg(sms_cfg);
			}

			machine->LoadXmlConfig();

			m_machineMap[ademco_id] = machine;

			if (cb && udata) {
				auto progress = std::make_shared<ProgressEx>();
				progress->main = true;
				progress->progress = static_cast<int>(i * MAX_MACHINE / count);
				progress->value = ademco_id;
				progress->total = count;
				cb(udata, progress);
			}

			LoadMapInfoFromDB(machine);
			//LoadUnbindZoneMapInfoFromDB(machine);
			LoadZoneInfoFromDB(machine, udata, cb);
			bool ok = mgr->_tree->AddChildMachine(machine);
			VERIFY(ok);
		}
	}
	recordset.Close();

	if (cb && udata) {
		auto progress = std::make_shared<ProgressEx>();
		progress->main = true;
		progress->progress = MAX_MACHINE;
		progress->value = MAX_MACHINE;
		progress->total = MAX_MACHINE;
		cb(udata, progress);
	}
}


#ifdef _DEBUG_TestLoadAlarmMachineFromDB
void CAlarmMachineManager::TestLoadAlarmMachineFromDB(void* udata, LoadDBProgressCB cb)
{
	AUTO_LOG_FUNCTION;
	static const wchar_t* query = L"select * from AlarmMachine order by ademco_id";
	ado::CADORecordset recordset_machine(m_db->GetDatabase());
	recordset_machine.Open(m_db->GetDatabase()->m_pConnection, query);
	DWORD machine_count = recordset_machine.GetRecordCount();
	ProgressEx progress;
	if (machine_count > 0) {
		CString null;
		null = GetStringFromAppResource(IDS_STRING_NULL);
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
				ado::CADORecordset recordset_map(m_db->GetDatabase());
				recordset_map.Open(m_db->GetDatabase()->m_pConnection, query);
				DWORD map_count = recordset_map.GetRecordCount();
				if (map_count > 0) {
					CString null;
					null = GetStringFromAppResource(IDS_STRING_NULL);
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
							ado::CADORecordset recordset_det(m_db->GetDatabase());
							recordset_det.Open(m_db->GetDatabase()->m_pConnection, query);
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
			ado::CADORecordset recordset_zone(m_db->GetDatabase());
			recordset_zone.Open(m_db->GetDatabase()->m_pConnection, query);
			DWORD count_zone = recordset_zone.GetRecordCount();
			if (count_zone > 0) {
				CString null;
				null = GetStringFromAppResource(IDS_STRING_NULL);
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
						ado::CADORecordset recordset_det2(m_db->GetDatabase());
						recordset_det2.Open(m_db->GetDatabase()->m_pConnection, query);
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
						ado::CADORecordset recordset_sub(m_db->GetDatabase());
						recordset_sub.Open(m_db->GetDatabase()->m_pConnection, query);
						DWORD count_sub = recordset_sub.GetRecordCount();
						if (count_sub == 1) {
							CString null;
							null = GetStringFromAppResource(IDS_STRING_NULL);
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
								ado::CADORecordset recordset_map2(m_db->GetDatabase());
								recordset_map2.Open(m_db->GetDatabase()->m_pConnection, query);
								DWORD map_count2 = recordset_map2.GetRecordCount();
								if (map_count2 > 0) {
									CString null;
									null = GetStringFromAppResource(IDS_STRING_NULL);
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
								ado::CADORecordset recordset_sub_zone(m_db->GetDatabase());
								recordset_sub_zone.Open(m_db->GetDatabase()->m_pConnection, query);
								DWORD count = recordset_sub_zone.GetRecordCount();
								if (count > 0) {
									CString null;
									null = GetStringFromAppResource(IDS_STRING_NULL);
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


void CAlarmMachineManager::LoadMapInfoFromDB(const core::CAlarmMachinePtr& machine)
{
	AUTO_LOG_FUNCTION;
	MapType mt = machine->get_is_submachine() ? MAP_SUB_MACHINE : MAP_MACHINE;
	CString query;
	query.Format(L"select * from MapInfo where type=%d and machine_id=%d order by id", mt, 
				 machine->get_is_submachine() ? machine->get_id() : machine->get_ademco_id());
	ado::CADORecordset recordset(m_db->GetDatabase());
	recordset.Open(m_db->GetDatabase()->m_pConnection, query);
	DWORD count = recordset.GetRecordCount();
	if (count > 0) {
		CString null;
		null = GetStringFromAppResource(IDS_STRING_NULL);
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

			CMapInfoPtr mapInfo = std::make_shared<CMapInfo>();
			mapInfo->set_id(id);
			mapInfo->set_type(type);
			mapInfo->set_machine_id(machine_id);
			mapInfo->set_alias(alias);
			mapInfo->set_path(path);
			//LoadZoneInfoFromDB(mapInfo);
			//LoadNoZoneHasMapDetectorInfoFromDB(mapInfo);
			machine->AddMap(mapInfo);
			m_mapList.push_back(mapInfo);
		}
	}
	recordset.Close();
}


void CAlarmMachineManager::LoadZoneInfoFromDB(const core::CAlarmMachinePtr& machine, void* udata, LoadDBProgressCB cb)
{
	AUTO_LOG_FUNCTION;
	auto subProgress = std::make_shared<ProgressEx>();
	//progress->subProgress = subProgress;
	if (cb && udata) {
		subProgress->main = false;
		subProgress->progress = 0;
		subProgress->value = 0;
		subProgress->total = MAX_MACHINE_ZONE;
		cb(udata, subProgress);
		JLOG(L"SUBPROGRESS reset 0 OK\n");
	}

	CString query;
	query.Format(L"select * from ZoneInfo where ademco_id=%d order by zone_value",
				 machine->get_ademco_id());
	ado::CADORecordset recordset(m_db->GetDatabase());
	JLOG(L"CADORecordset recordset %p\n", &recordset);
	BOOL ret = recordset.Open(m_db->GetDatabase()->m_pConnection, query); VERIFY(ret);
	JLOG(L"recordset.Open() return %d\n", ret);
	DWORD count = recordset.GetRecordCount();
	JLOG(L"recordset.GetRecordCount() return %d\n", count);
	if (count > 0) {
		CString null;
		null = GetStringFromAppResource(IDS_STRING_NULL);
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
			
			CZoneInfoPtr zone = std::make_shared<CZoneInfo>();
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
			CDetectorInfoPtr detInfo = LoadDetectorInfoFromDB(zone->get_detector_id());
			if (detInfo) {
				detInfo->set_zone_info_id(zone->get_id());
				detInfo->set_zone_value(zone_value);
				zone->SetDetectorInfo(detInfo);
				m_detectorList.push_back(detInfo);
			}

			if (zone->get_type() == ZT_SUB_MACHINE)
				LoadSubMachineInfoFromDB(zone);
			machine->AddZone(zone);

			if (cb && udata) { 
				subProgress->progress = static_cast<int>(i * MAX_MACHINE_ZONE / count);
				subProgress->value = zone_value;
				subProgress->total = count;
				cb(udata, subProgress);
			}
		}

	}
	recordset.Close();
}


CDetectorInfoPtr CAlarmMachineManager::LoadDetectorInfoFromDB(int id)
{
	AUTO_LOG_FUNCTION;
	CDetectorInfoPtr detector;
	CString query;
	query.Format(L"select * from DetectorInfo where id=%d", id);
	ado::CADORecordset recordset(m_db->GetDatabase());
	recordset.Open(m_db->GetDatabase()->m_pConnection, query);
	DWORD count = recordset.GetRecordCount();
	if (count == 1) {
		recordset.MoveFirst();
		int /*zone_info_id, */map_id, x, y, distance, angle, detector_lib_id;
		recordset.GetFieldValue(L"map_id", map_id);
		recordset.GetFieldValue(L"x", x);
		recordset.GetFieldValue(L"y", y);
		recordset.GetFieldValue(L"distance", distance);
		recordset.GetFieldValue(L"angle", angle);
		recordset.GetFieldValue(L"detector_lib_id", detector_lib_id);

		detector = std::make_shared<CDetectorInfo>();
		detector->set_id(id);
		detector->set_map_id(map_id);
		detector->set_x(x);
		detector->set_y(y);
		detector->set_distance(distance);
		detector->set_angle(angle);
		detector->set_detector_lib_id(detector_lib_id);
	}
	recordset.Close();
	return detector;
}


void CAlarmMachineManager::LoadCameraInfoFromDB()
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"select * from DetectorInfoOfCamera order by device_info_id and device_productor");
	ado::CADORecordset recordset(m_db->GetDatabase());
	recordset.Open(m_db->GetDatabase()->m_pConnection, query);
	DWORD count = recordset.GetRecordCount();
	if(count > 0)
		recordset.MoveFirst();
	for (DWORD i = 0; i < count; i++) {
		int id, ademco_id, sub_machine_id, map_id, x, y, distance, angle, detector_lib_id, device_info_id, device_productor;
		recordset.GetFieldValue(L"id", id);
		recordset.GetFieldValue(L"ademco_id", ademco_id);
		recordset.GetFieldValue(L"sub_machine_id", sub_machine_id);
		recordset.GetFieldValue(L"map_id", map_id);
		recordset.GetFieldValue(L"x", x);
		recordset.GetFieldValue(L"y", y);
		recordset.GetFieldValue(L"distance", distance);
		recordset.GetFieldValue(L"angle", angle);
		recordset.GetFieldValue(L"detector_lib_id", detector_lib_id);
		recordset.GetFieldValue(L"device_info_id", device_info_id);
		recordset.GetFieldValue(L"device_productor", device_productor);
		recordset.MoveNext();

		CDetectorInfoPtr detector = std::make_shared<CDetectorInfo>();
		detector->set_id(id);
		detector->set_map_id(map_id);
		detector->set_x(x);
		detector->set_y(y);
		detector->set_distance(distance);
		detector->set_angle(angle);
		detector->set_detector_lib_id(detector_lib_id);

		CCameraInfoPtr cameraInfo = std::make_shared<CCameraInfo>();
		cameraInfo->set_ademco_id(ademco_id);
		cameraInfo->set_sub_machine_id(sub_machine_id);
		cameraInfo->SetDetectorInfo(detector);
		cameraInfo->set_device_info_id(device_info_id);
		cameraInfo->set_productor(device_productor);

		m_cameraMap[std::pair<int, int>(device_info_id, device_productor)].push_back(cameraInfo);
		m_cameraIdMap[id] = cameraInfo;
	}
	recordset.Close();
}


void CAlarmMachineManager::ResolveCameraInfo(int device_id, int productor)
{
	std::list<std::pair<int, int>> unresolvedList;
	auto iter = m_cameraMap.find(std::pair<int, int>(device_id, productor));
	if (iter != m_cameraMap.end()) {
		for (auto camera : iter->second) {
			bool resolved = false;
			do {
				CAlarmMachinePtr machine = m_machineMap[camera->get_ademco_id()];
				if (!machine) break;
				if (camera->get_sub_machine_id() != -1) {
					auto zone = machine->GetZone(camera->get_sub_machine_id());
					if (!zone) break;
					machine = zone->GetSubMachineInfo();
					if (!machine) break;
				} 
				int map_id = camera->GetDetectorInfo()->get_map_id();
				auto map = machine->GetMapInfo(map_id);
				if (!map) break;
				map->AddInterface(camera);
				resolved = true;
			} while (false);
			if(!resolved)
				unresolvedList.push_back(std::make_pair(device_id, productor));
		}
	} else {
		unresolvedList.push_back(std::make_pair(device_id, productor));
	}

	CString query;
	for (auto pair : unresolvedList) {
		auto cameraList = m_cameraMap[pair];
		for (auto camera : cameraList) {
			m_cameraIdMap.erase(camera->GetDetectorInfo()->get_id());
		}
		m_cameraMap.erase(pair);
		query.Format(L"delete from DetectorInfoOfCamera where device_info_id=%d and device_productor=%d", pair.first, pair.second);
		ExecuteSql(query);
	}
}


void CAlarmMachineManager::DeleteCameraInfo(const CCameraInfoPtr& camera)
{
	AUTO_LOG_FUNCTION;
	assert(camera);
	auto pair = std::make_pair(camera->get_device_info_id(), camera->get_productor());
	CString query;
	query.Format(L"delete from DetectorInfoOfCamera where id=%d", camera->GetDetectorInfo()->get_id());
	ExecuteSql(query);
	m_cameraMap[pair].remove(camera);
	m_cameraIdMap.erase(camera->GetDetectorInfo()->get_id());
}


void CAlarmMachineManager::DeleteCameraInfo(int device_id, int productor)
{
	AUTO_LOG_FUNCTION;
	auto pair = std::make_pair(device_id, productor);
	CString query;
	query.Format(L"delete from DetectorInfoOfCamera where device_info_id=%d and device_productor=%d", device_id, productor);
	ExecuteSql(query);
	auto iter = m_cameraMap.find(pair);
	if (iter != m_cameraMap.end()) {
		for (auto i : iter->second) {
			m_cameraIdMap.erase(i->GetDetectorInfo()->get_id());
		}
		iter->second.clear();
		m_cameraMap.erase(iter);
	}
}


void CAlarmMachineManager::AddCameraInfo(const CCameraInfoPtr& camera)
{
	AUTO_LOG_FUNCTION;
	assert(camera);
	auto pair = std::make_pair(camera->get_device_info_id(), camera->get_productor());
	m_cameraMap[pair].push_back(camera);
	m_cameraIdMap[camera->GetDetectorInfo()->get_id()] = camera;
}


CMapInfoPtr CAlarmMachineManager::GetMapInfoById(int id)
{
	for (auto map : m_mapList) {
		if (map->get_id() == id) {
			return map;
		}
	}
	return nullptr;
}


void CAlarmMachineManager::LoadSubMachineInfoFromDB(const CZoneInfoPtr& zone)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"select * from SubMachine where id=%d",
				 zone->get_sub_machine_id());
	ado::CADORecordset recordset(m_db->GetDatabase());
	recordset.Open(m_db->GetDatabase()->m_pConnection, query);
	DWORD count = recordset.GetRecordCount();
	if (count == 1) {
		CString null;
		null = GetStringFromAppResource(IDS_STRING_NULL);
		recordset.MoveFirst();
		int status;
		CString /*alias, */contact, address, phone, phone_bk;
		COleDateTime expire_time; double x, y;
		//recordset.GetFieldValue(L"alias", alias);
		recordset.GetFieldValue(L"contact", contact);
		if (contact == null) { contact.Empty(); }
		recordset.GetFieldValue(L"address", address);
		if (address == null) { address.Empty(); }
		recordset.GetFieldValue(L"phone", phone);
		if (phone == null) { phone.Empty(); }
		recordset.GetFieldValue(L"phone_bk", phone_bk);
		if (phone_bk == null) { phone_bk.Empty(); }
		recordset.GetFieldValue(L"expire_time", expire_time);
		recordset.GetFieldValue(L"baidu_x", x);
		recordset.GetFieldValue(L"baidu_y", y);
		recordset.GetFieldValue(L"machine_status", status);

		CAlarmMachinePtr subMachine = std::make_shared<CAlarmMachine>();
		subMachine->set_is_submachine(true);
		subMachine->set_id(zone->get_sub_machine_id());
		subMachine->set_ademco_id(zone->get_ademco_id());
		subMachine->set_submachine_zone(zone->get_zone_value());
		subMachine->set_alias(zone->get_alias());
		subMachine->set_address((LPCTSTR)address);
		subMachine->set_contact((LPCTSTR)contact);
		subMachine->set_phone((LPCTSTR)phone);
		subMachine->set_phone_bk((LPCTSTR)phone_bk);
		subMachine->set_machine_status(Integer2MachineStatus(status));
		if (expire_time.GetStatus() != COleDateTime::valid) {
			expire_time = COleDateTime::GetCurrentTime();
		}
		subMachine->set_expire_time(expire_time);
		subMachine->set_coor(web::BaiduCoordinate(x, y));
		SmsConfigure sms_cfg;
		CSms* sms = CSms::GetInstance();
		if (sms->get_sms_config(subMachine->get_is_submachine(), zone->get_ademco_id(), 
			subMachine->get_submachine_zone(), sms_cfg)) {
			subMachine->set_sms_cfg(sms_cfg);
		} else {
			sms->add_sms_config(subMachine->get_is_submachine(), zone->get_ademco_id(),
								subMachine->get_submachine_zone(), sms_cfg);
			subMachine->set_sms_cfg(sms_cfg);
		}

		CAlarmMachinePtr parentMachine = GetMachine(zone->get_ademco_id());
		if (parentMachine) {
			subMachine->set_machine_type(parentMachine->get_machine_type());
		}

		LoadMapInfoFromDB(subMachine);
		LoadSubZoneInfoOfSubMachineFromDB(subMachine);
		subMachine->LoadXmlConfig();
		zone->SetSubMachineInfo(subMachine);
	}
	recordset.Close();
}


void CAlarmMachineManager::LoadSubZoneInfoOfSubMachineFromDB(const core::CAlarmMachinePtr& subMachine)
{
	CString query;
	query.Format(L"select * from SubZone where sub_machine_id=%d",
				 subMachine->get_id());
	ado::CADORecordset recordset(m_db->GetDatabase());
	recordset.Open(m_db->GetDatabase()->m_pConnection, query);
	DWORD count = recordset.GetRecordCount();
	if (count > 0) {
		CString null;
		null = GetStringFromAppResource(IDS_STRING_NULL);
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

			CZoneInfoPtr subZone = std::make_shared<CZoneInfo>();
			subZone->set_id(id);
			subZone->set_ademco_id(subMachine->get_ademco_id());
			subZone->set_zone_value(subMachine->get_submachine_zone());
			subZone->set_sub_zone(sub_zone_value);
			subZone->set_sub_machine_id(subMachine->get_id());
			subZone->set_alias(alias);
			subZone->set_detector_id(detector_info_id);
			//subZone->set_property_id(property_info_id);
			subZone->set_type(ZT_SUB_MACHINE_ZONE);

			CDetectorInfoPtr detInfo = LoadDetectorInfoFromDB(subZone->get_detector_id());
			if (detInfo) {
				detInfo->set_zone_info_id(subZone->get_id());
				int zone_value = subZone->get_sub_zone();
				detInfo->set_zone_value(zone_value);
				subZone->SetDetectorInfo(detInfo);
				m_detectorList.push_back(detInfo);
			}

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
	ado::CADORecordset recordset(m_db->GetDatabase());
	recordset.Open(m_db->GetDatabase()->m_pConnection, query);
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

			CDetectorLibDataPtr data = std::make_shared<CDetectorLibData>();
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
//	ado::CADORecordset recordset(m_db->GetDatabase());
//	recordset.Open(m_db->GetDatabase()->m_pConnection, query);
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
	return m_machineMap.size();
}


CAlarmMachinePtr CAlarmMachineManager::GetMachine(int ademco_id)
{
	auto iter = m_machineMap.find(ademco_id);
	if (iter != m_machineMap.end() && iter->second) {
		return iter->second;
	}
	return nullptr;
}


BOOL CAlarmMachineManager::CheckIsValidMachine(int ademco_id, /*const char* device_id, */int zone)
{
	if (ademco_id < 0 || MAX_MACHINE <= ademco_id) {
		return FALSE;
	}

	if (zone < 0 || MAX_MACHINE_ZONE < zone) {
		return FALSE;
	}

	auto iter = m_machineMap.find(ademco_id);
	if (iter != m_machineMap.end() && iter->second) {
		if (iter->second) {
			if (!iter->second->get_banned()) {
				return TRUE;
			}
		}
	}
	return FALSE;
}


BOOL CAlarmMachineManager::CheckIfMachineAdemcoIdCanUse(int ademco_id)
{
	if (ademco_id < 0 || MAX_MACHINE <= ademco_id) {
		return FALSE;
	}

	auto iter = m_machineMap.find(ademco_id);
	if (iter != m_machineMap.end() && iter->second) {
		return FALSE;
	}

	return TRUE;
}


BOOL CAlarmMachineManager::DistributeAdemcoID(int& ademco_id)
{
	if (m_machineMap.size() >= MAX_MACHINE) {
		return FALSE;
	}
	
	for (int i = 0; i < MAX_MACHINE; i++) {
		auto machine = m_machineMap[i];
		if (!machine) {
			ademco_id = i;
			return TRUE;
		}
	}

	return FALSE;
}


BOOL CAlarmMachineManager::AddMachine(const core::CAlarmMachinePtr& machine)
{
	int ademco_id = machine->get_ademco_id();
	if (ademco_id < 0 || MAX_MACHINE <= ademco_id) {
		return FALSE;
	}

	std::lock_guard<std::mutex> lock(m_lock4Machines);
	CString query;
	query.Format(L"insert into [AlarmMachine] ([ademco_id],[device_id],[banned],[machine_type],[has_video],[alias],[contact],[address],[phone],[phone_bk],[group_id],[expire_time]) values(%d,'%s',%d,%d,%d,'%s','%s','%s','%s','%s',%d,'%s')",
				 ademco_id, L"", machine->get_banned(),
				 machine->get_machine_type(), machine->get_has_video(), 
				 machine->get_alias(), machine->get_contact(),
				 machine->get_address(), machine->get_phone(), 
				 machine->get_phone_bk(), machine->get_group_id(),
				 machine->get_expire_time().Format(L"%Y-%m-%d %H:%M:%S"));
	int id = AddAutoIndexTableReturnID(query);
	if (-1 == id) {
		return FALSE;
	}

	machine->set_id(id);

	m_machineMap[ademco_id] = machine;
	return TRUE;
}


BOOL CAlarmMachineManager::DeleteMachine(const core::CAlarmMachinePtr& machine)
{
	int ademco_id = machine->get_ademco_id();
	if (ademco_id < 0 || MAX_MACHINE <= ademco_id) {
		return FALSE;
	}

	std::lock_guard<std::mutex> lock(m_lock4Machines);

	machine->kill_connction();
	CString query;
	query.Format(L"delete from AlarmMachine where id=%d and ademco_id=%d",
				 machine->get_id(), machine->get_ademco_id());
	if (m_db->GetDatabase()->Execute(query)) {
		// delete all camera info
		CMapInfoList mapList;
		machine->GetAllMapInfo(mapList);
		for (auto map : mapList) {
			std::list<CDetectorBindInterfacePtr> interfaceList;
			map->GetAllInterfaceInfo(interfaceList);
			for (auto pInterface : interfaceList) {
				if (pInterface->GetInterfaceType() == DIT_CAMERA_INFO) {
					CCameraInfoPtr cam = std::dynamic_pointer_cast<CCameraInfo>(pInterface);
					DeleteCameraInfo(cam);
				}
			}
		}

		// delete all zone & detector info of machine
		std::list<CZoneInfoPtr> zoneList;
		machine->GetAllZoneInfo(zoneList);
		for (auto zone : zoneList) {
			int detector_id = zone->get_detector_id();
			if (-1 != detector_id) {
				query.Format(L"delete from DetectorInfo where id=%d", detector_id);
				VERIFY(m_db->GetDatabase()->Execute(query));
			}
			CAlarmMachinePtr subMachine = zone->GetSubMachineInfo();
			if (subMachine) {
				DeleteSubMachine(zone);
			}
			DeleteVideoBindInfoByZoneInfo(zone);
		}

		query.Format(L"delete from ZoneInfo where ademco_id=%d", machine->get_ademco_id());
		VERIFY(m_db->GetDatabase()->Execute(query));

		query.Format(L"delete from MapInfo where machine_id=%d and type=%d", 
					 machine->get_ademco_id(), MAP_MACHINE);
		VERIFY(m_db->GetDatabase()->Execute(query));
		


		CGroupInfoPtr group = CGroupManager::GetInstance()->GetGroupInfo(machine->get_group_id());
		group->RemoveChildMachine(machine); 

		CSms::GetInstance()->del_sms_config(machine->get_sms_cfg().id);
		
		m_machineMap.erase(ademco_id);
		return TRUE;
	}
	return FALSE;
}


BOOL CAlarmMachineManager::DeleteSubMachine(const CZoneInfoPtr& zoneInfo)
{
	ASSERT(zoneInfo);
	CAlarmMachinePtr subMachine = zoneInfo->GetSubMachineInfo();
	ASSERT(subMachine);

	CSms::GetInstance()->del_sms_config(subMachine->get_sms_cfg().id);

	CString query;
	query.Format(L"delete from SubMachine where id=%d",
				 subMachine->get_id());
	JLOG(L"%s\n", query);
	VERIFY(m_db->GetDatabase()->Execute(query));

	// delete all camera info
	CMapInfoList mapList;
	subMachine->GetAllMapInfo(mapList);
	for (auto map : mapList) {
		std::list<CDetectorBindInterfacePtr> interfaceList;
		map->GetAllInterfaceInfo(interfaceList);
		for (auto pInterface : interfaceList) {
			if (pInterface->GetInterfaceType() == DIT_CAMERA_INFO) {
				CCameraInfoPtr cam = std::dynamic_pointer_cast<CCameraInfo>(pInterface);
				DeleteCameraInfo(cam);
			}
		}
	}

	// delete all zone & detector info of machine
	std::list<CZoneInfoPtr> zoneList;
	subMachine->GetAllZoneInfo(zoneList);
	for (auto zone : zoneList) {
		int detector_id = zone->get_detector_id();
		if (-1 != detector_id) {
			query.Format(L"delete from DetectorInfo where id=%d", detector_id);
			JLOG(L"%s\n", query);
			VERIFY(m_db->GetDatabase()->Execute(query));
		}
		DeleteVideoBindInfoByZoneInfo(zone);
	}

	query.Format(L"delete from SubZone where sub_machine_id=%d",
				 subMachine->get_id());
	JLOG(L"%s\n", query);
	VERIFY(m_db->GetDatabase()->Execute(query));

	query.Format(L"delete from MapInfo where machine_id=%d and type=%d",
				 subMachine->get_id(), MAP_SUB_MACHINE);
	JLOG(L"%s\n", query);
	VERIFY(m_db->GetDatabase()->Execute(query));

	query.Format(L"update ZoneInfo set type=%d,sub_machine_id=-1 where id=%d",
				 ZT_ZONE, zoneInfo->get_id());
	JLOG(L"%s\n", query);
	VERIFY(m_db->GetDatabase()->Execute(query));

	return TRUE;
}


void CAlarmMachineManager::MachineEventHandler(EventSource resource, 
											   int ademco_id, int ademco_event,
											   int zone, int subzone, 
											   const time_t& timestamp,
											   const time_t& recv_time,
											   const ademco::char_array_ptr& xdata
											   )
{
	AUTO_LOG_FUNCTION;
	CAlarmMachinePtr machine = GetMachine(ademco_id);
	if (machine) {
		machine->SetAdemcoEvent(resource, ademco_event, zone, subzone, timestamp, recv_time, xdata);
	}
}


void CAlarmMachineManager::MachineOnline(ademco::EventSource resource, 
										 int ademco_id, BOOL online, const char* ipv4,
										 net::server::CClientDataPtr udata, RemoteControlCommandConnCB cb)
{
	AUTO_LOG_FUNCTION;
	CAlarmMachinePtr machine = GetMachine(ademco_id);
	if (machine) {
		time_t event_time = time(nullptr);
		machine->SetAdemcoEvent(resource, online ? EVENT_ONLINE : EVENT_OFFLINE, 0, 0, event_time, event_time);
		if (online && udata && cb) {
			machine->SetConnHangupCallback(udata, cb);
		}
		machine->set_ipv4(ipv4);
	}
}


BOOL CAlarmMachineManager::RemoteControlAlarmMachine(const CAlarmMachinePtr& machine, 
													 int ademco_event, int gg, int zone, 
													 const ademco::char_array_ptr& xdata,
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
	suser = GetStringFromAppResource(IDS_STRING_USER);
	sfm = GetStringFromAppResource(IDS_STRING_LOCAL_OP);
	fmMachine = GetStringFromAppResource(IDS_STRING_MACHINE);
	fmSubmachine = GetStringFromAppResource(IDS_STRING_SUBMACHINE);
	switch (ademco_event) {
		case EVENT_ARM:
			sop = GetStringFromAppResource(IDS_STRING_ARM);
			break;
		case EVENT_HALFARM:
			sop = GetStringFromAppResource(IDS_STRING_HALFARM);
			break;
		case EVENT_DISARM:
			sop = GetStringFromAppResource(IDS_STRING_DISARM);
			break;
		case EVENT_EMERGENCY:
			sop = GetStringFromAppResource(IDS_STRING_EMERGENCY);
			break;
		case EVENT_QUERY_SUB_MACHINE:
			sop = GetStringFromAppResource(IDS_STRING_QUERY);
			break;
		case EVENT_RETRIEVE_ZONE_OR_SUB_MACHINE:
			sop = GetStringFromAppResource(IDS_STRING_RETRIEVE);
			break;
		case EVENT_WRITE_TO_MACHINE: {
			sop = GetStringFromAppResource(IDS_STRING_WRITE2MACHINE);
			CString s, szone; szone = GetStringFromAppResource(IDS_STRING_ZONE);
			s.Format(L"(%s%03d)", szone, zone);
			sop += s;
		}
			break;
		default:
			assert(0);
			break;
	}
	CUserInfoPtr user = CUserManager::GetInstance()->GetCurUserInfo();
	srecord.Format(L"%s(ID:%d,%s)%s:%s", suser,
				   user->get_user_id(), user->get_user_name(),
				   sfm, sop);

	if (machine->get_is_submachine()) {
		CAlarmMachinePtr netMachine = GetMachine(machine->get_ademco_id());
		if (netMachine) {
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
												zone, srecord, time(nullptr),
												RECORD_LEVEL_USERCONTROL);

	return net::CNetworkConnector::GetInstance()->Send(machine->get_ademco_id(),
													   ademco_event, gg, zone, xdata);
}


void CAlarmMachineManager::DisarmPasswdWrong(int ademco_id)
{
	CString spasswdwrong;
	spasswdwrong = GetStringFromAppResource(IDS_STRING_USER_PASSWD_WRONG);
	CHistoryRecord::GetInstance()->InsertRecord(ademco_id, m_prevCallDisarmZoneValue,
												spasswdwrong, time(nullptr),
												RECORD_LEVEL_USERCONTROL);
	if (m_prevCallDisarmAdemcoID != ademco_id)
		return;

	auto xdata = std::make_shared<ademco::char_array>();
	CInputDlg dlg(m_pPrevCallDisarmWnd);
	if (dlg.DoModal() != IDOK)
		return;
	if (dlg.m_edit.GetLength() != 6)
		return;

	CString srecord, suser, sfm, sop, snull;
	suser = GetStringFromAppResource(IDS_STRING_USER);
	sfm = GetStringFromAppResource(IDS_STRING_LOCAL_OP);
	sop = GetStringFromAppResource(IDS_STRING_DISARM);
	snull = GetStringFromAppResource(IDS_STRING_NULL);
	
	CUserInfoPtr user = CUserManager::GetInstance()->GetCurUserInfo();
	CAlarmMachinePtr machine = GetMachine(ademco_id);
	srecord.Format(L"%s(ID:%d,%s)%s:%s%04d(%s)", suser,
				   user->get_user_id(), user->get_user_name(),
				   sfm, sop, ademco_id, machine ? machine->get_alias() : snull);
	CHistoryRecord::GetInstance()->InsertRecord(machine->get_ademco_id(),
												m_prevCallDisarmZoneValue,
												srecord, time(nullptr),
												RECORD_LEVEL_USERCONTROL);

	USES_CONVERSION;
	const char* a = W2A(dlg.m_edit);
	for (int i = 0; i < 6; i++) {
		xdata->push_back(a[i]);
	}
	//xdata_len = strlen(xdata);
	net::CNetworkConnector::GetInstance()->Send(ademco_id, 
												ademco::EVENT_DISARM, 
												m_prevCallDisarmGG, 
												m_prevCallDisarmZoneValue, 
												xdata);
}


void CAlarmMachineManager::EnterEditMode()
{
	AUTO_LOG_FUNCTION;
	for (auto iter : m_machineMap) {
		if(iter.second)
			iter.second->EnterBufferMode();
	}
	
}


void CAlarmMachineManager::LeaveEditMode()
{
	AUTO_LOG_FUNCTION;
	for (auto iter : m_machineMap) {
		if (iter.second)
			iter.second->LeaveBufferMode();
	}
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
		if (mgr->GetMachineCount() == 0)
			continue;
		
		std::lock_guard<std::mutex> lock(mgr->m_lock4Machines);
		CAlarmMachineList *subMachineList = nullptr;
		for (int i = 0; i < MAX_MACHINE; i++) {
			if (WAIT_OBJECT_0 == WaitForSingleObject(mgr->m_hEventExit, 0))
				break;
			CAlarmMachinePtr machine = mgr->m_machineMap[i];
			if (machine && machine->get_online() && machine->get_submachine_count() > 0) {
				if (!machine->EnterBufferMode()) {
					machine->SetOotebmObj(nullptr, nullptr);
					continue;
				}
				machine->SetOotebmObj(OnOtherCallEnterBufferMode, mgr);
				CZoneInfoList list;
				machine->GetAllZoneInfo(list);
				bool bAlreadyLeaveBuffMode = false;
				for (auto zoneInfo : list) {
					if (WAIT_OBJECT_0 == WaitForSingleObject(mgr->m_hEventExit, 0))
						break;
					if (WAIT_OBJECT_0 == WaitForSingleObject(mgr->m_hEventOotebm, 0)) {
						machine->SetOotebmObj(nullptr, nullptr); 
						machine->LeaveBufferMode();
						bAlreadyLeaveBuffMode = true;
						break;
					}
					CAlarmMachinePtr subMachine = zoneInfo->GetSubMachineInfo();
					if (subMachine) {
						time_t lastActionTime = subMachine->GetLastActionTime();
						time_t check_time = time(nullptr);
						if ((check_time - lastActionTime) * 1000 >= MAX_SUBMACHINE_ACTION_TIME_OUT) {
							if (subMachineList == nullptr) {
								subMachineList = new CAlarmMachineList();
							}
							subMachineList->push_back(subMachine);
						}
					}
				}
				if (!bAlreadyLeaveBuffMode) {
					machine->SetOotebmObj(nullptr, nullptr);
					machine->LeaveBufferMode();
				}
			}
		}
		if (subMachineList && subMachineList->size() > 0) {
			CAlarmCenterApp* app = reinterpret_cast<CAlarmCenterApp*>(AfxGetApp());
			ASSERT(app);
			CAlarmCenterDlg* dlg = reinterpret_cast<CAlarmCenterDlg*>(app->GetMainWnd());
			ASSERT(dlg);
			dlg->PostMessage(WM_NEEDQUERYSUBMACHINE, 
							 (WPARAM)subMachineList, subMachineList->size());
		}
	}
	return 0;
}


void core::CAlarmMachineManager::DeleteVideoBindInfoByZoneInfo(const CZoneInfoPtr& zoneInfo)
{
	AUTO_LOG_FUNCTION;

	video::ZoneUuid uuid(zoneInfo->get_ademco_id(), zoneInfo->get_zone_value(), 0);
	if (zoneInfo->get_type() == ZT_SUB_MACHINE_ZONE) {
		uuid._gg = INDEX_SUB_MACHINE;
	}
	video::CVideoManager::GetInstance()->UnbindZoneAndDevice(uuid);
}


CDetectorInfoPtr core::CAlarmMachineManager::GetDetectorInfo(int id)
{ 
	for (auto det : m_detectorList) { 
		if (det->get_id() == id)
			return det; 
	} 
	return nullptr; 
}



NAMESPACE_END



