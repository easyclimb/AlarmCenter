#pragma once
#include <string>

namespace util {

class CConfigBase
{
public:
	explicit CConfigBase();
	virtual ~CConfigBase();

protected:
	std::wstring cfg_path_ = L"";
	void init();
	bool load();
	bool save();
};

}
