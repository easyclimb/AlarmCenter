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
#include "HistoryRecord.h"
#include "AppResource.h"
#include "EditZoneDlg.h"
#include "ZoneInfo.h"
#include "EditMapDlg.h"
#include "EditDetectorDlg.h"

using namespace gui;
using namespace ademco;
using namespace core;

static const int TIMER_ID_TRAVERSE_ADEMCO_LIST = 1;
static const int TIMER_ID_REMOTE_CONTROL_MACHINE = 2;
static const int REMOTE_CONTROL_DISABLE_TIMEUP = 60;

static void __stdcall OnNewRecord(void* udata, const HistoryRecord* record)
{
	CAlarmMachineDlg* dlg = reinterpret_cast<CAlarmMachineDlg*>(udata); assert(dlg);
	dlg->SendMessage(WM_NEWRECORD, (WPARAM)(record));
}


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
	//, m_machine()
	//, m_machineType(0)
	, m_machine(NULL)
	, m_maxHistory2Show(0)
	, m_nRemoteControlTimeCounter(0)
	, m_curRemoteControlCommand(0)
	, m_strBtnArm(L"")
	, m_strBtnDisarm(L"")
	, m_strBtnEmergency(L"")
{
	/*m_machine = NULL;
	m_machine.subMachine = NULL;*/
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
	DDX_Control(pDX, IDC_LIST_HISTORY, m_listHistory);
	DDX_Control(pDX, IDC_STATIC_HISTORY, m_groupHistory);
	DDX_Control(pDX, IDC_BUTTON_EDIT_VIDEO, m_btnEditVideoInfo);
	DDX_Control(pDX, IDC_STATIC_CONN, m_staticConn);
	DDX_Control(pDX, IDC_STATIC_MACHINE_STATUS, m_staticMachineStatus);
}


BEGIN_MESSAGE_MAP(CAlarmMachineDlg, CDialogEx)
	ON_WM_DESTROY()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CAlarmMachineDlg::OnTcnSelchangeTab)
	//ON_MESSAGE(WM_DISPATCHEVENT, &CAlarmMachineDlg::OnDispatchevent)
	ON_BN_CLICKED(IDC_BUTTON_ARM, &CAlarmMachineDlg::OnBnClickedButtonArm)
	ON_BN_CLICKED(IDC_BUTTON_DISARM, &CAlarmMachineDlg::OnBnClickedButtonDisarm)
	ON_BN_CLICKED(IDC_BUTTON_EMERGENCY, &CAlarmMachineDlg::OnBnClickedButtonEmergency)
	ON_BN_CLICKED(IDC_BUTTON_CLEARMSG, &CAlarmMachineDlg::OnBnClickedButtonClearmsg)
	ON_MESSAGE(WM_NEWRECORD, &CAlarmMachineDlg::OnNewrecordResult)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_EDIT_ZONE, &CAlarmMachineDlg::OnBnClickedButtonEditZone)
	ON_MESSAGE(WM_INVERSIONCONTROL, &CAlarmMachineDlg::OnInversionControl)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_MAP, &CAlarmMachineDlg::OnBnClickedButtonEditMap)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_DETECTOR, &CAlarmMachineDlg::OnBnClickedButtonEditDetector)
END_MESSAGE_MAP()


// CAlarmMachineDlg message handlers


void CAlarmMachineDlg::SetMachineInfo(CAlarmMachine* machine)
{
	m_machine = machine;
}


