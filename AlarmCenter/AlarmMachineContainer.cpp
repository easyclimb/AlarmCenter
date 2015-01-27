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

HICON CAlarmMachineContainerDlg::m_hIconArm			= NULL;
HICON CAlarmMachineContainerDlg::m_hIconDisarm		= NULL;
HICON CAlarmMachineContainerDlg::m_hIconNetOk		= NULL;
HICON CAlarmMachineContainerDlg::m_hIconNetFailed	= NULL;



static void _stdcall OnMachineStatusChange(void* data, core::MachineStatus status)
{
	gui::CButtonEx* btn = reinterpret_cast<gui::CButtonEx*>(data); ASSERT(btn);
	btn->OnStatusChange(status);
}

// CAlarmMachineContainerDlg dialog

IMPLEMENT_DYNAMIC(CAlarmMachineContainerDlg, CDialogEx)

CAlarmMachineContainerDlg::CAlarmMachineContainerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAlarmMachineContainerDlg::IDD, pParent)
	, m_scrollHelper(NULL)
	, m_machineDlg(NULL)
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


BOOL CAlarmMachineContainerDlg::InsertMachine(core::CAlarmMachine* machine)
{
	static const int btnWidth = 200;
	static const int btnHeight = 45;
	static const int xGaps = 25;
	static const int yGaps = 15;

	CRect rc;
	GetClientRect(rc);
	ClientToScreen(rc);
	rc.top += 10;
	CSize sz(rc.Width(), rc.Height());

	int nX = sz.cx / (btnWidth + xGaps);
	//int nY = sz.cy / (btnHeight + yGaps) - 1;

	CRect rcBtn(rc);
	//total = nX * nY;
	//total = 46;
	//total = min(total, 46);
	srand((unsigned int)time(NULL));

	int curNdx = (int)m_machineList.size();
	int x = curNdx % nX;
	int y = curNdx / nX;

	rcBtn.left = rc.left + xGaps + (btnWidth + xGaps) * x;
	rcBtn.right = rcBtn.left + btnWidth;
	rcBtn.top = rc.top + (btnHeight + yGaps) * y;
	rcBtn.bottom = rcBtn.top + btnHeight;
	ScreenToClient(rcBtn);

	wchar_t name[1024] = { 0 }; 
	const wchar_t* alias = machine->GetAlias();
	if (alias && wcslen(alias) > 0) {
		wcscpy_s(name, alias);
	} else {
		swprintf_s(name, L"%04d", machine->get_ademco_id());
	}

	gui::CButtonEx* btn = new gui::CButtonEx(name, rcBtn, this, IDC_BUTTON1, 
											 machine->get_ademco_id());
	btn->ShowWindow(SW_SHOW);
	machine->RegisterObserver(btn, OnMachineStatusChange);
	m_machineList.push_back(btn);

	return 0;
}


BOOL CAlarmMachineContainerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_hIconArm = (HICON)::LoadImage(AfxGetApp()->m_hInstance,
									MAKEINTRESOURCE(IDI_ICON_ARM),
									IMAGE_ICON, 32, 32,
									LR_DEFAULTCOLOR);

	m_hIconDisarm = (HICON)::LoadImage(AfxGetApp()->m_hInstance,
									   MAKEINTRESOURCE(IDI_ICON_DISARM),
									   IMAGE_ICON, 32, 32,
									   LR_DEFAULTCOLOR);

	m_hIconNetOk = (HICON)::LoadImage(AfxGetApp()->m_hInstance,
									  MAKEINTRESOURCE(IDI_ICON_NETOK),
									  IMAGE_ICON, 32, 32,
									  LR_DEFAULTCOLOR);

	m_hIconNetFailed = (HICON)::LoadImage(AfxGetApp()->m_hInstance,
										  MAKEINTRESOURCE(IDI_ICON_NETFAIL),
										  IMAGE_ICON, 32, 32,
										  LR_DEFAULTCOLOR);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CAlarmMachineContainerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	if (m_machineDlg != NULL) {
		if (IsWindow(m_machineDlg->GetSafeHwnd())) {
			m_machineDlg->DestroyWindow();
		}
		delete m_machineDlg;
		m_machineDlg = NULL;
	}	

	std::list<gui::CButtonEx*>::iterator iter = m_machineList.begin();
	while (iter != m_machineList.end()) {
		gui::CButtonEx* btn = *iter++;
		delete btn;
	}
	m_machineList.clear();

	if (m_hIconArm)			{	DeleteObject(m_hIconArm);		}
	if (m_hIconDisarm)		{	DeleteObject(m_hIconDisarm);	}
	if (m_hIconNetOk)		{	DeleteObject(m_hIconNetOk);		}
	if (m_hIconNetFailed)	{	DeleteObject(m_hIconNetFailed);	}
}


afx_msg LRESULT CAlarmMachineContainerDlg::OnBnclkedEx(WPARAM wParam, LPARAM lParam)
{
	int lr = static_cast<int>(wParam);
	int ademco_id = static_cast<int>(lParam);
	core::CAlarmMachine* machine = NULL;

	if (!core::CAlarmMachineManager::GetInstance()->GetMachine(ademco_id, machine) 
		|| (machine == NULL)) {
		return 0;
	}

	if (lr == 0) { // left button clicked
		if (m_machineDlg == NULL) {
			m_machineDlg = new gui::CAlarmmachineDlg(this);
		}

		if (IsWindow(m_machineDlg->GetSafeHwnd())) {
			m_machineDlg->DestroyWindow();
		}

		m_machineDlg->SetMachineInfo(machine);
		m_machineDlg->Create(IDD_DIALOG_MACHINE, this);
		m_machineDlg->ShowWindow(SW_SHOW);

	} else if (lr == 1) { // right button clicked

	}
	return 0;
}


