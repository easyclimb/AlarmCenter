
// TestNetmoduleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TestNetmodule.h"
#include "TestNetmoduleDlg.h"
#include "afxdialogex.h"
#include "server.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const int TIMER_ID_SEND = 1;
static const int TIMER_ID_RECV = 2;

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


// CTestNetmoduleDlg dialog



CTestNetmoduleDlg::CTestNetmoduleDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTestNetmoduleDlg::IDD, pParent)
	, m_ademco_id(4)
	, m_ademco_event(1704)
	, m_zone(1)
	, m_gg(0)
	, m_device_id(_T("ee71f17244ba5c95e3bf4990df79a1ac"))
	, m_sendGap(100)
	, m_expectEvent(1704)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestNetmoduleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_ademco_id);
	DDX_Text(pDX, IDC_EDIT2, m_device_id);
	DDX_Text(pDX, IDC_EDIT4, m_sendGap);
	DDX_Control(pDX, IDC_EDIT5, m_sendCnt);
	DDX_Control(pDX, IDC_EDIT6, m_recvCnt);
	DDX_Text(pDX, IDC_EDIT3, m_ademco_event);
	DDX_Text(pDX, IDC_EDIT7, m_expectEvent);
}

BEGIN_MESSAGE_MAP(CTestNetmoduleDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CTestNetmoduleDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CTestNetmoduleDlg::OnBnClickedCancel)
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON1, &CTestNetmoduleDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CTestNetmoduleDlg message handlers

BOOL CTestNetmoduleDlg::OnInitDialog()
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

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTestNetmoduleDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CTestNetmoduleDlg::OnPaint()
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
HCURSOR CTestNetmoduleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

static void __stdcall OnAdemcoEvent(void* udata, int ademco_event)
{
	CTestNetmoduleDlg* dlg = reinterpret_cast<CTestNetmoduleDlg*>(udata);
	dlg->OnAdemcoEventResult(ademco_event);
}


void CTestNetmoduleDlg::OnAdemcoEventResult(int ademco_event)
{
	AUTO_LOG_FUNCTION;
	while (!m_lock.TryLock()) {
		JLOG(L"try lock failed.\n");
		Sleep(100);
	}
	m_eventList.push_back(ademco_event);
	m_lock.UnLock();
}


void CTestNetmoduleDlg::OnBnClickedOk()
{
	UpdateData();
	net::server::CServer* server = net::server::CServer::GetInstance();
	server->m_cb = OnAdemcoEvent;
	server->m_udata = this;
	if (net::server::CServer::GetInstance()->Start(12345)) {
		SetTimer(TIMER_ID_RECV, 300, NULL);
		SetTimer(TIMER_ID_SEND, m_sendGap, NULL);
	}
}


void CTestNetmoduleDlg::OnBnClickedCancel()
{
	//CDialogEx::OnCancel();
	net::server::CServer::GetInstance()->Stop();
	net::server::CServer::ReleaseObject();
	KillTimer(TIMER_ID_RECV);
	
}


void CTestNetmoduleDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	OnBnClickedCancel();
	CDialogEx::OnCancel();
}


void CTestNetmoduleDlg::OnTimer(UINT_PTR nIDEvent)
{
	AUTO_LOG_FUNCTION;
	if (TIMER_ID_SEND == nIDEvent) {
		JLOG(L"TIMER_ID_SEND\n");
		net::server::CServer* server = net::server::CServer::GetInstance();
		if (server->IsConnectionEstablished()) {
			if (server->SendToClient(m_ademco_id, m_ademco_event, m_gg, m_zone, NULL, 0)) {
				int sendCnt = 0;
				CString s; m_sendCnt.GetWindowTextW(s);
				sendCnt = _ttoi(s) + 1;
				s.Format(L"%d", sendCnt);
				m_sendCnt.SetWindowTextW(s);
			}
		}
	} else if (TIMER_ID_RECV == nIDEvent) {
		JLOG(L"TIMER_ID_RECV\n");
		if (m_lock.TryLock()) {
			int recvCnt = 0;
			bool ok = false;
			while (m_eventList.size() > 0) {
				int ademco_event = m_eventList.front();
				m_eventList.pop_front();
				if (m_ademco_event == ademco_event) {
					recvCnt++;
					ok = true;
				}
			}
			if (ok) {
				CString s; m_recvCnt.GetWindowTextW(s);
				int oldCnt = _ttoi(s);
				s.Format(L"%d", oldCnt + recvCnt);
				m_recvCnt.SetWindowTextW(s);
			}
			m_lock.UnLock();
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CTestNetmoduleDlg::OnBnClickedButton1()
{
	KillTimer(TIMER_ID_SEND);
}