BOOL CAlarmMachineDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	if (!m_machine) {
		return FALSE;
	}

	CRect rc(0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN));
	//CRect rc;
	//GetClientRect(rc);
	//ClientToScreen(rc);
	if (m_machine->get_is_submachine())	{
		rc.DeflateRect(50, 50, 50, 50);
		m_btnArm.EnableWindow(0);
		m_btnDisarm.EnableWindow(0);
	} else {
		rc.DeflateRect(25, 25, 25, 25);
		m_btnArm.EnableWindow(1);
		m_btnDisarm.EnableWindow(1);
	}
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

	CRect rcBtn;
	m_btnEditVideoInfo.GetWindowRect(rcBtn);
	ScreenToClient(rcBtn);
	CRect rcHistory(rcLeft);
	rcHistory.top = rcBtn.bottom + 5;
	m_groupHistory.MoveWindow(rcHistory);
	rcHistory.DeflateRect(5, 18, 5, 5);
	m_listHistory.MoveWindow(rcHistory);
	m_listHistory.GetWindowRect(rcHistory);
	int columnHeight = m_listHistory.GetItemHeight(0);
	m_maxHistory2Show = rcHistory.Height() / columnHeight - 2;

	m_btnArm.SetIcon(CAppResource::m_hIconArm);
	m_btnDisarm.SetIcon(CAppResource::m_hIconDisarm);
	m_btnEmergency.SetIcon(CAppResource::m_hIconEmergency);

	m_btnEditVideoInfo.EnableWindow(0);

	m_tab.ShowWindow(SW_SHOW);

	m_btnArm.GetWindowTextW(m_strBtnArm);
	m_btnDisarm.GetWindowTextW(m_strBtnDisarm);
	m_btnEmergency.GetWindowTextW(m_strBtnEmergency);
	

	// 设置窗体标题
	CString text = L"", fmMachine, fmSubMachine, fmAlias, fmContact, 
		fmAddress, fmPhone, fmPhoneBk, fmNull;
	CString sid, smachine, sstatus;
	fmMachine.LoadStringW(IDS_STRING_MACHINE);
	fmSubMachine.LoadStringW(IDS_STRING_SUBMACHINE);
	fmAlias.LoadStringW(IDS_STRING_ALIAS);
	fmContact.LoadStringW(IDS_STRING_CONTACT);
	fmAddress.LoadStringW(IDS_STRING_ADDRESS);
	fmPhone.LoadStringW(IDS_STRING_PHONE);
	fmPhoneBk.LoadStringW(IDS_STRING_PHONE_BK);
	fmNull.LoadStringW(IDS_STRING_NULL);
	sstatus.LoadStringW(IDS_STRING_MACHINE_STATUS);

	if (m_machine->get_is_submachine()) {
		sid.Format(L"%s%03d", fmSubMachine, m_machine->get_submachine_zone());
		smachine.LoadStringW(IDS_STRING_SUBMACHINE);
	} else {
		sid.Format(L"%s%04d", fmMachine, m_machine->get_ademco_id());
		smachine.LoadStringW(IDS_STRING_MACHINE);
	}
	m_staticMachineStatus.SetWindowTextW(smachine + sstatus);

	text.Format(L"%s    %s:%s    %s:%s    %s:%s    %s:%s    %s:%s",
				sid,
				fmAlias, m_machine->get_alias(),
				fmContact, m_machine->get_contact(),
				fmAddress, m_machine->get_address(),
				fmPhone, m_machine->get_phone(),
				fmPhoneBk, m_machine->get_phone_bk());
	SetWindowText(text);



	// 1. 注册Ademco事件回调事件
	m_machine->RegisterObserver(this, OnAdemcoEvent);

	// 2. 设置主机状态图标
	if (m_machine->IsOnline()) {
		m_staticNet.SetIcon(CAppResource::m_hIconNetOk);
	} else {
		m_staticNet.SetIcon(CAppResource::m_hIconNetFailed);
	}

	if (m_machine->IsArmed()) {
		m_staticStatus.SetIcon(CAppResource::m_hIconArm);
	} else {
		m_staticStatus.SetIcon(CAppResource::m_hIconDisarm);
	}

	if (m_machine->get_is_submachine()) {
		//m_staticNet.ShowWindow(SW_HIDE);
		text.LoadStringW(IDS_STRING_SLAVE_CONN);
		m_staticConn.SetWindowTextW(text);
		//m_staticConn.ShowWindow(SW_HIDE);
	}

	MachineType mt = m_machine->get_type();
	m_btnEditVideoInfo.EnableWindow(mt == MT_VEDIO);

	// 3. 载入地图信息
	LoadMaps();

	// 4. 设置历史记录回调函数
	CHistoryRecord* hr = CHistoryRecord::GetInstance();
	if (m_machine->get_is_submachine()) {
		hr->GetTopNumRecordByAdemcoIDAndZone(m_maxHistory2Show, m_machine->get_ademco_id(),
											 m_machine->get_submachine_zone(),
											 this, OnNewRecord);
	} else {
		hr->GetTopNumRecordByAdemcoID(m_maxHistory2Show, m_machine->get_ademco_id(),
									  this, OnNewRecord);
	}
	hr->RegisterObserver(this, OnNewRecord);

	// 5. 设置定时器，延时获取Ademco事件列表
	//m_machine->TraverseAdmecoEventList(this, OnAdemcoEvent);
	SetTimer(100, TIMER_ID_TRAVERSE_ADEMCO_LIST, NULL);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CAlarmMachineDlg::LoadMaps()
{
	AUTO_LOG_FUNCTION;
	CRect rcTab;
	m_tab.GetClientRect(rcTab);
	rcTab.DeflateRect(5, 25, 5, 5);
	int prevSel = 0;
	CMapView* prevShowMap = NULL;
	if (m_tab.GetItemCount() > 0) {
		prevSel = m_tab.GetCurSel();
		ReleaseMaps();
	}

	CMapInfo* unbindZoneMapInfo = m_machine->GetUnbindZoneMap();
	if (unbindZoneMapInfo) {
		CMapView* mapView = new CMapView();
		mapView->SetRealParentWnd(this);
		mapView->SetMachineInfo(m_machine);
		mapView->SetMapInfo(unbindZoneMapInfo);
		mapView->Create(IDD_DIALOG_MAPVIEW, &m_tab);
		mapView->MoveWindow(rcTab, FALSE);
		mapView->ShowWindow(SW_HIDE);

		int ndx = m_tab.InsertItem(0, unbindZoneMapInfo->get_alias());
		assert(ndx == 0);
		MapViewWithNdx* mn = new MapViewWithNdx(mapView, ndx);
		m_mapViewList.push_back(mn);

		if (prevSel == ndx) {
			prevShowMap = mapView;
		}
	}

	CMapInfoList list;
	m_machine->GetAllMapInfo(list);
	CMapInfoListIter iter = list.begin();
	int nItem = 1;
	while (iter != list.end()) {
		CMapInfo* mapInfo = *iter++;
		CMapView* mapView = new CMapView();
		mapView->SetRealParentWnd(this);
		mapView->SetMachineInfo(m_machine);
		mapView->SetMapInfo(mapInfo);
		mapView->Create(IDD_DIALOG_MAPVIEW, &m_tab);
		mapView->MoveWindow(rcTab, FALSE);
		mapView->ShowWindow(SW_HIDE);

		int ndx = m_tab.InsertItem(nItem++, mapInfo->get_alias());
		assert(ndx != -1);
		MapViewWithNdx* mn = new MapViewWithNdx(mapView, ndx);
		m_mapViewList.push_back(mn);
		if (prevSel == ndx) {
			prevShowMap = mapView;
		}
	}

	m_tab.SetCurSel(prevSel);
	if (m_mapViewList.size() > 0) {
		if (prevShowMap) {
			prevShowMap->ShowWindow(SW_SHOW);
		} else {
			MapViewWithNdx* mn = m_mapViewList.front();
			mn->_mapView->ShowWindow(SW_SHOW);
		}
	}
}


