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
		const char* valChinese = "Chinese";
		const char* valEnglish = "English";
		const char* valTaiwanese = "Taiwanese";

	// section baidumap
	const char* sectionBaiduMap = "baiduMapConfig";
		const char* keyAutoRefresh = "autoRefresh";

	// section network
	const char* sectionNetwork = "network";
		// mode
		const char* keyNetworkMode = "mode";

		// section server
		const char* sectionServer = "server";
			const char* keyServerPort = "port";

		// section transmit server
		const char* sectionTransmitServer = "transmitServer";
			const char* keyServerIp = "ip";
			//const char* keyServerPort = "port";
			const char* keyServerIpBk = "ipBakcup";
			const char* keyServerPortBk = "portBackup";


}

std::wstring get_exe_path()
{
	wchar_t path[1024] = { 0 };
	GetModuleFileName(nullptr, path, 1024);
	std::wstring::size_type pos = std::wstring(path).find_last_of(L"\\/");
	return std::wstring(path).substr(0, pos);
}

CConfigHelper::CConfigHelper()
{
	_cfg_file = get_exe_path();
	_cfg_file += L"\\config\\config.json";
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

	_server_ip = "112.16.180.60";
	_server_port = 7892;
	_server_ip_bk = "113.140.30.118";
	_server_port_bk = 7892;
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
		_listening_port = value[sectionNetwork][sectionServer][keyServerPort].asUInt();

		// load transmit server
		_server_ip = value[sectionNetwork][sectionTransmitServer][keyServerIp].asString();
		_server_port = value[sectionNetwork][sectionTransmitServer][keyServerPort].asUInt();
		_server_ip_bk = value[sectionNetwork][sectionTransmitServer][keyServerIpBk].asString();
		_server_port_bk = value[sectionNetwork][sectionTransmitServer][keyServerPortBk].asUInt();

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
	value[sectionNetwork][sectionServer][keyServerPort] = _listening_port;

	// save transmit server
	value[sectionNetwork][sectionTransmitServer][keyServerIp] = _server_ip;
	value[sectionNetwork][sectionTransmitServer][keyServerPort] = _server_port;
	value[sectionNetwork][sectionTransmitServer][keyServerIpBk] = _server_ip_bk;
	value[sectionNetwork][sectionTransmitServer][keyServerPortBk] = _server_port_bk;

	Json::StyledWriter writer;
	out << writer.write(value);
	out.close();
	return true;
}


NAMESPACE_END
