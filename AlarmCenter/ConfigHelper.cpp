#include "stdafx.h"
#include "ConfigHelper.h"
#include <fstream>

#include "json/json.h"
using namespace Json;

namespace util {

IMPLEMENT_SINGLETON(CConfigHelper)

namespace {

	// section app
	const char* sectionApplication = "applicationConfig";
	const char* keyLanguage = "language";
	const char* valChinese = "Chinese";
	const char* valEnglish = "English";
	const char* valTaiwanese = "Taiwanese";

	// section baidumap
	const char* sectionBaiduMap = "baiduMapConfig";
	const char* keyAutoRefresh = "autoRefresh";

}

std::wstring get_exe_path()
{
	wchar_t path[1024] = { 0 };
	GetModuleFileName(nullptr, path, 1024);
	std::wstring::size_type pos = std::wstring(path).find_last_of(L"\\/");
	return std::wstring(path).substr(0, pos);
}

CConfigHelper::CConfigHelper()
	: _cfg_file(L"")
	, _lang(AL_CHINESE)
{
	_cfg_file = get_exe_path();
	_cfg_file += L"\\config\\config.json";
	load();
}


CConfigHelper::~CConfigHelper()
{

}


bool CConfigHelper::load()
{
	do {
		std::ifstream in(_cfg_file); if (!in) break;
		Reader reader;
		Value value;
		if (!reader.parse(in, value)) break;

		// load language
		std::string lang = value[sectionApplication][keyLanguage].asString();
		if (lang == valChinese) {
			_lang = AL_CHINESE;
		} else if (lang == valEnglish) {
			_lang = AL_ENGLISH;
		} else if (lang == valTaiwanese) {
			_lang = AL_TAIWANESE;
		} else { assert(0); break; }

		// load baidumap
		_baidumap_auto_refresh = value[sectionBaiduMap][keyAutoRefresh].asInt();

		in.close();
		return true;
	} while (false);

	// save default value
	_lang = AL_CHINESE;
	_baidumap_auto_refresh = 1;
	return save();
}


bool CConfigHelper::save()
{
	std::ofstream out(_cfg_file); if (!out)return false;
	Json::Value value;

	// save app config
	switch (_lang) {
	case util::AL_ENGLISH:
		value[sectionApplication][keyLanguage] = valEnglish;
		break;
	case util::AL_TAIWANESE:
		value[sectionApplication][keyLanguage] = valTaiwanese;
		break;
	case util::AL_CHINESE:
	default:
		value[sectionApplication][keyLanguage] = valChinese;
		break;
	}
	
	// save baidumap config
	value[sectionBaiduMap][keyAutoRefresh] = _baidumap_auto_refresh;


	Json::StyledWriter writer;
	out << writer.write(value);
	out.close();
	return true;
}


NAMESPACE_END
