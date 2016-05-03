#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include <list>

namespace core { class CGroupInfo;  };
#include "core.h"

// CMachineManagerDlg dialog

class CMachineManagerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMachineManagerDlg)
	typedef struct TreeItemData
	{
		bool _bGroup;
		core::CAlarmMachinePtr _machine;
		core::CGroupInfoPtr _group;
		TreeItemData() : _bGroup(false), _machine(nullptr), _group(nullptr) {}
		TreeItemData(const core::CAlarmMachinePtr& machine) :_bGroup(false), _machine(machine), _group(nullptr) {}
		TreeItemData(core::CGroupInfoPtr group) : _bGroup(true), _machine(nullptr), _group(group){}
	}TreeItemData;
	typedef std::shared_ptr<TreeItemData> TreeItemDataPtr;
	typedef std::map<HTREEITEM, TreeItemDataPtr> TidMap;

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
	TidMap m_tidMap;
#ifdef _DEBUG
	HTREEITEM m_prevHotItem = nullptr;
#endif
protected:
	void TraverseGroup(HTREEITEM hItemGroup, core::CGroupInfoPtr group);
	void EditingMachine(BOOL yes = TRUE);
	void ClearTree();
	void InitTree();
	void ClearChildItems(HTREEITEM hItemParent);
	core::CAlarmMachinePtr GetCurEditingMachine();
	HTREEITEM GetTreeGroupItemByGroupInfo(core::CGroupInfoPtr group);
	HTREEITEM GetTreeGroupItemByGroupInfoHelper(HTREEITEM hItem, core::CGroupInfoPtr group);
	void DeleteGroupItem(HTREEITEM hItem);
public:
	virtual BOOL OnInitDialog();
	CTreeCtrl m_tree;
	CEdit m_id;
	CComboBox m_banned;
	CEdit m_name;
	CEdit m_contact;
	CEdit m_addr;
	CEdit m_phone;
	CEdit m_phone_bk;
	CComboBox m_group;
	CButton m_btnDelMachine;
	CComboBox m_type;
	CEdit m_expire_time; 
	CEdit m_x;
	CEdit m_y;
	CButton m_pick_coor;
	CButton m_extend_expire;
	CButton m_chk_report_status;
	CButton m_chk_report_status_bk;
	CButton m_chk_report_exception;
	CButton m_chk_report_exception_bk;
	CButton m_chk_report_alarm;
	CButton m_chk_report_alarm_bk;
	CStatic m_staticHexAdemcoId;

	afx_msg void OnCbnSelchangeComboBanned();
	afx_msg void OnCbnSelchangeComboType();
	afx_msg void OnEnKillfocusEditName();
	afx_msg void OnEnKillfocusEditContact();
	afx_msg void OnEnKillfocusEditAddress();
	afx_msg void OnEnKillfocusEditPhone();
	afx_msg void OnEnKillfocusEditPhoneBk();
	afx_msg void OnCbnSelchangeComboGroup();
	afx_msg void OnBnClickedButtonExtend();
	afx_msg void OnBnClickedButtonPickCoor();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedCheck2();
	afx_msg void OnBnClickedCheck3();
	afx_msg void OnBnClickedCheck4();
	afx_msg void OnBnClickedCheck5();
	afx_msg void OnBnClickedCheck6();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
