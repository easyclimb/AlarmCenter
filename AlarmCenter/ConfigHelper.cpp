#include "stdafx.h"
#include "ConfigHelper.h"
#include <fstream>

#include "../contrib/json/json.h"
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

	// section network
	const char* sectionNetwork = "network";
	const char* keyIp = "ip";
	const char* keyPort = "port";
	const char* keyByIpPort = "byIpPort";
	const char* keyDomain = "domain";
	const char* keyAppKey = "appKey";

		// mode
		const char* keyNetworkMode = "mode";

		// section server
		const char* sectionServer = "server";

		const char* keyCsrAcct = "csrAcct";

		// section transmit server 1
		const char* sectionTransmitServer1 = "transmitServer1";
		
		// section transmit server 2
		const char* sectionTransmitServer2 = "transmitServer2";

		// section ezviz private cloud
		const char* sectionEzvizPrivateCloud = "ezvizPrivateCloud";

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
		// section baidu map dlg
		const char* sectionBaiduMapDlg = "baiduMapDlg";


	// section video
	const char* sectionVideo = "video";
		// back-end record time in minutes
		const char* keyBackEndRecordMinutes = "back_end_record_minutes";
		// default video level
		const char* keyDefaultVideoLevel = "default_video_level";
		// auto pley record when alarm if available
		const char* keyAutoPlayRecIfAwailable = "auto_play_rec_if_available";

	// section com
	const char* sectionCom = "com";
		const char* keyRememberComPort = "rem_com_port";
		const char* keyComPort = "com_port";
		const char* keyAutoConn = "auto_conn";

	// section congwin com
	const char* sectionCongwinCom = "congwin";
		const char* keyRouterMode = "router_mode";

	// section sound
	const char* sectionSound = "sound";
		const char* keyPlayAlarmSound = "alarm";
		const char* keyPlayExceptionSound = "exception";
}


