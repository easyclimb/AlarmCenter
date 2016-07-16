// AddMachineDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AddMachineDlg.h"
#include "afxdialogex.h"
#include "GroupInfo.h"
#include "AlarmMachineManager.h"
#include "AlarmMachine.h"
#include "ademco_func.h"
#include "AlarmCenter.h"
#include "InputGroupNameDlg.h"
#include "ExtendExpireTimeDlg.h"



namespace detail {
	const int COMBO_NDX_NO = 0;
	const int COMBO_NDX_YES = 1;

	const int COMBO_NDX_MAP = 0;
	const int COMBO_NDX_VIDEO = 1;

	int g_prevSelGroupNdx = 0;
	int g_prev_sel_type_ndx = 0;
};

using namespace detail;
using namespace core;

// CAddMachineDlg dialog

IMPLEMENT_DYNAMIC(CAddMachineDlg, CDialogEx)

CAddMachineDlg::CAddMachineDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CAddMachineDlg::IDD, pParent)
	, m_machine(nullptr)
{

}

CAddMachineDlg::~CAddMachineDlg()
{
}

void CAddMachineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_ademco_id);
	DDX_Control(pDX, IDC_COMBO1, m_banned);
	DDX_Control(pDX, IDC_COMBO_TYPE, m_type);
	//DDX_Control(pDX, IDC_EDIT2, m_device_id);
	DDX_Control(pDX, IDC_EDIT4, m_alias);
	DDX_Control(pDX, IDC_EDIT5, m_contact);
	DDX_Control(pDX, IDC_EDIT6, m_address);
	DDX_Control(pDX, IDC_EDIT7, m_phone);
	DDX_Control(pDX, IDC_EDIT8, m_phone_bk);
	//DDX_Control(pDX, IDC_COMBO2, m_group);
	DDX_Control(pDX, IDC_STATIC_NOTE2, m_note);
	DDX_Control(pDX, IDOK, m_ok);
	DDX_Control(pDX, IDC_EDIT9, m_expire_time);
	DDX_Control(pDX, IDC_COMBO3, m_cmb_ademco_id);
	DDX_Control(pDX, IDC_BUTTON_GROUP, m_btnGroup);
	DDX_Control(pDX, IDC_EDIT_GROUP, m_edit_group);
	DDX_Control(pDX, IDC_EDIT_RECEIVABLE, m_receivable_amount);
	DDX_Control(pDX, IDC_EDIT_PAID, m_paid_amount);
	DDX_Control(pDX, IDC_EDIT_OWED, m_owd_amount);
	DDX_Control(pDX, IDC_EDIT10, m_remind_time);
	DDX_Control(pDX, IDC_BUTTON_SET_ALARM, m_btn_set_remind);
	DDX_Control(pDX, IDC_BUTTON_SET_EXPIRE, m_btn_extend);
}


BEGIN_MESSAGE_MAP(CAddMachineDlg, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT1, &CAddMachineDlg::OnEnChangeEditAdemcoID)
	ON_BN_CLICKED(IDOK, &CAddMachineDlg::OnBnClickedOk)
	ON_EN_KILLFOCUS(IDC_EDIT2, &CAddMachineDlg::OnEnKillfocusEditDeviceID)
	ON_CBN_SELCHANGE(IDC_COMBO3, &CAddMachineDlg::OnCbnSelchangeCombo3)
	ON_CBN_EDITCHANGE(IDC_COMBO3, &CAddMachineDlg::OnCbnEditchangeCombo3)
	ON_BN_CLICKED(IDC_BUTTON_GROUP, &CAddMachineDlg::OnBnClickedButtonGroup)
	ON_CBN_SELCHANGE(IDC_COMBO_TYPE, &CAddMachineDlg::OnCbnSelchangeComboType)
	ON_EN_CHANGE(IDC_EDIT_RECEIVABLE, &CAddMachineDlg::OnEnChangeEditReceivable)
	ON_EN_CHANGE(IDC_EDIT_PAID, &CAddMachineDlg::OnEnChangeEditPaid)
	ON_BN_CLICKED(IDC_BUTTON_SET_EXPIRE, &CAddMachineDlg::OnBnClickedButtonSetExpire)
	ON_BN_CLICKED(IDC_BUTTON_SET_ALARM, &CAddMachineDlg::OnBnClickedButtonSetAlarm)
END_MESSAGE_MAP()


// CAddMachineDlg message handlers


