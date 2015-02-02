// AlarmmachineDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AlarmmachineDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "AlarmMachineContainer.h"
#include "MapView.h"
#include "MapInfo.h"
using namespace gui;
//namespace gui {

static void _stdcall OnMachineStatusChange(void* data, core::MachineStatus status)
{
	CAlarmMachineDlg* dlg = reinterpret_cast<CAlarmMachineDlg*>(data); ASSERT(dlg);
	dlg->OnStatusChange(status);
}
// CAlarmMachineDlg dialog

IMPLEMENT_DYNAMIC(CAlarmMachineDlg, CDialogEx)

CAlarmMachineDlg::CAlarmMachineDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAlarmMachineDlg::IDD, pParent)
	, m_machine(NULL)
	, m_mapView(NULL)
{

}

CAlarmMachineDlg::~CAlarmMachineDlg()
{
}

void CAlarmMachineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_CONTROL_PANEL, m_groupControlPanel);
	DDX_Control(pDX, IDC_STATIC_CONTENT, m_groupContent);
	DDX_Control(pDX, IDC_BUTTON_ARM, m_btnArm);
	DDX_Control(pDX, IDC_BUTTON_DISARM, m_btnDisarm);
	DDX_Control(pDX, IDC_BUTTON_EMERGENCY, m_btnEmergency);
	DDX_Control(pDX, IDC_BUTTON_CLEARMSG, m_btnClearMsg);
	DDX_Control(pDX, IDC_STATIC_NET, m_staticNet);
	DDX_Control(pDX, IDC_STATIC_STATUS, m_staticStatus);
}


BEGIN_MESSAGE_MAP(CAlarmMachineDlg, CDialogEx)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CAlarmMachineDlg message handlers


void CAlarmMachineDlg::SetMachineInfo(core::CAlarmMachine* machine)
{
	m_machine = machine;
}


BOOL CAlarmMachineDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CRect rc(0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN));
	MoveWindow(rc);
	//SetWindowPos(&CWnd::wndNoTopMost, 1, 1, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN), SWP_SHOWWINDOW);
	//ShowWindow(SW_MAXIMIZE);

	GetClientRect(rc);
	rc.DeflateRect(5, 5, 5, 5);
	CRect rcLeft(rc);
	rcLeft.right = rcLeft.left + 180;
	CRect rcRight(rc);
	rcRight.left = rcLeft.right + 5;
	m_groupControlPanel.MoveWindow(rcLeft);
	m_groupContent.MoveWindow(rcRight);

	ASSERT(m_machine);
	m_machine->RegisterObserver(this, OnMachineStatusChange);

	m_btnArm.SetIcon(CAlarmMachineContainerDlg::m_hIconArm);
	m_btnDisarm.SetIcon(CAlarmMachineContainerDlg::m_hIconDisarm);
	m_btnEmergency.SetIcon(CAlarmMachineContainerDlg::m_hIconEmergency);

	CString text = L"";
	text.Format(L"%s    %04d    %s    %s    %s    %s",
				m_machine->get_alias(),
				m_machine->get_ademco_id(),
				m_machine->get_contact(),
				m_machine->get_address(),
				m_machine->get_phone(),
				m_machine->get_phone_bk());
	SetWindowText(text);

	if (m_machine->IsOnline()) {
		m_staticNet.SetIcon(CAlarmMachineContainerDlg::m_hIconNetOk);
	} else {
		m_staticNet.SetIcon(CAlarmMachineContainerDlg::m_hIconNetFailed);
	}

	core::MachineStatus status = m_machine->GetStatus();
	OnStatusChange(status);

	CreateMap();

	rcRight.DeflateRect(5, 15, 5, 5);
	m_mapView = new CMapView();
	m_mapView->SetMapInfo(m_machine->GetFirstMap());
	m_mapView->Create(IDD_DIALOG_MAPVIEW, this);
	m_mapView->MoveWindow(rcRight, FALSE);
	m_mapView->ShowWindow(SW_SHOW);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CAlarmMachineDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	ASSERT(m_machine);
	m_machine->UnregisterObserver(this);
	m_machine = NULL;

	if (m_mapView) {
		m_mapView->DestroyWindow();
		delete m_mapView;
		m_mapView = NULL;
	}
}


void CAlarmMachineDlg::OnStatusChange(core::MachineStatus status)
{
	switch (status) {
		case core::MS_OFFLINE:
			m_staticNet.SetIcon(CAlarmMachineContainerDlg::m_hIconNetFailed);
			break;
		case core::MS_ONLINE:
			m_staticNet.SetIcon(CAlarmMachineContainerDlg::m_hIconNetOk);
			break;
		case core::MS_DISARM:
			m_staticStatus.SetIcon(CAlarmMachineContainerDlg::m_hIconDisarm);
			break;
		case core::MS_ARM:
			m_staticStatus.SetIcon(CAlarmMachineContainerDlg::m_hIconArm);
			break;
		default:	// means its alarming
			
			break;
	}
}


void CAlarmMachineDlg::CreateMap()
{
	CRect rc;
	m_groupContent.GetWindowRect(rc);
	//m_groupContent.ClientToScreen(rc);
	//rc.DeflateRect(5, 5, 5, 5);
	
	
}





//NAMESPACE_END
