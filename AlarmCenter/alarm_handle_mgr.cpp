#include "stdafx.h"
#include "alarm_handle_mgr.h"
#include "../contrib/sqlitecpp/SQLiteCpp.h"
#include "AppResource.h"

using namespace SQLite;

namespace core {

std::wstring security_guard::get_status_text() const
{
	switch (status_) {
	case standing_by:
		return tr(IDS_STRING_GUARD_STATUS_STANDBY);
		break;
	case on_handle:
		return tr(IDS_STRING_GUARD_STATUS_HANDLING);
		break;
	case offline:
	default:
		return tr(IDS_STRING_OFFLINE);
		break;
	}
}

class alarm_handle_mgr::alarm_handle_mgr_impl 
{
public:
	std::unique_ptr<SQLite::Database> db_;

	/*int execute_ret_row_id(const std::string& sql) {
		return db_->exec(sql);
	}*/

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
		case core::alarm_judgement_by_user_define:
			return tr(IDS_STRING_USER_DEFINE);
			break;
		default:
			return std::wstring(L"invalid alarm judgement type");
			break;
		}
	}

	alarm_handle_mgr_impl(){}

	// return true for init ok, false for already exists and need not to init
	bool init_db(/*std::map<int, std::wstring>& alarm_types*/) {
		using namespace SQLite;
		auto path = get_config_path() + "\\alarm_v1.0.db3";
		db_ = std::make_unique<Database>(path, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
		assert(db_);

		// check if db empty
		bool exists = false;
		try {
			
			{
				Statement query(*db_, "select name from sqlite_master where type='table'");
				if (!query.executeStep()) {
					// init tables

					db_->exec("create table table_judgement_types \
(id integer primary key AUTOINCREMENT, \
desc text)");

					std::stringstream ss;
					for (int judge = alarm_judgement_min + 1; judge < alarm_judgement_by_user_define; judge++) {
						//alarm_types[judge] = get_alarm_judgement_type_text(judge);
						ss << "insert into table_judgement_types (id, desc) values (" << judge
							<< ", \"" << utf8::w2a(double_quotes(get_alarm_judgement_type_text(judge))) << "\")";
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

		return !exists;
	}

	void load_alarm_judgement_types(std::map<int, std::wstring>& alarm_types) {
		for (int judge = alarm_judgement_min + 1; judge < alarm_judgement_by_user_define; judge++) {
			alarm_types[judge] = get_alarm_judgement_type_text(judge);
		}

		std::stringstream ss;
		ss << "select * from table_judgement_types where id >= " << alarm_judgement_by_user_define;
		Statement query(*db_, ss.str());
		while (query.executeStep()) {
			int id = query.getColumn(0);
			std::wstring txt = utf8::a2w(query.getColumn(1).getText());
			alarm_types[id] = txt;
		}
	}

	void load_security_guards(std::map<int, security_guard_ptr>& guards) {
		Statement query(*db_, "select * from table_guard");
		while (query.executeStep()) {
			int id = query.getColumn(0);
			auto name = utf8::a2w(query.getColumn(1).getText());
			auto phone = utf8::a2w(query.getColumn(2).getText());
			auto status = security_guard::integer_to_status(query.getColumn(3));

			auto guard = create_security_guard();
			guard->id_ = id;
			guard->name_ = name;
			guard->phone_ = phone;
			guard->status_ = status;
			guards[id] = guard;
		}
	}

	void load_alarm_handles(std::map<int, alarm_handle_ptr>& handles) {
		Statement query(*db_, "select * from table_handle");
		while (query.executeStep()) {
			auto handle = create_alarm_handle();
			handle->id_ = query.getColumn(0);
			handle->guard_id_ = query.getColumn(1);
			handle->time_point_assigned_ = string_to_time_point(query.getColumn(2).getText());
			handle->time_point_handled_ = string_to_time_point(query.getColumn(3).getText());
			handle->predict_minutes_to_handle_ = std::chrono::minutes(query.getColumn(4));
			handle->note_ = utf8::a2w(query.getColumn(5).getText());

			handles[handle->get_id()] = handle;
		}
	}
};

alarm_handle_mgr::alarm_handle_mgr()
{
	impl_ = std::make_unique<alarm_handle_mgr_impl>();
	impl_->init_db();
	impl_->load_alarm_judgement_types(buffered_alarm_judgement_types_);
	impl_->load_security_guards(buffered_security_guards_);
	impl_->load_alarm_handles(buffered_alarm_handles_);
}

alarm_handle_mgr::~alarm_handle_mgr()
{
	impl_ = nullptr;
}

std::vector<alarm_judgement_type_info> alarm_handle_mgr:: get_all_user_defined_judgements() const
{
	std::vector<alarm_judgement_type_info> v;
	auto iter = buffered_alarm_judgement_types_.find(alarm_judgement_by_user_define);
	while (iter != buffered_alarm_judgement_types_.end()) {
		v.push_back(*iter++);
	}
	return v;
}

alarm_judgement_type_info alarm_handle_mgr::get_alarm_judgement_type_info(int id)
{
	auto iter = buffered_alarm_judgement_types_.find(id);
	if (iter != buffered_alarm_judgement_types_.end()) {
		return *iter;
	}
	return create_alarm_judgement_type_info();
}

alarm_judgement_type_info alarm_handle_mgr::execute_add_judgement_type(const std::wstring & txt)
{
	for (auto pair : buffered_alarm_judgement_types_) {
		if (pair.second == txt) {
			return pair;
		}
	}

	std::stringstream ss;
	ss << "insert into table_judgement_types (id, desc) values (NULL,\""
		<< utf8::w2a(double_quotes(txt)) << "\")";
	auto sql = ss.str();
	impl_->db_->exec(sql);
	int id = impl_->db_->getLastInsertRowid() & 0xFFFFFFFF;
	alarm_judgement_type_info info;
	info.first = id;
	info.second = txt;
	buffered_alarm_judgement_types_.insert(info);
	return info;
}

alarm_judgement_ptr alarm_handle_mgr::get_alarm_judgement(int id)
{
	auto iter = buffered_alarm_judgements_.find(id);
	if (iter != buffered_alarm_judgements_.end()) {
		return iter->second;
	}
	return alarm_judgement_ptr();
}

alarm_judgement_ptr alarm_handle_mgr::execute_add_judgment(int judgement_type_id,
														   const std::wstring & note, const std::wstring & note1, const std::wstring & note2)
{
	alarm_judgement_ptr judgment = nullptr;
	auto judgement_type = get_alarm_judgement_type_info(judgement_type_id);
	if (judgement_type.first != alarm_judgement::alarm_judgement_min) {
		std::stringstream ss;
		ss << "insert into table_judgement (judgement_type_id, note, note1, note2) values(" << judgement_type.first
			<< ", \"" << utf8::w2a(double_quotes(note))
			<< "\", \"" << utf8::w2a(double_quotes(note1))
			<< "\", \"" << utf8::w2a(double_quotes(note2)) << "\")";
		auto sql = ss.str();
		impl_->db_->exec(sql);
		int id = impl_->db_->getLastInsertRowid() & 0xFFFFFFFF;
		judgment = create_alarm_judgement_ptr(judgement_type.first, note, note1, note2);
		judgment->id_ = id;
		buffered_alarm_judgements_[id] = judgment;
	
	}

	return judgment;
}

security_guard_ptr alarm_handle_mgr::get_security_guard(int id)
{
	auto iter = buffered_security_guards_.find(id);
	if (iter != buffered_security_guards_.end()) {
		return iter->second;
	}
	return security_guard_ptr();
}

valid_data_ids alarm_handle_mgr::get_security_guard_ids() const
{
	valid_data_ids ids;
	for (auto item : buffered_security_guards_) {
		ids.push_back(item.first);
	}
	return ids;
}

security_guard_ptr alarm_handle_mgr::execute_add_security_guard(const std::wstring & name, const std::wstring & phone)
{
	std::stringstream ss;
	ss << "insert into table_guard (name, phone, status) values (\""
		<< utf8::w2a(double_quotes(name)) << "\",\"" << utf8::w2a(double_quotes(phone)) <<  "\"," << security_guard::offline << ")";
	auto sql = ss.str();
	impl_->db_->exec(sql);
	int id = impl_->db_->getLastInsertRowid() & 0xFFFFFFFF;
	auto guard = create_security_guard();
	guard->id_ = id;
	guard->name_ = name;
	guard->phone_ = phone;
	guard->status_ = security_guard::offline;
	buffered_security_guards_[id] = guard;
	return guard;
}

bool alarm_handle_mgr::execute_rm_security_guard(int id)
{
	if (buffered_security_guards_.find(id) != buffered_security_guards_.end()) {
		std::stringstream ss;
		ss << "delete from table_guard where id=" << id;
		auto sql = ss.str();
		impl_->db_->exec(sql);
		buffered_security_guards_.erase(id);
		if (buffered_security_guards_.empty()) {
			impl_->db_->exec("update sqlite_sequence set seq=0 where name='table_guard'");
		}
		return true;
	}
	return false;
}

bool alarm_handle_mgr::execute_update_security_guard_info(int id, const std::wstring& name, const std::wstring& phone)
{
	if (buffered_security_guards_.find(id) != buffered_security_guards_.end()) {
		std::stringstream ss;
		ss << "update table_guard set name=\"" << utf8::w2a(double_quotes(name))
			<< "\", phone=\"" << utf8::w2a(double_quotes(phone))
			<< "\" where id=" << id;
		auto sql = ss.str();
		if (impl_->db_->exec(sql)) {
			buffered_security_guards_[id]->name_ = name;
			buffered_security_guards_[id]->phone_ = phone;
			return true;
		}
	}
	return false;
}

int alarm_handle_mgr::allocate_alarm_handle_id() const
{
	if (!buffered_alarm_handles_.empty()) {
		return buffered_alarm_handles_.rbegin()->first;
	}
	return 1;
}

auto alarm_handle_mgr::get_alarm_handle(int id)
{
	auto iter = buffered_alarm_handles_.find(id);
	if (iter != buffered_alarm_handles_.end()) {
		return iter->second;
	}
	return alarm_handle_ptr();
}

alarm_handle_ptr alarm_handle_mgr::execute_add_alarm_handle(int guard_id, const std::chrono::minutes & predict_minutes, const std::wstring & note)
{
	auto guard = get_security_guard(guard_id);
	if (guard) {
		auto time_assigned = std::chrono::system_clock::now();
		auto sta = time_point_to_string(time_assigned);
		std::stringstream ss;
		ss << "insert into table_handle (guard_id, time_assigned, pridict_minutes, note) values ("
			<< guard_id << ", \"" << sta << "\", " << predict_minutes.count() << ", \"" << utf8::w2a(double_quotes(note)) << "\")";
		auto sql = ss.str();
		impl_->db_->exec(sql);
		int id = impl_->db_->getLastInsertRowid() & 0xFFFFFFFF;
		
		auto handle = create_alarm_handle();
		handle->id_ = id;
		handle->guard_id_ = guard_id;
		handle->time_point_assigned_ = time_assigned;
		handle->predict_minutes_to_handle_ = predict_minutes;
		handle->note_ = note;

		buffered_alarm_handles_[id] = handle;
		return handle;
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

alarm_reason_ptr alarm_handle_mgr::execute_add_alarm_reason(int reason, const std::wstring & detail, const std::wstring & attachment)
{
	alarm_reason::by by;
	switch (reason) {
	case core::alarm_reason::real_alarm:
		by = core::alarm_reason::real_alarm;
		break;
	case core::alarm_reason::device_false_positive:
		by = core::alarm_reason::device_false_positive;
		break;
	case core::alarm_reason::test_device:
		by = core::alarm_reason::test_device;
		break;
	case core::alarm_reason::man_made_false_positive:
		by = core::alarm_reason::man_made_false_positive;
		break;
	case core::alarm_reason::other_reasons:
	default:
		by = core::alarm_reason::other_reasons;
		break;
	}

	std::stringstream ss;
	ss << "insert into table_reason (reason,detail,attach) values(" << by << ",\"" << utf8::w2a(double_quotes(detail))
		<< "\",\"" << utf8::w2a(double_quotes(attachment)) << "\")";
	auto sql = ss.str();
	impl_->db_->exec(sql);
	int id = impl_->db_->getLastInsertRowid() & 0xFFFFFFFF;

	auto _reason = create_alarm_reason();
	_reason->id_ = id;
	_reason->reason_ = by;
	_reason->detail_ = detail;
	_reason->attach_ = attachment;

	buffered_alarm_reasons_[id] = _reason;

	return _reason;
}

auto alarm_handle_mgr::get_alarm_info(int id)
{
	auto iter = buffered_alarms_.find(id);
	if (iter != buffered_alarms_.end()) {
		return iter->second;
	}
	return alarm_ptr();
}

alarm_ptr alarm_handle_mgr::execute_add_alarm(int ademco_id, int zone, int gg, 
											  const std::wstring & alarm_text, 
											  const std::chrono::system_clock::time_point & alarm_time, 
											  int judgement_id, int handle_id, int reason_id)
{
	std::stringstream ss;
	ss << "insert into table_alarm "
		<< "(aid,zone,gg,alarm_text,alarm_date,judgement_id,handle_id,reason_id,status) "
		<< "values (" << ademco_id << "," << zone << "," << gg << ",\"" << utf8::w2a(double_quotes(alarm_text))
		<< "\",\"" << time_point_to_string(alarm_time) << "\"," << judgement_id << "," << handle_id
		<< "," << reason_id << "," << alarm_status::alarm_status_not_judged << ")";

	auto sql = ss.str();
	impl_->db_->exec(sql);
	int id = impl_->db_->getLastInsertRowid() & 0xFFFFFFFF;

	auto alarm = create_alarm_info();
	alarm->id_ = id;
	alarm->aid_ = ademco_id;
	alarm->zone_ = zone;
	alarm->gg_ = gg;
	alarm->text_ = alarm_text;
	alarm->date_ = time_point_to_wstring(alarm_time);
	alarm->judgement_id_ = judgement_id;
	alarm->handle_id_ = handle_id;
	alarm->reason_id_ = reason_id;
	alarm->status_ = alarm_status::alarm_status_not_judged;

	buffered_alarms_[id] = alarm;

	return alarm;
}

alarm_ptr alarm_handle_mgr::execute_update_alarm_judgment(int alarm_id, const alarm_judgement_ptr & judgment)
{
	auto alarm = get_alarm_info(alarm_id);
	if (alarm) {
		std::stringstream ss;
		ss << "update table_alarm set judgement_id=" << judgment->get_id() << " where id=" << alarm_id;
		impl_->db_->exec(ss.str());
		alarm->judgement_id_ = judgment->get_id();
		buffered_alarms_[alarm_id] = alarm;
	}

	return alarm;
}

alarm_ptr alarm_handle_mgr::execute_update_alarm_reason(int alarm_id, const alarm_reason_ptr & reason)
{
	auto alarm = get_alarm_info(alarm_id);
	if (alarm) {
		std::stringstream ss;
		ss << "update table_alarm set reason_id=" << reason->get_id() << " where id=" << alarm_id;
		impl_->db_->exec(ss.str());
		alarm->reason_id_ = reason->get_id();
		buffered_alarms_[alarm_id] = alarm;
	}

	return alarm;
}

alarm_ptr alarm_handle_mgr::execute_update_alarm_handle(int alarm_id, const alarm_handle_ptr & handle)
{
	auto alarm = get_alarm_info(alarm_id);
	if (alarm) {
		std::stringstream ss;
		ss << "update table_alarm set handle_id=" << handle->get_id() << " where id=" << alarm_id;
		impl_->db_->exec(ss.str());
		alarm->handle_id_ = handle->get_id();
		buffered_alarms_[alarm_id] = alarm;
	}

	return alarm;
}









}