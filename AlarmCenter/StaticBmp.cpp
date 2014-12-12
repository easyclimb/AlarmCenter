// StaticBmp.cpp : implementation file
//

#include "stdafx.h"
#include "alarmcenter.h"
#include "StaticBmp.h"
//#include "D:/Global/FileOper.h"
#include "BmpEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStaticBmp

CStaticBmp::CStaticBmp()
: m_hBitmap(NULL)
, m_bmpPath(_T(""))
{}

CStaticBmp::~CStaticBmp()
{

}


BEGIN_MESSAGE_MAP(CStaticBmp, CStatic)
	//{{AFX_MSG_MAP(CStaticBmp)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStaticBmp message handlers

void CStaticBmp::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	if (!m_bmpPath.IsEmpty() && CFileOper::PathExists(m_bmpPath)) {
		if (m_hBitmap)
			DeleteObject(m_hBitmap);
		CRect rc;
		GetClientRect(rc);
		m_hBitmap = CBmpEx::GetHBitmapThumbnail(m_bmpPath, rc.Width(), rc.Height());
		if (m_hBitmap) {
			HDC hdcMem;	hdcMem = ::CreateCompatibleDC(dc.m_hDC);	ASSERT(hdcMem);
			HGDIOBJ pOld = ::SelectObject(hdcMem, m_hBitmap);
			SetStretchBltMode(dc.m_hDC, HALFTONE);
			::StretchBlt(dc.m_hDC, rc.left, rc.top, rc.Width(), rc.Height(),
						 hdcMem, 0, 0, rc.Width(), rc.Height(), SRCCOPY);
			::SelectObject(hdcMem, pOld);		::DeleteDC(hdcMem);
		}
	}
}

void CStaticBmp::ShowBmp(const CString &bmpPath)
{
	m_bmpPath = bmpPath;
	Invalidate(0);
}

void CStaticBmp::OnDestroy()
{
	CStatic::OnDestroy();

	if (m_hBitmap)
		DeleteObject(m_hBitmap);
}
