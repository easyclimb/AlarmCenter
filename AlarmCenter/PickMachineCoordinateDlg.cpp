// PickMachineCoordinateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "PickMachineCoordinateDlg.h"
#include "afxdialogex.h"
#include "ZoneInfo.h"
#include "AlarmMachine.h"
#include "AlarmMachineManager.h"
#include "BaiduMapDlg.h"
#include "CsrInfo.h"
#include "UserInfo.h"
#include "tinyxml/tinyxml.h"
#include "ConfigHelper.h"

using namespace core;
CPickMachineCoordinateDlg* g_baiduMapDlg = nullptr;

namespace {
	const int TIMER_ID_CHECK_MACHINE_LIST = 1;
};
// CPickMachineCoordinateDlg dialog

IMPLEMENT_DYNAMIC(CPickMachineCoordinateDlg, CDialogEx)

CPickMachineCoordinateDlg::CPickMachineCoordinateDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CPickMachineCoordinateDlg::IDD, pParent)
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

CPickMachineCoordinateDlg::~CPickMachineCoordinateDlg()
{
}

void CPickMachineCoordinateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_AUTO_LOCATE, m_btnAutoLocate);
	DDX_Control(pDX, IDC_CHECK_AUTO_ALARM, m_chkAutoAlarm);
	DDX_Control(pDX, IDC_BUTTON_SHOW_PATH, m_btnShowDrivingRoute);
	DDX_Control(pDX, IDC_CHECK_AUTO_ALARM2, m_chkAutoRefresh4NewAlarm);
	DDX_Control(pDX, IDC_COMBO1, m_cmbBufferedAlarmList);
}


BEGIN_MESSAGE_MAP(CPickMachineCoordinateDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CPickMachineCoordinateDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_LOCATE, &CPickMachineCoordinateDlg::OnBnClickedButtonAutoLocate)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_SET_PT, &CPickMachineCoordinateDlg::OnBnClickedButtonSetPt)
	ON_MESSAGE(WM_CHOSEN_BAIDU_PT, &CPickMachineCoordinateDlg::OnChosenBaiduPt)
	ON_BN_CLICKED(IDC_BUTTON_SHOW_PATH, &CPickMachineCoordinateDlg::OnBnClickedButtonShowPath)
	ON_WM_SIZE()
	ON_WM_MOVE()
	ON_BN_CLICKED(IDC_BUTTON_SHOW_MAP, &CPickMachineCoordinateDlg::OnBnClickedButtonShowMap)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_CHECK_AUTO_ALARM, &CPickMachineCoordinateDlg::OnBnClickedCheckAutoAlarm)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_CHECK_AUTO_ALARM2, &CPickMachineCoordinateDlg::OnBnClickedCheckAutoAlarm2)
END_MESSAGE_MAP()


// CPickMachineCoordinateDlg message handlers


void CPickMachineCoordinateDlg::OnBnClickedOk()
{

	CDialogEx::OnOK();
}


namespace {
	void __stdcall OnCurUserChanged(void* udata, const core::CUserInfo* user)
	{
		if (!udata || !user)
			return;

		CPickMachineCoordinateDlg* dlg = reinterpret_cast<CPickMachineCoordinateDlg*>(udata);
		if (user->get_user_priority() == core::UP_OPERATOR) {
			dlg->m_btnAutoLocate.EnableWindow(0);
		} else {
			dlg->m_btnAutoLocate.EnableWindow(1);
		}
	}
};
BOOL CPickMachineCoordinateDlg::OnInitDialog()
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

	core::CUserManager::GetInstance()->RegisterObserver(this, OnCurUserChanged);
	OnCurUserChanged(this, core::CUserManager::GetInstance()->GetCurUserInfo());

	m_map = new CBaiduMapDlg(this);
	CRect rc;
	GetClientRect(rc);
	rc.DeflateRect(0, 25, 0, 0);
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
	m_chkAutoAlarm.ShowWindow(SW_HIDE);

	SetTimer(TIMER_ID_CHECK_MACHINE_LIST, 1000, nullptr);
	m_bInitOver = TRUE;

	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CPickMachineCoordinateDlg::ResizeMap()
{
	CRect rc;
	GetClientRect(rc);
	rc.DeflateRect(0, 25, 0, 0);
	m_map->MoveWindow(rc, 1);
	m_map->ShowWindow(SW_SHOW);
}

