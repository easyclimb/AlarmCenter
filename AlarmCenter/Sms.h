#pragma once
namespace ado { class CDbOper; };

#include "core.h"

namespace core {
class sms_manager
{
private:
	std::shared_ptr<ado::CDbOper> m_db;
public:
	bool add_sms_config(bool is_submachine, int ademco_id, int zone_value, sms_config& cfg);
	bool del_sms_config(int id);
	bool get_sms_config(bool is_submachine, int ademco_id, int zone_value, sms_config& cfg);
	bool set_sms_config(const core::sms_config& cfg);
	//sms_manager();
	~sms_manager();
	DECLARE_UNCOPYABLE(sms_manager)
	DECLARE_SINGLETON(sms_manager)

};
NAMESPACE_END
