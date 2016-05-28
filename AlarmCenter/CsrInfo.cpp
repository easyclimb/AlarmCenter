#include "stdafx.h"
#include "CsrInfo.h"
#include "AlarmMachineManager.h"

namespace core {

IMPLEMENT_SINGLETON(csr_manager)

csr_manager::csr_manager()
	:_level(14)
	, _coor()
{
}


csr_manager::~csr_manager()
{
}


bool csr_manager::execute_set_zoom_level(int level)
{
	CString sql;
	sql.Format(L"update table_center set zoom_level=%d where ID=1", level);
	alarm_machine_manager* mgr = alarm_machine_manager::GetInstance();
	if (mgr->ExecuteSql(sql)) {
		set_level(level);
		return true;
	}
	return false;
}


bool csr_manager::execute_set_coor(const web::BaiduCoordinate& coor)
{
	CString sql;
	sql.Format(L"update table_center set map_coor_x=%f, map_coor_y=%f where ID=1", coor.x, coor.y);
	alarm_machine_manager* mgr = alarm_machine_manager::GetInstance();
	if (mgr->ExecuteSql(sql)) {
		_coor = coor;
		return true;
	}
	return false;
}



};
