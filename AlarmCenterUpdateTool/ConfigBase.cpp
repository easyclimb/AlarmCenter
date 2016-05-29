#include "stdafx.h"
#include "ConfigBase.h"
#include "../json/json.h"

namespace util {

namespace detail {

}

CConfigBase::CConfigBase()
{
	cfg_path_ = get_exe_path();
	cfg_path_ += L"\\data\\config\\group.json";
	load();
}


CConfigBase::~CConfigBase()
{
}



void CConfigBase::init()
{

}


bool CConfigBase::load()
{
	using namespace Json;
	do {
		std::ifstream in(cfg_path_); if (!in) break;
		Reader reader;
		Value value;
		if (!reader.parse(in, value)) break;



	} while (false);

	return save();
}


bool CConfigBase::save()
{
	using namespace detail;
	std::ofstream out(cfg_path_); if (!out)return false;
	Json::Value value;



	Json::StyledWriter writer;
	out << writer.write(value);
	out.close();

	return true;
}
}