CConfigHelper::CConfigHelper()
{
	_cfg_file = get_exe_path();
	_cfg_file += L"\\data\\config\\config.json";
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

	_baidumap_auto_refresh = 1;

	_network_mode = NETWORK_MODE_CSR;
	_listening_port = 12345;

	_csr_acct = "";

	_server1_by_ipport = 0;
	_server1_domain = "hb1212.com";
	_server1_ip = "115.231.175.17";
	_server1_port = 7892;

	_server2_by_ipport = 0;
	_server2_domain = "hb3344.com";
	_server2_ip = "112.16.180.60";
	_server2_port = 7892;

	ezviz_private_cloud_by_ipport_ = 0;
	ezviz_private_cloud_domain_ = "hb1212.com";
	ezviz_private_cloud_ip_ = "115.231.175.17";
	ezviz_private_cloud_port_ = 12346;
	ezviz_private_cloud_app_key_ = "3dab6e65179749febf25013cbe360614";

	_back_end_record_minutes = 10;
	default_video_level_ = 0;
	auto_play_rec_if_available_ = 0;

	play_alarm_sound_ = 1;
	play_exception_sound_ = 1;
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

		// load mode
		int network_mode = value[sectionNetwork][keyNetworkMode].asInt();
		switch (network_mode) {
		case util::NETWORK_MODE_TRANSMIT:
			_network_mode = util::NETWORK_MODE_TRANSMIT;
			break;
		case util::NETWORK_MODE_DUAL:
			_network_mode = util::NETWORK_MODE_DUAL;
			break;
		case util::NETWORK_MODE_CSR:
		default:
			_network_mode = util::NETWORK_MODE_CSR;
			break;
		}

		// load server
		_listening_port = value[sectionNetwork][sectionServer][keyPort].asUInt();
		_csr_acct = value[sectionNetwork][sectionServer][keyCsrAcct].asString();

		// load transmit server 1
		_server1_by_ipport = value[sectionNetwork][sectionTransmitServer1][keyByIpPort].asInt();
		_server1_domain = value[sectionNetwork][sectionTransmitServer1][keyDomain].asString();
		_server1_ip = value[sectionNetwork][sectionTransmitServer1][keyIp].asString();
		_server1_port = value[sectionNetwork][sectionTransmitServer1][keyPort].asUInt();

		// load transmit server 2
		_server2_by_ipport = value[sectionNetwork][sectionTransmitServer2][keyByIpPort].asInt();
		_server2_domain = value[sectionNetwork][sectionTransmitServer2][keyDomain].asString();
		_server2_ip = value[sectionNetwork][sectionTransmitServer2][keyIp].asString();
		_server2_port = value[sectionNetwork][sectionTransmitServer2][keyPort].asUInt();

		// load ezviz private cloud
		ezviz_private_cloud_by_ipport_ = value[sectionNetwork][sectionEzvizPrivateCloud][keyByIpPort].asInt();
		ezviz_private_cloud_domain_ = value[sectionNetwork][sectionEzvizPrivateCloud][keyDomain].asString();
		ezviz_private_cloud_ip_ = value[sectionNetwork][sectionEzvizPrivateCloud][keyIp].asString();
		ezviz_private_cloud_port_ = value[sectionNetwork][sectionEzvizPrivateCloud][keyPort].asUInt();
		ezviz_private_cloud_app_key_ = value[sectionNetwork][sectionEzvizPrivateCloud][keyAppKey].asString();
		if (ezviz_private_cloud_app_key_.empty()) {
			JLOG(L"ezviz_private_cloud_app_key_ is empty.");
			ezviz_private_cloud_app_key_ = "3dab6e65179749febf25013cbe360614";
		}

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
		// baidu map dlg
		rectBaiduMapDlg_.left = value[sectionUi][sectionBaiduMapDlg][keyLeft].asInt();
		rectBaiduMapDlg_.right = value[sectionUi][sectionBaiduMapDlg][keyRight].asInt();
		rectBaiduMapDlg_.top = value[sectionUi][sectionBaiduMapDlg][keyTop].asInt();
		rectBaiduMapDlg_.bottom = value[sectionUi][sectionBaiduMapDlg][keyBottom].asInt();
		maximizedBaiduMapDlg_ = value[sectionUi][sectionBaiduMapDlg][keyMax].asInt();

		// load video
		_back_end_record_minutes = value[sectionVideo][keyBackEndRecordMinutes].asUInt();
		if (_back_end_record_minutes == 0) {
			_back_end_record_minutes = 10;
		}
		default_video_level_ = value[sectionVideo][keyDefaultVideoLevel].asUInt();
		auto_play_rec_if_available_ = value[sectionVideo][keyAutoPlayRecIfAwailable].asUInt();

		// load com
		remember_com_port_ = value[sectionCom][keyRememberComPort].asUInt();
		com_port_ = value[sectionCom][keyComPort].asUInt();
		auto_conn_com_ = value[sectionCom][keyAutoConn].asUInt();

		// load congwin com 
		remember_congwin_com_port_ = value[sectionCongwinCom][keyRememberComPort].asUInt();
		congwin_com_port_ = value[sectionCongwinCom][keyComPort].asUInt();
		auto_conn_congwin_com_ = value[sectionCongwinCom][keyAutoConn].asUInt();
		congwin_fe100_router_mode_ = value[sectionCongwinCom][keyRouterMode].asUInt();

		// load sound
		Json::Value sound_settings;
		sound_settings[keyPlayAlarmSound] = 1;
		sound_settings[keyPlayExceptionSound] = 1;
		sound_settings = value.get(sectionSound, sound_settings);
		play_alarm_sound_ = sound_settings[keyPlayAlarmSound].asUInt();
		play_exception_sound_ = sound_settings[keyPlayExceptionSound].asUInt();

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
		ok2 = save2();
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

	// network 
	// mode
	value[sectionNetwork][keyNetworkMode] = _network_mode;

	// server
	value[sectionNetwork][sectionServer][keyPort] = _listening_port;
	value[sectionNetwork][sectionServer][keyCsrAcct] = _csr_acct;

	// transmit server 1
	value[sectionNetwork][sectionTransmitServer1][keyByIpPort] = _server1_by_ipport;
	value[sectionNetwork][sectionTransmitServer1][keyDomain] = _server1_domain;
	value[sectionNetwork][sectionTransmitServer1][keyIp] = _server1_ip;
	value[sectionNetwork][sectionTransmitServer1][keyPort] = _server1_port;

	// transmit server 2
	value[sectionNetwork][sectionTransmitServer2][keyByIpPort] = _server2_by_ipport;
	value[sectionNetwork][sectionTransmitServer2][keyDomain] = _server2_domain;
	value[sectionNetwork][sectionTransmitServer2][keyIp] = _server2_ip;
	value[sectionNetwork][sectionTransmitServer2][keyPort] = _server2_port;

	// ezviz private cloud
	value[sectionNetwork][sectionEzvizPrivateCloud][keyByIpPort] = ezviz_private_cloud_by_ipport_;
	value[sectionNetwork][sectionEzvizPrivateCloud][keyDomain] = ezviz_private_cloud_domain_;
	value[sectionNetwork][sectionEzvizPrivateCloud][keyIp] = ezviz_private_cloud_ip_;
	value[sectionNetwork][sectionEzvizPrivateCloud][keyPort] = ezviz_private_cloud_port_;
	value[sectionNetwork][sectionEzvizPrivateCloud][keyAppKey] = ezviz_private_cloud_app_key_;

	// ui
	// video player dlg
	value[sectionUi][sectionVideoPlayerDlg][keyLeft] = _rectVideoPlayerDlg.left;
	value[sectionUi][sectionVideoPlayerDlg][keyRight] = _rectVideoPlayerDlg.right;
	value[sectionUi][sectionVideoPlayerDlg][keyTop] = _rectVideoPlayerDlg.top;
	value[sectionUi][sectionVideoPlayerDlg][keyBottom] = _rectVideoPlayerDlg.bottom;
	value[sectionUi][sectionVideoPlayerDlg][keyMax] = _maximizedVideoPlayerDlg;
	value[sectionUi][sectionVideoPlayerDlg][keyShowVideoSameTimeRouteCount] = show_video_same_time_route_count_;
	// baidu map dlg
	value[sectionUi][sectionBaiduMapDlg][keyLeft] = rectBaiduMapDlg_.left;
	value[sectionUi][sectionBaiduMapDlg][keyRight] = rectBaiduMapDlg_.right;
	value[sectionUi][sectionBaiduMapDlg][keyTop] = rectBaiduMapDlg_.top;
	value[sectionUi][sectionBaiduMapDlg][keyBottom] = rectBaiduMapDlg_.bottom;
	value[sectionUi][sectionBaiduMapDlg][keyMax] = maximizedBaiduMapDlg_;

	// video
	value[sectionVideo][keyBackEndRecordMinutes] = _back_end_record_minutes;
	value[sectionVideo][keyDefaultVideoLevel] = default_video_level_;
	value[sectionVideo][keyAutoPlayRecIfAwailable] = auto_play_rec_if_available_;

	// com
	value[sectionCom][keyRememberComPort] = remember_com_port_;
	value[sectionCom][keyComPort] = com_port_;
	value[sectionCom][keyAutoConn] = auto_conn_com_;

	// congwin com 
	value[sectionCongwinCom][keyRememberComPort] = remember_congwin_com_port_;
	value[sectionCongwinCom][keyComPort] = congwin_com_port_;
	value[sectionCongwinCom][keyAutoConn] = auto_conn_congwin_com_;
	value[sectionCongwinCom][keyRouterMode] = congwin_fe100_router_mode_;

	// sound
	value[sectionSound][keyPlayAlarmSound] = play_alarm_sound_;
	value[sectionSound][keyPlayExceptionSound] = play_exception_sound_;

	Json::StyledWriter writer;
	out << writer.write(value);
	out.close();
	return true;
}


bool CConfigHelper::save2() 
{
	using namespace detail;
	std::ofstream out(lang_cfg_); if (!out)return false;
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

	Json::StyledWriter writer;
	out << writer.write(value);
	out.close();
	return true;
}


};