void CAlarmMachineDlg::ReleaseMaps()
{
	m_tab.DeleteAllItems();

	std::list<MapViewWithNdx*>::iterator iter = m_mapViewList.begin();
	while (iter != m_mapViewList.end()) {
		MapViewWithNdx* mn = *iter++;
		mn->_mapView->DestroyWindow();
		delete mn->_mapView;
		delete mn;
	}
	m_mapViewList.clear();
}


void CAlarmMachineDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	CHistoryRecord* hr = CHistoryRecord::GetInstance();
	hr->UnRegisterObserver(this);

	if (m_machine) {
		m_machine->UnRegisterObserver(this);
		m_machine = NULL;
	}

	ReleaseMaps();
}


void CAlarmMachineDlg::OnAdemcoEventResult(const ademco::AdemcoEvent* ademcoEvent)
{
	ASSERT(ademcoEvent);

	bool bsubmachine_status = ademcoEvent->_sub_zone == INDEX_SUB_MACHINE;
	if (bsubmachine_status != m_machine->get_is_submachine()) {
		return;
	}

	switch (ademcoEvent->_event) {
		case EVENT_CLEARMSG:
			//ClearMsg();
			break;
		case MS_OFFLINE:
			m_staticNet.SetIcon(CAppResource::m_hIconNetFailed);
			break;
		case MS_ONLINE:
			m_staticNet.SetIcon(CAppResource::m_hIconNetOk);
			break;
		case ademco::EVENT_DISARM:
			m_staticNet.SetIcon(CAppResource::m_hIconNetOk);
			m_staticStatus.SetIcon(CAppResource::m_hIconDisarm);
			KillTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
			m_nRemoteControlTimeCounter = 0;
			OnTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
			break;
		case ademco::EVENT_ARM:
			m_staticNet.SetIcon(CAppResource::m_hIconNetOk);
			m_staticStatus.SetIcon(CAppResource::m_hIconArm);
			KillTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
			m_nRemoteControlTimeCounter = 0;
			OnTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
			break;
		case ademco::EVENT_EMERGENCY:
			KillTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
			m_nRemoteControlTimeCounter = 0;
			OnTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
			break;
		case ademco::EVENT_SUBMACHINECNT:
			break;
		default:	// means its alarming
			//DispatchAdemcoEvent(ademcoEvent);
			//m_staticNet.SetIcon(CAppResource::m_hIconNetOk);
			//SendMessage(WM_DISPATCHEVENT, (WPARAM)ademcoEvent);
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


//afx_msg LRESULT CAlarmMachineDlg::OnDispatchevent(WPARAM wParam, LPARAM)
//{
//	const ademco::AdemcoEvent* ademcoEvent = reinterpret_cast<const ademco::AdemcoEvent*>(wParam);
//	DispatchAdemcoEvent(ademcoEvent);
//	return 0;
//}


void CAlarmMachineDlg::OnBnClickedButtonArm()
{
	m_nRemoteControlTimeCounter = REMOTE_CONTROL_DISABLE_TIMEUP;
	m_curRemoteControlCommand = ademco::EVENT_ARM;
	KillTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
	OnTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
	SetTimer(TIMER_ID_REMOTE_CONTROL_MACHINE, 1000, NULL);

	CAlarmMachineManager* manager = CAlarmMachineManager::GetInstance();
	bool bsubmachine = m_machine->get_is_submachine();
	manager->RemoteControlAlarmMachine(m_machine, ademco::EVENT_ARM, 
									   bsubmachine ? INDEX_SUB_MACHINE : INDEX_ZONE,
									   bsubmachine ? m_machine->get_submachine_zone() : 0,
									   this);
}


void CAlarmMachineDlg::OnBnClickedButtonDisarm()
{
	m_nRemoteControlTimeCounter = REMOTE_CONTROL_DISABLE_TIMEUP;
	m_curRemoteControlCommand = ademco::EVENT_DISARM;
	KillTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
	OnTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
	SetTimer(TIMER_ID_REMOTE_CONTROL_MACHINE, 1000, NULL);

	CAlarmMachineManager* manager = CAlarmMachineManager::GetInstance();
	bool bsubmachine = m_machine->get_is_submachine();
	BOOL ok = manager->RemoteControlAlarmMachine(m_machine, ademco::EVENT_DISARM,
												 bsubmachine ? INDEX_SUB_MACHINE : INDEX_ZONE,
												 bsubmachine ? m_machine->get_submachine_zone() : 0,
												 this);
	if (!ok) {
		KillTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
		m_nRemoteControlTimeCounter = 0;
		OnTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
	}
}


void CAlarmMachineDlg::OnBnClickedButtonEmergency()
{
	m_nRemoteControlTimeCounter = REMOTE_CONTROL_DISABLE_TIMEUP;
	m_curRemoteControlCommand = ademco::EVENT_EMERGENCY;
	KillTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
	OnTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
	SetTimer(TIMER_ID_REMOTE_CONTROL_MACHINE, 1000, NULL);

	CAlarmMachineManager* manager = CAlarmMachineManager::GetInstance();
	bool bsubmachine = m_machine->get_is_submachine();
	manager->RemoteControlAlarmMachine(m_machine, ademco::EVENT_EMERGENCY, 
									   bsubmachine ? INDEX_SUB_MACHINE : INDEX_ZONE,
									   bsubmachine ? m_machine->get_submachine_zone() : 0,
									   this);
}


void CAlarmMachineDlg::OnBnClickedButtonClearmsg()
{
	if (m_machine) {
		m_machine->clear_ademco_event_list();
	}
}


afx_msg LRESULT CAlarmMachineDlg::OnNewrecordResult(WPARAM wParam, LPARAM /*lParam*/)
{
	HistoryRecord* record = reinterpret_cast<HistoryRecord*>(wParam);
	if (!record || !m_machine)
		return 0;

	int ademco_id = record->ademco_id;
	if (ademco_id != m_machine->get_ademco_id())
		return 0;

	if (m_machine->get_is_submachine()) {
		if (m_machine->get_submachine_zone() != record->zone_value)
			return 0;
	}

	m_listHistory.SetRedraw(FALSE);
	if (m_listHistory.GetCount() > m_maxHistory2Show) {
		m_listHistory.DeleteString(0);
	}
	m_listHistory.InsertString(-1, record->record);
	m_listHistory.SetRedraw();

	return 0;
}


void CAlarmMachineDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (TIMER_ID_TRAVERSE_ADEMCO_LIST == nIDEvent) {
		KillTimer(TIMER_ID_TRAVERSE_ADEMCO_LIST);
		if (m_machine)
			m_machine->TraverseAdmecoEventList(this, OnAdemcoEvent);
	} else if (TIMER_ID_REMOTE_CONTROL_MACHINE == nIDEvent) {
		if (m_nRemoteControlTimeCounter > 0) {
			m_btnArm.EnableWindow(0);
			m_btnDisarm.EnableWindow(0);
			m_btnEmergency.EnableWindow(0);
			CString s;
			switch (m_curRemoteControlCommand) {
				case ademco::EVENT_ARM:
					s.Format(L"%s(%d)", m_strBtnArm, m_nRemoteControlTimeCounter);
					m_btnArm.SetWindowTextW(s);
					break;
				case ademco::EVENT_DISARM:
					s.Format(L"%s(%d)", m_strBtnDisarm, m_nRemoteControlTimeCounter);
					m_btnDisarm.SetWindowTextW(s);
					break;
				case ademco::EVENT_EMERGENCY:
					s.Format(L"%s(%d)", m_strBtnEmergency, m_nRemoteControlTimeCounter);
					m_btnEmergency.SetWindowTextW(s);
					break;
				default:
					m_nRemoteControlTimeCounter = 0;
					return;
					break;
			}
			m_nRemoteControlTimeCounter--;
		} else {
			KillTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
			m_nRemoteControlTimeCounter = 0;
			m_btnArm.SetWindowTextW(m_strBtnArm);
			m_btnDisarm.SetWindowTextW(m_strBtnDisarm);
			m_btnEmergency.SetWindowTextW(m_strBtnEmergency);
			m_btnArm.EnableWindow(1);
			m_btnDisarm.EnableWindow(1);
			m_btnEmergency.EnableWindow(1);
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CAlarmMachineDlg::OnBnClickedButtonEditZone()
{
	AUTO_LOG_FUNCTION;
	m_machine->EnterBufferMode();
	CEditZoneDlg dlg;
	dlg.m_machine = m_machine;
	dlg.DoModal();
	if (dlg.m_bNeedReloadMaps)
		LoadMaps();
	m_machine->LeaveBufferMode();
}


afx_msg LRESULT CAlarmMachineDlg::OnInversionControl(WPARAM wParam, LPARAM lParam)
{
	AUTO_LOG_FUNCTION;
	CMapView* view = reinterpret_cast<CMapView*>(wParam);
	InversionControlMapCommand icmc = static_cast<InversionControlMapCommand>(lParam);
	if (ICMC_SHOW != icmc && ICMC_RENAME != icmc)
		return 0;

	MapViewWithNdx* mnTarget = NULL;
	std::list<MapViewWithNdx*>::iterator iter = m_mapViewList.begin();
	while (iter != m_mapViewList.end()) {
		MapViewWithNdx* mn = *iter++;
		if (mn->_mapView == view) { // found
			mnTarget = mn;
		} else {
			mn->_mapView->ShowWindow(SW_HIDE);
		}
	}

	if (mnTarget) {
		m_tab.SetCurSel(mnTarget->_ndx);
		mnTarget->_mapView->ShowWindow(SW_SHOW);
		if (ICMC_RENAME == icmc) {
			TCITEM tcItem;
			CString pszString = mnTarget->_mapView->m_mapInfo->get_alias();
			tcItem.mask = TCIF_TEXT;
			tcItem.pszText = pszString.LockBuffer();
			m_tab.SetItem(mnTarget->_ndx, &tcItem);
			pszString.UnlockBuffer();
			mnTarget->_mapView->Invalidate(0);
			//m_tab.Invalidate(0);
		}
		
	}
	
	return 0;
}


void CAlarmMachineDlg::OnBnClickedButtonEditMap()
{
	AUTO_LOG_FUNCTION;
	m_machine->EnterBufferMode();
	CEditMapDlg dlg;
	dlg.m_machine = m_machine;
	dlg.DoModal();
	if (dlg.m_bNeedReloadMaps)
		LoadMaps();
	m_machine->LeaveBufferMode();
}


void CAlarmMachineDlg::OnBnClickedButtonEditDetector()
{
	AUTO_LOG_FUNCTION;
	m_machine->EnterBufferMode();
	CEditDetectorDlg dlg;
	dlg.m_machine = m_machine;
	dlg.DoModal();
	m_machine->LeaveBufferMode();
}
