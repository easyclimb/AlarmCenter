#include "stdafx.h"
#include "alarm_handle_mgr.h"
#include "../contrib/sqlitecpp/SQLiteCpp.h"
#include "AppResource.h"
#include "UserInfo.h"

using namespace SQLite;

namespace core {

std::wstring alarm_judgement_info::get_alarm_judgement_type_text(int judgement_type_id) {
	switch (judgement_type_id) {
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

std::wstring alarm_reason::get_reason_text(int reason)
{
	switch (reason) {
	case core::alarm_reason::real_alarm:
		return tr(IDS_STRING_ILLEGAL_ENTRANCE);
		break;
	case core::alarm_reason::device_false_positive:
		return tr(IDS_STRING_DEVICE_FALSE_POSITIVE);
		break;
	case core::alarm_reason::test_device:
		return tr(IDS_STRING_TEST_DEVICE);
		break;
	case core::alarm_reason::man_made_false_positive:
		return tr(IDS_STRING_MAN_MADE_FALSE_POSITIVE);
		break;
	case core::alarm_reason::other_reasons:
	default:
		return tr(IDS_STRING_OTHER_REASONS);
		break;
	}
}

alarm_reason::by alarm_reason::integer_to_by_what(int reason)
{
	switch (reason) {
	case core::alarm_reason::real_alarm:
		return core::alarm_reason::real_alarm;
		break;
	case core::alarm_reason::device_false_positive:
		return core::alarm_reason::device_false_positive;
		break;
	case core::alarm_reason::test_device:
		return core::alarm_reason::test_device;
		break;
	case core::alarm_reason::man_made_false_positive:
		return core::alarm_reason::man_made_false_positive;
		break;
	case core::alarm_reason::other_reasons:
	default:
		return core::alarm_reason::other_reasons;
		break;
	}
}


std::wstring alarm_info::get_text() const
{
	std::wstring all;
	for (auto txt : alarm_texts_) {
		all += txt->_txt + L"\r\n";
	}
	return all;
}

std::wstring alarm_info::get_alarm_status_text(int status)
{
	switch (status) {
	case core::alarm_status_not_handled:
		return tr(IDS_STRING_ALARM_STATUS_NOT_HANDLED);
		break;
	case core::alarm_status_not_cleared:
		return tr(IDS_STRING_ALARM_STATUS_PROCESSING);
		break;
	case core::alarm_status_cleared:
		return tr(IDS_STRING_ALARM_STATUS_CLEARED);
		break;
	case core::alarm_status_not_judged:
	default:
		return tr(IDS_STRING_ALARM_STATUS_NOT_JUDGED);
		break;
	}
	return std::wstring();
}

alarm_status alarm_info::integer_to_alarm_status(int status)
{
	switch (status) {
	case core::alarm_status_not_handled:
		return core::alarm_status_not_handled;
		break;
	case core::alarm_status_not_cleared:
		return core::alarm_status_not_cleared;
		break;
	case core::alarm_status_cleared:
		return core::alarm_status_cleared;
		break;
	case core::alarm_status_not_judged:
	default:
		return core::alarm_status_not_judged;
		break;
	}
}

class alarm_handle_mgr::alarm_handle_mgr_impl 
{
public:
	std::unique_ptr<SQLite::Database> db_;

	alarm_handle_mgr_impl(){}

	// return true for init ok, false for already exists and need not to init
	bool init_db() {
		using namespace SQLite;
		auto path = get_config_path() + "\\alarm_v1.2.db3";
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
						ss << "insert into table_judgement_types (id, desc) values (" << judge
							<< ", \"" << utf8::w2a(double_quotes(alarm_judgement_info::get_alarm_judgement_type_text(judge))) << "\")";
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
is_sub_machine integer, \
user_id integer, \
judgement_id integer, \
handle_id integer, \
reason_id integer, \
status integer)");

/*
gg integer, \
alarm_text text, \
alarm_date text, \

*/

					db_->exec("create table table_alarm_text \
(id integer primary key AUTOINCREMENT, \
alarm_id integer, \
zone integer, \
gg integer, \
ademco_event integer, \
alarm_text text, \
alarm_date text)");

					

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
			alarm_types[judge] = alarm_judgement_info::get_alarm_judgement_type_text(judge);
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

	void load_alarm_judgments(std::map<int, alarm_judgement_ptr>& judgments) {
		Statement query(*db_, "select * from table_judgement");
		while (query.executeStep()) {
			int index = 0;
			int id = query.getColumn(index++);
			int type = query.getColumn(index++);
			std::wstring note = utf8::a2w(query.getColumn(index++).getText());
			std::wstring note1 = utf8::a2w(query.getColumn(index++).getText());
			std::wstring note2 = utf8::a2w(query.getColumn(index++).getText());
			auto judgment = create_alarm_judgement_ptr(type, note, note1, note2);
			judgment->id_ = id;
			judgments[id] = judgment;
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

	void load_alarm_reasons(std::map<int, alarm_reason_ptr>& reasons) {
		Statement query(*db_, "select * from table_reason");
		while (query.executeStep()) {
			auto reason = create_alarm_reason();
			int id = 0;
			reason->id_ = query.getColumn(id++);
			reason->reason_ = alarm_reason::integer_to_by_what(query.getColumn(id++));
			reason->detail_ = utf8::a2w(query.getColumn(id++));
			reason->attach_ = utf8::a2w(query.getColumn(id++));

			reasons[reason->id_] = reason;
		}
	}

	int get_alarm_count() const {
		Statement query(*db_, "select count(id) from table_alarm");
		if (query.executeStep()) {
			return query.getColumn(0).getInt();
		}
		return 0;
	}

	void load_alarm_texts(const core::alarm_ptr& alarm) {
		std::stringstream ss;
		ss << "select * from table_alarm_text where alarm_id=" << alarm->get_id();
		Statement query(*db_, ss.str());
		while (query.executeStep()) {
			int ndx = 0;
			int id = query.getColumn(ndx++).getInt();
			int alarm_id = query.getColumn(ndx++).getInt();
			int zone = query.getColumn(ndx++);
			int gg = query.getColumn(ndx++);
			int adm_evnt = query.getColumn(ndx++);
			std::string txt = query.getColumn(ndx++).getText();
			std::string tim = query.getColumn(ndx++).getText();

			auto at = std::make_shared<core::alarm_text>();
			at->_zone = zone;
			at->_subzone = gg;
			at->_event = adm_evnt;
			at->_txt = utf8::a2w(txt).c_str();
			at->_time = std::chrono::system_clock::to_time_t(string_to_time_point(tim));
			alarm->alarm_texts_.push_back(at);
		}
	}
};

alarm_handle_mgr::alarm_handle_mgr()
{
	impl_ = std::make_unique<alarm_handle_mgr_impl>();
	impl_->init_db();

	impl_->load_alarm_judgement_types(buffered_alarm_judgement_types_);
	impl_->load_alarm_judgments(buffered_alarm_judgements_);
	impl_->load_security_guards(buffered_security_guards_);
	impl_->load_alarm_handles(buffered_alarm_handles_);
	impl_->load_alarm_reasons(buffered_alarm_reasons_);

	alarm_count_ = impl_->get_alarm_count();
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

alarm_handle_ptr alarm_handle_mgr::get_alarm_handle(int id)
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

bool alarm_handle_mgr::get_alarms_by_sql(const std::string & sql, const observer_ptr & ptr, bool asc)
{
	std::shared_ptr<observer_type> obs(ptr.lock());
	if (!obs) return false;

	Statement query(*impl_->db_, sql);

	std::list<alarm_ptr> tmp_list;

	while (query.executeStep()) {
		int index = 0;
		auto record = create_alarm_info();
		record->id_ = query.getColumn(index++).getInt();
		record->aid_ = query.getColumn(index++);
		record->zone_ = query.getColumn(index++);
		record->is_sub_machine_ = query.getColumn(index++).getInt() != 0;
		//record->gg_ = query.getColumn(index++);
		//record->text_ = utf8::a2w(query.getColumn(index++).getText());
		//record->date_ = utf8::a2w(query.getColumn(index++).getText());
		impl_->load_alarm_texts(record);
		record->user_id_ = query.getColumn(index++);
		record->judgement_id_ = query.getColumn(index++);
		record->handle_id_ = query.getColumn(index++);
		record->reason_id_ = query.getColumn(index++);
		record->status_ = alarm_info::integer_to_alarm_status(query.getColumn(index++));

		buffered_alarms_[record->id_] = record;
		tmp_list.push_back(record);

	}

	if (!asc) {
		tmp_list.reverse();
	}

	for (auto hr : tmp_list) {
		obs->on_update(hr);
	}
	return false;
}

alarm_reason_ptr alarm_handle_mgr::get_alarm_reason(int id)
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

alarm_ptr alarm_handle_mgr::get_alarm_info(int id)
{
	auto iter = buffered_alarms_.find(id);
	if (iter != buffered_alarms_.end()) {
		return iter->second;
	} else {
		std::stringstream ss;
		ss << "select * from table_alarm where id=" << id;
		auto sql = ss.str();
		Statement query(*impl_->db_, sql);
		if (query.executeStep()) {
			auto alarm = create_alarm_info();
			alarm->id_ = id;
			id = 1;
			alarm->aid_ = query.getColumn(id++);
			alarm->zone_ = query.getColumn(id++);
			alarm->is_sub_machine_ = query.getColumn(id++).getInt() != 0;
			impl_->load_alarm_texts(alarm);
			alarm->judgement_id_ = query.getColumn(id++);
			alarm->handle_id_ = query.getColumn(id++);
			alarm->reason_id_ = query.getColumn(id++);
			alarm->status_ = alarm_info::integer_to_alarm_status(query.getColumn(id++));

			buffered_alarms_[alarm->id_] = alarm;
			return alarm;
		}
	}
	return alarm_ptr();
}

alarm_ptr alarm_handle_mgr::execute_add_alarm(int ademco_id, int zone, /*int gg, 
											  const std::wstring & alarm_text, 
											  const std::chrono::system_clock::time_point & alarm_time, */
											  bool is_sub_machine,
											  int judgement_id, int handle_id, int reason_id)
{
	std::stringstream ss; // gg,alarm_text,alarm_date,
	ss << "insert into table_alarm "
		<< "(aid,zone,user_id,is_sub_machine,judgement_id,handle_id,reason_id,status) "
		<< "values (" << ademco_id << "," << zone << "," 

		/*<< gg << ",\"" << utf8::w2a(double_quotes(alarm_text))
		<< "\",\"" << time_point_to_string(alarm_time) << "\","*/ 

		<< user_manager::get_instance()->get_cur_user_id() << "," << is_sub_machine << "," << judgement_id << "," << handle_id
		<< "," << reason_id << "," << alarm_status::alarm_status_not_judged << ")";

	auto sql = ss.str();
	impl_->db_->exec(sql);
	int id = impl_->db_->getLastInsertRowid() & 0xFFFFFFFF;

	auto alarm = create_alarm_info();
	alarm->id_ = id;
	alarm->aid_ = ademco_id;
	alarm->zone_ = zone;
	alarm->is_sub_machine_ = is_sub_machine;
	//alarm->gg_ = gg;
	//alarm->text_ = alarm_text;
	//alarm->date_ = time_point_to_wstring(alarm_time);
	alarm->judgement_id_ = judgement_id;
	alarm->handle_id_ = handle_id;
	alarm->reason_id_ = reason_id;
	alarm->status_ = alarm_status::alarm_status_not_judged;

	buffered_alarms_[id] = alarm;

	return alarm;
}

alarm_ptr alarm_handle_mgr::execute_add_alarm_text(int alarm_id, const alarm_text_ptr & txt)
{
	auto alarm = get_alarm_info(alarm_id);
	if (alarm) {
		std::stringstream ss;
		ss << "insert into table_alarm_text "
			<< "(alarm_id, zone, gg, ademco_event, alarm_text, alarm_date) "
			<< "value(" << alarm_id
			<< ", " << txt->_zone << ", " << txt->_subzone << ", " << txt->_event
			<< ", \"" << utf8::w2a(double_quotes(txt->_txt))
			<< "\", \"" << time_point_to_string(std::chrono::system_clock::from_time_t(txt->_time)) << "\")";

		auto sql = ss.str();
		impl_->db_->exec(sql);
		//int id = impl_->db_->getLastInsertRowid() & 0xFFFFFFFF;
		alarm->alarm_texts_.push_back(txt);
	}
	return alarm;
}

alarm_ptr alarm_handle_mgr::execute_add_alarm_texts(int alarm_id, std::list<alarm_text_ptr> txts)
{
	auto alarm = get_alarm_info(alarm_id);
	if (alarm) {
		std::stringstream ss;
		for (auto txt : txts) {
			ss << "insert into table_alarm_text "
				<< "(alarm_id, zone, gg, ademco_event, alarm_text, alarm_date) "
				<< "values(" << alarm_id
				<< ", " << txt->_zone << ", " << txt->_subzone << ", " << txt->_event
				<< ", \"" << utf8::w2a(double_quotes(txt->_txt))
				<< "\", \"" << time_point_to_string(std::chrono::system_clock::from_time_t(txt->_time)) << "\")";

			auto sql = ss.str();
			try {
				impl_->db_->exec(sql);
			} catch (SQLite::Exception e) {
				JLOGA(e.what());
			}
			//int id = impl_->db_->getLastInsertRowid() & 0xFFFFFFFF;
			alarm->alarm_texts_.push_back(txt);
			ss.str(""); ss.clear();
		}
	}
	return alarm;
}

alarm_ptr alarm_handle_mgr::execute_update_alarm_judgment(int alarm_id, alarm_judgement_ptr & judgment)
{
	auto alarm = get_alarm_info(alarm_id);
	if (alarm) {
		std::stringstream ss;
		if (alarm->judgement_id_ == 0) { 
			judgment = execute_add_judgment(judgment->get_judgement_type_id(), judgment->get_note(), judgment->get_note1(), judgment->get_note2());
			ss << "update table_alarm set judgement_id=" << judgment->get_id() << " where id=" << alarm_id;
			impl_->db_->exec(ss.str());
			alarm->judgement_id_ = judgment->get_id();
		} else {
			ss << "update table_judgement set judgement_type_id=" << judgment->get_judgement_type_id()
				<< ", note=\"" << utf8::w2a(double_quotes(judgment->get_note()))
				<< "\", note1=\"" << utf8::w2a(double_quotes(judgment->get_note1()))
				<< "\", note2=\"" << utf8::w2a(double_quotes(judgment->get_note2()))
				<< "\" where id=" << alarm->get_judgement_id();
			
			impl_->db_->exec(ss.str());
			judgment->id_ = alarm->get_judgement_id();
		}

		buffered_alarm_judgements_[alarm->get_judgement_id()] = judgment;
		buffered_alarms_[alarm_id] = alarm;
	}

	return alarm;
}

alarm_ptr alarm_handle_mgr::execute_update_alarm_reason(int alarm_id, alarm_reason_ptr & reason)
{
	auto alarm = get_alarm_info(alarm_id);
	if (alarm) {
		std::stringstream ss;
		if (reason) {
			if (alarm->reason_id_ == 0 || reason->get_id() == 0) {
				reason = execute_add_alarm_reason(reason->get_reason(), reason->get_detail(), reason->get_attach());
				ss << "update table_alarm set reason_id=" << reason->get_id() << " where id=" << alarm_id;
				impl_->db_->exec(ss.str());
				alarm->reason_id_ = reason->get_id();
			} else {
				ss << "update table_reason set reason=" << reason->get_reason()
					<< ", detail=\"" << utf8::w2a(double_quotes(reason->get_detail()))
					<< "\", attach=\"" << utf8::w2a(double_quotes(reason->get_attach()))
					<< "\" where id=" << alarm->reason_id_;
				impl_->db_->exec(ss.str());
				reason->id_ = alarm->reason_id_;
				buffered_alarm_reasons_[reason->id_] = reason;
			}			
		} else {
			if (alarm->reason_id_ != 0) {
				ss << "delete from table_reason where id=" << alarm->reason_id_;
				impl_->db_->exec(ss.str());
				buffered_alarm_reasons_.erase(alarm->reason_id_);
			}

			ss.str(""); ss.clear();
			ss << "update table_alarm set reason_id=0 where id=" << alarm_id;
			impl_->db_->exec(ss.str());
			alarm->reason_id_ = 0;
		}
		
		buffered_alarms_[alarm_id] = alarm;
	}

	return alarm;
}

alarm_ptr alarm_handle_mgr::execute_update_alarm_handle(int alarm_id, alarm_handle_ptr & handle)
{
	auto alarm = get_alarm_info(alarm_id);
	if (alarm) {
		std::stringstream ss;
		if (handle) {
			if (alarm->handle_id_ == 0 || handle->get_id() == 0) {
				handle = execute_add_alarm_handle(handle->get_guard_id(), std::chrono::minutes(handle->get_predict_minutes_to_handle()), handle->get_note());
				ss << "update table_alarm set handle_id=" << handle->get_id() << " where id=" << alarm_id;
				impl_->db_->exec(ss.str());
				alarm->handle_id_ = handle->get_id();
			} else {
				ss << "update table_handle set guard_id=" << handle->get_guard_id()
					<< ",time_assigned=\"" << utf8::w2a(double_quotes(time_point_to_wstring(handle->get_assigned_time_point())))
					<< "\",time_handled=\"" << utf8::w2a(double_quotes(time_point_to_wstring(handle->get_handled_time_point())))
					<< "\",predict_minutes=\"" << handle->get_predict_minutes_to_handle()
					<< "\",note=\"" << utf8::w2a(double_quotes(handle->get_note()))
					<< "\" where id=" << alarm->handle_id_;
				impl_->db_->exec(ss.str());
				handle->id_ = alarm->handle_id_;
				buffered_alarm_handles_[handle->id_] = handle;
			}			
		} else {
			if (alarm->handle_id_ != 0) {
				ss << "delete from table_handle where id=" << alarm->handle_id_;
				impl_->db_->exec(ss.str());
				buffered_alarm_handles_.erase(alarm->handle_id_);
			}

			ss.str(""); ss.clear();
			ss << "update table_alarm set handle_id=0 where id=" << alarm_id;
			impl_->db_->exec(ss.str());
			alarm->handle_id_ = 0;
		}
		
		buffered_alarms_[alarm_id] = alarm;
	}

	return alarm;
}

alarm_ptr alarm_handle_mgr::execute_update_alarm_status(int alarm_id, alarm_status status)
{
	auto alarm = get_alarm_info(alarm_id);
	if (alarm) {
		std::stringstream ss;
		ss << "update table_alarm set status=" << status << " where id=" << alarm_id;
		impl_->db_->exec(ss.str());
		alarm->status_ = status;
		buffered_alarms_[alarm_id] = alarm;
	}

	return alarm;
}

alarm_ptr alarm_handle_mgr::execute_update_alarm_user(int alarm_id, int user_id)
{
	auto alarm = get_alarm_info(alarm_id);
	if (alarm) {
		std::stringstream ss;
		ss << "update table_alarm set user_id=" << user_id << " where id=" << alarm_id;
		impl_->db_->exec(ss.str());
		alarm->user_id_ = user_id;
		buffered_alarms_[alarm_id] = alarm;
	}

	return alarm;
}

int alarm_handle_mgr::get_min_alarm_id() const
{
	Statement query(*impl_->db_, "select min(id) from table_alarm");
	if (query.executeStep()) {
		return query.getColumn(0).getInt();
	}
	return 0;
}

bool alarm_handle_mgr::get_top_num_records_based_on_id(const int baseID, const int nums, const observer_ptr& ptr)
{
	AUTO_LOG_FUNCTION;
	CString query = _T("");
	query.Format(_T("select * from table_alarm where id >= %d order by id limit %d"), baseID, nums);
	return get_alarms_by_sql(utf8::w2a((LPCTSTR)query), ptr, false);
}










}