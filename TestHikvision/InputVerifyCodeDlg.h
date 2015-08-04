#pragma once
#include "afxwin.h"


// CInputVerifyCodeDlg dialog

class CInputVerifyCodeDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CInputVerifyCodeDlg)

public:
	CInputVerifyCodeDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CInputVerifyCodeDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_code;
	CString m_result;
	afx_msg void OnBnClickedOk();
};
