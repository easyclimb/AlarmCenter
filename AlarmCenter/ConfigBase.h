#pragma once

namespace util {

	class CConfigBase
	{
	public:
		explicit CConfigBase(const std::string& cfg_file_path);
		virtual ~CConfigBase();

	protected:
		std::string cfg_file_path_ = "";

	};

}
