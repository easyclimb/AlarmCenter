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
#include "alarm_center_map_service.h"
#include "SubMachineExpireManagerDlg.h"
#include "ZoneInfo.h"
#include "ConsumerTypeMgrDlg.h"
#include "consumer.h"

using namespace core;

namespace detail {
	const int COMBO_NDX_NO = 0;
	const int COMBO_NDX_YES = 1;

	const int COMBO_NDX_MAP = 0;
	const int COMBO_NDX_VIDEO = 1;

	const int TIMER_ID_FLUSH_BAIDU_POS = 1;
	const int TIMER_ID_CALC_OWED_AMOUNT = 2;

	

};

using namespace detail;
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
	DDX_Control(pDX, IDC_EDIT_RECEIVABLE, m_receivable_amount);
	DDX_Control(pDX, IDC_EDIT_PAID, m_paid_amount);
	DDX_Control(pDX, IDC_EDIT_OWED, m_owd_amount);
	DDX_Control(pDX, IDC_EDIT_REMIND_TIME, m_remind_time);
	DDX_Control(pDX, IDC_BUTTON_SET_REMIND_TIME, m_btn_set_remind);
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
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEHOVER()
	ON_BN_CLICKED(IDC_BUTTON_TYPE_MANAGER, &CMachineManagerDlg::OnBnClickedButtonTypeManager)
	ON_EN_CHANGE(IDC_EDIT_RECEIVABLE, &CMachineManagerDlg::OnEnChangeEditReceivable)
	ON_EN_CHANGE(IDC_EDIT_PAID, &CMachineManagerDlg::OnEnChangeEditPaid)
	ON_BN_CLICKED(IDC_BUTTON_SET_REMIND_TIME, &CMachineManagerDlg::OnBnClickedButtonSetRemindTime)
END_MESSAGE_MAP()


// CMachineManagerDlg message handlers

void CMachineManagerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	alarm_machine_manager::get_instance()->LeaveBufferMode();
	ClearTree();
}


void CMachineManagerDlg::InitTree()
{
	CString txt;
	auto mgr = group_manager::get_instance();
	group_info_ptr rootGroup = mgr->GetRootGroupInfo();
	if (rootGroup) {
		HTREEITEM hRoot = m_tree.GetRootItem();
		txt.Format(L"%s[%d]", rootGroup->get_formatted_group_name(), rootGroup->get_descendant_machine_count());
		HTREEITEM hRootGroup = m_tree.InsertItem(txt, hRoot);
		TreeItemDataPtr tid = std::make_shared<TreeItemData>(rootGroup);
		m_tidMap[hRootGroup] = tid;
		TraverseGroup(hRootGroup, rootGroup);
		m_curselTreeItemGroup = hRootGroup;
		m_tree.Expand(hRootGroup, TVE_EXPAND);
	}
}


void CMachineManagerDlg::ClearTree()
{
	m_tidMap.clear();
	m_tree.DeleteAllItems();
	EditingMachine(0);
}


