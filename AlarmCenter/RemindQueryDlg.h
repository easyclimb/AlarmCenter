#pragma once
#include "afxwin.h"


// CRemindQueryDlg dialog

class CRemindQueryDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CRemindQueryDlg)

public:
	CRemindQueryDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CRemindQueryDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_REMIND_QUERY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CButton m_btnOk;
	CString m_strOk;
	int m_counter;
};
