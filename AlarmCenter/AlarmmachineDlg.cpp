// AlarmmachineDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AlarmmachineDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "AlarmMachineContainer.h"

namespace gui {

static void _stdcall OnMachineStatusChange(void* data, core::MachineStatus status)
{
	CAlarmmachineDlg* dlg = reinterpret_cast<CAlarmmachineDlg*>(data); ASSERT(dlg);
	dlg->OnStatusChange(status);
}
// CAlarmmachineDlg dialog

IMPLEMENT_DYNAMIC(CAlarmmachineDlg, CDialogEx)

CAlarmmachineDlg::CAlarmmachineDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAlarmmachineDlg::IDD, pParent)
	, m_machine(NULL)
{

}

CAlarmmachineDlg::~CAlarmmachineDlg()
{
}

void CAlarmmachineDlg::DoDataExchange(CDataExchange* pDX)
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


BEGIN_MESSAGE_MAP(CAlarmmachineDlg, CDialogEx)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CAlarmmachineDlg message handlers


void CAlarmmachineDlg::SetMachineInfo(core::CAlarmMachine* machine)
{
	m_machine = machine;
}


BOOL CAlarmmachineDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	ASSERT(m_machine);
	m_machine->RegisterObserver(this, OnMachineStatusChange);

	m_btnArm.SetIcon(CAlarmMachineContainerDlg::m_hIconArm);
	m_btnDisarm.SetIcon(CAlarmMachineContainerDlg::m_hIconDisarm);
	m_btnEmergency.SetIcon(CAlarmMachineContainerDlg::m_hIconEmergency);

	CString alias = m_machine->GetAlias();
	if (alias.IsEmpty()) {
		alias.Format(L"%04d", m_machine->get_ademco_id());
	}
	SetWindowText(alias);

	if (m_machine->IsOnline()) {
		m_staticNet.SetIcon(CAlarmMachineContainerDlg::m_hIconNetOk);
	} else {
		m_staticNet.SetIcon(CAlarmMachineContainerDlg::m_hIconNetFailed);
	}

	core::MachineStatus status = m_machine->GetStatus();
	OnStatusChange(status);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CAlarmmachineDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	ASSERT(m_machine);
	m_machine->UnregisterObserver(this);
}


void CAlarmmachineDlg::OnStatusChange(core::MachineStatus status)
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


NAMESPACE_END