void CPickMachineCoordinateDlg::InitPosition()
{
	using namespace tinyxml;
	USES_CONVERSION;
	CString s; s.Format(L"%s\\config", GetModuleFilePath());
	CreateDirectory(s, nullptr);
	s += L"\\baidu.xml";
	
	TiXmlDocument doc(W2A(s));
	do {
		if (!doc.LoadFile()) {
			break;
		}

		TiXmlElement* root = doc.RootElement();
		if (!root)
			break;

		TiXmlElement* rc = root->FirstChildElement("rc");
		if (!rc)
			break;

		const char* sl = nullptr;
		const char* sr = nullptr;
		const char* st = nullptr;
		const char* sb = nullptr;
		const char* sm = nullptr;

		sl = rc->Attribute("l");
		sr = rc->Attribute("r");
		st = rc->Attribute("t");
		sb = rc->Attribute("b");
		sm = rc->Attribute("m");

		int l, r, t, b, m;
		l = r = t = b = m = 0;
		if (sl)
			l = atoi(sl);
		if(sr)
			r = atoi(sr);
		if (st)
			t = atoi(st);
		if (sb)
			b = atoi(sb);
		if(sm)
			m = atoi(sm);

		CRect rect(l, t, r, b);
		if (rect.IsRectNull() || rect.IsRectEmpty()) {
			break;
		}

		MoveWindow(rect);

		if (m) {
			ShowWindow(SW_SHOWMAXIMIZED);
		}
		return;
	}while (0);
	SavePosition();
}


void CPickMachineCoordinateDlg::SavePosition(BOOL bMaximized)
{
	using namespace tinyxml;
	USES_CONVERSION;
	CString s; s.Format(L"%s\\config", GetModuleFilePath());
	CreateDirectory(s, nullptr);
	s += L"\\baidu.xml";

	CRect rect;
	GetWindowRect(rect);

	TiXmlDocument doc;
	TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "", "");
	doc.LinkEndChild(decl);
	TiXmlElement *root = new TiXmlElement("BaiduConfig"); // 不能有空白符
	doc.LinkEndChild(root);

	TiXmlElement* rc = new TiXmlElement("rc"); // 不能有空白符
	rc->SetAttribute("l", rect.left);
	rc->SetAttribute("r", rect.right);
	rc->SetAttribute("t", rect.top);
	rc->SetAttribute("b", rect.bottom);
	rc->SetAttribute("m", bMaximized);
	root->LinkEndChild(rc);

	doc.SaveFile(W2A(s));
}


void CPickMachineCoordinateDlg::ShowCsrMap(const web::BaiduCoordinate& coor, int level)
{
	m_mode = MODE_CSR;
	CString title; title.LoadStringW(IDS_STRING_ALARM_CENTER);
	SetWindowText(title);
	m_map->ShowCoordinate(coor, level, title);
	ShowWindow(SW_SHOW);
	m_chkAutoAlarm.ShowWindow(SW_HIDE);
	m_btnShowDrivingRoute.ShowWindow(SW_HIDE);
	m_btnAutoLocate.ShowWindow(SW_HIDE);
}


