#include "stdafx.h"
#include "AlarmMachineManager.h"
#include "ado2.h"
#include "AlarmMachine.h"

namespace core {

CLock CAlarmMachineManager::m_lock;
static CAlarmMachineManager* g_pInstance = NULL;

CAlarmMachineManager::CAlarmMachineManager()
	: m_pDatabase(NULL)
{
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


void CAlarmMachineManager::LoadAlarmMachineFromDB()
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
			machine->SetID(id);
			machine->SetAdemcoID(ademco_id);
			machine->SetDeviceID(device_id);
			machine->SetAlias(alias);
			m_listAlarmMachine.push_back(machine);
			recordset.MoveNext();
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
		if (local_machine->GetAdemcoID() == ademco_id) {
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
		if (machine->GetAdemcoID() == ademco_id) {
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
	if (machine->GetAdemcoID() >= MAX_MACHINE - 1) {
		int temp_id = 0;
		std::list<CAlarmMachine*>::iterator iter = m_listAlarmMachine.begin();
		while (iter != m_listAlarmMachine.end()) {
			CAlarmMachine* machine = *iter++;
			if (machine->GetAdemcoID() != temp_id++) {
				ademco_id = temp_id;
				ok = TRUE;
				break;
			}
		}
	} else {
		ademco_id = machine->GetAdemcoID() + 1;
		ok = TRUE;
	}

	return ok;
}


BOOL CAlarmMachineManager::AddMachine(int ademco_id, const wchar_t* device_id, const wchar_t* alias)
{

	return FALSE;
}


void CAlarmMachineManager::MachineOnline(int ademco_id, BOOL online = TRUE)
{
	
}


void CAlarmMachineManager::MachineEventHandler(int ademco_id, int ademco_event, int zone)
{

}





NAMESPACE_END
