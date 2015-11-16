﻿// AlarmMachineDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AlarmMachineDlg.h"
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
#include "AlarmMachineContainer.h"
#include "QueryAllSubmachineDlg.h"
#include "InputDlg.h"
#include "RestoreMachineDlg.h"
#include "HistoryRecordDlg.h"
#include "PickMachineCoordinateDlg.h"
#include "VideoContainerDlg.h"
#include "SubMachineExpireManagerDlg.h"
#include "UserInfo.h"


using namespace gui;
using namespace ademco;
using namespace core;

static const int TIMER_ID_TRAVERSE_ADEMCO_LIST = 1;
static const int TIMER_ID_REMOTE_CONTROL_MACHINE = 2;
static const int TIMER_ID_HISTORY_RECORD = 3;
static const int TIMER_ID_CHECK_EXPIRE_TIME = 4;
static const int TIMER_ID_HANDLE_ADEMCO_EVENT = 5;

#ifdef _DEBUG
static const int REMOTE_CONTROL_DISABLE_TIMEUP = 60;
#else
static const int REMOTE_CONTROL_DISABLE_TIMEUP = 60;
#endif

static void __stdcall OnNewRecord(void* udata, const HistoryRecord* record)
{
	CAlarmMachineDlg* dlg = reinterpret_cast<CAlarmMachineDlg*>(udata); 
	if (!dlg || !dlg->m_machine)
		return;
	if (!record->record.IsEmpty()) {
		int ademco_id = record->ademco_id;
		if (ademco_id != dlg->m_machine->get_ademco_id())
			return;

		if (dlg->m_machine->get_is_submachine()) {
			if (dlg->m_machine->get_submachine_zone() != record->zone_value)
				return;
		}
	}
	
	dlg->m_lock4RecordList.Lock();
	dlg->m_recordList.AddTail(record->record);
	dlg->m_lock4RecordList.UnLock();
}


IMPLEMENT_ADEMCO_EVENT_CALL_BACK(CAlarmMachineDlg, OnAdemcoEvent)

// CAlarmMachineDlg dialog

IMPLEMENT_DYNAMIC(CAlarmMachineDlg, CDialogEx)

CAlarmMachineDlg::CAlarmMachineDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CAlarmMachineDlg::IDD, pParent)
	, m_machine(nullptr)
	, m_maxHistory2Show(0)
	, m_nRemoteControlTimeCounter(0)
	, m_curRemoteControlCommand(0)
	, m_strBtn1(L"")
	, m_strBtn2(L"")
	, m_strBtn3(L"")
	, m_container(nullptr)
	, m_videoContainerDlg(nullptr)
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
	DDX_Control(pDX, IDC_BUTTON_ARM, m_btn1);
	DDX_Control(pDX, IDC_BUTTON_DISARM, m_btn2);
	DDX_Control(pDX, IDC_BUTTON_EMERGENCY, m_btn3);
	DDX_Control(pDX, IDC_BUTTON_CLEARMSG, m_btnClearMsg);
	DDX_Control(pDX, IDC_STATIC_NET, m_staticNet);
	DDX_Control(pDX, IDC_STATIC_STATUS, m_staticStatus);
	DDX_Control(pDX, IDC_TAB1, m_tab);
	DDX_Control(pDX, IDC_LIST_HISTORY, m_listHistory);
	DDX_Control(pDX, IDC_STATIC_HISTORY, m_groupHistory);
	DDX_Control(pDX, IDC_BUTTON_EDIT_VIDEO, m_btnEditVideoInfo);
	DDX_Control(pDX, IDC_STATIC_CONN, m_staticConn);
	DDX_Control(pDX, IDC_STATIC_MACHINE_STATUS, m_staticMachineStatus);
	DDX_Control(pDX, IDC_BUTTON_MORE_HR, m_btnSeeMoreHr);
	DDX_Control(pDX, IDC_BUTTON_MANAGE_EXPIRE, m_btnManageExpire);
	DDX_Control(pDX, IDC_BUTTON_EDIT_ZONE, m_btnEditZone);
	DDX_Control(pDX, IDC_BUTTON_EDIT_MAP, m_btnEditMap);
	DDX_Control(pDX, IDC_BUTTON_EDIT_DETECTOR, m_btnEditDetector);
}


