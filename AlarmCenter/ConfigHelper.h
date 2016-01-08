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

	std::string get_csr_acct() const { return _csr_acct; }
	void set_csr_acct(const std::string& acct) { if (_csr_acct != acct) { _csr_acct = acct; save(); } }
	
	int get_server1_by_ipport() const { return _server1_by_ipport; }
	void set_server1_by_ipport(int b) { if (_server1_by_ipport != b) { _server1_by_ipport = b; save(); } }
	std::string get_server1_domain() const { return _server1_domain; }
	void set_server1_domain(const std::string& domain) { if (_server1_domain != domain) { _server1_domain = domain; save(); } }
	std::string get_server1_ip() const { return _server1_ip; }
	void set_server1_ip(const std::string& ip) { if (_server1_ip == ip) return; _server1_ip = ip; save(); }
	unsigned int get_server1_port() const { return _server1_port; }
	void set_server1_port(unsigned int port) { if (_server1_port == port) return; _server1_port = port; save(); }

	int get_server2_by_ipport() const { return _server2_by_ipport; }
	void set_server2_by_ipport(int b) { if (_server2_by_ipport != b) { _server2_by_ipport = b; save(); } }
	std::string get_server2_domain() const { return _server2_domain; }
	void set_server2_domain(const std::string& domain) { if (_server2_domain != domain) { _server2_domain = domain; save(); } }
	std::string get_server2_ip() const { return _server2_ip; }
	void set_server2_ip(const std::string& ip) { if (_server2_ip == ip) return; _server2_ip = ip; save(); }
	unsigned int get_server2_port() const { return _server2_port; }
	void set_server2_port(unsigned int port) { if (_server2_port == port) return; _server2_port = port; save(); }


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
	std::string _csr_acct = "";
	int _server1_by_ipport = 0;
	std::string _server1_domain;
	std::string _server1_ip = "";
	unsigned int _server1_port = 7892;

	int _server2_by_ipport = 1;
	std::string _server2_domain = "";
	std::string _server2_ip = "";
	unsigned int _server2_port = 7892;


protected:
	void init();
	bool load();
	bool save();
	DECLARE_SINGLETON(CConfigHelper)
};

NAMESPACE_END

