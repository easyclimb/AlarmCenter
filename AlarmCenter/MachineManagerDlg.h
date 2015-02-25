#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include <list>

namespace core { class CGroupInfo; class CAlarmMachine; };

// CMachineManagerDlg dialog

class CMachineManagerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMachineManagerDlg)
	typedef struct TreeItemData
	{
		bool _bGroup;
		void* _udata;
		TreeItemData() : _bGroup(false), _udata(NULL) {}
		TreeItemData(bool bGroup, void* udata) :_bGroup(bGroup), _udata(udata) {}
	}TreeItemData;

public:
	CMachineManagerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMachineManagerDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_MACHINE_MGR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnNMRClickTree1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonConfirmChange();
	afx_msg void OnBnClickedButtonDeleteMachine();
	afx_msg void OnBnClickedButtonCreateMachine();
	afx_msg void OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()

private:
	HTREEITEM m_curselTreeItemGroup;
	HTREEITEM m_curselTreeItemMachine;
	std::list<TreeItemData*> m_treeItamDataList;
protected:
	void TraverseGroup(HTREEITEM hItemGroup, core::CGroupInfo* group);
	void EditingMachine(BOOL yes = TRUE);
	void ClearTree();
	void ClearChildItems(HTREEITEM hItemParent);
	core::CAlarmMachine* GetCurEditingMachine();
public:
	virtual BOOL OnInitDialog();
	CTreeCtrl m_tree;
	
	CEdit m_id;
	CComboBox m_banned;
	CEdit m_acct;
	CEdit m_name;
	CEdit m_contact;
	CEdit m_addr;
	CEdit m_phone;
	CEdit m_phone_bk;
	CComboBox m_group;
	CButton m_btnDelMachine;
	CButton m_btnConfrimChange;
	CComboBox m_type;

	
	
	afx_msg void OnCbnSelchangeComboBanned();
	afx_msg void OnCbnSelchangeComboType();
};
