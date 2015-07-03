
// AlarmMachineConfigToolDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "AlarmMachineConfigTool.h"
#include "AlarmMachineConfigToolDlg.h"
#include "afxdialogex.h"
#include "AutoSerialPort.h"
#include "ContainerDlg.h"
#include "SoundPlayer.h"
#include "InputPswDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CAlarmMachineConfigToolDlg 对话框



CAlarmMachineConfigToolDlg::CAlarmMachineConfigToolDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAlarmMachineConfigToolDlg::IDD, pParent)
	, m_status(UNKNOWN)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAlarmMachineConfigToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_cmbCom);
	DDX_Control(pDX, IDC_BUTTON_OPEN_CLOSE, m_btnOpenClose);
	DDX_Control(pDX, IDC_STATIC_GROUP, m_staticGroup);
	DDX_Control(pDX, IDC_BUTTON_CONFIG_OR_QUIT, m_btnConfigOrQuitConfig);
}

BEGIN_MESSAGE_MAP(CAlarmMachineConfigToolDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CAlarmMachineConfigToolDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_CLOSE, &CAlarmMachineConfigToolDlg::OnBnClickedButtonOpenClose)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_CONFIG_OR_QUIT, &CAlarmMachineConfigToolDlg::OnBnClickedButtonConfigOrQuit)
END_MESSAGE_MAP()


// CAlarmMachineConfigToolDlg 消息处理程序

BOOL CAlarmMachineConfigToolDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	CRect rc;
	m_staticGroup.GetWindowRect(rc);
	ScreenToClient(rc);
	rc.DeflateRect(5, 25, 5, 5);
	/*m_container = new CContainerDlg();
	m_container->Create(IDD_DIALOG_CONTAINER, this);
	m_container->MoveWindow(rc);
	m_container->ShowWindow(SW_SHOW);*/

	int ndx = 0;
	CString port = L"COM";
	CStringList portList;
	CAutoSerialPort autoPort;
	autoPort.CheckValidSerialPorts(portList);
	while (portList.GetCount() > 0) {
		CString p = portList.RemoveHead();
		ndx = m_cmbCom.AddString(port + p);
		m_cmbCom.SetItemData(ndx, _ttoi(p));
	}
	m_cmbCom.SetCurSel(0);

	m_hEventExit = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hThreadDeal = CreateThread(NULL, 0, ThreadDeal, this, 0, NULL);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CAlarmMachineConfigToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CAlarmMachineConfigToolDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CAlarmMachineConfigToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CAlarmMachineConfigToolDlg::OnBnClickedOk()
{
}
unsigned char chk_sum(const char* buff, int size)
{
	unsigned char sum = 0;
	for (int i = 0; i < size; i++) {
		sum += static_cast<unsigned char>(buff[i]);
	}
	return sum;
}

DWORD WINAPI CAlarmMachineConfigToolDlg::ThreadDeal(LPVOID lp)
{
	CAlarmMachineConfigToolDlg* dlg = reinterpret_cast<CAlarmMachineConfigToolDlg*>(lp);
	while (1) {
		if (WAIT_OBJECT_0 == WaitForSingleObject(dlg->m_hEventExit, 1))
			break;

		Task* task = dlg->GetTask();
		if (task) {
			dlg->m_sendbuff.Write(task->buff, task->len);
			delete task;
			task = NULL;
		}

		if (dlg->m_buff.GetValidateLen() < 1)
			continue;

		unsigned char cmd[16] = { 0 };
		DWORD len = 0;
		bool b1 = false, b5 = false, ok = true;;

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
					if (ok) {
						dlg->m_buff.Read((char*)cmd + 2, len);
						BYTE snd = ((BYTE)cmd[5] & 0xF0) | (((BYTE)cmd[6] & 0xF0) >> 4);
						//BYTE set = (BYTE)cmd[3] & 0x01;
						//
						if (snd < SI_MAX) {
							CSoundPlayer::GetInstance()->PlayWavSound(snd);
							Status status = UNKNOWN;
							if (snd == SI_HOST_MODULATION) {
								status = CONFIG;
							}
							if (snd == SI_HOST_DUIMA) {
								status = DUIMA;
							}
							dlg->UpdateStatus(status);
						}
					}
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
			/*format(str, (const BYTE*)cmd, 3 + len);
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
			}*/
		} while (0);
		//dlg->m_lock.UnLock();
	}
	return 0;
}


BOOL CAlarmMachineConfigToolDlg::OnRecv(const char *cmd, WORD wLen)
{
	m_buff.Write(cmd, wLen);
	return TRUE;
}


