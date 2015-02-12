// MachineManagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "MachineManagerDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "GroupInfo.h"
using namespace core;


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
}


BEGIN_MESSAGE_MAP(CMachineManagerDlg, CDialogEx)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, &CMachineManagerDlg::OnTvnSelchangedTree1)
END_MESSAGE_MAP()


// CMachineManagerDlg message handlers


BOOL CMachineManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

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


void CMachineManagerDlg::OnTvnSelchangedTree1(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	HTREEITEM hItem = m_tree.GetSelectedItem();

	if (m_tree.ItemHasChildren(hItem)) {  // group item
		if (m_curselTreeItemGroup == hItem) { return; } 
		else { m_curselTreeItemGroup = hItem; }

		DWORD data = m_tree.GetItemData(hItem);
		CGroupInfo* group = reinterpret_cast<CGroupInfo*>(data);
		if (group) {
		}
	} else {	// machine item
		if (m_curselTreeItemMachine == hItem) { return; } 
		else { m_curselTreeItemMachine = hItem; }

		DWORD data = m_tree.GetItemData(hItem);
		CAlarmMachine* machine = reinterpret_cast<CAlarmMachine*>(data);
		if (data) {

		}
	}

	*pResult = 0;
}
