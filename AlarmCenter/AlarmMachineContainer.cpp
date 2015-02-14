// AlarmMachineContainer.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AlarmMachineContainer.h"
#include "afxdialogex.h"
#include "ScrollHelper.h"
#include "AlarmMachine.h"
#include "./imagin/Timer.h"
#include "ButtonEx.h"
#include "AlarmMachineManager.h"
#include "AlarmmachineDlg.h"
#include "GroupInfo.h"
using namespace gui;


//static void _stdcall on_ademco_event(void* data, int zone, int ademco_event)
//{
//	gui::CButtonEx* btn = reinterpret_cast<gui::CButtonEx*>(data); ASSERT(btn);
//	btn->OnAdemcoEvent(zone, ademco_event);
//}



// CAlarmMachineContainerDlg dialog

IMPLEMENT_DYNAMIC(CAlarmMachineContainerDlg, CDialogEx)

CAlarmMachineContainerDlg::CAlarmMachineContainerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAlarmMachineContainerDlg::IDD, pParent)
	, m_scrollHelper(NULL)
	, m_machineDlg(NULL)
	, m_curGroupInfo(NULL)
	, m_bShowing(FALSE)
{
	m_scrollHelper = new gui::control::CScrollHelper();
	//m_scrollHelper->AttachWnd(this);
}

CAlarmMachineContainerDlg::~CAlarmMachineContainerDlg()
{
	delete m_scrollHelper;
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
END_MESSAGE_MAP()


// CAlarmMachineContainerDlg message handlers


void CAlarmMachineContainerDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	//m_scrollHelper->OnHScroll(nSBCode, nPos, pScrollBar);
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CAlarmMachineContainerDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	//m_scrollHelper->OnVScroll(nSBCode, nPos, pScrollBar);
	CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
}


BOOL CAlarmMachineContainerDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	//BOOL wasScrolled = m_scrollHelper->OnMouseWheel(nFlags,	zDelta, pt);
	//return wasScrolled;
	return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}


void CAlarmMachineContainerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	//m_scrollHelper->OnSize(nType, cx, cy);
}


CRect CAlarmMachineContainerDlg::AssignBtnPosition(int ndx)
{
	static const int btnWidth = 200;
	static const int btnHeight = 40;
	static const int xGaps = 15;
	static const int yGaps = 15;

	CRect rc;
	GetClientRect(rc);
	ClientToScreen(rc);
	rc.top += 10;
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

	return rcBtn;
}


BOOL CAlarmMachineContainerDlg::InsertMachine(core::CAlarmMachine* machine)
{
	LOG_FUNCTION_AUTO;
	CString alias = machine->get_alias();
	if (alias.IsEmpty()) {
		alias.Format(L"%04d", machine->get_ademco_id());
	}
	SetWindowText(alias);

	CRect rcBtn = AssignBtnPosition(m_buttonList.size());

	gui::CButtonEx* btn = new gui::CButtonEx(alias, rcBtn, this, IDC_BUTTON_MACHINE,
											 machine);
	if (m_bShowing)
		btn->ShowWindow(SW_SHOW);
	else 
		btn->ShowWindow(SW_HIDE);

	m_buttonList.push_back(btn);

	return 0;
}


void CAlarmMachineContainerDlg::DeleteMachine(core::CAlarmMachine* machine)
{
	BOOL bDeleted = FALSE;
	std::list<CButtonEx*>::iterator iter = m_buttonList.begin();
	while (iter != m_buttonList.end()) {
		CButtonEx* btn = *iter;
		core::CAlarmMachine* btn_machine = btn->GetMachine();
		if (btn_machine && btn_machine == machine) {
			delete btn;
			m_buttonList.erase(iter);
			bDeleted = TRUE;
			break;
		}
		iter++;
	}

	if (bDeleted) {
		ReAssignBtnPosition();
	}
}


void CAlarmMachineContainerDlg::ReAssignBtnPosition()
{
	int ndx = 0;
	std::list<CButtonEx*>::iterator iter = m_buttonList.begin();
	while (iter != m_buttonList.end()) {
		CButtonEx* btn = *iter++;
		CRect rc = AssignBtnPosition(ndx++);
		btn->MoveWindow(rc);
	}
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
	if (m_machineDlg != NULL) {
		if (IsWindow(m_machineDlg->GetSafeHwnd())) {
			m_machineDlg->DestroyWindow();
		}
		delete m_machineDlg;
		m_machineDlg = NULL;
	}

	std::list<gui::CButtonEx*>::iterator iter = m_buttonList.begin();
	while (iter != m_buttonList.end()) {
		gui::CButtonEx* btn = *iter++;
		delete btn;
	}
	m_buttonList.clear();
}


afx_msg LRESULT CAlarmMachineContainerDlg::OnBnclkedEx(WPARAM wParam, LPARAM lParam)
{
	int lr = static_cast<int>(wParam);
	//int ademco_id = static_cast<int>(lParam);
	//core::CAlarmMachine* machine = NULL;
	core::CAlarmMachine* machine = reinterpret_cast<core::CAlarmMachine*>(lParam);

	/*if (!core::CAlarmMachineManager::GetInstance()->GetMachine(ademco_id, machine) 
		|| (machine == NULL)) {
		return 0;
	}*/

	if (lr == 0) { // left button clicked
		if (m_machineDlg == NULL) {
			m_machineDlg = new CAlarmMachineDlg(this);
		}

		int curShowingAdemcoID = m_machineDlg->GetAdemcoID();

		if (curShowingAdemcoID != machine->get_ademco_id()) {
			if (IsWindow(m_machineDlg->GetSafeHwnd())) {
				m_machineDlg->DestroyWindow();
			}
			m_machineDlg->SetMachineInfo(machine);
		}

		if (!IsWindow(m_machineDlg->m_hWnd)) {
			m_machineDlg->Create(IDD_DIALOG_MACHINE, this);
		}

		m_machineDlg->ShowWindow(SW_SHOW);

	} else if (lr == 1) { // right button clicked

	}
	return 0;
}




HBRUSH CAlarmMachineContainerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here

	// TODO:  Return a different brush if the default is not desired
	return hbr;
}


void CAlarmMachineContainerDlg::OnClose()
{
	return;
}


void CAlarmMachineContainerDlg::ShowMachinesOfGroup(core::CGroupInfo* group)
{
	using namespace core;
	assert(group);

	//if (m_curGroupInfo && (group->get_id() == m_curGroupInfo->get_id()))
	// 	return;

	ClearButtonList();
	m_curGroupInfo = group;

	CAlarmMachineList list;
	group->GetDescendantMachines(list);
	CAlarmMachineListIter iter = list.begin();
	while (iter != list.end()) {
		CAlarmMachine* machine = *iter++;
		InsertMachine(machine);
	}

}


void CAlarmMachineContainerDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	m_bShowing = bShow;

	std::list<gui::CButtonEx*>::iterator iter = m_buttonList.begin();
	while (iter != m_buttonList.end()) {
		gui::CButtonEx* btn = *iter++;
		btn->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
	}
}
