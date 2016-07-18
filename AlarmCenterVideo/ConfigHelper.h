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


class CConfigHelper : public dp::singleton<CConfigHelper>
{
public:
	ApplicationLanguage get_language() { return _lang; }
		
	// ui
	// video player dlg
	CRect get_rectVideoPlayerDlg() const { return _rectVideoPlayerDlg; }
	void set_rectVideoPlayerDlg(const CRect& rect) { if (rect == _rectVideoPlayerDlg) return; _rectVideoPlayerDlg = rect; save(); }
	int get_maximizedVideoPlayerDlg() const { return _maximizedVideoPlayerDlg; }
	void set_maximizedVideoPlayerDlg(int m) { if (m == _maximizedVideoPlayerDlg) return; _maximizedVideoPlayerDlg = m; save(); }
	int get_show_video_same_time_route_count() const { return show_video_same_time_route_count_; }
	void set_show_video_same_time_route_count(int cnt) { if (show_video_same_time_route_count_ == cnt)return; show_video_same_time_route_count_ = cnt; save(); }

	// video
	int get_back_end_record_minutes() const { return _back_end_record_minutes; }
	void set_back_end_record_minutes(int minutes) { if (minutes == _back_end_record_minutes) return; _back_end_record_minutes = minutes; save(); }
	int get_default_video_level() const { return default_video_level_; }
	void set_default_video_level(int level) { if (level == default_video_level_) return; default_video_level_ = level; save(); }
	int get_auto_play_rec_if_available() const { return auto_play_rec_if_available_; }
	void set_auto_play_rec_if_available(int b) { if (b == auto_play_rec_if_available_) return; auto_play_rec_if_available_ = b; save(); }

	std::string get_ezviz_app_key() const { return ezviz_app_key_; }
	std::string get_ezviz_private_cloud_ip() const { return ezviz_private_cloud_ip_; }
	unsigned int get_ezviz_private_cloud_port() const { return ezviz_private_cloud_port_; }

	~CConfigHelper();
private:
	std::wstring _cfg_file = L"";
	std::wstring lang_cfg_ = L"";
	ApplicationLanguage _lang = AL_CHINESE;
	ApplicationLanguage cur_lang_ = AL_CHINESE;

	// ui
	// video player dlg
	CRect _rectVideoPlayerDlg = { 0, 0, 0, 0 };
	int _maximizedVideoPlayerDlg = 0;
	int show_video_same_time_route_count_ = 1;

	// video
	int _back_end_record_minutes = 10;
	int default_video_level_ = 0;
	int auto_play_rec_if_available_ = 0;

	std::string ezviz_app_key_ = "";
	std::string ezviz_private_cloud_ip_ = "";
	unsigned int ezviz_private_cloud_port_ = 12346;

protected:
	void init();
	bool load();
	bool save();

protected:
	CConfigHelper();
};
};

