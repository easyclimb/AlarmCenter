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

	int get_baidumap_auto_refresh() const { return _baidumap_auto_refresh; }
	void set_baidumap_auto_refresh(int b) { if (b == _baidumap_auto_refresh) return; _baidumap_auto_refresh = b; save(); }

	// baidu map dlg
	CRect get_rectBaiduMapDlg() const { return rectBaiduMapDlg_; }
	void set_rectBaiduMapDlg(const CRect& rc) { if (rc == rectBaiduMapDlg_) return; rectBaiduMapDlg_ = rc; save(); }
	int get_maximizedBaiduMapDlg() const { return maximizedBaiduMapDlg_; }
	void set_maximizedBaiduMapDlg(int m) { if (m == maximizedBaiduMapDlg_) return; maximizedBaiduMapDlg_ = m; save(); }

	~CConfigHelper();
private:
	std::wstring _cfg_file = L"";
	std::wstring lang_cfg_ = L"";
	ApplicationLanguage _lang = AL_CHINESE;
	ApplicationLanguage cur_lang_ = AL_CHINESE;
	int _baidumap_auto_refresh = 1;

	// baidu map dlg
	CRect rectBaiduMapDlg_ = { 0,0,0,0 };
	int maximizedBaiduMapDlg_ = 0;

protected:
	void init();
	bool load();
	bool save();

protected:
	CConfigHelper();
};
};

