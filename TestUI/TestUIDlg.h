
// TestUIDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "BtnST.h"
//namespace gui {
//	namespace control { class CButtonST; };
//};

// CTestUIDlg dialog
class CTestUIDlg : public CDialogEx
{
// Construction
public:
	CTestUIDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_TESTUI_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	//gui::control::CButtonST* m_btn;
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	gui::control::CButtonST m_btn;
};