void CPickMachineCoordinateDlg::ShowMap(core::CAlarmMachine* machine)
{
	if (!machine)
		return;
	m_mode = MODE_MACHINE;
	m_machine = machine;

	CString title, smachine, ssubmachine; 
	smachine.LoadStringW(IDS_STRING_MACHINE);
	ssubmachine.LoadStringW(IDS_STRING_SUBMACHINE);
	if (machine->get_is_submachine()) {
		CAlarmMachine* parentMachine = nullptr;
		if (CAlarmMachineManager::GetInstance()->GetMachine(machine->get_ademco_id(), parentMachine) && parentMachine) {
			title.Format(L"%s%04d(%s) %s%03d(%s)",
						 smachine, m_machine->get_ademco_id(), parentMachine->get_alias(),
						 ssubmachine, machine->get_submachine_zone(), machine->get_alias());
		}
	} else {
		title.Format(L"%s%04d(%s)", smachine, m_machine->get_ademco_id(), m_machine->get_alias());
	}

	web::BaiduCoordinate coor = m_machine->get_coor();
	if (coor.x == 0. && coor.y == 0.) {
		OnBnClickedButtonAutoLocate();
	} else {
		m_map->ShowCoordinate(coor, machine->get_zoomLevel(), title);
	}

	SetWindowText(title);
	m_btnShowDrivingRoute.ShowWindow(SW_SHOW);
	m_btnAutoLocate.ShowWindow(SW_SHOW);
	m_chkAutoAlarm.ShowWindow(SW_SHOW);
	bool b = m_machine->get_auto_show_map_when_start_alarming();
	m_chkAutoAlarm.SetCheck(b ? 1 : 0);
	m_lastTimeShowMap = COleDateTime::GetCurrentTime();

	ShowWindow(SW_SHOW);
}


void CPickMachineCoordinateDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	SAFEDELETEDLG(m_map);
	core::CUserManager::GetInstance()->UnRegisterObserver(this);
}


void CPickMachineCoordinateDlg::OnBnClickedButtonAutoLocate()
{
	if (m_mode == MODE_MACHINE) {
		if (!m_machine)
			return;
		std::wstring addr;
		int city_code;
		web::BaiduCoordinate coor;
		if (web::CBaiduService::GetInstance()->locate(addr, city_code, coor)) {
			m_machine->execute_set_coor(coor);
			std::wstring  url = GetModuleFilePath();
			url += L"\\config";
			CreateDirectory(url.c_str(), nullptr);
			url += L"\\baidu.html";
			CString title, smachine; smachine.LoadStringW(IDS_STRING_MACHINE);
			title.Format(L"%s%04d(%s)", smachine, m_machine->get_ademco_id(), m_machine->get_alias());
			m_map->ShowCoordinate(coor, m_machine->get_zoomLevel(), title);
		} else {
			CString e; e.LoadStringW(IDS_STRING_E_AUTO_LACATE_FAILED);
			MessageBox(e, L"", MB_ICONERROR);
		}
	} else if (m_mode == MODE_CSR) {

	}
}


void CPickMachineCoordinateDlg::OnBnClickedButtonSetPt()
{
	
}


afx_msg LRESULT CPickMachineCoordinateDlg::OnChosenBaiduPt(WPARAM /*wParam*/, LPARAM /*lParam*/)
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


void CPickMachineCoordinateDlg::OnBnClickedButtonShowPath()
{
	if (!m_machine)
		return;
	web::BaiduCoordinate coor_csr = CCsrInfo::GetInstance()->get_coor();
	web::BaiduCoordinate coor_cli = m_machine->get_coor();
	CString scsr; scsr.LoadStringW(IDS_STRING_ALARM_CENTER);
	std::wstring csr = scsr.LockBuffer();
	scsr.UnlockBuffer();
	CString sdst, smachine; smachine.LoadStringW(IDS_STRING_MACHINE);
	sdst.Format(L"%s%04d(%s)", smachine, m_machine->get_ademco_id(), 
				m_machine->get_alias());
	std::wstring dst = sdst.LockBuffer();
	sdst.UnlockBuffer();
	m_map->ShowDrivingRoute(coor_csr, coor_cli, csr, dst);
}


void CPickMachineCoordinateDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	JLOG(L"cx %d, cy %d\n", cx, cy);

	if (m_bInitOver) {
		ResizeMap();
		SavePosition(nType == SIZE_MAXIMIZED);
		ShowMap(m_machine);
	}
}


void CPickMachineCoordinateDlg::OnMove(int x, int y)
{
	CDialogEx::OnMove(x, y);
	JLOG(L"x %d, y %d\n", x, y);

	if (m_bInitOver) {
		SavePosition();
	}
}


