
// TestBaiduMapDlg.h : header file
//

#pragma once
#include "explorer1.h"
#include "CWebBrowser2.h"

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
	CExplorer1 m_ie;
	afx_msg void OnBnClickedButton1();
};
