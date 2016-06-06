// TreeCtrlWithColorAndFont.cpp : implementation file
//

#include "stdafx.h"
//#include "AlarmCenter.h"
#include "TreeCtrlWithColorAndFont.h"


// CTreeCtrlWithColorAndFont

IMPLEMENT_DYNAMIC(CTreeCtrlWithColorAndFont, CTreeCtrl)

CTreeCtrlWithColorAndFont::CTreeCtrlWithColorAndFont()
{

}

CTreeCtrlWithColorAndFont::~CTreeCtrlWithColorAndFont()
{
}


BEGIN_MESSAGE_MAP(CTreeCtrlWithColorAndFont, CTreeCtrl)
	ON_WM_PAINT()
END_MESSAGE_MAP()



// CTreeCtrlWithColorAndFont message handlers




void CTreeCtrlWithColorAndFont::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // Do not call CTreeCtrl::OnPaint() for painting messages

					   // Create a memory DC compatible with the paint DC
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);

	CRect rcClip, rcClient;
	dc.GetClipBox(&rcClip);
	GetClientRect(&rcClient);

	// Select a compatible bitmap into the memory DC
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(&dc, rcClient.Width(), rcClient.Height());
	memDC.SelectObject(&bitmap);

	// Set clip region to be same as that in paint DC
	CRgn rgn;
	rgn.CreateRectRgnIndirect(&rcClip);
	memDC.SelectClipRgn(&rgn);
	rgn.DeleteObject();

	// First let the control do its default drawing.
	CWnd::DefWindowProc(WM_PAINT, (WPARAM)memDC.m_hDC, 0);

	HTREEITEM hItem = GetFirstVisibleItem();

	int n = GetVisibleCount() + 1;
	while (hItem && n--) {
		CRect rect;

		// Do not meddle with selected items or drop highlighted items
		UINT selflag = TVIS_DROPHILITED | TVIS_SELECTED;
		ColorAndFontPtr cf; 
		auto iter = m_itemMap.find(hItem);

		if (!(GetItemState(hItem, selflag) & selflag)
			&& iter != m_itemMap.end()) {
			cf = iter->second;
			CFont *pFontDC;
			CFont fontDC;
			LOGFONT logfont;

			if (cf->font.lfFaceName[0] != 0) {
				logfont = cf->font;
			} else {
				// No font specified, so use window font
				CFont *pFont = GetFont();
				pFont->GetLogFont(&logfont);
			}

			if (GetItemBold(hItem))
				logfont.lfWeight = 700;
			fontDC.CreateFontIndirect(&logfont);
			pFontDC = memDC.SelectObject(&fontDC);

			if (cf->color != (COLORREF)-1)
				memDC.SetTextColor(cf->color);

			CString sItem = GetItemText(hItem);

			GetItemRect(hItem, &rect, TRUE);
			memDC.SetBkColor(GetSysColor(COLOR_WINDOW));
			memDC.TextOut(rect.left + 2, rect.top + 1, sItem);

			memDC.SelectObject(pFontDC);
		}
		hItem = GetNextVisibleItem(hItem);
	}


	dc.BitBlt(rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height(), &memDC,
			  rcClip.left, rcClip.top, SRCCOPY);
}
