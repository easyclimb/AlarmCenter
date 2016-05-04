// AlarmMachineDlg.cpp : implementation file
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
#include "BaiduMapViewerDlg.h"
#include "VideoContainerDlg.h"
#include "SubMachineExpireManagerDlg.h"
#include "UserInfo.h"
#include "EditCameraDlg.h"


using namespace gui;
using namespace ademco;
using namespace core;


namespace detail {
	const int TIMER_ID_TRAVERSE_ADEMCO_LIST = 1;
	const int TIMER_ID_REMOTE_CONTROL_MACHINE = 2;
	const int TIMER_ID_HISTORY_RECORD = 3;
	const int TIMER_ID_CHECK_EXPIRE_TIME = 4;
	const int TIMER_ID_HANDLE_ADEMCO_EVENT = 5;

#ifdef _DEBUG
	const int REMOTE_CONTROL_DISABLE_TIMEUP = 60;
#else
	const int REMOTE_CONTROL_DISABLE_TIMEUP = 60;
#endif

};
using namespace detail;

class CAlarmMachineDlg::NewRecordObserver : public dp::observer<core::history_record_ptr>
{
public:
	explicit NewRecordObserver(CAlarmMachineDlg* dlg) : _dlg(dlg) {}
	virtual void on_update(const core::history_record_ptr& ptr) {
		if (_dlg) {
			if (!_dlg || !_dlg->m_machine)
				return;
			if (!ptr->record.IsEmpty()) {
				int ademco_id = ptr->ademco_id;
				if (ademco_id != _dlg->m_machine->get_ademco_id())
					return;

				if (_dlg->m_machine->get_is_submachine()) {
					if (_dlg->m_machine->get_submachine_zone() != ptr->zone_value)
						return;
				}
			}

			std::lock_guard<std::mutex> lock(_dlg->m_lock4RecordList);
			_dlg->m_recordList.AddTail(ptr->record);
		}
	}
private:
	CAlarmMachineDlg* _dlg;
};

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
{
}

CAlarmMachineDlg::~CAlarmMachineDlg()
{
}


void CAlarmMachineDlg::KillMeWhenYouDie(HWND hWnd)
{
	AUTO_LOG_FUNCTION;
	m_domodal_hwnd_list.push_back(hWnd);
	m_domodal_hwnd_list.unique();
}


void CAlarmMachineDlg::IDeadBeforeYou(HWND hWnd)
{
	AUTO_LOG_FUNCTION;
	if (!m_domodal_hwnd_list.empty()) {
		for (auto hwnd : m_domodal_hwnd_list) {
			if (hwnd == hWnd) {
				m_domodal_hwnd_list.remove(hwnd);
				return;
			}
		}
	}
	
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
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_EDIT_ZONE, &CAlarmMachineDlg::OnBnClickedButtonEditZone)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_MAP, &CAlarmMachineDlg::OnBnClickedButtonEditMap)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_DETECTOR, &CAlarmMachineDlg::OnBnClickedButtonEditDetector)
	ON_BN_CLICKED(IDC_BUTTON_MORE_HR, &CAlarmMachineDlg::OnBnClickedButtonMoreHr)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_SEE_BAIDU_MAP, &CAlarmMachineDlg::OnBnClickedButtonSeeBaiduMap)
	ON_BN_CLICKED(IDC_BUTTON_MANAGE_EXPIRE, &CAlarmMachineDlg::OnBnClickedButtonManageExpire)
	ON_BN_CLICKED(IDC_BUTTON_MGR_CAMERA_ICON, &CAlarmMachineDlg::OnBnClickedButtonMgrCameraIcon)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CAlarmMachineDlg message handlers


void CAlarmMachineDlg::SetMachineInfo(const core::alarm_machine_ptr& machine)
{
	m_machine = machine;
}
//
//namespace {
//	void __stdcall OnCurUserChanged(void* udata, core::user_info_ptr user)
//	{
//		if (!udata || !user)
//			return;
//
//		CAlarmMachineDlg* dlg = reinterpret_cast<CAlarmMachineDlg*>(udata);
//		dlg->OnCurUserChangedResult(user);
//	}
//};

