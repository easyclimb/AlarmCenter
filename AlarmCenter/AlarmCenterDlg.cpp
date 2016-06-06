
// AlarmCenterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AppResource.h"
#include "AlarmCenterDlg.h"
#include "afxdialogex.h"
#include "AlarmMachineContainer.h"
#include "AlarmMachineManager.h"
#include "AlarmMachine.h"
#include "BtnST.h"
#include "NetworkConnector.h"
#include "AlarmCenterInfoDlg.h"
#include "UserInfo.h"
#include "LoginDlg.h"
#include "UserManagerDlg.h"
#include "HistoryRecord.h"
#include "GroupInfo.h"
#include "MachineManagerDlg.h"
#include "HistoryRecordDlg.h"
#include "ProgressDlg.h"
#include "ConfigHelper.h"
#include "SoundPlayer.h"
#include "DestroyProgressDlg.h"
#include "RemindQueryDlg.h"
#include "AutoQueryDisconnectSubmachineDlg.h"
#include "afxwin.h"
#include "CsrInfo.h"
#include "baidu.h"
#include "Gsm.h"
#include "ExportHrProcessDlg.h"
#include "BaiduMapViewerDlg.h"
#include "VideoPlayerDlg.h"
#include "InputDlg.h"
#include "VideoManager.h"
#include "VideoDeviceInfoEzviz.h"
#include "DetectorInfo.h"
#include "VideoUserInfo.h"
#include "ZoneInfo.h"

#include <algorithm>
#include <iterator>
#include <memory>

using namespace core;

namespace detail {
#define HOTKEY_MUTE 11

	const int cTimerIdTime = 1;
	const int cTimerIdHistory = 2;
	const int cTimerIdRefreshGroupTree = 3;
	const int cTimerIdHandleMachineAlarmOrDisalarm = 4;
	const int cTimerIdCheckTimeup = 5;
	const int cTimerIdHandleDisarmPasswdWrong = 6;

	const int GAP_4_CHECK_TIME_UP = 60 * 1000;

	const int TAB_NDX_NORMAL = 0;
	const int TAB_NDX_ALARMING = 1;

	BOOL GetFormatedProductVersion(CString& version)
	{
		CString path = _T("");
		path.Format(_T("%s\\VersionNo.ini"), GetModuleFilePath());
		CFile file;
		if (file.Open(path, CFile::modeRead)) {
			size_t length = static_cast<size_t>(file.GetLength());
			auto buff = std::unique_ptr<char[]>(new char[length + 1]);
			//memset(buff, 0, length + 1);
			file.Read(buff.get(), length);
			auto wbuff = std::unique_ptr<wchar_t[]>(AnsiToUtf16(buff.get()));
			version.Format(L"AlarmCenter, Version %s", wbuff);
			file.Close();
			return TRUE;
		}
		return FALSE;
	}

	const COLORREF cColorRed = RGB(255, 0, 0);
	const COLORREF cColorBlack = RGB(0, 0, 0);
};


class CAlarmCenterDlg::CurUserChangedObserver : public dp::observer<core::user_info_ptr>
{
public:
	explicit CurUserChangedObserver(CAlarmCenterDlg* dlg) : _dlg(dlg) {}
	virtual void on_update(const core::user_info_ptr& ptr) {
		if (_dlg) {
			_dlg->PostMessage(WM_CURUSERCHANGED, (WPARAM)(ptr->get_user_id()));
		}
	}
private:
	CAlarmCenterDlg* _dlg;
};

class CAlarmCenterDlg::NewRecordObserver : public dp::observer<core::history_record_ptr>
{
public:
	explicit NewRecordObserver(CAlarmCenterDlg* dlg) : _dlg(dlg) {}
	virtual void on_update(const core::history_record_ptr& ptr) {
		if (_dlg) {
			std::lock_guard<std::mutex> lock(_dlg->m_lock4RecordList);
			_dlg->m_recordList.AddTail(ptr->record);
		}
	}
private:
	CAlarmCenterDlg* _dlg;
};


class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	CStatic m_staticVersion;
	virtual BOOL OnInitDialog();
	CEdit m_edit;
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_VER, m_staticVersion);
	DDX_Control(pDX, IDC_EDIT1, m_edit);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CAlarmCenterDlg dialog



CAlarmCenterDlg::CAlarmCenterDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CAlarmCenterDlg::IDD, pParent)
	, m_wndContainer(nullptr)
	, m_wndContainerAlarming(nullptr)
	, m_hIconComputer(nullptr)
	, m_hIconConnection(nullptr)
	, m_hIconInternet(nullptr)
	, m_alarmCenterInfoDlg(nullptr)
	, m_curselTreeItem(nullptr)
	, m_curselTreeItemData(0)
	, m_maxHistory2Show(20)
	, m_times4GroupOnlineCntChanged(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	//HINSTANCE dllShell32 = LoadLibrary(L"shell32.dll");
	//m_hIconComputer = LoadIcon(dllShell32, MAKEINTRESOURCE(16));
	//m_hIconInternet = LoadIcon(dllShell32, MAKEINTRESOURCE(14));
	//FreeLibrary(dllShell32);


}

void CAlarmCenterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_GROUP, m_groupMachineList);
	DDX_Control(pDX, IDC_STATIC_SYSTIME, m_staticSysTime);
	DDX_Control(pDX, IDC_STATIC_CONTROL_PANEL, m_groupControlPanel);
	DDX_Control(pDX, IDC_STATIC_G_NET, m_gNet);
	DDX_Control(pDX, IDC_STATIC_G_CSR, m_gCsr);
	DDX_Control(pDX, IDC_STATIC_TRANSMIT_STATUS, m_sTransmitServerStatus);
	DDX_Control(pDX, IDC_STATIC_LOCAL_PORT, m_sLocalPort);
	DDX_Control(pDX, IDC_EDIT_CUR_USER_ID, m_cur_user_id);
	DDX_Control(pDX, IDC_EDIT_CUR_USER_NAME, m_cur_user_name);
	DDX_Control(pDX, IDC_EDIT_CUR_USER_PHONE, m_cur_user_phone);
	DDX_Control(pDX, IDC_EDIT_CUR_USER_PRIORITY, m_cur_user_priority);
	DDX_Control(pDX, IDC_BUTTON_USERMGR, m_btnUserMgr);
	DDX_Control(pDX, IDC_LIST_HISTORY, m_listHistory);
	DDX_Control(pDX, IDC_TREE_MACHINE_GROUP, m_treeGroup);
	DDX_Control(pDX, IDC_TAB_CONTAINER, m_tab);
	DDX_Control(pDX, IDC_BUTTON_MACHINEMGR, m_btnMachineMgr);
	DDX_Control(pDX, IDC_BUTTON_SEE_MORE_HR, m_btnSeeMoreHr);
	DDX_Control(pDX, IDC_STATIC_GROUP_HISTORY, m_groupHistory);
	DDX_Control(pDX, IDC_STATIC_TRANSMIT_STATUS_BK, m_sTransmitServerBkStatus);
}

