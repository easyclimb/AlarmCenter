#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "MyListCtrl.h"

namespace core { class CUserInfo; };
// CUserManagerDlg dialog

class CUserManagerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CUserManagerDlg)

public:
	CUserManagerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CUserManagerDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_USERMGR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CMyListCtrl m_list;
	afx_msg void OnBnClickedButtonClear();
	afx_msg void OnBnClickedButtonUpdate();
	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedButtonDelete();
	CEdit m_id;
	CEdit m_name;
	CEdit m_phone;
	CComboBox m_priority;
	afx_msg void OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL OnInitDialog();
protected:
	void Insert2List(const core::CUserInfo* user);
	void LoadAllUserInfo();
	const core::CUserInfo* m_curUser;
public:
	CEdit m_passwd;
	afx_msg void OnBnClickedButtonChangePasswd();
	CButton m_btnDelete;
	CButton m_btnUpdate;
	CButton m_btnChangePasswd;
	CButton m_btnAdd;
};
