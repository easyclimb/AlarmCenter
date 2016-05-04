#include "stdafx.h"
#include "Sms.h"
#include "DbOper.h"

namespace core {

IMPLEMENT_SINGLETON(sms_manager)
sms_manager::sms_manager()
	: m_db(nullptr)
{
	m_db = std::make_shared<ado::CDbOper>();
	m_db->Open(L"sms.mdb");
}


sms_manager::~sms_manager()
{
}


bool sms_manager::add_sms_config(bool is_submachine, int ademco_id, int zone_value, sms_config& cfg)
{
	AUTO_LOG_FUNCTION;
	CString sql = L"";
	sql.Format(L"insert into sms_config([is_submachine],[ademco_id],[zone_value],[report_alarm],[report_exception],[report_status],[report_alarm_bk],[report_exception_bk],[report_status_bk]) values(%d,%d,%d,%d,%d,%d,%d,%d,%d)",
			   is_submachine, ademco_id, zone_value,
			   cfg.report_alarm, cfg.report_exception, cfg.report_status,
			   cfg.report_alarm_bk, cfg.report_exception_bk, cfg.report_status_bk);
	if (m_db->Execute(sql)) {
		ado::CADORecordset recordset(m_db->GetDatabase());
		recordset.Open(m_db->GetDatabase()->m_pConnection, L"select @@identity as _id_");
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


bool sms_manager::del_sms_config(int id)
{
	AUTO_LOG_FUNCTION;
	CString sql(L"");
	sql.Format(L"delete from sms_config where id=%d", id);
	return m_db->Execute(sql) ? true : false;
}


bool sms_manager::get_sms_config(bool is_submachine, int ademco_id, int zone_value, sms_config& cfg)
{
	AUTO_LOG_FUNCTION;
	CString sql = L"";
	sql.Format(L"select ID,report_alarm,report_exception,report_status,report_alarm_bk,report_exception_bk,report_status_bk from sms_config where is_submachine=%d and ademco_id=%d and zone_value=%d",
			   is_submachine, ademco_id, zone_value);
	ado::CADORecordset recordset(m_db->GetDatabase());
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


bool sms_manager::set_sms_config(const sms_config& cfg)
{
	AUTO_LOG_FUNCTION;
	CString sql = L"";
	sql.Format(L"update sms_config set report_alarm=%d,report_exception=%d,report_status=%d,report_alarm_bk=%d,report_exception_bk=%d,report_status_bk=%d where id=%d",
			   cfg.report_alarm, cfg.report_exception, cfg.report_status,
			   cfg.report_alarm_bk, cfg.report_exception_bk, cfg.report_status_bk,
			   cfg.id);
	return m_db->Execute(sql) ? true : false;
}

NAMESPACE_END
