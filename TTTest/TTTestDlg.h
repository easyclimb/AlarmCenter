// TTTestDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CTTTestDlg dialog
class CTTTestDlg : public CDialog
{
// Construction
public:
	CTTTestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_TTTEST_DIALOG };
	CString	m_strJunk;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	CEdit m_ctrlEdit;
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnEnChangeEdit1();
};
