#pragma once

#include "core.h"
#include "C:/dev/Global/dp.h"

namespace core {

class alarm_handle_mgr;

// 判断依据
class alarm_judgement_info
{
	friend class alarm_handle_mgr;

private:
	int id_ = 0;
	int judgement_type_id_ = 0;
	std::wstring note_ = {};
	std::wstring note1_ = {};
	std::wstring note2_ = {};

public:

	alarm_judgement_info(int judgement = alarm_judgement_min,
						 const std::wstring& note = {},
						 const std::wstring& note1 = {},
						 const std::wstring& note2 = {})
		: judgement_type_id_(judgement)
		, note_(note)
		, note1_(note1)
		, note2_(note2)
	{}

	auto get_id() const { return id_; }
	auto get_judgement_type_id() const { return judgement_type_id_; }
	auto get_note() const { return note_; }
	auto get_note1() const { return note1_; }
	auto get_note2() const { return note2_; }

	static std::wstring get_alarm_judgement_type_text(int judgement_type_id);
};


// 安保人员
class security_guard
{
	friend class alarm_handle_mgr;

public:
	enum status {
		offline,		// 离线
		standing_by,	// 待命
		on_handle,		// 出警
	};

	static auto integer_to_status(int s) {
		switch (s) {
		case core::security_guard::standing_by:
			return core::security_guard::standing_by;
			break;
		case core::security_guard::on_handle:
			return core::security_guard::on_handle;
			break;
		case core::security_guard::offline:
		default:
			return core::security_guard::offline;
			break;
		}
	}

private:
	int id_ = 0;
	std::wstring name_ = {};
	std::wstring phone_ = {};
	status status_ = offline;

public:
	auto get_id() const { return id_; }
	auto get_name() const { return name_; }
	auto get_phone() const { return phone_; }
	auto get_status() const { return status_; }
	auto get_formatted_name() const {
		std::wstringstream ss;
		ss << id_ << L" " << name_ << L" " << phone_;
		return ss.str();
	}
	std::wstring get_status_text() const;
};


// 警情处理
class alarm_handle
{
	friend class alarm_handle_mgr;

private:
	int id_ = 0;
	int guard_id_ = 0;
	std::chrono::system_clock::time_point time_point_assigned_ = {};
	std::chrono::system_clock::time_point time_point_handled_ = {};
	std::chrono::minutes predict_minutes_to_handle_ = std::chrono::minutes(handle_time_default);
	std::wstring note_ = {};

public:

	alarm_handle(int guard_id, 
				 const std::chrono::system_clock::time_point& assigned_time, 
				 const std::chrono::minutes& predict_minutes,
				 const std::wstring& note)
		: guard_id_(guard_id), time_point_assigned_(assigned_time), predict_minutes_to_handle_(predict_minutes), note_(note)
	{}

	enum {
		handle_time_min = 1,
		handle_time_default = 20,
		handle_time_max = 9999,
	};

	auto get_id() const { return id_; }
	auto get_guard_id() const { return guard_id_; }
	auto get_assigned_time_point() const { return time_point_assigned_; }
	auto get_handled_time_point() const { return time_point_handled_; }
	auto get_predict_minutes_to_handle() const { return predict_minutes_to_handle_.count(); }
	auto get_actually_minutes_to_handle() const { return std::chrono::duration_cast<std::chrono::minutes>(time_point_handled_ - time_point_assigned_).count(); }
	auto get_note() const { return note_; }

}; 


// 警情原因
class alarm_reason
{
	friend class alarm_handle_mgr;

public:
	enum by {
		real_alarm				= alarm_type::alarm_type_true,
		device_false_positive	= alarm_type::alarm_type_device_false_positive,
		test_device				= alarm_type::alarm_type_test_device,
		man_made_false_positive = alarm_type::alarm_type_man_made_false_positive,
		other_reasons,
	};

private:
	int id_ = 0;
	alarm_reason::by reason_ = alarm_reason::by::other_reasons;
	std::wstring detail_ = {};
	std::wstring attach_ = {};

public:

	alarm_reason(alarm_reason::by reason, const std::wstring& detail, const std::wstring& attach)
		: reason_(reason), detail_(detail), attach_(attach)
	{}

	auto get_id() const { return id_; }
	auto get_reason() const{ return reason_; }
	auto get_detail() const { return detail_; }
	auto get_attach() const { return attach_; }

	static std::wstring get_reason_text(int reason);
	static by integer_to_by_what(int reason);
}; 


// 警情
class alarm_info
{
	friend class alarm_handle_mgr;

private:

	int id_ = 0;
	int aid_ = 0;
	int zone_ = 0;
	int gg_ = 0;
	std::wstring text_ = {};
	std::wstring date_ = {};
	int user_id_ = 0;
	int judgement_id_ = 0;
	int handle_id_ = 0;
	int reason_id_ = 0;
	alarm_status status_ = alarm_status_not_judged;

public:
	auto get_id() const { return id_; }
	auto get_aid() const { return aid_; }
	auto get_zone() const { return zone_; }
	auto get_gg() const { return gg_; }
	auto get_text() const { return text_; }
	auto get_date() const { return date_; }
	auto get_user_id() const { return user_id_; }
	auto get_judgement_id() const { return judgement_id_; }
	auto get_handle_id() const { return handle_id_; }
	auto get_reason_id() const { return reason_id_; }
	auto get_status() const { return status_; }

