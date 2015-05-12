#pragma once

#ifdef _WIN32_WCE
#error "CDHtmlDialog is not supported for Windows CE."
#endif 

// CBaiduMapDlg dialog

typedef struct BaiduPoint
{
	double x;
	double y;
	BaiduPoint() : x(.0), y(.0) {}
	BaiduPoint(double x, double y) : x(x), y(y) {}
	BaiduPoint& operator=(const BaiduPoint& rhs) { x = rhs.x; y = rhs.y; return *this; }
	bool operator==(const BaiduPoint& rhs) { return x == rhs.x && y == rhs.y; }
}BaiduPoint;

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

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
public:
	afx_msg void OnBnClickedOk();
	BaiduPoint m_pt;
	CWnd* m_pRealParent;
	virtual void OnCancel();
	afx_msg void OnBnClickedButtonReset();
};