BOOL CAddMachineDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString yes, no;
	yes = TR(IDS_STRING_YES);
	no = TR(IDS_STRING_NO);
	int combo_ndx = -1;
	combo_ndx = m_banned.InsertString(detail::COMBO_NDX_NO, no);
	ASSERT(combo_ndx == detail::COMBO_NDX_NO);
	combo_ndx = m_banned.InsertString(detail::COMBO_NDX_YES, yes);
	ASSERT(combo_ndx == detail::COMBO_NDX_YES);
	m_banned.SetCurSel(detail::COMBO_NDX_NO);

	/*CString normal, video;
	normal = TR(IDS_STRING_TYPE_MAP);
	video = TR(IDS_STRING_TYPE_VIDEO);
	combo_ndx = m_type.InsertString(detail::COMBO_NDX_MAP, normal);
	ASSERT(combo_ndx == detail::COMBO_NDX_MAP);
	combo_ndx = m_type.InsertString(detail::COMBO_NDX_VIDEO, video);
	ASSERT(combo_ndx == detail::COMBO_NDX_VIDEO);
	m_type.SetCurSel(detail::COMBO_NDX_MAP);*/

	InitTypes();

	group_info_list list;
	auto mgr = group_manager::get_instance();
	group_info_ptr rootGroup = mgr->GetRootGroupInfo();

	//int ndx = m_group.InsertString(0, rootGroup->get_name());
	//m_group.SetItemData(ndx, rootGroup->get_id());

	/*rootGroup->GetDescendantGroups(list);
	for (auto group : list) {
		ndx = m_group.AddString(group->get_name());
		m_group.SetItemData(ndx, group->get_id());
	}
	m_group.SetCurSel(detail::g_prevSelGroupNdx);*/

	auto group = group_info_;

	if (group) {
		g_prevSelGroupNdx = group->get_id();
	} else {
		group = rootGroup->GetGroupInfo(g_prevSelGroupNdx);
		if (!group) {
			g_prevSelGroupNdx = rootGroup->get_id();
			group = rootGroup;
		}		
	}

	m_edit_group.SetWindowTextW(group->get_formatted_group_name());

	COleDateTime expire_time = COleDateTime::GetCurrentTime();
	// add a year
	{
		COleDateTime t1(2001, 1, 1, 22, 15, 0);
		COleDateTime t2(2002, 1, 1, 22, 15, 0);
		COleDateTimeSpan ts = t2 - t1;
		ASSERT((t1 + ts) == t2);
		ASSERT((t2 - ts) == t1);
		expire_time += ts;
	}
	m_ole_remind_time = expire_time;
	m_expire_time.SetWindowTextW(expire_time.Format(L"%Y-%m-%d %H:%M:%S"));
	m_remind_time.SetWindowTextW(m_ole_remind_time.Format(L"%Y-%m-%d %H:%M:%S"));
#ifdef _DEBUG
	JLOG(expire_time.Format(L"%Y-%m-%d %H:%M:%S"));
#endif

	m_machine = std::make_shared<alarm_machine>();

	SYSTEMTIME st;
	expire_time.GetAsSystemTime(st);
	m_machine->set_expire_time(std::chrono::system_clock::from_time_t(CTime(st).GetTime()));
	

	//m_ok.EnableWindow(0);

	auto machine_mgr = alarm_machine_manager::get_instance();
	CString txt;
	int count = 0;
	int ndx = 0;
	for (int i = 0; i < MAX_MACHINE && count < 100; i++)  {
		if (machine_mgr->CheckIfMachineAdemcoIdCanUse(i)) {
			txt.Format(TR(IDS_STRING_FM_ADEMCO_ID), i);
			ndx = m_cmb_ademco_id.InsertString(-1, txt);
			m_cmb_ademco_id.SetItemData(ndx, i);
			count++;
		}
	}
	if (m_cmb_ademco_id.GetCount() > 0) {
		m_cmb_ademco_id.SetCurSel(0);
		int ademco_id = m_cmb_ademco_id.GetItemData(0);
		txt.Format(TR(IDS_STRING_FM_ADEMCO_ID), ademco_id);
		m_alias.SetWindowTextW(txt);
	}
	OnCbnSelchangeCombo3();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}



void CAddMachineDlg::InitTypes()
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

	
	m_type.SetCurSel(g_prev_sel_type_ndx);
}




void CAddMachineDlg::OnEnChangeEditAdemcoID()
{
	CheckAdemcoID();
}


void CAddMachineDlg::OnEnKillfocusEditDeviceID()
{
	CheckDeviceID();
}


