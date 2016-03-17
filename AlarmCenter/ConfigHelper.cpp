#include "stdafx.h"
#include "ConfigHelper.h"
#include <fstream>

#include "json/json.h"
using namespace Json;

namespace util {

IMPLEMENT_SINGLETON(CConfigHelper)

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

		// mode
		const char* keyNetworkMode = "mode";

		// section server
		const char* sectionServer = "server";

		const char* keyCsrAcct = "csrAcct";

		// section transmit server 1
		const char* sectionTransmitServer1 = "transmitServer1";
		
		// section transmit server 2
		const char* sectionTransmitServer2 = "transmitServer2";

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

	std::wstring get_exe_path()
	{
		wchar_t path[1024] = { 0 };
		GetModuleFileName(nullptr, path, 1024);
		std::wstring::size_type pos = std::wstring(path).find_last_of(L"\\/");
		return std::wstring(path).substr(0, pos);
	}

}


CConfigHelper::CConfigHelper()
{
	_cfg_file = detail::get_exe_path();
	_cfg_file += L"\\data\\config\\config.json";
	load();
}


CConfigHelper::~CConfigHelper()
{

}


void CConfigHelper::init()
{
	// save default value

	_lang = AL_CHINESE;

	_baidumap_auto_refresh = 1;

	_network_mode = NETWORK_MODE_CSR;
	_listening_port = 12345;

	_csr_acct = "";

	_server1_by_ipport = 0;
	_server1_domain = "hb1212.com";
	_server1_ip = "112.16.180.60";
	_server1_port = 7892;

	_server2_by_ipport = 1;
	_server2_domain = "";
	_server2_ip = "113.140.30.118";
	_server2_port = 7892;
}


bool CConfigHelper::load()
{
	using namespace detail;
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


		// load ui
		// video player dlg
		_rectVideoPlayerDlg.left = value[sectionUi][sectionVideoPlayerDlg][keyLeft].asInt();
		_rectVideoPlayerDlg.right = value[sectionUi][sectionVideoPlayerDlg][keyRight].asInt();
		_rectVideoPlayerDlg.top = value[sectionUi][sectionVideoPlayerDlg][keyTop].asInt();
		_rectVideoPlayerDlg.bottom = value[sectionUi][sectionVideoPlayerDlg][keyBottom].asInt();
		_maximizedVideoPlayerDlg = value[sectionUi][sectionVideoPlayerDlg][keyMax].asUInt();

		in.close();
		return true;
	} while (false);

	init();
	return save();
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
	
	// save baidumap config
	value[sectionBaiduMap][keyAutoRefresh] = _baidumap_auto_refresh;

	// save network 
	// save mode
	value[sectionNetwork][keyNetworkMode] = _network_mode;

	// save server
	value[sectionNetwork][sectionServer][keyPort] = _listening_port;
	value[sectionNetwork][sectionServer][keyCsrAcct] = _csr_acct;

	// save transmit server 1
	value[sectionNetwork][sectionTransmitServer1][keyByIpPort] = _server1_by_ipport;
	value[sectionNetwork][sectionTransmitServer1][keyDomain] = _server1_domain;
	value[sectionNetwork][sectionTransmitServer1][keyIp] = _server1_ip;
	value[sectionNetwork][sectionTransmitServer1][keyPort] = _server1_port;

	// save transmit server 2
	value[sectionNetwork][sectionTransmitServer2][keyByIpPort] = _server2_by_ipport;
	value[sectionNetwork][sectionTransmitServer2][keyDomain] = _server2_domain;
	value[sectionNetwork][sectionTransmitServer2][keyIp] = _server2_ip;
	value[sectionNetwork][sectionTransmitServer2][keyPort] = _server2_port;

	// save ui
	// video player dlg
	value[sectionUi][sectionVideoPlayerDlg][keyLeft] = _rectVideoPlayerDlg.left;
	value[sectionUi][sectionVideoPlayerDlg][keyRight] = _rectVideoPlayerDlg.right;
	value[sectionUi][sectionVideoPlayerDlg][keyTop] = _rectVideoPlayerDlg.top;
	value[sectionUi][sectionVideoPlayerDlg][keyBottom] = _rectVideoPlayerDlg.bottom;
	value[sectionUi][sectionVideoPlayerDlg][keyMax] = _maximizedVideoPlayerDlg;


	Json::StyledWriter writer;
	out << writer.write(value);
	out.close();
	return true;
}


NAMESPACE_END
