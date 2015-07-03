#pragma once
#include "afxwin.h"


// CChangePswDlg dialog

class CChangePswDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CChangePswDlg)

public:
	CChangePswDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CChangePswDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_CHG_PSW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_title;
	CEdit m_psw_o;
	CEdit m_psw_n;
	CEdit m_psw_r;
	CString psw_o, psw_n, psw_r;
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
};
