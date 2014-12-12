#include "stdafx.h"
#include "AlarmMachineManager.h"
#include "ado2.h"
#include "AlarmMachine.h"

CLock CAlarmMachineManager::m_lock;
static CAlarmMachineManager* g_pInstance = NULL;

CAlarmMachineManager::CAlarmMachineManager()
	: m_pDatabase(NULL)
{
	LoadAlarmMachineFromDB();
}


CAlarmMachineManager::~CAlarmMachineManager()
{

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


void CAlarmMachineManager::LoadAlarmMachineFromDB()
{
	try {
		m_pDatabase = new CADODatabase();
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

	static const wchar_t* query = L"select * from AlarmMachine order by ID";
	CADORecordset recordset(m_pDatabase);
	recordset.Open(m_pDatabase->m_pConnection, query);
	DWORD count = recordset.GetRecordCount();
	if (count > 0) {
		recordset.MoveFirst();
		for (DWORD i = 0; i < count; i++) {
			CAlarmMachine *machine = new CAlarmMachine();
			int id, ademco_id;
			CString device_id;
			recordset.GetFieldValue(L"id", id);
			recordset.GetFieldValue(L"AdemcoID", ademco_id);
			recordset.GetFieldValue(L"DeviceID", device_id);
			machine->SetID(id);
			machine->SetAdemcoID(ademco_id);
			machine->SetDeviceID(device_id);
			m_vectorAlarmMachine.insert(machine);
		}
	}
}


int CAlarmMachineManager::GetMachineCount() const
{
	return m_vectorAlarmMachine.size();
}


BOOL CAlarmMachineManager::GetMachine(int id, CAlarmMachine*& machine)
{
	if (id >= 0 && id < m_vectorAlarmMachine.size()) {
		machine = m_vectorAlarmMachine[id];
		return TRUE;
	}
	return FALSE;
}
