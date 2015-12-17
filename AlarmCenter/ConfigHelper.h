#pragma once
#include <string>

namespace util {

typedef enum ApplicationLanguage
{
	AL_CHINESE = 0,
	AL_ENGLISH,
	AL_TAIWANESE,
}ApplicationLanguage;

extern std::wstring get_exe_path();

class CConfigHelper
{
public:
	ApplicationLanguage GetLanguage() { return _lang; }

	int get_baidumap_auto_refresh() const { return _baidumap_auto_refresh; }
	void set_baidumap_auto_refresh(int b) { _baidumap_auto_refresh = b; save(); }



	~CConfigHelper();
private:
	std::wstring _cfg_file;
	ApplicationLanguage _lang;
	int _baidumap_auto_refresh = 1;
protected:
	bool load();
	bool save();
	DECLARE_UNCOPYABLE(CConfigHelper)
	DECLARE_SINGLETON(CConfigHelper)
};

NAMESPACE_END

