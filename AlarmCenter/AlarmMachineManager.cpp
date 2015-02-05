#include "stdafx.h"
#include "AlarmMachineManager.h"
#include "ado2.h"
#include "AlarmMachine.h"
#include "ademco_func.h"
#include "MapInfo.h"
#include "ZoneInfo.h"
#include "ZonePropertyInfo.h"
#include "DetectorInfo.h"
#include "DetectorLib.h"
#include "ConfigHelper.h"

namespace core {

CLock CAlarmMachineManager::m_lock;
static CAlarmMachineManager* g_pInstance = NULL;

CAlarmMachineManager::CAlarmMachineManager()
	: m_pDatabase(NULL)
	//, m_detectorLib(NULL)
{
	InitDB();
	InitDetectorLib();
	LoadDetectorLibFromDB();
	LoadZonePropertyInfoFromDB();
	LoadAlarmMachineFromDB();
	
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

	//if (m_detectorLib) { delete m_detectorLib; }
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


const char* CAlarmMachineManager::GetCsrAcctA() const
{
	return m_csr_acctA;
}


const wchar_t* CAlarmMachineManager::GetCsrAcctW() const
{
	return m_csr_acctW;
}


void CAlarmMachineManager::SetCsrAcct(const char* csr_acct)
{
	strcpy_s(m_csr_acctA, csr_acct);
	USES_CONVERSION;
	wcscpy_s(m_csr_acctW, A2W(csr_acct));
}


void CAlarmMachineManager::SetCsrAcct(const wchar_t* csr_acct)
{
	wcscpy_s(m_csr_acctW, csr_acct);
	USES_CONVERSION;
	strcpy_s(m_csr_acctA, W2A(csr_acct));
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
		ApplicationLanguage lang = CConfigHelper::GetInstance()->GetLanguage();
		switch (lang) {
			case AL_CHINESE:condition = 0;
				break;
			case AL_ENGLISH:condition = 2;
				break;
			case AL_TAIWANESE:condition = 1;
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
		query.Format(format, DT_SINGLE,
					 TRIPLE_CONDITION(condition, _T("卧室主机HB-3030C"), _T("臥室主機HB-3030C"), _T("HB-3030C")),
					 detPath + _T("HB-3030C.bmp"), L"", ALN_0, ALG_0);
		VERIFY(m_pDatabase->Execute(query));

		// 液晶主机HB-BJQ-560
		query.Format(format, DT_SINGLE,
					 TRIPLE_CONDITION(condition, _T("液晶主机HB-BJQ-560"), _T("液晶主機HB-BJQ-560"), _T("HB-BJQ-560")),
					 detPath + _T("HB-BJQ-560.bmp"), L"", ALN_0, ALG_0);
		VERIFY(m_pDatabase->Execute(query));
	}
	CLog::WriteLog(_T("CDBOper::InitData() ok"));
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
			int id, ademco_id, banned;
			CString device_id, alias, contact, address, phone, phone_bk;
			recordset.GetFieldValue(L"id", id);
			recordset.GetFieldValue(L"AdemcoID", ademco_id);
			recordset.GetFieldValue(L"DeviceID", device_id);
			recordset.GetFieldValue(L"Banned", banned);
			recordset.GetFieldValue(L"Alias", alias);
			recordset.GetFieldValue(L"contact", contact);
			recordset.GetFieldValue(L"address", address);
			recordset.GetFieldValue(L"phone", phone);
			recordset.GetFieldValue(L"phone_bk", phone_bk);
			machine->set_id(id);
			machine->set_ademco_id(ademco_id);
			machine->set_device_id(device_id);
			machine->set_banned(banned != 0);
			machine->set_alias(alias);
			machine->set_contact(contact);
			machine->set_address(address);
			machine->set_phone(phone);
			machine->set_phone_bk(phone_bk);
			recordset.MoveNext();

			LoadMapInfoFromDB(machine);
			m_listAlarmMachine.push_back(machine);
			
		}
		//m_listAlarmMachine.sort();
	}
	recordset.Close();
}


