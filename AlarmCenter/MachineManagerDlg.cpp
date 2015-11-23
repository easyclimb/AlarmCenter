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
#include "ExtendExpireTimeDlg.h"
#include "PickMachineCoordinateDlg.h"
#include "SubMachineExpireManagerDlg.h"
#include "Sms.h"

using namespace core;

namespace {
	const int COMBO_NDX_NO = 0;
	const int COMBO_NDX_YES = 1;

	const int COMBO_NDX_MAP = 0;
	const int COMBO_NDX_VIDEO = 1;
};

// CMachineManagerDlg dialog

IMPLEMENT_DYNAMIC(CMachineManagerDlg, CDialogEx)

CMachineManagerDlg::CMachineManagerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CMachineManagerDlg::IDD, pParent)
	, m_curselTreeItemGroup(nullptr)
	, m_curselTreeItemMachine(nullptr)
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
	//DDX_Control(pDX, IDC_EDIT2, m_acct);
	DDX_Control(pDX, IDC_EDIT4, m_name);
	DDX_Control(pDX, IDC_EDIT5, m_contact);
	DDX_Control(pDX, IDC_EDIT6, m_addr);
	DDX_Control(pDX, IDC_EDIT7, m_phone);
	DDX_Control(pDX, IDC_EDIT8, m_phone_bk);
	DDX_Control(pDX, IDC_COMBO2, m_group);
	DDX_Control(pDX, IDC_BUTTON_DELETE_MACHINE, m_btnDelMachine);
	DDX_Control(pDX, IDC_COMBO_TYPE, m_type);
	DDX_Control(pDX, IDC_EDIT_EXPIRE_TIME, m_expire_time);
	DDX_Control(pDX, IDC_EDIT_X, m_x);
	DDX_Control(pDX, IDC_EDIT_Y, m_y);
	DDX_Control(pDX, IDC_BUTTON_PICK_COOR, m_pick_coor);
	DDX_Control(pDX, IDC_BUTTON_EXTEND, m_extend_expire);
	DDX_Control(pDX, IDC_CHECK1, m_chk_report_status);
	DDX_Control(pDX, IDC_CHECK4, m_chk_report_status_bk);
	DDX_Control(pDX, IDC_CHECK2, m_chk_report_exception);
	DDX_Control(pDX, IDC_CHECK5, m_chk_report_exception_bk);
	DDX_Control(pDX, IDC_CHECK3, m_chk_report_alarm);
	DDX_Control(pDX, IDC_CHECK6, m_chk_report_alarm_bk);
	DDX_Control(pDX, IDC_STATIC_HEX_ADEMCO_ID, m_staticHexAdemcoId);
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
	ON_BN_CLICKED(IDC_BUTTON_EXTEND, &CMachineManagerDlg::OnBnClickedButtonExtend)
	ON_BN_CLICKED(IDC_BUTTON_PICK_COOR, &CMachineManagerDlg::OnBnClickedButtonPickCoor)
	ON_BN_CLICKED(IDC_CHECK1, &CMachineManagerDlg::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_CHECK2, &CMachineManagerDlg::OnBnClickedCheck2)
	ON_BN_CLICKED(IDC_CHECK3, &CMachineManagerDlg::OnBnClickedCheck3)
	ON_BN_CLICKED(IDC_CHECK4, &CMachineManagerDlg::OnBnClickedCheck4)
	ON_BN_CLICKED(IDC_CHECK5, &CMachineManagerDlg::OnBnClickedCheck5)
	ON_BN_CLICKED(IDC_CHECK6, &CMachineManagerDlg::OnBnClickedCheck6)
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
	for (auto tid : m_treeItamDataList) {
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

	EditingMachine(FALSE);

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

	for (auto child_group : groupList) {
		txt.Format(L"%s", child_group->get_name()/*, child_group->get_machine_count()*/);
		HTREEITEM hChildGroupItem = m_tree.InsertItem(txt, hItemGroup);
		TreeItemData* tid = new TreeItemData(true, child_group);
		m_treeItamDataList.push_back(tid);
		m_tree.SetItemData(hChildGroupItem, (DWORD_PTR)tid);
		TraverseGroup(hChildGroupItem, child_group);
	}

	CAlarmMachineList machineList;
	group->GetChildMachines(machineList);
	for (auto machine : machineList) {
		txt.Format(L"%s(%04d)", machine->get_alias(), machine->get_ademco_id());
		HTREEITEM hChildItem = m_tree.InsertItem(txt, hItemGroup);
		TreeItemData* tid = new TreeItemData(false, machine);
		m_treeItamDataList.push_back(tid);
		m_tree.SetItemData(hChildItem, (DWORD_PTR)tid);
	}
}


