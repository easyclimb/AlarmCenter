
// SerialFilterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SerialFilter.h"
#include "SerialFilterDlg.h"
#include "afxdialogex.h"

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
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSerialFilterDlg dialog



CSerialFilterDlg::CSerialFilterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSerialFilterDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSerialFilterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_port);
	DDX_Control(pDX, IDC_LIST1, m_list1);
	DDX_Control(pDX, IDC_LIST2, m_list2);
}

BEGIN_MESSAGE_MAP(CSerialFilterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CSerialFilterDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CSerialFilterDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CSerialFilterDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CSerialFilterDlg::OnBnClickedButton4)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CSerialFilterDlg message handlers

BOOL CSerialFilterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
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

void CSerialFilterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSerialFilterDlg::OnPaint()
{
	if (IsIconic())
	{
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
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSerialFilterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CSerialFilterDlg::OnBnClickedButton1()
{
	CString s;
	m_port.GetWindowTextW(s);
	int port = _ttoi(s);
	if (InitPort(NULL, port, 9600)) {
		StartMonitoring();
		KillTimer(1);
		SetTimer(1, 1, NULL);
	} else {
		AfxMessageBox(L"NO COM");
		::PostQuitMessage(0);
	}
}

BOOL CSerialFilterDlg::OnRecv(const char *cmd, WORD wLen)
{
	m_buff.Write(cmd, wLen);
	return TRUE;
}


void CSerialFilterDlg::OnBnClickedButton2()
{
	ClosePort();
}


void CSerialFilterDlg::OnBnClickedButton3()
{
	m_list1.ResetContent();
}


void CSerialFilterDlg::OnBnClickedButton4()
{
	m_list2.ResetContent();
}


void CSerialFilterDlg::OnTimer(UINT_PTR nIDEvent)
{
	USES_CONVERSION;
	char cmd[16] = { 0 };
	char out[128] = { 0 };
	char tmp[16] = { 0 };
	int recv = 0, len = 0, ndx = 0;
	while (m_buff.GetValidateLen() >= 8) {
		do {
			if (m_buff.Read(cmd, 1) != 1)
				break;
			if (static_cast<BYTE>(cmd[0]) != 0xEB)
				break;
			if (m_buff.Read(cmd + 1, 1) != 1)
				break;
			if (m_buff.Read(cmd + 2, 1) != 1)
				break;
			
			switch (static_cast<BYTE>(cmd[1])) {
				case 0xB1:	// 主机到软件，长度为 CMD_LEN_SERIAL_COMPUTER
					len = cmd[2];
					if (len == 8) {
						if (m_buff.Read(cmd + 2, 5) == 5) {
							for (int i = 0; i < 8; i++) {
								sprintf_s(tmp, "%02X ", BYTE(cmd[i]));
								strcat_s(out, tmp);
							}
							ndx = m_list1.InsertString(-1, A2W(out));
							m_list1.SetCurSel(ndx);
						}
					} else if (len == 10) {
						if (m_buff.Read(cmd + 2, 7) == 7) {
							for (int i = 0; i < 10; i++) {
								sprintf_s(tmp, "%02X ", BYTE(cmd[i]));
								strcat_s(out, tmp);
							}
							ndx = m_list1.InsertString(-1, A2W(out));
							m_list1.SetCurSel(ndx);
						}
					}
					break;
				case 0xB5:
					len = cmd[2];
					if (len == 8) {
						if (m_buff.Read(cmd + 2, 5) == 5) {
							for (int i = 0; i < 8; i++) {
								sprintf_s(tmp, "%02X ", BYTE(cmd[i]));
								strcat_s(out, tmp);
							}
							ndx = m_list2.InsertString(-1, A2W(out));
							m_list2.SetCurSel(ndx);
						}
					} else if (len == 10) {
						if (m_buff.Read(cmd + 2, 7) == 7) {
							for (int i = 0; i < 10; i++) {
								sprintf_s(tmp, "%02X ", BYTE(cmd[i]));
								strcat_s(out, tmp);
							}
							ndx = m_list2.InsertString(-1, A2W(out));
							m_list2.SetCurSel(ndx);
						}
					}
					break;
				default:
					break;
			}
		} while (0);
	}

	__super::OnTimer(nIDEvent);
}


BOOL CSerialFilterDlg::OnSend(IN char* cmd, IN WORD wLen, OUT WORD& wRealLen)
{
	return FALSE;
}

void CSerialFilterDlg::OnConnectionEstablished()
{}