void CAlarmMachineManager::LoadMapInfoFromDB(CAlarmMachine* machine)
{
	CString query;
	query.Format(L"select * from MapInfo where ademco_id=%d order by id", 
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
			recordset.MoveNext();
			CMapInfo* mapInfo = new CMapInfo();
			mapInfo->set_id(id);
			mapInfo->set_type(type);
			mapInfo->set_ademco_id(ademco_id);
			mapInfo->set_path(path);
			LoadZoneInfoFromDB(mapInfo);
			machine->AddMap(mapInfo);
		}
	}
	recordset.Close();
}


void CAlarmMachineManager::LoadZoneInfoFromDB(CMapInfo* mapInfo)
{
	CString query;
	query.Format(L"select * from ZoneInfo where map_id=%d order by zone_id",
				 mapInfo->get_id());
	ado::CADORecordset recordset(m_pDatabase);
	recordset.Open(m_pDatabase->m_pConnection, query);
	DWORD count = recordset.GetRecordCount();
	if (count > 0) {
		recordset.MoveFirst();
		for (DWORD i = 0; i < count; i++) {
			int id, zone_id, ademco_id, map_id, 
				detector_id, detector_property_id;
			CString alias;
			recordset.GetFieldValue(L"id", id);
			recordset.GetFieldValue(L"zone_id", zone_id);
			recordset.GetFieldValue(L"ademco_id", ademco_id);
			recordset.GetFieldValue(L"map_id", map_id);
			recordset.GetFieldValue(L"alias", alias);
			recordset.GetFieldValue(L"detector_info_id", detector_id);
			recordset.GetFieldValue(L"detector_property_info_id", detector_property_id);
			recordset.MoveNext();
			
			CZoneInfo* zone = new CZoneInfo();
			zone->set_id(id);
			zone->set_zone_id(zone_id);
			zone->set_ademco_id(ademco_id);
			zone->set_map_id(map_id);
			//zone->set_type(type);
			zone->set_alias(alias);
			zone->set_detector_id(detector_id);
			zone->set_detector_property_id(detector_property_id);
			LoadDetectorInfoFromDB(zone);
			mapInfo->AddZone(zone);
		}
	}
	recordset.Close();
}