void CAlarmMachineDlg::OnCurUserChangedResult(const core::user_info_ptr& user)
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
				if (MT_IMPRESSED_GPRS_MACHINE_2050 != m_machine->get_machine_type())
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

	m_new_record_observer = std::make_shared<NewRecordObserver>(this);

	//CRect rc(0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN));
	RECT rt;
	SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&rt, 0);
	CRect rc(rt);
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
	m_observer = std::make_shared<ademco::AdemcoEventObserver<CAlarmMachineDlg>>(this);
	m_machine->register_observer(m_observer);
	//m_machine->RegisterObserver(this, OnAdemcoEvent);

	// 2. update icon
	if (m_machine->get_online()) {
		m_staticNet.SetIcon(CAppResource::m_hIconNetOk);
	} else {
		m_staticNet.SetIcon(CAppResource::m_hIconNetFailed);
	}

	switch (m_machine->get_machine_status()) {
	case core::MACHINE_ARM:
		m_staticStatus.SetIcon(CAppResource::m_hIconArm);
		break;
	case core::MACHINE_HALFARM:
		m_staticStatus.SetIcon(CAppResource::m_hIconHalfarm);
		break;
	case core::MACHINE_DISARM:
	case core::MACHINE_STATUS_UNKNOWN:
	default:
		m_staticStatus.SetIcon(CAppResource::m_hIconDisarm);
		break;
	}
	CString text, smachine, sstatus;
	sstatus = GetStringFromAppResource(IDS_STRING_MACHINE_STATUS);
	if (m_machine->get_is_submachine()) {
		text = GetStringFromAppResource(IDS_STRING_SLAVE_CONN);
		m_staticConn.SetWindowTextW(text);
		smachine = GetStringFromAppResource(IDS_STRING_SUBMACHINE);
		m_btnManageExpire.EnableWindow(0);
	} else {
		smachine = GetStringFromAppResource(IDS_STRING_MACHINE);
		if (MT_IMPRESSED_GPRS_MACHINE_2050 != m_machine->get_machine_type())
			m_btnManageExpire.EnableWindow();
	}
	m_staticMachineStatus.SetWindowTextW(smachine + sstatus);

	m_cur_user_changed_observer = std::make_shared<CurUserChangedObserver>(this);
	core::user_manager::GetInstance()->register_observer(m_cur_user_changed_observer);
	m_cur_user_changed_observer->on_update(core::user_manager::GetInstance()->GetCurUserInfo());

	// 3. load map info
	LoadMaps();

	// 4. setup history callback
	history_record_manager* hr = history_record_manager::GetInstance();
	if (m_machine->get_is_submachine()) {
		hr->GetTopNumRecordByAdemcoIDAndZone(1000, m_machine->get_ademco_id(),
											 m_machine->get_submachine_zone(),
											 m_new_record_observer);
	} else {
		hr->GetTopNumRecordByAdemcoID(1000, m_machine->get_ademco_id(),
									  m_new_record_observer);
	}
	//hr->RegisterObserver(this, OnNewRecord);
	hr->register_observer(m_new_record_observer);

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
	fmMachine = GetStringFromAppResource(IDS_STRING_MACHINE);
	fmSubMachine = GetStringFromAppResource(IDS_STRING_SUBMACHINE);
	fmAlias = GetStringFromAppResource(IDS_STRING_ALIAS);
	fmContact = GetStringFromAppResource(IDS_STRING_CONTACT);
	fmAddress = GetStringFromAppResource(IDS_STRING_ADDRESS);
	fmPhone = GetStringFromAppResource(IDS_STRING_PHONE);
	fmPhoneBk = GetStringFromAppResource(IDS_STRING_PHONE_BK);
	fmNull = GetStringFromAppResource(IDS_STRING_NULL);
	

	if (m_machine->get_is_submachine()) {
		sid.Format(L"%s%03d", fmSubMachine, m_machine->get_submachine_zone());
		
	} else {
		sid.Format(L"%s" + GetStringFromAppResource(IDS_STRING_FM_ADEMCO_ID), fmMachine, m_machine->get_ademco_id());
	}

	text.Format(L"%s    %s:%s    %s:%s    %s:%s    %s:%s    %s:%s",
				sid,
				fmAlias, m_machine->get_machine_name(),
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
		CString s, e; e = GetStringFromAppResource(IDS_STRING_EXPIRE);
		s.Format(L"%s\r\n%s", e, m_machine->get_formatted_machine_name());
		MessageBox(s);
	}
}


