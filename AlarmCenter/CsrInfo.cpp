#include "stdafx.h"
#include "CsrInfo.h"
#include "json\json.h"

namespace core {

namespace detail {
auto section_map = "map";
auto key_x = "x";
auto key_y = "y";
auto key_zoom_level = "zoom_level";


}

using namespace detail;

IMPLEMENT_SINGLETON(csr_manager)

csr_manager::csr_manager()
{
	cfg_path_ = get_exe_path();
	cfg_path_ += L"\\data\\config\\center.json";
	load();
}


csr_manager::~csr_manager()
{
}


void csr_manager::init()
{
	coor_.x = 108.953;
	coor_.y = 34.2778, 14;
	level_ = 14;
}


bool csr_manager::load()
{
	using namespace Json;
	do {
		std::ifstream in(cfg_path_); if (!in) break;
		Reader reader;
		Value value;
		if (!reader.parse(in, value)) break;

		coor_.x = value[section_map][key_x].asDouble();
		coor_.y = value[section_map][key_y].asDouble();
		level_ = value[section_map][key_zoom_level].asUInt();
		if (level_ < 1 || 20 < level_) {
			level_ = 14;
		}

		return true;
	} while (false);

	init();
	return save();
}


bool csr_manager::save()
{
	using namespace detail;
	std::ofstream out(cfg_path_); if (!out)return false;
	Json::Value value;

	value[section_map][key_x] = coor_.x;
	value[section_map][key_y] = coor_.y;
	value[section_map][key_zoom_level] = level_;

	Json::StyledWriter writer;
	out << writer.write(value);
	out.close();

	return true;
}


bool csr_manager::execute_set_zoom_level(int level)
{
	if (level != level_) {
		level_ = level;
		save();
	}
	return true;
}


bool csr_manager::execute_set_coor(const web::BaiduCoordinate& coor)
{
	if (coor_ != coor) {
		coor_ = coor;
		save();
	}
	return true;
}



};
