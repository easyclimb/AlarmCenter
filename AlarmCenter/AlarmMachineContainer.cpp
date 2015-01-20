// AlarmMachineContainer.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AlarmMachineContainer.h"
#include "afxdialogex.h"
#include "ScrollHelper.h"
#include "AlarmMachine.h"


HICON CAlarmMachineContainerDlg::m_hIconArm			= NULL;
HICON CAlarmMachineContainerDlg::m_hIconDisarm		= NULL;
HICON CAlarmMachineContainerDlg::m_hIconNetOk		= NULL;
HICON CAlarmMachineContainerDlg::m_hIconNetFailed	= NULL;

class CButtonEx
{
	DECLARE_UNCOPYABLE(CButtonEx)
private:
	CMFCButton* _button;
	DWORD _data;
	core::MachineStatus _status;
	CButtonEx() {}
public:
	CButtonEx(const wchar_t* text,
			  const RECT& rc,
			  CWnd* parent,
			  UINT id,
			  DWORD data) : _button(NULL), _data(data), _status(core::MS_OFFLINE)
	{
		_button = new CMFCButton();
		_button->Create(text, WS_CHILD | WS_VISIBLE | BS_ICON, rc, parent, id);
		ASSERT(IsWindow(_button->m_hWnd));

	}

	~CButtonEx()
	{
		_button->DestroyWindow();
		delete _button;
	}

	void ShowWindow(int nCmdShow)
	{
		if (_button && IsWindow(_button->m_hWnd)) {
			UpdateIcon();
			_button->ShowWindow(nCmdShow);
		}
	}

	void SetStatus(core::MachineStatus status)
	{
		if (_status != status) {
			_status = status;

			if (_button && IsWindow(_button->m_hWnd)) {
				UpdateIcon();
				_button->FlashWindow(TRUE);
			}
		}
	}
protected:
	void UpdateIcon()
	{
		switch (_status) {
			case core::MS_OFFLINE:
				_button->SetIcon(CAlarmMachineContainerDlg::m_hIconNetFailed);
				break;
			case core::MS_ONLINE:
				_button->SetIcon(CAlarmMachineContainerDlg::m_hIconNetOk);
				break;
			case core::MS_DISARM:
				_button->SetIcon(CAlarmMachineContainerDlg::m_hIconDisarm);
				break;
			case core::MS_ARM:
				_button->SetIcon(CAlarmMachineContainerDlg::m_hIconArm);
				break;
			default:
				_button->SetIcon(CAlarmMachineContainerDlg::m_hIconNetFailed);
				break;
		}
		_button->Invalidate();
	}
};

static void _stdcall OnMachineStatusChange(void* data, core::MachineStatus status)
{
	CButtonEx* btn = reinterpret_cast<CButtonEx*>(data);
	btn->SetStatus(status);
}

// CAlarmMachineContainerDlg dialog

IMPLEMENT_DYNAMIC(CAlarmMachineContainerDlg, CDialogEx)

CAlarmMachineContainerDlg::CAlarmMachineContainerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAlarmMachineContainerDlg::IDD, pParent)
	, m_scrollHelper(NULL)
{
	m_scrollHelper = new gui::CScrollHelper();
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
		swprintf_s(name, L"%04d", machine->GetAdemcoID());
	}

	CButtonEx* btn = new CButtonEx(name, rcBtn, this, IDC_BUTTON1, machine->GetAdemcoID());
	btn->ShowWindow(SW_SHOW);
	machine->SetMachineStatusCb(btn, OnMachineStatusChange);
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

	std::list<CButtonEx*>::iterator iter = m_machineList.begin();
	while (iter != m_machineList.end()) {
		CButtonEx* btn = *iter++;
		delete btn;
	}
	m_machineList.clear();

	if (m_hIconArm) {
		DeleteObject(m_hIconArm);
	}
	if (m_hIconDisarm) {
		DeleteObject(m_hIconDisarm);
	}
	if (m_hIconNetOk) {
		DeleteObject(m_hIconNetOk);
	}
	if (m_hIconNetFailed) {
		DeleteObject(m_hIconNetFailed);
	}
}