void CAlarmMachineDlg::UpdateBtn123()
{
	CString btnText;
	machine_type mt = m_machine->get_machine_type();
	if (MT_NETMOD != mt) {
		btnText = GetStringFromAppResource(IDS_STRING_BK_BTN);
		m_btn1.EnableWindow(0);
		m_btn2.EnableWindow(0);
		m_btn3.EnableWindow(0);
		m_btn1.SetWindowTextW(btnText + L" 1");
		m_btn2.SetWindowTextW(btnText + L" 2");
		m_btn3.SetWindowTextW(btnText + L" 3");
		m_btnManageExpire.EnableWindow(0);
		return;
	}

	if (m_machine->get_is_submachine()) {
		btnText = GetStringFromAppResource(IDS_STRING_QUERY);
		m_btn1.SetWindowTextW(btnText);
		m_btn1.EnableWindow();

		btnText = GetStringFromAppResource(IDS_STRING_BK_BTN);
		m_btn2.SetWindowTextW(btnText + L" 1");
		m_btn3.SetWindowTextW(btnText + L" 2");
	} else {
		CString fmAllSubMachine;
		fmAllSubMachine = GetStringFromAppResource(IDS_STRING_ALL_SUBMACHINE);
		btnText = GetStringFromAppResource(IDS_STRING_QUERY);
		m_btn1.SetWindowTextW(btnText + fmAllSubMachine);
		m_btn1.EnableWindow();

		btnText = GetStringFromAppResource(IDS_STRING_WRITE2MACHINE);
		m_btn2.SetWindowTextW(btnText);
		if (core::user_manager::GetInstance()->GetCurUserInfo()->get_user_priority() != UP_OPERATOR)
			m_btn2.EnableWindow();

		btnText = GetStringFromAppResource(IDS_STRING_BK_BTN);
		m_btn3.SetWindowTextW(btnText + L" 1");

		LoadMaps();
	}
}


void CAlarmMachineDlg::LoadMaps()
{
	AUTO_LOG_FUNCTION;
	CRect rcTab;
	m_tab.GetClientRect(rcTab);
	rcTab.DeflateRect(5, 25, 5, 5);
	int prevSel = 0;
	CWndPtr prevShowTab = nullptr;
	if (m_tab.GetItemCount() > 0) {
		prevSel = m_tab.GetCurSel();
		ReleaseMaps();
	}

	int nItem = 0;	
	if (!m_machine->get_is_submachine() && m_machine->get_machine_type() == MT_NETMOD) { // sub machines
		CString sAllSubMachine; sAllSubMachine = GetStringFromAppResource(IDS_STRING_ALL_SUBMACHINE);
		m_container = std::shared_ptr<CAlarmMachineContainerDlg>(new CAlarmMachineContainerDlg(),
																 [](CAlarmMachineContainerDlg* dlg) {SAFEDELETEDLG(dlg); });
		m_container->m_machine = m_machine;
		m_container->m_bSubmachineContainer = true;
		m_container->Create(IDD_DIALOG_CONTAINER, &m_tab);
		m_container->MoveWindow(rcTab, FALSE);
		m_container->ShowWindow(SW_HIDE);
		zone_info_list zoneList;
		m_machine->GetAllZoneInfo(zoneList);
		for (auto zoneInfo : zoneList) {
			alarm_machine_ptr subMachineInfo = zoneInfo->GetSubMachineInfo();
			if (subMachineInfo) {
				m_container->InsertMachine(subMachineInfo, -1, false);
			}
		}
		nItem = m_tab.InsertItem(nItem, sAllSubMachine);
		if (m_machine->get_alarmingSubMachineCount() > 0) {
			m_tab.HighlightItem(nItem, TRUE);
		}
		auto tvn = std::make_shared<TabViewWithNdx>(m_container, nItem);
		m_tabViewList.push_back(tvn);
		if (prevSel == nItem) {
			prevShowTab = m_container;
		}
		nItem++;
	}

	// map contains unbind zone
	map_info_ptr unbindZoneMapInfo = m_machine->GetUnbindZoneMap();
	if (unbindZoneMapInfo) {
		CMapViewPtr mapView = std::shared_ptr<CMapView>(new CMapView(), [](CMapView* view) {SAFEDELETEDLG(view); });
		mapView->SetRealParentWnd(this);
		mapView->SetMachineInfo(m_machine);
		mapView->SetMapInfo(unbindZoneMapInfo);
		mapView->Create(IDD_DIALOG_MAPVIEW, &m_tab);
		mapView->MoveWindow(rcTab, FALSE);
		mapView->ShowWindow(SW_HIDE);

		nItem = m_tab.InsertItem(nItem, unbindZoneMapInfo->get_alias());
		auto tvn = std::make_shared<TabViewWithNdx>(mapView, nItem);
		m_tabViewList.push_back(tvn);
		if (prevSel == nItem) {
			prevShowTab = mapView;
		}
		nItem++;
	}

	// normal maps
	map_info_list list;
	m_machine->GetAllMapInfo(list);
	for (auto mapInfo : list) {
		CMapViewPtr mapView = std::shared_ptr<CMapView>(new CMapView, [](CMapView* view) {SAFEDELETEDLG(view); });
		mapView->SetRealParentWnd(this);
		mapView->SetMachineInfo(m_machine);
		mapView->SetMapInfo(mapInfo);
		mapView->Create(IDD_DIALOG_MAPVIEW, &m_tab);
		mapView->MoveWindow(rcTab, FALSE);
		mapView->ShowWindow(SW_HIDE);

		nItem = m_tab.InsertItem(nItem, mapInfo->get_alias());
		TabViewWithNdxPtr tvn = std::make_shared<TabViewWithNdx>(mapView, nItem);
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
			TabViewWithNdxPtr tvn = m_tabViewList.front();
			tvn->_tabView->ShowWindow(SW_SHOW);
		}
	}

	m_tab.Invalidate();
}


