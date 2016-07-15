#include "stdafx.h"
#include "ConfigHelper.h"
#include <fstream>

#include "../json/json.h"
using namespace Json;

namespace util {

//IMPLEMENT_SINGLETON(CConfigHelper)

namespace detail {

	// section app
	const char* sectionApplication = "applicationConfig";
		const char* keyLanguage = "language";
		const char* valChinese = "zh-cn";
		const char* valEnglish = "en-us";
		const char* valTaiwanese = "zh-tw";

	// section baidumap
	const char* sectionBaiduMap = "baiduMapConfig";
		const char* keyAutoRefresh = "autoRefresh";

	// section ui
	const char* sectionUi = "ui";
		// section baidu map dlg
		const char* sectionBaiduMapDlg = "baiduMapDlg";
		const char* keyLeft = "left";
		const char* keyRight = "right";
		const char* keyTop = "top";
		const char* keyBottom = "bottom";
		const char* keyMax = "max";
}


CConfigHelper::CConfigHelper()
{
	_cfg_file = get_exe_path();
	_cfg_file += L"\\data\\config\\map_config.json";
	lang_cfg_ = get_exe_path();
	lang_cfg_ += L"\\data\\config\\lang.json";
	load();
}


CConfigHelper::~CConfigHelper()
{

}


void CConfigHelper::init()
{
	_baidumap_auto_refresh = 1;
}


bool CConfigHelper::load()
{
	using namespace detail;
	bool ok1 = false, ok2 = false;

	// load config
	do {
		std::ifstream in(_cfg_file); if (!in) break;
		Reader reader;
		Value value;
		if (!reader.parse(in, value)) break;

		// load baidumap
		_baidumap_auto_refresh = value[sectionBaiduMap][keyAutoRefresh].asInt();

		// baidu map dlg
		rectBaiduMapDlg_.left = value[sectionUi][sectionBaiduMapDlg][keyLeft].asInt();
		rectBaiduMapDlg_.right = value[sectionUi][sectionBaiduMapDlg][keyRight].asInt();
		rectBaiduMapDlg_.top = value[sectionUi][sectionBaiduMapDlg][keyTop].asInt();
		rectBaiduMapDlg_.bottom = value[sectionUi][sectionBaiduMapDlg][keyBottom].asInt();
		maximizedBaiduMapDlg_ = value[sectionUi][sectionBaiduMapDlg][keyMax].asInt();

		in.close();
		ok1 = true;
	} while (false);

	// load language
	do {
		std::ifstream in(lang_cfg_); if (!in) break;
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
		cur_lang_ = _lang;

		in.close();
		ok2 = true;
	} while (false);

	if (!ok1) {
		init();
		ok1 = save();
	}

	if (!ok2) {
		_lang = AL_CHINESE;
		cur_lang_ = AL_CHINESE;
		ok2 = true;
	}

	return ok1 && ok2;
}


bool CConfigHelper::save()
{
	using namespace detail;
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
	
	// baidumap config
	value[sectionBaiduMap][keyAutoRefresh] = _baidumap_auto_refresh;

	// baidu map dlg
	value[sectionUi][sectionBaiduMapDlg][keyLeft] = rectBaiduMapDlg_.left;
	value[sectionUi][sectionBaiduMapDlg][keyRight] = rectBaiduMapDlg_.right;
	value[sectionUi][sectionBaiduMapDlg][keyTop] = rectBaiduMapDlg_.top;
	value[sectionUi][sectionBaiduMapDlg][keyBottom] = rectBaiduMapDlg_.bottom;
	value[sectionUi][sectionBaiduMapDlg][keyMax] = maximizedBaiduMapDlg_;

	Json::StyledWriter writer;
	out << writer.write(value);
	out.close();
	return true;
}
};
