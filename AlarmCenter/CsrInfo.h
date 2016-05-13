#pragma once

#include "baidu.h"

namespace core {

class csr_manager : private boost::noncopyable
{
private:
	//CString _acct;
	CString _addr;
	int _city_code;
	int _level;
	web::BaiduCoordinate _coor;
public:
	~csr_manager();

	//DECLARE_GETTER_SETTER_STRING(_acct);
	DECLARE_GETTER_SETTER_STRING(_addr);
	DECLARE_GETTER_SETTER_INT(_city_code);
	DECLARE_GETTER_SETTER_INT(_level);
	//DECLARE_GETTER_SETTER(double, _x);
	DECLARE_GETTER_SETTER(web::BaiduCoordinate, _coor);
	//const char* get_acctA() const;

	//bool execute_set_acct(const wchar_t* acct);
	bool execute_set_addr(const wchar_t* addr);
	bool execute_set_city_code(int city_code);
	bool execute_set_zoom_level(int level);
	//bool execute_set_x(double x);
	//bool execute_set_y(double y);
	bool execute_set_coor(const web::BaiduCoordinate& coor);

	DECLARE_SINGLETON(csr_manager);
};


};

