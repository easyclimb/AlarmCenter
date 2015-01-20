
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

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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
	//DDX_Control(pDX, IDC_STATIC_COMPUTER, m_staticComputer);
	//DDX_Control(pDX, IDC_STATIC_CONNECTION, m_staticConnection);
	DDX_Control(pDX, IDC_STATIC_INTERNET, m_staticInternet);
	DDX_Control(pDX, IDC_STATIC_CONTROL_PANEL, m_groupControlPanel);
	//  DDX_Control(pDX, IDC_BUTTON1, m_btn1);
	//DDX_Control(pDX, IDC_BUTTON1, m_btn1);
}

BEGIN_MESSAGE_MAP(CAlarmCenterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CAlarmCenterDlg::OnBnClickedButton1)
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CAlarmCenterDlg message handlers

BOOL CAlarmCenterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	

	SetTimer(1, 1000, NULL);
	CRect rect(0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN));
	//#if !defined(DEBUG) && !defined(_DEBUG)
	SetWindowPos(&CWnd::wndTopMost, 0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN), SWP_SHOWWINDOW);
	//#else
	//MoveWindow(0, 0, rect.Width(), rect.Height(), TRUE);
	//#endif

	InitDisplay();

	m_btn1 = new CMFCButton();
	m_btn1->Create(L"HELLO WORLD", 
				   WS_CHILD | WS_VISIBLE | BS_ICON, 
				   CRect(20, 20, 150, 80),
				   this, IDC_BUTTON1);
	//m_btn1->setd
	InitAlarmMacines();

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

	m_staticInternet.MoveWindow(rc.left + 10, rc.top + 10, 32, 32);
	m_staticSysTime.MoveWindow(rc.left + 5, rc.bottom - 15, rc.Width(), 15);

	rc.DeflateRect(5, 5, 5, 18);
	CRect rcLeft(rc);
	rcLeft.right = rcLeft.left + 300;
	CRect rcRight(rc);
	rcRight.left = rcLeft.right + 5;

	m_groupControlPanel.MoveWindow(rcLeft);
	m_groupMachineList.MoveWindow(rcRight);

	m_wndContainer = new CAlarmMachineContainerDlg(this);
	m_wndContainer->Create(IDD_DIALOG_CONTAINER, this);
	rcRight.DeflateRect(5, 15, 5, 5);
	m_wndContainer->MoveWindow(rcRight);
	m_wndContainer->ShowWindow(SW_SHOW);
}


void CAlarmCenterDlg::OnBnClickedButton1()
{
	//static bool b = true;
	//CString exePath = GetModuleFilePath();
	//CImage image;
	//if (b) {
	//	//image.Load(L"./res/Network.png");
	//	
	//	m_staticInternet.ShowBmp(exePath + L"\\Resource\\Network.bmp");
	//}
	//else {
	//	//image.Load(L"./res/Network16.png");
	//	m_staticInternet.ShowBmp(exePath + L"\\Resource\\Network1.bmp");
	//}

	//b = !b;
	/*CBitmap bitmap;
	bitmap.Attach(image.Detach());
	m_staticInternet.SetBitmap(bitmap);*/

	//HICON hIcon = NULL;
	/*static int i = 0;
	switch (i) {
		case 0:
			m_btn1->SetIcon(m_hIconArm);
			break;
		case 1:
			m_btn1->SetIcon(m_hIconDisarm);
			break;
		case 2:
			m_btn1->SetIcon(m_hIconNetOk);
			break;
		case 3:
			m_btn1->SetIcon(m_hIconNetFailed);
			break;
		default:
			m_btn1->SetIcon(NULL);
			break;
	}
	i = (i+1) % 5;
	*/
}


void CAlarmCenterDlg::OnTimer(UINT_PTR nIDEvent)
{
	CTime now = CTime::GetCurrentTime();
	CString time = now.Format(L"%Y-%m-%d %H:%M:%S");
	m_staticSysTime.SetWindowTextW(time);
	CDialogEx::OnTimer(nIDEvent);
}


void CAlarmCenterDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	
	//m_btn1.CleanUp();
	m_btn1->DestroyWindow();
	delete m_btn1;
	SAFEDELETEDLG(m_wndContainer);
}
