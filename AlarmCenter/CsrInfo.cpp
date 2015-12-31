#include "stdafx.h"
#include "CsrInfo.h"
#include "AlarmMachineManager.h"

namespace core {

IMPLEMENT_SINGLETON(CCsrInfo)

CCsrInfo::CCsrInfo()
	: _acct()
	, _addr()
	, _city_code(0)
	, _level(14)
	, _coor()
	//, _x(.0)
	//, _y(.0)
{
}


CCsrInfo::~CCsrInfo()
{
}


const char* CCsrInfo::get_acctA() const
{
	static char __acct[64] = { 0 };
	USES_CONVERSION;
	strcpy_s(__acct, W2A(_acct));
	return __acct;
}


bool CCsrInfo::execute_set_acct(const wchar_t* acct)
{
	AUTO_LOG_FUNCTION;
	CString sql;
	sql.Format(L"update CsrInfo set CsrAcct='%s' where ID=1", acct);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (mgr->ExecuteSql(sql)) {
		set_acct(acct);
		return true;
	}
	return false;
}


bool CCsrInfo::execute_set_addr(const wchar_t* addr)
{
	CString sql;
	sql.Format(L"update CsrInfo set CsrAddress='%s' where ID=1", addr);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (mgr->ExecuteSql(sql)) {
		set_addr(addr);
		return true;
	}
	return false;
}


bool CCsrInfo::execute_set_city_code(int city_code)
{
	CString sql;
	sql.Format(L"update CsrInfo set CsrCityCode=%d where ID=1", city_code);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (mgr->ExecuteSql(sql)) {
		set_city_code(city_code);
		return true;
	}
	return false;
}


bool CCsrInfo::execute_set_zoom_level(int level)
{
	CString sql;
	sql.Format(L"update CsrInfo set ZoomLevel=%d where ID=1", level);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (mgr->ExecuteSql(sql)) {
		set_level(level);
		return true;
	}
	return false;
}

//bool CCsrInfo::execute_set_x(double x)
//{
//	CString sql;
//	sql.Format(L"update CsrInfo set CsrBaiduMapX=%f", x);
//	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
//	if (mgr->ExecuteSql(sql)) {
//		set_x(x);
//		return true;
//	}
//	return false;
//}


bool CCsrInfo::execute_set_coor(const web::BaiduCoordinate& coor)
{
	CString sql;
	sql.Format(L"update CsrInfo set CsrBaiduMapX=%f, CsrBaiduMapY=%f where ID=1", coor.x, coor.y);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (mgr->ExecuteSql(sql)) {
		_coor = coor;
		return true;
	}
	return false;
}



NAMESPACE_END
