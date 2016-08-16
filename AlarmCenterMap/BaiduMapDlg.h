#pragma once

#ifdef _WIN32_WCE
#error "CDHtmlDialog is not supported for Windows CE."
#endif 

// CBaiduMapDlg dialog
#include "../AlarmCenter/baidu.h"
#include "afxwin.h"

#define SET_WINDOW_TEXT(IDC, IDS) {CWnd*p = GetDlgItem(IDC); if(p){p->SetWindowText(TR(IDS));}}
#define SET_CLASSIC_WINDOW_TEXT(IDC) SET_WINDOW_TEXT(IDC, "IDS_STRING_"#IDC)


class CBaiduMapDlg : public CDialogEx
{
	//class CurUserChangedObserver;
	//std::shared_ptr<CurUserChangedObserver> m_cur_user_changed_observer;
	DECLARE_DYNCREATE(CBaiduMapDlg)

public:
	CBaiduMapDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CBaiduMapDlg();
// Overrides
// Dialog Data
	enum { IDD = IDD_DIALOG_BAIDU_MAP
	};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	CString m_title, m_info;
	DECLARE_MESSAGE_MAP()
	std::wstring m_url;
	std::string m_html;
	CRect _initRc;
	bool GenerateHtml(std::wstring& url, const web::BaiduCoordinate& coor, int zoomLevel, const CString& title, const CString& info);
public:
	DECLARE_GETTER_SETTER(CRect, _initRc);
	afx_msg void OnBnClickedOk();
	web::BaiduCoordinate m_coor;
	int m_zoomLevel;
	CWnd* m_pRealParent;
	virtual void OnCancel();
	afx_msg void OnBnClickedButtonReset();
	bool ShowCoordinate(const web::BaiduCoordinate& coor, int zoomLevel, const CString& title, const CString& info, bool bUseExternalWebBrowser = false);
	bool ShowDrivingRoute(const web::BaiduCoordinate& coor_start,
						  const web::BaiduCoordinate& coor_end,
						  const std::wstring& name_start,
						  const std::wstring& name_end);
	CButton m_btnUsePt;
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