void CAlarmMachineManager::LoadDetectorInfoFromDB(CZoneInfo* zone)
{
	CString query;
	query.Format(L"select * from DetectorInfo where id=%d",
				 zone->get_detector_id());
	ado::CADORecordset recordset(m_pDatabase);
	recordset.Open(m_pDatabase->m_pConnection, query);
	DWORD count = recordset.GetRecordCount();
	if (count > 0) {
		recordset.MoveFirst();
		for (DWORD i = 0; i < count; i++) {
			int id, /*zone_info_id, */x, y, distance, angle, detector_lib_id;
			recordset.GetFieldValue(L"id", id);
			//recordset.GetFieldValue(L"zone_info_id", zone_info_id);
			recordset.GetFieldValue(L"x", x);
			recordset.GetFieldValue(L"y", y);
			recordset.GetFieldValue(L"distance", distance);
			recordset.GetFieldValue(L"angle", angle);
			recordset.GetFieldValue(L"detector_lib_id", detector_lib_id);
			recordset.MoveNext();

			CDetectorInfo* detector = new CDetectorInfo();
			detector->set_id(id);
			//detector->set_zone_info_id(zone_info_id);
			detector->set_x(x);
			detector->set_y(y);
			detector->set_distance(distance);
			detector->set_angle(angle);
			detector->set_detector_lib_id(detector_lib_id);
			zone->SetDetectorInfo(detector);
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


void CAlarmMachineManager::LoadZonePropertyInfoFromDB()
{
	CString query;
	ApplicationLanguage lang = CConfigHelper::GetInstance()->GetLanguage();
	switch (lang) {
		case AL_CHINESE:
			query.Format(L"select id,zone_property,zone_property_text_ch as zone_property_text,zone_alarm_text_ch as zone_alarm_text from ZonePropertyInfo order by id");
			break;
		case AL_ENGLISH:
			query.Format(L"select id,zone_property,zone_property_text_en as zone_property_text,zone_alarm_text_en as zone_alarm_text from ZonePropertyInfo order by id");
			break;
		case AL_TAIWANESE:
			query.Format(L"select id,zone_property,zone_property_text_tw as zone_property_text,zone_alarm_text_tw as zone_alarm_text from ZonePropertyInfo order by id");
			break;
		default:
			ASSERT(0);
			break;
	}
	
	ado::CADORecordset recordset(m_pDatabase);
	recordset.Open(m_pDatabase->m_pConnection, query);
	DWORD count = recordset.GetRecordCount();
	if (count > 0) {
		CZonePropertyInfo* zonePropertyInfo = CZonePropertyInfo::GetInstance();
		recordset.MoveFirst();
		for (DWORD i = 0; i < count; i++) {
			int id, zone_property;
			CString zone_property_text, zone_alarm_text;
			recordset.GetFieldValue(L"id", id);
			recordset.GetFieldValue(L"zone_property", zone_property);
			recordset.GetFieldValue(L"zone_property_text", zone_property_text);
			recordset.GetFieldValue(L"zone_alarm_text", zone_alarm_text);
			recordset.MoveNext();

			CZonePropertyData* data = new CZonePropertyData();
			data->set_id(id);
			data->set_property(zone_property);
			data->set_property_text(zone_property_text);
			data->set_alarm_text(zone_alarm_text);
			zonePropertyInfo->AddZonePropertyData(data);
		}
	}
	recordset.Close();
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

BOOL CAlarmMachineManager::CheckMachine(int ademco_id, const char* device_id, int zone)
{
	if (zone < 0 || MAX_MACHINE_ZONE < zone)
		return FALSE;

	std::list<CAlarmMachine*>::iterator iter = m_listAlarmMachine.begin();
	while (iter != m_listAlarmMachine.end()) {
		CAlarmMachine* machine = *iter++;
		if (machine->get_ademco_id() == ademco_id) {
			if (strcmp(machine->GetDeviceIDA(), device_id) == 0) {
				return TRUE;
			}
		}
	}

	return FALSE;
}


BOOL CAlarmMachineManager::CheckMachine(const char* device_id)
{
	std::list<CAlarmMachine*>::iterator iter = m_listAlarmMachine.begin();
	while (iter != m_listAlarmMachine.end()) {
		CAlarmMachine* machine = *iter++;
		if (strcmp(machine->GetDeviceIDA(), device_id) == 0) {
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


BOOL CAlarmMachineManager::AddMachine(int ademco_id, const char* device_id, const wchar_t* alias)
{
	CAlarmMachine* machine = new CAlarmMachine();
	machine->set_ademco_id(ademco_id);
	machine->set_device_id(device_id);
	machine->set_alias(alias);

	std::list<CAlarmMachine*>::iterator pos = std::find(m_listAlarmMachine.begin(), 
														m_listAlarmMachine.end(), 
														machine);
	m_listAlarmMachine.insert(pos, machine);

	// todo: 写数据库

	return TRUE;
}


void CAlarmMachineManager::MachineOnline(int ademco_id, BOOL online)
{
	//CAlarmMachine* machine = new CAlarmMachine();
	//machine->SetAdemcoID(ademco_id);
	////machine->set_device_id(device_id);
	////machine->SetAlias(alias);

	//std::list<CAlarmMachine*>::iterator pos = std::find(m_listAlarmMachine.begin(),
	//													m_listAlarmMachine.end(),
	//													machine);
	//online;
	CAlarmMachine* machine = NULL;
	if (GetMachine(ademco_id, machine) && machine) {
		machine->SetAdemcoEvent(0, online ? MS_ONLINE : MS_OFFLINE);
	}
}


void CAlarmMachineManager::MachineEventHandler(int ademco_id, int ademco_event, int zone)
{
	CAlarmMachine* machine = NULL;
	if (GetMachine(ademco_id, machine) && machine) {
		machine->SetAdemcoEvent(zone, ademco_event);
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





NAMESPACE_END
