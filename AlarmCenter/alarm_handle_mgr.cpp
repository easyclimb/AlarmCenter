#include "stdafx.h"
#include "alarm_handle_mgr.h"
#include "../contrib/sqlitecpp/SQLiteCpp.h"
#include "AppResource.h"

using namespace SQLite;

namespace core {

class alarm_handle_mgr::alarm_handle_mgr_impl 
{
public:
	std::unique_ptr<SQLite::Database> db_;

	std::wstring get_alarm_judgement_type_text(int judge) {
		switch (judge) {
		case core::alarm_judgement_by_video_image:
			return tr(IDS_STRING_BY_VIDEO_AND_IMAGE);
			break;
		case core::alarm_judgement_by_confirm_with_owner:
			return tr(IDS_STRING_BY_CONFIRM_WITH_OWNER);
			break;
		case core::alarm_judgement_by_platform_tip:
			return tr(IDS_STRING_BY_PLATFORM_PROMPT);
			break;
		default:
			return std::wstring(L"invalid alarm judgement type");
			break;
		}
	}

	alarm_handle_mgr_impl(){}

	void init_db(std::map<int, std::wstring>& alarm_types) {
		using namespace SQLite;
		auto path = get_config_path() + "\\alarm.db3";
		db_ = std::make_unique<Database>(path, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
		assert(db_);

		try {
			// check if db empty
			bool exists = false;
			{
				Statement query(*db_, "select name from sqlite_master where type='table'");
				if (!query.executeStep()) {
					// init tables

					db_->exec("create table table_judgement_types \
(id integer primary key AUTOINCREMENT, \
desc text)");

					std::stringstream ss;
					for (int judge = alarm_judgement_min + 1; judge < alarm_judgement_max; judge++) {
						alarm_types[judge] = get_alarm_judgement_type_text(judge);
						ss << "insert into table_judgement_types (id, desc) values (" << judge
							<< ", " << utf8::w2a(double_quotes(get_alarm_judgement_type_text(judge))) << ")";
						db_->exec(ss.str());
						ss.str(""); ss.clear();
					}

					db_->exec("create table table_judgement \
(id integer primary key AUTOINCREMENT, \
judgement_type_id integer, \
note text, \
note1 text, \
note2 text)");					

					db_->exec("create table table_guard \
(id integer primary key AUTOINCREMENT, \
name text, \
phone text, \
status integer)");

					db_->exec("create table table_handle \
(id integer primary key AUTOINCREMENT, \
guard_id integer, \
time_assigned text, \
time_handled text, \
pridict_minutes integer, \
note text)");

					db_->exec("create table table_reason \
(id integer primary key AUTOINCREMENT, \
reason integer, \
detail text, \
attach text)");

					db_->exec("create table table_alarm \
(id integer primary key AUTOINCREMENT, \
aid integer, \
zone integer, \
gg integer, \
alarm_text text, \
alarm_date text, \
judgement_id integer, \
handle_id integer, \
reason_id integer, \
status integer)");

					

				} else {
					exists = true;
				}
			}

		} catch (std::exception& e) {
			JLOGA(e.what());
		}

	}
};

alarm_handle_mgr::alarm_handle_mgr()
{
	impl_ = std::make_unique<alarm_handle_mgr_impl>();
	impl_->init_db(buffered_alarm_judgement_types_);
}

alarm_handle_mgr::~alarm_handle_mgr()
{
	impl_ = nullptr;
}

alarm_judgement_type_info alarm_handle_mgr::get_first_user_defined_judgement() const
{
	auto iter = buffered_alarm_judgement_types_.find(alarm_judgement_max);
	if (iter != buffered_alarm_judgement_types_.end()) {
		return *iter;
	}
	return create_alarm_judgement_type_info();
}

alarm_judgement_type_info alarm_handle_mgr::get_alarm_judgement_type_info(int id)
{
	auto iter = buffered_alarm_judgement_types_.find(id);
	if (iter != buffered_alarm_judgement_types_.end()) {
		return *iter;
	}
	return create_alarm_judgement_type_info();
}

auto alarm_handle_mgr::get_alarm_judgement(int id)
{
	auto iter = buffered_alarm_judgements_.find(id);
	if (iter != buffered_alarm_judgements_.end()) {
		return iter->second;
	}
	return alarm_judgement_ptr();
}

auto alarm_handle_mgr::get_security_guard(int id)
{
	auto iter = buffered_security_guards_.find(id);
	if (iter != buffered_security_guards_.end()) {
		return iter->second;
	}
	return security_guard_ptr();
}

auto alarm_handle_mgr::get_alarm_handle(int id)
{
	auto iter = buffered_alarm_handles_.find(id);
	if (iter != buffered_alarm_handles_.end()) {
		return iter->second;
	}
	return alarm_handle_ptr();
}

auto alarm_handle_mgr::get_alarm_reason(int id)
{
	auto iter = buffered_alarm_reasons_.find(id);
	if (iter != buffered_alarm_reasons_.end()) {
		return iter->second;
	}
	return alarm_reason_ptr();
}

auto alarm_handle_mgr::get_alarm_info(int id)
{
	auto iter = buffered_alarms_.find(id);
	if (iter != buffered_alarms_.end()) {
		return iter->second;
	}
	return alarm_ptr();
}







}