void CPickMachineCoordinateDlg::OnBnClickedButtonShowMap()
{
	if (m_mode == MODE_MACHINE) {
		ShowMap(m_machine);
	} else if (m_mode == MODE_CSR) {
		core::CCsrInfo* csr = core::CCsrInfo::GetInstance();
		ShowCsrMap(csr->get_coor(), csr->get_level());
	}
}


void CPickMachineCoordinateDlg::OnClose()
{
	ShowWindow(SW_HIDE);
	//CDialogEx::OnClose();
}


void CPickMachineCoordinateDlg::OnBnClickedCheckAutoAlarm()
{
	BOOL b = m_chkAutoAlarm.GetCheck();
	if (m_machine) {
		m_machine->set_auto_show_map_when_start_alarming(b != 0);
	}
}


void CPickMachineCoordinateDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (TIMER_ID_CHECK_MACHINE_LIST == nIDEvent) {
		KillTimer(TIMER_ID_CHECK_MACHINE_LIST);
		if (m_lock4MachineUuidList.TryLock()) {
			if (!m_machineUuidList.empty()) {
				/*COleDateTime now = COleDateTime::GetCurrentTime();
				COleDateTimeSpan span = now - m_lastTimeShowMap;
				if (span.GetTotalSeconds() >= 3) {*/
					MachineUuid uuid = m_machineUuidList.front();
					m_machineUuidList.pop_front();
					core::CAlarmMachineManager* mgr = core::CAlarmMachineManager::GetInstance();
					core::CAlarmMachine* machine = nullptr;
					if (mgr->GetMachine(uuid.ademco_id, machine) && machine) {
						CString fmMachine; fmMachine.LoadStringW(IDS_STRING_MACHINE);
						CString txt;
						txt.Format(L"%s%06d[%s]", fmMachine, machine->get_ademco_id(), machine->get_alias());
						if (uuid.zone_value != 0) {
							
							core::CZoneInfo* zoneInfo = machine->GetZone(uuid.zone_value);
							if (zoneInfo) {
								core::CAlarmMachine* subMachine = zoneInfo->GetSubMachineInfo();
								if (subMachine) {
									machine = subMachine;
									CString fmSubmachine; fmSubmachine.LoadStringW(IDS_STRING_SUBMACHINE);
									CString txt2;
									txt2.Format(L"%s%03d[%s]", fmSubmachine, subMachine->get_submachine_zone(), subMachine->get_alias());
									txt += txt2;
								}
							} 
						}

						if (util::CConfigHelper::GetInstance()->get_baidumap_auto_refresh()) {
							ShowMap(machine);
						} else {
							// buffer to history combo
							bool b_exists = false;
							for (int i = 0; i < m_cmbBufferedAlarmList.GetCount(); i++) {
								MachineUuid* mu = reinterpret_cast<MachineUuid*>(m_cmbBufferedAlarmList.GetItemData(i));
								if (mu && mu->operator==(uuid)) {
									if (i != 0) {
										// move to first item
										CString t;
										m_cmbBufferedAlarmList.GetLBText(i, t);
										m_cmbBufferedAlarmList.DeleteString(i);
										m_cmbBufferedAlarmList.InsertString(0, t);
										m_cmbBufferedAlarmList.SetItemData(0, reinterpret_cast<DWORD_PTR>(mu));
									}
									b_exists = true; break;
								}
							}
							if (!b_exists) {
								MachineUuid* mu = new MachineUuid(uuid);
								m_cmbBufferedAlarmList.InsertString(0, txt);
								m_cmbBufferedAlarmList.SetItemData(0, reinterpret_cast<DWORD_PTR>(mu));
								m_cmbBufferedAlarmList.SetCurSel(0);
							}
						}
					}
				/*}*/
			}
			m_lock4MachineUuidList.UnLock();
		}
		SetTimer(TIMER_ID_CHECK_MACHINE_LIST, 100, nullptr);
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CPickMachineCoordinateDlg::OnBnClickedCheckAutoAlarm2()
{
	BOOL b = m_chkAutoRefresh4NewAlarm.GetCheck();
	util::CConfigHelper::GetInstance()->set_baidumap_auto_refresh(b);
	m_cmbBufferedAlarmList.EnableWindow(!b);
}