BEGIN_MESSAGE_MAP(CAlarmCenterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_NETWORKSTARTUPOK, &CAlarmCenterDlg::OnMsgTransmitserver)
	ON_MESSAGE(WM_CURUSERCHANGED, &CAlarmCenterDlg::OnMsgCuruserchangedResult)
	ON_BN_CLICKED(IDC_BUTTON_SWITCH_USER, &CAlarmCenterDlg::OnBnClickedButtonSwitchUser)
	ON_BN_CLICKED(IDC_BUTTON_USERMGR, &CAlarmCenterDlg::OnBnClickedButtonUsermgr)
	ON_BN_CLICKED(IDC_BUTTON_VIEW_QRCODE, &CAlarmCenterDlg::OnBnClickedButtonViewQrcode)
	ON_WM_CLOSE()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_MACHINE_GROUP, &CAlarmCenterDlg::OnTvnSelchangedTreeMachineGroup)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_CONTAINER, &CAlarmCenterDlg::OnTcnSelchangeTabContainer)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_MACHINE_GROUP, &CAlarmCenterDlg::OnNMDblclkTreeMachineGroup)
	ON_BN_CLICKED(IDC_BUTTON_MACHINEMGR, &CAlarmCenterDlg::OnBnClickedButtonMachinemgr)
	ON_BN_CLICKED(IDC_BUTTON_SEE_MORE_HR, &CAlarmCenterDlg::OnBnClickedButtonSeeMoreHr)
	ON_BN_CLICKED(IDC_BUTTON_MUTE, &CAlarmCenterDlg::OnBnClickedButtonMute)
	ON_MESSAGE(WM_NEEDQUERYSUBMACHINE, &CAlarmCenterDlg::OnMsgNeedQuerySubMachine)
	ON_MESSAGE(WM_NEED_TO_EXPORT_HR, &CAlarmCenterDlg::OnMsgNeedToExportHr)
	ON_WM_HOTKEY()
	ON_NOTIFY(NM_RCLICK, IDC_TREE_MACHINE_GROUP, &CAlarmCenterDlg::OnNMRClickTreeMachineGroup)
	ON_MESSAGE(WM_EXIT_ALARM_CENTER, &CAlarmCenterDlg::OnMsgWmExitProcess)
	ON_MESSAGE(WM_REMINDER_TIME_UP, &CAlarmCenterDlg::OnReminderTimeUp)
	ON_MESSAGE(WM_SERVICE_TIME_UP, &CAlarmCenterDlg::OnServiceTimeUp)
	ON_MESSAGE(WM_DISARM_PASSWD_WRONG, &CAlarmCenterDlg::OnMsgDisarmPasswdWrong)
END_MESSAGE_MAP()


// CAlarmCenterDlg message handlers

BOOL CAlarmCenterDlg::OnInitDialog()
{
	AUTO_LOG_FUNCTION;
	CDialogEx::OnInitDialog();

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr) {
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadStringW(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty()) {
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	JLOG(L"REGISTER USERINFO\n");
	core::user_manager* userMgr = core::user_manager::GetInstance();
	core::user_info_ptr user = userMgr->GetCurUserInfo();
	OnMsgCuruserchangedResult((WPARAM)user->get_user_id(), 0);
	m_cur_user_changed_observer = std::make_shared<CurUserChangedObserver>(this);
	userMgr->register_observer(m_cur_user_changed_observer);
	JLOG(L"REGISTER USERINFO ok\n");


	CString welcom;
	welcom = GetStringFromAppResource(IDS_STRING_WELCOM);
	m_new_record_observer = std::make_shared<NewRecordObserver>(this);
	core::history_record_manager* hr = core::history_record_manager::GetInstance();
	hr->register_observer(m_new_record_observer);
	hr->InsertRecord(-1, -1, welcom, time(nullptr), core::RECORD_LEVEL_SYSTEM);
	JLOG(L"INSERT WELCOM OK\n");

	//m_progressDlg = new CLoadFromDBProgressDlg();
	CLoadFromDBProgressDlg dlg;
	dlg.DoModal();
	JLOG(L"CLoadFromDBProgressDlg DoModal OK\n");
	//m_progressDlg->Create(IDD_DIALOG_PROGRESS, this);

	auto cfg = util::CConfigHelper::GetInstance();
	CString sPort;
	sPort.Format(L"%d", cfg->get_listening_port());
	m_sLocalPort.SetWindowTextW(sPort);

	
	//m_cur_user_id.EnableWindow(0);
	//m_cur_user_name.EnableWindow(0);
	//m_cur_user_phone.EnableWindow(0);

	SetTimer(detail::cTimerIdTime, 1000, nullptr);
	SetTimer(detail::cTimerIdHistory, 1000, nullptr);
	SetTimer(detail::cTimerIdRefreshGroupTree, 1000, nullptr);
	SetTimer(detail::cTimerIdHandleMachineAlarmOrDisalarm, 1000, nullptr);
	SetTimer(detail::cTimerIdCheckTimeup, detail::GAP_4_CHECK_TIME_UP, nullptr);
	SetTimer(detail::cTimerIdHandleDisarmPasswdWrong, 1000, nullptr);
//#if !defined(DEBUG) && !defined(_DEBUG)
	//SetWindowPos(&CWnd::wndTopMost, 0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN), SWP_SHOWWINDOW);
//#else
	//MoveWindow(0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN), TRUE);
//#endif

	//ShowWindow(SW_MAXIMIZE);
	//CRect rc;
	//GetWindowRect(rc);
	//ShowWindow(SW_NORMAL);
	RECT rt;
	SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&rt, 0);
	MoveWindow(CRect(rt), TRUE);


	InitDisplay();
	InitAlarmMacineTreeView();
	if (net::CNetworkConnector::GetInstance()->StartNetwork()) {
		CString s; s.Format(L"%d", cfg->get_listening_port());
		m_sLocalPort.SetWindowTextW(s);
	}

	auto network_mode = cfg->get_network_mode();
	CString grouptitle, unused;
	unused = GetStringFromAppResource(IDS_STRING_NETMODE_NONE);
	switch (network_mode) {
	case util::NETWORK_MODE_TRANSMIT:
		grouptitle = GetStringFromAppResource(IDS_STRING_NETMODE_TRANSMIT);
		m_sLocalPort.SetWindowTextW(unused);
		break;
	case util::NETWORK_MODE_DUAL:
		grouptitle = GetStringFromAppResource(IDS_STRING_NETMODE_DUAL);
		break;
	case util::NETWORK_MODE_CSR:
		grouptitle = GetStringFromAppResource(IDS_STRING_NETMODE_CSR);
		m_sTransmitServerStatus.SetWindowTextW(unused);
		m_sTransmitServerBkStatus.SetWindowTextW(unused);
	default:
		break;
	}
	m_gNet.SetWindowTextW(grouptitle);

	RegisterHotKey(GetSafeHwnd(), HOTKEY_MUTE, MOD_CONTROL, 'M');

	return TRUE;  // return TRUE  unless you set the focus to a control
}




void CAlarmCenterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	} else {
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAlarmCenterDlg::OnPaint()
{
	if (IsIconic()) {
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	} else {
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAlarmCenterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CAlarmCenterDlg::InitAlarmMacineTreeView()
{
	AUTO_LOG_FUNCTION;
	using namespace core;
	group_manager* mgr = group_manager::GetInstance();
	group_info_ptr rootGroup = mgr->GetRootGroupInfo();
	//rootGroup->register_observer(this, OnGroupOnlineMachineCountChanged);
	m_observer = std::make_shared<observer>(this);
	rootGroup->register_observer(m_observer);
	if (rootGroup) {
		CString txt;
		txt.Format(L"%s[%d/%d/%d]", rootGroup->get_formatted_group_name(),
				   rootGroup->get_alarming_descendant_machine_count(),
				   rootGroup->get_online_descendant_machine_count(),
				   rootGroup->get_descendant_machine_count());
		HTREEITEM hRoot = m_treeGroup.GetRootItem();
		HTREEITEM hRootGroup = m_treeGroup.InsertItem(txt, hRoot);
		m_treeGroup.SetItemData(hRootGroup, (DWORD_PTR)rootGroup->get_id());

		TraverseGroup(hRootGroup, rootGroup);

		bool ok = false;
		group_info_ptr curselGroupInfo = nullptr;
		if (rootGroup->get_child_group_count() > 0) {
			m_curselTreeItem = nullptr;
			m_curselTreeItemData = 0;
			group_info_ptr firstChildGroup = rootGroup->GetFirstChildGroupInfo();
			ok = SelectGroupItemOfTree((DWORD)firstChildGroup->get_id());
			if (ok)
				curselGroupInfo = firstChildGroup;
		} 

		if (!ok) {
			m_curselTreeItem = hRootGroup;
			m_curselTreeItemData = (DWORD)rootGroup->get_id();
			curselGroupInfo = rootGroup;
		}

		TCITEM tcItem;
		tcItem.mask = TCIF_TEXT;
		CString name = curselGroupInfo->get_formatted_group_name();
		tcItem.pszText = name.LockBuffer();
		m_tab.SetItem(detail::TAB_NDX_NORMAL, &tcItem);
		name.UnlockBuffer();
		m_tab.Invalidate(0);
		m_wndContainer->ShowMachinesOfGroup(curselGroupInfo);

		m_treeGroup.Expand(hRootGroup, TVE_EXPAND);
	}
}


void CAlarmCenterDlg::TraverseGroup(HTREEITEM hItemGroup, core::group_info_ptr group)
{
	using namespace core;
	CString txt;
	group_info_list groupList;
	group->GetChildGroups(groupList);

	for (auto child_group : groupList) {
		txt.Format(L"%s[%d/%d/%d]", child_group->get_formatted_group_name(),
				   child_group->get_alarming_descendant_machine_count(),
				   child_group->get_online_descendant_machine_count(), 
				   child_group->get_descendant_machine_count());
		HTREEITEM hChildItem = m_treeGroup.InsertItem(txt, hItemGroup);
		m_treeGroup.SetItemData(hChildItem, (DWORD_PTR)child_group->get_id());
		TraverseGroup(hChildItem, child_group);
	}
}


void CAlarmCenterDlg::TraverseGroupTree(HTREEITEM hItemParent)
{
	auto parent_group = core::group_manager::GetInstance()->GetGroupInfo(m_treeGroup.GetItemData(hItemParent));
	CString txt = L"";
	txt.Format(L"%s[%d/%d/%d]", parent_group->get_formatted_group_name(),
			   parent_group->get_alarming_descendant_machine_count(),
			   parent_group->get_online_descendant_machine_count(),
			   parent_group->get_descendant_machine_count());
	m_treeGroup.SetItemText(hItemParent, txt);
	if (parent_group->get_alarming_descendant_machine_count() > 0) {
		m_treeGroup.SetItemColor(hItemParent, detail::cColorRed);
	} else {
		m_treeGroup.SetItemColor(hItemParent, detail::cColorBlack);
	}
	HTREEITEM hItem = m_treeGroup.GetChildItem(hItemParent);
	while (hItem) {
		auto group = core::group_manager::GetInstance()->GetGroupInfo(m_treeGroup.GetItemData(hItem));
		if (group) {
			txt.Format(L"%s[%d/%d/%d]", group->get_formatted_group_name(),
					   group->get_alarming_descendant_machine_count(),
					   group->get_online_descendant_machine_count(),
					   group->get_descendant_machine_count());
			m_treeGroup.SetItemText(hItem, txt);
			if (group->get_alarming_descendant_machine_count() > 0) {
				m_treeGroup.SetItemColor(hItem, detail::cColorRed);
			} else {
				m_treeGroup.SetItemColor(hItem, detail::cColorBlack);
			}
		}
		if (m_treeGroup.ItemHasChildren(hItem))
			TraverseGroupTree(hItem);
		hItem = m_treeGroup.GetNextSiblingItem(hItem);
	}
}


void CAlarmCenterDlg::InitDisplay()
{
	AUTO_LOG_FUNCTION;
	CRect rc;
	GetClientRect(rc);

	rc.DeflateRect(5, 5, 5, 5);
	CRect rcLeft(rc);
	rcLeft.right = rcLeft.left + 218;
	CRect rcRight(rc);
	rcRight.left = rcLeft.right + 5;

	m_groupControlPanel.MoveWindow(rcLeft);
	//m_groupMachineList.MoveWindow(rcRight);
	m_tab.MoveWindow(rcRight);

	CRect rcHistory;
	m_groupHistory.GetWindowRect(rcHistory);
	ScreenToClient(rcHistory);
	rcHistory.bottom = rcLeft.bottom - 5;
	m_groupHistory.MoveWindow(rcHistory);
	rcHistory.DeflateRect(5, 18, 5, 5);
	m_listHistory.MoveWindow(rcHistory);
	m_listHistory.GetClientRect(rcHistory);
	int columnHeight = m_listHistory.GetItemHeight(0);
	m_maxHistory2Show = rcHistory.Height() / columnHeight - 2;

	
	//CRect rcQrcode(rcLeft);
	//rcQrcode.DeflateRect(5, 5, 5, 5);
	//rcQrcode.top += 520;
	//rcQrcode.bottom = rcQrcode.top + rcQrcode.Width() + 20;
	////ScreenToClient(rcQrcode);
	////ClientToScreen(rcQrcode);
	//m_qrcodeViewDlg->MoveWindow(rcQrcode);
	//m_qrcodeViewDlg->ShowWindow(SW_SHOW);
	//rcQrcode.top = rcQrcode.bottom + 5;
	//rcQrcode.bottom = rcQrcode.top + 5;
	//m_staticSysTime.MoveWindow(rcQrcode);

	auto deleter = [](CAlarmMachineContainerDlg* dlg) {SAFEDELETEDLG(dlg); };

	m_wndContainer = std::shared_ptr<CAlarmMachineContainerDlg>(new CAlarmMachineContainerDlg(this), deleter);
	m_wndContainer->Create(IDD_DIALOG_CONTAINER, &m_tab);
	CString txt;
	txt = GetStringFromAppResource(IDS_STRING_GROUP_ROOT);
	m_tab.InsertItem(detail::TAB_NDX_NORMAL, txt);

	m_wndContainerAlarming = std::shared_ptr<CAlarmMachineContainerDlg>(new CAlarmMachineContainerDlg(this), deleter);
	m_wndContainerAlarming->Create(IDD_DIALOG_CONTAINER, &m_tab);
	// m_tab.InsertItem(TAB_NDX_ALARMING, L"Alarming");

	CRect rcTab;
	m_tab.GetClientRect(rcTab);
	rcTab.DeflateRect(5, 25, 5, 5);
	// rcRight.DeflateRect(5, 15, 5, 5);
	m_wndContainer->MoveWindow(rcTab);
	m_wndContainer->ShowWindow(SW_SHOW);

	m_wndContainerAlarming->MoveWindow(rcTab);
	m_wndContainerAlarming->ShowWindow(SW_HIDE);

	m_tab.SetCurSel(detail::TAB_NDX_NORMAL);

	g_baiduMapDlg = new CBaiduMapViewerDlg();
	g_baiduMapDlg->Create(IDD_DIALOG_PICK_MACHINE_COOR, this);
	g_baiduMapDlg->ShowWindow(SW_SHOW);

	g_videoPlayerDlg = new CVideoPlayerDlg();
	g_videoPlayerDlg->Create(IDD_DIALOG_VIDEO_PLAYER, this);
	g_videoPlayerDlg->ShowWindow(SW_SHOW);

	m_alarmCenterInfoDlg = std::shared_ptr<CAlarmCenterInfoDlg>(new CAlarmCenterInfoDlg(this), [](CAlarmCenterInfoDlg* dlg) { SAFEDELETEDLG(dlg); });
	m_alarmCenterInfoDlg->Create(IDD_DIALOG_CSR_ACCT, this);

	// 2015-11-17 16:04:09 init video icon here
	//core::alarm_machine_manager::GetInstance()->LoadCameraInfoFromDB();
	video::ezviz::video_device_info_ezviz_list devList;
	video::video_manager::GetInstance()->GetVideoDeviceEzvizWithDetectorList(devList);
	if (!devList.empty()) {
		for (auto dev : devList) {
			core::alarm_machine_manager::GetInstance()->ResolveCameraInfo(dev->get_id(), dev->get_userInfo()->get_productorInfo().get_productor());
		}
	}
}


void CAlarmCenterDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (detail::cTimerIdTime == nIDEvent) {
		SYSTEMTIME st = { 0 };
		::GetLocalTime(&st);
		wchar_t now[1024] = { 0 };
		wsprintfW(now, L"%04d-%02d-%02d %02d:%02d:%02d", st.wYear,
				  st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		m_staticSysTime.SetWindowTextW(now);
	} else if (detail::cTimerIdHistory == nIDEvent) {
		if (m_lock4RecordList.try_lock()) {
			std::lock_guard<std::mutex> lock(m_lock4RecordList, std::adopt_lock);
			m_listHistory.SetRedraw(0);
			while (m_recordList.GetCount() > 0) {
				CString record = m_recordList.RemoveHead();
				if (record.IsEmpty()) {
					// trick. means its time to clear hr
					//m_recordList.RemoveAll();
					m_listHistory.ResetContent();
					break;
				}
				if (m_listHistory.GetCount() > m_maxHistory2Show) 
					m_listHistory.DeleteString(0);
				m_listHistory.InsertString(-1, record);
			}
			m_listHistory.SetRedraw();
		}
	} else if (detail::cTimerIdRefreshGroupTree == nIDEvent) {
		auto_timer timer(m_hWnd, detail::cTimerIdRefreshGroupTree, 500);
		if (m_times4GroupOnlineCntChanged > 0) {
			TraverseGroupTree(m_treeGroup.GetRootItem());
			m_times4GroupOnlineCntChanged = 0;
			//m_treeGroup.Invalidate();
			group_manager* mgr = group_manager::GetInstance();
			group_info_ptr rootGroup = mgr->GetRootGroupInfo();
			if (rootGroup->get_alarming_descendant_machine_count() == 0) {
				sound_manager::GetInstance()->Stop();
			}

			//RefreshCurrentGroup();
		}
	} else if (detail::cTimerIdHandleMachineAlarmOrDisalarm == nIDEvent) {
		auto_timer timer(m_hWnd, detail::cTimerIdHandleMachineAlarmOrDisalarm, 500);
		HandleMachineAlarm();
	} else if (detail::cTimerIdCheckTimeup == nIDEvent) {
		auto_timer timer(m_hWnd, detail::cTimerIdCheckTimeup, detail::GAP_4_CHECK_TIME_UP);
		core::alarm_machine_list list;
		
		auto mgr = core::alarm_machine_manager::GetInstance();
		if (m_lock_4_timeup.try_lock()) {
			std::lock_guard<std::mutex> lock(m_lock_4_timeup, std::adopt_lock);
			for (auto pair : m_reminder_timeup_list) {
				core::alarm_machine_ptr target_machine = nullptr;
				auto machine = mgr->GetMachine(pair.first);
				if (machine) {
					if (pair.second != 0) {
						auto zone = machine->GetZone(pair.second);
						if (zone) {
							target_machine = zone->GetSubMachineInfo();
						}
					} else {
						target_machine = machine;
					}
				}

				if (target_machine) {
					list.push_back(target_machine);
				}
			}
			m_reminder_timeup_list.clear();

			for (auto pair : m_service_timeup_list) {
				core::alarm_machine_ptr target_machine = nullptr;
				auto machine = mgr->GetMachine(pair.first);
				if (machine) {
					if (pair.second != 0) {
						auto zone = machine->GetZone(pair.second);
						if (zone) {
							target_machine = zone->GetSubMachineInfo();
						}
					} else {
						target_machine = machine;
					}
				}

				if (target_machine) {
					list.push_back(target_machine);
				}
			}
			m_service_timeup_list.clear();
		}

		std::list<std::wstring> msg_list;

		for (auto machine : list) {
			auto now = std::chrono::system_clock::now();
			auto expire = machine->get_expire_time();
			auto remind = machine->get_consumer()->remind_time;
			auto span1 = expire - now;
			auto span2 = remind - now;
			auto mins = std::chrono::duration_cast<std::chrono::minutes>(span1).count();
			auto min2 = std::chrono::duration_cast<std::chrono::minutes>(span2).count();

			auto consumer = machine->get_consumer();

			if (min2 <= 0) {
				CString s = GetStringFromAppResource(IDS_STRING_REMIND_TIME_UP);
				s.AppendFormat(L"\r\n%s" // ����
							   L"\r\n%s:%s" // ����
							   L"\r\n%s:%d" // Ӧ��
							   L"\r\n%s:%d" // �Ѹ�
							   L"\r\n%s:%d" // Ƿ��
							   L"\r\n%s:%s" // �Ƿ�Ƿ��
							   L"\r\n%s:%s" // ��ϵ��
							   L"\r\n%s:%s" // �绰
							   L"\r\n%s:%s", // ���õ绰
							   machine->get_formatted_name(),
							   GetStringFromAppResource(IDS_STRING_TYPE), consumer->type->name,
							   GetStringFromAppResource(IDS_STRING_RECEIVABLE), consumer->receivable_amount,
							   GetStringFromAppResource(IDS_STRING_PAID), consumer->paid_amount,
							   GetStringFromAppResource(IDS_STRING_OWED), consumer->get_owed_amount(),
							   GetStringFromAppResource(IDS_STRING_IS_OWED), GetStringFromAppResource(consumer->get_owed_amount() > 0 ? IDS_STRING_YES : IDS_STRING_NO),
							   GetStringFromAppResource(IDS_STRING_CONTACT), machine->get_contact(),
							   GetStringFromAppResource(IDS_STRING_PHONE), machine->get_phone(),
							   GetStringFromAppResource(IDS_STRING_PHONE_BK), machine->get_phone_bk()
							   );
				msg_list.push_back((LPCTSTR)s);
			}

			if (mins <= 0) {
				CString s = GetStringFromAppResource(IDS_STRING_EXPIRE);
				s.AppendFormat(L"\r\n%s" // ����
							   L"\r\n%s:%s" // ����
							   L"\r\n%s:%d" // Ӧ��
							   L"\r\n%s:%d" // �Ѹ�
							   L"\r\n%s:%d" // Ƿ��
							   L"\r\n%s:%s" // �Ƿ�Ƿ��
							   L"\r\n%s:%s" // ��ϵ��
							   L"\r\n%s:%s" // �绰
							   L"\r\n%s:%s", // ���õ绰
							   machine->get_formatted_name(),
							   GetStringFromAppResource(IDS_STRING_TYPE), consumer->type->name,
							   GetStringFromAppResource(IDS_STRING_RECEIVABLE), consumer->receivable_amount,
							   GetStringFromAppResource(IDS_STRING_PAID), consumer->paid_amount,
							   GetStringFromAppResource(IDS_STRING_OWED), consumer->get_owed_amount(),
							   GetStringFromAppResource(IDS_STRING_IS_OWED), GetStringFromAppResource(consumer->get_owed_amount() > 0 ? IDS_STRING_YES : IDS_STRING_NO),
							   GetStringFromAppResource(IDS_STRING_CONTACT), machine->get_contact(),
							   GetStringFromAppResource(IDS_STRING_PHONE), machine->get_phone(),
							   GetStringFromAppResource(IDS_STRING_PHONE_BK), machine->get_phone_bk()
							   );
				msg_list.push_back((LPCTSTR)s);
			}
		}

		list.clear();

		for (auto msg : msg_list) {
			MessageBox(msg.c_str());
		}

	} else if (detail::cTimerIdHandleDisarmPasswdWrong == nIDEvent) {
		auto_timer timer(m_hWnd, detail::cTimerIdHandleDisarmPasswdWrong, 1000);
		if (m_lock_4_passwd_wrong_ademco_id_list.try_lock()) {
			std::lock_guard<std::mutex> lock(m_lock_4_passwd_wrong_ademco_id_list, std::adopt_lock);
			for (auto ademco_id : m_disarm_passwd_wrong_ademco_id_list) {
				HandleMachineDisarmPasswdWrong(ademco_id);
			}
			m_disarm_passwd_wrong_ademco_id_list.clear();
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CAlarmCenterDlg::HandleMachineDisarmPasswdWrong(int ademco_id)
{
	auto mgr = alarm_machine_manager::GetInstance();
	alarm_machine_ptr machine = mgr->GetMachine(ademco_id);
	if (!machine)return;

	CString record;
	record.Format(L"%s %s %s",
				  machine->get_formatted_name(), 
				  GetStringFromAppResource(IDS_STRING_DISARM), 
				  GetStringFromAppResource(IDS_STRING_USER_PASSWD_WRONG));

	history_record_manager::GetInstance()->InsertRecord(ademco_id, 0,
														record, time(nullptr),
														RECORD_LEVEL_USERCONTROL);

	record.AppendFormat(L"\r\n%s", GetStringFromAppResource(IDS_STRING_ASK_RE_INPUT));
	if (IDYES != MessageBox(record, L"", MB_YESNOCANCEL))
		return;

	auto xdata = std::make_shared<ademco::char_array>();
	CInputPasswdDlg dlg(this);
	//dlg.m_prefix_title.Format(L"%s%s", GetStringFromAppResource(IDS_STRING_MACHINE), machine->get_formatted_machine_name());
	if (dlg.DoModal() != IDOK)
		return;
	if (dlg.m_edit.GetLength() != 6)
		return;

	CString srecord, suser, sfm, sop, snull;
	suser = GetStringFromAppResource(IDS_STRING_USER);
	sfm = GetStringFromAppResource(IDS_STRING_LOCAL_OP);
	sop = GetStringFromAppResource(IDS_STRING_DISARM);
	snull = GetStringFromAppResource(IDS_STRING_NULL);
	
	user_info_ptr user = user_manager::GetInstance()->GetCurUserInfo();
	
	srecord.Format(L"%s(ID:%d,%s)%s:%s%s", suser,
				   user->get_user_id(), user->get_user_name(),
				   sfm, sop, machine->get_formatted_name());
	history_record_manager::GetInstance()->InsertRecord(machine->get_ademco_id(), 0,
												srecord, time(nullptr),
												RECORD_LEVEL_USERCONTROL);

	USES_CONVERSION;
	const char* a = W2A(dlg.m_edit);
	for (int i = 0; i < 6; i++) {
		xdata->push_back(a[i]);
	}

	net::CNetworkConnector::GetInstance()->Send(ademco_id, ademco::EVENT_DISARM, 0, 0, xdata);
}


void CAlarmCenterDlg::RefreshCurrentGroup()
{
	m_wndContainer->Refresh();
}


void CAlarmCenterDlg::OnDestroy()
{
	AUTO_LOG_FUNCTION;

	CDialogEx::OnDestroy();
}


afx_msg LRESULT CAlarmCenterDlg::OnMsgTransmitserver(WPARAM wParam, LPARAM lParam)
{
	BOOL online = static_cast<BOOL>(wParam);
	CString status; CString txt;
	BOOL main_client = static_cast<BOOL>(lParam);
	//if (main_client) {
	//	if (online) {
	//		status = GetStringFromAppResource(IDS_STRING_TRANSMIT_CONN);
	//		m_sTransmitServerStatus.SetWindowTextW(status);
	//		txt = GetStringFromAppResource(main_client ? IDS_STRING_CONN_TO_SERVER_OK : IDS_STRING_TRANSMITBK_CONN);
	//	} else {
	//		status = GetStringFromAppResource(IDS_STRING_TRANSMIT_DISCONN);
	//		m_sTransmitServerStatus.SetWindowTextW(status);
	//		txt = GetStringFromAppResource(main_client ? IDS_STRING_LOST_SERVER_CONN : IDS_STRING_TRANSMITBK_DISCONN);
	//	}
	//} else { // m_sTransmitServerBkStatus
	auto hr = core::history_record_manager::GetInstance();
	if (online) {
		status = GetStringFromAppResource(IDS_STRING_TRANSMIT_CONN);
		main_client ? m_sTransmitServerStatus.SetWindowTextW(status) : m_sTransmitServerBkStatus.SetWindowTextW(status);
		txt = GetStringFromAppResource(main_client ? IDS_STRING_CONN_TO_SERVER_OK : IDS_STRING_CONN_TO_SERVERBK_OK);
		hr->InsertRecord(-1, -1, txt, time(nullptr), core::RECORD_LEVEL_SYSTEM);
	} else {
		status = GetStringFromAppResource(IDS_STRING_TRANSMIT_DISCONN);
		txt = GetStringFromAppResource(main_client ? IDS_STRING_LOST_SERVER_CONN : IDS_STRING_LOST_SERVERBK_CONN);
		hr->InsertRecord(-1, -1, txt, time(nullptr), core::RECORD_LEVEL_SYSTEM);
		//txt = GetStringFromAppResource(IDS_STRING_WILL_CONN_IN_TIME);
		//hr->InsertRecord(-1, -1, txt, time(nullptr), core::RECORD_LEVEL_SYSTEM);
	}
	//}
	main_client ? m_sTransmitServerStatus.SetWindowTextW(status) : m_sTransmitServerBkStatus.SetWindowTextW(status);
	

	return 0;
}


afx_msg LRESULT CAlarmCenterDlg::OnMsgCuruserchangedResult(WPARAM wParam, LPARAM /*lParam*/)
{
	auto user = core::user_manager::GetInstance()->GetUserInfo(wParam); assert(user);

	CString user_id;
	user_id.Format(L"%d", user->get_user_id());
	m_cur_user_id.SetWindowTextW(user_id);
	m_cur_user_name.SetWindowTextW(user->get_user_name());
	m_cur_user_phone.SetWindowTextW(user->get_user_phone());
	core::user_priority user_priority = user->get_user_priority();
	CString sPriority;
	m_btnUserMgr.EnableWindow(0);
	m_btnMachineMgr.EnableWindow(0);
	switch (user_priority) {
		case core::UP_SUPER:
			sPriority = GetStringFromAppResource(IDS_STRING_USER_SUPER);
			m_btnUserMgr.EnableWindow(1);
			m_btnMachineMgr.EnableWindow(1);
			break;
		case core::UP_ADMIN:
			sPriority = GetStringFromAppResource(IDS_STRING_USER_ADMIN);
			m_btnMachineMgr.EnableWindow(1);
			break;
		case core::UP_OPERATOR:
		default:
			sPriority = GetStringFromAppResource(IDS_STRING_USER_OPERATOR);
			break;
	}
	m_cur_user_priority.SetWindowTextW(sPriority);
	return 0;
}


void CAlarmCenterDlg::OnBnClickedButtonSwitchUser()
{
	CLoginDlg dlg(this);
	dlg.DoModal();
}


void CAlarmCenterDlg::OnBnClickedButtonUsermgr()
{
	CUserManagerDlg dlg(this);
	dlg.DoModal();
}


void CAlarmCenterDlg::OnBnClickedButtonViewQrcode()
{
	//CAlarmCenterInfoDlg dlg(this);
	//dlg.DoModal();
	m_alarmCenterInfoDlg->ShowWindow(SW_SHOW);
}


void CAlarmCenterDlg::OnBnClickedButtonMachinemgr()
{
	AUTO_LOG_FUNCTION;

	auto_timer timer(m_hWnd, detail::cTimerIdCheckTimeup, detail::GAP_4_CHECK_TIME_UP);

	m_wndContainer->ShowMachinesOfGroup(nullptr);
	CMachineManagerDlg dlg(this);
	dlg.DoModal();
	
	m_treeGroup.DeleteAllItems();

	using namespace core;
	group_manager* mgr = group_manager::GetInstance();
	group_info_ptr rootGroup = mgr->GetRootGroupInfo();
	if (rootGroup) {
		rootGroup->SortDescendantGroupsByName();
		CString txt;
		txt.Format(L"%s[%d/%d]", rootGroup->get_formatted_group_name(),
				   rootGroup->get_online_descendant_machine_count(),
				   rootGroup->get_descendant_machine_count());
		HTREEITEM hRoot = m_treeGroup.GetRootItem();
		HTREEITEM hRootGroup = m_treeGroup.InsertItem(txt, hRoot);
		m_treeGroup.SetItemData(hRootGroup, (DWORD_PTR)rootGroup->get_id());

		TraverseGroup(hRootGroup, rootGroup);

		//m_curselTreeItem = hRootGroup;
		//m_curselTreeItemData = (DWORD)rootGroup;
		// ����ѡ���ϴ�ѡ�еķ�����
		bool ok = false;
		group_info_ptr curselGroupInfo = nullptr;
		if (m_curselTreeItemData != 0) {
			ok = SelectGroupItemOfTree(m_curselTreeItemData);
			if (ok) {
				curselGroupInfo = mgr->GetGroupInfo(m_curselTreeItemData);
			}
		}

		// ���о�ѡ���һ���ӷ�����
		if (!ok) {
			if (rootGroup->get_child_group_count() > 0) {
				m_curselTreeItem = nullptr;
				m_curselTreeItemData = 0;
				group_info_ptr firstChildGroup = rootGroup->GetFirstChildGroupInfo();
				ok = SelectGroupItemOfTree((DWORD)firstChildGroup->get_id());
				if (ok)
					curselGroupInfo = mgr->GetGroupInfo(m_curselTreeItemData);
			}
		}

		// ʵ�ڲ��У�û���ˣ�ѡ�����ɡ�����
		if (!ok) {
			curselGroupInfo = rootGroup;
		}

		TCITEM tcItem;
		tcItem.mask = TCIF_TEXT;
		CString name = curselGroupInfo->get_formatted_group_name();
		tcItem.pszText = name.LockBuffer();
		m_tab.SetItem(detail::TAB_NDX_NORMAL, &tcItem);
		m_tab.Invalidate(0);
		name.UnlockBuffer();
		m_wndContainer->ShowMachinesOfGroup(curselGroupInfo);
	}
	
}


void CAlarmCenterDlg::OnClose()
{
	CDialogEx::OnClose();
}


void CAlarmCenterDlg::OnCancel()
{
#if !defined(DEBUG) && !defined(_DEBUG)
	CString confirm;
	confirm = GetStringFromAppResource(IDS_STRING_CONFIRM_QUIT);
	int ret = MessageBox(confirm, L"", MB_YESNO | MB_ICONQUESTION);
	if (ret == IDNO)
		return;
#endif

#ifdef _DEBUG
#define SLEEP { std::this_thread::sleep_for(std::chrono::milliseconds(100)); }
#else
#define SLEEP
#endif

	ExitAlarmCenter();
}


void CAlarmCenterDlg::OnTvnSelchangedTreeMachineGroup(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	using namespace core;
	HTREEITEM hItem = m_treeGroup.GetSelectedItem();
	if (m_curselTreeItem == hItem) { return; } 
	m_curselTreeItem = hItem;
	DWORD data = m_treeGroup.GetItemData(hItem);
	m_curselTreeItemData = data;
	group_info_ptr group = core::group_manager::GetInstance()->GetGroupInfo(data);
	if (group) {
		// change tab item text
		TCITEM tcItem;
		tcItem.mask = TCIF_TEXT;
		CString name = group->get_formatted_group_name();
		tcItem.pszText = name.LockBuffer();
		m_tab.SetItem(detail::TAB_NDX_NORMAL, &tcItem);
		name.UnlockBuffer();
		m_tab.Invalidate(0);
		// load machine of this gruop
		m_wndContainer->ShowMachinesOfGroup(group);
		
	}

	*pResult = 0;
}


void CAlarmCenterDlg::OnNMRClickTreeMachineGroup(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	using namespace core;
	*pResult = 0;

	CPoint pt;
	GetCursorPos(&pt);
	m_treeGroup.ScreenToClient(&pt);
	HTREEITEM  hItem = m_treeGroup.HitTest(pt);
	if (hItem == nullptr) {
		return;
	} 
	m_treeGroup.ClientToScreen(&pt);
	DWORD data = m_treeGroup.GetItemData(hItem);
	group_info_ptr group = core::group_manager::GetInstance()->GetGroupInfo(data);
	if (group) {
		CString txt; txt = GetStringFromAppResource(IDS_STRING_CLR_ALM_MSG);
		CMenu menu, *pMenu;
		//menu.CreatePopupMenu();
		//menu.AppendMenuW(MF_STRING, 1, txt);
		//txt = GetStringFromAppResource(IDS_STRING_ARM);
		//menu.AppendMenuW(MF_STRING, 2, txt);
		//////txt = GetStringFromAppResource(IDS_STRING_HALFARM);
		//////menu.AppendMenuW(MF_STRING, 3, txt);
		////txt = GetStringFromAppResource(IDS_STRING_EMERGENCY);
		////menu.AppendMenuW(MF_STRING, 3, txt);
		menu.LoadMenuW(IDR_MENU4);
		pMenu = menu.GetSubMenu(0);
		if (!pMenu)return;

		if (hItem != m_treeGroup.GetRootItem()) {
			pMenu->DeleteMenu(0, MF_BYPOSITION);
		} else {
			auto way = group_manager::GetInstance()->get_cur_sort_machine_way();
			switch (way) {
			case core::sort_by_ademco_id:
				pMenu->CheckMenuItem(ID_32791, MF_CHECKED);
				break;
			case core::sort_by_name:
				pMenu->CheckMenuItem(ID_32794, MF_CHECKED);
				break;
			case core::sort_by_on_offline:
				pMenu->CheckMenuItem(ID_32792, MF_CHECKED);
				break;
			case core::sort_by_arm_disarm:
				pMenu->CheckMenuItem(ID_32793, MF_CHECKED);
				break;
			case core::sort_by_event_level:
				pMenu->CheckMenuItem(ID_32795, MF_CHECKED);
				break;
			default:
				break;
			}
		}

		auto filter = group->get_cur_filter_way();
		switch (filter) {
		case core::filter_by_all:
			pMenu->CheckMenuItem(ID_FILTER_ALL, MF_CHECKED);
			break;
		case core::filter_by_online:pMenu->CheckMenuItem(ID_FILTER_ONLINE, MF_CHECKED);
			break;
		case core::filter_by_offline:pMenu->CheckMenuItem(ID_FILTER_OFFLINE, MF_CHECKED);
			break;
		case core::filter_by_arm:pMenu->CheckMenuItem(ID_FILTER_ARM, MF_CHECKED);
			break;
		case core::filter_by_disarm:pMenu->CheckMenuItem(ID_FILTER_DISARM, MF_CHECKED);
			break;
		case core::filter_by_event:pMenu->CheckMenuItem(ID_FILTER_EVENT, MF_CHECKED);
			break;
		default:
			break;
		}

		DWORD ret = pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
										  pt.x, pt.y, this);

		auto mgr = core::alarm_machine_manager::GetInstance();

		bool b_sort = false, b_filter = false;
		auto way = sort_by_ademco_id;

		switch (ret) {
		case ID_SUB_32796: {// clear alarm info
			m_wndContainerAlarming->ClearButtonList();
			group->ClearAlarmMsgOfDescendantAlarmingMachine();

			core::alarm_machine_list list;
			for (int i = 0; i < MAX_MACHINE; i++) {
				auto machine = mgr->GetMachine(i);
				if (machine && machine->get_alarming()) {
					list.push_back(machine);
				}
			}
			if (list.empty()) {
				core::sound_manager::GetInstance()->Stop();
			}
			m_wndContainerAlarming->Reset(list);
		}
			break;

		case ID_SUB_32797: { // arm
			core::alarm_machine_list list;
			group->GetDescendantMachines(list);
			for (auto machine : list) {
				if(machine->get_online() && machine->get_machine_status() != MACHINE_ARM)
					mgr->RemoteControlAlarmMachine(machine, EVENT_ARM, 0, 0, nullptr, nullptr, ES_UNKNOWN, this);
			}
		}
			break;

		/************** sort machine***************/
		case ID_32791: // by ademco id
			b_sort = true;
			way = sort_by_ademco_id;
			break;

		case ID_32794: // by name
			b_sort = true;
			way = sort_by_name;
			break;

		case ID_32792: // by on/off line
			b_sort = true;
			way = sort_by_on_offline;
			break;

		case ID_32793: // by arm/disarm
			b_sort = true;
			way = sort_by_arm_disarm;
			break;

		case ID_32795: // by event level
			b_sort = true;
			way = sort_by_event_level;
			break;
		/************** sort end***************/


		/************** filter machine***************/
		case ID_FILTER_ALL:
			b_filter = true; filter = filter_by_all;
			break;

		case ID_FILTER_ONLINE:
			b_filter = true; filter = filter_by_online;
			break;

		case ID_FILTER_OFFLINE:
			b_filter = true; filter = filter_by_offline;
			break;

		case ID_FILTER_ARM:
			b_filter = true; filter = filter_by_arm;
			break;

		case ID_FILTER_DISARM:
			b_filter = true; filter = filter_by_disarm;
			break;

		case ID_FILTER_EVENT:
			b_filter = true; filter = filter_by_event;
			break;


		/************** filter end***************/

		default:
			break;
		}

		if (b_filter) {
			group->set_cur_filter_way(filter);
			SelectGroupItemOfTree(group->get_id());
			m_wndContainer->ShowMachinesOfGroup(group);
		}

		if (b_sort) {
			group_manager::GetInstance()->set_cur_sort_machine_way(way);
			if (hItem != m_curselTreeItem) {
				group = group_manager::GetInstance()->GetGroupInfo(m_curselTreeItemData);
			}
			SelectGroupItemOfTree(group->get_id());
			m_wndContainer->ShowMachinesOfGroup(group);
		}

		/*if (b_filter || b_sort) {
			if (group) {
				
				if (hItem != m_curselTreeItem) {
					m_wndContainer->ShowMachinesOfGroup(group);
				} else {
					m_wndContainer->Refresh();
				}
			}
		}*/
	}
}


void CAlarmCenterDlg::OnTcnSelchangeTabContainer(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	int ndx = m_tab.GetCurSel();
	if (detail::TAB_NDX_NORMAL == ndx) {
		m_wndContainer->ShowWindow(SW_SHOW);
		m_wndContainerAlarming->ShowWindow(SW_HIDE);
	} else if (detail::TAB_NDX_ALARMING == ndx) {
		m_wndContainer->ShowWindow(SW_HIDE);
		m_wndContainerAlarming->ShowWindow(SW_SHOW);
	}
	*pResult = 0;
}


void CAlarmCenterDlg::HandleMachineAlarm()
{
	if (!m_lock4AdemcoEvent.try_lock())
		return;
	using namespace core;
	std::lock_guard<std::mutex> lock(m_lock4AdemcoEvent, std::adopt_lock);
	auto mgr = group_manager::GetInstance();

	while (!m_machineAlarmOrDisalarmList.empty()) {
		auto ad = m_machineAlarmOrDisalarmList.front();
		m_machineAlarmOrDisalarmList.pop_front();
		if (ad->alarm) {
			group_info_ptr group = mgr->GetGroupInfo(ad->machine->get_group_id());
			if (group) {
				// select the group tree item if its not selected
				DWORD data = m_treeGroup.GetItemData(m_curselTreeItem);
				if (data != (DWORD)group->get_id()) {
					// if cur show group is ancestor, need not to show
					bool bCurShowGroupIsAncenstor = false;
					group_info_ptr parent_group = group->get_parent_group();
					while (parent_group) {
						if ((DWORD)parent_group->get_id() == data) {
							bCurShowGroupIsAncenstor = true;
							break;
						}
						parent_group = parent_group->get_parent_group();
					}

					if (!bCurShowGroupIsAncenstor) {
						SelectGroupItemOfTree(DWORD(group->get_id()));
					}
				}
			}

			if (m_tab.GetItemCount() == 1) {
				CString txt;
				txt = GetStringFromAppResource(IDS_STRING_TAB_TEXT_ALARMING);
				m_tab.InsertItem(detail::TAB_NDX_ALARMING, txt);

				m_tab.Invalidate(0);
			}

			m_wndContainerAlarming->InsertMachine(ad->machine, -1, true);

		} else {
			m_wndContainerAlarming->DeleteMachine(ad->machine);
			if (m_wndContainerAlarming->GetMachineCount() == 0) {
				m_wndContainerAlarming->ShowWindow(SW_HIDE);
				//if (m_tab.GetCurSel() != TAB_NDX_NORMAL) {
				m_tab.DeleteItem(detail::TAB_NDX_ALARMING);
				m_tab.SetCurSel(detail::TAB_NDX_NORMAL);
				m_wndContainer->ShowWindow(SW_SHOW);
				m_tab.Invalidate(0);
				//}


			}

		}
		RefreshCurrentGroup();
		//m_treeGroup.Invalidate();
		return;
	}
}


bool CAlarmCenterDlg::SelectGroupItemOfTree(DWORD data)
{
	AUTO_LOG_FUNCTION;
	HTREEITEM hRoot = m_treeGroup.GetRootItem();
	if (m_treeGroup.GetItemData(hRoot) == data) {
		m_treeGroup.SelectItem(hRoot);
		m_curselTreeItem = hRoot;
		return true;
	} else {
		bool ok = SelectGroupItemOfTreeHelper(hRoot, data);
		if (ok) {
			m_curselTreeItemData = data;
			return true;
		}
	}
	m_curselTreeItemData = 0;
	return false;
}

bool CAlarmCenterDlg::SelectGroupItemOfTreeHelper(HTREEITEM hItemParent, DWORD data)
{
	HTREEITEM hItem = m_treeGroup.GetChildItem(hItemParent);
	while (hItem) {
		DWORD local_data = m_treeGroup.GetItemData(hItem);
		if (local_data == data) {
			m_treeGroup.SelectItem(hItem);
			m_curselTreeItem = hItem;
			return true;
		}

		if (SelectGroupItemOfTreeHelper(hItem, data))
			return true;

		hItem = m_treeGroup.GetNextSiblingItem(hItem);
	}

	return false;
}


void CAlarmCenterDlg::OnNMDblclkTreeMachineGroup(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	*pResult = 0;
}


void CAlarmCenterDlg::OnBnClickedButtonSeeMoreHr()
{
	CHistoryRecordDlg dlg(this); dlg.DoModal();
}


void CAlarmCenterDlg::OnBnClickedButtonMute()
{
	core::sound_manager::GetInstance()->Stop();
	CString srecord, suser, sfm, sop, fmMachine, fmSubmachine;
	suser = GetStringFromAppResource(IDS_STRING_USER);
	sfm = GetStringFromAppResource(IDS_STRING_LOCAL_OP);
	fmMachine = GetStringFromAppResource(IDS_STRING_MACHINE);
	fmSubmachine = GetStringFromAppResource(IDS_STRING_SUBMACHINE);
	sop = GetStringFromAppResource(IDS_STRING_MUTE_ONCE);
	user_info_ptr user = user_manager::GetInstance()->GetCurUserInfo();
	srecord.Format(L"%s(ID:%d,%s)%s:%s", suser,
				   user->get_user_id(), user->get_user_name(),
				   sfm, sop);

	history_record_manager::GetInstance()->InsertRecord(-1, -1, srecord, time(nullptr),
												RECORD_LEVEL_USERCONTROL);
}


BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString txt; 
	if (!detail::GetFormatedProductVersion(txt)) {
		txt = _T("AlarmCenter, Version 1.0");
	}
	m_staticVersion.SetWindowTextW(txt);

	CString path;
	path.Format(L"%s\\ChangeLog.txt", GetModuleFilePath());
	CFile file;
	if (file.Open(path, CFile::modeRead)) {
		UINT len = static_cast<UINT>(file.GetLength());
		auto buf = std::unique_ptr<char[]>(new char[len + 1]);
		file.Read(buf.get(), len);
		buf[len] = 0;
		std::string u8 = buf.get();
		std::wstring u16;
		utf8::utf8to16(u8.begin(), u8.end(), std::back_inserter(u16));
		m_edit.SetWindowTextW(u16.c_str());
	} else {
		CString e;
		e.Format(L"Open file '%s' failed!", path);
		m_edit.SetWindowTextW(e);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


afx_msg LRESULT CAlarmCenterDlg::OnMsgNeedQuerySubMachine(WPARAM wParam, LPARAM lParam)
{
	auto subMachineList = std::unique_ptr<alarm_machine_list>(reinterpret_cast<alarm_machine_list*>(wParam));
	size_t size = static_cast<size_t>(lParam); VERIFY(subMachineList->size() == size);
	CAutoQueryDisconnectSubmachineDlg autoDlg(this);
	std::copy(subMachineList->begin(), subMachineList->end(), 
			  std::back_inserter(autoDlg.m_subMachineList));
	autoDlg.DoModal();
	return 0;
}


afx_msg LRESULT CAlarmCenterDlg::OnMsgNeedToExportHr(WPARAM wParam, LPARAM /*lParam*/)
{
	int curRecord = static_cast<int>(wParam);
	CExportHrProcessDlg dlg(this);
	dlg.m_nTotalCount = curRecord;
	dlg.DoModal();

	CString s, fm;
	fm = GetStringFromAppResource(IDS_STRING_SYSTEM_EXPORT_HR);
	s.Format(fm, dlg.m_excelPath);
	history_record_manager* hr = history_record_manager::GetInstance();
	hr->InsertRecord(-1, -1, s, time(nullptr), RECORD_LEVEL_SYSTEM);
	return 0;
}


void CAlarmCenterDlg::OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2)
{
	if (HOTKEY_MUTE == nHotKeyId) {
		OnBnClickedButtonMute();
	}

	CDialogEx::OnHotKey(nHotKeyId, nKey1, nKey2);
}


void CAlarmCenterDlg::ExitAlarmCenter()
{
	if (m_bExiting)
		return;

	m_bExiting = true;

	UnregisterHotKey(GetSafeHwnd(), HOTKEY_MUTE);
	//core::group_manager::GetInstance()->GetRootGroupInfo()->UnRegisterObserver(this);
	m_observer.reset();
	m_new_record_observer.reset();
	//core::history_record_manager::GetInstance()->UnRegisterObserver(this);
	ShowWindow(SW_HIDE);
	auto dlg = std::make_unique<CDestroyProgressDlg>();
	dlg->Create(IDD_DIALOG_DESTROY_PROGRESS, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);
	dlg->CenterWindow(this);
	dlg->UpdateWindow();
	SLEEP;

	CString s; int ndx = 0;
	s = GetStringFromAppResource(IDS_STRING_DESTROY_START);
	JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	SLEEP;

	// timer
	s = GetStringFromAppResource(IDS_STRING_DESTROY_TIMER); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	KillTimer(1);
	SLEEP;

	// alarmmachine container
	s = GetStringFromAppResource(IDS_STRING_DESTROY_CONTAINER); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	m_wndContainer = nullptr;
	SLEEP;

	// alarming alarmmachine container
	s = GetStringFromAppResource(IDS_STRING_DESTROY_ALARMING); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	m_wndContainerAlarming = nullptr;
	SLEEP;

	// qrcode viewer
	s = GetStringFromAppResource(IDS_STRING_DESTROY_QR); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	m_alarmCenterInfoDlg = nullptr;
	//SAFEDELETEDLG(m_progressDlg);
	SLEEP;

	// video
	s = GetStringFromAppResource(IDS_STRING_DESTROY_VIDEO); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	SAFEDELETEDLG(g_baiduMapDlg);
	SAFEDELETEDLG(g_videoPlayerDlg);

	// stop network
	s = GetStringFromAppResource(IDS_STRING_DESTROY_NET); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	net::CNetworkConnector::GetInstance()->StopNetwork();
	SLEEP;

	// destroy network
	s = GetStringFromAppResource(IDS_STRING_DESTROY_NETWORK); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	net::CNetworkConnector::ReleaseObject();
	SLEEP;

	// machine manager
	s = GetStringFromAppResource(IDS_STRING_DESTROY_MGR); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	core::alarm_machine_manager::ReleaseObject();
	SLEEP;

	// config helper
	s = GetStringFromAppResource(IDS_STRING_DESTROY_CFG); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	util::CConfigHelper::ReleaseObject();
	SLEEP;

	// app res
	s = GetStringFromAppResource(IDS_STRING_DESTROY_RES); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	CAppResource::ReleaseObject();
	SLEEP;

	// hisroty record
	s = GetStringFromAppResource(IDS_STRING_DESTROY_HR); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	CString goodbye;
	goodbye = GetStringFromAppResource(IDS_STRING_GOODBYE);
	core::history_record_manager* hr = core::history_record_manager::GetInstance();
	hr->InsertRecord(-1, -1, goodbye, time(nullptr), core::RECORD_LEVEL_SYSTEM);
	//hr->UnRegisterObserver(this);
	hr->ReleaseObject();
	SLEEP;

	// user manager
	s = GetStringFromAppResource(IDS_STRING_DESTROY_USER); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	core::user_manager::ReleaseObject();
	SLEEP;

	// ok
	s = GetStringFromAppResource(IDS_STRING_DESTROY_SND); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	core::sound_manager::GetInstance()->Stop();
	core::sound_manager::ReleaseObject();
	SLEEP;

	core::csr_manager::ReleaseObject();

	gsm_manager::ReleaseObject();

	s = GetStringFromAppResource(IDS_STRING_DONE); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	SLEEP;

	//dlg->DestroyWindow();

	CDialogEx::OnCancel();
}


afx_msg LRESULT CAlarmCenterDlg::OnMsgWmExitProcess(WPARAM, LPARAM)
{
	ExitAlarmCenter();
	return 0;
}


afx_msg LRESULT CAlarmCenterDlg::OnReminderTimeUp(WPARAM wParam, LPARAM lParam)
{
	std::lock_guard<std::mutex> lock(m_lock_4_timeup);
	m_reminder_timeup_list.insert(std::make_pair(wParam, lParam));
	return 0;
}


afx_msg LRESULT CAlarmCenterDlg::OnServiceTimeUp(WPARAM wParam, LPARAM lParam)
{
	std::lock_guard<std::mutex> lock(m_lock_4_timeup);
	m_service_timeup_list.insert(std::make_pair(wParam, lParam));
	return 0;
}


afx_msg LRESULT CAlarmCenterDlg::OnMsgDisarmPasswdWrong(WPARAM wParam, LPARAM /*lParam*/)
{
	std::lock_guard<std::mutex> lock(m_lock_4_passwd_wrong_ademco_id_list);
	m_disarm_passwd_wrong_ademco_id_list.insert(int(wParam));
	return 0;
}
