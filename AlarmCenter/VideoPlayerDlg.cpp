// VideoPlayerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "VideoPlayerDlg.h"
#include "afxdialogex.h"
#include "tinyxml/tinyxml.h"


// CVideoPlayerDlg dialog
CVideoPlayerDlg* g_videoPlayerDlg = NULL;
IMPLEMENT_DYNAMIC(CVideoPlayerDlg, CDialogEx)

CVideoPlayerDlg::CVideoPlayerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CVideoPlayerDlg::IDD, pParent)
	, m_bInitOver(FALSE)
{

}

CVideoPlayerDlg::~CVideoPlayerDlg()
{
}

void CVideoPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PLAYER, m_player);
}


BEGIN_MESSAGE_MAP(CVideoPlayerDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CVideoPlayerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CVideoPlayerDlg::OnBnClickedCancel)
	ON_WM_MOVE()
	ON_MESSAGE(WM_INVERSIONCONTROL, &CVideoPlayerDlg::OnInversioncontrol)
END_MESSAGE_MAP()


// CVideoPlayerDlg message handlers


BOOL CVideoPlayerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	//CRect rc;
	//GetWindowRect(rc);
	//HMONITOR hMonitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
	//HMONITOR hMonitor = MonitorFromRect(rc, MONITOR_DEFAULTTONEAREST);
	//MONITORINFO mi = { 0 }; mi.cbSize = sizeof(mi);
	//if (GetMonitorInfo(hMonitor, &mi)) {
	//	//m_player.SetMonitorRect(mi.rcMonitor);
	//} else {
	//	DWORD e = GetLastError();
	//	LOG(L"GetMonitorInfo failed %d\n", e);
	//}
	GetWindowRect(m_rcNormal);
	m_player.GetWindowRect(m_rcNormalPlayer);
	LoadPosition();
	
	m_bInitOver = TRUE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CVideoPlayerDlg::LoadPosition()
{
	using namespace tinyxml;
	USES_CONVERSION;
	CString s; s.Format(L"%s\\config", GetModuleFilePath());
	CreateDirectory(s, NULL);
	s += L"\\video.xml";

	TiXmlDocument doc(W2A(s));
	do {
		if (!doc.LoadFile()) {
			break;
		}

		TiXmlElement* root = doc.RootElement();
		if (!root)
			break;

		TiXmlElement* rc = root->FirstChildElement("rc");
		if (!rc)
			break;

		const char* sl = NULL;
		const char* sr = NULL;
		const char* st = NULL;
		const char* sb = NULL;
		const char* sm = NULL;

		sl = rc->Attribute("l");
		sr = rc->Attribute("r");
		st = rc->Attribute("t");
		sb = rc->Attribute("b");
		sm = rc->Attribute("m");

		int l, r, t, b, m;
		l = r = t = b = m = 0;
		if (sl)
			l = atoi(sl);
		if (sr)
			r = atoi(sr);
		if (st)
			t = atoi(st);
		if (sb)
			b = atoi(sb);
		if (sm)
			m = atoi(sm);

		CRect rect(l, t, r, b);
		if (rect.IsRectNull() || rect.IsRectEmpty()) {
			break;
		}

		MoveWindow(rect);

		//if (m) {
			//ShowWindow(SW_SHOWMAXIMIZED);
			m_player.SetMaximized(m);
			OnInversioncontrol(m, 0);
		//}
	} while (0);
}


void CVideoPlayerDlg::SavePosition()
{
	using namespace tinyxml;
	USES_CONVERSION;
	CString s; s.Format(L"%s\\config", GetModuleFilePath());
	CreateDirectory(s, NULL);
	s += L"\\video.xml";

	CRect rect;
	GetWindowRect(rect);

	TiXmlDocument doc;
	TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "", "");
	doc.LinkEndChild(decl);
	TiXmlElement *root = new TiXmlElement("VideoConfig"); // 不能有空白符
	doc.LinkEndChild(root);

	TiXmlElement* rc = new TiXmlElement("rc"); // 不能有空白符
	rc->SetAttribute("l", rect.left);
	rc->SetAttribute("r", rect.right);
	rc->SetAttribute("t", rect.top);
	rc->SetAttribute("b", rect.bottom);
	rc->SetAttribute("m", m_player.GetMaximized());
	root->LinkEndChild(rc);

	doc.SaveFile(W2A(s));
}

void CVideoPlayerDlg::OnBnClickedOk()
{
}


void CVideoPlayerDlg::OnBnClickedCancel()
{
}


void CVideoPlayerDlg::OnMove(int x, int y)
{
	CDialogEx::OnMove(x, y);

	if (m_bInitOver) {
		GetWindowRect(m_rcNormal);
		m_player.GetWindowRect(m_rcNormalPlayer);
		SavePosition();
	}
}


afx_msg LRESULT CVideoPlayerDlg::OnInversioncontrol(WPARAM wParam, LPARAM /*lParam*/)
{
	return 0;
	if (m_bInitOver) {
		//BOOL bMax = m_player.GetMaximized();
		//bMax = !bMax;
		//m_player.SetMaximized(bMax);
		BOOL bMax = static_cast<BOOL>(wParam);
		if (bMax) {
			HMONITOR hMonitor = MonitorFromWindow(GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
			MONITORINFO mi = { 0 }; mi.cbSize = sizeof(mi);
			GetMonitorInfo(hMonitor, &mi);
			//m_player.SetMonitorRect(mi.rcMonitor);
			
			MoveWindow(&mi.rcMonitor);
			CRect rc;
			GetClientRect(rc);
			//ClientToScreen(rc);
			m_player.MoveWindow(rc);
		} else {
			MoveWindow(m_rcNormal);
			m_player.MoveWindow(m_rcNormalPlayer);
		}
		
		SavePosition();
	}
	return 0;
}
