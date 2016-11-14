#include "stdafx.h"
#include "consumer.h"
#include "../contrib/sqlitecpp/SQLiteCpp.h"
#include "AppResource.h"

namespace core {

///////////////////////////// consumer_manager implement //////////////////////////
consumer_manager::consumer_manager()
{
	AUTO_LOG_FUNCTION;
	using namespace SQLite;
	auto path = get_config_path() + "\\service.db3";
	db_ = std::make_shared<Database>(path, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
	assert(db_);
	if (!db_) { return; }

	try {
		// check if db empty
		bool exists = false;
		{
			Statement query(*db_, "select name from sqlite_master where type='table'");
			if (!query.executeStep()) {
				// init tables
				db_->exec("drop table if exists table_consumer_type");
				db_->exec("drop table if exists consumers");
				db_->exec("create table table_consumer_type (id integer primary key AUTOINCREMENT, type_name text)");
				db_->exec("create table table_consumers (id integer primary key AUTOINCREMENT, \
ademco_id integer, \
zone_value integer, \
type_id integer, \
receivable_amount integer, \
paid_amount integer, \
remind_time text)");

				// init some default consumer types
				CString sql;
				sql.Format(L"insert into table_consumer_type values(NULL, \"%s\")", TR(IDS_STRING_CONSUMER_T_HOME));
				db_->exec(utf8::w2a((LPCTSTR)sql));
				sql.Format(L"insert into table_consumer_type values(NULL, \"%s\")", TR(IDS_STRING_CONSUMER_T_SHOP));
				db_->exec(utf8::w2a((LPCTSTR)sql));
				sql.Format(L"insert into table_consumer_type values(NULL, \"%s\")", TR(IDS_STRING_CONSUMER_T_OFFICE));
				db_->exec(utf8::w2a((LPCTSTR)sql));

			} else {
				exists = true;
			}
		}

		if (exists) {
			db_->exec("alter table table_consumers rename to table_consumers_old");
			db_->exec("create table table_consumers (id integer primary key AUTOINCREMENT, \
ademco_id integer, \
zone_value integer, \
type_id integer, \
receivable_amount integer, \
paid_amount integer, \
remind_time text)");

			{
				SQLite::Statement query(*db_, "insert into table_consumers (ademco_id, zone_value, type_id, remind_time) select ademco_id, zone_value, type_id, remind_time from table_consumers_old");
				query.exec();
				while (!query.isDone()) {
					std::this_thread::sleep_for(std::chrono::seconds(1));
				}
			}
			db_->exec("drop table table_consumers_old");
		}

		Statement query(*db_, "select * from table_consumer_type");

		while (query.executeStep()) {
			int id = static_cast<int>(query.getColumn(0));
			const char* name = query.getColumn(1);
			auto wname = utf8::a2w(name);
			add_type(id, wname.c_str());
		}
	} catch (std::exception& e) {
		JLOGA(e.what());
	}
}


consumer_list consumer_manager::load_consumers() const
{
	using namespace SQLite;
	consumer_list list;
	Statement query(*db_, "select * from table_consumers");

	int id, ademco_id, zone_value, type_id, receivable_amount, paid_amount;
	std::string remind_time;
	while (query.executeStep()) {
		int ndx = 0;
		id = query.getColumn(ndx++);
		ademco_id = query.getColumn(ndx++);
		zone_value = query.getColumn(ndx++);
		type_id = query.getColumn(ndx++);
		receivable_amount = query.getColumn(ndx++);
		paid_amount = query.getColumn(ndx++);
		remind_time = query.getColumn(ndx++).getText();

		auto consumer_type = get_consumer_type_by_id(type_id);
		if (consumer_type) {
			auto a_consumer = std::make_shared<consumer>(id, ademco_id, zone_value, consumer_type, receivable_amount, paid_amount,
														 string_to_time_point(remind_time));
			list.push_back(a_consumer);
		}
	}

	return list;
}


consumer_ptr consumer_manager::execute_add_consumer(int ademco_id, int zone_value, const consumer_type_ptr& type,
													int receivable_amount, int paid_amount, const std::chrono::system_clock::time_point& remind_time)
{
	using namespace SQLite;
	assert(type); if (!type) {
		return nullptr;
	}
	CString sql;
	sql.Format(L"insert into table_consumers ([ademco_id],[zone_value],[type_id],[receivable_amount],[paid_amount],[remind_time]) values(%d,%d,%d,%d,%d,'%s')",
			   ademco_id, zone_value, type->id, receivable_amount, paid_amount, time_point_to_wstring(remind_time).c_str());
	db_->exec(utf8::w2a((LPCTSTR)sql));
	int id = static_cast<int>(db_->getLastInsertRowid());
	return std::make_shared<consumer>(id, ademco_id, zone_value, type, receivable_amount, paid_amount, remind_time);
}


bool consumer_manager::execute_delete_consumer(const consumer_ptr& consumer)
{
	CString sql;
	sql.Format(L"delete from table_consumers where id=%d", consumer->id);
	if (db_->exec(utf8::w2a((LPCTSTR)sql)) > 0) {
		return true;
	}
	return false;
}


bool consumer_manager::execute_update_consumer(const consumer_ptr& consumer)
{
	CString sql;
	sql.Format(L"update table_consumers set type_id=%d,receivable_amount=%d,paid_amount=%d,remind_time='%s' where id=%d",
			   consumer->type->id, consumer->receivable_amount, consumer->paid_amount,
			   time_point_to_wstring(consumer->remind_time).c_str(), consumer->id);
	return db_->exec(utf8::w2a((LPCTSTR)sql)) > 0;
}


consumer_manager::~consumer_manager()
{

}


bool consumer_manager::execute_add_type(int& id, const CString& type_name)
{
	CString sql;
	sql.Format(L"insert into table_consumer_type ([type_name]) values('%s')", type_name);
	db_->exec(utf8::w2a((LPCTSTR)sql));
	id = static_cast<int>(db_->getLastInsertRowid());
	add_type(id, (LPCTSTR)type_name);
	return true;

}


bool consumer_manager::execute_rename(int id, const CString& new_name)
{
	CString sql;
	sql.Format(L"update table_consumer_type set type_name='%s' where id=%d", new_name, id);
	if (db_->exec(utf8::w2a((LPCTSTR)sql)) == 1) {
		consumer_type_map_[id]->name = (LPCTSTR)new_name;
		return true;
	}

	return false;
}
/////////////////// end of consumer / consumer_manager implement ////////////////////



}
