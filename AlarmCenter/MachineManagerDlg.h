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
		TreeItemData() : _bGroup(false), _udata(nullptr) {}
		TreeItemData(bool bGroup, void* udata) :_bGroup(bGroup), _udata(udata) {}
	}TreeItemData;

public:
	CMachineManagerDlg(CWnd* pParent = nullptr);   // standard constructor
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
	HTREEITEM GetTreeGroupItemByGroupInfo(core::CGroupInfo* group);
	HTREEITEM GetTreeGroupItemByGroupInfoHelper(HTREEITEM hItem, core::CGroupInfo* group);
	void DeleteGroupItem(HTREEITEM hItem);
public:
	virtual BOOL OnInitDialog();
	CTreeCtrl m_tree;
	
	CEdit m_id;
	CComboBox m_banned;
	//CEdit m_acct;
	CEdit m_name;
	CEdit m_contact;
	CEdit m_addr;
	CEdit m_phone;
	CEdit m_phone_bk;
	CComboBox m_group;
	CButton m_btnDelMachine;
	CComboBox m_type;

	afx_msg void OnCbnSelchangeComboBanned();
	afx_msg void OnCbnSelchangeComboType();
	afx_msg void OnEnKillfocusEditName();
	afx_msg void OnEnKillfocusEditContact();
	afx_msg void OnEnKillfocusEditAddress();
	afx_msg void OnEnKillfocusEditPhone();
	afx_msg void OnEnKillfocusEditPhoneBk();
	afx_msg void OnCbnSelchangeComboGroup();
	CEdit m_expire_time;
	afx_msg void OnBnClickedButtonExtend();
	CEdit m_x;
	CEdit m_y;
	afx_msg void OnBnClickedButtonPickCoor();
	CButton m_pick_coor;
	CButton m_extend_expire;
	CButton m_chk_report_status;
	CButton m_chk_report_status_bk;
	CButton m_chk_report_exception;
	CButton m_chk_report_exception_bk;
	CButton m_chk_report_alarm;
	CButton m_chk_report_alarm_bk;
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedCheck2();
	afx_msg void OnBnClickedCheck3();
	afx_msg void OnBnClickedCheck4();
	afx_msg void OnBnClickedCheck5();
	afx_msg void OnBnClickedCheck6();
	CStatic m_staticHexAdemcoId;
};
