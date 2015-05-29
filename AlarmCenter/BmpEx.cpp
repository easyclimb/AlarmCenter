// BmpEx.cpp: implementation of the CBmpEx class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BmpEx.h"
#include "dib.h"
#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace gui
{
	namespace control
	{
		CBmpEx::CBmpEx()
		{

		}

		CBmpEx::~CBmpEx()
		{

		}

		HBITMAP CBmpEx::GetHBitmapThumbnail(const CString& bmpPath, int width, int height)
		{
			HBITMAP hBitmap = NULL;

			CFile file; CFileException error;
			if (!file.Open(bmpPath, CFile::modeRead, &error)) {
				error.ReportError();
				return NULL;
			}

			CDib dib;
			dib.Read(file); file.Close();

			int nWidth = dib.m_pBMI->bmiHeader.biWidth;
			int nHeight = dib.m_pBMI->bmiHeader.biHeight;

			nWidth = abs(nWidth);
			nHeight = abs(nHeight);

			if (width != 0) {
				if (height == 0) {
					height = width * nHeight / nWidth;
				}
			} else {
				width = nWidth;
				height = nHeight;
			}

			dib.m_pBMI->bmiHeader.biWidth = width;
			dib.m_pBMI->bmiHeader.biHeight = height;

			// create thumbnail bitmap section
			hBitmap = ::CreateDIBSection(NULL,
										 dib.m_pBMI,
										 DIB_RGB_COLORS,
										 NULL,
										 NULL,
										 0);

			// restore dib header
			dib.m_pBMI->bmiHeader.biWidth = nWidth;
			dib.m_pBMI->bmiHeader.biHeight = nHeight;

			// select thumbnail bitmap into screen dc
			HDC hMemDC = ::CreateCompatibleDC(NULL);
			HGDIOBJ hOldObj = ::SelectObject(hMemDC, hBitmap);

			// grayscale image, need palette
			HPALETTE hPal = NULL;
			if (dib.m_pPalette != NULL) {
				hPal = ::SelectPalette(hMemDC, (HPALETTE)dib.m_pPalette->GetSafeHandle(), FALSE);
				::RealizePalette(hMemDC);
			}

			// set stretch mode
			::SetStretchBltMode(hMemDC, COLORONCOLOR);

			// populate the thumbnail bitmap bits
			::StretchDIBits(hMemDC, 0, 0,
							width,
							height,
							0, 0,
							dib.m_pBMI->bmiHeader.biWidth,
							dib.m_pBMI->bmiHeader.biHeight,
							dib.m_pBits,
							dib.m_pBMI,
							DIB_RGB_COLORS,
							SRCCOPY);

			// restore DC object
			::SelectObject(hMemDC, hOldObj);

			// restore DC palette
			if (dib.m_pPalette != NULL)
				::SelectPalette(hMemDC, (HPALETTE)hPal, FALSE);

			// clean up
			::DeleteObject(hMemDC);

			return hBitmap;
		}

	};
};


