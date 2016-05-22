#include "stdafx.h"
#include "Sms.h"
#include "sqlitecpp/SQLiteCpp.h"

using namespace SQLite;

namespace core {

IMPLEMENT_SINGLETON(sms_manager)
sms_manager::sms_manager()
	: db_(nullptr)
{
	AUTO_LOG_FUNCTION;
	
	auto path = get_config_path() + "\\sms.db3";
	db_ = std::make_shared<Database>(path, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
	assert(db_);
	if (!db_) { return; }

	try {
		// check if db empty
		{
			Statement query(*db_, "select name from sqlite_master where type='table'");
			if (!query.executeStep()) {
				// init tables
				db_->exec("drop table if exists sms_config");
				db_->exec("create table sms_config (id integer primary key, is_submachine integer, ademco_id integer, zone_value integer,  report_alarm integer, report_exception integer, report_status integer, report_alarm_bk integer, report_exception_bk integer, report_status_bk integer)");
			} else {
				std::string name = query.getColumn(0);
				JLOGA(name.c_str());
				while (query.executeStep()) {
					name = query.getColumn(0).getText();
					JLOGA(name.c_str());
				}
			}
		}




	} catch (std::exception& e) {
		JLOGA(e.what());
	}
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
	
	db_->exec(utf8::w2a((LPCTSTR)sql));
	cfg.id = static_cast<int>(db_->getLastInsertRowid());
	return true;
}


bool sms_manager::del_sms_config(int id)
{
	AUTO_LOG_FUNCTION;
	CString sql(L"");
	sql.Format(L"delete from sms_config where id=%d", id);
	return db_->exec(utf8::w2a((LPCTSTR)sql)) > 0;
}


bool sms_manager::get_sms_config(bool is_submachine, int ademco_id, int zone_value, sms_config& cfg)
{
	
	AUTO_LOG_FUNCTION;
	CString sql = L"";
	sql.Format(L"select id,report_alarm,report_exception,report_status,report_alarm_bk,report_exception_bk,report_status_bk from sms_config where is_submachine=%d and ademco_id=%d and zone_value=%d",
			   is_submachine, ademco_id, zone_value);
	
	auto sqla = utf8::w2a((LPCTSTR(sql)));
	try {
		Statement query(*db_, sqla);
		if (query.executeStep()) {
			cfg.id = static_cast<int>(query.getColumn(0));
			cfg.report_alarm = query.getColumn(1).getInt() > 0;
			cfg.report_exception = query.getColumn(2).getInt() > 0;
			cfg.report_status = query.getColumn(3).getInt() > 0;
			cfg.report_alarm_bk = query.getColumn(4).getInt() > 0;
			cfg.report_exception_bk = query.getColumn(5).getInt() > 0;
			cfg.report_status_bk = query.getColumn(6).getInt() > 0;
			return true;
		}
	} catch (std::exception& e) {
		JLOGA(e.what());
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
	return db_->exec(utf8::w2a(LPCTSTR(sql))) > 0;
}



};