BEGIN_MESSAGE_MAP(CAlarmMachineDlg, CDialogEx)
	ON_WM_DESTROY()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CAlarmMachineDlg::OnTcnSelchangeTab)
	ON_BN_CLICKED(IDC_BUTTON_ARM, &CAlarmMachineDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON_DISARM, &CAlarmMachineDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON_EMERGENCY, &CAlarmMachineDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON_CLEARMSG, &CAlarmMachineDlg::OnBnClickedButtonClearmsg)
	ON_MESSAGE(WM_NEWRECORD, &CAlarmMachineDlg::OnNewrecordResult)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_EDIT_ZONE, &CAlarmMachineDlg::OnBnClickedButtonEditZone)
	ON_MESSAGE(WM_INVERSIONCONTROL, &CAlarmMachineDlg::OnInversionControl)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_MAP, &CAlarmMachineDlg::OnBnClickedButtonEditMap)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_DETECTOR, &CAlarmMachineDlg::OnBnClickedButtonEditDetector)
	ON_BN_CLICKED(IDC_BUTTON_MORE_HR, &CAlarmMachineDlg::OnBnClickedButtonMoreHr)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_SEE_BAIDU_MAP, &CAlarmMachineDlg::OnBnClickedButtonSeeBaiduMap)
	ON_BN_CLICKED(IDC_BUTTON_MANAGE_EXPIRE, &CAlarmMachineDlg::OnBnClickedButtonManageExpire)
END_MESSAGE_MAP()


// CAlarmMachineDlg message handlers


void CAlarmMachineDlg::SetMachineInfo(CAlarmMachine* machine)
{
	m_machine = machine;
}


static void __stdcall OnCurUserChanged(void* udata, const core::CUserInfo* user)
{
	if (!udata || !user)
		return;

	CAlarmMachineDlg* dlg = reinterpret_cast<CAlarmMachineDlg*>(udata);
	dlg->OnCurUserChangedResult(user);
}


void CAlarmMachineDlg::OnCurUserChangedResult(const core::CUserInfo* user)
{
	if (user->get_user_priority() == core::UP_OPERATOR) {
		m_btnManageExpire.EnableWindow(0);
		m_btnEditZone.EnableWindow(0);
		m_btnEditMap.EnableWindow(0);
		m_btnEditDetector.EnableWindow(0);
		m_btnEditVideoInfo.EnableWindow(0);
		m_btn2.EnableWindow(0);
	} else {
		if (m_machine) {
			if (!m_machine->get_is_submachine()) {
				if (MT_NETMOD == m_machine->get_machine_type())
					m_btn2.EnableWindow(1);
				m_btnManageExpire.EnableWindow(1);
			}
			if (m_machine->get_has_video())
				m_btnEditVideoInfo.EnableWindow(1);
		}
		m_btnEditZone.EnableWindow(1);
		m_btnEditMap.EnableWindow(1);
		m_btnEditDetector.EnableWindow(1);
		
	}
}


