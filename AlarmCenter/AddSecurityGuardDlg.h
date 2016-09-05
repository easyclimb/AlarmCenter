#pragma once
#include "afxwin.h"


// CAddSecurityGuardDlg dialog

class CAddSecurityGuardDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAddSecurityGuardDlg)

public:
	CAddSecurityGuardDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAddSecurityGuardDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ADD_GUARD };
#endif

public:
	int guard_id_ = 0;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_name;
	CEdit m_phone;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
};
