
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
#include "QrcodeViewerDlg.h"
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
#include "PickMachineCoordinateDlg.h"
#include "VideoPlayerDlg.h"

#include <algorithm>
#include <iterator>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define HOTKEY_MUTE 11

using namespace core;

static void __stdcall OnCurUserChanged(void* udata, const core::CUserInfo* user)
{
	AUTO_LOG_FUNCTION;
	CAlarmCenterDlg* dlg = reinterpret_cast<CAlarmCenterDlg*>(udata); assert(dlg);
	if (dlg && IsWindow(dlg->m_hWnd))
		dlg->SendMessage(WM_CURUSERCHANGED, (WPARAM)(user));
}

static void __stdcall OnNewRecord(void* udata, const core::HistoryRecord* record)
{
	AUTO_LOG_FUNCTION;
	CAlarmCenterDlg* dlg = reinterpret_cast<CAlarmCenterDlg*>(udata); assert(dlg);
	//dlg->SendMessage(WM_NEWRECORD, (WPARAM)(record));
	dlg->m_lock4RecordList.Lock();
	dlg->m_recordList.AddTail(record->record);
	dlg->m_lock4RecordList.UnLock();
}


static void _stdcall OnGroupOnlineMachineCountChanged(void* udata, int)
{
	CAlarmCenterDlg* dlg = reinterpret_cast<CAlarmCenterDlg*>(udata); assert(dlg);
	dlg->m_times4GroupOnlineCntChanged++;
}

//static void __stdcall OnAdemcoEvent(void* udata, const core::AdemcoEvent* ademcoEvent)
//{
//	AUTO_LOG_FUNCTION;
//	CAlarmCenterDlg* dlg = reinterpret_cast<CAlarmCenterDlg*>(udata); assert(dlg);
//	if (dlg && IsWindow(dlg->m_hWnd))
//		dlg->SendMessage(WM_ADEMCOEVENT, (WPARAM)ademcoEvent);
//}



static const int cTimerIdTime = 1;
static const int cTimerIdHistory = 2;
static const int cTimerIdRefreshGroupTree = 3;

static const int TAB_NDX_NORMAL = 0;
static const int TAB_NDX_ALARMING = 1;

// CAboutDlg dialog used for App About

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
	, m_qrcodeViewDlg(nullptr)
	//, m_progressDlg(nullptr)
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
	ON_MESSAGE(WM_NEWRECORD, &CAlarmCenterDlg::OnNewrecordResult)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_MACHINE_GROUP, &CAlarmCenterDlg::OnTvnSelchangedTreeMachineGroup)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_CONTAINER, &CAlarmCenterDlg::OnTcnSelchangeTabContainer)
	ON_MESSAGE(WM_ADEMCOEVENT, &CAlarmCenterDlg::OnAdemcoevent)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_MACHINE_GROUP, &CAlarmCenterDlg::OnNMDblclkTreeMachineGroup)
	ON_BN_CLICKED(IDC_BUTTON_MACHINEMGR, &CAlarmCenterDlg::OnBnClickedButtonMachinemgr)
	ON_BN_CLICKED(IDC_BUTTON_SEE_MORE_HR, &CAlarmCenterDlg::OnBnClickedButtonSeeMoreHr)
	ON_BN_CLICKED(IDC_BUTTON_MUTE, &CAlarmCenterDlg::OnBnClickedButtonMute)
	ON_MESSAGE(WM_NEEDQUERYSUBMACHINE, &CAlarmCenterDlg::OnNeedQuerySubMachine)
	ON_MESSAGE(WM_NEED_TO_EXPORT_HR, &CAlarmCenterDlg::OnNeedToExportHr)
	ON_WM_HOTKEY()
	ON_NOTIFY(NM_RCLICK, IDC_TREE_MACHINE_GROUP, &CAlarmCenterDlg::OnNMRClickTreeMachineGroup)
END_MESSAGE_MAP()


// CAlarmCenterDlg message handlers

