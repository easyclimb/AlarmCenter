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
	, _buttonCb(NULL)
	, _udata(NULL)
	, m_clrFace(RGB(255, 255, 255))
	, m_clrText(RGB(0, 0, 0))
	, m_timerData(NULL)
	, m_timerCB(NULL)
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
END_MESSAGE_MAP()



// CMFCButtonEx message handlers




void CMFCButtonEx::OnBnClicked()
{
	m_bRbtnDown = FALSE;
	CString txt;
	GetWindowText(txt);
	LOG(L"CMFCButtonEx::OnBnClicked() %s\n", txt);

	if (_buttonCb) {
		_buttonCb(BC_LEFT, _udata);
	}
}


void CMFCButtonEx::OnRButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();
	m_bRbtnDown = TRUE;
	CString txt;
	GetWindowText(txt);
	LOG(L"CMFCButtonEx::OnRButtonDown() %s\n", txt);
	CMFCButton::OnRButtonDown(nFlags, point);
}


void CMFCButtonEx::OnRButtonUp(UINT nFlags, CPoint point)
{
	CMFCButton::OnRButtonUp(nFlags, point);

	if (m_bRbtnDown) {
		m_bRbtnDown = FALSE;
		CString txt;
		GetWindowText(txt);
		LOG(L"CMFCButtonEx::OnRButtonUp() %s\n", txt);
		if (_buttonCb) {
			_buttonCb(BC_RIGHT, _udata);
		}
	}
	
}


void CMFCButtonEx::OnBnKillfocus()
{
	m_bRbtnDown = FALSE;
}


void CMFCButtonEx::SetTimerEx(void* udata, TimerProcEx cb)
{
	m_timerCB = cb;
	m_timerData = udata;

	KillTimer(1);
	SetTimer(1, 1000, NULL);
}


void CMFCButtonEx::KillTimerEx()
{
	KillTimer(1);
}

//HBRUSH CMFCButtonEx::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
//{
//	HBRUSH hbr = CButton::OnCtlColor(pDC, pWnd, nCtlColor);
//	//pDC->SetTextColor(m_clrText);
//	//pDC->SetBkColor(m_clrFace);
//	return hbr;
//}

//NAMESPACE_END
//NAMESPACE_END


void CMFCButtonEx::OnTimer(UINT_PTR nIDEvent)
{
	if (m_timerCB) {
		m_timerCB(m_timerData);
	}
	CMFCButton::OnTimer(nIDEvent);
}
