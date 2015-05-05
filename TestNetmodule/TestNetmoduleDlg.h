
// TestNetmoduleDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include <list>

// CTestNetmoduleDlg dialog
class CTestNetmoduleDlg : public CDialogEx
{
// Construction
public:
	CTestNetmoduleDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_TESTNETMODULE_DIALOG };

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
	int m_ademco_id;
	int m_ademco_event;
	int m_zone;
	int m_gg;
	int m_expectEvent;
	CString m_device_id;
	int m_sendGap;
	CEdit m_sendCnt;
	CEdit m_recvCnt;
	CLock m_lock;
	std::list<int> m_eventList;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void OnAdemcoEventResult(int ademco_event);
	afx_msg void OnBnClickedButton1();
};
