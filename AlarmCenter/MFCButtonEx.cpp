// MFCButtonEx.cpp : implementation file
//

#include "stdafx.h"
#include "MFCButtonEx.h"

//namespace gui {
//namespace control {

// CMFCButtonEx

IMPLEMENT_DYNAMIC(CMFCButtonEx, CMFCButton)

CMFCButtonEx::CMFCButtonEx()
	: m_bRbtnDown(FALSE)
	, _buttonCb(nullptr)
	, _udata(nullptr)
	, m_clrFace(RGB(255, 255, 255))
	, m_clrText(RGB(0, 0, 0))
	, m_timerData(nullptr)
	, m_timerCB(nullptr)
{

}

CMFCButtonEx::~CMFCButtonEx()
{
}


BEGIN_MESSAGE_MAP(CMFCButtonEx, CMFCButton)
	ON_CONTROL_REFLECT(BN_CLICKED, &CMFCButtonEx::OnBnClicked)
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_CONTROL_REFLECT(BN_KILLFOCUS, &CMFCButtonEx::OnBnKillfocus)
	ON_WM_TIMER()
	ON_CONTROL_REFLECT(BN_SETFOCUS, &CMFCButtonEx::OnBnSetfocus)
	ON_WM_SETFOCUS()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()



// CMFCButtonEx message handlers




void CMFCButtonEx::OnBnClicked()
{
	m_bRbtnDown = FALSE;
	CString txt;
	GetWindowText(txt);
	JLOG(L"CMFCButtonEx::OnBnClicked() %s\n", txt);

	if (_buttonCb) {
		_buttonCb(BM_LEFT, _udata);
	}
}


void CMFCButtonEx::OnRButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();
	m_bRbtnDown = TRUE;
	CString txt;
	GetWindowText(txt);
	JLOG(L"CMFCButtonEx::OnRButtonDown() %s\n", txt);
	CMFCButton::OnRButtonDown(nFlags, point);
}


void CMFCButtonEx::OnRButtonUp(UINT nFlags, CPoint point)
{
	CMFCButton::OnRButtonUp(nFlags, point);

	if (m_bRbtnDown) {
		m_bRbtnDown = FALSE;
		CString txt;
		GetWindowText(txt);
		JLOG(L"CMFCButtonEx::OnRButtonUp() %s\n", txt);
		if (_buttonCb) {
			_buttonCb(BM_RIGHT, _udata);
		}
	}
	
}


void CMFCButtonEx::OnBnKillfocus()
{
	m_bRbtnDown = FALSE;
}


void CMFCButtonEx::SetTimerEx(UINT nTimerID, void* udata, TimerProcEx cb)
{
	m_timerCB = cb;
	m_timerData = udata;

	auto_timer timer(m_hWnd, nTimerID, 1000);
}


void CMFCButtonEx::KillTimerEx(UINT nTimerID)
{
	KillTimer(nTimerID);
}

//HBRUSH CMFCButtonEx::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
//{
//	HBRUSH hbr = CButton::OnCtlColor(pDC, pWnd, nCtlColor);
//	//pDC->SetTextColor(m_clrText);
//	//pDC->SetBkColor(m_clrFace);
//	return hbr;
//}

//;
//;


void CMFCButtonEx::OnTimer(UINT_PTR nIDEvent)
{
	if (m_timerCB) {
		m_timerCB(m_timerData, nIDEvent);
	}
	CMFCButton::OnTimer(nIDEvent);
}


void CMFCButtonEx::OnBnSetfocus()
{
	Invalidate();
}


void CMFCButtonEx::OnSetFocus(CWnd* pOldWnd)
{
	CMFCButton::OnSetFocus(pOldWnd);

	Invalidate();
}


void CMFCButtonEx::OnMouseMove(UINT nFlags, CPoint point)
{
	if (_buttonCb) {
		_buttonCb(BM_MOVE, _udata);
	}
	CMFCButton::OnMouseMove(nFlags, point);
}
