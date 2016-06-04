#pragma once
#include "afxwin.h"


// CAddVideoUserJovisionDlg dialog

class CAddVideoUserJovisionDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAddVideoUserJovisionDlg)

public:
	CAddVideoUserJovisionDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAddVideoUserJovisionDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ADD_VIDEO_USER_JOVISION };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_name;
	CEdit m_phone;
	CEdit m_default_user_name;
	CEdit m_default_user_passwd;
	afx_msg void OnBnClickedOk();
};
