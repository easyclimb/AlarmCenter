// AlarmMachineContainer.cpp : implementation file
//

#include "stdafx.h"
#include "AppResource.h"
#include "AlarmMachineContainer.h"
#include "afxdialogex.h"
#include "ScrollHelper.h"
#include "AlarmMachine.h"
#include "./imagin/Timer.h"
#include "ButtonEx.h"
#include "AlarmMachineManager.h"
#include "AlarmMachineDlg.h"
#include "GroupInfo.h"
#include "ZoneInfo.h"
#include <set>
#include <algorithm>

using namespace gui;


//static void __stdcall on_ademco_event(void* data, int zone, int ademco_event)
//{
//	gui::CButtonEx* btn = reinterpret_cast<gui::CButtonEx*>(data); ASSERT(btn);
//	btn->OnAdemcoEvent(zone, ademco_event);
//}


//CAlarmMachineDlg* CAlarmMachineContainerDlg::m_machineDlg = nullptr;
// CAlarmMachineContainerDlg dialog

IMPLEMENT_DYNAMIC(CAlarmMachineContainerDlg, CDialogEx)

CAlarmMachineContainerDlg::CAlarmMachineContainerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CAlarmMachineContainerDlg::IDD, pParent)
	, m_scrollHelper()
	//, m_machineDlg(nullptr)
	, m_curGroupInfo(nullptr)
	, m_bShowing(FALSE)
	, m_bFocused(FALSE)
{
	m_scrollHelper = std::make_unique<gui::control::CScrollHelper>();
	m_scrollHelper->AttachWnd(this);
}

CAlarmMachineContainerDlg::~CAlarmMachineContainerDlg()
{
}

void CAlarmMachineContainerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAlarmMachineContainerDlg, CDialogEx)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_BNCLKEDEX, &CAlarmMachineContainerDlg::OnBnclkedEx)
	ON_WM_CTLCOLOR()
	ON_WM_CLOSE()
	ON_WM_SHOWWINDOW()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_MESSAGE(WM_ADEMCOEVENT, &CAlarmMachineContainerDlg::OnMsgAdemcoevent)
END_MESSAGE_MAP()


// CAlarmMachineContainerDlg message handlers


void CAlarmMachineContainerDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	m_scrollHelper->OnHScroll(nSBCode, nPos, pScrollBar);
	//CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CAlarmMachineContainerDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	m_scrollHelper->OnVScroll(nSBCode, nPos, pScrollBar);
	//CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
}


BOOL CAlarmMachineContainerDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	BOOL wasScrolled = m_scrollHelper->OnMouseWheel(nFlags,	zDelta, pt);
	return wasScrolled;
	//return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}


void CAlarmMachineContainerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	m_scrollHelper->OnSize(nType, cx, cy);
}


CRect CAlarmMachineContainerDlg::AssignBtnPosition(int ndx)
{
	static const int btnWidth = 260;
	static const int btnHeight = 70;
	static const int xGaps = 8;
	static const int yGaps = 18;

	CRect rc;
	GetClientRect(rc);
	ClientToScreen(rc);
	rc.top += 10;
	rc.right -= ::GetSystemMetrics(SM_CXVSCROLL);
	CSize sz(rc.Width(), rc.Height());

	int nX = sz.cx / (btnWidth + xGaps);

	int x = ndx % nX;
	int y = ndx / nX;

	CRect rcBtn(rc);
	rcBtn.left = rc.left + xGaps + (btnWidth + xGaps) * x;
	rcBtn.right = rcBtn.left + btnWidth;
	rcBtn.top = rc.top + (btnHeight + yGaps) * y;
	rcBtn.bottom = rcBtn.top + btnHeight;
	ScreenToClient(rcBtn);

	m_clientSize.cx = rc.Width();
	m_clientSize.cy = rcBtn.bottom - rc.top + 150;
	m_scrollHelper->SetDisplaySize(m_clientSize.cx, m_clientSize.cy);
	return rcBtn;
}


BOOL CAlarmMachineContainerDlg::InsertMachine(const core::alarm_machine_ptr& machine, int ndx, bool need_check_dup)
{
	AUTO_LOG_FUNCTION;
	if (need_check_dup) {
		auto iter = m_machineDlgMap.find(machine);
		if (iter != m_machineDlgMap.end()) {
			return TRUE;
		}
	}

	if (ndx == -1) {
		ndx = m_machineDlgMap.size();
	}

	CString alias = machine->get_machine_name();
	if (alias.IsEmpty()) {
		alias.Format(L"%s%06d", GetStringFromAppResource(IDS_STRING_MACHINE), machine->get_ademco_id());
	}

	CRect rcBtn = AssignBtnPosition(ndx);
	auto btn = std::make_shared<gui::CButtonEx>(alias, rcBtn, this, IDC_BUTTON_MACHINE, machine);

	if (m_bShowing)
		btn->ShowButton(SW_SHOW);
	else 
		btn->ShowButton(SW_HIDE);

	auto dlg = std::shared_ptr<CAlarmMachineDlg>(new CAlarmMachineDlg(this));
	dlg->SetMachineInfo(machine);
	m_machineDlgMap[machine] = MachineButtonAndDialog(CButtonExWithShowOrHide(true, btn), dlg);

	return 0;
}


