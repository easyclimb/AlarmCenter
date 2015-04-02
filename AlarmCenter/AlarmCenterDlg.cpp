
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

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static void __stdcall OnCurUserChanged(void* udata, const core::CUserInfo* user)
{
	CAlarmCenterDlg* dlg = reinterpret_cast<CAlarmCenterDlg*>(udata); assert(dlg);
	dlg->SendMessage(WM_CURUSERCHANGED, (WPARAM)(user));
}

static void __stdcall OnNewRecord(void* udata, const core::HistoryRecord* record)
{
	CAlarmCenterDlg* dlg = reinterpret_cast<CAlarmCenterDlg*>(udata); assert(dlg);
	dlg->SendMessage(WM_NEWRECORD, (WPARAM)(record));
}

static void __stdcall OnAdemcoEvent(void* udata, const core::AdemcoEvent* ademcoEvent)
{
	CAlarmCenterDlg* dlg = reinterpret_cast<CAlarmCenterDlg*>(udata); assert(dlg);
	dlg->SendMessage(WM_ADEMCOEVENT, (WPARAM)ademcoEvent);
}



static const int cTimerIdTime = 1;
static const int cTimerIdHistory = 2;
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
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CAlarmCenterDlg dialog



CAlarmCenterDlg::CAlarmCenterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAlarmCenterDlg::IDD, pParent)
	, m_wndContainer(NULL)
	, m_wndContainerAlarming(NULL)
	, m_hIconComputer(NULL)
	, m_hIconConnection(NULL)
	, m_hIconInternet(NULL)
	, m_qrcodeViewDlg(NULL)
	//, m_progressDlg(NULL)
	, m_curselTreeItem(NULL)
	, m_curselTreeItemData(0)
	, m_maxHistory2Show(20)
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
	ON_MESSAGE(WM_TRANSMITSERVER, &CAlarmCenterDlg::OnTransmitserver)
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
END_MESSAGE_MAP()


// CAlarmCenterDlg message handlers

BOOL CAlarmCenterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CAppResource::GetInstance();

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL) {
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
	
	core::CUserManager* userMgr = core::CUserManager::GetInstance();
	const core::CUserInfo* user = userMgr->GetCurUserInfo();
	OnCuruserchangedResult((WPARAM)user, 0);
	userMgr->RegisterObserver(this, OnCurUserChanged);

	CString welcom;
	welcom.LoadStringW(IDS_STRING_WELCOM);
	core::CHistoryRecord* hr = core::CHistoryRecord::GetInstance();
	hr->RegisterObserver(this, OnNewRecord);
	hr->InsertRecord(-1, -1, welcom, time(NULL), core::RECORD_LEVEL_USERLOG);

	//m_progressDlg = new CLoadFromDBProgressDlg();
	CLoadFromDBProgressDlg dlg;
	dlg.DoModal();
	//m_progressDlg->Create(IDD_DIALOG_PROGRESS, this);
	

	CAlarmCenterApp* app = (CAlarmCenterApp*)AfxGetApp();
	CString sPort;
	sPort.Format(L"%d", app->m_local_port);
	m_sLocalPort.SetWindowTextW(sPort);

	//m_cur_user_id.EnableWindow(0);
	//m_cur_user_name.EnableWindow(0);
	//m_cur_user_phone.EnableWindow(0);

	

	SetTimer(cTimerIdTime, 1000, NULL);
	//SetTimer(cTimerIdHistory, 2000, NULL);

#if !defined(DEBUG) && !defined(_DEBUG)
	SetWindowPos(&CWnd::wndTopMost, 0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN), SWP_SHOWWINDOW);
#else
	MoveWindow(0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN), TRUE);