BOOL CAlarmMachineConfigToolDlg::OnSend(IN char* cmd, IN WORD wLen, OUT WORD& wRealLen)
{
	wRealLen = m_sendbuff.GetValidateLen() & 0xFFFF;
	if (wRealLen > 0) {
		m_sendbuff.Read(cmd, wRealLen);
		return TRUE;
	}
	return FALSE;
}


void CAlarmMachineConfigToolDlg::OnConnectionEstablished()
{}



void CAlarmMachineConfigToolDlg::OnBnClickedButtonOpenClose()
{
	CString s;
	BOOL bopen = FALSE;
	m_btnOpenClose.GetWindowTextW(s);
	if (s.Compare(L"打开") == 0) {
		bopen = TRUE;
	}

	if (bopen) {
		int ndx = m_cmbCom.GetCurSel();
		int port = (int)m_cmbCom.GetItemData(ndx);
		if (InitPort(NULL, port, 9600)) {
			m_cmbCom.EnableWindow(0);
			m_btnOpenClose.SetWindowTextW(L"关闭");
			StartMonitoring();
			KillTimer(1);
			SetTimer(1, 500, NULL);
		} else {
			AfxMessageBox(L"打开串口失败!");
			::PostQuitMessage(0);
		}
	} else {
		ClosePort();
		m_btnOpenClose.SetWindowTextW(L"打开");
		m_cmbCom.EnableWindow(1);
	}
}


void CAlarmMachineConfigToolDlg::OnDestroy()
{
	ClosePort();
	KillTimer(1);
	SetEvent(m_hEventExit);
	WaitForSingleObject(m_hThreadDeal, INFINITE);
	CLOSEHANDLE(m_hEventExit);
	CLOSEHANDLE(m_hThreadDeal);

	//SAFEDELETEDLG(m_container);

	__super::OnDestroy();
}


void CAlarmMachineConfigToolDlg::OnBnClickedButtonConfigOrQuit()
{
#pragma warning(disable: 4309)
	static char cmd_quit_config[] = {
		RD_0, RD_SIGN,
		RD_0, RD_SIGN,
		RD_0, RD_SIGN, };
	static char buff[5] = { 0xEB, 0xAB, 0x3, 0x0, 0x0 };
	CString s;
	m_btnConfigOrQuitConfig.GetWindowTextW(s);
	if (s.Compare(L"进入设置状态") == 0) {
		char cmd[] = { /*0xEB, 0xAB, 15,*/
			RD_SIGN, RD_SIGN, RD_SIGN, 
		};
		for (int i = 0; i < sizeof(cmd_quit_config); i++) {
			buff[3] = cmd_quit_config[i];
			buff[4] = chk_sum(buff, 4);
			AddTask(new Task(buff, sizeof(buff) / sizeof(char)));
			Sleep(100);
		}
		for (int i = 0; i < sizeof(cmd); i++) {
			buff[3] = cmd[i];
			buff[4] = chk_sum(buff, 4);
			AddTask(new Task(buff, sizeof(buff) / sizeof(char)));
			Sleep(100);
		}
		CInputPswDlg dlg;
		if (dlg.DoModal() != IDOK) {
			for (int i = 0; i < sizeof(cmd_quit_config); i++) {
				buff[3] = cmd_quit_config[i];
				buff[4] = chk_sum(buff, 4);
				AddTask(new Task(buff, sizeof(buff) / sizeof(char)));
				Sleep(100);
			}
		} else {
			USES_CONVERSION;
			const char* psw = W2A(dlg.m_strPsw);
			for (int i = 0; i < 6; i++) {
				buff[3] = DecToRd(psw[i] - '0');
				buff[4] = chk_sum(buff, 4);
				AddTask(new Task(buff, sizeof(buff) / sizeof(char)));
				Sleep(100);
			}
			buff[3] = RD_SIGN;
			buff[4] = chk_sum(buff, 4);
			AddTask(new Task(buff, sizeof(buff) / sizeof(char)));
		}
		//AddTask(new Task(cmd, sizeof(cmd) / sizeof(char)));
	} else if (s.Compare(L"退出设置") == 0) {
		for (int i = 0; i < sizeof(cmd_quit_config); i++) {
			buff[3] = cmd_quit_config[i];
			buff[4] = chk_sum(buff, 4);
			AddTask(new Task(buff, sizeof(buff) / sizeof(char)));
			Sleep(100);
		}
	}
}


void CAlarmMachineConfigToolDlg::UpdateStatus(Status status)
{
	if (m_status != status) {
		m_status = status;
		if (status == UNKNOWN) {
			m_btnConfigOrQuitConfig.SetWindowTextW(L"进入设置状态");
		} else if (status == CONFIG) {
			m_btnConfigOrQuitConfig.SetWindowTextW(L"退出设置");
		}
	}
}
