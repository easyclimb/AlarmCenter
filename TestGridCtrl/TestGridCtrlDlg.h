
// TestGridCtrlDlg.h : header file
//

#pragma once
#include "C:\dev\AlarmCenter\AlarmCenter\GridCtrl_src\GridCtrl.h"


// CTestGridCtrlDlg dialog
class CTestGridCtrlDlg : public CDialogEx
{
// Construction
public:
	CTestGridCtrlDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TESTGRIDCTRL_DIALOG };
#endif

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
	gui::control::grid_ctrl::CGridCtrl m_grid;
};
