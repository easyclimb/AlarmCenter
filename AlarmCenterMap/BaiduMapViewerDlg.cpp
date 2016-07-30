// PickMachineCoordinateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenterMap.h"
#include "BaiduMapViewerDlg.h"
#include "afxdialogex.h"
#include "BaiduMapDlg.h"
#include "ConfigHelper.h"
#include "alarm_center_map_client.h"
#include "../rpc/alarm_center_map.pb.h"

using namespace core;
CBaiduMapViewerDlg* g_baiduMapDlg = nullptr;

//
//class CBaiduMapViewerDlg::CurUserChangedObserver : public dp::observer<core::user_info_ptr>
//{
//public:
//	explicit CurUserChangedObserver(CBaiduMapViewerDlg* dlg) : _dlg(dlg) {}
//	virtual void on_update(const core::user_info_ptr& ptr) {
//		if (_dlg) {
//			if (ptr->get_user_priority() == core::UP_OPERATOR) {
//				_dlg->m_btnAutoLocate.EnableWindow(0);
//			} else {
//				_dlg->m_btnAutoLocate.EnableWindow(1);
//			}
//		}
//	}
//private:
//	CBaiduMapViewerDlg* _dlg;
//};

namespace detail {
	const int TIMER_ID_CHECK_MACHINE_LIST = 1;
	const int TIMER_ID_REFRESH_ON_INIT = 2;
};
// CBaiduMapViewerDlg dialog

IMPLEMENT_DYNAMIC(CBaiduMapViewerDlg, CDialogEx)

CBaiduMapViewerDlg::CBaiduMapViewerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CBaiduMapViewerDlg::IDD, pParent)
	, m_mode(MODE_READY)
	, m_map(nullptr)
	, m_bSizing(FALSE)
	, m_bMoving(FALSE)
	, m_x(0)
	, m_y(0)
	, m_cx(0)
	, m_cy(0)
	, m_bInitOver(FALSE)
	, m_lastTimeShowMap(COleDateTime::GetCurrentTime())
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
	ON_MESSAGE(WM_SHOW_CSR_MAP, &CBaiduMapViewerDlg::OnMsgShowCsrMap)
	ON_MESSAGE(WM_SHOW_MACHINE_MAP, &CBaiduMapViewerDlg::OnMsgShowMachineMap)
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

	m_chkAutoRefresh4NewAlarm.SetCheck(util::CConfigHelper::get_instance()->get_baidumap_auto_refresh());
	OnBnClickedCheckAutoAlarm2();
	//SetTimer(1, 5000, nullptr);
	//g_baiduMapDlg = this;
	//assert(m_machine);
	//m_cur_user_changed_observer = std::make_shared<CurUserChangedObserver>(this);
	//core::user_manager::get_instance()->register_observer(m_cur_user_changed_observer);
	//m_cur_user_changed_observer->on_update(core::user_manager::get_instance()->GetCurUserInfo());

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

	double x, y;
	int level;
	ipc::alarm_center_map_client::get_instance()->get_csr_info(x, y, level);
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
	auto cfg = util::CConfigHelper::get_instance();

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
	auto cfg = util::CConfigHelper::get_instance();

	CRect rect;
	GetWindowRect(rect);

	cfg->set_rectBaiduMapDlg(rect);
	cfg->set_maximizedBaiduMapDlg(bMaximized);
}


void CBaiduMapViewerDlg::ShowCsrMap(const web::BaiduCoordinate& coor, int level)
{
	m_mode = MODE_CSR;
	CString title; title = TR(IDS_STRING_ALARM_CENTER);
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


void CBaiduMapViewerDlg::ShowMachineMap(const std::shared_ptr<alarm_center_map::machine_info>& info)
{
	m_mode = MODE_MACHINE;

	current_machine_.first = info->ademco_id();
	current_machine_.second = info->zone_value();

	CString title = utf8::a2w(info->title()).c_str();
	CString sinfo = utf8::a2w(info->info()).c_str();

	web::BaiduCoordinate coor(info->pt().x(), info->pt().y());
	if (coor.x == 0. && coor.y == 0.) {
		coor.x = 108.953;
		coor.y = 34.2778;
		m_map->ShowCoordinate(coor, 5, title, sinfo);
	} else {
		m_map->ShowCoordinate(coor, info->pt().level(), title, sinfo);
	}

	SetWindowText(title);
	m_chkAutoAlarm.EnableWindow(1);
	m_btnShowDrivingRoute.EnableWindow(1);
	m_btnAutoLocate.EnableWindow(1);
	bool b = info->auto_popup();
	m_chkAutoAlarm.SetCheck(b ? 1 : 0);
	m_lastTimeShowMap = COleDateTime::GetCurrentTime();

	ShowWindow(SW_SHOW);
}


void CBaiduMapViewerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	//m_cur_user_changed_observer.reset();
}


