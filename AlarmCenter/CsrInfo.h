#pragma once

#include "baidu.h"

namespace core {

class csr_manager : private boost::noncopyable
{
private:
	int level_ = 14;
	web::BaiduCoordinate coor_ = {};

	std::wstring cfg_path_ = L"";

protected:
	void init();
	bool load();
	bool save();

public:
	~csr_manager();

	int get_level() const { return level_; };
	web::BaiduCoordinate get_coor() const { return coor_; };

	bool execute_set_zoom_level(int level);
	bool execute_set_coor(const web::BaiduCoordinate& coor);

	DECLARE_SINGLETON(csr_manager);
};


};

