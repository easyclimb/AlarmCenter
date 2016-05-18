// PickMachineCoordinateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "BaiduMapViewerDlg.h"
#include "afxdialogex.h"
#include "ZoneInfo.h"
#include "AlarmMachine.h"
#include "AlarmMachineManager.h"
#include "BaiduMapDlg.h"
#include "CsrInfo.h"
#include "UserInfo.h"
#include "ConfigHelper.h"

using namespace core;
CBaiduMapViewerDlg* g_baiduMapDlg = nullptr;


class CBaiduMapViewerDlg::CurUserChangedObserver : public dp::observer<core::user_info_ptr>
{
public:
	explicit CurUserChangedObserver(CBaiduMapViewerDlg* dlg) : _dlg(dlg) {}
	virtual void on_update(const core::user_info_ptr& ptr) {
		if (_dlg) {
			if (ptr->get_user_priority() == core::UP_OPERATOR) {
				_dlg->m_btnAutoLocate.EnableWindow(0);
			} else {
				_dlg->m_btnAutoLocate.EnableWindow(1);
			}
		}
	}
private:
	CBaiduMapViewerDlg* _dlg;
};

namespace detail {
	const int TIMER_ID_CHECK_MACHINE_LIST = 1;
	const int TIMER_ID_REFRESH_ON_INIT = 2;
};
// CBaiduMapViewerDlg dialog

IMPLEMENT_DYNAMIC(CBaiduMapViewerDlg, CDialogEx)

CBaiduMapViewerDlg::CBaiduMapViewerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CBaiduMapViewerDlg::IDD, pParent)
	, m_mode(MODE_MACHINE)
	, m_machine(nullptr)
	, m_map(nullptr)
	, m_bSizing(FALSE)
	, m_bMoving(FALSE)
	, m_x(0)
	, m_y(0)
	, m_cx(0)
	, m_cy(0)
	, m_bInitOver(FALSE)
	, m_lastTimeShowMap(COleDateTime::GetCurrentTime())
	, m_pCsrInfoWnd(nullptr)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CBaiduMapViewerDlg::~CBaiduMapViewerDlg()
{
}

void CBaiduMapViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_AUTO_LOCATE, m_btnAutoLocate);
	DDX_Control(pDX, IDC_CHECK_AUTO_ALARM, m_chkAutoAlarm);
	DDX_Control(pDX, IDC_BUTTON_SHOW_PATH, m_btnShowDrivingRoute);
	DDX_Control(pDX, IDC_CHECK_AUTO_ALARM2, m_chkAutoRefresh4NewAlarm);
	DDX_Control(pDX, IDC_COMBO1, m_cmbBufferedAlarmList);
}


BEGIN_MESSAGE_MAP(CBaiduMapViewerDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CBaiduMapViewerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_LOCATE, &CBaiduMapViewerDlg::OnBnClickedButtonAutoLocate)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_SET_PT, &CBaiduMapViewerDlg::OnBnClickedButtonSetPt)
	ON_MESSAGE(WM_CHOSEN_BAIDU_PT, &CBaiduMapViewerDlg::OnChosenBaiduPt)
	ON_BN_CLICKED(IDC_BUTTON_SHOW_PATH, &CBaiduMapViewerDlg::OnBnClickedButtonShowPath)
	ON_WM_SIZE()
	ON_WM_MOVE()
	ON_BN_CLICKED(IDC_BUTTON_SHOW_MAP, &CBaiduMapViewerDlg::OnBnClickedButtonShowMap)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_CHECK_AUTO_ALARM, &CBaiduMapViewerDlg::OnBnClickedCheckAutoAlarm)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_CHECK_AUTO_ALARM2, &CBaiduMapViewerDlg::OnBnClickedCheckAutoAlarm2)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CBaiduMapViewerDlg::OnCbnSelchangeComboBufferedAlarm)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_CMB, &CBaiduMapViewerDlg::OnBnClickedButtonClearCmb)
	ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()


// CBaiduMapViewerDlg message handlers


void CBaiduMapViewerDlg::OnBnClickedOk()
{

	CDialogEx::OnOK();
}


