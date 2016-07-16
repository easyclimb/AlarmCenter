// StaticBmp.cpp : implementation file
//

#include "stdafx.h"
#include "StaticBmp.h"
#include "BmpEx.h"

namespace gui
{
	namespace control
	{
		CStaticBmp::CStaticBmp()
			: m_hBitmap(nullptr)
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
			CRect rc;
			GetClientRect(rc);

			if (!m_bmpPath.IsEmpty()) {
				if (updated_) {
					if (m_hBitmap)
						DeleteObject(m_hBitmap);
					m_hBitmap = (HBITMAP)::LoadImage(0, m_bmpPath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
					updated_ = false;
				}
				//m_hBitmap = CBmpEx::GetHBitmapThumbnail(m_bmpPath, rc.Width(), rc.Height());
				
				if (m_hBitmap) {
					HDC hdcMem;	hdcMem = ::CreateCompatibleDC(dc.m_hDC);	ASSERT(hdcMem);
					HGDIOBJ pOld = ::SelectObject(hdcMem, m_hBitmap);
					SetStretchBltMode(dc.m_hDC, HALFTONE);
					::TransparentBlt(dc.m_hDC, rc.left, rc.top, rc.Width(), rc.Height(),
								 hdcMem, 0, 0, rc.Width(), rc.Height(), RGB(255, 255, 255));
					::SelectObject(hdcMem, pOld);		::DeleteDC(hdcMem);
				}
			} else {
				CBrush brush;
				brush.Attach(GetStockObject(WHITE_BRUSH));
				dc.FillRect(rc, &brush);
			}
		}

		void CStaticBmp::ShowBmp(const CString &bmpPath)
		{
			if (bmpPath != m_bmpPath) {
				m_bmpPath = bmpPath;
				updated_ = true;
				Invalidate(0);
			}
		}

		void CStaticBmp::OnDestroy()
		{
			CStatic::OnDestroy();

			if (m_hBitmap)
				DeleteObject(m_hBitmap);
		}

	};
};
