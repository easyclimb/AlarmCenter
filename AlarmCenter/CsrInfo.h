#pragma once

namespace core {

class CCsrInfo
{
private:
	wchar_t* _acct;
	wchar_t* _addr;
	int _city_code;
	double _x;
	double _y;
public:
	~CCsrInfo();

	DECLARE_GETTER_SETTER_STRING(_acct);
	DECLARE_GETTER_SETTER_STRING(_addr);
	DECLARE_GETTER_SETTER_INT(_city_code);
	DECLARE_GETTER_SETTER(double, _x);
	DECLARE_GETTER_SETTER(double, _y);
	const char* get_acctA() const;

	bool execute_set_acct(const wchar_t* acct);
	bool execute_set_addr(const wchar_t* addr);
	bool execute_set_city_code(int city_code);
	bool execute_set_x(double x);
	bool execute_set_y(double y);

	DECLARE_UNCOPYABLE(CCsrInfo);
	DECLARE_SINGLETON(CCsrInfo);
};


NAMESPACE_END