BOOL CBaiduMapViewerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	InitPosition();

	m_chkAutoRefresh4NewAlarm.SetCheck(util::CConfigHelper::GetInstance()->get_baidumap_auto_refresh());
	OnBnClickedCheckAutoAlarm2();
	//SetTimer(1, 5000, nullptr);
	//g_baiduMapDlg = this;
	//assert(m_machine);
	m_cur_user_changed_observer = std::make_shared<CurUserChangedObserver>(this);
	core::user_manager::GetInstance()->register_observer(m_cur_user_changed_observer);
	m_cur_user_changed_observer->on_update(core::user_manager::GetInstance()->GetCurUserInfo());

	m_map = std::shared_ptr<CBaiduMapDlg>(new CBaiduMapDlg(this), [](CBaiduMapDlg* dlg) { SAFEDELETEDLG(dlg); });
	CRect rc;
	GetClientRect(rc);
	rc.DeflateRect(0, 35, 0, 0);
	m_map->set_initRc(rc);
	m_map->m_pRealParent = this;
	m_map->Create(IDD_DIALOG_BAIDU_MAP, this);
	ResizeMap();
	/*if (m_machine) {
		web::BaiduCoordinate coor = m_machine->get_coor();
		if (coor.x == 0. && coor.y == 0.) {
			OnBnClickedButtonAutoLocate();
		} else {
			ShowMap(m_machine);
		}
	}*/
	m_chkAutoAlarm.EnableWindow(0);
	m_btnShowDrivingRoute.EnableWindow(0);
	m_btnAutoLocate.EnableWindow(0);

	SetTimer(detail::TIMER_ID_CHECK_MACHINE_LIST, 1000, nullptr);
	SetTimer(detail::TIMER_ID_REFRESH_ON_INIT, 8000, nullptr);
	m_bInitOver = TRUE;

	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CBaiduMapViewerDlg::ResizeMap()
{
	CRect rc;
	GetClientRect(rc);
	m_map->MoveWindow(rc, 0);
	rc.DeflateRect(0, 45, 0, 0);
	m_map->MoveWindow(rc, 1);
	m_map->ShowWindow(SW_SHOW);
	Invalidate();
}

void CBaiduMapViewerDlg::InitPosition()
{
	AUTO_LOG_FUNCTION;
	auto cfg = util::CConfigHelper::GetInstance();

	do {
		CRect rect = cfg->get_rectBaiduMapDlg();
		if (rect.IsRectNull() || rect.IsRectEmpty()) {
			break;
		}
		int m = cfg->get_maximizedBaiduMapDlg();

		if (m) {
			//rect.right = rect.left + 800;
			//rect.bottom = rect.top + 600;
			rect.DeflateRect(10, 10, 10, 10);
			MoveWindow(rect);
			ShowWindow(SW_MAXIMIZE);
		} else {
			MoveWindow(rect);
		}

	} while (0);
}


void CBaiduMapViewerDlg::SavePosition(BOOL bMaximized)
{
	auto cfg = util::CConfigHelper::GetInstance();

	CRect rect;
	GetWindowRect(rect);

	cfg->set_rectBaiduMapDlg(rect);
	cfg->set_maximizedBaiduMapDlg(bMaximized);
}


void CBaiduMapViewerDlg::ShowCsrMap(const web::BaiduCoordinate& coor, int level)
{
	m_mode = MODE_CSR;
	CString title; title = GetStringFromAppResource(IDS_STRING_ALARM_CENTER);
	SetWindowText(title);
	if (coor.x == 0.0 && coor.y == 0.0) {
		web::BaiduCoordinate coor2;
		coor2.x = 108.953;
		coor2.y = 34.2778;
		m_map->ShowCoordinate(coor2, 5, title, title);
	} else {
		m_map->ShowCoordinate(coor, level, title, title);
	}
	
	ShowWindow(SW_SHOW);
	m_chkAutoAlarm.EnableWindow(0);
	m_btnShowDrivingRoute.EnableWindow(0);
	m_btnAutoLocate.EnableWindow(0);
}


void CBaiduMapViewerDlg::ShowMap(const core::alarm_machine_ptr& machine)
{
	if (!machine)
		return;
	m_mode = MODE_MACHINE;
	m_machine = machine;

	CString title = m_machine->get_formatted_name();
	CString info = m_machine->get_formatted_info(L"<p/>");

	web::BaiduCoordinate coor = m_machine->get_coor();
	if (coor.x == 0. && coor.y == 0.) {
		//OnBnClickedButtonAutoLocate();
		coor.x = 108.953;
		coor.y = 34.2778;
		m_map->ShowCoordinate(coor, 5, title, info);
	} else {
		m_map->ShowCoordinate(coor, machine->get_zoomLevel(), title, info);
	}

	SetWindowText(title);
	m_chkAutoAlarm.EnableWindow(1);
	m_btnShowDrivingRoute.EnableWindow(1);
	m_btnAutoLocate.EnableWindow(1);
	bool b = m_machine->get_auto_show_map_when_start_alarming();
	m_chkAutoAlarm.SetCheck(b ? 1 : 0);
	m_lastTimeShowMap = COleDateTime::GetCurrentTime();

	ShowWindow(SW_SHOW);
}


void CBaiduMapViewerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	m_cur_user_changed_observer.reset();
}


void CBaiduMapViewerDlg::OnBnClickedButtonAutoLocate()
{
	/*if (m_mode == MODE_MACHINE) {
		if (!m_machine)
			return;
		std::wstring addr;
		int city_code;
		web::BaiduCoordinate coor;
		if (web::CBaiduService::GetInstance()->locate(addr, city_code, coor)) {
			m_machine->execute_set_coor(coor);
			std::wstring  url = GetModuleFilePath();
			url += L"\\data\\config";
			CreateDirectory(url.c_str(), nullptr);
			url += L"\\baidu.html";
			CString title, smachine; smachine = GetStringFromAppResource(IDS_STRING_MACHINE);
			title.Format(L"%s%04d(%s)", smachine, m_machine->get_ademco_id(), m_machine->get_alias());
			m_map->ShowCoordinate(coor, m_machine->get_zoomLevel(), title);
		} else {
			CString e; e = GetStringFromAppResource(IDS_STRING_E_AUTO_LACATE_FAILED);
			MessageBox(e, L"", MB_ICONERROR);
		}
	} else if (m_mode == MODE_CSR) {

	}*/
}


void CBaiduMapViewerDlg::OnBnClickedButtonSetPt()
{
	
}


afx_msg LRESULT CBaiduMapViewerDlg::OnChosenBaiduPt(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	AUTO_LOG_FUNCTION;
	web::BaiduCoordinate coor = m_map->m_coor;
	if (m_mode == MODE_MACHINE) {
		JLOG(L"MODE_MACHINE.\n");
		if (m_machine && m_machine->execute_set_coor(coor)) {
			m_machine->set_zoomLevel(m_map->m_zoomLevel);
			JLOG(L"succeed.\n");
			ShowMap(m_machine);
		}
	} else if (m_mode == MODE_CSR) {
		JLOG(L"MODE_CSR.\n");
		if (m_pCsrInfoWnd && ::IsWindow(m_pCsrInfoWnd->GetSafeHwnd())) {
			m_pCsrInfoWnd->PostMessageW(WM_CHOSEN_BAIDU_PT);
			JLOG(L"succeed.\n");
		}
	}
	return 0;
}


void CBaiduMapViewerDlg::OnBnClickedButtonShowPath()
{
	if (!m_machine)
		return;
	web::BaiduCoordinate coor_csr = csr_manager::GetInstance()->get_coor();
	web::BaiduCoordinate coor_cli = m_machine->get_coor();
	if (coor_cli.x == 0.0 && coor_cli.y == 0.0) {
		MessageBox(GetStringFromAppResource(IDS_STRING_NO_POS));
		return;
	}
	if (coor_cli == coor_csr) {
		MessageBox(GetStringFromAppResource(IDS_STRING_SAME_POS));
		return;
	}

	CString scsr; scsr = GetStringFromAppResource(IDS_STRING_ALARM_CENTER);
	std::wstring csr = scsr.LockBuffer();
	scsr.UnlockBuffer();
	CString sdst = m_machine->get_formatted_name();
	std::wstring dst = sdst.LockBuffer();
	sdst.UnlockBuffer();
	m_map->ShowDrivingRoute(coor_csr, coor_cli, csr, dst);
}


void CBaiduMapViewerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	JLOG(L"cx %d, cy %d\n", cx, cy);

	if (m_bInitOver) {
		ResizeMap();
		SavePosition(nType == SIZE_MAXIMIZED);
		ShowMap(m_machine);
	}
}


void CBaiduMapViewerDlg::OnMove(int x, int y)
{
	CDialogEx::OnMove(x, y);
	JLOG(L"x %d, y %d\n", x, y);

	if (m_bInitOver) {
		SavePosition();
	}
}


void CBaiduMapViewerDlg::OnBnClickedButtonShowMap()
{
	if (m_mode == MODE_MACHINE) {
		ShowMap(m_machine);
	} else if (m_mode == MODE_CSR) {
		core::csr_manager* csr = core::csr_manager::GetInstance();
		ShowCsrMap(csr->get_coor(), csr->get_level());
	}
}


void CBaiduMapViewerDlg::OnClose()
{
	ShowWindow(SW_HIDE);
	//CDialogEx::OnClose();
}


