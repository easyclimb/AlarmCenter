// AlarmmachineDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AlarmmachineDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"

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
			//_button->SetIcon(CAlarmMachineContainerDlg::m_hIconNetFailed);
			break;
	//	case core::MS_ONLINE:
	//		_button->SetTextColor(RGB(0, 0, 0));
	//		_button->SetIcon(CAlarmMachineContainerDlg::m_hIconNetOk);
	//		break;
	//	case core::MS_DISARM:
	//		_button->SetTextColor(RGB(0, 0, 0));
	//		_button->SetIcon(CAlarmMachineContainerDlg::m_hIconDisarm);
	//		break;
	//	case core::MS_ARM:
	//		_button->SetTextColor(RGB(0, 0, 0));
	//		_button->SetIcon(CAlarmMachineContainerDlg::m_hIconArm);
	//		break;
		default:	// means its alarming
			
			break;
	}
}

