#pragma once
#include "afxwin.h"


// CInputGroupNameDlg dialog

class CInputGroupNameDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CInputGroupNameDlg)

public:
	CInputGroupNameDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CInputGroupNameDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_GROUP_NAME };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
//	CEdit m_txt;
	CString m_value;
	afx_msg void OnBnClickedOk();
};