void CMachineManagerDlg::ClearChildItems(HTREEITEM hItemParent)
{
	HTREEITEM hItem = m_tree.GetChildItem(hItemParent);
	while (hItem) {
		m_tidMap.erase(hItem);
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

	SetWindowText(TR(IDS_STRING_IDD_DIALOG_MACHINE_MGR));
	SET_WINDOW_TEXT(IDC_STATIC_GROUP_INFO, IDS_STRING_IDC_STATIC_006);
	SET_WINDOW_TEXT(IDC_STATIC_MACHINE_ID, IDS_STRING_IDC_STATIC_008);
	SET_WINDOW_TEXT(IDC_STATIC_1, IDS_STRING_IDC_STATIC_081);
	SET_WINDOW_TEXT(IDC_STATIC_2, IDS_STRING_IDC_STATIC_010);
	SET_WINDOW_TEXT(IDC_STATIC_3, IDS_STRING_HRLV);
	SET_WINDOW_TEXT(IDC_STATIC_4, IDS_STRING_IDC_STATIC_011);
	SET_WINDOW_TEXT(IDC_STATIC_5, IDS_STRING_IDC_STATIC_012);
	SET_WINDOW_TEXT(IDC_STATIC_6, IDS_STRING_IDC_STATIC_013);
	SET_WINDOW_TEXT(IDC_STATIC_7, IDS_STRING_IDC_STATIC_054);
	SET_WINDOW_TEXT(IDC_BUTTON_PICK_COOR_8, IDS_STRING_IDC_BUTTON_PICK_COOR);
	SET_WINDOW_TEXT(IDC_STATIC_9, IDS_STRING_IDC_STATIC_014);
	SET_WINDOW_TEXT(IDC_STATIC_10, IDS_STRING_IDC_STATIC_015);
	SET_WINDOW_TEXT(IDC_STATIC_11, IDS_STRING_IDC_STATIC_016);
	SET_WINDOW_TEXT(IDC_STATIC_12, IDS_STRING_IDC_STATIC_094);
	SET_WINDOW_TEXT(IDC_STATIC_13, IDS_STRING_IDC_STATIC_099);
	SET_WINDOW_TEXT(IDC_BUTTON_TYPE_MANAGER, IDS_STRING_IDC_BUTTON_TYPE_MANAGER);
	SET_WINDOW_TEXT(IDC_BUTTON_CREATE_MACHINE, IDS_STRING_IDR_MENU2_POPUP_001_ID_GROUP_CREATE_MACHINE);
	SET_WINDOW_TEXT(IDC_BUTTON_DELETE_MACHINE, IDS_STRING_IDR_MENU3_POPUP_002_ID_GROUP_32788);
	SET_WINDOW_TEXT(IDC_STATIC_143, IDS_STRING_IDC_STATIC_092);
	SET_WINDOW_TEXT(IDC_STATIC_REC_AM, IDS_STRING_RECEIVABLE_AMOUNT);
	SET_WINDOW_TEXT(IDC_STATIC_15, IDS_STRING_IDC_STATIC_019);
	SET_WINDOW_TEXT(IDC_STATIC_16, IDS_STRING_IDC_STATIC_020);	//IDS_STRING_REPORT_CHANGE
	SET_WINDOW_TEXT(IDC_CHECK1, IDS_STRING_REPORT_CHANGE);
	SET_WINDOW_TEXT(IDC_CHECK2, IDS_STRING_ERPORT_ABNORMAL_EVENTS);
	SET_WINDOW_TEXT(IDC_CHECK3, IDS_STRING_ALARM_EVENTS);
	SET_WINDOW_TEXT(IDC_CHECK4, IDS_STRING_REPORT_CHANGE);
	SET_WINDOW_TEXT(IDC_CHECK5, IDS_STRING_ERPORT_ABNORMAL_EVENTS);
	SET_WINDOW_TEXT(IDC_CHECK6, IDS_STRING_ALARM_EVENTS);
	SET_WINDOW_TEXT(IDC_BUTTON_EXTEND, IDS_STRING_IDC_BUTTON_EXTEND);
	SET_WINDOW_TEXT(IDC_BUTTON_SET_REMIND_TIME, IDS_STRING_IDC_BUTTON_SET_ALARM);


	alarm_machine_manager::get_instance()->EnterBufferMode();

	CString yes, no;
	yes = TR(IDS_STRING_YES);
	no = TR(IDS_STRING_NO);
	int combo_ndx = -1;
	combo_ndx = m_banned.InsertString(detail::COMBO_NDX_NO, no);
	ASSERT(combo_ndx == detail::COMBO_NDX_NO);
	combo_ndx = m_banned.InsertString(detail::COMBO_NDX_YES, yes);
	ASSERT(combo_ndx == detail::COMBO_NDX_YES);

	/*CString normal, video;
	normal = TR(IDS_STRING_TYPE_MAP);
	video = TR(IDS_STRING_TYPE_VIDEO);
	combo_ndx = m_type.InsertString(detail::COMBO_NDX_MAP, normal);
	ASSERT(combo_ndx == detail::COMBO_NDX_MAP);
	combo_ndx = m_type.InsertString(detail::COMBO_NDX_VIDEO, video);
	ASSERT(combo_ndx == detail::COMBO_NDX_VIDEO);*/


	InitTypes();


	EditingMachine(FALSE);

	InitTree();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CMachineManagerDlg::InitTypes()
{
	m_type.ResetContent();
	int combo_ndx = -1;
	auto mgr = consumer_manager::get_instance();
	auto types = mgr->get_all_types();
	for (auto iter : types) {
		combo_ndx = m_type.AddString(iter.second->name);
		m_type.SetItemData(combo_ndx, iter.first);
	}

	combo_ndx = m_type.AddString(TR(IDS_STRING_USER_DEFINE));
	m_type.SetItemData(combo_ndx, 0xFFFFFFFF);
}


void CMachineManagerDlg::TraverseGroup(HTREEITEM hItemGroup, core::group_info_ptr group)
{
	group_info_list groupList;
	group->GetChildGroups(groupList);

	CString txt;
	for (auto child_group : groupList) {
		txt.Format(L"%s[%d]", child_group->get_formatted_group_name(), child_group->get_descendant_machine_count());
		HTREEITEM hChildGroupItem = m_tree.InsertItem(txt, hItemGroup);
		TreeItemDataPtr tid = std::make_shared<TreeItemData>(child_group);
		m_tidMap[hChildGroupItem] = tid;
		TraverseGroup(hChildGroupItem, child_group);
	}

	alarm_machine_list machineList;
	group->GetChildMachines(machineList);
	for (auto machine : machineList) {
		HTREEITEM hChildItem = m_tree.InsertItem(machine->get_formatted_name(), hItemGroup);
		TreeItemDataPtr tid = std::make_shared<TreeItemData>(machine);
		m_tidMap[hChildItem] = tid;
	}
}


void CMachineManagerDlg::update_ancester_items_text(HTREEITEM hItem) 
{
	if (!hItem)
		return;
	CMachineManagerDlg::TreeItemDataPtr tid = m_tidMap[hItem];
	if (!tid)
		return;
	if (tid->_bGroup && tid->_group) {
		auto group = tid->_group;
		CString txt;
		txt.Format(L"%s[%d]", group->get_formatted_group_name(), group->get_descendant_machine_count());
		m_tree.SetItemText(hItem, txt);

		hItem = m_tree.GetParentItem(hItem);
		update_ancester_items_text(hItem);
	}
}

void CMachineManagerDlg::EditingMachine(BOOL yes)
{
	m_pick_coor.EnableWindow(yes);
	m_btnDelMachine.EnableWindow(yes);
	m_extend_expire.EnableWindow(yes);
	m_btn_set_remind.EnableWindow(yes);

	m_banned.EnableWindow(yes);
	//m_acct.EnableWindow(yes);
	m_type.EnableWindow(yes);
	m_name.EnableWindow(yes);
	m_contact.EnableWindow(yes);
	m_addr.EnableWindow(yes);
	m_phone.EnableWindow(yes);
	m_phone_bk.EnableWindow(yes);
	//m_group.EnableWindow(yes);

	m_receivable_amount.EnableWindow(yes);
	m_paid_amount.EnableWindow(yes);

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
	KillTimer(detail::TIMER_ID_FLUSH_BAIDU_POS);
	if (pResult) *pResult = 0;

	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (nullptr == hItem)
		return;

	TreeItemDataPtr tid = m_tidMap[hItem];
	if (!tid)
		return;

	if (tid->_bGroup) {  // group item
		EditingMachine();
		m_curselTreeItemMachine = nullptr;
		if (m_curselTreeItemGroup == hItem) { EditingMachine(FALSE); return; }
		else { m_curselTreeItemGroup = hItem; }

		EditingMachine(FALSE);
	} else {	// machine item
		m_curselTreeItemGroup = nullptr;
		if (m_curselTreeItemMachine == hItem) { return; } 
		else { 
			EditingMachine(); 
			m_curselTreeItemMachine = hItem;
		}

		alarm_machine_ptr machine = tid->_machine;
		if (!machine)
			return;

		CString txt;
		txt.Format(TR(IDS_STRING_FM_ADEMCO_ID), machine->get_ademco_id());
		m_id.SetWindowTextW(txt);

		txt.Format(L"%06X", machine->get_ademco_id());
		m_staticHexAdemcoId.SetWindowTextW(txt);

		int ndx = machine->get_banned();
		m_banned.SetCurSel(ndx);

		auto consumer = machine->get_consumer();
		for (int i = 0; i < m_type.GetCount(); i++) {
			if (static_cast<int>(m_type.GetItemData(i)) == consumer->type->id) {
				m_type.SetCurSel(i);
				break;
			}
		}

		m_name.SetWindowTextW(machine->get_machine_name());
		m_contact.SetWindowTextW(machine->get_contact());
		m_addr.SetWindowTextW(machine->get_address());
		m_phone.SetWindowTextW(machine->get_phone());
		m_phone_bk.SetWindowTextW(machine->get_phone_bk());
		m_expire_time.SetWindowTextW(time_point_to_wstring(machine->get_expire_time()).c_str());
		m_remind_time.SetWindowTextW(time_point_to_wstring(machine->get_consumer()->remind_time).c_str());

		web::BaiduCoordinate coor = machine->get_coor();
		txt.Format(L"%f", coor.x);
		m_x.SetWindowTextW(txt);
		txt.Format(L"%f", coor.y);
		m_y.SetWindowTextW(txt);

		m_group.ResetContent();
		int theNdx = -1;
		group_info_ptr rootGroup = group_manager::get_instance()->GetRootGroupInfo();
		m_group.InsertString(0, rootGroup->get_formatted_group_name());
		if (machine->get_group_id() == rootGroup->get_id()) {
			theNdx = 0;
		}

		group_info_list list;
		rootGroup->GetDescendantGroups(list);
		for (auto group : list) {
			ndx = m_group.AddString(group->get_formatted_group_name());
			m_group.SetItemData(ndx, (DWORD)group->get_id());
			if (machine->get_group_id() == group->get_id()) {
				theNdx = ndx;
			}
		}
		m_group.SetCurSel(theNdx);

		sms_config cfg = machine->get_sms_cfg();
		m_chk_report_alarm.SetCheck(cfg.report_alarm);
		m_chk_report_status.SetCheck(cfg.report_status);
		m_chk_report_exception.SetCheck(cfg.report_exception);
		m_chk_report_alarm_bk.SetCheck(cfg.report_alarm_bk);
		m_chk_report_status_bk.SetCheck(cfg.report_status_bk);
		m_chk_report_exception_bk.SetCheck(cfg.report_exception_bk);
		
		txt.Format(L"%d", consumer->receivable_amount);
		m_receivable_amount.SetWindowTextW(txt);
		txt.Format(L"%d", consumer->paid_amount);
		m_paid_amount.SetWindowTextW(txt);
		txt.Format(L"%d", consumer->get_owed_amount());
		m_owd_amount.SetWindowTextW(txt);

		auto_timer timer1(m_hWnd, detail::TIMER_ID_FLUSH_BAIDU_POS, 1000);
		auto_timer timer2(m_hWnd, detail::TIMER_ID_CALC_OWED_AMOUNT, 500);
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
		TreeItemDataPtr tid = m_tidMap[hItem];
		if (!tid)
			return;

		m_tree.SelectItem(hItem);
		EditingMachine(TRUE);

		group_info_ptr group = tid->_group;
		if (!group) { // machine item
			auto machine = GetCurEditingMachine();
			if (!machine) return;
			CMenu menu, *pMenu, subMenu;
			menu.LoadMenuW(IDR_MENU3);
			pMenu = menu.GetSubMenu(0);

			if (machine->get_submachine_count() == 0) {
				pMenu->EnableMenuItem(1, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
			}

			std::vector<group_info_ptr> vMoveto;
			int nItem = 1;
			vMoveto.push_back(nullptr); // placeholder
			subMenu.CreatePopupMenu();

			group_info_ptr rootGroup = group_manager::get_instance()->GetRootGroupInfo();
			if (machine->get_group_id() != rootGroup->get_id()) {
				subMenu.AppendMenuW(MF_STRING, nItem++, TR(IDS_STRING_GROUP_ROOT));
				vMoveto.push_back(rootGroup);
			}

			///////////////////////////////////////// define functions //////////////////////////////////////////////
			std::function<void(const group_info_ptr& groupInfo, const int machine_group_id, CMenu& subMenu, int& nItem, std::vector<group_info_ptr>& vMoveto)> iter_func;
			iter_func = [&iter_func](const group_info_ptr& groupInfo, const int machine_group_id, CMenu& subMenu, int& nItem, std::vector<group_info_ptr>& vMoveto) {
				group_info_list list;
				groupInfo->GetChildGroups(list);
				for (auto child_group : list) {
					vMoveto.push_back(child_group);
					if (child_group->get_child_group_count() > 0) {
						CMenu childMenu;
						childMenu.CreatePopupMenu();
						childMenu.AppendMenuW(MF_STRING, nItem++, child_group->get_formatted_group_name() + L"(" +
											  TR(IDS_STRING_SELF) + L")");
						iter_func(child_group, machine_group_id, childMenu, nItem, vMoveto);
						subMenu.InsertMenuW(childMenu.GetMenuItemCount(), MF_POPUP | MF_BYPOSITION,
											(UINT)childMenu.GetSafeHmenu(), child_group->get_formatted_group_name());
					} else {
						subMenu.AppendMenuW(MF_STRING, nItem++, child_group->get_formatted_group_name());
					}
				}
			};			

			/////////////////////////////////////  functions end  //////////////////////////////////////////////////////////

			iter_func(rootGroup, machine->get_group_id(), subMenu, nItem, vMoveto);
			CString sMoveTo;
			sMoveTo = TR(IDS_STRING_MOVE_TO);
			pMenu->InsertMenuW(2, MF_POPUP | MF_BYPOSITION,
							   (UINT)subMenu.GetSafeHmenu(), sMoveTo);

			GetCursorPos(&pt);
			DWORD ret = pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
											  pt.x, pt.y, this);
			JLOG(L"TrackPopupMenu ret %d\n", ret);

			if (1 <= ret && ret < vMoveto.size()) { // move to
				group_info_ptr dstGroup = vMoveto[ret];
				int old_group_id = machine->get_group_id();
				if (old_group_id == dstGroup->get_id()) {
					JLOG(L"same group, canceld move");
					return;
				}
				group_info_ptr oldGroup = group_manager::get_instance()->GetGroupInfo(old_group_id);
				HTREEITEM hOldParent = m_tree.GetParentItem(hItem);
				machine->execute_set_group_id(dstGroup->get_id());
				update_ancester_items_text(hOldParent);
				m_tidMap.erase(hItem);
				m_tree.DeleteItem(hItem);
				m_curselTreeItemMachine = m_tree.GetSelectedItem();
				HTREEITEM hGroup = GetTreeGroupItemByGroupInfo(dstGroup);
				HTREEITEM newhItem = m_tree.InsertItem(machine->get_formatted_name(), hGroup);
				update_ancester_items_text(hGroup);
				m_tidMap[newhItem] = tid;

				OnTvnSelchangedTree1(nullptr, nullptr);

				CString rec, smachine, sfield;
				smachine = TR(IDS_STRING_MACHINE);
				sfield = TR(IDS_STRING_GROUP);
				

				rec.Format(L"%s(" + TR(IDS_STRING_FM_ADEMCO_ID) + L") %s: %s --> %s", 
						   smachine, machine->get_ademco_id(),
							sfield, oldGroup->get_formatted_group_name(),
							dstGroup->get_formatted_group_name());
				history_record_manager::get_instance()->InsertRecord(machine->get_ademco_id(), 0, rec,
															time(nullptr), RECORD_LEVEL_USEREDIT);

			} else if (ID_GROUP_32788 == ret) { // 删除主机
				OnBnClickedButtonDeleteMachine();
			} else if (ID_32787 == ret) { // 管理服务期限
				if (machine->get_submachine_count() == 0) return;
				CMachineExpireManagerDlg dlg(this);
				zone_info_list zoneList;
				machine->GetAllZoneInfo(zoneList);
				std::list<alarm_machine_ptr> machineList;
				for (auto zoneInfo : zoneList) {
					alarm_machine_ptr subMachine = zoneInfo->GetSubMachineInfo();
					if (subMachine) {
						machineList.push_back(subMachine);
					}
				}
				dlg.m_bSubMachine = true;
				dlg.m_machine = machine;
				dlg.SetExpiredMachineList(machineList);
				dlg.DoModal();
				
			}
		} else { // group item
			CMenu menu, *pMenu, subMenu;
			menu.LoadMenuW(IDR_MENU2);
			pMenu = menu.GetSubMenu(0);
			std::vector<group_info_ptr> vMoveto;

			if (group->IsRootItem()) { 
				pMenu->EnableMenuItem(1, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
				pMenu->EnableMenuItem(2, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
			} else {
				// 添加子菜单项
				int nItem = 1;
				vMoveto.push_back(group); // placeholder
				subMenu.CreatePopupMenu();
				group_info_ptr rootGroup = group_manager::get_instance()->GetRootGroupInfo();
				if (group->get_parent_group() != rootGroup) { // 不能移动至父亲分组
					CString rootName;
					rootName = TR(IDS_STRING_GROUP_ROOT);
					subMenu.AppendMenuW(MF_STRING, nItem++, rootName);
					vMoveto.push_back(rootGroup);
				}

				// 生成 "移动至-->"子菜单的菜单项
				std::function<void(const group_info_ptr& groupInfo, const group_info_ptr& srcGroup, CMenu& subMenu, int& nItem, std::vector<group_info_ptr>& vMoveto)> iter_func;
				iter_func = [&iter_func](const group_info_ptr& groupInfo, const group_info_ptr& srcGroup, CMenu& subMenu, int& nItem, std::vector<group_info_ptr>& vMoveto) {
					group_info_list list;
					groupInfo->GetChildGroups(list);
					for (auto child_group : list) {
						if (child_group != srcGroup // 不能移动至同一个分组
							&& !srcGroup->IsDescendantGroup(child_group)) {  // 不能移动至自己的后代分组
							if (child_group->get_child_group_count() > 0) {
								CMenu childMenu;
								childMenu.CreatePopupMenu();
								if (child_group != srcGroup->get_parent_group()) { // 不能移动至父亲分组，你丫本来就在那
									childMenu.AppendMenuW(MF_STRING, nItem++, child_group->get_formatted_group_name() + L"(" +
														  TR(IDS_STRING_SELF) + L")");
									vMoveto.push_back(child_group);
								}
								
								iter_func(child_group, srcGroup, childMenu, nItem, vMoveto);

								subMenu.InsertMenuW(childMenu.GetMenuItemCount(), MF_POPUP | MF_BYPOSITION,
													(UINT)childMenu.GetSafeHmenu(), child_group->get_formatted_group_name());
							} else {
								if (child_group != srcGroup->get_parent_group()) { // 不能移动至父亲分组，你丫本来就在那
									subMenu.AppendMenuW(MF_STRING, nItem++, child_group->get_formatted_group_name());
									vMoveto.push_back(child_group);
								}
							}
						}
					}
				};
				iter_func(rootGroup, group, subMenu, nItem, vMoveto);
				CString sMoveTo;
				sMoveTo = TR(IDS_STRING_MOVE_TO);
				pMenu->InsertMenuW(4, MF_POPUP | MF_BYPOSITION,
								   (UINT)subMenu.GetSafeHmenu(), sMoveTo);
			}

			GetCursorPos(&pt);
			DWORD ret = pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
											pt.x, pt.y, this);
			JLOG(L"TrackPopupMenu ret %d\n", ret);

			if (1 <= ret && ret < vMoveto.size()) { // move to
				group_info_ptr dstGroup = vMoveto[ret];
				JLOG(L"move %s to %s\n", group->get_formatted_group_name(), dstGroup->get_formatted_group_name());
				if (group->ExecuteMove2Group(dstGroup)) {
					JLOG(L"move to succeed\n");
					CString rec, sgroup, sop;
					sgroup = TR(IDS_STRING_GROUP);
					sop = TR(IDS_STRING_GROUP_MOV);
					rec.Format(L"%s %s %s %s", sgroup, 
							   group->get_formatted_group_name(), sop, 
							   dstGroup->get_formatted_group_name());
					history_record_manager::get_instance()->InsertRecord(-1, -1, rec, time(nullptr),
																RECORD_LEVEL_USEREDIT);
					if (dstGroup->IsRootItem()) {
						m_tree.DeleteAllItems();
						ClearTree();
						HTREEITEM hRoot = m_tree.GetRootItem();
						CString txt;
						txt.Format(L"%s[%d]", dstGroup->get_formatted_group_name(), dstGroup->get_descendant_machine_count());
						HTREEITEM hRootGroup = m_tree.InsertItem(txt, hRoot);
						TreeItemDataPtr _tid = std::make_shared<TreeItemData>(dstGroup);
						m_tidMap[hRootGroup] = _tid;
						TraverseGroup(hRootGroup, dstGroup);
						m_curselTreeItemGroup = hRootGroup;
						m_tree.Expand(hRootGroup, TVE_EXPAND);
					} else {
						m_curselTreeItemGroup = nullptr;
						DeleteGroupItem(hItem);
						HTREEITEM hItemDst = GetTreeGroupItemByGroupInfo(dstGroup);
						HTREEITEM hItemDstParent = m_tree.GetParentItem(hItemDst);
						//DWORD dstData = m_tree.GetItemData(hItemDst);
						m_tree.DeleteItem(hItemDst);
						CString txt;
						txt.Format(L"%s[%d]", dstGroup->get_formatted_group_name(), dstGroup->get_descendant_machine_count());
						hItemDst = m_tree.InsertItem(txt, hItemDstParent);
						//m_tree.SetItemData(hItemDst, dstData);
						TreeItemDataPtr _tid = std::make_shared<TreeItemData>(dstGroup);
						m_tidMap[hItemDst] = _tid;
						TraverseGroup(hItemDst, dstGroup);
						//m_curselTreeItemGroup = hItemDst;
						//m_tree.Expand(hItemDst, TVE_EXPAND);
					}
				}
			} else if (ret == ID_GROUP_ADD) { // add sub group
				CInputContentDlg dlg(this);
				if (IDOK != dlg.DoModal() || dlg.m_value.IsEmpty()) return;
				JLOG(L"add sub group %s\n", dlg.m_value);
				group_info_ptr child_group = group->ExecuteAddChildGroup(dlg.m_value);
				if (!child_group)
					return;
				CString rec, sgroup, sop;
				sgroup = TR(IDS_STRING_GROUP);
				sop = TR(IDS_STRING_GROUP_ADD_SUB);
				rec.Format(L"%s %s %s %s", sgroup, 
						   group->get_formatted_group_name(), sop,
						   child_group->get_formatted_group_name());
				history_record_manager::get_instance()->InsertRecord(-1, -1, rec, time(nullptr),
															RECORD_LEVEL_USEREDIT);
				CString txt;
				txt.Format(L"%s[%d]", child_group->get_formatted_group_name(), child_group->get_descendant_machine_count());
				HTREEITEM  hItemNewGroup = m_tree.InsertItem(child_group->get_formatted_group_name(), hItem);
				TreeItemDataPtr _tid = std::make_shared<TreeItemData>(child_group);
				m_tidMap[hItemNewGroup] = _tid;
				m_tree.Expand(hItem, TVE_EXPAND);
				JLOG(L"group %s add sub group %s succeed\n", group->get_formatted_group_name(),
					 child_group->get_formatted_group_name());
			} else if (ret == ID_GROUP_DEL) { // delete group
				JLOG(L"deleting group %s\n", group->get_formatted_group_name());
				CString rec, sgroup, sop;
				sgroup = TR(IDS_STRING_GROUP);
				sop = TR(IDS_STRING_GROUP_DEL);
				rec.Format(L"%s %s %s", sgroup, group->get_formatted_group_name(), sop);
				history_record_manager::get_instance()->InsertRecord(-1, -1, rec, time(nullptr),
															RECORD_LEVEL_USEREDIT);
				group_info_ptr parentGroup = group->get_parent_group();
				if (parentGroup->ExecuteDeleteChildGroup(group)) {
					HTREEITEM hItemParent = m_tree.GetParentItem(hItem);
					ClearChildItems(hItemParent);
					TraverseGroup(hItemParent, parentGroup);
				}
			} else if (ret == ID_GROUP_RENAME) { // rename
				JLOG(L"renaming group %s\n", group->get_formatted_group_name());
				CInputContentDlg dlg(this);
				if (IDOK == dlg.DoModal()) {
					CString rec, sgroup, sop;
					sgroup = TR(IDS_STRING_GROUP);
					sop = TR(IDS_STRING_GROUP_REN);
					rec.Format(L"%s %s %s %s", sgroup, group->get_formatted_group_name(),
							   sop, dlg.m_value);
					history_record_manager::get_instance()->InsertRecord(-1, -1, rec, time(nullptr),
																RECORD_LEVEL_USEREDIT);
					if (group->ExecuteRename(dlg.m_value)) {
						JLOG(L"rename to %s\n", group->get_formatted_group_name());
						m_tree.SetItemText(hItem, group->get_formatted_group_name());
					}
				}
			} else if (ret == ID_GROUP_EXPIRE_MANAGE) {
				alarm_machine_list list;
				group->GetDescendantMachines(list);
				CMachineExpireManagerDlg dlg(this);
				dlg.SetExpiredMachineList(list);
				dlg.DoModal();
				if (dlg.m_bUpdatedMachineName) {
					ClearTree();
					InitTree();
				}
			} else if (ID_GROUP_CREATE_MACHINE == ret) {
				DoCreateMachine(group);
			}
		}
	}
}


void CMachineManagerDlg::DeleteGroupItem(HTREEITEM hItem)
{
	TreeItemDataPtr tid = m_tidMap[hItem];
	if (tid->_bGroup) {
		HTREEITEM hChild = m_tree.GetChildItem(hItem);
		while (hChild) {
			DeleteGroupItem(hChild);
			hChild = m_tree.GetNextSiblingItem(hChild);
		}
	}

	m_tidMap.erase(hItem);
	m_tree.DeleteItem(hItem);
}


HTREEITEM CMachineManagerDlg::GetTreeGroupItemByGroupInfo(group_info_ptr group)
{
	HTREEITEM hRoot = m_tree.GetRootItem();
	HTREEITEM hItem = GetTreeGroupItemByGroupInfoHelper(hRoot, group);
	return hItem;
}


HTREEITEM CMachineManagerDlg::GetTreeGroupItemByGroupInfoHelper(HTREEITEM hItem, 
																group_info_ptr group)
{
	AUTO_LOG_FUNCTION;
	
#ifdef _DEBUG
	CString txt = L"";
	txt = m_tree.GetItemText(hItem);
	JLOG(L"hItem %p %s\n", hItem, txt);
#endif
	
	TreeItemDataPtr tid = m_tidMap[hItem];
	if (tid->_bGroup && tid->_group == group) {
		return hItem;
	}

	HTREEITEM hChild = m_tree.GetChildItem(hItem);
	while (hChild) {
#ifdef _DEBUG
		txt = m_tree.GetItemText(hChild);
		JLOG(L"hChild %p %s\n", hItem, txt);
#endif
		TreeItemDataPtr _tid = m_tidMap[hChild];
		if (_tid->_bGroup) {
			if (_tid->_group == group) {
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


alarm_machine_ptr CMachineManagerDlg::GetCurEditingMachine()
{
	do {
		if (!m_curselTreeItemMachine)
			break;

		TreeItemDataPtr tid = m_tidMap[m_curselTreeItemMachine];
		if (!tid || tid->_bGroup)
			break;

		alarm_machine_ptr machine = tid->_machine;
		if (!machine)
			break;

		return machine;
	} while (0);

	return nullptr;
}


void CMachineManagerDlg::OnBnClickedButtonConfirmChange()
{}


bool CMachineManagerDlg::DoDeleteMachineReturnParentItem(HTREEITEM& hParent)
{
	alarm_machine_ptr machine = GetCurEditingMachine();
	if (!machine) return false;

	CString s, fm; fm = TR(IDS_STRING_FM_CONFIRM_DEL_MACHINE);
	s.Format(fm, machine->get_ademco_id(), machine->get_machine_name());
	if (IDOK != MessageBox(s, L"", MB_ICONQUESTION | MB_OKCANCEL))
		return false;

	auto mgr = alarm_machine_manager::get_instance();
	if (mgr->DeleteMachine(machine)) {
		TreeItemDataPtr tid = m_tidMap[m_curselTreeItemMachine];
		m_tidMap.erase(m_curselTreeItemMachine);
		hParent = m_tree.GetParentItem(m_curselTreeItemMachine);
		m_tree.DeleteItem(m_curselTreeItemMachine);
		m_curselTreeItemMachine = nullptr;
		m_tree.SelectItem(m_tree.GetRootItem());
		return true;
	}

	return false;
}

void CMachineManagerDlg::OnBnClickedButtonDeleteMachine()
{
	HTREEITEM hParent;
	if (DoDeleteMachineReturnParentItem(hParent)) {
		update_ancester_items_text(hParent);
	}
}


void CMachineManagerDlg::OnBnClickedButtonCreateMachine()
{
	DoCreateMachine(nullptr);
}


void CMachineManagerDlg::DoCreateMachine(const core::group_info_ptr& group)
{
	CAddMachineDlg dlg(this);
	dlg.group_info_ = group;
	if (IDOK != dlg.DoModal())
		return;

	alarm_machine_ptr machine = dlg.m_machine;
	auto mgr = alarm_machine_manager::get_instance();
	if (mgr->AddMachine(machine)) {
		group_info_ptr the_group = group_manager::get_instance()->GetGroupInfo(machine->get_group_id());
		the_group->AddChildMachine(machine);
		HTREEITEM hItem = GetTreeGroupItemByGroupInfo(the_group);
		if (hItem) {
			HTREEITEM hChild = m_tree.InsertItem(machine->get_formatted_name(), hItem);
			TreeItemDataPtr tid = std::make_shared<TreeItemData>(machine);
			m_tidMap[hChild] = tid;
			m_tree.Expand(hItem, TVE_EXPAND);
			m_tree.SelectItem(hChild);
		}
	}

}


void CMachineManagerDlg::OnCbnSelchangeComboBanned()
{
	int ndx = m_banned.GetCurSel();
	if (ndx != detail::COMBO_NDX_NO && ndx != detail::COMBO_NDX_YES) return;

	alarm_machine_ptr machine = GetCurEditingMachine();
	if (!machine) return;

	bool banned = ndx == detail::COMBO_NDX_YES;
	if (banned != machine->get_banned()) {
		bool ok = machine->execute_set_banned(banned);
		if (ok) {
			
		} else {
			m_banned.SetCurSel(banned ? detail::COMBO_NDX_NO : detail::COMBO_NDX_YES);
		}
	}
}


void CMachineManagerDlg::OnCbnSelchangeComboType()
{
	int ndx = m_type.GetCurSel(); if (ndx < 0) return;
	//if (ndx != detail::COMBO_NDX_MAP && ndx != detail::COMBO_NDX_VIDEO) return;
	//bool has_video = ndx == detail::COMBO_NDX_VIDEO;

	alarm_machine_ptr machine = GetCurEditingMachine();
	if (!machine) return;

	//if (has_video != machine->get_has_video()) {
	//	bool ok = machine->execute_set_has_video(has_video);
	//	if (ok) {
	//		CString rec, fm, stype;
	//		fm = TR(IDS_STRING_FM_TYPE);
	//		stype = TR(ndx == detail::COMBO_NDX_MAP ? IDS_STRING_TYPE_MAP : IDS_STRING_TYPE_VIDEO);
	//		rec.Format(fm, machine->get_ademco_id(), /*machine->GetDeviceIDW(), */stype);
	//		history_record_manager::get_instance()->InsertRecord(machine->get_ademco_id(),
	//													0, rec, time(nullptr),
	//													RECORD_LEVEL_USEREDIT);
	//	} else {
	//		m_type.SetCurSel(ndx == detail::COMBO_NDX_MAP ? detail::COMBO_NDX_VIDEO : detail::COMBO_NDX_MAP);
	//	}
	//}

	int data = static_cast<int>(m_type.GetItemData(ndx));
	if (data == 0xFFFFFFFF) {
		CInputContentDlg dlg(this);
		dlg.m_title = TR(IDS_STRING_INPUT_TYPE);
		int ret = dlg.DoModal();
		if (ret != IDOK) return;
		auto mgr = core::consumer_manager::get_instance();
		int id;
		if (mgr->execute_add_type(id, dlg.m_value)) {
			auto type = mgr->get_consumer_type_by_id(id);
			assert(type);
			if (type) {
				InitTypes();
				for (int i = 0; i < m_type.GetCount(); i++) {
					if (static_cast<int>(m_type.GetItemData(i)) == type->id) {
						m_type.SetCurSel(i);
						break;
					}
				}

				machine->get_consumer()->type = type;


			}
		}


	} else if( data != machine->get_consumer()->id) {
		auto tmp = std::make_shared<consumer>(*machine->get_consumer());
		
		auto mgr = consumer_manager::get_instance();
		auto type = mgr->get_consumer_type_by_id(data);
		tmp->type = type;
		if (mgr->execute_update_consumer(tmp)) {
			machine->set_consumer(tmp);
		} else {
			assert(0);
		}
	}
}


void CMachineManagerDlg::OnEnKillfocusEditName()
{
	alarm_machine_ptr machine = GetCurEditingMachine();
	if (!machine) return;

	CString txt;
	m_name.GetWindowTextW(txt);
	if (txt.Compare(machine->get_machine_name()) != 0) {
		machine->execute_set_alias(txt);
		m_tree.SetItemText(m_curselTreeItemMachine, machine->get_formatted_name());
	}
}


void CMachineManagerDlg::OnEnKillfocusEditContact()
{
	alarm_machine_ptr machine = GetCurEditingMachine();
	if (!machine) return;

	CString txt;
	m_contact.GetWindowTextW(txt);
	if (txt.Compare(machine->get_contact()) != 0) {
		machine->execute_set_contact(txt);
	}
}


void CMachineManagerDlg::OnEnKillfocusEditAddress()
{
	alarm_machine_ptr machine = GetCurEditingMachine();
	if (!machine) return;

	CString txt;
	m_addr.GetWindowTextW(txt);
	if (txt.Compare(machine->get_address()) != 0) {
		machine->execute_set_address(txt);
	}
}


void CMachineManagerDlg::OnEnKillfocusEditPhone()
{
	alarm_machine_ptr machine = GetCurEditingMachine();
	if (!machine) return;

	CString txt;
	m_phone.GetWindowTextW(txt);
	if (txt.Compare(machine->get_phone()) != 0) {
		machine->execute_set_phone(txt);
	}
}


void CMachineManagerDlg::OnEnKillfocusEditPhoneBk()
{
	alarm_machine_ptr machine = GetCurEditingMachine();
	if (!machine) return;

	CString txt;
	m_phone_bk.GetWindowTextW(txt);
	if (txt.Compare(machine->get_phone_bk()) != 0) {
		machine->execute_set_phone_bk(txt);
	}
}


void CMachineManagerDlg::OnCbnSelchangeComboGroup()
{
	/*alarm_machine_ptr machine = GetCurEditingMachine();
	if (!machine) return;

	int ndx = m_group.GetCurSel();
	if (ndx < 0) return;

	DWORD data = m_group.GetItemData(ndx);
	group_info_ptr group = core::group_manager::get_instance()->GetGroupInfo(data);
	if (!group) return;

	if (group->get_id() != machine->get_group_id()) {
		int old_group_id = machine->get_group_id();
		machine->execute_set_group_id(group->get_id());
		TreeItemDataPtr tid = m_tidMap[m_curselTreeItemMachine];
		m_tidMap.erase(m_curselTreeItemMachine);
		m_tree.DeleteItem(m_curselTreeItemMachine);
		m_curselTreeItemMachine = m_tree.GetSelectedItem();
		HTREEITEM hGroup = GetTreeGroupItemByGroupInfo(group);
		CString txt;
		txt.Format(L"%s(" + TR(IDS_STRING_FM_ADEMCO_ID) + L")", machine->get_alias(), machine->get_ademco_id());
		HTREEITEM hItem = m_tree.InsertItem(txt, hGroup);
		m_tidMap[hItem] = tid;

		OnTvnSelchangedTree1(nullptr, nullptr);

		CString rootName;
		rootName = TR(IDS_STRING_GROUP_ROOT);
		CString rec, smachine, sfield, sold_group, sgroup;
		smachine = TR(IDS_STRING_MACHINE);
		sfield = TR(IDS_STRING_GROUP);
		group_info_ptr oldGroup = group_manager::get_instance()->GetGroupInfo(old_group_id);

		if (oldGroup->IsRootItem()) { sold_group = rootName;
		} else { sold_group = oldGroup->get_name(); }

		if (group->IsRootItem()) {
			sgroup = rootName;
		} else { sgroup = group->get_name(); }

		rec.Format(L"%s(" + TR(IDS_STRING_FM_ADEMCO_ID) + L") %s: %s(%d) --> %s(%d)", smachine, machine->get_ademco_id(),
				   sfield, sold_group, oldGroup->get_id(),
				   sgroup, group->get_id());
		history_record_manager::get_instance()->InsertRecord(machine->get_ademco_id(), 0, rec,
													time(nullptr), RECORD_LEVEL_USEREDIT);
	}*/
}


void CMachineManagerDlg::OnBnClickedButtonExtend()
{
	alarm_machine_ptr machine = GetCurEditingMachine();
	if (!machine) return;


	CMenu menu, *sub;
	menu.LoadMenuW(IDR_MENU6);
	sub = menu.GetSubMenu(0); assert(sub); if (!sub) return;
	CRect rc;
	m_extend_expire.GetWindowRect(rc);
	DWORD ret = sub->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
									rc.right, rc.top, this);
	JLOG(L"TrackPopupMenu ret %d\n", ret);

	auto expire_time = machine->get_expire_time();

	switch (ret) {
	case ID_EXTEND_1_MONTH:
	{
		expire_time += std::chrono::hours(24) * 30;
	}
	break;

	case ID_EXTEND_2_MONTH:
	{
		expire_time += std::chrono::hours(24) * 30 * 2;
	}
	break;

	case ID_EXTEND_3_MONTH:
	{
		expire_time += std::chrono::hours(24) * 30 * 3;
	}
	break;

	case ID_EXTEND_6_MONTH:
	{
		expire_time += std::chrono::hours(24) * 30 * 6;
	}
	break;

	case ID_EXTEND_1_YEAR:
	{
		expire_time += std::chrono::hours(24) * 365;
	}
	break;

	case ID_EXTEND_2_YEAR:
	{
		expire_time += std::chrono::hours(24) * 730;
	}
	break;

	case ID_EXTEND_SET:
	{
		CExtendExpireTimeDlg dlg(this); if (IDOK != dlg.DoModal()) return;
		expire_time = dlg.m_dateTime;
	}
	break;

	default:
		return;
		break;
	}

	if (machine->execute_update_expire_time(expire_time)) {
		m_remind_time.SetWindowTextW(time_point_to_wstring(machine->get_consumer()->remind_time).c_str());
		m_expire_time.SetWindowTextW(time_point_to_wstring(expire_time).c_str());
	}
}


void CMachineManagerDlg::OnBnClickedButtonPickCoor()
{
	AUTO_LOG_FUNCTION;
	alarm_machine_ptr machine = GetCurEditingMachine();
	if (!machine) return;

	/*CBaiduMapViewerDlg dlg;
	dlg.m_machine = machine;
	dlg.DoModal();*/
	//if (g_baiduMapDlg && IsWindow(g_baiduMapDlg->m_hWnd)) {
	//	g_baiduMapDlg->ShowMap(machine);
	//}

	ipc::alarm_center_map_service::get_instance()->show_map(machine);

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
	alarm_machine_ptr machine = GetCurEditingMachine();
	if (!machine) return;
	BOOL b = m_chk_report_status.GetCheck();
	sms_config cfg = machine->get_sms_cfg();
	cfg.report_status = b ? true : false;
	machine->execute_set_sms_cfg(cfg);
}


void CMachineManagerDlg::OnBnClickedCheck2()
{
	AUTO_LOG_FUNCTION;
	alarm_machine_ptr machine = GetCurEditingMachine();
	if (!machine) return;
	BOOL b = m_chk_report_exception.GetCheck();
	sms_config cfg = machine->get_sms_cfg();
	cfg.report_exception = b ? true : false;
	machine->execute_set_sms_cfg(cfg);
}


void CMachineManagerDlg::OnBnClickedCheck3()
{
	AUTO_LOG_FUNCTION;
	alarm_machine_ptr machine = GetCurEditingMachine();
	if (!machine) return;
	BOOL b = m_chk_report_alarm.GetCheck();
	sms_config cfg = machine->get_sms_cfg();
	cfg.report_alarm = b ? true : false;
	machine->execute_set_sms_cfg(cfg);
}


void CMachineManagerDlg::OnBnClickedCheck4()
{
	AUTO_LOG_FUNCTION;
	alarm_machine_ptr machine = GetCurEditingMachine();
	if (!machine) return;
	BOOL b = m_chk_report_status_bk.GetCheck();
	sms_config cfg = machine->get_sms_cfg();
	cfg.report_status_bk = b ? true : false;
	machine->execute_set_sms_cfg(cfg);
}


void CMachineManagerDlg::OnBnClickedCheck5()
{
	AUTO_LOG_FUNCTION;
	alarm_machine_ptr machine = GetCurEditingMachine();
	if (!machine) return;
	BOOL b = m_chk_report_exception_bk.GetCheck();
	sms_config cfg = machine->get_sms_cfg();
	cfg.report_exception_bk = b ? true : false;
	machine->execute_set_sms_cfg(cfg);
}


void CMachineManagerDlg::OnBnClickedCheck6()
{
	AUTO_LOG_FUNCTION;
	alarm_machine_ptr machine = GetCurEditingMachine();
	if (!machine) return;
	BOOL b = m_chk_report_alarm_bk.GetCheck();
	sms_config cfg = machine->get_sms_cfg();
	cfg.report_alarm_bk = b ? true : false;
	machine->execute_set_sms_cfg(cfg);
}


void CMachineManagerDlg::OnTimer(UINT_PTR nIDEvent)
{
	CDialogEx::OnTimer(nIDEvent);

	if (detail::TIMER_ID_FLUSH_BAIDU_POS == nIDEvent) {
		alarm_machine_ptr machine = GetCurEditingMachine();
		if (!machine) return;
		auto coor = machine->get_coor();
		CString txt;
		txt.Format(L"%f", coor.x);
		m_x.SetWindowTextW(txt);
		txt.Format(L"%f", coor.y);
		m_y.SetWindowTextW(txt);
	} else if (detail::TIMER_ID_CALC_OWED_AMOUNT == nIDEvent) {
		alarm_machine_ptr machine = GetCurEditingMachine();
		if (!machine) return;

		CString s;
		m_receivable_amount.GetWindowTextW(s);
		int receivable_amount = _ttoi(s);
		m_paid_amount.GetWindowTextW(s);
		int paid_amount = _ttoi(s);

		auto a_consumer = machine->get_consumer();
		if (a_consumer->paid_amount == paid_amount  && a_consumer->receivable_amount == receivable_amount) {
			return;
		}

		auto tmp = std::make_shared<consumer>(*a_consumer);
		tmp->receivable_amount = receivable_amount;
		tmp->paid_amount = paid_amount;

		auto mgr = consumer_manager::get_instance();
		if (mgr->execute_update_consumer(tmp)) {
			s.Format(L"%d", tmp->get_owed_amount());
			m_owd_amount.SetWindowTextW(s);
			machine->set_consumer(tmp);
		}
	}
	
}


void CMachineManagerDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	

	CDialogEx::OnMouseMove(nFlags, point);
}


void CMachineManagerDlg::OnMouseHover(UINT nFlags, CPoint point)
{
	

	CDialogEx::OnMouseHover(nFlags, point);
}


BOOL CMachineManagerDlg::PreTranslateMessage(MSG* pMsg)
{
#ifdef _DEBUG
	if (pMsg->message == WM_MOUSEMOVE) {
		CPoint pt;
		GetCursorPos(&pt);
		m_tree.ScreenToClient(&pt);
		CRect rc;
		m_tree.GetClientRect(rc);
		if (!PtInRect(rc, pt)) {
			return FALSE;
		}
		m_tree.SetFocus();
		UINT flags;
		HTREEITEM hItem = m_tree.HitTest(pt, &flags);

		if (hItem == m_prevHotItem) return FALSE;
		m_prevHotItem = hItem;

		if (hItem && (TVHT_ONITEM & flags)) {
			auto tid = m_tidMap[hItem];
			if (tid) {
				if (tid->_bGroup) {
					JLOG(L"is group, %s, child_group_count %d", 
						 tid->_group->get_formatted_group_name(), 
						 tid->_group->get_child_group_count());
				} else {
					JLOG(L"is machine, %s", tid->_machine->get_formatted_name());
				}
			} else {
				JLOG(L"tid is null");
			}
		}
	}
#endif // _DEBUG

	

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CMachineManagerDlg::OnBnClickedButtonTypeManager()
{
	CConsumerTypeMgrDlg dlg;
	dlg.DoModal();

	InitTypes();

	m_curselTreeItemMachine = nullptr;
	OnTvnSelchangedTree1(nullptr, nullptr);
}


void CMachineManagerDlg::OnEnChangeEditReceivable()
{
	CString s;
	m_receivable_amount.GetWindowTextW(s);
	int receivable_amount = _ttoi(s);
	m_paid_amount.GetWindowTextW(s);
	int paid_amount = _ttoi(s);
	s.Format(L"%d", receivable_amount - paid_amount);
	m_owd_amount.SetWindowTextW(s);
}


void CMachineManagerDlg::OnEnChangeEditPaid()
{
	CString s;
	m_receivable_amount.GetWindowTextW(s);
	int receivable_amount = _ttoi(s);
	m_paid_amount.GetWindowTextW(s);
	int paid_amount = _ttoi(s);
	s.Format(L"%d", receivable_amount - paid_amount);
	m_owd_amount.SetWindowTextW(s);
}


void CMachineManagerDlg::OnBnClickedButtonSetRemindTime()
{
	alarm_machine_ptr machine = GetCurEditingMachine();
	if (!machine) return;

	CMenu menu, *sub;
	menu.LoadMenuW(IDR_MENU5);
	sub = menu.GetSubMenu(0); assert(sub); if (!sub) return;
	CRect rc;
	m_btn_set_remind.GetWindowRect(rc);
	DWORD ret = sub->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
									rc.right, rc.top, this);
	JLOG(L"TrackPopupMenu ret %d\n", ret);

	auto remind_time = machine->get_expire_time();

	switch (ret) {
	case ID_S_DAY:
		break;

	case ID_S_WEEK:
	{
		remind_time -= std::chrono::hours(24) * 7;
	}
		break;

	case ID_S_MONTH:
	{
		remind_time -= std::chrono::hours(24) * 30;
	}
		break;

	case ID_S_USER_SET:
	{
		CExtendExpireTimeDlg dlg(this); if (IDOK != dlg.DoModal()) return;
		remind_time = dlg.m_dateTime;
	}
		break;

	default:
		return;
		break;
	}
	
	auto a_consumer = machine->get_consumer();
	if (a_consumer->remind_time == remind_time) return;

	auto mgr = consumer_manager::get_instance();
	auto tmp = std::make_shared<consumer>(*a_consumer);
	tmp->remind_time = remind_time;
	if (mgr->execute_update_consumer(tmp)) {
		m_remind_time.SetWindowTextW(time_point_to_wstring(remind_time).c_str());
		machine->set_consumer(tmp);
	}
}
