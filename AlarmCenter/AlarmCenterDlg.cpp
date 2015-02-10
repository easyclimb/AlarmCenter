
// AlarmCenterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AlarmCenterDlg.h"
#include "afxdialogex.h"
#include "AlarmMachineContainer.h"
#include "AlarmMachineManager.h"
#include "AlarmMachine.h"
#include "BtnST.h"
#include "NetworkConnector.h"
#include "QrcodeViewerDlg.h"
#include "UserInfo.h"
#include "LoginDlg.h"
#include "UserManagerDlg.h"
#include "HistoryRecord.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static void __stdcall OnCurUserChangedResult(void* udata, core::CUserInfo* user)
{
	CAlarmCenterDlg* dlg = reinterpret_cast<CAlarmCenterDlg*>(udata);
	dlg->SendMessage(WM_CURUSERCHANGED, (WPARAM)(user));
}

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
	, m_hIconComputer(NULL)
	, m_hIconConnection(NULL)
	, m_hIconInternet(NULL)
	, m_qrcodeViewDlg(NULL)
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
}

BEGIN_MESSAGE_MAP(CAlarmCenterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_TRANSMITSERVER, &CAlarmCenterDlg::OnTransmitserver)
	ON_MESSAGE(WM_CURUSERCHANGED, &CAlarmCenterDlg::OnCuruserchanged)
	ON_BN_CLICKED(IDC_BUTTON_SWITCH_USER, &CAlarmCenterDlg::OnBnClickedButtonSwitchUser)
	ON_BN_CLICKED(IDC_BUTTON_USERMGR, &CAlarmCenterDlg::OnBnClickedButtonUsermgr)
	ON_BN_CLICKED(IDC_BUTTON_VIEW_QRCODE, &CAlarmCenterDlg::OnBnClickedButtonViewQrcode)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CAlarmCenterDlg message handlers

BOOL CAlarmCenterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

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

	CAlarmCenterApp* app = (CAlarmCenterApp*)AfxGetApp();
	CString sPort;
	sPort.Format(L"%d", app->m_local_port);
	m_sLocalPort.SetWindowTextW(sPort);

	//m_cur_user_id.EnableWindow(0);
	//m_cur_user_name.EnableWindow(0);
	//m_cur_user_phone.EnableWindow(0);

	core::CUserManager* userMgr = core::CUserManager::GetInstance();
	core::CUserInfo* user = userMgr->GetCurUserInfo();
	OnCuruserchanged((WPARAM)user, 0);
	userMgr->RegisterObserver(this, OnCurUserChangedResult);

	CString welcom;
	welcom.LoadStringW(IDS_STRING_WELCOM);
	core::CHistoryRecord::GetInstance()->InsertRecord(-1, welcom, time(NULL), 
													  core::RECORD_LEVEL_1);

	SetTimer(1, 1000, NULL);

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
	core::CAlarmMachineManager *manager = core::CAlarmMachineManager::GetInstance();
	core::CAlarmMachine* machine = NULL;
	if (manager->GetFirstMachine(machine) && machine) {
		m_wndContainer->InsertMachine(machine);
		while (manager->GetNextMachine(machine) && machine) {
			m_wndContainer->InsertMachine(machine);
		}
	}
}


void CAlarmCenterDlg::InitDisplay()
{
	CRect rc;
	GetClientRect(rc);

	rc.DeflateRect(5, 5, 5, 5);
	CRect rcLeft(rc);
	rcLeft.right = rcLeft.left + 320;
	CRect rcRight(rc);
	rcRight.left = rcLeft.right + 5;

	m_groupControlPanel.MoveWindow(rcLeft);
	m_groupMachineList.MoveWindow(rcRight);

	//m_qrcodeViewDlg = new CQrcodeViewerDlg(this);
	//m_qrcodeViewDlg->Create(IDD_DIALOG_CSR_ACCT, this);
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

	m_wndContainer = new CAlarmMachineContainerDlg(this);
	m_wndContainer->Create(IDD_DIALOG_CONTAINER, this);
	rcRight.DeflateRect(5, 15, 5, 5);
	m_wndContainer->MoveWindow(rcRight);
	m_wndContainer->ShowWindow(SW_SHOW);

	


}


void CAlarmCenterDlg::OnTimer(UINT_PTR nIDEvent)
{
	SYSTEMTIME st = { 0 };
	::GetLocalTime(&st);
	wchar_t now[1024] = { 0 };
	wsprintfW(now, L"%04d-%02d-%-2d %02d:%02d:%02d", st.wYear,
			  st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	//CTime now = CTime::GetCurrentTime();
	//CString time = now.Format(L"%Y-%m-%d %H:%M:%S");
	m_staticSysTime.SetWindowTextW(now);
	CDialogEx::OnTimer(nIDEvent);
}


void CAlarmCenterDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	CString goodbye;
	goodbye.LoadStringW(IDS_STRING_GOODBYE);
	core::CHistoryRecord::GetInstance()->InsertRecord(-1, goodbye, time(NULL),
													  core::RECORD_LEVEL_1);

	KillTimer(1);
	net::CNetworkConnector::GetInstance()->StopNetWork();
	SAFEDELETEDLG(m_wndContainer); 
	SAFEDELETEDLG(m_qrcodeViewDlg);
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


afx_msg LRESULT CAlarmCenterDlg::OnCuruserchanged(WPARAM wParam, LPARAM /*lParam*/)
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
	switch (user_priority) {
		case core::UP_SUPER:
			sPriority.LoadStringW(IDS_STRING_USER_SUPER);
			m_btnUserMgr.EnableWindow(1);
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
	CLoginDlg dlg;
	dlg.DoModal();
}


void CAlarmCenterDlg::OnBnClickedButtonUsermgr()
{
	CUserManagerDlg dlg;
	dlg.DoModal();
}


void CAlarmCenterDlg::OnBnClickedButtonViewQrcode()
{
	CQrcodeViewerDlg dlg;
	dlg.DoModal();
}


void CAlarmCenterDlg::OnClose()
{
	CDialogEx::OnClose();
}


void CAlarmCenterDlg::OnCancel()
{
	CString confirm;
	confirm.LoadStringW(IDS_STRING_CONFIRM_QUIT);
	int ret = MessageBox(confirm, NULL, MB_YESNO | MB_ICONQUESTION);
	if (ret == IDNO)
		return;
	CDialogEx::OnCancel();
}
