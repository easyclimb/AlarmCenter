#pragma once
#include "afxwin.h"


// CInputPswDlg dialog

class CInputPswDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CInputPswDlg)

public:
	CInputPswDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CInputPswDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_INPUT_PSW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_psw;
	CString m_strPsw;
	afx_msg void OnBnClickedOk();
};
