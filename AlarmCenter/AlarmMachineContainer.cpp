// AlarmMachineContainer.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AlarmMachineContainer.h"
#include "afxdialogex.h"
#include "ScrollHelper.h"
#include "AlarmMachine.h"

// CAlarmMachineContainerDlg dialog

IMPLEMENT_DYNAMIC(CAlarmMachineContainerDlg, CDialogEx)

CAlarmMachineContainerDlg::CAlarmMachineContainerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAlarmMachineContainerDlg::IDD, pParent)
	, m_scrollHelper(NULL)
{
	m_scrollHelper = new gui::CScrollHelper();
	m_scrollHelper->AttachWnd(this);
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
END_MESSAGE_MAP()


// CAlarmMachineContainerDlg message handlers


void CAlarmMachineContainerDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default
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


BOOL CAlarmMachineContainerDlg::InsertMachine(core::CAlarmMachine* machine)
{
	static const int btnWidth = 200;
	static const int btnHeight = 45;
	static const int xGaps = 25;
	static const int yGaps = 15;
	CRect rc;
	GetClientRect(rc);
	ClientToScreen(rc);
	CSize sz(rc.Width(), rc.Height());

	int nX = sz.cx / (btnWidth + xGaps);
	//int nY = sz.cy / (btnHeight + yGaps) - 1;

	CString txt = _T("");
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
		swprintf_s(name, L"%04d", machine->GetAdemcoID());
	}

	ButtonEx* btn = new ButtonEx(alias, rcBtn, this, IDC_BUTTON1, machine->GetAdemcoID());
	btn->_button->ShowWindow(SW_SHOW);
	m_machineList.push_back(btn);

	return 0;
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

	std::list<PButtonEx>::iterator iter = m_machineList.begin();
	while (iter != m_machineList.end()) {
		PButtonEx btn = *iter++;
		delete btn;
	}
	m_machineList.clear();
}
