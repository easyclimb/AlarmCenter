// VideoPlayerCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "VideoPlayerCtrl.h"


// CVideoPlayerCtrl

IMPLEMENT_DYNAMIC(CVideoPlayerCtrl, CStatic)

CVideoPlayerCtrl::CVideoPlayerCtrl()
	: m_device(NULL)
	, m_bPlaying(FALSE)
	, m_bMaximized(FALSE)
	, m_rcNormal()
	, m_rcMonitor()
{

}

CVideoPlayerCtrl::~CVideoPlayerCtrl()
{
}


BEGIN_MESSAGE_MAP(CVideoPlayerCtrl, CStatic)
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()



// CVideoPlayerCtrl message handlers



void CVideoPlayerCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{


	CStatic::OnRButtonUp(nFlags, point);
}


void CVideoPlayerCtrl::SetMaximized(BOOL b)
{
	if (m_bMaximized != b) {
		m_bMaximized = b;
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
	}
}


void CVideoPlayerCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	//SetMaximized(!m_bMaximized);
	CWnd* parent = GetParent();
	if (parent) {
		// 1 for fullscreen, 0 for normal
		parent->SendMessage(WM_INVERSIONCONTROL);
	}

	CStatic::OnLButtonDblClk(nFlags, point);
}