BOOL CAlarmMachineDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	if (!m_machine) {
		return FALSE;
	}

	CRect rc(0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN));
	rc.DeflateRect(25, 25, 25, 25);
	MoveWindow(rc);

	GetClientRect(rc);
	rc.DeflateRect(5, 5, 5, 5);
	CRect rcLeft(rc);
	rcLeft.right = rcLeft.left + 180;
	CRect rcRight(rc);
	rcRight.left = rcLeft.right + 5;
	m_groupControlPanel.MoveWindow(rcLeft);
	m_groupContent.ShowWindow(SW_HIDE);
	m_tab.MoveWindow(rcRight);

	CRect rcBtn;
	m_btnEditVideoInfo.GetWindowRect(rcBtn);
	ScreenToClient(rcBtn);
	CRect rcHistory(rcLeft);
	rcHistory.top = rcBtn.bottom + 5;
	m_groupHistory.MoveWindow(rcHistory);
	m_btnSeeMoreHr.GetWindowRect(rcBtn);
	int btnHeight = rcBtn.Height();
	int btnWidth = rcBtn.Width();
	rcBtn.top = rcHistory.top - 4;
	rcBtn.bottom = rcBtn.top + btnHeight;
	rcBtn.right = rcHistory.right - 2;
	rcBtn.left = rcBtn.right - btnWidth;
	m_btnSeeMoreHr.MoveWindow(rcBtn);
	rcHistory.DeflateRect(5, 18, 5, 5);
	m_listHistory.MoveWindow(rcHistory);
	m_listHistory.GetWindowRect(rcHistory);
	int columnHeight = m_listHistory.GetItemHeight(0);
	m_maxHistory2Show = rcHistory.Height() / columnHeight - 2;

	UpdateBtn123();

	m_btnEditVideoInfo.EnableWindow(m_machine->get_has_video());

	m_tab.ShowWindow(SW_SHOW);

	m_btn1.GetWindowTextW(m_strBtn1);
	m_btn2.GetWindowTextW(m_strBtn2);
	m_btn3.GetWindowTextW(m_strBtn3);
	
	UpdateCaption();
	

	// 1. register callback
	m_machine->RegisterObserver(this, OnAdemcoEvent);

	// 2. update icon
	if (m_machine->get_online()) {
		m_staticNet.SetIcon(CAppResource::m_hIconNetOk);
	} else {
		m_staticNet.SetIcon(CAppResource::m_hIconNetFailed);
	}

	if (m_machine->get_armed()) {
		m_staticStatus.SetIcon(CAppResource::m_hIconArm);
	} else {
		m_staticStatus.SetIcon(CAppResource::m_hIconDisarm);
	}

	CString text, smachine, sstatus;
	sstatus.LoadStringW(IDS_STRING_MACHINE_STATUS);
	if (m_machine->get_is_submachine()) {
		text.LoadStringW(IDS_STRING_SLAVE_CONN);
		m_staticConn.SetWindowTextW(text);
		smachine.LoadStringW(IDS_STRING_SUBMACHINE);
		m_btnManageExpire.EnableWindow(0);
	} else {
		smachine.LoadStringW(IDS_STRING_MACHINE);
		m_btnManageExpire.EnableWindow();
	}
	m_staticMachineStatus.SetWindowTextW(smachine + sstatus);

	core::CUserManager::GetInstance()->RegisterObserver(this, OnCurUserChanged);
	OnCurUserChanged(this, core::CUserManager::GetInstance()->GetCurUserInfo());

	// 3. load map info
	LoadMaps();

	// 4. setup history callback
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

	// 5. setup timers
	SetTimer(TIMER_ID_TRAVERSE_ADEMCO_LIST, 100, nullptr);
	SetTimer(TIMER_ID_HISTORY_RECORD, 1000, nullptr);
	SetTimer(TIMER_ID_HANDLE_ADEMCO_EVENT, 1000, nullptr);
	SetTimer(TIMER_ID_CHECK_EXPIRE_TIME, 3000, nullptr);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CAlarmMachineDlg::UpdateCaption()
{
	CString text = L"", fmMachine, fmSubMachine, fmAlias, fmContact,
		fmAddress, fmPhone, fmPhoneBk, fmNull;
	CString sid;
	fmMachine.LoadStringW(IDS_STRING_MACHINE);
	fmSubMachine.LoadStringW(IDS_STRING_SUBMACHINE);
	fmAlias.LoadStringW(IDS_STRING_ALIAS);
	fmContact.LoadStringW(IDS_STRING_CONTACT);
	fmAddress.LoadStringW(IDS_STRING_ADDRESS);
	fmPhone.LoadStringW(IDS_STRING_PHONE);
	fmPhoneBk.LoadStringW(IDS_STRING_PHONE_BK);
	fmNull.LoadStringW(IDS_STRING_NULL);
	

	if (m_machine->get_is_submachine()) {
		sid.Format(L"%s%03d", fmSubMachine, m_machine->get_submachine_zone());
		
	} else {
		sid.Format(L"%s%04d", fmMachine, m_machine->get_ademco_id());
	}

	text.Format(L"%s    %s:%s    %s:%s    %s:%s    %s:%s    %s:%s",
				sid,
				fmAlias, m_machine->get_alias(),
				fmContact, m_machine->get_contact(),
				fmAddress, m_machine->get_address(),
				fmPhone, m_machine->get_phone(),
				fmPhoneBk, m_machine->get_phone_bk());
	SetWindowText(text);
}


void CAlarmMachineDlg::CheckIfExpire()
{
	COleDateTime now = COleDateTime::GetCurrentTime();
	COleDateTime expire = m_machine->get_expire_time();
	COleDateTimeSpan span = expire - now;
	double mins = span.GetTotalMinutes();
	if (mins <= 0) {
		CString s, e; e.LoadStringW(IDS_STRING_EXPIRE);
		s.Format(L"%s\r\n%04d(%s)", e, m_machine->get_ademco_id(), m_machine->get_alias());
		MessageBox(s);
	}
}


void CAlarmMachineDlg::UpdateBtn123()
{
	CString btnText;
	MachineType mt = m_machine->get_machine_type();
	if (MT_NETMOD != mt) {
		btnText.LoadStringW(IDS_STRING_BK_BTN);
		m_btn1.EnableWindow(0);
		m_btn2.EnableWindow(0);
		m_btn3.EnableWindow(0);
		m_btn1.SetWindowTextW(btnText + L" 1");
		m_btn2.SetWindowTextW(btnText + L" 2");
		m_btn3.SetWindowTextW(btnText + L" 3");
		return;
	}

	if (m_machine->get_is_submachine()) {
		btnText.LoadStringW(IDS_STRING_QUERY);
		m_btn1.SetWindowTextW(btnText);
		m_btn1.EnableWindow();

		btnText.LoadStringW(IDS_STRING_BK_BTN);
		m_btn2.SetWindowTextW(btnText + L" 1");
		m_btn3.SetWindowTextW(btnText + L" 2");
	} else {
		CString fmAllSubMachine;
		fmAllSubMachine.LoadStringW(IDS_STRING_ALL_SUBMACHINE);
		btnText.LoadStringW(IDS_STRING_QUERY);
		m_btn1.SetWindowTextW(btnText + fmAllSubMachine);
		m_btn1.EnableWindow();

		btnText.LoadStringW(IDS_STRING_WRITE2MACHINE);
		m_btn2.SetWindowTextW(btnText);
		if (core::CUserManager::GetInstance()->GetCurUserInfo()->get_user_priority() != UP_OPERATOR)
			m_btn2.EnableWindow();

		btnText.LoadStringW(IDS_STRING_BK_BTN);
		m_btn3.SetWindowTextW(btnText + L" 1");
	}
}


