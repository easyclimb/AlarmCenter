#pragma once

#include "baidu.h"

namespace core {

class csr_manager : private boost::noncopyable
{
private:
	int _level;
	web::BaiduCoordinate _coor;
public:
	~csr_manager();

	DECLARE_GETTER_SETTER_INT(_level);
	DECLARE_GETTER_SETTER(web::BaiduCoordinate, _coor);

	bool execute_set_zoom_level(int level);
	bool execute_set_coor(const web::BaiduCoordinate& coor);

	DECLARE_SINGLETON(csr_manager);
};


};

