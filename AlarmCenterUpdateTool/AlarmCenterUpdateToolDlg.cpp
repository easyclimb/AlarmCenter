
// AlarmCenterUpdateToolDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "AlarmCenterUpdateTool.h"
#include "AlarmCenterUpdateToolDlg.h"
#include "afxdialogex.h"
#include "update_tool.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

std::shared_ptr<update_tool> g_up = nullptr;
std::chrono::steady_clock::time_point g_tp;

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CAlarmCenterUpdateToolDlg �Ի���



CAlarmCenterUpdateToolDlg::CAlarmCenterUpdateToolDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_ALARMCENTERUPDATETOOL_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAlarmCenterUpdateToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
	DDX_Control(pDX, IDC_STATIC_TIME, m_time);
	DDX_Control(pDX, IDC_STATIC_PROGRESS, m_static_progress);
	DDX_Control(pDX, IDC_BUTTON_SEE, m_btn_see);
	DDX_Control(pDX, IDC_STATIC_DETAIL, m_group_detail);
	DDX_Control(pDX, IDC_LIST1, m_detail);
}

BEGIN_MESSAGE_MAP(CAlarmCenterUpdateToolDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CAlarmCenterUpdateToolDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CAlarmCenterUpdateToolDlg::OnBnClickedCancel)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_SEE, &CAlarmCenterUpdateToolDlg::OnBnClickedButtonSee)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CAlarmCenterUpdateToolDlg ��Ϣ�������

BOOL CAlarmCenterUpdateToolDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	SetWindowText(get_string_from_resouce(IDS_STRING_TITLE).c_str());
	expand(false);

	g_tp = std::chrono::steady_clock::now();
	SetTimer(1, 100, nullptr);

	g_up = std::make_shared<update_tool>();

	g_up->start();

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CAlarmCenterUpdateToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	} else {
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CAlarmCenterUpdateToolDlg::OnPaint()
{
	if (IsIconic()) {
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	} else {
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CAlarmCenterUpdateToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CAlarmCenterUpdateToolDlg::OnBnClickedOk()
{
}


void CAlarmCenterUpdateToolDlg::OnBnClickedCancel()
{
}


void CAlarmCenterUpdateToolDlg::OnClose()
{
	return;

	//CDialogEx::OnClose();
}


void CAlarmCenterUpdateToolDlg::expand(bool b)
{
	CRect rc, rc2;
	GetWindowRect(rc);
	m_group_detail.GetWindowRect(rc2);
	//ScreenToClient(rc2);

	if (b) {
		rc.bottom = rc2.bottom + 5;
		MoveWindow(rc);
	} else {
		rc.bottom = rc2.top;
		MoveWindow(rc);
	}
}


void CAlarmCenterUpdateToolDlg::OnBnClickedButtonSee()
{
	CString name, show, hide, done;
	show = get_string_from_resouce(IDS_STRING_DETAIL).c_str();
	hide = get_string_from_resouce(IDS_STRING_HIDE).c_str();
	done = get_string_from_resouce(IDS_STRING_DONE).c_str();
	m_btn_see.GetWindowText(name);

	if (name == show) {
		expand();
		m_btn_see.SetWindowTextW(hide);
	}

	if (name == hide) {
		expand(false);
		m_btn_see.SetWindowTextW(show);
	}

	if (name == done) {
		CDialogEx::OnOK();
	}
}


void CAlarmCenterUpdateToolDlg::OnTimer(UINT_PTR nIDEvent)
{
	CString txt;

	up_ptr up = g_up->get_up();
	if (up) {
		m_progress.SetRange32(0, up->total_);
		m_progress.SetPos(up->progress_);
		
		txt.Format(L"%d/%d", up->progress_, up->total_);
		m_static_progress.SetWindowTextW(txt);

		int ndx = m_detail.AddString(up->msg_.c_str());
		m_detail.SetCurSel(ndx);

		if (up->progress_ == up->total_) {
			// done 
			KillTimer(1);

			m_btn_see.SetWindowTextW(get_string_from_resouce(IDS_STRING_DONE).c_str());
		}
	}

	auto now = std::chrono::steady_clock::now();
	auto diff = now - g_tp;
	auto secs = std::chrono::duration_cast<std::chrono::seconds>(diff);
	int min = secs.count() / 60;
	int sec = secs.count() % 60;
	txt.Format(L"%02d:%02d", min, sec);
	m_time.SetWindowTextW(txt);

	CDialogEx::OnTimer(nIDEvent);
}