	static std::wstring get_alarm_status_text(int status);
	static alarm_status integer_to_alarm_status(int status);
}; 

// 判断依据
typedef std::pair<int, std::wstring> alarm_judgement_type_info;
inline auto create_alarm_judgement_type_info() { return alarm_judgement_type_info(alarm_judgement_min, L"alarm_judgement_min"); }

// 判断依据详情
typedef std::shared_ptr<alarm_judgement_info> alarm_judgement_ptr;
inline auto create_alarm_judgement_ptr(int judgement_type_id = alarm_judgement_min,
									   const std::wstring& note = {},
									   const std::wstring& note1 = {},
									   const std::wstring& note2 = {}) {
	return std::make_shared<alarm_judgement_info>(judgement_type_id, note, note1, note2);
}

// 安保人员
typedef std::shared_ptr<security_guard> security_guard_ptr;
inline auto create_security_guard() { return std::make_shared<security_guard>(); }

// 警情处理
typedef std::shared_ptr<alarm_handle> alarm_handle_ptr;
inline auto create_alarm_handle(int guard_id = 0,
								const std::chrono::system_clock::time_point& assigned_time = std::chrono::system_clock::now(),
								const std::chrono::minutes& predict_minutes = std::chrono::minutes(alarm_handle::handle_time_default),
								const std::wstring& note = L"") { 
	return std::make_shared<alarm_handle>(guard_id, assigned_time, predict_minutes, note);
}

// 警情原因
typedef std::shared_ptr<alarm_reason> alarm_reason_ptr;
inline auto create_alarm_reason(alarm_reason::by reason = alarm_reason::by::real_alarm, const std::wstring& detail = {}, const std::wstring& attach = {}) {
	return std::make_shared<alarm_reason>(reason, detail, attach); 
}

// 警情
typedef std::shared_ptr<alarm_info> alarm_ptr;
inline auto create_alarm_info() { return std::make_shared<alarm_info>(); }

typedef std::vector<int> valid_data_ids;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class alarm_handle_mgr : public dp::observable<alarm_ptr>, public dp::singleton<alarm_handle_mgr>
{
protected:
	alarm_handle_mgr();

	class alarm_handle_mgr_impl;
	std::unique_ptr<alarm_handle_mgr_impl> impl_ = {};

	std::map<int, std::wstring> buffered_alarm_judgement_types_ = {};
	std::map<int, alarm_judgement_ptr> buffered_alarm_judgements_ = {};
	std::map<int, security_guard_ptr> buffered_security_guards_ = {};
	std::map<int, alarm_handle_ptr> buffered_alarm_handles_ = {};
	std::map<int, alarm_reason_ptr> buffered_alarm_reasons_ = {};
	std::map<int, alarm_ptr> buffered_alarms_ = {};

	int alarm_count_ = 0;


protected:
	alarm_handle_ptr execute_add_alarm_handle(int guard_id, const std::chrono::minutes& predict_minutes, const std::wstring& note);
	bool get_alarms_by_sql(const std::string& sql, const observer_ptr& ptr, bool asc = true);

public:

	virtual ~alarm_handle_mgr();

	std::vector<alarm_judgement_type_info> get_all_user_defined_judgements() const;
	alarm_judgement_type_info get_alarm_judgement_type_info(int id);
	alarm_judgement_type_info execute_add_judgement_type(const std::wstring& txt);

	alarm_judgement_ptr get_alarm_judgement(int id);
	alarm_judgement_ptr execute_add_judgment(int judgement_type_id,
											 const std::wstring& note = {},
											 const std::wstring& note1 = {},
											 const std::wstring& note2 = {});

	security_guard_ptr get_security_guard(int id);
	valid_data_ids get_security_guard_ids() const;
	security_guard_ptr execute_add_security_guard(const std::wstring& name, const std::wstring& phone);
	bool execute_rm_security_guard(int id);
	bool execute_update_security_guard_info(int id, const std::wstring& name, const std::wstring& phone);

	int allocate_alarm_handle_id() const;
	alarm_handle_ptr get_alarm_handle(int id);
	

	alarm_reason_ptr get_alarm_reason(int id);
	alarm_reason_ptr execute_add_alarm_reason(int reason, const std::wstring& detail, const std::wstring& attachment);

	int get_alarm_count() const { return alarm_count_; }
	alarm_ptr get_alarm_info(int id);
	alarm_ptr execute_add_alarm(int ademco_id, int zone, int gg,
								const std::wstring& alarm_text,
								const std::chrono::system_clock::time_point& alarm_time,
								int judgement_id, int handle_id, int reason_id);
	alarm_ptr execute_update_alarm_judgment(int alarm_id, alarm_judgement_ptr& judgment);
	alarm_ptr execute_update_alarm_reason(int alarm_id, alarm_reason_ptr& reason);
	alarm_ptr execute_update_alarm_handle(int alarm_id, alarm_handle_ptr& handle);
	alarm_ptr execute_update_alarm_status(int alarm_id, alarm_status status);
	alarm_ptr execute_update_alarm_user(int alarm_id, int user_id);

	int get_min_alarm_id() const;
	bool get_top_num_records_based_on_id(const int baseID, const int nums, const observer_ptr& ptr);
};


}