void CAlarmMachineDlg::LoadMaps()
{
	AUTO_LOG_FUNCTION;
	CRect rcTab;
	m_tab.GetClientRect(rcTab);
	rcTab.DeflateRect(5, 25, 5, 5);
	int prevSel = 0;
	CWnd* prevShowTab = nullptr;
	if (m_tab.GetItemCount() > 0) {
		prevSel = m_tab.GetCurSel();
		ReleaseMaps();
	}

	int nItem = 0;	
	if (!m_machine->get_is_submachine()) {
		// sub machines
		CString sAllSubMachine; sAllSubMachine.LoadStringW(IDS_STRING_ALL_SUBMACHINE);
		m_container = new CAlarmMachineContainerDlg();
		m_container->Create(IDD_DIALOG_CONTAINER, &m_tab);
		m_container->MoveWindow(rcTab, FALSE);
		m_container->ShowWindow(SW_HIDE);
		CZoneInfoList zoneList;
		m_machine->GetAllZoneInfo(zoneList);
		for (auto zoneInfo : zoneList) {
			CAlarmMachine* subMachineInfo = zoneInfo->GetSubMachineInfo();
			if (subMachineInfo) {
				m_container->InsertMachine(subMachineInfo);
			}
		}
		nItem = m_tab.InsertItem(nItem, sAllSubMachine);
		if (m_machine->get_alarmingSubMachineCount() > 0) {
			m_tab.HighlightItem(nItem, TRUE);
		}
		TabViewWithNdx* tvn = new TabViewWithNdx(m_container, nItem);
		m_tabViewList.push_back(tvn);
		if (prevSel == nItem) {
			prevShowTab = m_container;
		}
		nItem++;
	}

	// map contains unbind zone
	CMapInfo* unbindZoneMapInfo = m_machine->GetUnbindZoneMap();
	if (unbindZoneMapInfo) {
		CMapView* mapView = new CMapView();
		mapView->SetRealParentWnd(this);
		mapView->SetMachineInfo(m_machine);
		mapView->SetMapInfo(unbindZoneMapInfo);
		mapView->Create(IDD_DIALOG_MAPVIEW, &m_tab);
		mapView->MoveWindow(rcTab, FALSE);
		mapView->ShowWindow(SW_HIDE);

		nItem = m_tab.InsertItem(nItem, unbindZoneMapInfo->get_alias());
		TabViewWithNdx* tvn = new TabViewWithNdx(mapView, nItem);
		m_tabViewList.push_back(tvn);
		if (prevSel == nItem) {
			prevShowTab = mapView;
		}
		nItem++;
	}

	// normal maps
	CMapInfoList list;
	m_machine->GetAllMapInfo(list);
	for (auto mapInfo : list) {
		CMapView* mapView = new CMapView();
		mapView->SetRealParentWnd(this);
		mapView->SetMachineInfo(m_machine);
		mapView->SetMapInfo(mapInfo);
		mapView->Create(IDD_DIALOG_MAPVIEW, &m_tab);
		mapView->MoveWindow(rcTab, FALSE);
		mapView->ShowWindow(SW_HIDE);

		nItem = m_tab.InsertItem(nItem, mapInfo->get_alias());
		TabViewWithNdx* tvn = new TabViewWithNdx(mapView, nItem);
		m_tabViewList.push_back(tvn);
		if (prevSel == nItem) {
			prevShowTab = mapView;
		}
		nItem++;
	}

	m_tab.SetCurSel(prevSel);
	if (m_tabViewList.size() > 0) {
		if (prevShowTab) {
			prevShowTab->ShowWindow(SW_SHOW);
		} else {
			TabViewWithNdx* tvn = m_tabViewList.front();
			tvn->_tabView->ShowWindow(SW_SHOW);
		}
	}
}


void CAlarmMachineDlg::ReleaseMaps()
{
	m_tab.DeleteAllItems();

	for (auto tvn : m_tabViewList) {
		tvn->_tabView->DestroyWindow();
		delete tvn->_tabView;
		delete tvn;
	}
	m_tabViewList.clear();
	m_container = nullptr;
}


