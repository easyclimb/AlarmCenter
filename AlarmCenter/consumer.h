#pragma once

#include "core.h"

namespace SQLite { class Database; }

namespace core {

struct consumer {
	int id, ademco_id, zone_value;
	consumer_type_ptr type;
	int receivable_amount;
	int paid_amount;
	std::chrono::system_clock::time_point remind_time;

	consumer(int id, int ademco_id, int zone_value,
			 const consumer_type_ptr& type,
			 int receivable_amount, int paid_amount,
			 const std::chrono::system_clock::time_point& remind_time)
		: id(id), ademco_id(ademco_id), zone_value(zone_value),
		type(type), receivable_amount(receivable_amount),
		paid_amount(paid_amount), remind_time(remind_time) {}

	int get_owed_amount() const { return receivable_amount - paid_amount; }
};



class consumer_manager : public dp::singleton<consumer_manager>
{
	//friend class alarm_machine_manager;
public:

	consumer_ptr execute_add_consumer(int ademco_id, int zone_value, const consumer_type_ptr& type,
									  int receivalble_amount, int paid_amount,
									  const std::chrono::system_clock::time_point& remind_time);

	bool execute_delete_consumer(const consumer_ptr& consumer);
	bool execute_update_consumer(const consumer_ptr& consumer);

	bool execute_add_type(int& id, const CString& type_name);
	bool execute_rename(int id, const CString& new_name);

	consumer_type_ptr get_consumer_type_by_id(int id) const {
		auto iter = consumer_type_map_.find(id);
		return ((iter == consumer_type_map_.end()) || (iter->first != id)) ? nullptr : iter->second;
	}

	consumer_list load_consumers() const;

	consumer_type_map get_all_types() const { return consumer_type_map_; }

	//DECLARE_SINGLETON(consumer_manager);
	~consumer_manager();

protected:



	void add_type(int id, const CString& type_name) {
		consumer_type_map_[id] = std::make_shared<consumer_type>(id, type_name);
	}


protected:

	consumer_manager();
private:
	std::shared_ptr<SQLite::Database> db_;
	consumer_type_map consumer_type_map_;
};





}
