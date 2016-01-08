#pragma once
#include <string>
#include <boost/noncopyable.hpp>

namespace util {

typedef enum ApplicationLanguage
{
	AL_CHINESE = 0,
	AL_ENGLISH,
	AL_TAIWANESE,
}ApplicationLanguage;

typedef enum NetworkMode {
	NETWORK_MODE_CSR = 1, // 直连型接警中心
	NETWORK_MODE_TRANSMIT = 2, // 中转型接警中心
	NETWORK_MODE_DUAL = NETWORK_MODE_CSR | NETWORK_MODE_TRANSMIT, // 混合模式
}NetworkMode;

extern std::wstring get_exe_path();

class CConfigHelper : public boost::noncopyable
{
public:
	ApplicationLanguage GetLanguage() { return _lang; }

	int get_baidumap_auto_refresh() const { return _baidumap_auto_refresh; }
	void set_baidumap_auto_refresh(int b) { if (b == _baidumap_auto_refresh) return; _baidumap_auto_refresh = b; save(); }

	NetworkMode get_network_mode() const { return _network_mode; }
	void set_network_mode(NetworkMode mode) { if (_network_mode != mode) { _network_mode = mode; save(); } }

	unsigned int get_listening_port() const { return _listening_port; }
	void set_listening_port(unsigned int port) { if (_listening_port == port) return; _listening_port = port; save(); }
	
	std::string get_server_ip() const { return _server_ip; }
	void set_server_ip(const std::string& ip) { if (_server_ip == ip) return; _server_ip = ip; save(); }
	unsigned int get_server_port() const { return _server_port; }
	void set_server_port(unsigned int port) { if (_server_port == port) return; _server_port = port; save(); }

	std::string get_server_ip_bk() const { return _server_ip_bk; }
	void set_server_ip_bk(const std::string& ip) { if (_server_ip_bk == ip) return; _server_ip_bk = ip; save(); }
	unsigned int get_server_port_bk() const { return _server_port_bk; }
	void set_server_port_bk(unsigned int port) { if (_server_port_bk == port) return; _server_port_bk = port; save(); }


	~CConfigHelper();
private:
	std::wstring _cfg_file = L"";
	ApplicationLanguage _lang = AL_CHINESE;
	int _baidumap_auto_refresh = 1;

	// network
	NetworkMode _network_mode = NETWORK_MODE_CSR;
	// server listening port
	unsigned int _listening_port = 12345;
	// transmit server 2016-1-6 16:39:52
	std::string _server_ip = "";
	unsigned int _server_port = 7892;
	std::string _server_ip_bk = "";
	unsigned int _server_port_bk = 7892;


protected:
	void init();
	bool load();
	bool save();
	DECLARE_SINGLETON(CConfigHelper)
};

NAMESPACE_END

