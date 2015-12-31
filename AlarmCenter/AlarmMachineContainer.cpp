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
#include "AlarmMachineDlg.h"
#include "GroupInfo.h"
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
	static const int btnHeight = 40;
	static const int xGaps = 8;
	static const int yGaps = 18;

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

	m_clientSize.cx = rc.Width();
	m_clientSize.cy = rcBtn.bottom - rc.top + 150;
	m_scrollHelper->SetDisplaySize(m_clientSize.cx, m_clientSize.cy);
	return rcBtn;
}


BOOL CAlarmMachineContainerDlg::InsertMachine(core::CAlarmMachine* machine)
{
	AUTO_LOG_FUNCTION;
	for (auto btn : m_buttonList) {
		if (btn->GetMachine() == machine) {
			return TRUE;
		}
	}
	
	CString alias = machine->get_alias();
	if (alias.IsEmpty()) {
		alias.Format(L"%04d", machine->get_ademco_id());
	}
	CRect rcBtn = AssignBtnPosition(m_buttonList.size());
	auto btn = std::make_shared<gui::CButtonEx>(alias, rcBtn, this, IDC_BUTTON_MACHINE, machine);
	if (m_bShowing)
		btn->ShowWindow(SW_SHOW);
	else 
		btn->ShowWindow(SW_HIDE);

	m_buttonList.push_back(btn);

	// m_machineDlgList
	auto dlg = std::shared_ptr<CAlarmMachineDlg>(new CAlarmMachineDlg(this), [](CAlarmMachineDlg* p) {SAFEDELETEDLG(p); });
	dlg->SetMachineInfo(machine);
	//dlg->Create(IDD_DIALOG_MACHINE, this);
	m_machineDlgMap.insert(std::pair<int, CAlarmMachineDlgPtr>(reinterpret_cast<int>(machine), dlg));

	return 0;
}


void CAlarmMachineContainerDlg::DeleteMachine(core::CAlarmMachine* machine)
{
	bool bDeleted = FALSE;
	for (auto btn : m_buttonList) {
		core::CAlarmMachine* btn_machine = btn->GetMachine();
		if (btn_machine && btn_machine == machine) {
			m_buttonList.remove(btn);
			bDeleted = true;
			break;
		}
	}

	if (bDeleted) {
		auto iter = m_machineDlgMap.find(reinterpret_cast<int>(machine));
		if (iter != m_machineDlgMap.end()) {
			m_machineDlgMap.erase(iter);
		} else {
			bDeleted = false;
		}
	}

	if (bDeleted) {
		ReAssignBtnPosition();
	}
}


void CAlarmMachineContainerDlg::ReAssignBtnPosition()
{
	int ndx = 0;
	for (auto btn : m_buttonList) {
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
	m_machineDlgMap.clear();
	m_buttonList.clear();
	m_scrollHelper->SetDisplaySize(0, 0);
}


afx_msg LRESULT CAlarmMachineContainerDlg::OnBnclkedEx(WPARAM wParam, LPARAM lParam)
{
	int lr = static_cast<int>(wParam);
	core::CAlarmMachine* machine = reinterpret_cast<core::CAlarmMachine*>(lParam);

	if (lr == 0) { // left button clicked
		auto iter = m_machineDlgMap.find(reinterpret_cast<int>(machine));
		if (iter != m_machineDlgMap.end()) {
			CAlarmMachineDlgPtr dlg = iter->second;
			if (!IsWindow(dlg->m_hWnd)) {
				dlg->Create(IDD_DIALOG_MACHINE, this);
			}
			dlg->ShowWindow(SW_SHOW);
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


void CAlarmMachineContainerDlg::ShowMachinesOfGroup(core::CGroupInfo* group)
{
	using namespace core;
	ClearButtonList();
	m_curGroupInfo = group;

	if (group) {
		CAlarmMachineList list;
		group->GetDescendantMachines(list);
		for (auto machine : list) {
			InsertMachine(machine);
		}
	}

	ShowWindow(m_bShowing ? SW_SHOW : SW_HIDE);
}


void CAlarmMachineContainerDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	m_bShowing = bShow;

	for (auto btn : m_buttonList) {
		btn->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
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
