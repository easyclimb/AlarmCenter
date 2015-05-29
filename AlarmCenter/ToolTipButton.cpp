// ToolTipButton.cpp : implementation file
//

#include "stdafx.h"
#include "ToolTipButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


namespace gui {namespace control {
/////////////////////////////////////////////////////////////////////////////
// CToolTipButton

CToolTipButton::CToolTipButton()
{
	m_ToolTip.m_hWnd = NULL;
	m_bMouseIn = FALSE;
}

CToolTipButton::~CToolTipButton()
{}


BEGIN_MESSAGE_MAP(CToolTipButton, CButton)
	//{{AFX_MSG_MAP(CToolTipButton)
	// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CToolTipButton message handlers

BOOL CToolTipButton::PreTranslateMessage(MSG* pMsg)
{
	InitToolTip();
	m_ToolTip.RelayEvent(pMsg);
	return CButton::PreTranslateMessage(pMsg);
}

void CToolTipButton::InitToolTip()
{
	if (m_ToolTip.m_hWnd == NULL) {
		// Create ToolTip control
		m_ToolTip.Create(this, TTS_ALWAYSTIP);
		// Create inactive
		m_ToolTip.Activate(FALSE);
		// Enable multiline
		m_ToolTip.SendMessage(TTM_SETMAXTIPWIDTH, 0, 400);
		//m_ToolTip.SendMessage(TTM_SETTITLE, TTI_INFO, (LPARAM)_T("Title"));
	} // if
} // End of InitToolTip

// This function sets the text to show in the button tooltip.
//
// Parameters:
//		[IN]	nText
//				ID number of the string resource containing the text to show.
//		[IN]	bActivate
//				If TRUE the tooltip will be created active.
//
void CToolTipButton::SetTooltipText(int nText, BOOL bActivate)
{
	CString sText;

	// Load string resource
	sText.LoadString(nText);
	// If string resource is not empty
	if (sText.IsEmpty() == FALSE) SetTooltipText((LPCTSTR)sText, bActivate);
} // End of SetTooltipText

// This function sets the text to show in the button tooltip.
//
// Parameters:
//		[IN]	lpszText
//				Pointer to a null-terminated string containing the text to show.
//		[IN]	bActivate
//				If TRUE the tooltip will be created active.
//
void CToolTipButton::SetTooltipText(LPCTSTR lpszText, BOOL bActivate)
{
	// We cannot accept NULL pointer
	if (lpszText == NULL) return;

	// Initialize ToolTip
	InitToolTip();

	// If there is no tooltip defined then add it
	if (m_ToolTip.GetToolCount() == 0) {
		CRect rectBtn;
		GetClientRect(rectBtn);
		m_ToolTip.AddTool(this, lpszText, rectBtn, 1);
	} // if

	// Set text for tooltip
	m_ToolTip.UpdateTipText(lpszText, this, 1);
	m_ToolTip.Activate(bActivate);
} // End of SetTooltipText


void CToolTipButton::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!m_bMouseIn) {
		TRACKMOUSEEVENT csTME;
		csTME.cbSize = sizeof (csTME);
		csTME.dwFlags = TME_LEAVE | TME_HOVER;
		csTME.hwndTrack = m_hWnd;// 指定要 追踪 的窗口 
		csTME.dwHoverTime = 10;  // 鼠标在按钮上停留超过 10ms ，才认为状态为 HOVER
		::_TrackMouseEvent(&csTME);
		m_bMouseIn = TRUE;
		m_ToolTip.Popup();
	}

	CButton::OnMouseMove(nFlags, point);
}


void CToolTipButton::OnMouseLeave()
{
	m_bMouseIn = FALSE;

	CButton::OnMouseLeave();
}

NAMESPACE_END
NAMESPACE_END
