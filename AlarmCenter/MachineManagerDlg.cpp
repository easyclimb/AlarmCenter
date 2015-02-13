// MachineManagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "MachineManagerDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "GroupInfo.h"
#include "AlarmMachineManager.h"
#include <vector>
#include "InputGroupNameDlg.h"


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
	std::list<TreeItemData*>::iterator iter = m_treeItamDataList.begin();
	while (iter != m_treeItamDataList.end()) {
		TreeItemData* tid = *iter++;
		delete tid;
	}
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
		txt.Format(L"%s", rootName/*, rootGroup->get_machine_count()*/);
		HTREEITEM hRoot = m_tree.GetRootItem();
		HTREEITEM hRootGroup = m_tree.InsertItem(txt, hRoot);
		TreeItemData* tid = new TreeItemData(true, rootGroup);
		m_treeItamDataList.push_back(tid);
		m_tree.SetItemData(hRootGroup, (DWORD_PTR)tid);

		TraverseGroup(hRootGroup, rootGroup);

		m_curselTreeItemGroup = hRootGroup;
		
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
		txt.Format(L"%s", child_group->get_name()/*, child_group->get_machine_count()*/);
		HTREEITEM hChildItem = m_tree.InsertItem(txt, hItemGroup);
		TreeItemData* tid = new TreeItemData(true, child_group);
		m_treeItamDataList.push_back(tid);
		m_tree.SetItemData(hChildItem, (DWORD_PTR)tid);
		TraverseGroup(hChildItem, child_group);
	}

	CAlarmMachineList machineList;
	group->GetChildMachines(machineList);
	std::list<CAlarmMachine*>::iterator machine_iter = machineList.begin();
	while (machine_iter != machineList.end()) {
		CAlarmMachine* machine = *machine_iter++;
		txt.Format(L"%s(%04d)", machine->get_alias(), machine->get_ademco_id());
		HTREEITEM hChildItem = m_tree.InsertItem(txt, hItemGroup);
		TreeItemData* tid = new TreeItemData(false, machine);
		m_treeItamDataList.push_back(tid);
		m_tree.SetItemData(hChildItem, (DWORD_PTR)tid);
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
	*pResult = 0;
	HTREEITEM hItem = m_tree.GetSelectedItem();
	DWORD data = m_tree.GetItemData(hItem);
	TreeItemData* tid = reinterpret_cast<TreeItemData*>(data);
	if (!tid)
		return;

	if (tid->_bGroup) {  // group item
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

		CAlarmMachine* machine = reinterpret_cast<CAlarmMachine*>(tid->_udata);
		if (!machine)
			return;

		EditingMachine();

		CString id;
		id.Format(L"%04d", machine->get_ademco_id());
		m_id.SetWindowTextW(id);

		int ndx = machine->get_banned();
		m_banned.SetCurSel(ndx);

		m_acct.SetWindowTextW(machine->GetDeviceIDW());
		m_name.SetWindowTextW(machine->get_alias());
		m_contact.SetWindowTextW(machine->get_contact());
		m_addr.SetWindowTextW(machine->get_address());
		m_phone.SetWindowTextW(machine->get_phone());
		m_phone_bk.SetWindowTextW(machine->get_phone_bk());
			
		m_group.ResetContent();
		int theNdx = -1;
		CString rootName;
		rootName.LoadStringW(IDS_STRING_GROUP_ROOT);
		CGroupInfo* rootGroup = CGroupManager::GetInstance()->GetRootGroupInfo();
		m_group.InsertString(0, rootName);
		m_group.SetItemData(0, (DWORD)rootGroup);
		if (machine->get_group_id() == rootGroup->get_id()) {
			theNdx = 0;
		}

		CCGroupInfoList list;
		rootGroup->GetDescendantGroups(list);
		CGroupInfoListIter iter = list.begin();
		while (iter != list.end()) {
			CGroupInfo* group = *iter++;
			int ndx = m_group.AddString(group->get_name());
			m_group.SetItemData(ndx, (DWORD)group);
			if (machine->get_group_id() == group->get_id()) {
				theNdx = ndx;
			}
		}
		m_group.SetCurSel(theNdx);
	}
}


