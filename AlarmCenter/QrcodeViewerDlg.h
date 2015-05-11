#pragma once
#include "StaticBmp.h"
#include "afxwin.h"
#include <iostream>
//using std::string;
//namespace gui { namespace control { class CStaticBmp; }; };
// CQrcodeViewerDlg dialog
class CBaiduMapDlg;

class CQrcodeViewerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CQrcodeViewerDlg)

public:
	CQrcodeViewerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CQrcodeViewerDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_CSR_ACCT };
	std::string m_acct;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	CString m_md5_path;
	CString m_bmp_path;
	void GenerateAcct(char* buff, int buff_size);
	void InitAcct();
	void InitLocation();
	bool GenerateHtml(std::wstring& url, double x, double y);
	DECLARE_MESSAGE_MAP()
public:
	gui::control::CStaticBmp m_staticEx;
//	CEdit m_csr_acct;
	virtual BOOL OnInitDialog();
	CString m_acct_text;
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	CStatic m_staticBaiduMap;
	CEdit m_addr;
	CEdit m_x;
	CEdit m_y;
private:
	CBaiduMapDlg* m_map1;
public:
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButtonLocateAuto();
	afx_msg void OnBnClickedButtonLocateToAddr();
	afx_msg void OnBnClickedButtonLocateToCoor();
protected:
	afx_msg LRESULT OnChosenBaiduPt(WPARAM wParam, LPARAM lParam);
};
