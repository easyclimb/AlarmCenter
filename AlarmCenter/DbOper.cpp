#include "stdafx.h"
#include "DbOper.h"


namespace ado {
CDbOper::CDbOper()
	: m_pDatabase(NULL)
{}


CDbOper::~CDbOper()
{
	if (m_pDatabase) {
		if (m_pDatabase->IsOpen()) {
			m_pDatabase->Close();
		}
		delete m_pDatabase;
	}
}


void CDbOper::Check()
{
	assert(m_pDatabase);
	if (!m_pDatabase) {
		LOG(L"check m_pDatabase is NULL, exit process!\n");
		ExitProcess(0);
	}
}


bool CDbOper::Open(const CString& mdbPath, const CString& passwd)
{
	AUTO_LOG_FUNCTION;
	if (m_pDatabase)
		return true;

	m_pDatabase = new ado::CADODatabase();
	TCHAR szMdbPath[1024];
	_tcscpy_s(szMdbPath, GetModuleFilePath());
	_tcscat_s(szMdbPath, _T("\\config"));
	CreateDirectory(szMdbPath, NULL);
	_tcscat_s(szMdbPath, _T("\\"));
	_tcscat_s(szMdbPath, mdbPath);
	if (!CFileOper::PathExists(szMdbPath)) {
		CString e; e.Format(L"File %s missed or broken!", mdbPath);
		MessageBox(NULL, e, L"Error", MB_OK | MB_ICONERROR); 
		ExitProcess(0);
		return false;
	}
	CString strConn = _T("");
	strConn.Format(_T("Provider=Microsoft.Jet.OLEDB.4.0; Data Source='%s';Jet OLEDB:Database Password='%s'"),
				   mdbPath, passwd);
	CLog::WriteLog(strConn);
	if (!m_pDatabase->Open(strConn)) {
		CString e; e.Format(L"File %s missed or broken!", mdbPath);
		MessageBox(NULL, e, L"Error", MB_OK | MB_ICONERROR);
		return false;
	} else {
		return true;
	}
}


bool CDbOper::Execute(const CString& sql)
{
	Check();
	return m_pDatabase->Execute(sql) ? true : false;
}


int CDbOper::AddAutoIndexTableReturnID(const CString& query)
{
	AUTO_LOG_FUNCTION;
	if (!m_pDatabase)
		return -1;

	if (!Execute(query))
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



NAMESPACE_END
