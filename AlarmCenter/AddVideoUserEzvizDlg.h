#pragma once
#include "afxwin.h"


// CAddVideoUserEzvizDlg dialog

class CAddVideoUserEzvizDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAddVideoUserEzvizDlg)

public:
	CAddVideoUserEzvizDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAddVideoUserEzvizDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_ADD_VIDEO_USER_EZVIZ };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CEdit m_name;
	CEdit m_phone;
	CString m_strName;
	CString m_strPhone;
};