void CAlarmMachineDlg::OnDestroy()
{
	AUTO_LOG_FUNCTION;
	CDialogEx::OnDestroy();
	CHistoryRecord* hr = CHistoryRecord::GetInstance();
	hr->UnRegisterObserver(this);

	core::CUserManager::GetInstance()->UnRegisterObserver(this);

	if (m_machine) {
		m_machine->UnRegisterObserver(this);
		m_machine = nullptr;
	}

	KillTimer(TIMER_ID_TRAVERSE_ADEMCO_LIST);
	KillTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
	KillTimer(TIMER_ID_HISTORY_RECORD);
	KillTimer(TIMER_ID_CHECK_EXPIRE_TIME);
	KillTimer(TIMER_ID_HANDLE_ADEMCO_EVENT);

	for (auto ademcoEvent : _ademcoEventList) {
		delete ademcoEvent;
	}
	_ademcoEventList.clear();

	ReleaseMaps();
}


void CAlarmMachineDlg::OnAdemcoEventResult(const ademco::AdemcoEvent* ademcoEvent)
{
	AUTO_LOG_FUNCTION;
	ASSERT(ademcoEvent);
	if (nullptr == m_machine)
		return;

	if (ademco::EVENT_IM_GONNA_DIE == ademcoEvent->_event) {
		m_machine = nullptr;
		return;
	}

	m_lock4AdemcoEventList.Lock();
	_ademcoEventList.push_back(new AdemcoEvent(*ademcoEvent));
	m_lock4AdemcoEventList.UnLock();

	
}


int CAlarmMachineDlg::GetAdemcoID() const
{
	if (m_machine) {
		return m_machine->get_ademco_id();
	}
	return -1;
}


int CAlarmMachineDlg::GetZoneValue() const
{
	if (m_machine) {
		return m_machine->get_is_submachine() ? m_machine->get_submachine_zone() : 0;
	}
	return -1;
}


void CAlarmMachineDlg::OnTcnSelchangeTab(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	int ndx = m_tab.GetCurSel();
	if (ndx == -1)return;

	for (auto tvn : m_tabViewList) {
		if (tvn->_ndx == ndx) { // found
			tvn->_tabView->ShowWindow(SW_SHOW);
		} else {
			tvn->_tabView->ShowWindow(SW_HIDE);
		}
	}

	*pResult = 0;
}


void CAlarmMachineDlg::OnBnClickedButton1()
{
	bool bsubmachine = m_machine->get_is_submachine();
	m_nRemoteControlTimeCounter = REMOTE_CONTROL_DISABLE_TIMEUP;
	m_curRemoteControlCommand = ademco::EVENT_QUERY_SUB_MACHINE;

	if (bsubmachine) {
		KillTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
		OnTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
		SetTimer(TIMER_ID_REMOTE_CONTROL_MACHINE, 1000, nullptr);

		CAlarmMachineManager* manager = CAlarmMachineManager::GetInstance();
		manager->RemoteControlAlarmMachine(m_machine, ademco::EVENT_QUERY_SUB_MACHINE,
										   INDEX_SUB_MACHINE,
										   m_machine->get_submachine_zone(),
										   nullptr, 0, this);
	} else {
		if (m_machine->get_submachine_count() == 0) {
			CString e; e.LoadStringW(IDS_STRING_E_MACHINE_NO_SUB);
			MessageBox(e, L"", MB_ICONINFORMATION);
			return;
		}
		CQueryAllSubmachineDlg dlg(this);
		dlg.m_machine = m_machine;
		dlg.DoModal();
	}
}


