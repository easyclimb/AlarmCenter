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
#include "ademco_event.h"
using namespace gui;
using namespace ademco;
//namespace gui {

//static void _stdcall OnAdemcoEvent(void* data, int zone, int ademco_event)
//{
//	CAlarmMachineDlg* dlg = reinterpret_cast<CAlarmMachineDlg*>(data); ASSERT(dlg);
//	dlg->OnAdemcoEvent(zone, ademco_event);
//}
IMPLEMENT_ADEMCO_EVENT_CALL_BACK(CAlarmMachineDlg, OnAdemcoEvent)

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
	m_machine->RegisterObserver(this, OnAdemcoEvent);

	m_btnArm.SetIcon(CAlarmMachineContainerDlg::m_hIconArm);
	m_btnDisarm.SetIcon(CAlarmMachineContainerDlg::m_hIconDisarm);
	m_btnEmergency.SetIcon(CAlarmMachineContainerDlg::m_hIconEmergency);

	CString text = L"", fmAlias, fmContact, fmAddress, fmPhone, fmPhoneBk, fmNull;
	CString alias, contact, address, phone, phone_bk;
	fmAlias.LoadStringW(IDS_STRING_ALIAS);
	fmContact.LoadStringW(IDS_STRING_CONTACT);
	fmAddress.LoadStringW(IDS_STRING_ADDRESS);
	fmPhone.LoadStringW(IDS_STRING_PHONE);
	fmPhoneBk.LoadStringW(IDS_STRING_PHONE_BK);
	fmNull.LoadStringW(IDS_STRING_NULL);

	alias = m_machine->get_alias();
	contact = m_machine->get_contact();
	address = m_machine->get_address();
	phone = m_machine->get_phone();
	phone_bk = m_machine->get_phone_bk();

	text.Format(L"ID:%04d    %s:%s    %s:%s    %s:%s    %s:%s    %s:%s",
				   m_machine->get_ademco_id(),
				   fmAlias, alias.IsEmpty() ? fmNull : alias,
				   fmContact, contact.IsEmpty() ? fmNull : contact,
				   fmAddress, address.IsEmpty() ? fmNull : address,
				   fmPhone, phone.IsEmpty() ? fmNull : phone,
				   fmPhoneBk, phone_bk.IsEmpty() ? fmNull : phone_bk);
	SetWindowText(text);

	if (m_machine->IsOnline()) {
		m_staticNet.SetIcon(CAlarmMachineContainerDlg::m_hIconNetOk);
	} else {
		m_staticNet.SetIcon(CAlarmMachineContainerDlg::m_hIconNetFailed);
	}

	rcRight.DeflateRect(5, 15, 5, 5);
	m_mapView = new CMapView();
	m_mapView->SetMapInfo(m_machine->GetFirstMap());
	m_mapView->Create(IDD_DIALOG_MAPVIEW, this);
	m_mapView->MoveWindow(rcRight, FALSE);
	m_mapView->ShowWindow(SW_SHOW);

	m_machine->TraverseAdmecoEventList(this, OnAdemcoEvent);

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


void CAlarmMachineDlg::OnAdemcoEventResult(const ademco::AdemcoEvent* ademcoEvent)
{
	ASSERT(ademcoEvent);
	switch (ademcoEvent->_ademco_event) {
		case MS_OFFLINE:
			m_staticNet.SetIcon(CAlarmMachineContainerDlg::m_hIconNetFailed);
			break;
		case MS_ONLINE:
			m_staticNet.SetIcon(CAlarmMachineContainerDlg::m_hIconNetOk);
			break;
		case ademco::EVENT_DISARM:
			m_staticStatus.SetIcon(CAlarmMachineContainerDlg::m_hIconDisarm);
			break;
		case ademco::EVENT_ARM:
			m_staticStatus.SetIcon(CAlarmMachineContainerDlg::m_hIconArm);
			break;
		default:	// means its alarming
			if (m_mapView) {
				m_mapView->HandleAdemcoEvent(ademcoEvent);
			}
			break;
	}
}


int CAlarmMachineDlg::GetAdemcoID() const
{
	if (m_machine) {
		return m_machine->get_ademco_id();
	}
	return -1;
}



//NAMESPACE_END
