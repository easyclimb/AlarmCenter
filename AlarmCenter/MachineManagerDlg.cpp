// MachineManagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "MachineManagerDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "GroupInfo.h"
#include "AlarmMachineManager.h"

using namespace core;

static const int COMBO_NDX_NO = 0;
static const int COMBO_NDX_YES = 1;

// CMachineManagerDlg dialog

IMPLEMENT_DYNAMIC(CMachineManagerDlg, CDialogEx)

CMachineManagerDlg::CMachineManagerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMachineManagerDlg::IDD, pParent)
	, m_curselTreeItemGroup(NULL)
	, m_curselTreeItemMachine(NULL)
{

}

CMachineManagerDlg::~CMachineManagerDlg()
{
}

void CMachineManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE1, m_tree);
	DDX_Control(pDX, IDC_EDIT1, m_id);
	DDX_Control(pDX, IDC_COMBO1, m_banned);
	DDX_Control(pDX, IDC_EDIT2, m_acct);
	DDX_Control(pDX, IDC_EDIT4, m_name);
	DDX_Control(pDX, IDC_EDIT5, m_contact);
	DDX_Control(pDX, IDC_EDIT6, m_addr);
	DDX_Control(pDX, IDC_EDIT7, m_phone);
	DDX_Control(pDX, IDC_EDIT8, m_phone_bk);
	DDX_Control(pDX, IDC_COMBO2, m_group);
	DDX_Control(pDX, IDC_BUTTON_DELETE_MACHINE, m_btnDelMachine);
	DDX_Control(pDX, IDC_BUTTON_CONFIRM_CHANGE, m_btnConfrimChange);
}


BEGIN_MESSAGE_MAP(CMachineManagerDlg, CDialogEx)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, &CMachineManagerDlg::OnTvnSelchangedTree1)
	ON_NOTIFY(NM_RCLICK, IDC_TREE1, &CMachineManagerDlg::OnNMRClickTree1)
	ON_BN_CLICKED(IDC_BUTTON_CONFIRM_CHANGE, &CMachineManagerDlg::OnBnClickedButtonConfirmChange)
	ON_BN_CLICKED(IDC_BUTTON_DELETE_MACHINE, &CMachineManagerDlg::OnBnClickedButtonDeleteMachine)
	ON_BN_CLICKED(IDC_BUTTON_CREATE_MACHINE, &CMachineManagerDlg::OnBnClickedButtonCreateMachine)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CMachineManagerDlg message handlers

void CMachineManagerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	CAlarmMachineManager::GetInstance()->LeaveEditMode();
}