void CAlarmMachineDlg::OnBnClickedButton2()
{
	bool bsubmachine = m_machine->get_is_submachine();
	if (bsubmachine) {
		m_nRemoteControlTimeCounter = REMOTE_CONTROL_DISABLE_TIMEUP;
		m_curRemoteControlCommand = ademco::EVENT_DISARM;
		KillTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
		OnTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
		SetTimer(TIMER_ID_REMOTE_CONTROL_MACHINE, 1000, nullptr);

		CAlarmMachineManager* manager = CAlarmMachineManager::GetInstance();
		char xdata[64] = { 0 };
		int xdata_len = 0;
		if (!m_machine->get_is_submachine()) {
			CInputDlg dlg(this);
			if (dlg.DoModal() != IDOK)
				return;
			if (dlg.m_edit.GetLength() != 6)
				return;

			USES_CONVERSION;
			strcpy_s(xdata, W2A(dlg.m_edit));
			xdata_len = strlen(xdata);
		}
		BOOL ok = manager->RemoteControlAlarmMachine(m_machine, ademco::EVENT_DISARM,
													 bsubmachine ? INDEX_SUB_MACHINE : INDEX_ZONE,
													 bsubmachine ? m_machine->get_submachine_zone() : 0,
													 xdata, xdata_len, this);
		if (!ok) {
			KillTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
			m_nRemoteControlTimeCounter = 0;
			OnTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
		}
	} else {
		if (m_machine->get_zone_count() == 0) {
			CString e; e.LoadStringW(IDS_STRING_E_MACHINE_NO_ZONE);
			MessageBox(e, L"", MB_ICONINFORMATION);
			return;
		} else {
			int wire_zone_cnt = 0;
			CZoneInfo* wireZone = nullptr;
			for (int i = WIRE_ZONE_RANGE_BEG; i <= WIRE_ZONE_RANGE_END; i++) {
				wireZone = m_machine->GetZone(i);
				if (wireZone)
					wire_zone_cnt++;
			}
			if (wire_zone_cnt == m_machine->get_zone_count()) {
				CString e; e.LoadStringW(IDS_STRING_WIRE_ZONE_NO_NEED_RESTORE);
				MessageBox(e, L"", MB_ICONINFORMATION);
				return;
			}
		}
		CRestoreMachineDlg dlg(this);
		dlg.m_machine = m_machine;
		dlg.DoModal();
	}
	
}