bool CAddMachineDlg::CheckAdemcoID()
{
	int ademco_id = -1;
	int ndx = m_cmb_ademco_id.GetCurSel();
	if (ndx < 0) {
		CString num;
		m_cmb_ademco_id.GetWindowTextW(num);
		ademco_id = _ttoi(num);
	} else {
		ademco_id = m_cmb_ademco_id.GetItemData(ndx);
	}

	auto mgr = alarm_machine_manager::get_instance();
	if (!mgr->CheckIfMachineAdemcoIdCanUse(ademco_id)) {
		CString s; s = TR(IDS_STRING_ERR_AID);
		m_note.SetWindowTextW(s);
		m_ok.EnableWindow(0);
		return false;
	}

	m_note.SetWindowTextW(L"");
	m_ok.EnableWindow();
	m_machine->set_ademco_id(ademco_id);
	return true;
}


bool CAddMachineDlg::CheckDeviceID()
{
	return true;
}


void CAddMachineDlg::OnBnClickedOk()
{
	if (!CheckAdemcoID() || !CheckDeviceID())
		return;

	int ndx = m_banned.GetCurSel();
	if (ndx < 0)		return;
	bool banned = ndx == detail::COMBO_NDX_YES;
	m_machine->set_banned(banned);

	ndx = m_type.GetCurSel();
	if (ndx < 0)		return;
	//m_machine->set_has_video(ndx == detail::COMBO_NDX_VIDEO);
	auto mgr = core::consumer_manager::get_instance();
	auto type = mgr->get_consumer_type_by_id(m_type.GetItemData(ndx));
	if (!type) return;

	//auto a_consumer = std::make_shared<consumer>(-1, m_machine->get_ademco_id(), 0, type, 0, 0);
	CString s;
	m_receivable_amount.GetWindowTextW(s);
	int receivable_amount = _ttoi(s);
	m_paid_amount.GetWindowTextW(s);
	int paid_amount = _ttoi(s);
	SYSTEMTIME st;
	m_ole_remind_time.GetAsSystemTime(st);
	auto a_consumer = mgr->execute_add_consumer(m_machine->get_ademco_id(), 0, type, receivable_amount, paid_amount, 
												std::chrono::system_clock::from_time_t(CTime(st).GetTime())); assert(a_consumer);
	m_machine->set_consumer(a_consumer);

	m_alias.GetWindowTextW(s);
	m_machine->set_alias((LPCTSTR)s);

	m_contact.GetWindowTextW(s);
	m_machine->set_contact((LPCTSTR)s);

	m_address.GetWindowTextW(s);
	m_machine->set_address((LPCTSTR)s);

	m_phone.GetWindowTextW(s);
	m_machine->set_phone((LPCTSTR)s);

	m_phone_bk.GetWindowTextW(s);
	m_machine->set_phone_bk((LPCTSTR)s);

	//ndx = m_group.GetCurSel();
	//if (ndx < 0)	return;
	//detail::g_prevSelGroupNdx = ndx;
	//m_machine->set_group_id(m_group.GetItemData(ndx));
	m_machine->set_group_id(g_prevSelGroupNdx);

	m_machine->set_machine_type(MT_UNKNOWN);
	
	CDialogEx::OnOK();
}


void CAddMachineDlg::OnCancel()
{
	m_machine.reset();
	CDialogEx::OnCancel();
}




void CAddMachineDlg::OnCbnSelchangeCombo3()
{
	int ndx = m_cmb_ademco_id.GetCurSel();
	if (ndx < 0)
		return;

	//int ademco_id = m_cmb_ademco_id.GetItemData(ndx);
	//CString s, f; f = TR(IDS_STRING_HEX);
	//s.Format(L"%s%04X", f, ademco_id);
	//m_note.SetWindowTextW(s);
}


void CAddMachineDlg::OnCbnEditchangeCombo3()
{
	CString t; m_cmb_ademco_id.GetWindowTextW(t);
	if (t.IsEmpty())
		return;
	if (CheckAdemcoID()) {

	}
}


