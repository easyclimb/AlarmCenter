#pragma once
#include "afxwin.h"


// CInputContentDlg dialog

class CInputContentDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CInputContentDlg)

public:
	CInputContentDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CInputContentDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_GROUP_NAME };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
//	CEdit m_txt;
	CString m_value;
	CString m_title;
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
};