void CAlarmMachineDlg::OnBnClickedButton3()
{
	if (!m_machine->get_is_submachine()) {
#ifdef _DEBUG
		static int ndx = 0;
		m_tab.HighlightItem(ndx++);
#endif
		return;
	}
	m_nRemoteControlTimeCounter = REMOTE_CONTROL_DISABLE_TIMEUP;
	m_curRemoteControlCommand = ademco::EVENT_EMERGENCY;
	KillTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
	OnTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
	SetTimer(TIMER_ID_REMOTE_CONTROL_MACHINE, 1000, nullptr);
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

	if (RECORD_LEVEL_CLEARHR == record->level) {
		m_listHistory.ResetContent();
		return 0;
	}

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
		if (--m_nRemoteControlTimeCounter > 0) {
			m_btn1.EnableWindow(0);
			m_btn2.EnableWindow(0);
			m_btn3.EnableWindow(0);
			CString s;
			switch (m_curRemoteControlCommand) {
				case ademco::EVENT_QUERY_SUB_MACHINE:
					s.Format(L"%s(%d)", m_strBtn1, m_nRemoteControlTimeCounter);
					m_btn1.SetWindowTextW(s);
					if (m_nRemoteControlTimeCounter % (REMOTE_CONTROL_DISABLE_TIMEUP / 3) == 0) {
						CAlarmMachineManager* manager = CAlarmMachineManager::GetInstance();
						manager->RemoteControlAlarmMachine(m_machine, 
														   EVENT_QUERY_SUB_MACHINE,
														   INDEX_SUB_MACHINE,
														   m_machine->get_submachine_zone(),
														   nullptr, 0, this);
					}
					break;
				default:
					m_nRemoteControlTimeCounter = 0;
					return;
					break;
			}
		} else {
			KillTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
			m_nRemoteControlTimeCounter = 0;
			if (EVENT_QUERY_SUB_MACHINE == m_curRemoteControlCommand) {
				CString e; e.LoadStringW(IDS_STRING_QUERY_FAILED);
				MessageBox(e, L"", MB_ICONERROR);
				CHistoryRecord::GetInstance()->InsertRecord(m_machine->get_ademco_id(),
															m_machine->get_is_submachine() ? m_machine->get_submachine_zone() : 0,
															e, time(nullptr), RECORD_LEVEL_USERCONTROL);
				m_machine->set_online(false);
				m_machine->SetAdemcoEvent(ES_UNKNOWN, EVENT_OFFLINE, m_machine->get_submachine_zone(),
										  INDEX_SUB_MACHINE, time(nullptr),time(nullptr), nullptr, 0);
			}
			UpdateBtn123();
		}
	} else if (TIMER_ID_HISTORY_RECORD == nIDEvent) {
		if (m_lock4RecordList.TryLock()) {
			m_listHistory.SetRedraw(0);
			while (m_recordList.GetCount() > 0) {
				CString record = m_recordList.RemoveHead();
				if (record.IsEmpty()) {
					m_listHistory.ResetContent();
					break;
				}

				if (m_listHistory.GetCount() > m_maxHistory2Show)
					m_listHistory.DeleteString(0);
				m_listHistory.InsertString(-1, record);
			}
			m_listHistory.SetRedraw();
			m_lock4RecordList.UnLock();
		}
	} else if (TIMER_ID_CHECK_EXPIRE_TIME == nIDEvent) {
		KillTimer(TIMER_ID_CHECK_EXPIRE_TIME);
		CheckIfExpire();
		SetTimer(TIMER_ID_CHECK_EXPIRE_TIME, 60 * 1000, nullptr);
	} else if (TIMER_ID_HANDLE_ADEMCO_EVENT == nIDEvent){
		if (m_lock4AdemcoEventList.TryLock()) {
			while (_ademcoEventList.size() > 0){
				AdemcoEvent* ademcoEvent = _ademcoEventList.front();
				_ademcoEventList.pop_front();
				HandleAdemcoEvent(ademcoEvent);
				delete ademcoEvent;
			}
			m_lock4AdemcoEventList.UnLock();
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CAlarmMachineDlg::HandleAdemcoEvent(const ademco::AdemcoEvent* ademcoEvent) 
{
	bool bsubmachine_status = ademcoEvent->_sub_zone != INDEX_ZONE;
	if (bsubmachine_status != m_machine->get_is_submachine()) {
		if (!m_machine->get_is_submachine()) {
			if (m_container) {
				TabViewWithNdx* mnTarget = nullptr;
				for (auto tvn : m_tabViewList) {
					if (tvn->_tabView == m_container) { // found
						mnTarget = tvn;
					} else {
						tvn->_tabView->ShowWindow(SW_HIDE);
					}
				}

				if (mnTarget) {
					m_tab.SetCurSel(mnTarget->_ndx);
					mnTarget->_tabView->ShowWindow(SW_SHOW);
					if (m_machine->get_alarmingSubMachineCount() <= 0) { // should never be < 0
						m_tab.HighlightItem(mnTarget->_ndx, FALSE);
					} else {
						m_tab.HighlightItem(mnTarget->_ndx, TRUE);
					}
				}
			}
		} else {
			if (ademcoEvent->_event == EVENT_I_AM_NET_MODULE) {
				UpdateBtn123();
			}
		}
		return;
	}

	switch (ademcoEvent->_event) {
	case EVENT_CLEARMSG:
		ClearMsg();
		break;
	case ademco::EVENT_OFFLINE:
		m_staticNet.SetIcon(CAppResource::m_hIconNetFailed);
		break;
	case ademco::EVENT_ONLINE:
		m_staticNet.SetIcon(CAppResource::m_hIconNetOk);
		break;
	case ademco::EVENT_DISARM:
		m_staticNet.SetIcon(CAppResource::m_hIconNetOk);
		m_staticStatus.SetIcon(CAppResource::m_hIconDisarm);
		KillTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
		if (m_nRemoteControlTimeCounter > 0) {
			CString i; i.LoadStringW(IDS_STRING_QUERY_SUCCESS);
			CHistoryRecord::GetInstance()->InsertRecord(m_machine->get_ademco_id(),
														m_machine->get_is_submachine() ? m_machine->get_submachine_zone() : 0,
														i, time(nullptr), RECORD_LEVEL_USERCONTROL);
			m_nRemoteControlTimeCounter = 0;
		}
		UpdateBtn123();
		break;
	case ademco::EVENT_ARM:
		m_staticNet.SetIcon(CAppResource::m_hIconNetOk);
		m_staticStatus.SetIcon(CAppResource::m_hIconArm);
		KillTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
		if (m_nRemoteControlTimeCounter > 0) {
			CString i; i.LoadStringW(IDS_STRING_QUERY_SUCCESS);
			CHistoryRecord::GetInstance()->InsertRecord(m_machine->get_ademco_id(),
														m_machine->get_is_submachine() ? m_machine->get_submachine_zone() : 0,
														i, time(nullptr), RECORD_LEVEL_USERCONTROL);
			m_nRemoteControlTimeCounter = 0;
		}
		UpdateBtn123();
		break;
	case ademco::EVENT_EMERGENCY:
		KillTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
		m_nRemoteControlTimeCounter = 0;
		OnTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
		break;
	case ademco::EVENT_SUBMACHINECNT:
		break;
	case EVENT_I_AM_NET_MODULE:
		UpdateBtn123();
		break;
	case EVENT_MACHINE_ALIAS:
		UpdateCaption();
		break;
	default:	// means its alarming
		break;
	}
}


void CAlarmMachineDlg::ClearMsg()
{
	for (int i = 0; i < m_tab.GetItemCount(); i++) {
		m_tab.HighlightItem(i, FALSE);
	}
}


void CAlarmMachineDlg::OnBnClickedButtonEditZone()
{
	AUTO_LOG_FUNCTION;
	CEditZoneDlg dlg;
	dlg.m_machine = m_machine;
	dlg.m_machineDlg = this;
	dlg.DoModal();
	if (dlg.m_bNeedReloadMaps)
		LoadMaps();
	//while (!m_machine->LeaveBufferMode()) { Sleep(100); }
}


afx_msg LRESULT CAlarmMachineDlg::OnInversionControl(WPARAM wParam, LPARAM lParam)
{
	AUTO_LOG_FUNCTION;
	CMapView* view = reinterpret_cast<CMapView*>(wParam);
	InversionControlMapCommand icmc = static_cast<InversionControlMapCommand>(lParam);
	if (ICMC_SHOW != icmc && ICMC_RENAME != icmc && ICMC_ADD_ALARM_TEXT != icmc && ICMC_CLR_ALARM_TEXT != icmc)
		return 0;

	TabViewWithNdx* mnTarget = nullptr;
	for (auto tvn : m_tabViewList) {
		if (tvn->_tabView == view) { // found
			mnTarget = tvn;
		} else {
			tvn->_tabView->ShowWindow(SW_HIDE);
		}
	}

	if (mnTarget) {
		if (m_tab.GetCurSel() != mnTarget->_ndx) {
			m_tab.SetCurSel(mnTarget->_ndx);
			mnTarget->_tabView->ShowWindow(SW_SHOW);
		}
		if (ICMC_CLR_ALARM_TEXT == icmc) {
			m_tab.HighlightItem(mnTarget->_ndx, FALSE);
		}
		if (ICMC_ADD_ALARM_TEXT == icmc) {
			m_tab.HighlightItem(mnTarget->_ndx, TRUE);
		}
		if (ICMC_RENAME == icmc) {
			TCITEM tcItem;
			CString pszString = view->m_mapInfo->get_alias();
			tcItem.mask = TCIF_TEXT;
			tcItem.pszText = pszString.LockBuffer();
			m_tab.SetItem(mnTarget->_ndx, &tcItem);
			pszString.UnlockBuffer();
			mnTarget->_tabView->Invalidate(0);
			//m_tab.Invalidate(0);
		}
	}
	
	return 0;
}


void CAlarmMachineDlg::OnBnClickedButtonEditMap()
{
	AUTO_LOG_FUNCTION;
	DWORD start = GetTickCount();
	while (!m_machine->EnterBufferMode()) {
		if (GetTickCount() - start > 3000) {
			CString e; e.LoadStringW(IDS_STRING_MACHINE_BUSY);
			MessageBox(e, L"", MB_OK | MB_ICONINFORMATION);
			return;
		}
		Sleep(100);
	}
	CEditMapDlg dlg;
	dlg.m_machine = m_machine;
	dlg.DoModal();
	if (dlg.m_bNeedReloadMaps)
		LoadMaps();
	while (!m_machine->LeaveBufferMode()) { Sleep(100); }
}


void CAlarmMachineDlg::OnBnClickedButtonEditDetector()
{
	AUTO_LOG_FUNCTION;
	DWORD start = GetTickCount();
	while (!m_machine->EnterBufferMode()) {
		if (GetTickCount() - start > 3000) {
			CString e; e.LoadStringW(IDS_STRING_MACHINE_BUSY);
			MessageBox(e, L"", MB_OK | MB_ICONINFORMATION);
			return;
		}
		Sleep(100);
	}
	CEditDetectorDlg dlg;
	dlg.m_machine = m_machine;
	dlg.DoModal();
	while (!m_machine->LeaveBufferMode()) { Sleep(100); }
}


void CAlarmMachineDlg::OnBnClickedButtonMoreHr()
{
	CHistoryRecordDlg dlg; 
	dlg.m_ademco_id = m_machine->get_ademco_id();
	if (m_machine->get_is_submachine()) {
		dlg.m_zone_value = m_machine->get_submachine_zone();
	}
	dlg.DoModal();
}


void CAlarmMachineDlg::OnClose()
{
	//OnDestroy();
	ShowWindow(SW_HIDE);
	//CDialogEx::OnCancel();
}


void CAlarmMachineDlg::OnBnClickedButtonSeeBaiduMap()
{
	if (m_machine && g_baiduMapDlg && IsWindow(g_baiduMapDlg->m_hWnd)) {
		g_baiduMapDlg->ShowMap(m_machine);
	}
}


void CAlarmMachineDlg::OnBnClickedButtonManageExpire()
{
	AUTO_LOG_FUNCTION;
	if (m_machine->get_is_submachine()) return;
	CMachineExpireManagerDlg dlg;
	CZoneInfoList list;
	m_machine->GetAllZoneInfo(list);
	std::list<CAlarmMachine*> machineList;
	for (auto zoneInfo : list) {
		CAlarmMachine* subMachine = zoneInfo->GetSubMachineInfo();
		if (subMachine) {
			machineList.push_back(subMachine);
		}
	}
	dlg.SetExpiredMachineList(machineList);
	dlg.DoModal();
}