void CAddMachineDlg::OnBnClickedButtonGroup()
{
	CMenu menu; menu.CreatePopupMenu();
	std::vector<group_info_ptr> vMoveto;

	// 添加子菜单项
	int nItem = 1;
	vMoveto.push_back(nullptr); // placeholder
		
	group_info_ptr rootGroup = group_manager::get_instance()->GetRootGroupInfo();
	{ 
		CString rootName;
		rootName = TR(IDS_STRING_GROUP_ROOT);
		menu.AppendMenuW(MF_STRING, nItem++, rootName);
		vMoveto.push_back(rootGroup);
	}

	std::function<void(const group_info_ptr& groupInfo, CMenu& subMenu, int& nItem, std::vector<group_info_ptr>& vMoveto)> iter_func;
	iter_func = [&iter_func](const group_info_ptr& groupInfo, CMenu& subMenu, int& nItem, std::vector<group_info_ptr>& vMoveto) {
		group_info_list list;
		groupInfo->GetChildGroups(list);
		for (auto child_group : list) {
			if (child_group->get_child_group_count() > 0) {
				CMenu childMenu;
				childMenu.CreatePopupMenu();
				childMenu.AppendMenuW(MF_STRING, nItem++,
									  child_group->get_formatted_group_name() + L" (" + TR(IDS_STRING_SELF) + L")");
				vMoveto.push_back(child_group);

				iter_func(child_group, childMenu, nItem, vMoveto);
				subMenu.InsertMenuW(childMenu.GetMenuItemCount(), MF_POPUP | MF_BYPOSITION,
									(UINT)childMenu.GetSafeHmenu(), child_group->get_formatted_group_name());
			} else {
				subMenu.AppendMenuW(MF_STRING, nItem++, child_group->get_formatted_group_name());
				vMoveto.push_back(child_group);
			}
		}
	};

	iter_func(rootGroup, menu, nItem, vMoveto);

	CRect rc;
	m_btnGroup.GetWindowRect(rc);
	DWORD ret = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
									rc.right, rc.top, this);
	JLOG(L"TrackPopupMenu ret %d\n", ret);

	if (1 <= ret && ret < vMoveto.size()) { // group item clicked
		auto dst_group = vMoveto[ret];
		if (!dst_group) return;
		g_prevSelGroupNdx = dst_group->get_id();
		m_edit_group.SetWindowTextW(dst_group->get_formatted_group_name());
	}
}


void CAddMachineDlg::OnCbnSelchangeComboType()
{
	int ndx = m_type.GetCurSel(); if (ndx < 0) return;
	//if (ndx != detail::COMBO_NDX_MAP && ndx != detail::COMBO_NDX_VIDEO) return;
	//bool has_video = ndx == detail::COMBO_NDX_VIDEO;

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
			}
		}
	} else {
		g_prev_sel_type_ndx = ndx;
	}
}


void CAddMachineDlg::OnEnChangeEditReceivable()
{
	CalcOwdAmount();
}


void CAddMachineDlg::OnEnChangeEditPaid()
{
	CalcOwdAmount();
}


void CAddMachineDlg::CalcOwdAmount()
{
	CString s;
	m_receivable_amount.GetWindowTextW(s);
	int receivable_amount = _ttoi(s);
	m_paid_amount.GetWindowTextW(s);
	int paid_amount = _ttoi(s);
	s.Format(L"%d", receivable_amount - paid_amount);
	m_owd_amount.SetWindowTextW(s);
}


void CAddMachineDlg::OnBnClickedButtonSetExpire()
{
	CMenu menu, *sub;
	menu.LoadMenuW(IDR_MENU6);
	sub = menu.GetSubMenu(0); assert(sub); if (!sub) return;
	CRect rc;
	m_btn_extend.GetWindowRect(rc);
	DWORD ret = sub->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
									rc.right, rc.top, this);
	JLOG(L"TrackPopupMenu ret %d\n", ret);

	auto expire_time = m_machine->get_expire_time();

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

	m_expire_time.SetWindowTextW(time_point_to_wstring(expire_time).c_str());
	m_machine->set_expire_time(expire_time);
}


void CAddMachineDlg::OnBnClickedButtonSetAlarm()
{
	CMenu menu, *sub;
	menu.LoadMenuW(IDR_MENU5);
	sub = menu.GetSubMenu(0); assert(sub); if (!sub) return;
	CRect rc;
	m_btn_set_remind.GetWindowRect(rc);
	DWORD ret = sub->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
									rc.right, rc.top, this);
	JLOG(L"TrackPopupMenu ret %d\n", ret);

	auto remind_time = m_machine->get_expire_time();

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

	m_ole_remind_time = std::chrono::system_clock::to_time_t(remind_time);
	m_remind_time.SetWindowTextW(m_ole_remind_time.Format(L"%Y-%m-%d %H:%M:%S"));
}
