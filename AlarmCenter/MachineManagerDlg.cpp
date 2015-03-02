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
#include "HistoryRecord.h"
#include "AddMachineDlg.h"

using namespace core;

static const int COMBO_NDX_NO = 0;
static const int COMBO_NDX_YES = 1;

static const int COMBO_NDX_MAP = 0;
static const int COMBO_NDX_VIDEO = 1;

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
	DDX_Control(pDX, IDC_COMBO_TYPE, m_type);
}


BEGIN_MESSAGE_MAP(CMachineManagerDlg, CDialogEx)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, &CMachineManagerDlg::OnTvnSelchangedTree1)
	ON_NOTIFY(NM_RCLICK, IDC_TREE1, &CMachineManagerDlg::OnNMRClickTree1)
	ON_BN_CLICKED(IDC_BUTTON_CONFIRM_CHANGE, &CMachineManagerDlg::OnBnClickedButtonConfirmChange)
	ON_BN_CLICKED(IDC_BUTTON_DELETE_MACHINE, &CMachineManagerDlg::OnBnClickedButtonDeleteMachine)
	ON_BN_CLICKED(IDC_BUTTON_CREATE_MACHINE, &CMachineManagerDlg::OnBnClickedButtonCreateMachine)
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_COMBO1, &CMachineManagerDlg::OnCbnSelchangeComboBanned)
	ON_CBN_SELCHANGE(IDC_COMBO_TYPE, &CMachineManagerDlg::OnCbnSelchangeComboType)
	ON_EN_KILLFOCUS(IDC_EDIT4, &CMachineManagerDlg::OnEnKillfocusEditName)
	ON_EN_KILLFOCUS(IDC_EDIT5, &CMachineManagerDlg::OnEnKillfocusEditContact)
	ON_EN_KILLFOCUS(IDC_EDIT6, &CMachineManagerDlg::OnEnKillfocusEditAddress)
	ON_EN_KILLFOCUS(IDC_EDIT7, &CMachineManagerDlg::OnEnKillfocusEditPhone)
	ON_EN_KILLFOCUS(IDC_EDIT8, &CMachineManagerDlg::OnEnKillfocusEditPhoneBk)
	ON_CBN_SELCHANGE(IDC_COMBO2, &CMachineManagerDlg::OnCbnSelchangeComboGroup)
END_MESSAGE_MAP()


// CMachineManagerDlg message handlers

void CMachineManagerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	CAlarmMachineManager::GetInstance()->LeaveEditMode();
	ClearTree();
}


void CMachineManagerDlg::ClearTree()
{
	std::list<TreeItemData*>::iterator iter = m_treeItamDataList.begin();
	while (iter != m_treeItamDataList.end()) {
		TreeItemData* tid = *iter++;
		delete tid;
	}
	m_treeItamDataList.clear();
}


