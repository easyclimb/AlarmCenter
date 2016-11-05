#pragma once
#include <string>
#include <boost/noncopyable.hpp>

namespace util {

typedef enum ApplicationLanguage
{
	AL_CHINESE = 0,
	AL_TAIWANESE,
	AL_ENGLISH,
}ApplicationLanguage;

inline ApplicationLanguage Integer2ApplicationLanguage(int lang) {
	switch (lang) {
	case util::AL_TAIWANESE:
		return util::AL_TAIWANESE;
		break;
	case util::AL_ENGLISH:
		return util::AL_ENGLISH;
		break;
	case util::AL_CHINESE:
	default:
		return util::AL_CHINESE;
		break;
	}
}

typedef enum NetworkMode {
	NETWORK_MODE_CSR = 1, // 直连型接警中心
	NETWORK_MODE_TRANSMIT = 2, // 中转型接警中心
	NETWORK_MODE_DUAL = NETWORK_MODE_CSR | NETWORK_MODE_TRANSMIT, // 混合模式
}NetworkMode;


class CConfigHelper : public dp::singleton<CConfigHelper>
{
public:
	ApplicationLanguage get_current_language() { return cur_lang_; }
	ApplicationLanguage get_language() { return _lang; }
	void set_language(ApplicationLanguage lang) { if (lang == _lang) return; _lang = lang; }


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
	
	int get_ezviz_private_cloud_by_ipport() const { return ezviz_private_cloud_by_ipport_; }
	void set_ezviz_private_cloud_by_ipport(int b) { if (ezviz_private_cloud_by_ipport_ != b) { ezviz_private_cloud_by_ipport_ = b; save(); } }
	std::string get_ezviz_private_cloud_domain() const { return ezviz_private_cloud_domain_; }
	void set_ezviz_private_cloud_domain(const std::string& domain) { if (ezviz_private_cloud_domain_ != domain) { ezviz_private_cloud_domain_ = domain; save(); } }
	std::string get_ezviz_private_cloud_ip() const { return ezviz_private_cloud_ip_; }
	void set_ezviz_private_cloud_ip(const std::string& ip) { if (ezviz_private_cloud_ip_ == ip) return; ezviz_private_cloud_ip_ = ip; save(); }
	unsigned int get_ezviz_private_cloud_port() const { return ezviz_private_cloud_port_; }
	void set_ezviz_private_cloud_port(unsigned int port) { if (ezviz_private_cloud_port_ == port) return; ezviz_private_cloud_port_ = port; save(); }
	std::string get_ezviz_private_cloud_app_key() const { return ezviz_private_cloud_app_key_; }
	void set_ezviz_private_cloud_app_key(const std::string& appKey) { if (ezviz_private_cloud_app_key_ == appKey) return; ezviz_private_cloud_app_key_ = appKey; save(); }
	
	// ui
	// video player dlg
	CRect get_rectVideoPlayerDlg() const { return _rectVideoPlayerDlg; }
	void set_rectVideoPlayerDlg(const CRect& rect) { if (rect == _rectVideoPlayerDlg) return; _rectVideoPlayerDlg = rect; save(); }
	int get_maximizedVideoPlayerDlg() const { return _maximizedVideoPlayerDlg; }
	void set_maximizedVideoPlayerDlg(int m) { if (m == _maximizedVideoPlayerDlg) return; _maximizedVideoPlayerDlg = m; save(); }
	int get_show_video_same_time_route_count() const { return show_video_same_time_route_count_; }
	void set_show_video_same_time_route_count(int cnt) { if (show_video_same_time_route_count_ == cnt)return; show_video_same_time_route_count_ = cnt; save(); }
	// baidu map dlg
	CRect get_rectBaiduMapDlg() const { return rectBaiduMapDlg_; }
	void set_rectBaiduMapDlg(const CRect& rc) { if (rc == rectBaiduMapDlg_) return; rectBaiduMapDlg_ = rc; save(); }
	int get_maximizedBaiduMapDlg() const { return maximizedBaiduMapDlg_; }
	void set_maximizedBaiduMapDlg(int m) { if (m == maximizedBaiduMapDlg_) return; maximizedBaiduMapDlg_ = m; save(); }

	// video
	int get_back_end_record_minutes() const { return _back_end_record_minutes; }
	void set_back_end_record_minutes(int minutes) { if (minutes == _back_end_record_minutes) return; _back_end_record_minutes = minutes; save(); }
	int get_default_video_level() const { return default_video_level_; }
	void set_default_video_level(int level) { if (level == default_video_level_) return; default_video_level_ = level; save(); }
	int get_auto_play_rec_if_available() const { return auto_play_rec_if_available_; }
	void set_auto_play_rec_if_available(int b) { if (b == auto_play_rec_if_available_) return; auto_play_rec_if_available_ = b; save(); }

	// com
	int get_remember_com_port() const { return remember_com_port_; }
	void set_remember_com_port(int rem) { if (rem == remember_com_port_) return; remember_com_port_ = rem; save(); }
	int get_com_port() const { return com_port_; }
	void set_com_port(int port) { if (port == com_port_) return; com_port_ = port; save(); }
	int get_auto_conn_com() const { return auto_conn_com_; }
	void set_auto_conn_com(int auto_conn) { if (auto_conn == auto_conn_com_) return; auto_conn_com_ = auto_conn; save(); }

	// congwin fe100 com
	int get_remember_congwin_com_port() const { return remember_congwin_com_port_; }
	void set_remember_congwin_com_port(int rem) { if (rem == remember_congwin_com_port_) return; remember_congwin_com_port_ = rem; save(); }
	int get_congwin_com_port() const { return congwin_com_port_; }
	void set_congwin_com_port(int port) { if (port == congwin_com_port_) return; congwin_com_port_ = port; save(); }
	int get_auto_conn_congwin_com() const { return auto_conn_congwin_com_; }
	void set_auto_conn_congwin_com(int auto_conn) { if (auto_conn == auto_conn_congwin_com_) return; auto_conn_congwin_com_ = auto_conn; save(); }

	void save_to_file() { save(); save2(); }
	~CConfigHelper();
private:
	std::wstring _cfg_file = L"";
	std::wstring lang_cfg_ = L"";
	ApplicationLanguage _lang = AL_CHINESE;
	ApplicationLanguage cur_lang_ = AL_CHINESE;
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

	// ezviz private cloud service
	int ezviz_private_cloud_by_ipport_ = 0;
	std::string ezviz_private_cloud_domain_ = "";
	std::string ezviz_private_cloud_ip_ = "";
	unsigned int ezviz_private_cloud_port_ = 12346;
	std::string ezviz_private_cloud_app_key_ = "";

	// ui
	// video player dlg
	CRect _rectVideoPlayerDlg = { 0, 0, 0, 0 };
	int _maximizedVideoPlayerDlg = 0;
	int show_video_same_time_route_count_ = 1;
	// baidu map dlg
	CRect rectBaiduMapDlg_ = { 0,0,0,0 };
	int maximizedBaiduMapDlg_ = 0;

	// video
	int _back_end_record_minutes = 10;
	int default_video_level_ = 0;
	int auto_play_rec_if_available_ = 0;

	// com 
	int remember_com_port_ = 0;
	int com_port_ = 0;
	int auto_conn_com_ = 0;

	// congwin fe100 com
	int remember_congwin_com_port_ = 0;
	int congwin_com_port_ = 0;
	int auto_conn_congwin_com_ = 0;

protected:
	void init();
	bool load();
	bool save();
	bool save2();
	//DECLARE_SINGLETON(CConfigHelper)

protected:
	CConfigHelper();
};

};

