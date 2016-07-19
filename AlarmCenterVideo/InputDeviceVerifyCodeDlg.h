#pragma once
#include "afxwin.h"


// CInputDeviceVerifyCodeDlg dialog

class CInputDeviceVerifyCodeDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CInputDeviceVerifyCodeDlg)

public:
	CInputDeviceVerifyCodeDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CInputDeviceVerifyCodeDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_INPUT_VERIFY_CODE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_result;
	CEdit m_code;
	afx_msg void OnBnClickedOk();
};
