#pragma once
#include "afxwin.h"


// CLoginDlg dialog

class CLoginDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CLoginDlg)

public:
	CLoginDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLoginDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_LOGIN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
//	CComboBox m_user_id;
//	CComboBox m_user_name;
	CEdit m_user_passwd;
	CButton m_chkLogByID;
	afx_msg void OnBnClickedCheckLogbyUserid();
	afx_msg void OnBnClickedOk();
	CEdit m_user_id;
	CEdit m_user_name;
	CStatic m_note_id;
	CStatic m_note_name;
	CStatic m_note_passwd;
	afx_msg void OnEnChangeEditUserid();
	afx_msg void OnEnChangeEditUserName();
	virtual BOOL OnInitDialog();
};
