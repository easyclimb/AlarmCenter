
// SerialFilterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SerialFilter.h"
#include "SerialFilterDlg.h"
#include "afxdialogex.h"
#include "AutoSerialPort.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void format(CString& str, const unsigned char* buff, int len)
{
	CString tmp;
	for (int i = 0; i < len; i++) {
		tmp.Format(L" %02X", buff[i]);
		str += tmp;
	}
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
	DDX_Control(pDX, IDC_LIST3, m_list3);
	DDX_Control(pDX, IDC_BUTTON1, m_btnOpen);
	DDX_Control(pDX, IDC_COMBO1, m_cmb);
	DDX_Control(pDX, IDC_CHECK1, m_chkTop);
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
	ON_BN_CLICKED(IDC_BUTTON5, &CSerialFilterDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT1, &CSerialFilterDlg::OnBnClickedButtonExport1)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT2, &CSerialFilterDlg::OnBnClickedButtonExport2)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CHECK1, &CSerialFilterDlg::OnBnClickedCheck1)
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

	int ndx = 0;
	CString port = L"COM";
	CStringList portList;
	CAutoSerialPort autoPort;
	autoPort.CheckValidSerialPorts(portList);
	while (portList.GetCount() > 0) {
		CString p = portList.RemoveHead();
		ndx = m_cmb.AddString(port + p);
		m_cmb.SetItemData(ndx, _ttoi(p));
	}
	m_cmb.SetCurSel(0);

	m_hEventExit = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hThreadDeal = CreateThread(NULL, 0, ThreadDeal, this, 0, NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}