void CAlarmMachineDlg::ReleaseMaps()
{
	m_tab.DeleteAllItems();
	m_tabViewList.clear();
	m_container = nullptr;
}


void CAlarmMachineDlg::OnDestroy()
{
	AUTO_LOG_FUNCTION;
	
	//history_record_manager* hr = history_record_manager::GetInstance();
	//hr->UnRegisterObserver(this);
	m_new_record_observer = nullptr;

	m_cur_user_changed_observer = nullptr;

	m_observer = nullptr;
	if (m_machine) {
		//m_machine->UnRegisterObserver(this);
		m_machine = nullptr;
	}

	KillTimer(TIMER_ID_TRAVERSE_ADEMCO_LIST);
	KillTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
	KillTimer(TIMER_ID_HISTORY_RECORD);
	KillTimer(TIMER_ID_CHECK_EXPIRE_TIME);
	KillTimer(TIMER_ID_HANDLE_ADEMCO_EVENT);

	_ademcoEventList.clear();

	ReleaseMaps();

	for (auto hwnd : m_domodal_hwnd_list) {
		::SendMessage(hwnd, WM_EXIT_ALARM_CENTER, 0, 0);
	}
	m_domodal_hwnd_list.clear();

	CDialogEx::OnDestroy();
}


void CAlarmMachineDlg::OnAdemcoEventResult(const ademco::AdemcoEventPtr& ademcoEvent)
{
	AUTO_LOG_FUNCTION;
	ASSERT(ademcoEvent);
	if (nullptr == m_machine)
		return;

	if (ademco::EVENT_IM_GONNA_DIE == ademcoEvent->_event) {
		m_machine = nullptr;
		return;
	}

	std::lock_guard<std::mutex> lock(m_lock4AdemcoEventList);
	_ademcoEventList.push_back(ademcoEvent);
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

		alarm_machine_manager* manager = alarm_machine_manager::GetInstance();
		manager->RemoteControlAlarmMachine(m_machine, ademco::EVENT_QUERY_SUB_MACHINE,
										   INDEX_SUB_MACHINE,
										   m_machine->get_submachine_zone(),
										   nullptr, nullptr, ES_UNKNOWN, this);
	} else {
		if (m_machine->get_submachine_count() == 0) {
			CString e; e = GetStringFromAppResource(IDS_STRING_E_MACHINE_NO_SUB);
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
	USES_CONVERSION;
	bool bsubmachine = m_machine->get_is_submachine();
	if (bsubmachine) {
		m_nRemoteControlTimeCounter = REMOTE_CONTROL_DISABLE_TIMEUP;
		m_curRemoteControlCommand = ademco::EVENT_DISARM;
		KillTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
		OnTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
		SetTimer(TIMER_ID_REMOTE_CONTROL_MACHINE, 1000, nullptr);

		alarm_machine_manager* manager = alarm_machine_manager::GetInstance();
		auto xdata = std::make_shared<ademco::char_array>();
		if (!m_machine->get_is_submachine()) {
			CInputDlg dlg(this);
			if (dlg.DoModal() != IDOK)
				return;
			if (dlg.m_edit.GetLength() != 6)
				return;
			auto a = W2A(dlg.m_edit);
			for (int i = 0; i < 6; i++) { xdata->push_back(a[i]); }
		}
		BOOL ok = manager->RemoteControlAlarmMachine(m_machine, ademco::EVENT_DISARM,
													 bsubmachine ? INDEX_SUB_MACHINE : INDEX_ZONE,
													 bsubmachine ? m_machine->get_submachine_zone() : 0,
													 xdata, nullptr, ES_UNKNOWN, this);
		if (!ok) {
			KillTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
			m_nRemoteControlTimeCounter = 0;
			OnTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
		}
	} else {
		if (m_machine->get_zone_count() == 0) {
			CString e; e = GetStringFromAppResource(IDS_STRING_E_MACHINE_NO_ZONE);
			MessageBox(e, L"", MB_ICONINFORMATION);
			return;
		} else {
			int wire_zone_cnt = 0;
			zone_info_ptr wireZone = nullptr;
			for (int i = WIRE_ZONE_RANGE_BEG; i <= WIRE_ZONE_RANGE_END; i++) {
				wireZone = m_machine->GetZone(i);
				if (wireZone)
					wire_zone_cnt++;
			}
			if (wire_zone_cnt == m_machine->get_zone_count()) {
				CString e; e = GetStringFromAppResource(IDS_STRING_WIRE_ZONE_NO_NEED_RESTORE);
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


void CAlarmMachineDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (TIMER_ID_TRAVERSE_ADEMCO_LIST == nIDEvent) {
		KillTimer(TIMER_ID_TRAVERSE_ADEMCO_LIST);
		if (m_machine)
			m_machine->TraverseAdmecoEventList(m_observer);
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
						alarm_machine_manager* manager = alarm_machine_manager::GetInstance();
						manager->RemoteControlAlarmMachine(m_machine, 
														   EVENT_QUERY_SUB_MACHINE,
														   INDEX_SUB_MACHINE,
														   m_machine->get_submachine_zone(),
														   nullptr, nullptr, ES_UNKNOWN, this);
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
				CString e; e = GetStringFromAppResource(IDS_STRING_QUERY_FAILED);
				MessageBox(e, L"", MB_ICONERROR);
				auto t = time(nullptr);
				history_record_manager::GetInstance()->InsertRecord(m_machine->get_ademco_id(),
															m_machine->get_is_submachine() ? m_machine->get_submachine_zone() : 0,
															e, t, RECORD_LEVEL_USERCONTROL);
				m_machine->set_online(false);
				m_machine->SetAdemcoEvent(ES_UNKNOWN, EVENT_OFFLINE, m_machine->get_submachine_zone(),
										  INDEX_SUB_MACHINE, t, t);
			}
			UpdateBtn123();
		}
	} else if (TIMER_ID_HISTORY_RECORD == nIDEvent) {
		if (m_lock4RecordList.try_lock()) {
			std::lock_guard<std::mutex> lock(m_lock4RecordList, std::adopt_lock);
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
		}
	} else if (TIMER_ID_CHECK_EXPIRE_TIME == nIDEvent) {
		KillTimer(TIMER_ID_CHECK_EXPIRE_TIME);
		CheckIfExpire();
		SetTimer(TIMER_ID_CHECK_EXPIRE_TIME, 60 * 1000, nullptr);
	} else if (TIMER_ID_HANDLE_ADEMCO_EVENT == nIDEvent){
		if (m_lock4AdemcoEventList.try_lock()) {
			std::lock_guard<std::mutex> lock(m_lock4AdemcoEventList, std::adopt_lock);
			while (_ademcoEventList.size() > 0){
				const ademco::AdemcoEventPtr& ademcoEvent = _ademcoEventList.front();
				HandleAdemcoEvent(ademcoEvent);
				_ademcoEventList.pop_front();
			}
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CAlarmMachineDlg::HandleAdemcoEvent(const ademco::AdemcoEventPtr& ademcoEvent)
{
	bool bsubmachine_status = ademcoEvent->_sub_zone != INDEX_ZONE;
	if (bsubmachine_status != m_machine->get_is_submachine()) {
		if (!m_machine->get_is_submachine()) {
			if (m_container) {
				TabViewWithNdxPtr mnTarget = nullptr;
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
			if (ademcoEvent->_event == EVENT_I_AM_NET_MODULE || ademcoEvent->_event == EVENT_I_AM_EXPRESSED_GPRS_2050_MACHINE) {
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
			CString i; i = GetStringFromAppResource(IDS_STRING_QUERY_SUCCESS);
			history_record_manager::GetInstance()->InsertRecord(m_machine->get_ademco_id(),
														m_machine->get_is_submachine() ? m_machine->get_submachine_zone() : 0,
														i, time(nullptr), RECORD_LEVEL_USERCONTROL);
			m_nRemoteControlTimeCounter = 0;
		}
		//UpdateBtn123();
		break;
	case ademco::EVENT_ARM:
		m_staticNet.SetIcon(CAppResource::m_hIconNetOk);
		m_staticStatus.SetIcon(CAppResource::m_hIconArm);
		KillTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
		if (m_nRemoteControlTimeCounter > 0) {
			CString i; i = GetStringFromAppResource(IDS_STRING_QUERY_SUCCESS);
			history_record_manager::GetInstance()->InsertRecord(m_machine->get_ademco_id(),
														m_machine->get_is_submachine() ? m_machine->get_submachine_zone() : 0,
														i, time(nullptr), RECORD_LEVEL_USERCONTROL);
			m_nRemoteControlTimeCounter = 0;
		}
		//UpdateBtn123();
		break;
	case ademco::EVENT_HALFARM:
		m_staticNet.SetIcon(CAppResource::m_hIconNetOk);
		m_staticStatus.SetIcon(CAppResource::m_hIconHalfarm);
		KillTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
		if (m_nRemoteControlTimeCounter > 0) {
			CString i; i = GetStringFromAppResource(IDS_STRING_QUERY_SUCCESS);
			history_record_manager::GetInstance()->InsertRecord(m_machine->get_ademco_id(),
														m_machine->get_is_submachine() ? m_machine->get_submachine_zone() : 0,
														i, time(nullptr), RECORD_LEVEL_USERCONTROL);
			m_nRemoteControlTimeCounter = 0;
		}
		//UpdateBtn123();
		break;
	case ademco::EVENT_EMERGENCY:
		KillTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
		m_nRemoteControlTimeCounter = 0;
		OnTimer(TIMER_ID_REMOTE_CONTROL_MACHINE);
		break;
	case ademco::EVENT_SUBMACHINECNT:
		break;
	case EVENT_SIGNAL_STRENGTH_CHANGED:
		break;
	case EVENT_I_AM_NET_MODULE:
	case EVENT_I_AM_EXPRESSED_GPRS_2050_MACHINE:
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
	DWORD start = GetTickCount();
	while (!alarm_machine_manager::GetInstance()->EnterBufferMode()) {
		if (GetTickCount() - start > 3000) {
			CString e; e = GetStringFromAppResource(IDS_STRING_MACHINE_BUSY);
			MessageBox(e, L"", MB_OK | MB_ICONINFORMATION);
			return;
		}
		Sleep(100);
	}
	m_machine->LeaveBufferMode();
	CEditZoneDlg dlg(this);
	dlg.m_machine = m_machine;
	dlg.m_machineDlg = this;
	dlg.DoModal();
	if (dlg.m_bNeedReloadMaps)
		LoadMaps();
	while (!alarm_machine_manager::GetInstance()->LeaveBufferMode()) { Sleep(100); }
}


void CAlarmMachineDlg::OnInversionControl(core::CWndPtr wnd, core::inversion_control_map_command icmc)
{
	AUTO_LOG_FUNCTION;
	CMapViewPtr view = std::dynamic_pointer_cast<CMapView>(wnd);
	if (ICMC_SHOW != icmc && ICMC_RENAME != icmc && ICMC_ADD_ALARM_TEXT != icmc && ICMC_CLR_ALARM_TEXT != icmc)
		return;

	TabViewWithNdxPtr mnTarget = nullptr;
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
}


void CAlarmMachineDlg::OnBnClickedButtonEditMap()
{
	AUTO_LOG_FUNCTION;
	DWORD start = GetTickCount();
	while (!alarm_machine_manager::GetInstance()->EnterBufferMode()) {
		if (GetTickCount() - start > 3000) {
			CString e; e = GetStringFromAppResource(IDS_STRING_MACHINE_BUSY);
			MessageBox(e, L"", MB_OK | MB_ICONINFORMATION);
			return;
		}
		Sleep(100);
	}
	CEditMapDlg dlg(this);
	dlg.m_machine = m_machine;
	dlg.DoModal();
	if (dlg.m_bNeedReloadMaps) {
		LoadMaps();
		m_tab.Invalidate();
	}
	while (!alarm_machine_manager::GetInstance()->LeaveBufferMode()) { Sleep(100); }
}


void CAlarmMachineDlg::OnBnClickedButtonEditDetector()
{
	AUTO_LOG_FUNCTION;
	DWORD start = GetTickCount();
	while (!alarm_machine_manager::GetInstance()->EnterBufferMode()) {
		if (GetTickCount() - start > 3000) {
			CString e; e = GetStringFromAppResource(IDS_STRING_MACHINE_BUSY);
			MessageBox(e, L"", MB_OK | MB_ICONINFORMATION);
			return;
		}
		Sleep(100);
	}
	CEditDetectorDlg dlg(this);
	dlg.m_machine = m_machine;
	dlg.DoModal();
	while (!alarm_machine_manager::GetInstance()->LeaveBufferMode()) { Sleep(100); }

}


void CAlarmMachineDlg::OnBnClickedButtonMgrCameraIcon()
{
	AUTO_LOG_FUNCTION;
	DWORD start = GetTickCount();
	while (!alarm_machine_manager::GetInstance()->EnterBufferMode()) {
		if (GetTickCount() - start > 3000) {
			CString e; e = GetStringFromAppResource(IDS_STRING_MACHINE_BUSY);
			MessageBox(e, L"", MB_OK | MB_ICONINFORMATION);
			return;
		}
		Sleep(100);
	}
	CEditCameraDlg dlg(this);
	dlg.m_machine = m_machine;
	dlg.DoModal();
	while (!alarm_machine_manager::GetInstance()->LeaveBufferMode()) { Sleep(100); }
}


void CAlarmMachineDlg::OnBnClickedButtonMoreHr()
{
	CHistoryRecordDlg dlg(this);
	dlg.MySonYourFatherIsAlarmMachineDlg(this);
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
	auto wnd = GetParent();
	if (wnd) {
		//wnd->Invalidate(0);
	}
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
	DWORD start = GetTickCount();
	while (!alarm_machine_manager::GetInstance()->EnterBufferMode()) {
		if (GetTickCount() - start > 3000) {
			CString e; e = GetStringFromAppResource(IDS_STRING_MACHINE_BUSY);
			MessageBox(e, L"", MB_OK | MB_ICONINFORMATION);
			return;
		}
		Sleep(100);
	}
	if (m_machine->get_is_submachine()) return;
	CMachineExpireManagerDlg dlg(this);
	zone_info_list list;
	m_machine->GetAllZoneInfo(list);
	std::list<alarm_machine_ptr> machineList;
	for (auto zoneInfo : list) {
		alarm_machine_ptr subMachine = zoneInfo->GetSubMachineInfo();
		if (subMachine) {
			machineList.push_back(subMachine);
		}
	}
	dlg.m_bSubMachine = true;
	dlg.m_machine = m_machine;
	dlg.SetExpiredMachineList(machineList);
	dlg.DoModal();
	while (!alarm_machine_manager::GetInstance()->LeaveBufferMode()) { Sleep(100); }
}






void CAlarmMachineDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	if (bShow) {
		//Invalidate();
		int ndx = m_tab.GetCurSel();
		if (ndx == -1)return;

		for (auto tvn : m_tabViewList) {
			if (tvn->_ndx == ndx) { // found
				tvn->_tabView->ShowWindow(SW_HIDE);
				tvn->_tabView->ShowWindow(SW_SHOW);
				break;
			} 
		}
	}
}
