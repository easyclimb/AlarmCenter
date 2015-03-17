// ListBoxEx.cpp : implementation file
//

#include "stdafx.h"
#include "ListBoxEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CListBoxEx
namespace gui {
namespace control {


CListBoxEx::CListBoxEx()
{
	m_ToolTip.m_hWnd = NULL;
	m_nLastHit = 0;
}

CListBoxEx::~CListBoxEx()
{
}


BEGIN_MESSAGE_MAP(CListBoxEx, CListBox)
	//{{AFX_MSG_MAP(CListBoxEx)
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CListBoxEx message handlers

void CListBoxEx::OnMouseMove(UINT nFlags, CPoint point) 
{
	BOOL bOutSide = FALSE;
	int index = this->ItemFromPoint(point, bOutSide);
	if(index >= 0 && index < GetCount())
	{
		if(index != m_nLastHit)
		{
			CString txt;
			GetText(index, txt);
			SetTooltipText(txt, 1);
			m_nLastHit = index;
		}
	}
	CListBox::OnMouseMove(nFlags, point);
}

void CListBoxEx::InitToolTip()
{
	if (m_ToolTip.m_hWnd == NULL)
	{
		m_ToolTip.Create(this, TTS_ALWAYSTIP);
		m_ToolTip.Activate(FALSE);
		m_ToolTip.SendMessage(TTM_SETMAXTIPWIDTH, 0, 400);
	}
}

void CListBoxEx::SetTooltipText(int nText, BOOL bActivate)
{
	CString sText;
	sText.LoadString(nText);
	if (sText.IsEmpty() == FALSE) SetTooltipText((LPCTSTR)sText, bActivate);
}

void CListBoxEx::SetTooltipText(LPCTSTR lpszText, BOOL bActivate)
{
	if (lpszText == NULL) return;
	InitToolTip();
	if (m_ToolTip.GetToolCount() == 0)
	{
		CRect rectBtn; 
		GetClientRect(rectBtn);
		m_ToolTip.AddTool(this, lpszText, rectBtn, 1);
	}

	m_ToolTip.UpdateTipText(lpszText, this, 1);
	m_ToolTip.Activate(bActivate);
}

BOOL CListBoxEx::PreTranslateMessage(MSG* pMsg) 
{
	InitToolTip();
	m_ToolTip.RelayEvent(pMsg);
	return CListBox::PreTranslateMessage(pMsg);
}

NAMESPACE_END
NAMESPACE_END