void CBaiduMapViewerDlg::OnBnClickedButtonAutoLocate()
{
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
		auto info = machine_info_map_[current_machine_];
		info->mutable_pt()->set_x(coor.x);
		info->mutable_pt()->set_y(coor.y);
		info->mutable_pt()->set_level(m_map->m_zoomLevel);

		auto client = ipc::alarm_center_map_client::get_instance();
		client->set_machine_info(info->pt().x(), info->pt().y(), info->pt().level(), info->ademco_id(), info->zone_value(), info->auto_popup());

	} else if (m_mode == MODE_CSR) {
		JLOG(L"MODE_CSR.\n");
		
		auto client = ipc::alarm_center_map_client::get_instance();
		client->set_csr_info(coor.x, coor.y, m_map->m_zoomLevel);
	}
	return 0;
}


void CBaiduMapViewerDlg::OnBnClickedButtonShowPath()
{
	web::BaiduCoordinate coor_csr(csr_coor_);
	auto info = machine_info_map_[current_machine_];
	web::BaiduCoordinate coor_cli(info->pt().x(), info->pt().y());

	if (coor_cli.x == 0.0 && coor_cli.y == 0.0) {
		MessageBox(TR(IDS_STRING_NO_POS));
		return;
	}

	if (coor_cli == coor_csr) {
		MessageBox(TR(IDS_STRING_SAME_POS));
		return;
	}

	std::wstring csr = tr(IDS_STRING_ALARM_CENTER);
	std::wstring dst = utf8::a2w(info->title()).c_str();
	m_map->ShowDrivingRoute(coor_csr, coor_cli, csr, dst);
}


void CBaiduMapViewerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	JLOG(L"cx %d, cy %d\n", cx, cy);

	if (m_bInitOver) {
		ResizeMap();
		SavePosition(nType == SIZE_MAXIMIZED);
		OnBnClickedButtonShowMap();
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
		ShowMachineMap(machine_info_map_[current_machine_]);
	} else if (m_mode == MODE_CSR) {
		ShowCsrMap(csr_coor_, csr_level_);
	}
}


void CBaiduMapViewerDlg::OnClose()
{
	CDialogEx::OnClose();
}


void CBaiduMapViewerDlg::OnBnClickedCheckAutoAlarm()
{
	BOOL b = m_chkAutoAlarm.GetCheck();
	auto info = machine_info_map_[current_machine_];
	info->set_auto_popup(b ? true : false);
	auto client = ipc::alarm_center_map_client::get_instance();
	client->set_machine_info(info->pt().x(), info->pt().y(), info->pt().level(), info->ademco_id(), info->zone_value(), info->auto_popup());
}


void CBaiduMapViewerDlg::OnTimer(UINT_PTR nIDEvent)
{
	CDialogEx::OnTimer(nIDEvent);
}


void CBaiduMapViewerDlg::OnBnClickedCheckAutoAlarm2()
{
	BOOL b = m_chkAutoRefresh4NewAlarm.GetCheck();
	util::CConfigHelper::get_instance()->set_baidumap_auto_refresh(b);
	m_cmbBufferedAlarmList.EnableWindow(!b);
}


void CBaiduMapViewerDlg::OnCbnSelchangeComboBufferedAlarm()
{
	int ndx = m_cmbBufferedAlarmList.GetCurSel(); if (ndx < 0)return;
	MachineUuid uuid = m_uuidMap[ndx];
	
	ShowMachineMap(machine_info_map_[uuid]);
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


afx_msg LRESULT CBaiduMapViewerDlg::OnMsgShowCsrMap(WPARAM wParam, LPARAM /*lParam*/)
{
	auto client = ipc::alarm_center_map_client::get_instance();
	client->get_csr_info(csr_coor_.x, csr_coor_.y, csr_level_);

	if (wParam) {
		ShowCsrMap(csr_coor_, csr_level_);
	}

	return 0;
}


afx_msg LRESULT CBaiduMapViewerDlg::OnMsgShowMachineMap(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	auto client = ipc::alarm_center_map_client::get_instance();
	auto v = client->get_machines();

	for (auto info : v) {
		MachineUuid uuid(info->ademco_id(), info->zone_value());
		machine_info_map_[uuid] = info;

		if (util::CConfigHelper::get_instance()->get_baidumap_auto_refresh()) {
			ShowMachineMap(info);
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
				m_cmbBufferedAlarmList.InsertString(0, utf8::a2w(info->title()).c_str());
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

	return 0;
}