#endif

	InitDisplay();
	InitAlarmMacines();
	net::CNetworkConnector::GetInstance()->StartNetwork(app->m_local_port,
														app->m_transmit_server_ip,
														app->m_transmit_server_port);
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
	using namespace core;
	CGroupManager* mgr = CGroupManager::GetInstance();
	CGroupInfo* rootGroup = mgr->GetRootGroupInfo();
	if (rootGroup) {
		CString txt;
		txt.Format(L"%s %d", rootGroup->get_name(), rootGroup->get_child_machine_count());
		HTREEITEM hRoot = m_treeGroup.GetRootItem();
		HTREEITEM hRootGroup = m_treeGroup.InsertItem(txt, hRoot);
		m_treeGroup.SetItemData(hRootGroup, (DWORD_PTR)rootGroup);

		TraverseGroup(hRootGroup, rootGroup);

		bool ok = false;
		CGroupInfo* curselGroupInfo = NULL;
		if (rootGroup->get_child_group_count() > 0) {
			m_curselTreeItem = NULL;
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
	//CGroupManager* mgr = CGroupManager::GetInstance();
	CString txt;
	CGroupInfoList groupList;
	group->GetChildGroups(groupList);

	std::list<CGroupInfo*>::iterator group_iter = groupList.begin();
	while (group_iter != groupList.end()) {
		CGroupInfo* child_group = *group_iter++;
		txt.Format(L"%s %d", child_group->get_name(), child_group->get_child_machine_count());
		HTREEITEM hChildItem = m_treeGroup.InsertItem(txt, hItemGroup);
		m_treeGroup.SetItemData(hChildItem, (DWORD_PTR)child_group);
		TraverseGroup(hChildItem, child_group);
	}

	/*CAlarmMachineList machineList;
	group->GetChildMachines(machineList);
	std::list<CAlarmMachine*>::iterator machine_iter = machineList.begin();
	while (machine_iter != machineList.end()) {
		CAlarmMachine* machine = *machine_iter++;
		txt.Format(L"%s(%04d)", machine->get_alias(), machine->get_ademco_id());
		HTREEITEM hChildItem = m_treeGroup.InsertItem(txt, hItemGroup);
		m_treeGroup.SetItemData(hChildItem, (DWORD_PTR)machine);
	}*/
}


void CAlarmCenterDlg::InitDisplay()
{
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
	m_maxHistory2Show = rcHistory.Height() / columnHeight - 1;

	m_qrcodeViewDlg = new CQrcodeViewerDlg(this);
	m_qrcodeViewDlg->Create(IDD_DIALOG_CSR_ACCT, this);
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

	}

	CDialogEx::OnTimer(nIDEvent);
}


void CAlarmCenterDlg::OnDestroy()
{
	KillTimer(1);
	SAFEDELETEDLG(m_wndContainer);
	SAFEDELETEDLG(m_wndContainerAlarming);
	SAFEDELETEDLG(m_qrcodeViewDlg);
	//SAFEDELETEDLG(m_progressDlg);
	net::CNetworkConnector::GetInstance()->StopNetWork();
	net::CNetworkConnector::ReleaseObject();
	core::CAlarmMachineManager::ReleaseObject();
	util::CConfigHelper::ReleaseObject();
	CAppResource::ReleaseObject();
	CString goodbye;
	goodbye.LoadStringW(IDS_STRING_GOODBYE);
	core::CHistoryRecord* hr = core::CHistoryRecord::GetInstance();
	hr->InsertRecord(-1, -1, goodbye, time(NULL), core::RECORD_LEVEL_USERLOG);
	hr->UnRegisterObserver(this);
	hr->ReleaseObject();
	core::CUserManager::ReleaseObject();
	core::CSoundPlayer::ReleaseObject();
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
	m_wndContainer->ShowMachinesOfGroup(NULL);
	CMachineManagerDlg dlg(this);
	dlg.DoModal();
	
	m_treeGroup.DeleteAllItems();

	using namespace core;
	CGroupManager* mgr = CGroupManager::GetInstance();
	CGroupInfo* rootGroup = mgr->GetRootGroupInfo();
	if (rootGroup) {
		CString txt;
		txt.Format(L"%s %d", rootGroup->get_name(), rootGroup->get_child_machine_count());
		HTREEITEM hRoot = m_treeGroup.GetRootItem();
		HTREEITEM hRootGroup = m_treeGroup.InsertItem(txt, hRoot);
		m_treeGroup.SetItemData(hRootGroup, (DWORD_PTR)rootGroup);

		TraverseGroup(hRootGroup, rootGroup);

		//m_curselTreeItem = hRootGroup;
		//m_curselTreeItemData = (DWORD)rootGroup;
		// 优先选择上次选中的分组项
		bool ok = false;
		CGroupInfo* curselGroupInfo = NULL;
		if (m_curselTreeItemData != 0) {
			ok = SelectGroupItemOfTree(m_curselTreeItemData);
			if (ok) {
				curselGroupInfo = reinterpret_cast<CGroupInfo*>(m_curselTreeItemData);
			}
		}

		// 不行就选择第一个子分组项
		if (!ok) {
			if (rootGroup->get_child_group_count() > 0) {
				m_curselTreeItem = NULL;
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
	CDialogEx::OnCancel();
}


afx_msg LRESULT CAlarmCenterDlg::OnNewrecordResult(WPARAM wParam, LPARAM /*lParam*/)
{
	core::HistoryRecord* record = reinterpret_cast<core::HistoryRecord*>(wParam);
	assert(record);
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
	// LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	HTREEITEM hItem = m_treeGroup.GetSelectedItem();

	//if (m_treeGroup.ItemHasChildren(hItem)) {  // group item
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
			m_wndContainer->ShowWindow(SW_SHOW);
		}
	//} else {	// machine item
	//
	//}

	*pResult = 0;
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
}
