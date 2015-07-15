#include "stdafx.h"
#include "Sms.h"
#include <odbcinst.h>
#include <afxdb.h>
#include <comdef.h>
#include "ado2.h"

namespace core {

IMPLEMENT_SINGLETON(CSms)
CSms::CSms()
	: m_pDatabase(NULL)

{
	try {
		m_pDatabase = new ado::CADODatabase();
		LOG(_T("CSms after new, m_pDatabase %x"), m_pDatabase);
		LPCTSTR pszMdb = L"sms.mdb";
		TCHAR szMdbPath[1024];
		_tcscpy_s(szMdbPath, GetModuleFilePath());
		_tcscat_s(szMdbPath, _T("\\config"));
		CreateDirectory(szMdbPath, NULL);
		_tcscat_s(szMdbPath, _T("\\"));
		_tcscat_s(szMdbPath, pszMdb);
		CLog::WriteLog(_T("CSms before pathexists"));
		if (!CFileOper::PathExists(szMdbPath)) {
			MessageBox(NULL, L"File 'sms.mdb' missed or broken!", L"Error", MB_OK | MB_ICONERROR);
			ExitProcess(0);
			return;
		}
		TRACE(_T("after pathexists"));

		CString strConn = _T("");
		strConn.Format(_T("Provider=Microsoft.Jet.OLEDB.4.0; Data Source='%s';Jet OLEDB:Database"), szMdbPath);
		CLog::WriteLog(strConn);
		if (!m_pDatabase->Open(strConn)) {
			TRACE(_T("CSms m_pDatabase->Open() error"));
			MessageBox(NULL, L"File sms.mdb missed or broken!", L"Error", MB_OK | MB_ICONERROR);
			ExitProcess(0);
		} else {
			LOG(_T("m_pDatabase->Open() ok"));
			LOG(_T("CSms ConnectDB %s success\n"), strConn);
		}
	} catch (...) {
		AfxMessageBox(_T("connect to access error!"));
		ExitProcess(0);
	}
}


CSms::~CSms()
{
	if (m_pDatabase) {
		if (m_pDatabase->IsOpen()) {
			m_pDatabase->Close();
		}
		delete m_pDatabase;
	}
}


bool CSms::add_sms_config(bool is_submachine, int ademco_id, int zone_value, SmsConfigure& cfg)
{
	AUTO_LOG_FUNCTION;
	CString sql = L"";
	sql.Format(L"insert into sms_config([is_submachine],[ademco_id],[zone_value],[report_alarm],[report_exception],[report_status],[report_alarm_bk],[report_exception_bk],[report_status_bk]) values(%d,%d,%d,%d,%d,%d,%d,%d,%d)",
			   is_submachine, ademco_id, zone_value,
			   cfg.report_alarm, cfg.report_exception, cfg.report_status,
			   cfg.report_alarm_bk, cfg.report_exception_bk, cfg.report_status_bk);
	if (m_pDatabase->Execute(sql)) {
		ado::CADORecordset recordset(m_pDatabase);
		recordset.Open(m_pDatabase->m_pConnection, L"select @@identity as _id_");
		DWORD count = recordset.GetRecordCount();
		if (count == 1) {
			recordset.MoveFirst();
			recordset.GetFieldValue(L"_id_", cfg.id);
			recordset.Close();
			return true;
		}
	}
	return false;
}


bool CSms::del_sms_config(int id)
{
	AUTO_LOG_FUNCTION;
	CString sql(L"");
	sql.Format(L"delete from sms_config where id=%d", id);
	return m_pDatabase->Execute(sql) ? true : false;
}


bool CSms::get_sms_config(bool is_submachine, int ademco_id, int zone_value, SmsConfigure& cfg)
{
	AUTO_LOG_FUNCTION;
	CString sql = L"";
	sql.Format(L"select ID,report_alarm,report_exception,report_status,report_alarm_bk,report_exception_bk,report_status_bk from sms_config where is_submachine=%d and ademco_id=%d and zone_value=%d",
			   is_submachine, ademco_id, zone_value);
	ado::CADORecordset recordset(m_pDatabase);
	if (recordset.Open(sql) && recordset.GetRecordCount() > 0) {
		recordset.GetFieldValue(L"ID", cfg.id);
		recordset.GetFieldValue(L"report_alarm", cfg.report_alarm);
		recordset.GetFieldValue(L"report_exception", cfg.report_exception);
		recordset.GetFieldValue(L"report_status", cfg.report_status);
		recordset.GetFieldValue(L"report_alarm_bk", cfg.report_alarm_bk);
		recordset.GetFieldValue(L"report_exception_bk", cfg.report_exception_bk);
		recordset.GetFieldValue(L"report_status_bk", cfg.report_status_bk);
		recordset.Close();
		return true;
	}
	return false;
}


bool CSms::set_sms_config(const SmsConfigure& cfg)
{
	AUTO_LOG_FUNCTION;
	CString sql = L"";
	sql.Format(L"update sms_config set report_alarm=%d,report_exception=%d,report_status=%d,report_alarm_bk=%d,report_exception_bk=%d,report_status_bk=%d where id=%d",
			   cfg.report_alarm, cfg.report_exception, cfg.report_status,
			   cfg.report_alarm_bk, cfg.report_exception_bk, cfg.report_status_bk,
			   cfg.id);
	return m_pDatabase->Execute(sql) ? true : false;
}

NAMESPACE_END
