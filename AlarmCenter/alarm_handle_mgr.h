#pragma once

#include "core.h"

namespace core {

class alarm_handle_mgr;

// 判断依据
class alarm_judgement_info
{
	friend class alarm_handle_mgr;

private:
	int id_ = 0;
	int judgement_id_ = 0;
	std::wstring desc_ = {};
	std::wstring note_ = {};
	std::wstring note1_ = {};
	std::wstring note2_ = {};

public:

	alarm_judgement_info(int judgement = alarm_judgement_min,
						 const std::wstring& desc = L"",
						 const std::wstring& note = L"",
						 const std::wstring& note1 = L"",
						 const std::wstring& note2 = L"")
		: judgement_id_(judgement)
		, desc_(desc)
		, note_(note)
		, note1_(note1)
		, note2_(note2)
	{}


	auto get_id() const { return id_; }
	auto get_judgement_id() const { return judgement_id_; }
	auto get_desc() const { return desc_; }
	auto get_note() const { return note_; }
	auto get_note1() const { return note1_; }
	auto get_note2() const { return note2_; }

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
	std::chrono::minutes predict_minutes_to_handle_ = std::chrono::minutes(default_handle_time);
	std::wstring note_ = {};

public:

	enum {
		default_handle_time = 20,
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

private:
	int id_ = 0;
	int reason_ = 0;
	std::wstring detail_ = {};
	std::wstring attach_ = {};

public:
	auto get_id() const { return id_; }
	auto get_detail() const { return detail_; }
	auto get_attach() const { return attach_; }

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

	auto get_judgement_id() const { return judgement_id_; }
	auto get_handle_id() const { return handle_id_; }
	auto get_reason_id() const { return reason_id_; }
	auto get_status() const { return status_; }

}; 

// 判断依据
typedef std::pair<int, std::wstring> alarm_judgement_type_info;
inline auto create_alarm_judgement_type_info() { return alarm_judgement_type_info(alarm_judgement_min, L"alarm_judgement_min"); }

// 判断依据详情
typedef std::shared_ptr<alarm_judgement_info> alarm_judgement_ptr;

inline auto create_alarm_judgement_ptr(int judgement = alarm_judgement_min, 
									   const std::wstring& desc = L"", 
									   const std::wstring& note = L"",
									   const std::wstring& note1 = L"", 
									   const std::wstring& note2 = L"") {
	return std::make_shared<alarm_judgement_info>(judgement, desc, note, note1, note2);
}

// 安保人员
typedef std::shared_ptr<security_guard> security_guard_ptr;
inline auto create_security_guard() { return std::make_shared<security_guard>(); }

// 警情处理
typedef std::shared_ptr<alarm_handle> alarm_handle_ptr;
inline auto create_alarm_handle() { return std::make_shared<alarm_handle>(); }

// 警情原因
typedef std::shared_ptr<alarm_reason> alarm_reason_ptr;
inline auto create_alarm_reason() { return std::make_shared<alarm_reason>(); }

// 警情
typedef std::shared_ptr<alarm_info> alarm_ptr;
inline auto create_alarm_info() { return std::make_shared<alarm_info>(); }

typedef std::vector<int> valid_data_ids;

class alarm_handle_mgr : public dp::singleton<alarm_handle_mgr>
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



public:

	virtual ~alarm_handle_mgr();

	std::vector<alarm_judgement_type_info> get_all_user_defined_judgements() const;
	alarm_judgement_type_info get_alarm_judgement_type_info(int id);
	alarm_judgement_type_info execute_add_judgement_type(const std::wstring& txt);

	auto get_alarm_judgement(int id);
	security_guard_ptr get_security_guard(int id);
	valid_data_ids get_security_guard_ids() const;
	security_guard_ptr execute_add_security_guard(const std::wstring& name, const std::wstring& phone);
	bool execute_rm_security_guard(int id);
	bool execute_update_security_guard_info(int id, const std::wstring& name, const std::wstring& phone);

	int allocate_alarm_handle_id() const;
	auto get_alarm_handle(int id);


	auto get_alarm_reason(int id);
	auto get_alarm_info(int id);




};


}