DWORD WINAPI CSerialFilterDlg::ThreadDeal(LPVOID lp)
{
	CSerialFilterDlg* dlg = reinterpret_cast<CSerialFilterDlg*>(lp);
	while (1) {
		if (WAIT_OBJECT_0 == WaitForSingleObject(dlg->m_hEventExit, 1))
			break;
		if (dlg->m_buff.GetValidateLen() < 10)
			continue;

		unsigned char cmd[16] = { 0 };
		DWORD len = 0;
		bool b1 = false, b5 = false, ok = true;;

		/*CTime now = CTime::GetCurrentTime();*/
		//CString str = now.Format(L"%H:%M:%S - ");
		SYSTEMTIME st = { 0 };
		GetLocalTime(&st);
		CString str;
		str.Format(L"%02d:%02d:%02d.%03d - ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

		//dlg->m_lock.Lock();
		do {
			if (1 != dlg->m_buff.Read((char*)cmd, 1))
				break;
			if (0xEB != cmd[0])
				break;
			if (1 != dlg->m_buff.Read((char*)cmd + 1, 1))
				break;
			BYTE type = (BYTE)cmd[1];
			switch (type) {
				// host send
				case 0xAA:
					len = 2;
					while (dlg->m_buff.GetValidateLen() < len) {
						if (WAIT_OBJECT_0 == WaitForSingleObject(dlg->m_hEventExit, 100)) {
							ok = false;
							break;
						}
					}
					if (ok)
						dlg->m_buff.Read((char*)cmd + 2, len);
					break;
				case 0xAB:
					len = 3;
					while (dlg->m_buff.GetValidateLen() < len) {
						if (WAIT_OBJECT_0 == WaitForSingleObject(dlg->m_hEventExit, 100)) {
							ok = false;
							break;
						}
					}
					if (ok)
						dlg->m_buff.Read((char*)cmd + 2, len);
					break;
				case 0xAE:
					len = 7;
					while (dlg->m_buff.GetValidateLen() < len) {
						if (WAIT_OBJECT_0 == WaitForSingleObject(dlg->m_hEventExit, 100)) {
							ok = false;
							break;
						}
					}
					if (ok)
						dlg->m_buff.Read((char*)cmd + 2, len);
					break;
				case 0xAF:
					len = 6;
					while (dlg->m_buff.GetValidateLen() < len) {
						if (WAIT_OBJECT_0 == WaitForSingleObject(dlg->m_hEventExit, 100)) {
							ok = false;
							break;
						}
					}
					if (ok)
						dlg->m_buff.Read((char*)cmd + 2, len);
					break;
				/*case 0xB0:
					while (dlg->m_buff.GetValidateLen() < 1) { Sleep(100); }
					dlg->m_buff.Read(cmd + 2, 1);
					len = (BYTE)cmd[2];
					while (dlg->m_buff.GetValidateLen() < len) { Sleep(100); }
					dlg->m_buff.Read(cmd + 3, len++);
					break;
				case 0xB1:
					while (dlg->m_buff.GetValidateLen() < 1) { Sleep(100); }
					dlg->m_buff.Read(cmd + 2, 1);
					len = (BYTE)cmd[2];
					while (dlg->m_buff.GetValidateLen() < len) { Sleep(100); }
					dlg->m_buff.Read(cmd + 3, len++);
					break;
				case 0xB3:
					while (dlg->m_buff.GetValidateLen() < 1) { Sleep(100); }
					dlg->m_buff.Read(cmd + 2, 1);
					len = (BYTE)cmd[2];
					while (dlg->m_buff.GetValidateLen() < len) { Sleep(100); }
					dlg->m_buff.Read(cmd + 3, len++);
					break;
				case 0xB5:
					while (dlg->m_buff.GetValidateLen() < 1) { Sleep(100); }
					dlg->m_buff.Read(cmd + 2, 1);
					len = (BYTE)cmd[2];
					break;*/
				case 0xB1:
					b1 = true;
				case 0xB5:
					b5 = true;
				default:
					while (dlg->m_buff.GetValidateLen() < 1) {
						if (WAIT_OBJECT_0 == WaitForSingleObject(dlg->m_hEventExit, 100)) {
							ok = false;
							break;
						}
					}
					if (!ok)
						break;
					dlg->m_buff.Read((char*)cmd + 2, 1);
					len = (BYTE)cmd[2] - 3;
					while (dlg->m_buff.GetValidateLen() < len) {
						if (WAIT_OBJECT_0 == WaitForSingleObject(dlg->m_hEventExit, 100)) {
							ok = false;
							break;
						}
					}
					if (!ok)
						break;
					dlg->m_buff.Read((char*)cmd + 3, len);
					break;
			}
			if (!ok)
				break;
			format(str, (const BYTE*)cmd, 3 + len);
			if (b1) {
				dlg->m_lock1.Lock();
				dlg->m_strlist1.AddTail(str);
				dlg->m_lock1.UnLock();
			} else if (b5) {
				dlg->m_lock2.Lock();
				dlg->m_strlist2.AddTail(str);
				dlg->m_lock2.UnLock();
			} else {
				dlg->m_lock3.Lock();
				dlg->m_strlist3.AddTail(str);
				dlg->m_lock3.UnLock();
			}
		} while (0);
		//dlg->m_lock.UnLock();
	}
	return 0;
}


void CSerialFilterDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSerialFilterDlg::OnPaint()
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
HCURSOR CSerialFilterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CSerialFilterDlg::OnBnClickedButton1()
{
	CString s;
	BOOL bopen = FALSE;
	m_btnOpen.GetWindowTextW(s);
	if (s.Compare(L"OPEN") == 0) {
		bopen = TRUE;
	}

	if (bopen) {
		//m_port.GetWindowTextW(s);
		int ndx = m_cmb.GetCurSel();
		int port = (int)m_cmb.GetItemData(ndx);
		if (InitPort(NULL, port, 9600)) {
			m_btnOpen.SetWindowTextW(L"CLOSE");
			StartMonitoring();
			KillTimer(1);
			SetTimer(1, 500, NULL);
		} else {
			AfxMessageBox(L"NO COM");
			::PostQuitMessage(0);
		}
	} else {
		ClosePort();
		m_btnOpen.SetWindowTextW(L"OPEN");
	}
}



enum CmdState
{
	CS_NORMAL,
	CS_LEN,
	CS_END,
};

BOOL CSerialFilterDlg::OnRecv(const char *cmd, WORD wLen)
{
	/*CString str;
	if (wLen > 3) {
	BYTE c = BYTE(cmd[1]);
	switch (c) {
	case 0xB1:
	format(str, (const BYTE*)cmd, wLen);
	m_lock1.Lock();
	m_strlist1.AddTail(str);
	m_lock1.UnLock();
	break;
	case 0xB5:
	format(str, (const BYTE*)cmd, wLen);
	m_lock2.Lock();
	m_strlist2.AddTail(str);
	m_lock2.UnLock();
	break;
	default:
	format(str, (const BYTE*)cmd, wLen);
	m_lock3.Lock();
	m_strlist3.AddTail(str);
	m_lock3.UnLock();
	break;
	}
	}*/
	//static unsigned char buff[1024] = { 0 };
	//static int pos = 0;
	//static int len = 0;
	////static bool eb = false;
	//static bool b1 = false;
	//static bool b5 = false;
	//static CTime now = CTime::GetCurrentTime();
	//static CString str = now.Format(L"%H:%M:%S - ");
	//static CmdState cs = CS_NORMAL;
	//
	//for (int i = 0; i < wLen; i++) {
	//	if (pos == 1024)
	//		pos = 0;
	//	BYTE c = (BYTE)cmd[i];
	//	buff[pos] = c;
	//	switch (c) {
	//		case 0xEB:
	//			//eb = true;
	//			switch (cs) {
	//				case CS_NORMAL:

	//					break;
	//				case CS_LEN:
	//					break;
	//				case CS_END:
	//					break;
	//				default:
	//					break;
	//			}
	//			if (b1) {
	//				b1 = false;
	//				format(str, buff, pos);
	//				m_lock1.Lock();
	//				m_strlist1.AddTail(str);
	//				m_lock1.UnLock();
	//			} else if (b5) {
	//				b5 = false;
	//				format(str, buff, pos);
	//				m_lock2.Lock();
	//				m_strlist2.AddTail(str);
	//				m_lock2.UnLock();
	//			} else {
	//				format(str, buff, pos);
	//				m_lock3.Lock();
	//				m_strlist3.AddTail(str);
	//				m_lock3.UnLock();
	//			}
	//			pos = 0;
	//			buff[pos++] = (BYTE)cmd[i];
	//			now = CTime::GetCurrentTime();
	//			str = now.Format(L"%H:%M:%S - ");
	//			break;
	//		case 0xB1:
	//			if (eb) {
	//				eb = false;
	//				b1 = true;
	//				pos++;
	//			}
	//			break;
	//		case 0xB5:
	//			if (eb) {
	//				eb = false;
	//				b5 = true;
	//				pos++;
	//			}
	//			break;
	//		default:
	//			eb = false;
	//			pos++;
	//			break;
	//	}

	//	

	//}

	m_buff.Write(cmd, wLen);
	return TRUE;
}


void CSerialFilterDlg::OnBnClickedButton2()
{
	ClosePort();
}


void CSerialFilterDlg::OnBnClickedButton3()
{
	m_lock1.Lock();
	m_list1.ResetContent();
	m_lock1.UnLock();
}


void CSerialFilterDlg::OnBnClickedButton4()
{
	m_lock2.Lock();
	m_list2.ResetContent();
	m_lock2.UnLock();
}


void CSerialFilterDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (m_lock1.TryLock()) {
		while (m_strlist1.GetCount() > 0) {
			CString str = m_strlist1.RemoveHead();
			int ndx = m_list1.InsertString(-1, str);
			m_list1.SetCurSel(ndx);
		}
		m_lock1.UnLock();
	}
	if (m_lock2.TryLock()) {
		while (m_strlist2.GetCount() > 0) {
			CString str = m_strlist2.RemoveHead();
			int ndx = m_list2.InsertString(-1, str);
			m_list2.SetCurSel(ndx);
		}
		m_lock2.UnLock();
	}
	if (m_lock3.TryLock()) {
		if (m_list3.GetCount() > 1000) {
			m_list3.ResetContent();
		}
		while (m_strlist3.GetCount() > 0) {
			CString str = m_strlist3.RemoveHead();
			int ndx = m_list3.InsertString(-1, str);
			m_list3.SetCurSel(ndx);
		}
		m_lock3.UnLock();
	}

	__super::OnTimer(nIDEvent);
}


