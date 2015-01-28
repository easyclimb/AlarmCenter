#include "stdafx.h"
#include "AlarmMachineManager.h"
#include "ado2.h"
#include "AlarmMachine.h"
#include "ademco_func.h"
#include "MapInfo.h"
#include "ZoneInfo.h"
#include "DetectorInfo.h"
#include "DetectorLib.h"

namespace core {

CLock CAlarmMachineManager::m_lock;
static CAlarmMachineManager* g_pInstance = NULL;

CAlarmMachineManager::CAlarmMachineManager()
	: m_pDatabase(NULL)
	, m_detectorLib(NULL)
{
	LoadAlarmMachineFromDB();
	m_detectorLib = new CDetectorLib();
	LoadDetectorLibFromDB();
}


CAlarmMachineManager::~CAlarmMachineManager()
{
	std::list<CAlarmMachine*>::iterator iter = m_listAlarmMachine.begin();
	while (iter != m_listAlarmMachine.end()) {
		CAlarmMachine* machine = *iter++;
		delete machine;
	}
	m_listAlarmMachine.clear();

	if (m_pDatabase) {
		if (m_pDatabase->IsOpen()) {
			m_pDatabase->Close();
		}
		delete m_pDatabase;
	}

	if (m_detectorLib) { delete m_detectorLib; }
}

CAlarmMachineManager* CAlarmMachineManager::GetInstance()
{
	m_lock.Lock();
	if (g_pInstance == NULL) {
		static CAlarmMachineManager manager;
		g_pInstance = &manager;
	}
	m_lock.UnLock();
	return g_pInstance;
}


const wchar_t* CAlarmMachineManager::GetCsrAcct() const
{
	return m_csr_acct;
}


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
		strConn.Format(_T("Provider=Microsoft.Jet.OLEDB.4.0; Data Source='%s';Jet OLEDB:Database Password='888101'"), szMdbPath);
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


void CAlarmMachineManager::InitDetectorLib()
{
	//CLog::WriteLog(_T("CDBOper::InitData()"));
	//const TCHAR *query = _T("select * from detector_lib order by id");
	////CADORecordset * pDataGridRecord = new CADORecordset(m_pDatabase);
	//std::auto_ptr<ado::CADORecordset> pDataGridRecord(new ado::CADORecordset(m_pDatabase));
	//pDataGridRecord->Open(m_pDatabase->m_pConnection, query);
	//CLog::WriteLog(_T("pDataGridRecord->Open(m_pDatabase->m_pConnection 0x%x, %s)"),
	//			   m_pDatabase->m_pConnection, query);
	//CLog::WriteLog(_T("pDataGridRecord->Open() over, calling GetRecordCount"));
	//ULONG count = pDataGridRecord->GetRecordCount();
	//CLog::WriteLog(_T("GetRecordCount over, count is %d"), count);
	//if (count == 0) {
	//	//BOOL bChinese = CConfig::IsChinese();
	//	int condition = 0;
	//	CString language = CConfig::GetLanguage();
	//	if (language.CompareNoCase(L"Chinese.dll") == 0) {
	//		condition = 0;
	//	} else if (language.CompareNoCase(L"Taiwaness.dll") == 0) {
	//		condition = 1;
	//	} else if (language.CompareNoCase(L"English.dll") == 0) {
	//		condition = 2;
	//	} else {
	//		ASSERT(0);
	//	}

	//	CString detPath = _T("");
	//	detPath.Format(_T("%s\\Detectors\\"), GetModuleFilePath());

	//	DETECTOR_LIB dl;

	//	// A2
	//	dl.type = DT_DOUBLE;
	//	dl.antline_num = ALN_2;
	//	dl.antline_gap = 12;
	//	dl.name = _T("A2");
	//	dl.path = detPath + _T("A2.bmp");
	//	dl.path_pair = detPath + L"A2Receiver.bmp";
	//	CLog::WriteLog(_T("before add lib"));
	//	Add(&dl, DB_TABLE_DETECTOR_LIB);

	//	// A4
	//	dl.type = DT_DOUBLE;
	//	dl.antline_num = ALN_4;
	//	dl.antline_gap = 12;
	//	dl.name = _T("A4");
	//	dl.path = detPath + _T("A4.bmp");
	//	dl.path_pair = detPath + _T("A4Receiver.bmp");
	//	Add(&dl, DB_TABLE_DETECTOR_LIB);

	//	// A8
	//	dl.type = DT_DOUBLE;
	//	dl.antline_num = ALN_8;
	//	dl.antline_gap = 12;
	//	dl.name = _T("A8");
	//	dl.path = detPath + _T("A8.bmp");
	//	dl.path_pair = detPath + _T("A8Receiver.bmp");
	//	Add(&dl, DB_TABLE_DETECTOR_LIB);

	//	// R2
	//	dl.type = DT_DOUBLE;
	//	dl.antline_num = ALN_2;
	//	dl.antline_gap = 12;
	//	dl.name = _T("R2");
	//	dl.path = detPath + _T("R2.bmp");
	//	dl.path_pair = detPath + _T("R2Receiver.bmp");
	//	Add(&dl, DB_TABLE_DETECTOR_LIB);

	//	// R3
	//	dl.type = DT_DOUBLE;
	//	dl.antline_num = ALN_3;
	//	dl.antline_gap = 12;
	//	dl.name = _T("R3");
	//	dl.path = detPath + _T("R3.bmp");
	//	dl.path_pair = detPath + _T("R3Receiver.bmp");
	//	Add(&dl, DB_TABLE_DETECTOR_LIB);

	//	// R4
	//	dl.type = DT_DOUBLE;
	//	dl.antline_num = ALN_4;
	//	dl.antline_gap = 12;
	//	dl.name = _T("R4");
	//	dl.path = detPath + _T("R4.bmp");
	//	dl.path_pair = detPath + _T("R4Receiver.bmp");
	//	Add(&dl, DB_TABLE_DETECTOR_LIB);

	//	// R6
	//	dl.type = DT_DOUBLE;
	//	dl.antline_num = ALN_6;
	//	dl.antline_gap = 12;
	//	dl.name = _T("R6");
	//	dl.path = detPath + _T("R6.bmp");
	//	dl.path_pair = detPath + _T("R6Receiver.bmp");
	//	Add(&dl, DB_TABLE_DETECTOR_LIB);

	//	// R8
	//	dl.type = DT_DOUBLE;
	//	dl.antline_num = ALN_8;
	//	dl.antline_gap = 16;
	//	dl.name = _T("R8");
	//	dl.path = detPath + _T("R8.bmp");
	//	dl.path_pair = detPath + _T("R8Receiver.bmp");
	//	Add(&dl, DB_TABLE_DETECTOR_LIB);

	//	// S4
	//	dl.type = DT_DOUBLE;
	//	dl.antline_num = ALN_4;
	//	dl.antline_gap = 12;
	//	dl.name = _T("S4");
	//	dl.path = detPath + _T("S4.bmp");
	//	dl.path_pair = detPath + _T("S4Receiver.bmp");
	//	Add(&dl, DB_TABLE_DETECTOR_LIB);

	//	// S4-D
	//	dl.type = DT_DOUBLE;
	//	dl.antline_num = ALN_4;
	//	dl.antline_gap = 12;
	//	dl.name = _T("S4-D");
	//	dl.path = detPath + _T("S4-D.bmp");
	//	dl.path_pair = detPath + _T("S4-DReceiver.bmp");
	//	Add(&dl, DB_TABLE_DETECTOR_LIB);

	//	// S8
	//	dl.type = DT_DOUBLE;
	//	dl.antline_num = ALN_8;
	//	dl.antline_gap = 14;
	//	dl.name = _T("S8");
	//	dl.path = detPath + _T("S8.bmp");
	//	dl.path_pair = detPath + _T("S8Receiver.bmp");
	//	Add(&dl, DB_TABLE_DETECTOR_LIB);

	//	// S8-D
	//	dl.type = DT_DOUBLE;
	//	dl.antline_num = ALN_8;
	//	dl.antline_gap = 14;
	//	dl.name = _T("S8-D");
	//	dl.path = detPath + _T("S8-D.bmp");
	//	dl.path_pair = detPath + _T("S8-DReceiver.bmp");
	//	Add(&dl, DB_TABLE_DETECTOR_LIB);

	//	// T205
	//	dl.type = DT_SINGLE;
	//	dl.antline_num = ALN_0;
	//	dl.antline_gap = 0;
	//	dl.name = _T("T205");
	//	dl.path = detPath + _T("T205.bmp");
	//	dl.path_pair.Empty();
	//	Add(&dl, DB_TABLE_DETECTOR_LIB);

	//	// JHD-2
	//	dl.type = DT_SINGLE;
	//	dl.antline_num = ALN_0;
	//	dl.antline_gap = 0;
	//	dl.name = _T("JHD-2");
	//	dl.path = detPath + _T("JHD-2.bmp");
	//	dl.path_pair.Empty();
	//	Add(&dl, DB_TABLE_DETECTOR_LIB);

	//	// T201
	//	dl.type = DT_SINGLE;
	//	dl.antline_num = ALN_0;
	//	dl.antline_gap = 0;
	//	dl.name = _T("T201");
	//	dl.path = detPath + _T("T201.bmp");
	//	dl.path_pair.Empty();
	//	Add(&dl, DB_TABLE_DETECTOR_LIB);

	//	// T601
	//	dl.type = DT_SINGLE;
	//	dl.antline_num = ALN_0;
	//	dl.antline_gap = 0;
	//	dl.name = _T("T601");
	//	dl.path = detPath + _T("T601.bmp");
	//	dl.path_pair.Empty();
	//	Add(&dl, DB_TABLE_DETECTOR_LIB);

	//	// 无线门磁
	//	dl.type = DT_SINGLE;
	//	dl.antline_num = ALN_0;
	//	dl.antline_gap = 0;
	//	dl.name = TRIPLE_CONDITION(condition, _T("无线门磁"), _T("無線門磁"), _T("WirelessDoorSensor"));
	//	dl.path = detPath + L"WirelessDoorSensor.bmp";
	//	dl.path_pair.Empty();
	//	Add(&dl, DB_TABLE_DETECTOR_LIB);

	//	// 紧急按钮HB-A380
	//	dl.type = DT_SINGLE;
	//	dl.antline_num = ALN_0;
	//	dl.antline_gap = 0;
	//	dl.name = TRIPLE_CONDITION(condition, _T("紧急按钮HB-A380"), _T("緊急按鈕HB-A380"), _T("EmergencyButtonHB-A380"));
	//	dl.path = detPath + L"EmergencyButtonHB-A380.bmp";
	//	dl.path_pair.Empty();
	//	Add(&dl, DB_TABLE_DETECTOR_LIB);

	//	// 卧室主机HB-3030C
	//	dl.type = DT_SINGLE;
	//	dl.antline_num = ALN_0;
	//	dl.antline_gap = 0;
	//	dl.name = TRIPLE_CONDITION(condition, _T("卧室主机HB-3030C"), _T("臥室主機HB-3030C"), _T("HB-3030C"));
	//	dl.path = detPath + _T("HB-3030C.bmp");
	//	dl.path_pair.Empty();
	//	Add(&dl, DB_TABLE_DETECTOR_LIB);

	//	// 液晶主机HB-BJQ-560
	//	dl.type = DT_SINGLE;
	//	dl.antline_num = ALN_0;
	//	dl.antline_gap = 0;
	//	dl.name = TRIPLE_CONDITION(condition, _T("液晶主机HB-BJQ-560"), _T("液晶主機HB-BJQ-560"), _T("HB-BJQ-560"));
	//	dl.path = detPath + _T("HB-BJQ-560.bmp");
	//	dl.path_pair.Empty();
	//	Add(&dl, DB_TABLE_DETECTOR_LIB);
	//}
	//CLog::WriteLog(_T("CDBOper::InitData() ok"));
}


void CAlarmMachineManager::LoadAlarmMachineFromDB()
{
	static const wchar_t* query = L"select * from AlarmMachine order by AdemcoID";
	ado::CADORecordset recordset(m_pDatabase);
	recordset.Open(m_pDatabase->m_pConnection, query);
	DWORD count = recordset.GetRecordCount();
	if (count > 0) {
		recordset.MoveFirst();
		for (DWORD i = 0; i < count; i++) {
			CAlarmMachine *machine = new CAlarmMachine();
			int id, ademco_id;
			CString device_id, alias;
			recordset.GetFieldValue(L"id", id);
			recordset.GetFieldValue(L"AdemcoID", ademco_id);
			recordset.GetFieldValue(L"DeviceID", device_id);
			recordset.GetFieldValue(L"Alias", alias);
			machine->set_id(id);
			machine->set_ademco_id(ademco_id);
			machine->SetDeviceID(device_id);
			machine->SetAlias(alias);
			LoadMapInfoFromDB(machine);
			m_listAlarmMachine.push_back(machine);
			recordset.MoveNext();
		}
		m_listAlarmMachine.sort();
	}
}


void CAlarmMachineManager::LoadMapInfoFromDB(CAlarmMachine* machine)
{
	CString query;
	query.Format(L"select * from Map where ademco_id=%d order by id", 
				 machine->get_ademco_id());
	ado::CADORecordset recordset(m_pDatabase);
	recordset.Open(m_pDatabase->m_pConnection, query);
	DWORD count = recordset.GetRecordCount();
	if (count > 0) {
		recordset.MoveFirst();
		for (DWORD i = 0; i < count; i++) {
			int id, type, ademco_id;
			CString path; 
			recordset.GetFieldValue(L"id", id);
			recordset.GetFieldValue(L"type", type);
			recordset.GetFieldValue(L"ademco_id", ademco_id);
			recordset.GetFieldValue(L"path", path);
			CMapInfo* map = new CMapInfo();
			map->set_id(id);
			map->set_type(type);
			map->set_ademco_id(ademco_id);
			map->set_path(path);
			machine->AddMap(map);
		}
	}
}


void CAlarmMachineManager::LoadZoneInfoFromDB(CAlarmMachine* machine)
{
	CString query;
	query.Format(L"select * from Zone where ademco_id=%d order by zone_id",
				 machine->get_ademco_id());
	ado::CADORecordset recordset(m_pDatabase);
	recordset.Open(m_pDatabase->m_pConnection, query);
	DWORD count = recordset.GetRecordCount();
	if (count > 0) {
		recordset.MoveFirst();
		for (DWORD i = 0; i < count; i++) {
			int id, zone_id, ademco_id, map_id, type, detector_id;
			CString alias;
			recordset.GetFieldValue(L"id", id);
			recordset.GetFieldValue(L"zone_id", zone_id);
			recordset.GetFieldValue(L"ademco_id", ademco_id);
			recordset.GetFieldValue(L"map_id", map_id);
			recordset.GetFieldValue(L"type", type);
			recordset.GetFieldValue(L"alias", alias);
			recordset.GetFieldValue(L"detector_id", detector_id);
			
			CZoneInfo* zone = new CZoneInfo();
			zone->set_id(id);
			zone->set_zone_id(zone_id);
			zone->set_ademco_id(ademco_id);
			zone->set_map_id(map_id);
			zone->set_type(type);
			zone->set_alias(alias);
			zone->set_detector_id(detector_id);
			LoadDetectorInfoFromDB(zone);
			machine->AddZone(zone);
		}
	}
}


void CAlarmMachineManager::LoadDetectorInfoFromDB(CZoneInfo* zone)
{
	CString query;
	query.Format(L"select * from Detector where zone_id=%d order by id",
				 zone->get_zone_id());
	ado::CADORecordset recordset(m_pDatabase);
	recordset.Open(m_pDatabase->m_pConnection, query);
	DWORD count = recordset.GetRecordCount();
	if (count > 0) {
		recordset.MoveFirst();
		for (DWORD i = 0; i < count; i++) {
			int id, zone_id, x, y, distance, angle, detector_lib_id;
			recordset.GetFieldValue(L"id", id);
			recordset.GetFieldValue(L"zone_id", zone_id);
			recordset.GetFieldValue(L"x", x);
			recordset.GetFieldValue(L"y", y);
			recordset.GetFieldValue(L"distance", distance);
			recordset.GetFieldValue(L"angle", angle);
			recordset.GetFieldValue(L"detector_lib_id", detector_lib_id);

			CDetectorInfo* detector = new CDetectorInfo();
			detector->set_id(id);
			detector->set_zone_id(zone_id);
			detector->set_x(x);
			detector->set_y(y);
			detector->set_distance(distance);
			detector->set_angle(angle);
			detector->set_detector_lib_id(detector_lib_id);
			zone->SetDetectorInfo(detector);
		}
	}
}


void CAlarmMachineManager::LoadDetectorLibFromDB()
{
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

			CDetectorLibData* data = new CDetectorLibData();
			data->set_id(id);
			data->set_type(type);
			data->set_detector_name(detector_name);
			data->set_path(path);
			data->set_path_pair(path_pair);
			data->set_antline_num(antline_num);
			data->set_antline_gap(antline_gap);
			m_detectorLib->AddDetectorLibData(data);
		}
	}
}