BOOL CAlarmCenterDlg::OnInitDialog()
{
	AUTO_LOG_FUNCTION;
	CDialogEx::OnInitDialog();

	CAppResource::GetInstance();

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr) {
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
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
	const core::CUserInfo* user = userMgr->GetCurUserInfo();
	OnCuruserchangedResult((WPARAM)user, 0);
	userMgr->RegisterObserver(this, OnCurUserChanged);
	JLOG(L"REGISTER USERINFO ok\n");


	CString welcom;
	welcom.LoadStringW(IDS_STRING_WELCOM);
	core::CHistoryRecord* hr = core::CHistoryRecord::GetInstance();
	hr->RegisterObserver(this, OnNewRecord);
	hr->InsertRecord(-1, -1, welcom, time(nullptr), core::RECORD_LEVEL_SYSTEM);
	JLOG(L"INSERT WELCOM OK\n");

	//m_progressDlg = new CLoadFromDBProgressDlg();
	CLoadFromDBProgressDlg dlg;
	dlg.DoModal();
	JLOG(L"CLoadFromDBProgressDlg DoModal OK\n");
	//m_progressDlg->Create(IDD_DIALOG_PROGRESS, this);

	CAlarmCenterApp* app = (CAlarmCenterApp*)AfxGetApp();
	CString sPort;
	sPort.Format(L"%d", app->m_local_port);
	m_sLocalPort.SetWindowTextW(sPort);

	//m_cur_user_id.EnableWindow(0);
	//m_cur_user_name.EnableWindow(0);
	//m_cur_user_phone.EnableWindow(0);

	SetTimer(cTimerIdTime, 1000, nullptr);
	SetTimer(cTimerIdHistory, 1000, nullptr); 
	SetTimer(cTimerIdRefreshGroupTree, 1000, nullptr);

//#if !defined(DEBUG) && !defined(_DEBUG)
	//SetWindowPos(&CWnd::wndTopMost, 0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN), SWP_SHOWWINDOW);
//#else
	MoveWindow(0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN), TRUE);
//#endif

	InitDisplay();
	InitAlarmMacines();
	if (net::CNetworkConnector::GetInstance()->StartNetwork(app->m_local_port,
		app->m_transmit_server_ip,
		app->m_transmit_server_port)) {
		CString s; s.Format(L"%d", app->m_local_port);
		m_sLocalPort.SetWindowTextW(s);
	}

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


void CAlarmCenterDlg::InitAlarmMacines()
{
	AUTO_LOG_FUNCTION;
	using namespace core;
	CGroupManager* mgr = CGroupManager::GetInstance();
	CGroupInfo* rootGroup = mgr->GetRootGroupInfo();
	rootGroup->RegisterObserver(this, OnGroupOnlineMachineCountChanged);
	if (rootGroup) {
		CString txt;
		txt.Format(L"%s[%d/%d]", rootGroup->get_name(), 
				   rootGroup->get_online_descendant_machine_count(),
				   rootGroup->get_descendant_machine_count());
		HTREEITEM hRoot = m_treeGroup.GetRootItem();
		HTREEITEM hRootGroup = m_treeGroup.InsertItem(txt, hRoot);
		m_treeGroup.SetItemData(hRootGroup, (DWORD_PTR)rootGroup);

		TraverseGroup(hRootGroup, rootGroup);

		bool ok = false;
		CGroupInfo* curselGroupInfo = nullptr;
		if (rootGroup->get_child_group_count() > 0) {
			m_curselTreeItem = nullptr;
			m_curselTreeItemData = 0;
			CGroupInfo* firstChildGroup = rootGroup->GetFirstChildGroupInfo();
			ok = SelectGroupItemOfTree((DWORD)firstChildGroup);
			if (ok)
				curselGroupInfo = firstChildGroup;
		} 

		if (!ok) {
			m_curselTreeItem = hRootGroup;
			m_curselTreeItemData = (DWORD)rootGroup;
			curselGroupInfo = rootGroup;
		}

		TCITEM tcItem;
		tcItem.mask = TCIF_TEXT;
		CString name = curselGroupInfo->get_name();
		tcItem.pszText = name.LockBuffer();
		m_tab.SetItem(TAB_NDX_NORMAL, &tcItem);
		name.UnlockBuffer();
		m_wndContainer->ShowMachinesOfGroup(curselGroupInfo);

		m_treeGroup.Expand(hRootGroup, TVE_EXPAND);
	}
}