BOOL CSerialFilterDlg::OnSend(IN char* /*cmd*/, IN WORD /*wLen*/, OUT WORD& /*wRealLen*/)
{
	return FALSE;
}

void CSerialFilterDlg::OnConnectionEstablished()
{}


void CSerialFilterDlg::OnBnClickedButton5()
{
	m_lock3.Lock();
	m_list3.ResetContent();
	m_lock3.UnLock();
}


BOOL GetSaveAsFilePath(HWND hWnd, CString& path)
{
	static CString prevPath = _T("");
RE_SAVE_AS:
	TCHAR szFilename[MAX_PATH] = { 0 };
	BOOL bResult = FALSE;
	DWORD dwError = NOERROR;
	OPENFILENAME ofn = { 0 };

	ofn.lStructSize = sizeof (OPENFILENAME);
	ofn.lpstrFilter = _T("Excel File(*.txt)\0*.txt\0\0");
	ofn.lpstrFile = szFilename;
	ofn.nMaxFile = MAX_PATH;
	ofn.hwndOwner = hWnd;
	ofn.Flags = OFN_EXPLORER |
		OFN_ENABLEHOOK |
		OFN_HIDEREADONLY |
		OFN_NOCHANGEDIR |
		OFN_PATHMUSTEXIST;
	ofn.lpfnHook = NULL;

	bResult = GetSaveFileName(&ofn);
	if (bResult == FALSE) {
		dwError = CommDlgExtendedError();
		return FALSE;
	}

	CString fileName = szFilename;
	if (CFileOper::GetFileExt(fileName).CompareNoCase(L"txt") != 0)
		fileName += L".txt";

	if (CFileOper::PathExists(fileName)) {
		int ret = MessageBox(hWnd, L"File exists, replace it?", L"", MB_YESNOCANCEL | MB_ICONQUESTION);
		if (ret == IDYES)
			DeleteFile(fileName);
		else if (ret == IDNO)
			goto RE_SAVE_AS;
		else
			return FALSE;
	}
	prevPath = fileName;
	path = fileName;
	return TRUE;
}


