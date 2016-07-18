// VideoPlayerCtrl.cpp : implementation file
//

#include "stdafx.h"
//#include "AlarmCenter.h"
#include "VideoPlayerCtrl.h"


// CVideoPlayerCtrl

IMPLEMENT_DYNAMIC(CVideoPlayerCtrl, CStatic)

CVideoPlayerCtrl::CVideoPlayerCtrl()
	: m_bPlaying(FALSE)
{

}

CVideoPlayerCtrl::~CVideoPlayerCtrl()
{
}


BEGIN_MESSAGE_MAP(CVideoPlayerCtrl, CStatic)
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_SHOWWINDOW()
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_MOVE()
END_MESSAGE_MAP()



// CVideoPlayerCtrl message handlers



void CVideoPlayerCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{


	CStatic::OnRButtonUp(nFlags, point);
}


//void CVideoPlayerCtrl::SetMaximized(BOOL b)
//{
	//if (m_bMaximized != b) {
	//	m_bMaximized = b;
		//if (b) {
		//	GetWindowRect(m_rcNormal);
		//	//MoveWindow(m_rcMonitor);
		//	//ShowWindow(SW_MAXIMIZE);
		//	SetWindowPos(&CWnd::wndTopMost, m_rcMonitor.left, m_rcMonitor.top, m_rcMonitor.Width(), m_rcMonitor.Height(), SWP_SHOWWINDOW);
		//} else {
		//	//MoveWindow(m_rcNormal);
		//	//ShowWindow(SW_SHOWNORMAL);
		//	SetWindowPos(&CWnd::wndTopMost, m_rcNormal.left, m_rcNormal.top, m_rcNormal.Width(), m_rcNormal.Height(), SWP_SHOWWINDOW);
		//}
	//}
//}


void CVideoPlayerCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	//SetMaximized(!m_bMaximized);
	CWnd* parent = GetParent();
	if (parent) {
		// 1 for fullscreen, 0 for normal
		parent->SendMessage(WM_INVERSIONCONTROL, 0);
	}

	CStatic::OnLButtonDblClk(nFlags, point);
}


void CVideoPlayerCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	focused_ = !focused_;
	JLOG(L"video player ctrl #%d, focused_ %d", ndx_, focused_);

	CRect rc;
	GetClientRect(rc);
	rc.DeflateRect(1, 1, 1, 1);
	Invalidate();
	//auto dc = GetDC();
	//if (focused_) {
	//	//dc->Draw3dRect(rc, RGB(0, 0, 255), RGB(0, 255, 0));
	//	dc->DrawFocusRect(rc);
	//} else {
	//	dc->DrawFocusRect(rc);
	//}

	CStatic::OnLButtonDown(nFlags, point);
}


void CVideoPlayerCtrl::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CStatic::OnShowWindow(bShow, nStatus);

	//if (bShow && real_player) {
	//	CRect rc;
	//	GetClientRect(rc);
	//	rc.DeflateRect(1, 1, 1, 1);

	//	real_player->MoveWindow(rc);
	//	auto dc = GetDC();
	//	//dc->Draw3dRect(rc, RGB(0, 0, 255), RGB(0, 255, 0));
	//	dc->DrawFocusRect(rc);
	//}
}


int CVideoPlayerCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CStatic::OnCreate(lpCreateStruct) == -1)
		return -1;

	brush_.CreateSolidBrush(RGB(100, 100, 100));

	//CRect rc;
	//rc.left = lpCreateStruct->x;
	//rc.top = lpCreateStruct->y;
	//rc.right = rc.left + lpCreateStruct->cx;
	//rc.bottom = rc.top + lpCreateStruct->cy;

	////GetClientRect(rc);
	//rc.DeflateRect(2, 2, 2, 2);

	CRect rc;
	GetClientRect(rc);
	rc.DeflateRect(1, 1, 1, 1);

	real_player = new CStatic();
	real_player->Create(L"", WS_CHILD | WS_VISIBLE, rc, this);
	real_player->ShowWindow(SW_SHOW);

	return 0;
}


HWND CVideoPlayerCtrl::GetRealHwnd() const
{
	if (real_player) {
		return real_player->GetSafeHwnd();
	} else {
		return GetSafeHwnd();
	}
}


CRect CVideoPlayerCtrl::GetRealRect() const {
	AUTO_LOG_FUNCTION;
	CRect rc = { 0 };
	GetClientRect(rc);
	rc.DeflateRect(1, 1, 1, 1);

	//if (real_player) {
	//	real_player->MoveWindow(rc);
	//	real_player->GetClientRect(&rc);
	//	real_player->ClientToScreen(&rc);
	//}
	ClientToScreen(rc);
	
	JLOGA("%d %d %d %d %dX%d", rc.left, rc.top, rc.right, rc.bottom, rc.Width(), rc.Height());
	return rc;
}

void CVideoPlayerCtrl::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   
	CRect rc;
	GetClientRect(rc);
	//rc.DeflateRect(1, 1, 1, 1);

	//dc.Rectangle(rc);

	dc.FillRect(rc, &brush_);

	if (focused_) {
		dc.Draw3dRect(rc, RGB(255, 50, 50), RGB(255, 50, 50));
		//dc.DrawFocusRect(rc);
	} else {
		//dc.DrawFocusRect(rc);
	}
}


void CVideoPlayerCtrl::OnDestroy()
{
	CStatic::OnDestroy();

	SAFEDELETEDLG(real_player);
	brush_.DeleteObject();
}


void CVideoPlayerCtrl::OnSize(UINT nType, int cx, int cy)
{
	CStatic::OnSize(nType, cx, cy);

	if (real_player) {
		CRect rc;
		GetClientRect(rc);
		rc.DeflateRect(1, 1, 1, 1);

		real_player->MoveWindow(rc);
	}

}


void CVideoPlayerCtrl::OnMove(int x, int y)
{
	CStatic::OnMove(x, y);

	if (real_player) {
		CRect rc;
		GetClientRect(rc);
		rc.DeflateRect(1, 1, 1, 1);

		real_player->MoveWindow(rc);
	}
}