void CMachineManagerDlg::EditingMachine(BOOL yes)
{
	m_pick_coor.EnableWindow(yes);
	m_btnDelMachine.EnableWindow(yes);
	m_extend_expire.EnableWindow(yes);

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

	m_chk_report_status.EnableWindow(yes);
	m_chk_report_status_bk.EnableWindow(yes);
	m_chk_report_exception.EnableWindow(yes);
	m_chk_report_exception_bk.EnableWindow(yes);
	m_chk_report_alarm.EnableWindow(yes);
	m_chk_report_alarm_bk.EnableWindow(yes);
}


void CMachineManagerDlg::OnTvnSelchangedTree1(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	if (pResult) *pResult = 0;
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (nullptr == hItem)
		return;
	DWORD data = m_tree.GetItemData(hItem);
	TreeItemData* tid = reinterpret_cast<TreeItemData*>(data);
	if (!tid)
		return;

	if (tid->_bGroup) {  // group item
		EditingMachine();
		m_curselTreeItemMachine = nullptr;
		if (m_curselTreeItemGroup == hItem) { EditingMachine(FALSE); return; }
		else { m_curselTreeItemGroup = hItem; }

		/*DWORD data = m_tree.GetItemData(hItem);
		CGroupInfo* group = reinterpret_cast<CGroupInfo*>(data);
		if (group) {
		}*/

		EditingMachine(FALSE);

	} else {	// machine item
		m_curselTreeItemGroup = nullptr;
		if (m_curselTreeItemMachine == hItem) { return; } 
		else { 
			EditingMachine(); 
			m_curselTreeItemMachine = hItem;
		}

		CAlarmMachine* machine = reinterpret_cast<CAlarmMachine*>(tid->_udata);
		if (!machine)
			return;

		CString txt;
		txt.Format(L"%04d", machine->get_ademco_id());
		m_id.SetWindowTextW(txt);

		txt.Format(L"%06X", machine->get_ademco_id());
		m_staticHexAdemcoId.SetWindowTextW(txt);

		int ndx = machine->get_banned();
		m_banned.SetCurSel(ndx);

		int type = machine->get_machine_type();
		m_type.SetCurSel(type);

		//m_acct.SetWindowTextW(machine->GetDeviceIDW());
		m_name.SetWindowTextW(machine->get_alias());
		m_contact.SetWindowTextW(machine->get_contact());
		m_addr.SetWindowTextW(machine->get_address());
		m_phone.SetWindowTextW(machine->get_phone());
		m_phone_bk.SetWindowTextW(machine->get_phone_bk());
		m_expire_time.SetWindowTextW(machine->get_expire_time().Format(L"%Y-%m-%d %H:%M:%S"));
		web::BaiduCoordinate coor = machine->get_coor();
		txt.Format(L"%f", coor.x);
		m_x.SetWindowTextW(txt);
		txt.Format(L"%f", coor.y);
		m_y.SetWindowTextW(txt);

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
		for (auto group : list) {
			ndx = m_group.AddString(group->get_name());
			m_group.SetItemData(ndx, (DWORD)group);
			if (machine->get_group_id() == group->get_id()) {
				theNdx = ndx;
			}
		}
		m_group.SetCurSel(theNdx);

		SmsConfigure cfg = machine->get_sms_cfg();
		m_chk_report_alarm.SetCheck(cfg.report_alarm);
		m_chk_report_status.SetCheck(cfg.report_status);
		m_chk_report_exception.SetCheck(cfg.report_exception);
		m_chk_report_alarm_bk.SetCheck(cfg.report_alarm_bk);
		m_chk_report_status_bk.SetCheck(cfg.report_status_bk);
		m_chk_report_exception_bk.SetCheck(cfg.report_exception_bk);
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
			for (auto child_group : list) {
				if (child_group != group // 不能移动至同一个分组
					&& !group->IsDescendantGroup(child_group) // 不能移动至自己的后代分组
					&& (child_group != group->get_parent_group())) { // 不能移动至父亲分组，你丫本来就在那
					subMenu.AppendMenuW(MF_STRING, nItem++, child_group->get_name());
					vMoveto.push_back(child_group);
				}
			}

			CString sMoveTo;
			sMoveTo.LoadStringW(IDS_STRING_MOVE_TO);
			pMenu->InsertMenuW(4, MF_POPUP | MF_BYPOSITION,
							   (UINT)subMenu.GetSafeHmenu(), sMoveTo);
		}

		GetCursorPos(&pt);
		DWORD ret = pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
										pt.x, pt.y, this);
		JLOG(L"TrackPopupMenu ret %d\n", ret);

		if (1 <= ret && ret < vMoveto.size()) { // move to

			CGroupInfo* dstGroup = vMoveto[ret];
			JLOG(L"move %d %s to %d %s\n", group->get_id(), group->get_name(),
				dstGroup->get_id(), dstGroup->get_name());
			if (group->ExecuteMove2Group(dstGroup)) {
				JLOG(L"move to succeed\n");
				CString rec, sgroup, sop;
				sgroup.LoadStringW(IDS_STRING_GROUP);
				sop.LoadStringW(IDS_STRING_GROUP_MOV);
				rec.Format(L"%s %s(%d) %s %s(%d)", sgroup, group->get_name(),
						   group->get_id(), sop, dstGroup->get_name(), dstGroup->get_id());
				CHistoryRecord::GetInstance()->InsertRecord(-1, -1, rec, time(nullptr),
															RECORD_LEVEL_USEREDIT);
				if (dstGroup->IsRootItem()) {
					m_tree.DeleteAllItems();
					ClearTree();

					HTREEITEM hRoot = m_tree.GetRootItem();
					HTREEITEM hRootGroup = m_tree.InsertItem(dstGroup->get_name(), hRoot);
					TreeItemData* _tid = new TreeItemData(true, dstGroup);
					m_treeItamDataList.push_back(_tid);
					m_tree.SetItemData(hRootGroup, (DWORD_PTR)_tid);

					TraverseGroup(hRootGroup, dstGroup);

					m_curselTreeItemGroup = hRootGroup;
					m_tree.Expand(hRootGroup, TVE_EXPAND);
				} else {
					m_curselTreeItemGroup = nullptr;
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
			if (IDOK != dlg.DoModal() || dlg.m_value.IsEmpty()) return;
			JLOG(L"add sub group %s\n", dlg.m_value);
			CGroupInfo* child_group = group->ExecuteAddChildGroup(dlg.m_value);
			if (!child_group)
				return;
			CString rec, sgroup, sop;
			sgroup.LoadStringW(IDS_STRING_GROUP);
			sop.LoadStringW(IDS_STRING_GROUP_ADD_SUB);
			rec.Format(L"%s %s(%d) %s %s(%d)", sgroup, group->get_name(), 
						group->get_id(), sop, dlg.m_value, child_group->get_id());
			CHistoryRecord::GetInstance()->InsertRecord(-1, -1, rec, time(nullptr),
														RECORD_LEVEL_USEREDIT);
			HTREEITEM  hItemNewGroup = m_tree.InsertItem(child_group->get_name(), hItem);
			TreeItemData* _tid = new TreeItemData(true, child_group);
			m_treeItamDataList.push_back(_tid);
			m_tree.SetItemData(hItemNewGroup, (DWORD_PTR)_tid);
			m_tree.Expand(hItem, TVE_EXPAND);
			JLOG(L"add sub group succeed, %d %d %s\n", child_group->get_id(), 
				child_group->get_parent_id(), child_group->get_name());
		} else if (ret == ID_GROUP_DEL) { // delete group
			JLOG(L"delete group %d %s\n", group->get_id(), group->get_name());
			CString rec, sgroup, sop;
			sgroup.LoadStringW(IDS_STRING_GROUP);
			sop.LoadStringW(IDS_STRING_GROUP_DEL);
			rec.Format(L"%s %s(%d) %s", sgroup, group->get_name(), group->get_id(), sop);
			CHistoryRecord::GetInstance()->InsertRecord(-1, -1, rec, time(nullptr),
														RECORD_LEVEL_USEREDIT);
			CGroupInfo* parentGroup = group->get_parent_group();
			if (parentGroup->ExecuteDeleteChildGroup(group)) {
				HTREEITEM hItemParent = m_tree.GetParentItem(hItem);
				ClearChildItems(hItemParent);
				TraverseGroup(hItemParent, parentGroup);
			}
		} else if (ret == ID_GROUP_RENAME) { // rename
			JLOG(L"rename from %d %s\n", group->get_id(), group->get_name());
			CInputGroupNameDlg dlg;
			if (IDOK == dlg.DoModal()) {
				CString rec, sgroup, sop;
				sgroup.LoadStringW(IDS_STRING_GROUP);
				sop.LoadStringW(IDS_STRING_GROUP_REN);
				rec.Format(L"%s %s(%d) %s %s", sgroup, group->get_name(),
						   group->get_id(), sop, dlg.m_value);
				CHistoryRecord::GetInstance()->InsertRecord(-1, -1, rec, time(nullptr),
															RECORD_LEVEL_USEREDIT);
				if (group->ExecuteRename(dlg.m_value)) {
					JLOG(L"rename to %d %s\n", group->get_id(), group->get_name());
					m_tree.SetItemText(hItem, group->get_name());
				}
			}
		} else if (ret == ID_GROUP_EXPIRE_MANAGE) {
			CAlarmMachineList list;
			group->GetChildMachines(list);
			CMachineExpireManagerDlg dlg;
			dlg.SetExpiredMachineList(list);
			dlg.DoModal();
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
	AUTO_LOG_FUNCTION;
	
#ifdef _DEBUG
	CString txt = L"";
	txt = m_tree.GetItemText(hItem);
	JLOG(L"hItem %p %s\n", hItem, txt);
#endif
	
	TreeItemData* tid = reinterpret_cast<TreeItemData*>(m_tree.GetItemData(hItem));
	if (tid->_bGroup && tid->_udata == group) {
		return hItem;
	}

	HTREEITEM hChild = m_tree.GetChildItem(hItem);
	while (hChild) {
#ifdef _DEBUG
		txt = m_tree.GetItemText(hChild);
		JLOG(L"hChild %p %s\n", hItem, txt);
#endif
		TreeItemData* _tid = reinterpret_cast<TreeItemData*>(m_tree.GetItemData(hChild));
		if (_tid->_bGroup) {
			if (_tid->_udata == group) {
				return hChild;
			} 

			HTREEITEM dst = GetTreeGroupItemByGroupInfoHelper(hChild, group);
			if (dst) {
				return dst;
			}
		}
		hChild = m_tree.GetNextSiblingItem(hChild);
	}
	return nullptr;
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

	return nullptr;
}


void CMachineManagerDlg::OnBnClickedButtonConfirmChange()
{}


void CMachineManagerDlg::OnBnClickedButtonDeleteMachine()
{
	CAlarmMachine* machine = GetCurEditingMachine();
	if (!machine) return;

	CString s, fm; fm.LoadStringW(IDS_STRING_FM_CONFIRM_DEL_MACHINE);
	s.Format(fm, machine->get_ademco_id(), machine->get_alias());
	if (IDOK != MessageBox(s, L"", MB_ICONQUESTION | MB_OKCANCEL))
		return;

	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (mgr->DeleteMachine(machine)) {
		TreeItemData* tid = reinterpret_cast<TreeItemData*>(m_tree.GetItemData(m_curselTreeItemMachine));
		m_treeItamDataList.remove(tid);
		delete tid;
		m_tree.DeleteItem(m_curselTreeItemMachine);
		m_curselTreeItemMachine = nullptr;
		m_tree.SelectItem(m_tree.GetRootItem());
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
		group->AddChildMachine(machine);
		HTREEITEM hItem = GetTreeGroupItemByGroupInfo(group);
		if (hItem) {
			CString txt;
			txt.Format(L"%s(%04d)", machine->get_alias(), machine->get_ademco_id());
			HTREEITEM hChild = m_tree.InsertItem(txt, hItem);
			TreeItemData* tid = new TreeItemData(false, machine);
			m_treeItamDataList.push_back(tid);
			m_tree.SetItemData(hChild, reinterpret_cast<DWORD_PTR>(tid));
			m_tree.Expand(hItem, TVE_EXPAND);
			m_tree.SelectItem(hChild);
		}
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
			rec.Format(fm, machine->get_ademco_id()/*, machine->GetDeviceIDW()*/);
			CHistoryRecord::GetInstance()->InsertRecord(machine->get_ademco_id(),
														0, rec, time(nullptr), 
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
	bool has_video = ndx == COMBO_NDX_VIDEO;

	CAlarmMachine* machine = GetCurEditingMachine();
	if (!machine) return;

	if (has_video != machine->get_has_video()) {
		bool ok = machine->execute_set_has_video(has_video);
		if (ok) {
			CString rec, fm, stype;
			fm.LoadStringW(IDS_STRING_FM_TYPE);
			stype.LoadStringW(ndx == COMBO_NDX_MAP ? IDS_STRING_TYPE_MAP : IDS_STRING_TYPE_VIDEO);
			rec.Format(fm, machine->get_ademco_id(), /*machine->GetDeviceIDW(), */stype);
			CHistoryRecord::GetInstance()->InsertRecord(machine->get_ademco_id(),
														0, rec, time(nullptr),
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
		CHistoryRecord::GetInstance()->InsertRecord(machine->get_ademco_id(), 0, rec,
													time(nullptr), RECORD_LEVEL_USEREDIT);

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
		CHistoryRecord::GetInstance()->InsertRecord(machine->get_ademco_id(), 0, rec,
													time(nullptr), RECORD_LEVEL_USEREDIT);
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
		CHistoryRecord::GetInstance()->InsertRecord(machine->get_ademco_id(), 0, rec,
													time(nullptr), RECORD_LEVEL_USEREDIT);

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
		CHistoryRecord::GetInstance()->InsertRecord(machine->get_ademco_id(), 0, rec,
													time(nullptr), RECORD_LEVEL_USEREDIT);

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
		CHistoryRecord::GetInstance()->InsertRecord(machine->get_ademco_id(), 0, rec,
													time(nullptr), RECORD_LEVEL_USEREDIT);

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
		data = m_tree.GetItemData(m_curselTreeItemMachine);
		TreeItemData* tid = reinterpret_cast<TreeItemData*>(data);
		//HTREEITEM hNext = m_tree.GetNextSiblingItem(m_curselTreeItemMachine);
		m_tree.DeleteItem(m_curselTreeItemMachine);
		m_curselTreeItemMachine = m_tree.GetSelectedItem();
		HTREEITEM hGroup = GetTreeGroupItemByGroupInfo(group);
		CString txt;
		txt.Format(L"%s(%04d)", machine->get_alias(), machine->get_ademco_id());
		HTREEITEM hItem = m_tree.InsertItem(txt, hGroup);
		m_tree.SetItemData(hItem, (DWORD_PTR)tid);
		//m_tree.SelectItem(nullptr); OnTvnSelchangedTree1(nullptr, nullptr);
		//m_curselTreeItemMachine = nullptr;
		//m_tree.SelectItem(m_curselTreeItemMachine); 
		//if (hNext) { m_tree.SelectItem(hNext); }
		OnTvnSelchangedTree1(nullptr, nullptr);

		CString rootName;
		rootName.LoadStringW(IDS_STRING_GROUP_ROOT);
		CString rec, smachine, sfield, sold_group, sgroup;
		smachine.LoadStringW(IDS_STRING_MACHINE);
		sfield.LoadStringW(IDS_STRING_GROUP);
		CGroupInfo* oldGroup = CGroupManager::GetInstance()->GetGroupInfo(old_group_id);

		if (oldGroup->IsRootItem()) { sold_group = rootName;
		} else { sold_group = oldGroup->get_name(); }

		if (group->IsRootItem()) {
			sgroup = rootName;
		} else { sgroup = group->get_name(); }

		rec.Format(L"%s(%04d) %s: %s(%d) --> %s(%d)", smachine, machine->get_ademco_id(),
				   sfield, sold_group, oldGroup->get_id(),
				   sgroup, group->get_id());
		CHistoryRecord::GetInstance()->InsertRecord(machine->get_ademco_id(), 0, rec,
													time(nullptr), RECORD_LEVEL_USEREDIT);
	}
}


void CMachineManagerDlg::OnBnClickedButtonExtend()
{
	CAlarmMachine* machine = GetCurEditingMachine();
	if (!machine) return;
	CExtendExpireTimeDlg dlg; if (IDOK != dlg.DoModal()) return;
	COleDateTime datetime = dlg.m_dateTime;
#ifdef _DEBUG
	CString s = datetime.Format(L"%Y-%m-%d %H:%M:%S");
#endif
	if (machine->execute_update_expire_time(datetime)) {
		m_expire_time.SetWindowTextW(datetime.Format(L"%Y-%m-%d %H:%M:%S"));
	}
}


void CMachineManagerDlg::OnBnClickedButtonPickCoor()
{
	AUTO_LOG_FUNCTION;
	CAlarmMachine* machine = GetCurEditingMachine();
	if (!machine) return;

	/*CPickMachineCoordinateDlg dlg;
	dlg.m_machine = machine;
	dlg.DoModal();*/
	if (g_baiduMapDlg && IsWindow(g_baiduMapDlg->m_hWnd)) {
		g_baiduMapDlg->ShowMap(machine);
	}
	web::BaiduCoordinate coor = machine->get_coor();
	CString txt;
	txt.Format(L"%f", coor.x);
	m_x.SetWindowTextW(txt);
	txt.Format(L"%f", coor.y);
	m_y.SetWindowTextW(txt);
	
}


void CMachineManagerDlg::OnBnClickedCheck1()
{
	AUTO_LOG_FUNCTION;
	CAlarmMachine* machine = GetCurEditingMachine();
	if (!machine) return;
	BOOL b = m_chk_report_status.GetCheck();
	SmsConfigure cfg = machine->get_sms_cfg();
	cfg.report_status = b ? true : false;
	if (CSms::GetInstance()->set_sms_config(cfg)) {
		machine->set_sms_cfg(cfg);
	} 
}


void CMachineManagerDlg::OnBnClickedCheck2()
{
	AUTO_LOG_FUNCTION;
	CAlarmMachine* machine = GetCurEditingMachine();
	if (!machine) return;
	BOOL b = m_chk_report_exception.GetCheck();
	SmsConfigure cfg = machine->get_sms_cfg();
	cfg.report_exception = b ? true : false;
	if (CSms::GetInstance()->set_sms_config(cfg)) {
		machine->set_sms_cfg(cfg);
	}
}


void CMachineManagerDlg::OnBnClickedCheck3()
{
	AUTO_LOG_FUNCTION;
	CAlarmMachine* machine = GetCurEditingMachine();
	if (!machine) return;
	BOOL b = m_chk_report_alarm.GetCheck();
	SmsConfigure cfg = machine->get_sms_cfg();
	cfg.report_alarm = b ? true : false;
	if (CSms::GetInstance()->set_sms_config(cfg)) {
		machine->set_sms_cfg(cfg);
	}
}


void CMachineManagerDlg::OnBnClickedCheck4()
{
	AUTO_LOG_FUNCTION;
	CAlarmMachine* machine = GetCurEditingMachine();
	if (!machine) return;
	BOOL b = m_chk_report_status_bk.GetCheck();
	SmsConfigure cfg = machine->get_sms_cfg();
	cfg.report_status_bk = b ? true : false;
	if (CSms::GetInstance()->set_sms_config(cfg)) {
		machine->set_sms_cfg(cfg);
	}
}


void CMachineManagerDlg::OnBnClickedCheck5()
{
	AUTO_LOG_FUNCTION;
	CAlarmMachine* machine = GetCurEditingMachine();
	if (!machine) return;
	BOOL b = m_chk_report_exception_bk.GetCheck();
	SmsConfigure cfg = machine->get_sms_cfg();
	cfg.report_exception_bk = b ? true : false;
	if (CSms::GetInstance()->set_sms_config(cfg)) {
		machine->set_sms_cfg(cfg);
	}
}


void CMachineManagerDlg::OnBnClickedCheck6()
{
	AUTO_LOG_FUNCTION;
	CAlarmMachine* machine = GetCurEditingMachine();
	if (!machine) return;
	BOOL b = m_chk_report_alarm_bk.GetCheck();
	SmsConfigure cfg = machine->get_sms_cfg();
	cfg.report_alarm_bk = b ? true : false;
	if (CSms::GetInstance()->set_sms_config(cfg)) {
		machine->set_sms_cfg(cfg);
	}
}
