#pragma once
#include "afxcmn.h"
#include "afxwin.h"

namespace core { class CGroupInfo; };

// CMachineManagerDlg dialog

class CMachineManagerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMachineManagerDlg)

public:
	CMachineManagerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMachineManagerDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_MACHINE_MGR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	HTREEITEM m_curselTreeItemGroup;
	HTREEITEM m_curselTreeItemMachine;
protected:
	void TraverseGroup(HTREEITEM hItemGroup, core::CGroupInfo* group);

public:
	virtual BOOL OnInitDialog();
	CTreeCtrl m_tree;
	afx_msg void OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult);
	CEdit m_id;
	CComboBox m_banned;
	CEdit m_acct;
	CEdit m_name;
	CEdit m_contact;
	CEdit m_addr;
	CEdit m_phone;
	CEdit m_phone_bk;
	CComboBox m_group;
};
