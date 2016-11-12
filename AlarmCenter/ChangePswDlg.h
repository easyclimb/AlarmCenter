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
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CHANGE_PSW };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	CEdit m_old_psw;
	CEdit m_new_psw;
	CEdit m_new_psw_2;
};
