#pragma once

#ifdef _WIN32_WCE
#error "CDHtmlDialog is not supported for Windows CE."
#endif 

// CBaiduMapDlg dialog
#include "baidu.h"


class CBaiduMapDlg : public CDHtmlDialog
{
	DECLARE_DYNCREATE(CBaiduMapDlg)

public:
	CBaiduMapDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CBaiduMapDlg();
// Overrides
	HRESULT OnButtonOK(IHTMLElement *pElement);
	HRESULT OnButtonCancel(IHTMLElement *pElement);

// Dialog Data
	enum { IDD = IDD_DIALOG_BAIDU_MAP, IDH = IDR_HTML_BAIDUMAPDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	bool VoidCall(const wchar_t* funcName);
	CString m_title;
	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
public:
	afx_msg void OnBnClickedOk();
	web::BaiduCoordinate m_coor;
	CWnd* m_pRealParent;
	virtual void OnCancel();
	afx_msg void OnBnClickedButtonReset();
	bool GenerateHtml(std::wstring& url, const web::BaiduCoordinate& coor, const CString& title);
};