void CMachineManagerDlg::ClearChildItems(HTREEITEM hItemParent)
{
	HTREEITEM hItem = m_tree.GetChildItem(hItemParent);
	while (hItem) {
		DWORD data = m_tree.GetItemData(hItem);
		TreeItemData* tid = reinterpret_cast<TreeItemData*>(data);
		m_treeItamDataList.remove(tid);
		delete tid;
		HTREEITEM hItemOld = hItem;
		hItem = m_tree.GetNextSiblingItem(hItem);
		if (hItem && m_tree.ItemHasChildren(hItem)) {
			ClearChildItems(hItem);
		}
		m_tree.DeleteItem(hItemOld);
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

	CString normal, video;
	normal.LoadStringW(IDS_STRING_TYPE_MAP);
	video.LoadStringW(IDS_STRING_TYPE_VIDEO);
	combo_ndx = m_type.InsertString(COMBO_NDX_MAP, normal);
	ASSERT(combo_ndx == COMBO_NDX_MAP);
	combo_ndx = m_type.InsertString(COMBO_NDX_VIDEO, video);
	ASSERT(combo_ndx == COMBO_NDX_VIDEO);

	

	CGroupManager* mgr = CGroupManager::GetInstance();
	CGroupInfo* rootGroup = mgr->GetRootGroupInfo();
	if (rootGroup) {
		CString txt;
		txt.Format(L"%s", rootGroup->get_name()/*, rootGroup->get_machine_count()*/);
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
	CGroupInfoList groupList;
	group->GetChildGroups(groupList);

	std::list<CGroupInfo*>::iterator group_iter = groupList.begin();
	while (group_iter != groupList.end()) {
		CGroupInfo* child_group = *group_iter++;
		txt.Format(L"%s", child_group->get_name()/*, child_group->get_machine_count()*/);
		HTREEITEM hChildGroupItem = m_tree.InsertItem(txt, hItemGroup);
		TreeItemData* tid = new TreeItemData(true, child_group);
		m_treeItamDataList.push_back(tid);
		m_tree.SetItemData(hChildGroupItem, (DWORD_PTR)tid);
		TraverseGroup(hChildGroupItem, child_group);
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
	//m_acct.EnableWindow(yes);
	m_type.EnableWindow(yes);
	m_name.EnableWindow(yes);
	m_contact.EnableWindow(yes);
	m_addr.EnableWindow(yes);
	m_phone.EnableWindow(yes);
	m_phone_bk.EnableWindow(yes);
	m_group.EnableWindow(yes);

	if (!yes)
		m_group.ResetContent();
	else {
		m_name.SendMessage(WM_KILLFOCUS);
		m_contact.SendMessage(WM_KILLFOCUS);
		m_addr.SendMessage(WM_KILLFOCUS);
		m_phone.SendMessage(WM_KILLFOCUS);
		m_phone_bk.SendMessage(WM_KILLFOCUS);
	}
}


void CMachineManagerDlg::OnTvnSelchangedTree1(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	*pResult = 0;
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (NULL == hItem)
		return;
	DWORD data = m_tree.GetItemData(hItem);
	TreeItemData* tid = reinterpret_cast<TreeItemData*>(data);
	if (!tid)
		return;

	if (tid->_bGroup) {  // group item
		EditingMachine();
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
		else { 
			EditingMachine(); 
			m_curselTreeItemMachine = hItem;
		}

		CAlarmMachine* machine = reinterpret_cast<CAlarmMachine*>(tid->_udata);
		if (!machine)
			return;

		CString id;
		id.Format(L"%04d", machine->get_ademco_id());
		m_id.SetWindowTextW(id);

		int ndx = machine->get_banned();
		m_banned.SetCurSel(ndx);

		int type = machine->get_type();
		m_type.SetCurSel(type);

		m_acct.SetWindowTextW(machine->GetDeviceIDW());
		m_name.SetWindowTextW(machine->get_alias());
		m_contact.SetWindowTextW(machine->get_contact());
		m_addr.SetWindowTextW(machine->get_address());
		m_phone.SetWindowTextW(machine->get_phone());
		m_phone_bk.SetWindowTextW(machine->get_phone_bk());
			
		m_group.ResetContent();
		int theNdx = -1;
		CGroupInfo* rootGroup = CGroupManager::GetInstance()->GetRootGroupInfo();
		m_group.InsertString(0, rootGroup->get_name());
		m_group.SetItemData(0, (DWORD)rootGroup);
		if (machine->get_group_id() == rootGroup->get_id()) {
			theNdx = 0;
		}

		CGroupInfoList list;
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
			int nItem = 1;
			vMoveto.push_back(group);
			subMenu.CreatePopupMenu();
			CGroupInfo* rootGroup = CGroupManager::GetInstance()->GetRootGroupInfo();
			if (group->get_parent_group() != rootGroup) { // 不能移动至父亲分组
				CString rootName;
				rootName.LoadStringW(IDS_STRING_GROUP_ROOT);
				subMenu.AppendMenuW(MF_STRING, nItem++, rootName);
				vMoveto.push_back(rootGroup);
			}

			// 生成 "移动至-->"子菜单的菜单项
			CGroupInfoList list;
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
		LOG(L"TrackPopupMenu ret %d\n", ret);

		if (1 <= ret && ret < vMoveto.size()) { // move to
			CGroupInfo* dstGroup = vMoveto[ret];
			LOG(L"move %d %s to %d %s\n", group->get_id(), group->get_name(),
				dstGroup->get_id(), dstGroup->get_name());
			if (group->ExecuteMove2Group(dstGroup)) {
				LOG(L"move to succeed\n");
				CString rec, sgroup, sop;
				sgroup.LoadStringW(IDS_STRING_GROUP);
				sop.LoadStringW(IDS_STRING_GROUP_MOV);
				rec.Format(L"%s %s(%d) %s %s(%d)", sgroup, group->get_name(),
						   group->get_id(), sop, dstGroup->get_name(), dstGroup->get_id());
				CHistoryRecord::GetInstance()->InsertRecord(-1, rec, time(NULL),
															RECORD_LEVEL_USEREDIT);
				if (dstGroup->IsRootItem()) {
					m_tree.DeleteAllItems();
					ClearTree();

					HTREEITEM hRoot = m_tree.GetRootItem();
					HTREEITEM hRootGroup = m_tree.InsertItem(dstGroup->get_name(), hRoot);
					TreeItemData* tid = new TreeItemData(true, dstGroup);
					m_treeItamDataList.push_back(tid);
					m_tree.SetItemData(hRootGroup, (DWORD_PTR)tid);

					TraverseGroup(hRootGroup, dstGroup);

					m_curselTreeItemGroup = hRootGroup;
					m_tree.Expand(hRootGroup, TVE_EXPAND);
				} else {
					DeleteGroupItem(hItem);
					HTREEITEM hItemDst = GetTreeGroupItemByGroupInfo(dstGroup);
					HTREEITEM hItemDstParent = m_tree.GetParentItem(hItemDst);
					DWORD dstData = m_tree.GetItemData(hItemDst);
					m_tree.DeleteItem(hItemDst);
					hItemDst = m_tree.InsertItem(dstGroup->get_name(), hItemDstParent);
					m_tree.SetItemData(hItemDst, dstData);
					TraverseGroup(hItemDst, dstGroup);
				}
			}
		} else if (ret == ID_GROUP_ADD) { // add sub group
			CInputGroupNameDlg dlg;
			if (IDOK == dlg.DoModal()) {
				LOG(L"add sub group %s\n", dlg.m_value);
				CGroupInfo* child_group = group->ExecuteAddChildGroup(dlg.m_value);
				if (!child_group)
					return;
				CString rec, sgroup, sop;
				sgroup.LoadStringW(IDS_STRING_GROUP);
				sop.LoadStringW(IDS_STRING_GROUP_ADD_SUB);
				rec.Format(L"%s %s(%d) %s %s(%d)", sgroup, group->get_name(), 
						   group->get_id(), sop, dlg.m_value, child_group->get_id());
				CHistoryRecord::GetInstance()->InsertRecord(-1, rec, time(NULL),
															RECORD_LEVEL_USEREDIT);
				HTREEITEM  hItemNewGroup = m_tree.InsertItem(child_group->get_name(), hItem);
				TreeItemData* tid = new TreeItemData(true, child_group);
				m_treeItamDataList.push_back(tid);
				m_tree.SetItemData(hItemNewGroup, (DWORD_PTR)tid);
				m_tree.Expand(hItem, TVE_EXPAND);
				LOG(L"add sub group succeed, %d %d %s\n", child_group->get_id(), 
					child_group->get_parent_id(), child_group->get_name());
			}
		} else if (ret == ID_GROUP_DEL) { // delete group
			LOG(L"delete group %d %s\n", group->get_id(), group->get_name());
			CString rec, sgroup, sop;
			sgroup.LoadStringW(IDS_STRING_GROUP);
			sop.LoadStringW(IDS_STRING_GROUP_DEL);
			rec.Format(L"%s %s(%d) %s", sgroup, group->get_name(), group->get_id(), sop);
			CHistoryRecord::GetInstance()->InsertRecord(-1, rec, time(NULL),
														RECORD_LEVEL_USEREDIT);
			CGroupInfo* parentGroup = group->get_parent_group();
			if (parentGroup->ExecuteDeleteChildGroup(group)) {
				HTREEITEM hItemParent = m_tree.GetParentItem(hItem);
				ClearChildItems(hItemParent);
				TraverseGroup(hItemParent, parentGroup);
			}
		} else if (ret == ID_GROUP_RENAME) { // rename
			LOG(L"rename from %d %s\n", group->get_id(), group->get_name());
			CInputGroupNameDlg dlg;
			if (IDOK == dlg.DoModal()) {
				CString rec, sgroup, sop;
				sgroup.LoadStringW(IDS_STRING_GROUP);
				sop.LoadStringW(IDS_STRING_GROUP_REN);
				rec.Format(L"%s %s(%d) %s %s", sgroup, group->get_name(),
						   group->get_id(), sop, dlg.m_value);
				CHistoryRecord::GetInstance()->InsertRecord(-1, rec, time(NULL),
															RECORD_LEVEL_USEREDIT);
				if (group->ExecuteRename(dlg.m_value)) {
					LOG(L"rename to %d %s\n", group->get_id(), group->get_name());
					m_tree.SetItemText(hItem, group->get_name());
				}
			}
		}
	}
}


void CMachineManagerDlg::DeleteGroupItem(HTREEITEM hItem)
{
	TreeItemData* tid = reinterpret_cast<TreeItemData*>(m_tree.GetItemData(hItem));
	if (tid->_bGroup) {
		HTREEITEM hChild = m_tree.GetChildItem(hItem);
		while (hChild) {
			DeleteGroupItem(hChild);
			hChild = m_tree.GetNextSiblingItem(hChild);
		}
	}

	m_treeItamDataList.remove(tid);
	delete tid;
	m_tree.DeleteItem(hItem);
}


HTREEITEM CMachineManagerDlg::GetTreeGroupItemByGroupInfo(CGroupInfo* group)
{
	HTREEITEM hRoot = m_tree.GetRootItem();
	HTREEITEM hItem = GetTreeGroupItemByGroupInfoHelper(hRoot, group);
	return hItem;
}


HTREEITEM CMachineManagerDlg::GetTreeGroupItemByGroupInfoHelper(HTREEITEM hItem, 
																CGroupInfo* group)
{
	LOG_FUNCTION_AUTO;

#ifdef _DEBUG
	CString txt = m_tree.GetItemText(hItem);
	LOG(L"hItem %p %s\n", hItem, txt);
#endif
	
	TreeItemData* tid = reinterpret_cast<TreeItemData*>(m_tree.GetItemData(hItem));
	if (tid->_bGroup && tid->_udata == group) {
		return hItem;
	}

	HTREEITEM hChild = m_tree.GetChildItem(hItem);
	while (hChild) {
#ifdef _DEBUG
		CString txt = m_tree.GetItemText(hChild);
		LOG(L"hChild %p %s\n", hItem, txt);
#endif
		TreeItemData* tid = reinterpret_cast<TreeItemData*>(m_tree.GetItemData(hChild));
		if (tid->_bGroup) {
			if (tid->_udata == group) {
				return hChild;
			} 

			HTREEITEM dst = GetTreeGroupItemByGroupInfoHelper(hChild, group);
			if (dst) {
				return dst;
			}
		}
		hChild = m_tree.GetNextSiblingItem(hChild);
	}
	return NULL;
}


CAlarmMachine* CMachineManagerDlg::GetCurEditingMachine()
{
	do {
		if (!m_curselTreeItemMachine)
			break;

		DWORD data = m_tree.GetItemData(m_curselTreeItemMachine);
		TreeItemData* tid = reinterpret_cast<TreeItemData*>(data);
		if (!tid || tid->_bGroup)
			break;

		CAlarmMachine* machine = reinterpret_cast<CAlarmMachine*>(tid->_udata);
		if (!machine)
			break;

		return machine;
	} while (0);

	return NULL;
}


void CMachineManagerDlg::OnBnClickedButtonConfirmChange()
{}


void CMachineManagerDlg::OnBnClickedButtonDeleteMachine()
{
	CAlarmMachine* machine = GetCurEditingMachine();
	if (!machine) return;

	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (mgr->DeleteMachine(machine)) {
		TreeItemData* tid = reinterpret_cast<TreeItemData*>(m_tree.GetItemData(m_curselTreeItemMachine));
		m_treeItamDataList.remove(tid);
		delete tid;
		m_tree.DeleteItem(m_curselTreeItemMachine);
		m_curselTreeItemMachine = NULL;
	}
}


void CMachineManagerDlg::OnBnClickedButtonCreateMachine()
{
	CAddMachineDlg dlg;
	if (IDOK != dlg.DoModal())
		return;

	CAlarmMachine* machine = dlg.m_machine;
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (mgr->AddMachine(machine)) {
		CGroupInfo* group = CGroupManager::GetInstance()->GetGroupInfo(machine->get_group_id());

	}
}


void CMachineManagerDlg::OnCbnSelchangeComboBanned()
{
	int ndx = m_banned.GetCurSel();
	if (ndx != COMBO_NDX_NO && ndx != COMBO_NDX_YES) return;

	CAlarmMachine* machine = GetCurEditingMachine();
	if (!machine) return;

	bool banned = ndx == COMBO_NDX_YES;
	if (banned != machine->get_banned()) {
		bool ok = machine->execute_set_banned(banned);
		if (ok) {
			CString rec, fm;
			fm.LoadStringW(banned ? IDS_STRING_FM_BANNED : IDS_STRING_FM_UNBANNED);
			rec.Format(fm, machine->get_ademco_id(), machine->GetDeviceIDW());
			CHistoryRecord::GetInstance()->InsertRecord(machine->get_ademco_id(),
														rec, time(NULL), 
														RECORD_LEVEL_USEREDIT);
		} else {
			m_banned.SetCurSel(banned ? COMBO_NDX_NO : COMBO_NDX_YES);
		}
	}
}


void CMachineManagerDlg::OnCbnSelchangeComboType()
{
	int ndx = m_type.GetCurSel();
	if (ndx != COMBO_NDX_MAP && ndx != COMBO_NDX_VIDEO) return;

	CAlarmMachine* machine = GetCurEditingMachine();
	if (!machine) return;

	if (ndx != machine->get_type()) {
		bool ok = machine->execute_set_type(ndx);
		if (ok) {
			CString rec, fm, stype;
			fm.LoadStringW(IDS_STRING_FM_TYPE);
			stype.LoadStringW(ndx == COMBO_NDX_MAP ? IDS_STRING_TYPE_MAP : IDS_STRING_TYPE_VIDEO);
			rec.Format(fm, machine->get_ademco_id(), machine->GetDeviceIDW(), stype);
			CHistoryRecord::GetInstance()->InsertRecord(machine->get_ademco_id(),
														rec, time(NULL),
														RECORD_LEVEL_USEREDIT);
		} else {
			m_type.SetCurSel(ndx == COMBO_NDX_MAP ? COMBO_NDX_VIDEO : COMBO_NDX_MAP);
		}
	}
}


void CMachineManagerDlg::OnEnKillfocusEditName()
{
	CAlarmMachine* machine = GetCurEditingMachine();
	if (!machine) return;

	CString txt;
	m_name.GetWindowTextW(txt);
	if (txt.Compare(machine->get_alias()) != 0) {
		CString rec, smachine, sfield;
		smachine.LoadStringW(IDS_STRING_MACHINE);
		sfield.LoadStringW(IDS_STRING_ALIAS);
		rec.Format(L"%s(%04d) %s: %s --> %s", smachine, machine->get_ademco_id(),
				   sfield, machine->get_alias(), txt);
		CHistoryRecord::GetInstance()->InsertRecord(machine->get_ademco_id(), rec,
													time(NULL), RECORD_LEVEL_USEREDIT);

		machine->execute_set_alias(txt);
		CString newTxt;
		newTxt.Format(L"%s(%04d)", txt, machine->get_ademco_id());
		m_tree.SetItemText(m_curselTreeItemMachine, newTxt);
	}
}


void CMachineManagerDlg::OnEnKillfocusEditContact()
{
	CAlarmMachine* machine = GetCurEditingMachine();
	if (!machine) return;

	CString txt;
	m_contact.GetWindowTextW(txt);
	if (txt.Compare(machine->get_contact()) != 0) {
		CString rec, smachine, sfield;
		smachine.LoadStringW(IDS_STRING_MACHINE);
		sfield.LoadStringW(IDS_STRING_CONTACT);
		rec.Format(L"%s(%04d) %s: %s --> %s", smachine, machine->get_ademco_id(),
				   sfield, machine->get_contact(), txt);
		CHistoryRecord::GetInstance()->InsertRecord(machine->get_ademco_id(), rec,
													time(NULL), RECORD_LEVEL_USEREDIT);
		machine->execute_set_contact(txt);
	}
}


void CMachineManagerDlg::OnEnKillfocusEditAddress()
{
	CAlarmMachine* machine = GetCurEditingMachine();
	if (!machine) return;

	CString txt;
	m_addr.GetWindowTextW(txt);
	if (txt.Compare(machine->get_address()) != 0) {
		CString rec, smachine, sfield;
		smachine.LoadStringW(IDS_STRING_MACHINE);
		sfield.LoadStringW(IDS_STRING_ADDRESS);
		rec.Format(L"%s(%04d) %s: %s --> %s", smachine, machine->get_ademco_id(),
				   sfield, machine->get_address(), txt);
		CHistoryRecord::GetInstance()->InsertRecord(machine->get_ademco_id(), rec,
													time(NULL), RECORD_LEVEL_USEREDIT);

		machine->execute_set_address(txt);
	}
}


void CMachineManagerDlg::OnEnKillfocusEditPhone()
{
	CAlarmMachine* machine = GetCurEditingMachine();
	if (!machine) return;

	CString txt;
	m_phone.GetWindowTextW(txt);
	if (txt.Compare(machine->get_phone()) != 0) {
		CString rec, smachine, sfield;
		smachine.LoadStringW(IDS_STRING_MACHINE);
		sfield.LoadStringW(IDS_STRING_PHONE);
		rec.Format(L"%s(%04d) %s: %s --> %s", smachine, machine->get_ademco_id(),
				   sfield, machine->get_phone(), txt);
		CHistoryRecord::GetInstance()->InsertRecord(machine->get_ademco_id(), rec,
													time(NULL), RECORD_LEVEL_USEREDIT);

		machine->execute_set_phone(txt);
	}
}


void CMachineManagerDlg::OnEnKillfocusEditPhoneBk()
{
	CAlarmMachine* machine = GetCurEditingMachine();
	if (!machine) return;

	CString txt;
	m_phone_bk.GetWindowTextW(txt);
	if (txt.Compare(machine->get_phone_bk()) != 0) {
		CString rec, smachine, sfield;
		smachine.LoadStringW(IDS_STRING_MACHINE);
		sfield.LoadStringW(IDS_STRING_PHONE_BK);
		rec.Format(L"%s(%04d) %s: %s --> %s", smachine, machine->get_ademco_id(),
				   sfield, machine->get_phone_bk(), txt);
		CHistoryRecord::GetInstance()->InsertRecord(machine->get_ademco_id(), rec,
													time(NULL), RECORD_LEVEL_USEREDIT);

		machine->execute_set_phone_bk(txt);
	}
}


void CMachineManagerDlg::OnCbnSelchangeComboGroup()
{
	CAlarmMachine* machine = GetCurEditingMachine();
	if (!machine) return;

	int ndx = m_group.GetCurSel();
	if (ndx < 0) return;

	DWORD data = m_group.GetItemData(ndx);
	CGroupInfo* group = reinterpret_cast<CGroupInfo*>(data);
	if (!group) return;

	if (group->get_id() != machine->get_group_id()) {
		int old_group_id = machine->get_group_id();
		machine->execute_set_group_id(group->get_id());
		DWORD data = m_tree.GetItemData(m_curselTreeItemMachine);
		TreeItemData* tid = reinterpret_cast<TreeItemData*>(data);
		m_tree.DeleteItem(m_curselTreeItemMachine);
		HTREEITEM hGroup = GetTreeGroupItemByGroupInfo(group);
		CString txt;
		txt.Format(L"%s(%04d)", machine->get_alias(), machine->get_ademco_id());
		m_curselTreeItemMachine = m_tree.InsertItem(txt, hGroup);
		m_tree.SetItemData(m_curselTreeItemMachine, (DWORD_PTR)tid);
		m_tree.SelectItem(NULL);
		m_curselTreeItemMachine = NULL;
		//m_tree.SelectItem(m_curselTreeItemMachine); 

		CString rootName;
		rootName.LoadStringW(IDS_STRING_GROUP_ROOT);
		CString rec, smachine, sfield, sold_group, sgroup;
		smachine.LoadStringW(IDS_STRING_MACHINE);
		sfield.LoadStringW(IDS_STRING_GROUP);
		CGroupInfo* oldGroup = CGroupManager::GetInstance()->GetGroupInfo(old_group_id);
		if (oldGroup->IsRootItem()) {
			sold_group = rootName;
		} else {
			sold_group = oldGroup->get_name();
		}
		if (group->IsRootItem()) {
			sgroup = rootName;
		} else {
			sgroup = group->get_name();
		}
		rec.Format(L"%s(%04d) %s: %s(%d) --> %s(%d)", smachine, machine->get_ademco_id(),
				   sfield, sold_group, oldGroup->get_id(),
				   sgroup, group->get_id());
		CHistoryRecord::GetInstance()->InsertRecord(machine->get_ademco_id(), rec,
													time(NULL), RECORD_LEVEL_USEREDIT);
	}
}