BOOL CMachineManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CAlarmMachineManager::GetInstance()->EnterEditMode();

	CString yes, no;
	yes.LoadStringW(IDS_STRING_YES);
	no.LoadStringW(IDS_STRING_NO);
	int combo_ndx = -1;
	combo_ndx = m_banned.InsertString(COMBO_NDX_NO, no);
	ASSERT(combo_ndx == COMBO_NDX_NO);
	combo_ndx = m_banned.InsertString(COMBO_NDX_YES, yes);
	ASSERT(combo_ndx == COMBO_NDX_YES);

	CString rootName;
	rootName.LoadStringW(IDS_STRING_GROUP_ROOT);

	CGroupManager* mgr = CGroupManager::GetInstance();
	CGroupInfo* rootGroup = mgr->GetRootGroupInfo();
	if (rootGroup) {
		CString txt;
		txt.Format(L"%s[%d]", rootName, rootGroup->get_machine_count());
		HTREEITEM hRoot = m_tree.GetRootItem();
		HTREEITEM hRootGroup = m_tree.InsertItem(txt, hRoot);
		m_tree.SetItemData(hRootGroup, (DWORD_PTR)rootGroup);

		TraverseGroup(hRootGroup, rootGroup);

		m_curselTreeItemGroup = hRootGroup;
		
		CAlarmMachineList machineList;
		rootGroup->GetChildMachines(machineList);
		std::list<CAlarmMachine*>::iterator machine_iter = machineList.begin();
		while (machine_iter != machineList.end()) {
			CAlarmMachine* machine = *machine_iter++;
			txt.Format(L"%04d(%s)", machine->get_ademco_id(), machine->get_alias());
			HTREEITEM hChildItem = m_tree.InsertItem(txt, hRootGroup);
			m_tree.SetItemData(hChildItem, (DWORD_PTR)machine);
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CMachineManagerDlg::TraverseGroup(HTREEITEM hItemGroup, core::CGroupInfo* group)
{
	CString txt;
	CCGroupInfoList groupList;
	group->GetChildGroups(groupList);

	std::list<CGroupInfo*>::iterator group_iter = groupList.begin();
	while (group_iter != groupList.end()) {
		CGroupInfo* child_group = *group_iter++;
		txt.Format(L"%s[%d]", child_group->get_name(), child_group->get_machine_count());
		HTREEITEM hChildItem = m_tree.InsertItem(txt, hItemGroup);
		m_tree.SetItemData(hChildItem, (DWORD_PTR)child_group);
		TraverseGroup(hChildItem, child_group);
	}

	CAlarmMachineList machineList;
	group->GetChildMachines(machineList);
	std::list<CAlarmMachine*>::iterator machine_iter = machineList.begin();
	while (machine_iter != machineList.end()) {
		CAlarmMachine* machine = *machine_iter++;
		txt.Format(L"%s(%04d)", machine->get_alias(), machine->get_ademco_id());
		HTREEITEM hChildItem = m_tree.InsertItem(txt, hItemGroup);
		m_tree.SetItemData(hChildItem, (DWORD_PTR)machine);
	}
}


void CMachineManagerDlg::EditingMachine(BOOL yes)
{
	m_btnDelMachine.EnableWindow(yes);
	m_btnConfrimChange.EnableWindow(yes);

	m_banned.EnableWindow(yes);
	m_acct.EnableWindow(yes);
	m_name.EnableWindow(yes);
	m_contact.EnableWindow(yes);
	m_addr.EnableWindow(yes);
	m_phone.EnableWindow(yes);
	m_phone_bk.EnableWindow(yes);
	m_group.EnableWindow(yes);

	if (!yes)
		m_group.ResetContent();
}


void CMachineManagerDlg::OnTvnSelchangedTree1(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	HTREEITEM hItem = m_tree.GetSelectedItem();

	if (m_tree.ItemHasChildren(hItem)) {  // group item
		m_curselTreeItemMachine = NULL;
		if (m_curselTreeItemGroup == hItem) { return; } 
		else { m_curselTreeItemGroup = hItem; }

		/*DWORD data = m_tree.GetItemData(hItem);
		CGroupInfo* group = reinterpret_cast<CGroupInfo*>(data);
		if (group) {
		}*/

		EditingMachine(FALSE);

	} else {	// machine item
		m_curselTreeItemGroup = NULL;
		if (m_curselTreeItemMachine == hItem) { return; } 
		else { m_curselTreeItemMachine = hItem; }

		DWORD data = m_tree.GetItemData(hItem);
		CAlarmMachine* machine = reinterpret_cast<CAlarmMachine*>(data);
		if (data) {
			EditingMachine();

			CString id;
			id.Format(L"%04d", machine->get_ademco_id());
			m_id.SetWindowTextW(id);

			int ndx = machine->get_banned();
			m_banned.SetCurSel(ndx);

			m_name.SetWindowTextW(machine->get_alias());
			m_contact.SetWindowTextW(machine->get_contact());
			m_addr.SetWindowTextW(machine->get_address());
			m_phone.SetWindowTextW(machine->get_phone());
			m_phone_bk.SetWindowTextW(machine->get_phone_bk());
			m_group.ResetContent();

			CCGroupInfoList list;
			CGroupManager::GetInstance()->GetRootGroupInfo()->GetDescendantGroups(list);
			CGroupInfoListIter iter = list.begin();
			while (iter != list.end()) {
				CGroupInfo* group = *iter++;
				int ndx = m_group.AddString(group->get_name());
				m_group.SetItemData(ndx, (DWORD)group);
			}

		}
	}

	*pResult = 0;
}


void CMachineManagerDlg::OnNMRClickTree1(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{

	*pResult = 0;
}


void CMachineManagerDlg::OnBnClickedButtonConfirmChange()
{

}


void CMachineManagerDlg::OnBnClickedButtonDeleteMachine()
{

}


void CMachineManagerDlg::OnBnClickedButtonCreateMachine()
{

}


