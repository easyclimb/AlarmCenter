// AlarmmachineDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AlarmmachineDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "AlarmMachineContainer.h"
#include "AlarmMachineManager.h"
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
	DDX_Control(pDX, IDC_TAB1, m_tab);
}


BEGIN_MESSAGE_MAP(CAlarmMachineDlg, CDialogEx)
	ON_WM_DESTROY()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CAlarmMachineDlg::OnTcnSelchangeTab)
	ON_MESSAGE(WM_DISPATCHEVENT, &CAlarmMachineDlg::OnDispatchevent)
	ON_BN_CLICKED(IDC_BUTTON_ARM, &CAlarmMachineDlg::OnBnClickedButtonArm)
	ON_BN_CLICKED(IDC_BUTTON_DISARM, &CAlarmMachineDlg::OnBnClickedButtonDisarm)
	ON_BN_CLICKED(IDC_BUTTON_EMERGENCY, &CAlarmMachineDlg::OnBnClickedButtonEmergency)
	ON_BN_CLICKED(IDC_BUTTON_CLEARMSG, &CAlarmMachineDlg::OnBnClickedButtonClearmsg)
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
	m_groupContent.ShowWindow(SW_HIDE);
	//m_groupContent.MoveWindow(rcRight);
	m_tab.MoveWindow(rcRight);

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

	//rcRight.DeflateRect(5, 15, 5, 5);
	
	//m_tab.InsertItem(0, L"abc");
	m_tab.ShowWindow(SW_SHOW);
	CRect rcTab;
	m_tab.GetClientRect(rcTab);
	rcTab.DeflateRect(5, 25, 5, 5);

	core::CMapInfo* noZoneMapInfo = m_machine->GetNoZoneMap();
	if (noZoneMapInfo) {
		CMapView* mapView = new CMapView();
		mapView->SetMachineInfo(m_machine);
		mapView->SetMapInfo(noZoneMapInfo);
		mapView->Create(IDD_DIALOG_MAPVIEW, &m_tab);
		mapView->MoveWindow(rcTab, FALSE);
		mapView->ShowWindow(SW_HIDE);

		int ndx = m_tab.InsertItem(0, noZoneMapInfo->get_alias());
		assert(ndx == 0);
		MapViewWithNdx* mn = new MapViewWithNdx(mapView, ndx);
		m_mapViewList.push_back(mn);
	}

	core::CMapInfo* mapInfo = m_machine->GetFirstMap();
	int nItem = 1;
	while (mapInfo) {
		CMapView* mapView = new CMapView();
		mapView->SetMachineInfo(m_machine);
		mapView->SetMapInfo(mapInfo);
		mapView->Create(IDD_DIALOG_MAPVIEW, &m_tab);
		mapView->MoveWindow(rcTab, FALSE);
		mapView->ShowWindow(SW_HIDE);

		int ndx = m_tab.InsertItem(nItem++, mapInfo->get_alias());
		assert(ndx != -1);
		MapViewWithNdx* mn = new MapViewWithNdx(mapView, ndx);
		m_mapViewList.push_back(mn);
		mapInfo = m_machine->GetNextMap();
	}

	

	m_tab.SetCurSel(0);
	if (m_mapViewList.size() > 0) {
		MapViewWithNdx* mn = m_mapViewList.front();
		mn->_mapView->ShowWindow(SW_SHOW);
	}

	m_machine->TraverseAdmecoEventList(this, OnAdemcoEvent);

	

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CAlarmMachineDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	m_tab.DeleteAllItems();
	ASSERT(m_machine);
	m_machine->UnRegisterObserver(this);
	m_machine = NULL;

	std::list<MapViewWithNdx*>::iterator iter = m_mapViewList.begin();
	while (iter != m_mapViewList.end()) {
		MapViewWithNdx* mn = *iter++;
		mn->_mapView->DestroyWindow();
		delete mn->_mapView;
		delete mn;
	}
	m_mapViewList.clear();
}


void CAlarmMachineDlg::ClearMsg()
{
	std::list<MapViewWithNdx*>::iterator iter = m_mapViewList.begin();
	while (iter != m_mapViewList.end()) {
		MapViewWithNdx* mn = *iter++;
		mn->_mapView->ClearMsg();
	}
}


