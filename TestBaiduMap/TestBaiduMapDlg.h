
// TestBaiduMapDlg.h : header file
//

#pragma once
#include "explorer1.h"
#include "CWebBrowser2.h"
#include "afxwin.h"

// CTestBaiduMapDlg dialog
class CTestBaiduMapDlg : public CDialogEx
{
// Construction
public:
	CTestBaiduMapDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_TESTBAIDUMAP_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	//CExplorer1 m_ie;
	CWebBrowser2 m_ie;
	afx_msg void OnBnClickedButton1();
	CEdit m_url;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_EVENTSINK_MAP()
	void DocumentCompleteExplorer1(LPDISPATCH pDisp, VARIANT* URL);
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
};
