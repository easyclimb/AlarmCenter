#include "stdafx.h"
#include "CsrInfo.h"
#include "AlarmMachineManager.h"

namespace core {

IMPLEMENT_SINGLETON(CCsrInfo)

CCsrInfo::CCsrInfo()
	: _acct(NULL)
	, _addr(NULL)
	, _city_code(0)
	, _x(.0)
	, _y(.0)
{
	_acct = new wchar_t[1]; _acct[0] = 0;
	_addr = new wchar_t[1]; _addr[0] = 0;
}


CCsrInfo::~CCsrInfo()
{
	SAFEDELETEARR(_acct);
	SAFEDELETEARR(_addr);
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
	CString sql;
	sql.Format(L"update CsrInfo set CsrAcct='%s'", acct);
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
	sql.Format(L"update CsrInfo set CsrAddress='%s'", addr);
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
	sql.Format(L"update CsrInfo set CsrCityCode=%d", city_code);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (mgr->ExecuteSql(sql)) {
		set_city_code(city_code);
		return true;
	}
	return false;
}


bool CCsrInfo::execute_set_x(double x)
{
	CString sql;
	sql.Format(L"update CsrInfo set CsrBaiduMapX=%f", x);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (mgr->ExecuteSql(sql)) {
		set_x(x);
		return true;
	}
	return false;
}


bool CCsrInfo::execute_set_y(double y)
{
	CString sql;
	sql.Format(L"update CsrInfo set CsrBaiduMapY=%f", y);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (mgr->ExecuteSql(sql)) {
		set_y(y);
		return true;
	}
	return false;
}



NAMESPACE_END