void CMachineManagerDlg::OnNMRClickTree1(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	*pResult = 0;
	CPoint pt;
	GetCursorPos(&pt);
	m_tree.ScreenToClient(&pt);
	UINT flags;
	HTREEITEM hItem = m_tree.HitTest(pt, &flags);

	if (hItem && (TVHT_ONITEM & flags)) {  
		DWORD data = m_tree.GetItemData(hItem);
		TreeItemData* tid = reinterpret_cast<TreeItemData*>(data);
		if (!tid || !tid->_bGroup) // make sure it is a group item.
			return;

		m_tree.SelectItem(hItem);
		EditingMachine(FALSE);

		CGroupInfo* group = reinterpret_cast<CGroupInfo*>(tid->_udata);
		if (!group)
			return;

		CMenu menu, *pMenu, subMenu;
		menu.LoadMenuW(IDR_MENU2);
		pMenu = menu.GetSubMenu(0);
		std::vector<CGroupInfo*> vMoveto;

		if (group->IsRootItem()) { 
			pMenu->EnableMenuItem(1, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
			pMenu->EnableMenuItem(2, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		} else {
			// 添加子菜单项
			int nItem = 0;
			subMenu.CreatePopupMenu();
			CGroupInfo* rootGroup = CGroupManager::GetInstance()->GetRootGroupInfo();
			if (group->get_parent_group() != rootGroup) { // 不能移动至父亲分组
				CString rootName;
				rootName.LoadStringW(IDS_STRING_GROUP_ROOT);
				subMenu.AppendMenuW(MF_STRING, nItem++, rootName);
				vMoveto.push_back(rootGroup);
			}

			// 生成 "移动至-->"子菜单的菜单项
			CCGroupInfoList list;
			rootGroup->GetDescendantGroups(list);
			CGroupInfoListIter iter = list.begin();
			while (iter != list.end()) {
				CGroupInfo* child_group = *iter++;
				if (child_group != group // 不能移动至同一个分组
					&& !group->IsDescendantGroup(child_group) // 不能移动至自己的后代分组
					&& (child_group != group->get_parent_group())) { // 不能移动至父亲分组，你丫本来就在那
					subMenu.AppendMenuW(MF_STRING, nItem++, child_group->get_name());
					vMoveto.push_back(child_group);
				}
			}

			CString sMoveTo;
			sMoveTo.LoadStringW(IDS_STRING_MOVE_TO);
			pMenu->InsertMenuW(3, MF_POPUP | MF_BYPOSITION,
							   (UINT)subMenu.GetSafeHmenu(), sMoveTo);
		}

		GetCursorPos(&pt);
		DWORD ret = pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
										pt.x, pt.y, this);
		LOG(L"%d\n", ret);

		if (0 <= ret && ret < vMoveto.size()) { // move to
			LOG(L"move %d %s to %d %s\n", group->get_id(), group->get_name(),
				vMoveto[ret]->get_id(), vMoveto[ret]->get_name());
		} else if (ret == ID_GROUP_ADD) { // add sub group
			LOG(L"add\n");
			CInputGroupNameDlg dlg;
			if (IDOK == dlg.DoModal()) {
				CGroupInfo* child_group = group->ExecuteAddChildGroup(dlg.m_value);
				if (!child_group)
					return;
				HTREEITEM  hItemNewGroup = m_tree.InsertItem(child_group->get_name(), hItem);
				TreeItemData* tid = new TreeItemData(true, child_group);
				m_treeItamDataList.push_back(tid);
				m_tree.SetItemData(hItemNewGroup, (DWORD_PTR)tid);
				m_tree.Expand(hItem, TVE_EXPAND);
			}
		} else if (ret == ID_GROUP_DEL) { // delete group
			LOG(L"delete\n");
		} else if (ret == ID_GROUP_RENAME) { // rename
			LOG(L"rename\n");
		}
	}
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


