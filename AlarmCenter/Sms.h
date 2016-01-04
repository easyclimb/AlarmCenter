#pragma once
namespace ado { class CDbOper; };

#include "core.h"

namespace core {
class CSms
{
private:
	std::shared_ptr<ado::CDbOper> m_db;
public:
	bool add_sms_config(bool is_submachine, int ademco_id, int zone_value, SmsConfigure& cfg);
	bool del_sms_config(int id);
	bool get_sms_config(bool is_submachine, int ademco_id, int zone_value, SmsConfigure& cfg);
	bool set_sms_config(const core::SmsConfigure& cfg);
	//CSms();
	~CSms();
	DECLARE_UNCOPYABLE(CSms)
	DECLARE_SINGLETON(CSms)

};
NAMESPACE_END