BOOL CAlarmMachineContainerDlg::Reset(core::alarm_machine_list& list)
{
	ClearButtonList();

	for (auto machine : list) {
		CString alias = machine->get_machine_name();
		if (alias.IsEmpty()) {
			alias.Format(L"%s%06d", GetStringFromAppResource(IDS_STRING_MACHINE), machine->get_ademco_id());
		}
		CRect rcBtn = AssignBtnPosition(m_machineDlgMap.size());
		auto btn = std::make_shared<gui::CButtonEx>(alias, rcBtn, this, IDC_BUTTON_MACHINE, machine);
		if (m_bShowing)
			btn->ShowButton(SW_SHOW);
		else
			btn->ShowButton(SW_HIDE);

		auto dlg = std::shared_ptr<CAlarmMachineDlg>(new CAlarmMachineDlg(this));
		dlg->SetMachineInfo(machine);
		m_machineDlgMap[machine] = MachineButtonAndDialog(CButtonExWithShowOrHide(true, btn), dlg);
	}
	return TRUE;
}


void CAlarmMachineContainerDlg::DeleteMachine(const core::alarm_machine_ptr& machine)
{
	//bool bDeleted = FALSE;
	/*for (auto btn : m_buttonList) {
		core::alarm_machine_ptr btn_machine = btn->GetMachine();
		if (btn_machine && btn_machine == machine) {
			m_buttonList.remove(btn);
			bDeleted = true;
			break;
		}
	}*/

	//if (bDeleted) {
		auto iter = m_machineDlgMap.find(machine);
		if (iter != m_machineDlgMap.end()) {
			iter = m_machineDlgMap.erase(iter);
		}/* else {
			bDeleted = false;
		}*/
		int distance = std::distance(m_machineDlgMap.begin(), iter);
		while (iter != m_machineDlgMap.end()) {
			auto rc = AssignBtnPosition(distance++);
			iter->second.first.second->MoveWindow(rc);
			iter++;
		}
	//}

	/*if (bDeleted) {
		ReAssignBtnPosition();
	}*/
}


void CAlarmMachineContainerDlg::ReAssignBtnPosition()
{
	//int ndx = 0;
	/*for (auto btn : m_buttonList) {
		CRect rc = AssignBtnPosition(ndx++);
		btn->MoveWindow(rc);
	}

	for (auto iter : m_machineDlgMap) {

	}*/
}


BOOL CAlarmMachineContainerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CAlarmMachineContainerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	ClearButtonList();
}


void CAlarmMachineContainerDlg::ClearButtonList()
{
	for (auto iter : m_machineDlgMap) {
		//iter.second->DestroyWindow();
		//iter.second->PostMessageW(WM_DESTROY);
		MachineButtonAndDialog pair = iter.second;
		auto hWnd = pair.second->m_hWnd;
		if (hWnd) {
			pair.second->SendMessageW(WM_DESTROY);
		}
	}
	m_machineDlgMap.clear();
	//m_buttonList.clear();
	m_scrollHelper->SetDisplaySize(0, 0);
}


afx_msg LRESULT CAlarmMachineContainerDlg::OnBnclkedEx(WPARAM wParam, LPARAM lParam)
{
	int lr = static_cast<int>(wParam);
	core::alarm_machine_ptr machine;
	if (m_bSubmachineContainer) {
		auto zone = m_machine->GetZone(lParam);
		if(zone)
			machine = zone->GetSubMachineInfo();
	} else {
		machine = core::alarm_machine_manager::GetInstance()->GetMachine(lParam);
	}

	if (lr == 0 && machine) { // left button clicked
		auto iter = m_machineDlgMap.find(machine);
		if (iter != m_machineDlgMap.end()) {
			MachineButtonAndDialog pair = iter->second;
			CAlarmMachineDlgPtr dlg = pair.second;
			if (!IsWindow(dlg->m_hWnd)) {
				dlg->Create(IDD_DIALOG_MACHINE, this);
			}
			dlg->ShowWindow(SW_SHOW);
			//dlg->Invalidate();
			//Invalidate(0);
		}

	} else if (lr == 1) { // right button clicked

	}
	return 0;
}


HBRUSH CAlarmMachineContainerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	return hbr;
}


void CAlarmMachineContainerDlg::OnClose()
{
	return;
}


