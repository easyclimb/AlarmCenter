#pragma once

#ifdef _WIN32_WCE
#error "CDHtmlDialog is not supported for Windows CE."
#endif 

// CTestDHtmlDlg dialog

class CTestDHtmlDlg : public CDHtmlDialog
{
	DECLARE_DYNCREATE(CTestDHtmlDlg)

public:
	CTestDHtmlDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTestDHtmlDlg();
// Overrides
	HRESULT OnButtonOK(IHTMLElement *pElement);
	HRESULT OnButtonCancel(IHTMLElement *pElement);

// Dialog Data
	enum { IDD = IDD_DIALOG1, IDH = IDR_HTML_TESTDHTMLDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
public:
	virtual void OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl);
	afx_msg void OnBnClickedOk();
};