void CAlarmMachineDlg::OnAdemcoEventResult(const ademco::AdemcoEvent* ademcoEvent)
{
	ASSERT(ademcoEvent);
	switch (ademcoEvent->_event) {
		case EVENT_CLEARMSG:
			ClearMsg();
			break;
		case MS_OFFLINE:
			m_staticNet.SetIcon(CAlarmMachineContainerDlg::m_hIconNetFailed);
			break;
		case MS_ONLINE:
			m_staticNet.SetIcon(CAlarmMachineContainerDlg::m_hIconNetOk);
			break;
		case ademco::EVENT_DISARM:
			m_staticNet.SetIcon(CAlarmMachineContainerDlg::m_hIconNetOk);
			m_staticStatus.SetIcon(CAlarmMachineContainerDlg::m_hIconDisarm);
			break;
		case ademco::EVENT_ARM:
			m_staticNet.SetIcon(CAlarmMachineContainerDlg::m_hIconNetOk);
			m_staticStatus.SetIcon(CAlarmMachineContainerDlg::m_hIconArm);
			break;
		default:	// means its alarming
			//DispatchAdemcoEvent(ademcoEvent);
			m_staticNet.SetIcon(CAlarmMachineContainerDlg::m_hIconNetOk);
			SendMessage(WM_DISPATCHEVENT, (WPARAM)ademcoEvent);
			break;
	}
}


void CAlarmMachineDlg::DispatchAdemcoEvent(const ademco::AdemcoEvent* ademcoEvent)
{
	bool found = false;
	MapViewWithNdx* targetMN = NULL;
	std::list<MapViewWithNdx*>::iterator iter = m_mapViewList.begin();
	while (iter != m_mapViewList.end()) {
		MapViewWithNdx* mn = *iter++;
		if (mn->_mapView->IsThisYourZone(ademcoEvent->_zone)) { // found
			found = true;
			targetMN = mn;
			//mn->_mapView->ShowWindow(SW_SHOW);
			
		} else {
			//mn->_mapView->ShowWindow(SW_HIDE);
		}
		mn->_mapView->ShowWindow(SW_HIDE);
	}

	if (found) {
		m_tab.SetCurSel(targetMN->_ndx);
		std::list<MapViewWithNdx*>::iterator iter = m_mapViewList.begin();
		while (iter != m_mapViewList.end()) {
			MapViewWithNdx* mn = *iter++;
			if (mn == targetMN) { // found
				mn->_mapView->ShowWindow(SW_SHOW);
				mn->_mapView->HandleAdemcoEvent(ademcoEvent);
			} else {
				mn->_mapView->ShowWindow(SW_HIDE);
			}
		}
		
	} else {
		// not found, means this zone has not bind to map or detector.
		iter = m_mapViewList.begin();

		if (iter != m_mapViewList.end()) {
			MapViewWithNdx* mn = *iter++;
			m_tab.SetCurSel(mn->_ndx);
			mn->_mapView->ShowWindow(SW_SHOW);
			mn->_mapView->HandleAdemcoEvent(ademcoEvent);
		}

		while (iter != m_mapViewList.end()) {
			MapViewWithNdx* mn = *iter++;
			mn->_mapView->ShowWindow(SW_HIDE);
		}
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


void CAlarmMachineDlg::OnTcnSelchangeTab(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	int ndx = m_tab.GetCurSel();
	if (ndx == -1)return;

	std::list<MapViewWithNdx*>::iterator iter = m_mapViewList.begin();
	while (iter != m_mapViewList.end()) {
		MapViewWithNdx* mn = *iter++;
		if (mn->_ndx == ndx) { // found
			mn->_mapView->ShowWindow(SW_SHOW);
		} else {
			mn->_mapView->ShowWindow(SW_HIDE);
		}
	}

	*pResult = 0;
}


afx_msg LRESULT CAlarmMachineDlg::OnDispatchevent(WPARAM wParam, LPARAM)
{
	const ademco::AdemcoEvent* ademcoEvent = reinterpret_cast<const ademco::AdemcoEvent*>(wParam);
	DispatchAdemcoEvent(ademcoEvent);
	return 0;
}


void CAlarmMachineDlg::OnBnClickedButtonArm()
{
	core::CAlarmMachineManager* manager = core::CAlarmMachineManager::GetInstance();
	manager->RemoteControlAlarmMachine(m_machine, ademco::EVENT_ARM, this);
}


void CAlarmMachineDlg::OnBnClickedButtonDisarm()
{
	core::CAlarmMachineManager* manager = core::CAlarmMachineManager::GetInstance();
	manager->RemoteControlAlarmMachine(m_machine, ademco::EVENT_DISARM, this);
}


void CAlarmMachineDlg::OnBnClickedButtonEmergency()
{
	core::CAlarmMachineManager* manager = core::CAlarmMachineManager::GetInstance();
	manager->RemoteControlAlarmMachine(m_machine, ademco::EVENT_EMERGENCY, this);
}


void CAlarmMachineDlg::OnBnClickedButtonClearmsg()
{
	if (m_machine) {
		m_machine->clear_ademco_event_list();
	}
}