void CAlarmMachineContainerDlg::ShowMachinesOfGroup(const core::group_info_ptr& group)
{
	using namespace core;

	if (!group)
		return;

	//do {
	//	if (!group) {
	//		//m_curGroupInfo = group;
	//		break;
	//	}

	//	//if (!m_curGroupInfo || (group->get_id() != m_curGroupInfo->get_id())) {
	//	//	m_curMachineList.clear();
	//	//	group->GetDescendantMachines(m_curMachineList/*, group->get_cur_filter_way()*/);
	//	//	m_curGroupInfo = group;
	//	//	break;
	//	//}

	//	

	//	//alarm_machine_list list;
	//	//group->GetDescendantMachines(list/*, group->get_cur_filter_way()*/);

	//	//auto lists_have_diff_content = [](alarm_machine_list & l1, alarm_machine_list& l2) {
	//	//	/*alarm_machine_list both;
	//	//	std::set_intersection(l1.begin(), l1.end(), l2.begin(), l2.end(), std::back_inserter(both));
	//	//	return both.size() != l1.size() || both.size() != l2.size();*/
	//	//	if (l1.size() != l2.size()) return true;

	//	//	auto iter1 = l1.begin();
	//	//	auto iter2 = l2.begin();
	//	//	while (iter1 != l1.end() && iter2 != l2.end()) {
	//	//		if (*iter1++ != *iter2++) {
	//	//			return true;
	//	//		}
	//	//	}
	//	//	return false;
	//	//};

	//	//if (lists_have_diff_content(list, m_curMachineList)) {
	//	//	m_curMachineList = list;
	//	//	m_curGroupInfo = group;
	//	//	break;
	//	//}

	//	//if (m_curGroupInfo == group) {
	//	//	Refresh();
	//	//	return;
	//	//} else {
	//	//	group->GetDescendantMachines(m_curMachineList/*, group->get_cur_filter_way()*/);
	//	//	break;
	//	//}

	//	return;
	//} while (false);

	////ClearButtonList();

	

	//Refresh();

	auto iter = m_groupMap.find(group);
	if (iter == m_groupMap.end()) {

		// #1. hide other btns
		for (auto& pair : m_machineDlgMap) {
			pair.second.first.first = false;
			pair.second.first.second->ShowButton(SW_HIDE);
		}

		// #2. create new btns 
		alarm_machine_list list;
		group->GetDescendantMachines(list);
		m_groupMap[group] = list;

		int ndx = 0;
		for (auto machine : list) {
			InsertMachine(machine, ndx++, false);
		}

		m_curGroupInfo = group;

	} else {
		
		m_curGroupInfo = iter->first;
		Refresh();

	}

	Invalidate(0);
	ShowWindow(m_bShowing ? SW_SHOW : SW_HIDE);
}


void CAlarmMachineContainerDlg::Refresh()
{
	AUTO_LOG_FUNCTION;
	using namespace core;
	if (!m_curGroupInfo) return;
	auto& list = m_groupMap[m_curGroupInfo];
	list.clear();
	//m_curGroupInfo->SortDescendantMachines(group_manager::GetInstance()->get_cur_sort_machine_way());
	m_curGroupInfo->GetFilteredDescendantMachines(list, m_curGroupInfo->get_cur_filter_way());
	int ndx = 0;
	auto tmp_map = m_machineDlgMap;
	//auto tmp_list = list;
	for (auto machine : list) {
		auto iter = m_machineDlgMap.find(machine);
		if (iter == m_machineDlgMap.end()) {
			InsertMachine(machine, ndx, false);
		} else { 
			auto& pair = iter->second;
			auto rc = AssignBtnPosition(ndx);
			pair.first.first = true;
			pair.first.second->MoveWindow(rc);
			pair.first.second->ShowButton(SW_SHOW);
			tmp_map.erase(machine);
			//tmp_list.remove(machine);
		}
		ndx++;
	}

	//for (auto machine : tmp_list) {
	//	InsertMachine(machine, ndx++, false);
	//}

	for (auto iter : tmp_map) {
		auto& real_iter = m_machineDlgMap[iter.first];
		real_iter.first.first = false;
		real_iter.first.second->ShowButton(SW_HIDE);
	}

}


void CAlarmMachineContainerDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	m_bShowing = bShow;

	/*for (auto btn : m_buttonList) {
		btn->ShowButton(bShow ? SW_SHOW : SW_HIDE);
	}*/

	for (auto iter : m_machineDlgMap) {
		auto btn_with_show_or_hide = iter.second.first;

		if (btn_with_show_or_hide.first) {
			btn_with_show_or_hide.second->ShowButton(bShow ? SW_SHOW : SW_HIDE);
		} else {

		}
		//iter.second.first->ShowButton(bShow ? SW_SHOW : SW_HIDE);
	}
}


void CAlarmMachineContainerDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!m_bFocused) {
		m_bFocused = TRUE;
		SetFocus();
	}

	CDialogEx::OnMouseMove(nFlags, point);
}


void CAlarmMachineContainerDlg::OnMouseLeave()
{
	if (m_bFocused) {
		m_bFocused = FALSE;
	}
	CDialogEx::OnMouseLeave();
}


afx_msg LRESULT CAlarmMachineContainerDlg::OnMsgAdemcoevent(WPARAM wParam, LPARAM /*lParam*/)
{
	int ademco_id = static_cast<int>(wParam);
	auto mgr = core::alarm_machine_manager::GetInstance();
	auto machine = mgr->GetMachine(ademco_id);
	if (machine) {
		auto group_id = machine->get_group_id();
		auto group = core::group_manager::GetInstance()->GetGroupInfo(group_id);
		if (group && m_curGroupInfo == group) {
			Refresh();
		}
		
	}
	return 0;
}