void CAlarmCenterDlg::TraverseGroup(HTREEITEM hItemGroup, core::CGroupInfo* group)
{
	using namespace core;
	CString txt;
	CGroupInfoList groupList;
	group->GetChildGroups(groupList);

	for (auto child_group : groupList) {
		txt.Format(L"%s[%d/%d]", child_group->get_name(),
				   child_group->get_online_descendant_machine_count(), 
				   child_group->get_descendant_machine_count());
		HTREEITEM hChildItem = m_treeGroup.InsertItem(txt, hItemGroup);
		m_treeGroup.SetItemData(hChildItem, (DWORD_PTR)child_group);
		TraverseGroup(hChildItem, child_group);
	}
}


void CAlarmCenterDlg::TraverseGroupTree(HTREEITEM hItemParent)
{
	const core::CGroupInfo* parent_group = reinterpret_cast<const core::CGroupInfo*>(m_treeGroup.GetItemData(hItemParent));
	CString txt = L"";
	txt.Format(L"%s[%d/%d]", parent_group->get_name(),
			   parent_group->get_online_descendant_machine_count(),
			   parent_group->get_descendant_machine_count());
	m_treeGroup.SetItemText(hItemParent, txt);
	HTREEITEM hItem = m_treeGroup.GetChildItem(hItemParent);
	while (hItem) {
		const core::CGroupInfo* group = reinterpret_cast<const core::CGroupInfo*>( m_treeGroup.GetItemData(hItem));
		if (group) {
			txt.Format(L"%s[%d/%d]", group->get_name(),
					   group->get_online_descendant_machine_count(),
					   group->get_descendant_machine_count());
			m_treeGroup.SetItemText(hItem, txt);
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

	m_wndContainer = new CAlarmMachineContainerDlg(&m_tab);
	m_wndContainer->Create(IDD_DIALOG_CONTAINER, &m_tab);
	CString txt;
	txt.LoadStringW(IDS_STRING_GROUP_ROOT);
	m_tab.InsertItem(TAB_NDX_NORMAL, txt);

	m_wndContainerAlarming = new CAlarmMachineContainerDlg(&m_tab);
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

	m_tab.SetCurSel(TAB_NDX_NORMAL);

	g_baiduMapDlg = new CPickMachineCoordinateDlg();
	g_baiduMapDlg->Create(IDD_DIALOG_PICK_MACHINE_COOR, this);
	g_baiduMapDlg->ShowWindow(SW_SHOW);

	g_videoPlayerDlg = new CVideoPlayerDlg();
	g_videoPlayerDlg->Create(IDD_DIALOG_VIDEO_PLAYER, this);
	g_videoPlayerDlg->ShowWindow(SW_SHOW);

	m_qrcodeViewDlg = new CQrcodeViewerDlg(this);
	m_qrcodeViewDlg->Create(IDD_DIALOG_CSR_ACCT, this);
}


void CAlarmCenterDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (cTimerIdTime == nIDEvent) {
		SYSTEMTIME st = { 0 };
		::GetLocalTime(&st);
		wchar_t now[1024] = { 0 };
		wsprintfW(now, L"%04d-%02d-%02d %02d:%02d:%02d", st.wYear,
				  st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		m_staticSysTime.SetWindowTextW(now);
	} else if (cTimerIdHistory == nIDEvent) {
		if (m_lock4RecordList.TryLock()) {
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
			m_lock4RecordList.UnLock();
		}
	} else if (cTimerIdRefreshGroupTree == nIDEvent) {
		if (m_times4GroupOnlineCntChanged > 0) {
			TraverseGroupTree(m_treeGroup.GetRootItem());
			m_times4GroupOnlineCntChanged = 0;
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CAlarmCenterDlg::OnDestroy()
{
	AUTO_LOG_FUNCTION;

	CDialogEx::OnDestroy();
}


afx_msg LRESULT CAlarmCenterDlg::OnTransmitserver(WPARAM wParam, LPARAM /*lParam*/)
{
	BOOL online = static_cast<BOOL>(wParam);
	CString status;
	if (online) {
		status.LoadStringW(IDS_STRING_TRANSMIT_CONN);
		m_sTransmitServerStatus.SetWindowTextW(status);
	} else {
		status.LoadStringW(IDS_STRING_TRANSMIT_DISCONN);
		m_sTransmitServerStatus.SetWindowTextW(status);
	}
	return 0;
}


afx_msg LRESULT CAlarmCenterDlg::OnCuruserchangedResult(WPARAM wParam, LPARAM /*lParam*/)
{
	core::CUserInfo* user = reinterpret_cast<core::CUserInfo*>(wParam); assert(user);

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
			sPriority.LoadStringW(IDS_STRING_USER_SUPER);
			m_btnUserMgr.EnableWindow(1);
			m_btnMachineMgr.EnableWindow(1);
			break;
		case core::UP_ADMIN:
			sPriority.LoadStringW(IDS_STRING_USER_ADMIN);
			m_btnMachineMgr.EnableWindow(1);
			break;
		case core::UP_OPERATOR:
		default:
			sPriority.LoadStringW(IDS_STRING_USER_OPERATOR);
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
	//CQrcodeViewerDlg dlg(this);
	//dlg.DoModal();
	m_qrcodeViewDlg->ShowWindow(SW_SHOW);
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
	CGroupInfo* rootGroup = mgr->GetRootGroupInfo();
	if (rootGroup) {
		CString txt;
		txt.Format(L"%s[%d/%d]", rootGroup->get_name(),
				   rootGroup->get_online_descendant_machine_count(),
				   rootGroup->get_descendant_machine_count());
		HTREEITEM hRoot = m_treeGroup.GetRootItem();
		HTREEITEM hRootGroup = m_treeGroup.InsertItem(txt, hRoot);
		m_treeGroup.SetItemData(hRootGroup, (DWORD_PTR)rootGroup);

		TraverseGroup(hRootGroup, rootGroup);

		//m_curselTreeItem = hRootGroup;
		//m_curselTreeItemData = (DWORD)rootGroup;
		// 优先选择上次选中的分组项
		bool ok = false;
		CGroupInfo* curselGroupInfo = nullptr;
		if (m_curselTreeItemData != 0) {
			ok = SelectGroupItemOfTree(m_curselTreeItemData);
			if (ok) {
				curselGroupInfo = reinterpret_cast<CGroupInfo*>(m_curselTreeItemData);
			}
		}

		// 不行就选择第一个子分组项
		if (!ok) {
			if (rootGroup->get_child_group_count() > 0) {
				m_curselTreeItem = nullptr;
				m_curselTreeItemData = 0;
				CGroupInfo* firstChildGroup = rootGroup->GetFirstChildGroupInfo();
				ok = SelectGroupItemOfTree((DWORD)firstChildGroup);
				if (ok)
					curselGroupInfo = reinterpret_cast<CGroupInfo*>(m_curselTreeItemData);
			}
		}

		// 实在不行，没辙了，选择根项吧。。。
		if (!ok) {
			curselGroupInfo = rootGroup;
		}

		TCITEM tcItem;
		tcItem.mask = TCIF_TEXT;
		CString name = curselGroupInfo->get_name();
		tcItem.pszText = name.LockBuffer();
		m_tab.SetItem(TAB_NDX_NORMAL, &tcItem);
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
	confirm.LoadStringW(IDS_STRING_CONFIRM_QUIT);
	int ret = MessageBox(confirm, L"", MB_YESNO | MB_ICONQUESTION);
	if (ret == IDNO)
		return;
#endif

#ifdef _DEBUG
#define SLEEP {Sleep(100);}
#else
#define SLEEP
#endif

	UnregisterHotKey(GetSafeHwnd(), HOTKEY_MUTE);
	core::CGroupManager::GetInstance()->GetRootGroupInfo()->UnRegisterObserver(this);
	core::CHistoryRecord::GetInstance()->UnRegisterObserver(this);
	ShowWindow(SW_HIDE);
	CDestroyProgressDlg* dlg = new CDestroyProgressDlg();
	dlg->Create(IDD_DIALOG_DESTROY_PROGRESS, this);
	dlg->ShowWindow(SW_SHOW);
	dlg->CenterWindow(this);
	dlg->UpdateWindow();
	SLEEP;

	CString s; int ndx = 0;
	s.LoadStringW(IDS_STRING_DESTROY_START);
	JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	SLEEP;
	
	// timer
	s.LoadStringW(IDS_STRING_DESTROY_TIMER); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	KillTimer(1);
	SLEEP;

	// alarmmachine container
	s.LoadStringW(IDS_STRING_DESTROY_CONTAINER); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	SAFEDELETEDLG(m_wndContainer);
	SLEEP;

	// alarming alarmmachine container
	s.LoadStringW(IDS_STRING_DESTROY_ALARMING); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	SAFEDELETEDLG(m_wndContainerAlarming);
	SLEEP;

	// qrcode viewer
	s.LoadStringW(IDS_STRING_DESTROY_QR); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	SAFEDELETEDLG(m_qrcodeViewDlg);
	//SAFEDELETEDLG(m_progressDlg);
	SLEEP;

	// video
	s.LoadStringW(IDS_STRING_DESTROY_VIDEO); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	SAFEDELETEDLG(g_baiduMapDlg);
	SAFEDELETEDLG(g_videoPlayerDlg);

	// stop network
	s.LoadStringW(IDS_STRING_DESTROY_NET); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	net::CNetworkConnector::GetInstance()->StopNetWork();
	SLEEP;

	// destroy network
	s.LoadStringW(IDS_STRING_DESTROY_NETWORK); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	net::CNetworkConnector::ReleaseObject();
	SLEEP;

	// machine manager
	s.LoadStringW(IDS_STRING_DESTROY_MGR); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	core::CAlarmMachineManager::ReleaseObject();
	SLEEP;

	// config helper
	s.LoadStringW(IDS_STRING_DESTROY_CFG); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	util::CConfigHelper::ReleaseObject();
	SLEEP;

	// app res
	s.LoadStringW(IDS_STRING_DESTROY_RES); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	CAppResource::ReleaseObject();
	SLEEP;

	// hisroty record
	s.LoadStringW(IDS_STRING_DESTROY_HR); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	CString goodbye;
	goodbye.LoadStringW(IDS_STRING_GOODBYE);
	core::CHistoryRecord* hr = core::CHistoryRecord::GetInstance();
	hr->InsertRecord(-1, -1, goodbye, time(nullptr), core::RECORD_LEVEL_SYSTEM);
	//hr->UnRegisterObserver(this);
	hr->ReleaseObject();
	SLEEP;

	// user manager
	s.LoadStringW(IDS_STRING_DESTROY_USER); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	core::CUserManager::ReleaseObject();
	SLEEP;

	// ok
	s.LoadStringW(IDS_STRING_DESTROY_SND); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	core::CSoundPlayer::GetInstance()->Stop();
	core::CSoundPlayer::ReleaseObject();
	SLEEP;

	web::CBaiduService::ReleaseObject();
	core::CCsrInfo::ReleaseObject();
	
	CGsm::ReleaseObject();
	CSms::ReleaseObject();
	
	//video::ezviz::CSdkMgrEzviz::ReleaseObject();
	//video::ezviz::CPrivateCloudConnector::ReleaseObject();

	s.LoadStringW(IDS_STRING_DONE); JLOG(s);
	ndx = dlg->m_list.InsertString(ndx, s);
	dlg->m_list.SetCurSel(ndx++);
	dlg->UpdateWindow();
	SLEEP;

	SAFEDELETEDLG(dlg);
	CDialogEx::OnCancel();
}


afx_msg LRESULT CAlarmCenterDlg::OnNewrecordResult(WPARAM wParam, LPARAM /*lParam*/)
{
	AUTO_LOG_FUNCTION;
	core::HistoryRecord* record = reinterpret_cast<core::HistoryRecord*>(wParam);
	assert(record);
	if (RECORD_LEVEL_CLEARHR == record->level) {
		m_listHistory.ResetContent();
		return 0;
	}

	m_listHistory.SetRedraw(0);
	if (m_listHistory.GetCount() > m_maxHistory2Show) {
		m_listHistory.DeleteString(0);
	}
	m_listHistory.InsertString(-1, record->record);
	m_listHistory.SetRedraw();
	return 0;
}


void CAlarmCenterDlg::OnTvnSelchangedTreeMachineGroup(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	using namespace core;
	HTREEITEM hItem = m_treeGroup.GetSelectedItem();
	if (m_curselTreeItem == hItem) { return; } 
	m_curselTreeItem = hItem;
	DWORD data = m_treeGroup.GetItemData(hItem);
	m_curselTreeItemData = data;
	CGroupInfo* group = reinterpret_cast<CGroupInfo*>(data);
	if (group) {
		// change tab item text
		TCITEM tcItem;
		tcItem.mask = TCIF_TEXT;
		CString name = group->get_name();
		tcItem.pszText = name.LockBuffer();
		m_tab.SetItem(TAB_NDX_NORMAL, &tcItem);
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
	if (hItem == NULL) {
		return;
	} 
	m_treeGroup.ClientToScreen(&pt);
	DWORD data = m_treeGroup.GetItemData(hItem);
	CGroupInfo* group = reinterpret_cast<CGroupInfo*>(data);
	if (group) {
		CString txt; txt.LoadStringW(IDS_STRING_CLR_ALM_MSG);
		CMenu menu;
		menu.CreatePopupMenu();
		menu.AppendMenuW(MF_STRING, 1, txt);
		DWORD ret = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
										pt.x, pt.y, this);
		if (1 == ret) {
			group->ClearAlarmMsgOfDescendantAlarmingMachine();
		}
	}
}


void CAlarmCenterDlg::OnTcnSelchangeTabContainer(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	int ndx = m_tab.GetCurSel();
	if (TAB_NDX_NORMAL == ndx) {
		m_wndContainer->ShowWindow(SW_SHOW);
		m_wndContainerAlarming->ShowWindow(SW_HIDE);
	} else if (TAB_NDX_ALARMING == ndx) {
		m_wndContainer->ShowWindow(SW_HIDE);
		m_wndContainerAlarming->ShowWindow(SW_SHOW);
	}
	*pResult = 0;
}

// wParam: CAlarmMachine*
// lParam: 0 for clrmsg, 1 for alarming
afx_msg LRESULT CAlarmCenterDlg::OnAdemcoevent(WPARAM wParam, LPARAM lParam)
{
	AUTO_LOG_FUNCTION;
	m_lock4AdemcoEvent.Lock();
	using namespace core;
	CAlarmMachine* machine = reinterpret_cast<CAlarmMachine*>(wParam);
	ASSERT(machine);
	BOOL bAlarming = (BOOL)lParam;

	if (bAlarming) {
		CGroupInfo* group = CGroupManager::GetInstance()->GetGroupInfo(machine->get_group_id());
		if (group) {
			// select the group tree item if its not selected
			DWORD data = m_treeGroup.GetItemData(m_curselTreeItem);
			if (data != (DWORD)group) {
				// if cur show group is ancestor, need not to show
				bool bCurShowGroupIsAncenstor = false;
				CGroupInfo* parent_group = group->get_parent_group();
				while (parent_group) {
					if ((DWORD)parent_group == data) {
						bCurShowGroupIsAncenstor = true;
						break;
					}
					parent_group = parent_group->get_parent_group();
				}

				if (!bCurShowGroupIsAncenstor) {
					SelectGroupItemOfTree(DWORD(group));
				}
				// m_wndContainer->ShowMachinesOfGroup(group);
			}
		}

		if (m_tab.GetItemCount() == 1) {
			CString txt;
			txt.LoadStringW(IDS_STRING_TAB_TEXT_ALARMING);
			m_tab.InsertItem(TAB_NDX_ALARMING, txt);

			//m_wndContainerAlarming->ShowWindow(SW_HIDE);
			//m_wndContainer->ShowWindow(SW_SHOW);
			//m_tab.SetCurSel(TAB_NDX_NORMAL);
			m_tab.Invalidate(0);
		}

		m_wndContainerAlarming->InsertMachine(machine);

		/*if (machine->get_auto_show_map_when_start_alarming()) {
			g_baiduMapDlg->ShowMap(machine);
		}*/

	} else {
		m_wndContainerAlarming->DeleteMachine(machine);
		if (m_wndContainerAlarming->GetMachineCount() == 0) {
			m_wndContainerAlarming->ShowWindow(SW_HIDE);
			//if (m_tab.GetCurSel() != TAB_NDX_NORMAL) {
			m_tab.DeleteItem(TAB_NDX_ALARMING);
			m_tab.SetCurSel(TAB_NDX_NORMAL);
			m_wndContainer->ShowWindow(SW_SHOW);
			m_tab.Invalidate(0);
			//}
		}
	}
	m_lock4AdemcoEvent.UnLock();
	return 0;
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
	CHistoryRecordDlg dlg; dlg.DoModal();
}


void CAlarmCenterDlg::OnBnClickedButtonMute()
{
	core::CSoundPlayer::GetInstance()->Stop();
	CString srecord, suser, sfm, sop, fmMachine, fmSubmachine;
	suser.LoadStringW(IDS_STRING_USER);
	sfm.LoadStringW(IDS_STRING_LOCAL_OP);
	fmMachine.LoadStringW(IDS_STRING_MACHINE);
	fmSubmachine.LoadStringW(IDS_STRING_SUBMACHINE);
	sop.LoadStringW(IDS_STRING_MUTE_ONCE);
	const CUserInfo* user = CUserManager::GetInstance()->GetCurUserInfo();
	srecord.Format(L"%s(ID:%d,%s)%s:%s", suser,
				   user->get_user_id(), user->get_user_name(),
				   sfm, sop);

	CHistoryRecord::GetInstance()->InsertRecord(-1, -1, srecord, time(nullptr),
												RECORD_LEVEL_USERCONTROL);
}



static BOOL GetProductVersion(CString& version)
{
	CString path = _T("");
	path.Format(_T("%s\\VersionNo.ini"), GetModuleFilePath());
	CFile file;
	if (file.Open(path, CFile::modeRead)) {
		size_t length = static_cast<size_t>(file.GetLength());
		char *buff = new char[length + 1];
		memset(buff, 0, length + 1);
		file.Read(buff, length);
		wchar_t *wbuff = AnsiToUtf16(buff);
		version.Format(L"AlarmCenter, Version %s", wbuff);
		file.Close();
		delete[] buff;
		delete[] wbuff;
		return TRUE;
	}
	return FALSE;
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString txt; 
	if (!GetProductVersion(txt)) {
		txt = _T("AlarmCenter, Version 1.0");
	}
	m_staticVersion.SetWindowTextW(txt);

	CString path;
	path.Format(L"%s\\ChangeLog.txt", GetModuleFilePath());
	CFile file;
	if (file.Open(path, CFile::modeRead)) {
		UINT len = static_cast<UINT>(file.GetLength());
		char *buf = new char[len + 1];
		memset(buf, 0, len + 1);
		file.Read(buf, len);
		buf[len] = 0;
		USES_CONVERSION;
		CString up = A2W(buf);
		m_edit.SetWindowTextW(up);
		delete[] buf;
	} else {
		CString e;
		e.Format(L"Open file '%s' failed!");
		m_edit.SetWindowTextW(e);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


afx_msg LRESULT CAlarmCenterDlg::OnNeedQuerySubMachine(WPARAM wParam, LPARAM lParam)
{
	CAlarmMachineList* subMachineList = reinterpret_cast<CAlarmMachineList*>(wParam);
	size_t size = static_cast<size_t>(lParam); VERIFY(subMachineList->size() == size);
	//CRemindQueryDlg dlg; if (IDOK != dlg.DoModal()) { delete subMachineList; return 0; }
	CAutoQueryDisconnectSubmachineDlg autoDlg;
	std::copy(subMachineList->begin(), subMachineList->end(), 
			  std::back_inserter(autoDlg.m_subMachineList));
	autoDlg.DoModal();
	delete subMachineList;
	return 0;
}


afx_msg LRESULT CAlarmCenterDlg::OnNeedToExportHr(WPARAM wParam, LPARAM /*lParam*/)
{
	int curRecord = static_cast<int>(wParam);
	//int maxRecord = static_cast<int>(lParam);
	/*CString s, fm;
	fm.LoadStringW(IDS_STRING_FM_REMIND_BK_HR);
	s.Format(fm, curRecord, maxRecord);
	MessageBox(s, L"", MB_ICONINFORMATION);*/
	CExportHrProcessDlg dlg;
	dlg.m_nTotalCount = curRecord;
	dlg.DoModal();

	CString s, fm;
	fm.LoadStringW(IDS_STRING_SYSTEM_EXPORT_HR);
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


