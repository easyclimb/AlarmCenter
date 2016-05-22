#pragma once
namespace SQLite { class Database; };

#include "core.h"

namespace core {

class sms_manager : private boost::noncopyable
{
private:
	std::shared_ptr<SQLite::Database> db_;
public:
	bool add_sms_config(bool is_submachine, int ademco_id, int zone_value, sms_config& cfg);
	bool del_sms_config(int id);
	bool get_sms_config(bool is_submachine, int ademco_id, int zone_value, sms_config& cfg);
	bool set_sms_config(const core::sms_config& cfg);

	~sms_manager();
	DECLARE_SINGLETON(sms_manager)

};

};