void CBaiduMapViewerDlg::OnBnClickedCheckAutoAlarm()
{
	BOOL b = m_chkAutoAlarm.GetCheck();
	if (m_machine) {
		m_machine->set_auto_show_map_when_start_alarming(b != 0);
	}
}


void CBaiduMapViewerDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (detail::TIMER_ID_CHECK_MACHINE_LIST == nIDEvent) {
		auto_timer timer(m_hWnd, detail::TIMER_ID_CHECK_MACHINE_LIST, 100);
		if (m_lock4MachineUuidList.try_lock()) {
			std::lock_guard<std::mutex> lock(m_lock4MachineUuidList, std::adopt_lock);
			if (!m_machineUuidList.empty()) {
				MachineUuid uuid = m_machineUuidList.front();
				m_machineUuidList.pop_front();
					
				core::alarm_machine_ptr machine = nullptr;
				CString txt;
				if (GetMachineByUuidAndFormatText(uuid, machine, txt)) {
					if (util::CConfigHelper::GetInstance()->get_baidumap_auto_refresh()) {
						ShowMap(machine);
					} else {
						// buffer to history combo
						bool b_exists = false;
						for (int i = 0; i < m_cmbBufferedAlarmList.GetCount(); i++) {
							MachineUuid mu = m_uuidMap[i];
							if (mu==uuid) {
								if (i != 0) {
									// already exists
									// move to first item
									CString t;
									m_cmbBufferedAlarmList.GetLBText(i, t);
									m_cmbBufferedAlarmList.DeleteString(i);
									m_cmbBufferedAlarmList.InsertString(0, t);
									std::map<int, MachineUuid> dummy;
									dummy[0] = uuid;
									m_uuidMap.erase(i);
									i = 1;
									for (auto u : m_uuidMap) {
										dummy[i++] = u.second;
									}
									m_uuidMap = dummy;
								}
								b_exists = true; break;
							}
						}
						if (!b_exists) {
							m_cmbBufferedAlarmList.InsertString(0, txt);
							m_cmbBufferedAlarmList.SetCurSel(0);
							std::map<int, MachineUuid> dummy;
							dummy[0] = uuid;
							int i = 1;
							for (auto u : m_uuidMap) {
								dummy[i++] = u.second;
							}
							m_uuidMap = dummy;
						}
					}
				}
			}
		}
	} else if (detail::TIMER_ID_REFRESH_ON_INIT == nIDEvent) {
		KillTimer(detail::TIMER_ID_REFRESH_ON_INIT); // only once 
		ResizeMap();
	}

	CDialogEx::OnTimer(nIDEvent);
}


bool CBaiduMapViewerDlg::GetMachineByUuidAndFormatText(const MachineUuid& uuid, core::alarm_machine_ptr& machine, CString& txt)
{
	core::alarm_machine_manager* mgr = core::alarm_machine_manager::GetInstance();
	machine = mgr->GetMachine(uuid.first);
	if (machine) {
		txt = machine->get_formatted_name();
		if (uuid.second != 0) {
			core::zone_info_ptr zoneInfo = machine->GetZone(uuid.second);
			if (zoneInfo) {
				core::alarm_machine_ptr subMachine = zoneInfo->GetSubMachineInfo();
				if (subMachine) {
					machine = subMachine;
				}
			}
		}
		return true;
	}
	return false;
}


void CBaiduMapViewerDlg::OnBnClickedCheckAutoAlarm2()
{
	BOOL b = m_chkAutoRefresh4NewAlarm.GetCheck();
	util::CConfigHelper::GetInstance()->set_baidumap_auto_refresh(b);
	m_cmbBufferedAlarmList.EnableWindow(!b);
}


void CBaiduMapViewerDlg::OnCbnSelchangeComboBufferedAlarm()
{
	int ndx = m_cmbBufferedAlarmList.GetCurSel(); if (ndx < 0)return;
	MachineUuid mu = m_uuidMap[ndx];
	core::alarm_machine_ptr machine = nullptr;
	CString txt;
	if (GetMachineByUuidAndFormatText(mu, machine, txt)) {
		ShowMap(machine);
	}
}


void CBaiduMapViewerDlg::OnBnClickedButtonClearCmb()
{
	m_uuidMap.clear();
	m_cmbBufferedAlarmList.ResetContent();
}


void CBaiduMapViewerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (nID == SC_MAXIMIZE) {
		//m_maximized_ = 1;
	} else if (nID == SC_RESTORE) {
		//m_maximized_ = 0;
	}

	CDialogEx::OnSysCommand(nID, lParam);
}
