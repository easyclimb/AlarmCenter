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

	// section ui
	const char* sectionUi = "ui";
		// section video player dlg
		const char* sectionVideoPlayerDlg = "videoPlayerDlg";
			// pos, maximized
			const char* keyLeft = "left";
			const char* keyRight = "right";
			const char* keyTop = "top";
			const char* keyBottom = "bottom";
			const char* keyMax = "max";
			const char* keyShowVideoSameTimeRouteCount = "show_video_same_time_route_count";

	// section video
	const char* sectionVideo = "video";
		// back-end record time in minutes
		const char* keyBackEndRecordMinutes = "back_end_record_minutes";
		// default video level
		const char* keyDefaultVideoLevel = "default_video_level";
		// auto pley record when alarm if available
		const char* keyAutoPlayRecIfAwailable = "auto_play_rec_if_available";

	// section network
	const char* sectionNetwork = "network";
	const char* keyIp = "ip";
	const char* keyPort = "port";
	// section ezviz private cloud
	const char* sectionEzvizPrivateCloud = "ezvizPrivateCloud";
	const char* keyAppKey = "appKey";
}


CConfigHelper::CConfigHelper()
{
	_cfg_file = get_exe_path();
	_cfg_file += L"\\data\\config\\video_config.json";
	lang_cfg_ = get_exe_path();
	lang_cfg_ += L"\\data\\config\\lang.json";
	load();
}


CConfigHelper::~CConfigHelper()
{

}


void CConfigHelper::init()
{
	// save default value

	//_lang = AL_CHINESE;
	//cur_lang_ = AL_CHINESE;

	_back_end_record_minutes = 10;
	default_video_level_ = 0;
	auto_play_rec_if_available_ = 0;
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

		// load ui
		// video player dlg
		_rectVideoPlayerDlg.left = value[sectionUi][sectionVideoPlayerDlg][keyLeft].asInt();
		_rectVideoPlayerDlg.right = value[sectionUi][sectionVideoPlayerDlg][keyRight].asInt();
		_rectVideoPlayerDlg.top = value[sectionUi][sectionVideoPlayerDlg][keyTop].asInt();
		_rectVideoPlayerDlg.bottom = value[sectionUi][sectionVideoPlayerDlg][keyBottom].asInt();
		_maximizedVideoPlayerDlg = value[sectionUi][sectionVideoPlayerDlg][keyMax].asUInt();
		show_video_same_time_route_count_ = value[sectionUi][sectionVideoPlayerDlg][keyShowVideoSameTimeRouteCount].asUInt();
		if (show_video_same_time_route_count_ == 0) {
			show_video_same_time_route_count_ = 1;
		}

		// load video
		_back_end_record_minutes = value[sectionVideo][keyBackEndRecordMinutes].asUInt();
		if (_back_end_record_minutes == 0) {
			_back_end_record_minutes = 10;
		}
		default_video_level_ = value[sectionVideo][keyDefaultVideoLevel].asUInt();
		auto_play_rec_if_available_ = value[sectionVideo][keyAutoPlayRecIfAwailable].asUInt();

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

	// load ezviz app key from config.json
	bool ok3 = true;
	do{
		auto path = get_exe_path_a() + "\\data\\config\\config.json";
		std::ifstream in(path);
		if (!in) {
			ok3 = false;
			break;
		}

		Reader reader;
		Value value;
		if (!reader.parse(in, value)) break;

		ezviz_private_cloud_ip_ = value[sectionNetwork][sectionEzvizPrivateCloud][keyIp].asString();
		ezviz_private_cloud_port_ = value[sectionNetwork][sectionEzvizPrivateCloud][keyPort].asUInt();
		ezviz_app_key_ = value[sectionNetwork][sectionEzvizPrivateCloud][keyAppKey].asString();
		if (ezviz_app_key_.empty()) {
			ok3 = false;
			break;
		}

		in.close();
	} while (0);

	if (!ok3) {
		ezviz_private_cloud_ip_ = "115.231.175.17";
		ezviz_private_cloud_port_ = 12346;
		ezviz_app_key_ = "3dab6e65179749febf25013cbe360614";
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
	
	// ui
	// video player dlg
	value[sectionUi][sectionVideoPlayerDlg][keyLeft] = _rectVideoPlayerDlg.left;
	value[sectionUi][sectionVideoPlayerDlg][keyRight] = _rectVideoPlayerDlg.right;
	value[sectionUi][sectionVideoPlayerDlg][keyTop] = _rectVideoPlayerDlg.top;
	value[sectionUi][sectionVideoPlayerDlg][keyBottom] = _rectVideoPlayerDlg.bottom;
	value[sectionUi][sectionVideoPlayerDlg][keyMax] = _maximizedVideoPlayerDlg;
	value[sectionUi][sectionVideoPlayerDlg][keyShowVideoSameTimeRouteCount] = show_video_same_time_route_count_;

	// video
	value[sectionVideo][keyBackEndRecordMinutes] = _back_end_record_minutes;
	value[sectionVideo][keyDefaultVideoLevel] = default_video_level_;
	value[sectionVideo][keyAutoPlayRecIfAwailable] = auto_play_rec_if_available_;

	Json::StyledWriter writer;
	out << writer.write(value);
	out.close();
	return true;
}

};