void CSerialFilterDlg::OnBnClickedButtonExport1()
{
	ClosePort();
	m_btnOpen.SetWindowTextW(L"OPEN");
	USES_CONVERSION;
	CString path;
	if (GetSaveAsFilePath(m_hWnd, path)) {
		CFile f;
		if (f.Open(path, CFile::modeCreate | CFile::modeWrite)) {
			f.Write("EB B1 command:\r\n", strlen("EB B1 command:\r\n"));
			CString line;
			for (int i = 0; i < m_list1.GetCount(); i++) {
				m_list1.GetText(i, line);
				const char* sline = W2A(line);
				f.Write(sline, strlen(sline));
				f.Write("\r\n", 2);
			}
			f.Close();
			MessageBox(L"ok");
		}
	}
	OnBnClickedButton1();
}


void CSerialFilterDlg::OnBnClickedButtonExport2()
{
	ClosePort();
	m_btnOpen.SetWindowTextW(L"OPEN");
	USES_CONVERSION;
	CString path;
	if (GetSaveAsFilePath(m_hWnd, path)) {
		CFile f;
		if (f.Open(path, CFile::modeCreate | CFile::modeWrite)) {
			f.Write("EB B5 command:\r\n", strlen("EB B5 command:\r\n"));
			CString line;
			for (int i = 0; i < m_list2.GetCount(); i++) {
				m_list2.GetText(i, line);
				const char* sline = W2A(line);
				f.Write(sline, strlen(sline));
				f.Write("\r\n", 2);
			}
			f.Close();
			MessageBox(L"ok");
		}
	}
	OnBnClickedButton1();
}


void CSerialFilterDlg::OnDestroy()
{
	ClosePort();
	KillTimer(1);
	SetEvent(m_hEventExit);
	WaitForSingleObject(m_hThreadDeal, INFINITE);
	CLOSEHANDLE(m_hEventExit);
	CLOSEHANDLE(m_hThreadDeal);

	__super::OnDestroy();
}


void CSerialFilterDlg::OnBnClickedCheck1()
{
	CRect rc;
	GetWindowRect(rc);
	BOOL b = m_chkTop.GetCheck();
	if (b)
		::SetWindowPos(m_hWnd, HWND_TOPMOST, rc.left, rc.top, rc.Width(), rc.Height(), SWP_SHOWWINDOW);
	else
		::SetWindowPos(m_hWnd, HWND_NOTOPMOST, rc.left, rc.top, rc.Width(), rc.Height(), SWP_SHOWWINDOW);
}
