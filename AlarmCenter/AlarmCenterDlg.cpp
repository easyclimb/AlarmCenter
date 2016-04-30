
// AlarmCenterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AlarmCenterDlg.h"
#include "afxdialogex.h"
#include "AlarmMachineContainer.h"
#include "AlarmMachineManager.h"
#include "AlarmMachine.h"
#include "AppResource.h"
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
#include "Sms.h"
#include "ExportHrProcessDlg.h"
#include "BaiduMapViewerDlg.h"
#include "VideoPlayerDlg.h"
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


class CAlarmCenterDlg::CurUserChangedObserver : public dp::observer<core::CUserInfoPtr>
{
public:
	explicit CurUserChangedObserver(CAlarmCenterDlg* dlg) : _dlg(dlg) {}
	virtual void on_update(const core::CUserInfoPtr& ptr) {
		if (_dlg) {
			_dlg->PostMessage(WM_CURUSERCHANGED, (WPARAM)(ptr->get_user_id()));
		}
	}
private:
	CAlarmCenterDlg* _dlg;
};

class CAlarmCenterDlg::NewRecordObserver : public dp::observer<core::HistoryRecordPtr>
{
public:
	explicit NewRecordObserver(CAlarmCenterDlg* dlg) : _dlg(dlg) {}
	virtual void on_update(const core::HistoryRecordPtr& ptr) {
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
	ON_MESSAGE(WM_NETWORKSTARTUPOK, &CAlarmCenterDlg::OnTransmitserver)
	ON_MESSAGE(WM_CURUSERCHANGED, &CAlarmCenterDlg::OnCuruserchangedResult)
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
	ON_MESSAGE(WM_NEEDQUERYSUBMACHINE, &CAlarmCenterDlg::OnNeedQuerySubMachine)
	ON_MESSAGE(WM_NEED_TO_EXPORT_HR, &CAlarmCenterDlg::OnNeedToExportHr)
	ON_WM_HOTKEY()
	ON_NOTIFY(NM_RCLICK, IDC_TREE_MACHINE_GROUP, &CAlarmCenterDlg::OnNMRClickTreeMachineGroup)
	ON_MESSAGE(WM_EXIT_ALARM_CENTER, &CAlarmCenterDlg::OnWmExitProcess)
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
	core::CUserManager* userMgr = core::CUserManager::GetInstance();
	core::CUserInfoPtr user = userMgr->GetCurUserInfo();
	OnCuruserchangedResult((WPARAM)user->get_user_id(), 0);
	m_cur_user_changed_observer = std::make_shared<CurUserChangedObserver>(this);
	userMgr->register_observer(m_cur_user_changed_observer);
	JLOG(L"REGISTER USERINFO ok\n");


	CString welcom;
	welcom = GetStringFromAppResource(IDS_STRING_WELCOM);
	m_new_record_observer = std::make_shared<NewRecordObserver>(this);
	core::CHistoryRecord* hr = core::CHistoryRecord::GetInstance();
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
	CGroupManager* mgr = CGroupManager::GetInstance();
	CGroupInfoPtr rootGroup = mgr->GetRootGroupInfo();
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
		CGroupInfoPtr curselGroupInfo = nullptr;
		if (rootGroup->get_child_group_count() > 0) {
			m_curselTreeItem = nullptr;
			m_curselTreeItemData = 0;
			CGroupInfoPtr firstChildGroup = rootGroup->GetFirstChildGroupInfo();
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
		m_wndContainer->ShowMachinesOfGroup(curselGroupInfo);

		m_treeGroup.Expand(hRootGroup, TVE_EXPAND);
	}
}


void CAlarmCenterDlg::TraverseGroup(HTREEITEM hItemGroup, core::CGroupInfoPtr group)
{
	using namespace core;
	CString txt;
	CGroupInfoList groupList;
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
	auto parent_group = core::CGroupManager::GetInstance()->GetGroupInfo(m_treeGroup.GetItemData(hItemParent));
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
		auto group = core::CGroupManager::GetInstance()->GetGroupInfo(m_treeGroup.GetItemData(hItem));
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
	//core::CAlarmMachineManager::GetInstance()->LoadCameraInfoFromDB();
	video::ezviz::CVideoDeviceInfoEzvizList devList;
	video::CVideoManager::GetInstance()->GetVideoDeviceEzvizWithDetectorList(devList);
	if (!devList.empty()) {
		for (auto dev : devList) {
			core::CAlarmMachineManager::GetInstance()->ResolveCameraInfo(dev->get_id(), dev->get_userInfo()->get_productorInfo().get_productor());
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
		KillTimer(detail::cTimerIdRefreshGroupTree);
		if (m_times4GroupOnlineCntChanged > 0) {
			TraverseGroupTree(m_treeGroup.GetRootItem());
			m_times4GroupOnlineCntChanged = 0;
			//m_treeGroup.Invalidate();
			CGroupManager* mgr = CGroupManager::GetInstance();
			CGroupInfoPtr rootGroup = mgr->GetRootGroupInfo();
			if (rootGroup->get_alarming_descendant_machine_count() == 0) {
				CSoundPlayer::GetInstance()->Stop();
			}
		}
		SetTimer(detail::cTimerIdRefreshGroupTree, 500, nullptr);
	} else if (detail::cTimerIdHandleMachineAlarmOrDisalarm == nIDEvent) {
		KillTimer(detail::cTimerIdHandleMachineAlarmOrDisalarm);
		HandleMachineAlarm();
		SetTimer(detail::cTimerIdHandleMachineAlarmOrDisalarm, 200, nullptr);
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CAlarmCenterDlg::OnDestroy()
{
	AUTO_LOG_FUNCTION;

	CDialogEx::OnDestroy();
}


afx_msg LRESULT CAlarmCenterDlg::OnTransmitserver(WPARAM wParam, LPARAM lParam)
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
	auto hr = core::CHistoryRecord::GetInstance();
	if (online) {
		status = GetStringFromAppResource(IDS_STRING_TRANSMIT_CONN);
		main_client ? m_sTransmitServerStatus.SetWindowTextW(status) : m_sTransmitServerBkStatus.SetWindowTextW(status);
		txt = GetStringFromAppResource(main_client ? IDS_STRING_CONN_TO_SERVER_OK : IDS_STRING_CONN_TO_SERVERBK_OK);
		hr->InsertRecord(-1, -1, txt, time(nullptr), core::RECORD_LEVEL_SYSTEM);
	} else {
		status = GetStringFromAppResource(IDS_STRING_TRANSMIT_DISCONN);
		txt = GetStringFromAppResource(main_client ? IDS_STRING_LOST_SERVER_CONN : IDS_STRING_LOST_SERVERBK_CONN);
		hr->InsertRecord(-1, -1, txt, time(nullptr), core::RECORD_LEVEL_SYSTEM);
		txt = GetStringFromAppResource(IDS_STRING_WILL_CONN_IN_TIME);
		hr->InsertRecord(-1, -1, txt, time(nullptr), core::RECORD_LEVEL_SYSTEM);
	}
	//}
	main_client ? m_sTransmitServerStatus.SetWindowTextW(status) : m_sTransmitServerBkStatus.SetWindowTextW(status);
	

	return 0;
}


afx_msg LRESULT CAlarmCenterDlg::OnCuruserchangedResult(WPARAM wParam, LPARAM /*lParam*/)
{
	auto user = core::CUserManager::GetInstance()->GetUserInfo(wParam); assert(user);

	CString user_id;
	user_id.Format(L"%d", user->get_user_id());
	m_cur_user_id.SetWindowTextW(user_id);
	m_cur_user_name.SetWindowTextW(user->get_user_name());
	m_cur_user_phone.SetWindowTextW(user->get_user_phone());
	core::UserPriority user_priority = user->get_user_priority();
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
	m_wndContainer->ShowMachinesOfGroup(nullptr);
	CMachineManagerDlg dlg(this);
	dlg.DoModal();
	
	m_treeGroup.DeleteAllItems();

	using namespace core;
	CGroupManager* mgr = CGroupManager::GetInstance();
	CGroupInfoPtr rootGroup = mgr->GetRootGroupInfo();
	if (rootGroup) {
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
		// 优先选择上次选中的分组项
		bool ok = false;
		CGroupInfoPtr curselGroupInfo = nullptr;
		if (m_curselTreeItemData != 0) {
			ok = SelectGroupItemOfTree(m_curselTreeItemData);
			if (ok) {
				curselGroupInfo = mgr->GetGroupInfo(m_curselTreeItemData);
			}
		}

		// 不行就选择第一个子分组项
		if (!ok) {
			if (rootGroup->get_child_group_count() > 0) {
				m_curselTreeItem = nullptr;
				m_curselTreeItemData = 0;
				CGroupInfoPtr firstChildGroup = rootGroup->GetFirstChildGroupInfo();
				ok = SelectGroupItemOfTree((DWORD)firstChildGroup->get_id());
				if (ok)
					curselGroupInfo = mgr->GetGroupInfo(m_curselTreeItemData);
			}
		}

		// 实在不行，没辙了，选择根项吧。。。
		if (!ok) {
			curselGroupInfo = rootGroup;
		}

		TCITEM tcItem;
		tcItem.mask = TCIF_TEXT;
		CString name = curselGroupInfo->get_formatted_group_name();
		tcItem.pszText = name.LockBuffer();
		m_tab.SetItem(detail::TAB_NDX_NORMAL, &tcItem);
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
#define SLEEP {Sleep(100);}
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
	CGroupInfoPtr group = core::CGroupManager::GetInstance()->GetGroupInfo(data);
	if (group) {
		// change tab item text
		TCITEM tcItem;
		tcItem.mask = TCIF_TEXT;
		CString name = group->get_formatted_group_name();
		tcItem.pszText = name.LockBuffer();
		m_tab.SetItem(detail::TAB_NDX_NORMAL, &tcItem);
		name.UnlockBuffer();
		// load machine of this gruop
		m_wndContainer->ShowMachinesOfGroup(group);
		m_tab.Invalidate(0);
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
	CGroupInfoPtr group = core::CGroupManager::GetInstance()->GetGroupInfo(data);
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
			auto way = CGroupManager::GetInstance()->get_cur_sort_machine_way();
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

		DWORD ret = pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
										  pt.x, pt.y, this);

		auto mgr = core::CAlarmMachineManager::GetInstance();

		bool b_sort = false;
		auto way = sort_by_ademco_id;

		switch (ret) {
		case ID_SUB_32796: {// clear alarm info
			m_wndContainerAlarming->ClearButtonList();
			group->ClearAlarmMsgOfDescendantAlarmingMachine();

			core::CAlarmMachineList list;
			for (int i = 0; i < MAX_MACHINE; i++) {
				auto machine = mgr->GetMachine(i);
				if (machine && machine->get_alarming()) {
					list.push_back(machine);
				}
			}
			if (list.empty()) {
				core::CSoundPlayer::GetInstance()->Stop();
			}
			m_wndContainerAlarming->Reset(list);
		}
			break;

		case ID_SUB_32797: { // arm
			core::CAlarmMachineList list;
			group->GetDescendantMachines(list);
			for (auto machine : list) {
				mgr->RemoteControlAlarmMachine(machine, EVENT_ARM, 0, 0, nullptr, nullptr, ES_UNKNOWN, this);
			}
		}
			break;

			// sort machine
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

		default:
			break;
		}

		if (b_sort) {
			CGroupManager::GetInstance()->set_cur_sort_machine_way(way);
			if (hItem != m_curselTreeItem) {
				group = CGroupManager::GetInstance()->GetGroupInfo(m_curselTreeItemData);
			}
			if (group) {
				m_wndContainer->ShowMachinesOfGroup(group);
			}
		}
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
	auto mgr = CGroupManager::GetInstance();

	while (!m_machineAlarmOrDisalarmList.empty()) {
		auto ad = m_machineAlarmOrDisalarmList.front();
		m_machineAlarmOrDisalarmList.pop_front();
		if (ad->alarm) {
			CGroupInfoPtr group = mgr->GetGroupInfo(ad->machine->get_group_id());
			if (group) {
				// select the group tree item if its not selected
				DWORD data = m_treeGroup.GetItemData(m_curselTreeItem);
				if (data != (DWORD)group->get_id()) {
					// if cur show group is ancestor, need not to show
					bool bCurShowGroupIsAncenstor = false;
					CGroupInfoPtr parent_group = group->get_parent_group();
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

			m_wndContainerAlarming->InsertMachine(ad->machine, true);

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
	core::CSoundPlayer::GetInstance()->Stop();
	CString srecord, suser, sfm, sop, fmMachine, fmSubmachine;
	suser = GetStringFromAppResource(IDS_STRING_USER);
	sfm = GetStringFromAppResource(IDS_STRING_LOCAL_OP);
	fmMachine = GetStringFromAppResource(IDS_STRING_MACHINE);
	fmSubmachine = GetStringFromAppResource(IDS_STRING_SUBMACHINE);
	sop = GetStringFromAppResource(IDS_STRING_MUTE_ONCE);
	CUserInfoPtr user = CUserManager::GetInstance()->GetCurUserInfo();
	srecord.Format(L"%s(ID:%d,%s)%s:%s", suser,
				   user->get_user_id(), user->get_user_name(),
				   sfm, sop);

	CHistoryRecord::GetInstance()->InsertRecord(-1, -1, srecord, time(nullptr),
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


afx_msg LRESULT CAlarmCenterDlg::OnNeedQuerySubMachine(WPARAM wParam, LPARAM lParam)
{
	auto subMachineList = std::unique_ptr<CAlarmMachineList>(reinterpret_cast<CAlarmMachineList*>(wParam));
	size_t size = static_cast<size_t>(lParam); VERIFY(subMachineList->size() == size);
	CAutoQueryDisconnectSubmachineDlg autoDlg(this);
	std::copy(subMachineList->begin(), subMachineList->end(), 
			  std::back_inserter(autoDlg.m_subMachineList));
	autoDlg.DoModal();
	return 0;
}


afx_msg LRESULT CAlarmCenterDlg::OnNeedToExportHr(WPARAM wParam, LPARAM /*lParam*/)
{
	int curRecord = static_cast<int>(wParam);
	CExportHrProcessDlg dlg(this);
	dlg.m_nTotalCount = curRecord;
	dlg.DoModal();

	CString s, fm;
	fm = GetStringFromAppResource(IDS_STRING_SYSTEM_EXPORT_HR);
	s.Format(fm, dlg.m_excelPath);
	CHistoryRecord* hr = CHistoryRecord::GetInstance();
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
	UnregisterHotKey(GetSafeHwnd(), HOTKEY_MUTE);
	//core::CGroupManager::GetInstance()->GetRootGroupInfo()->UnRegisterObserver(this);
	m_observer.reset();
	m_new_record_observer.reset();
	//core::CHistoryRecord::GetInstance()->UnRegisterObserver(this);
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
	core::CAlarmMachineManager::ReleaseObject();
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
	core::CHistoryRecord* hr = core::CHistoryRecord::GetInstance();
	hr->InsertRecord(-1, -1, goodbye, time(nullptr), core::RECORD_LEVEL_SYSTEM);
	//hr->UnRegisterObserver(this);
	hr->ReleaseObject();
	SLEEP;

	// user manager
	s = GetStringFromAppResource(IDS_STRING_DESTROY_USER); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	core::CUserManager::ReleaseObject();
	SLEEP;

	// ok
	s = GetStringFromAppResource(IDS_STRING_DESTROY_SND); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	core::CSoundPlayer::GetInstance()->Stop();
	core::CSoundPlayer::ReleaseObject();
	SLEEP;

	//web::CBaiduService::ReleaseObject();
	core::CCsrInfo::ReleaseObject();

	CGsm::ReleaseObject();
	CSms::ReleaseObject();

	//video::ezviz::CSdkMgrEzviz::ReleaseObject();
	//video::ezviz::CPrivateCloudConnector::ReleaseObject();

	s = GetStringFromAppResource(IDS_STRING_DONE); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	SLEEP;

	dlg->DestroyWindow();

	CDialogEx::OnCancel();
}


afx_msg LRESULT CAlarmCenterDlg::OnWmExitProcess(WPARAM, LPARAM)
{
	ExitAlarmCenter();
	return 0;
}