int CAlarmMachineManager::GetMachineCount() const
{
	return m_listAlarmMachine.size();
}


BOOL CAlarmMachineManager::GetMachine(int ademco_id, CAlarmMachine*& machine)
{
	std::list<CAlarmMachine*>::iterator iter = m_listAlarmMachine.begin();
	while (iter != m_listAlarmMachine.end()) {
		CAlarmMachine* local_machine = *iter++;
		if (local_machine->get_ademco_id() == ademco_id) {
			machine = local_machine;
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CAlarmMachineManager::GetFirstMachine(CAlarmMachine*& machine)
{
	if (0 < m_listAlarmMachine.size()) {
		m_curMachinePos = m_listAlarmMachine.begin();
		machine = *m_curMachinePos++;
		return TRUE;
	}
	return FALSE;
}


BOOL CAlarmMachineManager::GetNextMachine(CAlarmMachine*& machine)
{
	if (0 < m_listAlarmMachine.size() && m_curMachinePos != m_listAlarmMachine.end()) {
		machine = *m_curMachinePos++;
		return TRUE;
	}
	return FALSE;
}

BOOL CAlarmMachineManager::CheckMachine(int ademco_id, const wchar_t* device_id, int zone)
{
	if (0 < zone || zone > MAX_MACHINE_ZONE)
		return FALSE;

	std::list<CAlarmMachine*>::iterator iter = m_listAlarmMachine.begin();
	while (iter != m_listAlarmMachine.end()) {
		CAlarmMachine* machine = *iter++;
		if (machine->get_ademco_id() == ademco_id) {
			if (wcscmp(machine->GetDeviceIDW(), device_id) == 0) {
				return TRUE;
			}
		}
	}

	return FALSE;
}


BOOL CAlarmMachineManager::CheckMachine(const wchar_t* device_id)
{
	std::list<CAlarmMachine*>::iterator iter = m_listAlarmMachine.begin();
	while (iter != m_listAlarmMachine.end()) {
		CAlarmMachine* machine = *iter++;
		if (wcscmp(machine->GetDeviceIDW(), device_id) == 0) {
			return TRUE;
		}
	}

	return FALSE;
}


BOOL CAlarmMachineManager::DistributeAdemcoID(int& ademco_id)
{
	if (m_listAlarmMachine.size() >= MAX_MACHINE) {
		return FALSE;
	}

	BOOL ok = FALSE;
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

	return ok;
}


BOOL CAlarmMachineManager::AddMachine(int ademco_id, const wchar_t* device_id, const wchar_t* alias)
{
	CAlarmMachine* machine = new CAlarmMachine();
	machine->set_ademco_id(ademco_id);
	machine->SetDeviceID(device_id);
	machine->SetAlias(alias);

	std::list<CAlarmMachine*>::iterator pos = std::find(m_listAlarmMachine.begin(), 
														m_listAlarmMachine.end(), 
														machine);
	m_listAlarmMachine.insert(pos, machine);

	return TRUE;
}


void CAlarmMachineManager::MachineOnline(int ademco_id, BOOL online)
{
	//CAlarmMachine* machine = new CAlarmMachine();
	//machine->SetAdemcoID(ademco_id);
	////machine->SetDeviceID(device_id);
	////machine->SetAlias(alias);

	//std::list<CAlarmMachine*>::iterator pos = std::find(m_listAlarmMachine.begin(),
	//													m_listAlarmMachine.end(),
	//													machine);
	//online;
	CAlarmMachine* machine = NULL;
	if (GetMachine(ademco_id, machine) && machine) {
		machine->SetStatus(online ? MS_ONLINE : MS_OFFLINE);
	}
}


void CAlarmMachineManager::MachineEventHandler(int ademco_id, int ademco_event, int zone)
{
	CAlarmMachine* machine = NULL;
	if (GetMachine(ademco_id, machine) && machine) {
		switch (ademco_event) {	
			case ademco::EVENT_ARM:
				machine->SetStatus(MS_ARM);
				break;
			case ademco::EVENT_DISARM:
				machine->SetStatus(MS_DISARM);
				break;
			case ademco::EVENT_HALFARM:
				machine->SetStatus(MS_HALFARM);
				break;
			case ademco::EVENT_EMERGENCY:
				machine->SetStatus(MS_EMERGENCY);
				break;
			case ademco::EVENT_BURGLAR:
				machine->SetStatus(MS_BUGLAR);
				break;
			case ademco::EVENT_FIRE:
				machine->SetStatus(MS_FIRE);
				break;
			case ademco::EVENT_DURESS:
				machine->SetStatus(MS_DURESS);
				break;
			case ademco::EVENT_GAS:
				machine->SetStatus(MS_GAS);
				break;
			case ademco::EVENT_WATER:
				machine->SetStatus(MS_WATER);
				break;
			case ademco::EVENT_TEMPER:
				machine->SetStatus(MS_TEMPER);
				break;
			case ademco::EVENT_LOWBATTERY:
				machine->SetStatus(MS_LOWBATTERY);
				break;
			case ademco::EVENT_SOLARDISTURB:
				machine->SetStatus(MS_SOLARDISTURB);
				break;
			case ademco::EVENT_DISCONNECT:
				machine->SetStatus(MS_DISCONNECT);
				break;
			case ademco::EVENT_SERIAL485DIS:
				machine->SetStatus(MS_SERIAL485DIS);
				break;
			case ademco::EVENT_SERIAL485CONN:
				machine->SetStatus(MS_SERIAL485CONN);
				break;
			case ademco::EVENT_DOORRINGING:
				machine->SetStatus(MS_DOORRINGING);
				break;
			default:
				machine->SetStatus(MS_ONLINE);
				break;
		}
	}
	zone;
}





NAMESPACE_END
