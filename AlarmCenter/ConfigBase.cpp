#include "stdafx.h"
#include "ConfigBase.h"


namespace util {

	CConfigBase::CConfigBase(const std::string& cfg_file_path)
	{
		cfg_file_path_ = cfg_file_path;
	}


	CConfigBase::~CConfigBase()
	{
	}



}
