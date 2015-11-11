#pragma once

#ifdef _WIN32_WCE
#error "CDHtmlDialog is not supported for Windows CE."
#endif 

// CBaiduMapDlg dialog
#include "baidu.h"
#include "afxwin.h"

//#define USE_DHTML
#ifndef USE_DHTML
//#include "CWebClient.h"  
#endif

class CBaiduMapDlg
#ifdef USE_DHTML
	: public CDHtmlDialog
#else
	: public CDialogEx
#endif
{
	DECLARE_DYNCREATE(CBaiduMapDlg)

public:
	CBaiduMapDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CBaiduMapDlg();
// Overrides
#ifdef USE_DHTML
	HRESULT OnButtonOK(IHTMLElement *pElement);
	HRESULT OnButtonCancel(IHTMLElement *pElement);
#endif
// Dialog Data
	enum { IDD = IDD_DIALOG_BAIDU_MAP
#ifdef USE_DHTML
		, IDH = IDR_HTML_BAIDUMAPDLG
#endif
	};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	bool VoidCall(const wchar_t* funcName);
	CString m_title;
	DECLARE_MESSAGE_MAP()
#ifdef USE_DHTML
	DECLARE_DHTML_EVENT_MAP()
#else
	//CefRefPtr<CefClient> m_client;
	//CefRefPtr<CWebClient>  m_cWebClient;
#endif
	std::wstring m_url;
	
	bool GenerateHtml(std::wstring& url, const web::BaiduCoordinate& coor, int zoomLevel, const CString& title);
public:
	afx_msg void OnBnClickedOk();
	web::BaiduCoordinate m_coor;
	int m_zoomLevel;
	CWnd* m_pRealParent;
	virtual void OnCancel();
	afx_msg void OnBnClickedButtonReset();
	bool ShowCoordinate(const web::BaiduCoordinate& coor, int zoomLevel, const CString& title, bool bUseExternalWebBrowser = false);
	bool ShowDrivingRoute(const web::BaiduCoordinate& coor_start,
						  const web::BaiduCoordinate& coor_end,
						  const std::wstring& name_start,
						  const std::wstring& name_end);
	CButton m_btnUsePt;
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
