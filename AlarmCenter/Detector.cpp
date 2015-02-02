// Detector.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "Detector.h"
#include "aarot.hpp"
#include "BmpEx.h"
#include "Coordinate.h"
#include "ZoneInfo.h"
#include "ZonePropertyInfo.h"
#include "DetectorInfo.h"
#include "DetectorLib.h"
using namespace core;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace gui
{

static HBITMAP GetRotatedBitmapNT(HBITMAP hBitmap, float radians, COLORREF clrBack);
static HANDLE GetRotatedBitmap(HANDLE hDIB, float radians, COLORREF clrBack);
/////////////////////////////////////////////////////////////////////////////
// CDetector
//BOOL CDetector::m_bCurColorRed = FALSE;

CDetector::CDetector(CZoneInfo* zoneInfo, CWnd* parentWnd, BOOL bMainDetector)
	: m_pPairDetector(NULL)
	, m_hRgn(NULL)
	//, m_hRgnRotated(NULL)
	, m_hBitmap(NULL)
	, m_hBitmapRotated(NULL)
	, m_zoneInfo(NULL)
	, m_detectorInfo(NULL)
	, m_detectorLibData(NULL)
	, m_bFocused(FALSE)
	, m_bManualRotate(TRUE)
	, m_bAlarming(FALSE)
	, m_bCurColorRed(FALSE)
	, m_TimerIDRepaint(1)
	, m_TimerIDAlarm(2)
	, m_hBrushFocused(NULL)
	, m_hBrushAlarmed(NULL)
	, m_bAntlineGenerated(FALSE)
	, m_pts(NULL)
	, m_bNeedRecalcPts(FALSE)
	, m_parentWnd(parentWnd)
	, m_bMainDetector(bMainDetector)
	, m_bMouseIn(FALSE)
{
	ASSERT(zoneInfo);
	m_zoneInfo = zoneInfo;
	m_detectorInfo = m_zoneInfo->GetDetectorInfo();
	CDetectorLib* lib = CDetectorLib::GetInstance();
	const CDetectorLibData* data = lib->GetDetectorLibData(m_detectorInfo->get_detector_lib_id());
	m_detectorLibData = new CDetectorLibData();
	m_detectorLibData->set_antline_gap(data->get_antline_gap());
	m_detectorLibData->set_antline_num(data->get_antline_num());
	m_detectorLibData->set_detector_name(data->get_detector_name());
	m_detectorLibData->set_id(data->get_id());
	m_detectorLibData->set_path(data->get_path());
	m_detectorLibData->set_path_pair(data->get_path_pair());
	m_detectorLibData->set_type(data->get_type());

	InitializeCriticalSection(&m_cs);
}

CDetector::~CDetector()
{
	delete m_detectorLibData;
	DeleteCriticalSection(&m_cs);
}

BEGIN_MESSAGE_MAP(CDetector, CButton)
	//{{AFX_MSG_MAP(CDetector)
	ON_WM_LBUTTONDOWN()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_WM_CREATE()
	ON_WM_MOUSELEAVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDetector message handlers
//CPoint pt(10,10);

BOOL CDetector::CreateDetector()
{
	static int width = (int)(DETECTORWIDTH * 1.5);
	static int height = (int)(DETECTORWIDTH * 1.5);

	ASSERT(m_detectorInfo);
	CRect rc;

	rc.left = m_detectorInfo->get_x();
	rc.top = m_detectorInfo->get_y();
	rc.right = rc.left + width;
	rc.bottom = rc.top + height;

	BOOL ok = TRUE;

	do {
#ifdef _DEBUG
		static int i = 0;
		CString txt = _T("");
		txt.Format(_T("Num %d"), i++);
		ok = Create(txt, WS_CHILD | WS_VISIBLE, rc, m_parentWnd, 0);
#else 
		ok = Create(NULL, WS_CHILD | WS_VISIBLE, rc, m_parentWnd, 0);
#endif
		if (!ok) { break; }
		if (!m_bMainDetector) { break; }
		if (m_detectorLibData->get_type() != DT_DOUBLE) { break; }

		m_pPairDetector = new CDetector(m_zoneInfo, m_parentWnd, FALSE);
		CPoint ptRtd = control::CCoordinate::GetRotatedPoint(CPoint(rc.left, rc.top),
															 m_detectorInfo->get_distance(),
															 -m_detectorInfo->get_angle());
		rc.left = ptRtd.x;
		rc.top = ptRtd.y;
		rc.right = rc.left + width;
		rc.bottom = rc.top + height;

		ok = m_pPairDetector->Create(NULL, WS_CHILD | WS_VISIBLE, rc, m_parentWnd, 0);
		if (!ok) { break; }

	} while (0);

	return ok;
}

static void SetPixelEx(CDC *pDC, CPoint pt, COLORREF clr)
{
	int i = 0;
	CPoint pts[8];
	for (i = 0; i < 8; i++)
		pts[i] = pt;

	pts[0].x = pt.x - 1;
	pts[0].y = pt.y - 1;

	pts[1].x = pt.x;
	pts[1].y = pt.y - 1;

	pts[2].x = pt.x + 1;
	pts[2].y = pt.y - 1;

	pts[3].x = pt.x - 1;
	pts[3].y = pt.y;

	pts[4].x = pt.x + 1;
	pts[4].y = pt.y;

	pts[5].x = pt.x - 1;
	pts[5].y = pt.y + 1;

	pts[6].x = pt.x;
	pts[6].y = pt.y + 1;

	pts[7].x = pt.x + 1;
	pts[7].y = pt.y + 1;

	pDC->SetPixel(pt, clr);
	for (i = 0; i < 8; i++)
		pDC->SetPixel(pts[i], clr);
}

void CDetector::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CLocalLock lock(&m_cs);
	if (m_hBitmap == NULL)
		return;

	if (m_bManualRotate) {
		m_bManualRotate = FALSE;

		if (m_detectorInfo->get_angle() % 360 == 0) {
			if (m_hBitmapRotated != m_hBitmap) {
				if (m_hBitmapRotated)
					DeleteObject(m_hBitmapRotated);
				m_hBitmapRotated = m_hBitmap;
			}
		} else {
			if (m_hBitmapRotated != m_hBitmap) {
				if (m_hBitmapRotated)
					DeleteObject(m_hBitmapRotated);
			}
			m_hBitmapRotated = control::aarot::rotate(m_hBitmap,
													  m_detectorInfo->get_angle(),
													  NULL, RGB(255, 255, 255),
													  1, 0, NULL);
		}

		BITMAP bmp;
		GetObject(m_hBitmapRotated, sizeof(bmp), &bmp);
		m_sizeBmp.cx = bmp.bmWidth;
		m_sizeBmp.cy = bmp.bmHeight;
		m_pt.x = m_sizeBmp.cx / 2;
		m_pt.y = m_sizeBmp.cy / 2;
	}

	if (m_hRgn)	::DeleteObject(m_hRgn);
	m_hRgn = BitmapToRegion(m_hBitmapRotated, RGB(255, 255, 255));
	CDC *pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CDC MemDC;	MemDC.CreateCompatibleDC(pDC);
	HGDIOBJ pOld = MemDC.SelectObject(m_hBitmapRotated);

	pDC->BitBlt(0, 0, m_sizeBmp.cx, m_sizeBmp.cy, &MemDC, 0, 0, SRCCOPY);

	if (m_bFocused)
		pDC->FrameRgn(CRgn::FromHandle(m_hRgn), CBrush::FromHandle(m_hBrushFocused), 2, 2);
	else if (m_bAlarming && m_bCurColorRed)
		pDC->FrameRgn(CRgn::FromHandle(m_hRgn), CBrush::FromHandle(m_hBrushAlarmed), 2, 2);

	MemDC.SelectObject(pOld);
	MemDC.DeleteDC();
	SetWindowRgn(m_hRgn, 1);

	if (!m_bAntlineGenerated || m_bNeedRecalcPts) {
		GenerateAntlinePts();
		m_bNeedRecalcPts = FALSE;
	}
}

HRGN CDetector::BitmapToRegion(HBITMAP hBmp, COLORREF cTransparentColor, COLORREF cTolerance)
{
	HRGN hRgn = NULL;

	if (hBmp) {
		// Create a memory DC inside which we will scan the bitmap content
		HDC hMemDC = CreateCompatibleDC(NULL);
		if (hMemDC) {
			// Get bitmap size
			BITMAP bm;
			GetObject(hBmp, sizeof(bm), &bm);

			// Create a 32 bits depth bitmap and select it into the memory DC 
			BITMAPINFOHEADER RGB32BITSBITMAPINFO = {
				sizeof(BITMAPINFOHEADER),	// biSize 
				bm.bmWidth,					// biWidth; 
				bm.bmHeight,				// biHeight; 
				1,							// biPlanes; 
				32,							// biBitCount 
				BI_RGB,						// biCompression; 
				0,							// biSizeImage; 
				0,							// biXPelsPerMeter; 
				0,							// biYPelsPerMeter; 
				0,							// biClrUsed; 
				0							// biClrImportant; 
			};
			VOID * pbits32;
			HBITMAP hbm32 = CreateDIBSection(hMemDC,
												(BITMAPINFO *)&RGB32BITSBITMAPINFO, DIB_RGB_COLORS, &pbits32, NULL, 0);
			if (hbm32) {
				HBITMAP holdBmp = (HBITMAP)SelectObject(hMemDC, hbm32);

				// Create a DC just to copy the bitmap into the memory DC
				HDC hDC = CreateCompatibleDC(hMemDC);
				if (hDC) {
					// Get how many bytes per row we have for the bitmap bits (rounded up to 32 bits)
					BITMAP bm32;
					GetObject(hbm32, sizeof(bm32), &bm32);
					while (bm32.bmWidthBytes % 4)
						bm32.bmWidthBytes++;

					// Copy the bitmap into the memory DC
					HBITMAP holdBmp = (HBITMAP)SelectObject(hDC, hBmp);
					BitBlt(hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, hDC, 0, 0, SRCCOPY);

					// For better performances, we will use the ExtCreateRegion() function to create the
					// region. This function take a RGNDATA structure on entry. We will add rectangles by
					// amount of ALLOC_UNIT number in this structure.
#define ALLOC_UNIT	100
					DWORD maxRects = ALLOC_UNIT;
					HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, sizeof(RGNDATAHEADER)+(sizeof(RECT)* maxRects));
					RGNDATA *pData = (RGNDATA *)GlobalLock(hData);
					pData->rdh.dwSize = sizeof(RGNDATAHEADER);
					pData->rdh.iType = RDH_RECTANGLES;
					pData->rdh.nCount = pData->rdh.nRgnSize = 0;
					SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);

					// Keep on hand highest and lowest values for the "transparent" pixels
					BYTE lr = GetRValue(cTransparentColor);
					BYTE lg = GetGValue(cTransparentColor);
					BYTE lb = GetBValue(cTransparentColor);
					BYTE hr = static_cast<unsigned char>(min(0xff, lr + GetRValue(cTolerance)));
					BYTE hg = static_cast<unsigned char>(min(0xff, lg + GetGValue(cTolerance)));
					BYTE hb = static_cast<unsigned char>(min(0xff, lb + GetBValue(cTolerance)));

					// Scan each bitmap row from bottom to top (the bitmap is inverted vertically)
					BYTE *p32 = (BYTE *)bm32.bmBits + (bm32.bmHeight - 1) * bm32.bmWidthBytes;
					for (int y = 0; y < bm.bmHeight; y++) {
						// Scan each bitmap pixel from left to right
						for (int x = 0; x < bm.bmWidth; x++) {
							// Search for a continuous range of "non transparent pixels"
							int x0 = x;
							LONG *p = (LONG *)p32 + x;
							while (x < bm.bmWidth) {
								BYTE b = GetRValue(*p);
								if (b >= lr && b <= hr) {
									b = GetGValue(*p);
									if (b >= lg && b <= hg) {
										b = GetBValue(*p);
										if (b >= lb && b <= hb)
											// This pixel is "transparent"
											break;
									}
								}
								p++;
								x++;
							}

							if (x > x0) {
								// Add the pixels (x0, y) to (x, y+1) as a new rectangle in the region
								if (pData->rdh.nCount >= maxRects) {
									GlobalUnlock(hData);
									maxRects += ALLOC_UNIT;
									hData = GlobalReAlloc(hData, sizeof(RGNDATAHEADER)+(sizeof(RECT)* maxRects), GMEM_MOVEABLE);
									pData = (RGNDATA *)GlobalLock(hData);
								}
								RECT *pr = (RECT *)&pData->Buffer;
								SetRect(&pr[pData->rdh.nCount], x0, y, x, y + 1);
								if (x0 < pData->rdh.rcBound.left)
									pData->rdh.rcBound.left = x0;
								if (y < pData->rdh.rcBound.top)
									pData->rdh.rcBound.top = y;
								if (x > pData->rdh.rcBound.right)
									pData->rdh.rcBound.right = x;
								if (y + 1 > pData->rdh.rcBound.bottom)
									pData->rdh.rcBound.bottom = y + 1;
								pData->rdh.nCount++;

								// On Windows98, ExtCreateRegion() may fail if the number of rectangles is too
								// large (ie: > 4000). Therefore, we have to create the region by multiple steps.
								if (pData->rdh.nCount == 2000) {
									HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER)+(sizeof(RECT)* maxRects), pData);
									if (hRgn) {
										CombineRgn(hRgn, hRgn, h, RGN_OR);
										DeleteObject(h);
									} else
										hRgn = h;
									pData->rdh.nCount = 0;
									SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);
								}
							}
						}

						// Go to next row (remember, the bitmap is inverted vertically)
						p32 -= bm32.bmWidthBytes;
					}

					// Create or extend the region with the remaining rectangles
					HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER)+(sizeof(RECT)* maxRects), pData);
					if (hRgn) {
						CombineRgn(hRgn, hRgn, h, RGN_OR);
						DeleteObject(h);
					} else
						hRgn = h;

					// Clean up
					GlobalUnlock(hData);
					GlobalFree(hData);
					SelectObject(hDC, holdBmp);
					DeleteDC(hDC);
				}

				DeleteObject(SelectObject(hMemDC, holdBmp));
			}

			DeleteDC(hMemDC);
		}
	}

	return hRgn;

}

void CDetector::PreSubclassWindow()
{
	ModifyStyle(0, BS_OWNERDRAW);
	ModifyStyle(0, WS_EX_TRANSPARENT);
	ModifyStyle(0, WS_CLIPSIBLINGS);
	CButton::PreSubclassWindow();
}

void CDetector::Rotate(int angle)
{
	//if((int)angle % 360 == 0)	return;
	m_bManualRotate = TRUE;
	//KillTimer(m_TimerIDRepaint);
	//SetTimer(m_TimerIDRepaint, 2000, NULL);
	m_detectorInfo->set_angle(angle);
	ReleasePts();
	m_bAntlineGenerated = FALSE;
	Invalidate();
}

int CDetector::GetPtn() const
{
	return m_detectorLibData->get_antline_num();
}

// 由调用者提供和释放pts
void CDetector::GetPts(CPoint* &pts)
{
	CLocalLock lock(&m_cs);
	//memcpy(pts, m_pts, sizeof(CPoint) * m_detectorInfo->antline_num);
	if (m_bNeedRecalcPts) {
		m_bNeedRecalcPts = FALSE;
		GenerateAntlinePts();
	}
	pts = m_pts;
}

void CDetector::OnLButtonDown(UINT nFlags, CPoint point)
{
	CButton::OnLButtonDown(nFlags, point);
}

void CDetector::OnTimer(UINT nIDEvent)
{
	if (m_TimerIDRepaint == nIDEvent) {
		KillTimer(1);
		Invalidate();//Invalidate();
	} else if (m_TimerIDAlarm == nIDEvent) {
		if (this->m_pPairDetector)
			m_pPairDetector->SendMessage(WM_TIMER, m_TimerIDAlarm);
		m_bCurColorRed = !m_bCurColorRed;
		Invalidate(0);
		//InvalidateRgn(CRgn::FromHandle(m_hRgn));
	}
	CButton::OnTimer(nIDEvent);
}

static HBITMAP GetRotatedBitmapNT(HBITMAP hBitmap, float radians, COLORREF clrBack)
{
	// Create a memory DC compatible with the display  
	CDC sourceDC, destDC;
	sourceDC.CreateCompatibleDC(NULL);
	destDC.CreateCompatibleDC(NULL);

	// Get logical coordinates  
	BITMAP bm;
	::GetObject(hBitmap, sizeof(bm), &bm);

	float cosine = (float)cos(radians);
	float sine = (float)sin(radians);

	// Compute dimensions of the resulting bitmap  
	// First get the coordinates of the 3 corners other than origin  
	int x1 = (int)(bm.bmHeight * sine);
	int y1 = (int)(bm.bmHeight * cosine);
	int x2 = (int)(bm.bmWidth * cosine + bm.bmHeight * sine);
	int y2 = (int)(bm.bmHeight * cosine - bm.bmWidth * sine);
	int x3 = (int)(bm.bmWidth * cosine);
	int y3 = (int)(-bm.bmWidth * sine);

	int minx = min(0, min(x1, min(x2, x3)));
	int miny = min(0, min(y1, min(y2, y3)));
	int maxx = max(0, max(x1, max(x2, x3)));
	int maxy = max(0, max(y1, max(y2, y3)));

	int w = maxx - minx;
	int h = maxy - miny;

	// Create a bitmap to hold the result  
	HBITMAP hbmResult = ::CreateCompatibleBitmap(CClientDC(NULL), w, h);

	HBITMAP hbmOldSource = (HBITMAP)::SelectObject(sourceDC.m_hDC, hBitmap);
	HBITMAP hbmOldDest = (HBITMAP)::SelectObject(destDC.m_hDC, hbmResult);

	// Draw the background color before we change mapping mode  
	HBRUSH hbrBack = CreateSolidBrush(clrBack);
	HBRUSH hbrOld = (HBRUSH)::SelectObject(destDC.m_hDC, hbrBack);
	destDC.PatBlt(0, 0, w, h, PATCOPY);
	::DeleteObject(::SelectObject(destDC.m_hDC, hbrOld));

	// We will use world transform to rotate the bitmap  
	SetGraphicsMode(destDC.m_hDC, GM_ADVANCED);
	XFORM xform;
	xform.eM11 = cosine;
	xform.eM12 = -sine;
	xform.eM21 = sine;
	xform.eM22 = cosine;
	xform.eDx = (float)-minx;
	xform.eDy = (float)-miny;

	SetWorldTransform(destDC.m_hDC, &xform);

	// Now do the actual rotating - a pixel at a time  
	destDC.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &sourceDC, 0, 0, SRCCOPY);

	// Restore DCs  
	::SelectObject(sourceDC.m_hDC, hbmOldSource);
	::SelectObject(destDC.m_hDC, hbmOldDest);

	return hbmResult;
}

static HANDLE GetRotatedBitmap(HANDLE hDIB, float radians, COLORREF clrBack)
{
	// Get source bitmap info
	BITMAPINFO &bmInfo = *(LPBITMAPINFO)hDIB;
	int bpp = bmInfo.bmiHeader.biBitCount;		// Bits per pixel

	int nColors = bmInfo.bmiHeader.biClrUsed ? bmInfo.bmiHeader.biClrUsed :
		1 << bpp;
	int nWidth = bmInfo.bmiHeader.biWidth;
	int nHeight = bmInfo.bmiHeader.biHeight;
	int nRowBytes = ((((nWidth * bpp) + 31) & ~31) / 8);

	// Make sure height is positive and biCompression is BI_RGB or BI_BITFIELDS
	DWORD &compression = bmInfo.bmiHeader.biCompression;
	if (nHeight < 0 || (compression != BI_RGB && compression != BI_BITFIELDS))
		return NULL;

	LPVOID lpDIBBits;
	if (bmInfo.bmiHeader.biBitCount > 8)
		lpDIBBits = (LPVOID)((LPDWORD)(bmInfo.bmiColors +
		bmInfo.bmiHeader.biClrUsed) +
		((compression == BI_BITFIELDS) ? 3 : 0));
	else
		lpDIBBits = (LPVOID)(bmInfo.bmiColors + nColors);


	// Compute the cosine and sine only once
	float cosine = (float)cos(radians);
	float sine = (float)sin(radians);

	// Compute dimensions of the resulting bitmap
	// First get the coordinates of the 3 corners other than origin
	int x1 = (int)(-nHeight * sine);
	int y1 = (int)(nHeight * cosine);
	int x2 = (int)(nWidth * cosine - nHeight * sine);
	int y2 = (int)(nHeight * cosine + nWidth * sine);
	int x3 = (int)(nWidth * cosine);
	int y3 = (int)(nWidth * sine);

	int minx = min(0, min(x1, min(x2, x3)));
	int miny = min(0, min(y1, min(y2, y3)));
	int maxx = max(x1, max(x2, x3));
	int maxy = max(y1, max(y2, y3));

	int w = maxx - minx;
	int h = maxy - miny;


	// Create a DIB to hold the result
	int nResultRowBytes = ((((w * bpp) + 31) & ~31) / 8);
	long len = nResultRowBytes * h;
	int nHeaderSize = ((LPBYTE)lpDIBBits - (LPBYTE)hDIB);
	HANDLE hDIBResult = GlobalAlloc(GMEM_FIXED, len + nHeaderSize);
	// Initialize the header information
	memcpy((void*)hDIBResult, (void*)hDIB, nHeaderSize);
	BITMAPINFO &bmInfoResult = *(LPBITMAPINFO)hDIBResult;
	bmInfoResult.bmiHeader.biWidth = w;
	bmInfoResult.bmiHeader.biHeight = h;
	bmInfoResult.bmiHeader.biSizeImage = len;

	LPVOID lpDIBBitsResult = (LPVOID)((LPBYTE)hDIBResult + nHeaderSize);

	// Get the back color value (index)
	ZeroMemory(lpDIBBitsResult, len);
	DWORD dwBackColor;
	switch (bpp) {
		case 1:	//Monochrome
			if (clrBack == RGB(255, 255, 255))
				memset(lpDIBBitsResult, 0xff, len);
			break;
		case 4:
		case 8:	//Search the color table
			int i;
			for (i = 0; i < nColors; i++) {
				if (bmInfo.bmiColors[i].rgbBlue == GetBValue(clrBack)
					&& bmInfo.bmiColors[i].rgbGreen == GetGValue(clrBack)
					&& bmInfo.bmiColors[i].rgbRed == GetRValue(clrBack)) {
					if (bpp == 4) i = i | i << 4;
					memset(lpDIBBitsResult, i, len);
					break;
				}
			}
			// If not match found the color remains black
			break;
		case 16:
			// Windows95 supports 5 bits each for all colors or 5 bits for red & blue
			// and 6 bits for green - Check the color mask for RGB555 or RGB565
			if (*((DWORD*)bmInfo.bmiColors) == 0x7c00) {
				// Bitmap is RGB555
				dwBackColor = ((GetRValue(clrBack) >> 3) << 10) +
					((GetRValue(clrBack) >> 3) << 5) +
					(GetBValue(clrBack) >> 3);
			} else {
				// Bitmap is RGB565
				dwBackColor = ((GetRValue(clrBack) >> 3) << 11) +
					((GetRValue(clrBack) >> 2) << 5) +
					(GetBValue(clrBack) >> 3);
			}
			break;
		case 24:
		case 32:
			dwBackColor = (((DWORD)GetRValue(clrBack)) << 16) |
				(((DWORD)GetGValue(clrBack)) << 8) |
				(((DWORD)GetBValue(clrBack)));
			break;
	}


	// Now do the actual rotating - a pixel at a time
	// Computing the destination point for each source point
	// will leave a few pixels that do not get covered
	// So we use a reverse transform - e.i. compute the source point
	// for each destination point

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int sourcex = (int)((x + minx)*cosine + (y + miny)*sine);
			int sourcey = (int)((y + miny)*cosine - (x + minx)*sine);
			if (sourcex >= 0 && sourcex < nWidth && sourcey >= 0
				&& sourcey < nHeight) {
				// Set the destination pixel
				switch (bpp) {
					BYTE mask;
					case 1:		//Monochrome
						mask = static_cast<unsigned char>(*((LPBYTE)lpDIBBits + nRowBytes*sourcey +
							sourcex / 8) & (0x80 >> sourcex % 8));
						//Adjust mask for destination bitmap
						mask = static_cast<unsigned char>(mask ? (0x80 >> x % 8) : 0);
						*((LPBYTE)lpDIBBitsResult + nResultRowBytes*(y)+
							(x / 8)) &= ~(0x80 >> x % 8);
						*((LPBYTE)lpDIBBitsResult + nResultRowBytes*(y)+
							(x / 8)) |= mask;
						break;
					case 4:
						mask = static_cast<unsigned char>(*((LPBYTE)lpDIBBits + nRowBytes*sourcey +
							sourcex / 2) & ((sourcex & 1) ? 0x0f : 0xf0));
						//Adjust mask for destination bitmap
						if ((sourcex & 1) != (x & 1))
							mask = static_cast<unsigned char>((mask & 0xf0) ? (mask >> 4) : (mask << 4));
						*((LPBYTE)lpDIBBitsResult + nResultRowBytes*(y)+
							(x / 2)) &= ~((x & 1) ? 0x0f : 0xf0);
						*((LPBYTE)lpDIBBitsResult + nResultRowBytes*(y)+
							(x / 2)) |= mask;
						break;
					case 8:
						BYTE pixel;
						pixel = *((LPBYTE)lpDIBBits + nRowBytes*sourcey +
									sourcex);
						*((LPBYTE)lpDIBBitsResult + nResultRowBytes*(y)+
							(x)) = pixel;
						break;
					case 16:
						DWORD dwPixel;
						dwPixel = *((LPWORD)((LPBYTE)lpDIBBits +
							nRowBytes*sourcey + sourcex * 2));
						*((LPWORD)((LPBYTE)lpDIBBitsResult +
							nResultRowBytes*y + x * 2)) = (WORD)dwPixel;
						break;
					case 24:
						dwPixel = *((LPDWORD)((LPBYTE)lpDIBBits +
							nRowBytes*sourcey + sourcex * 3)) & 0xffffff;
						*((LPDWORD)((LPBYTE)lpDIBBitsResult +
							nResultRowBytes*y + x * 3)) |= dwPixel;
						break;
					case 32:
						dwPixel = *((LPDWORD)((LPBYTE)lpDIBBits +
							nRowBytes*sourcey + sourcex * 4));
						*((LPDWORD)((LPBYTE)lpDIBBitsResult +
							nResultRowBytes*y + x * 4)) = dwPixel;
				}
			} else {
				// Draw the background color. The background color
				// has already been drawn for 8 bits per pixel and less
				switch (bpp) {
					case 16:
						*((LPWORD)((LPBYTE)lpDIBBitsResult +
							nResultRowBytes*y + x * 2)) =
							(WORD)dwBackColor;
						break;
					case 24:
						*((LPDWORD)((LPBYTE)lpDIBBitsResult +
							nResultRowBytes*y + x * 3)) |= dwBackColor;
						break;
					case 32:
						*((LPDWORD)((LPBYTE)lpDIBBitsResult +
							nResultRowBytes*y + x * 4)) = dwBackColor;
						break;
				}
			}
		}
	}

	return hDIBResult;
}

void CDetector::SetFocus(BOOL bFocus)
{
	if (m_bFocused ^ bFocus) {
		m_bFocused = bFocus;
		Invalidate(0);
		CLog::WriteLog(_T("CDetector::SetFocus(BOOL bFocus %d) zone %d)"),
						bFocus, m_zoneInfo->get_zone_id());
	}
}

void CDetector::Alarm(BOOL bAlarm)
{
	if (m_bAlarming != bAlarm) {
		m_bAlarming = bAlarm;
		if (m_bAlarming) {
			CLog::WriteLog(_T("#%d Alarm init+++++++++++++++++++++++++++++\n"), 
						   m_zoneInfo->get_zone_id());
			if (::IsWindow(m_hWnd)) {
				KillTimer(m_TimerIDAlarm);
				SetTimer(m_TimerIDAlarm, ALARM_FLICK_GAP, NULL);
			}
			if (this->m_pPairDetector)
				m_pPairDetector->m_bAlarming = TRUE;
		} else {
			if (::IsWindow(m_hWnd)) {
				KillTimer(m_TimerIDAlarm);
				Invalidate();
			}
			if (this->m_pPairDetector) {
				m_pPairDetector->m_bAlarming = FALSE;
				if (::IsWindow(this->m_pPairDetector->m_hWnd)) {
					m_pPairDetector->Invalidate();
				}
			}
		}
	}
}

void CDetector::OnDestroy()
{
	CButton::OnDestroy();
	m_bAntlineGenerated = FALSE;
	KillTimer(m_TimerIDAlarm);
	KillTimer(m_TimerIDRepaint);
	ReleasePts();
	if (m_hRgn)	::DeleteObject(m_hRgn); m_hRgn = NULL;
	if (m_hBitmap) ::DeleteObject(m_hBitmap);	m_hBitmap = NULL;
	if (m_hBrushFocused)	DeleteObject(m_hBrushFocused);	m_hBrushFocused = NULL;
	if (m_hBrushAlarmed)	DeleteObject(m_hBrushAlarmed);	m_hBrushAlarmed = NULL;

	if (m_bMainDetector && m_pPairDetector) {
		SAFEDELETEDLG(m_pPairDetector);
	}
}

void CDetector::InitToolTip()
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
}

BOOL CDetector::PreTranslateMessage(MSG* pMsg)
{
	InitToolTip();
	m_ToolTip.RelayEvent(pMsg);
	return CButton::PreTranslateMessage(pMsg);
}

void CDetector::SetTooltipText(LPCTSTR lpszText, BOOL bActivate)
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

void CDetector::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!m_bMouseIn) {
		m_bMouseIn = TRUE;
		CZonePropertyInfo* info = CZonePropertyInfo::GetInstance();
		CZonePropertyData* data = info->GetZonePropertyData(m_zoneInfo->get_detector_property_id());

		CString tip = _T(""), strZone = _T(""), strProperty = L"", strAlias = L"";
		strZone.LoadString(IDS_STRING_ZONE);
		strProperty.LoadString(IDS_STRING_PROPERTY);
		strAlias.LoadString(IDS_STRING_ALIAS);
		tip.Format(_T("%s:%03d\r\n%s:%s\r\n%s:%s"),
				   strZone, m_zoneInfo->get_zone_id(),
				   strProperty, data->get_property_text(),
				   strAlias, m_zoneInfo->get_alias());
		SetTooltipText(tip);
	}
	CButton::OnMouseMove(nFlags, point);
}


void CDetector::OnMouseLeave()
{
	m_bMouseIn = FALSE;

	CButton::OnMouseLeave();
}


int CDetector::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CButton::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_bAntlineGenerated = FALSE;
	m_hBitmap = control::CBmpEx::GetHBitmapThumbnail(m_detectorLibData->get_path(),
													 DETECTORWIDTH, DETECTORWIDTH);
	m_hBrushFocused = CreateSolidBrush(RGB(0, 255, 0));
	m_hBrushAlarmed = CreateSolidBrush(RGB(255, 0, 0));
	m_bManualRotate = TRUE;

	if (m_bAlarming && m_bMainDetector)
		SetTimer(m_TimerIDAlarm, ALARM_FLICK_GAP, NULL);

	return 0;
}

void CDetector::GenerateAntlinePts()
{
	CLocalLock lock(&m_cs);
	SAFEDELETEARR(m_pts);
	if (m_pts == NULL) {
		m_pt.x = m_sizeBmp.cx / 2;
		m_pt.y = m_sizeBmp.cy / 2;

		int antline_num = m_detectorLibData->get_antline_num();
		if (antline_num == 0)
			return;

		m_pts = new CPoint[antline_num];

		if (antline_num == 1) {
			m_pts[0] = m_pt;
			ClientToScreen(&m_pts[0]);
		} else {
			int distance = 0;
			int mid = antline_num / 2;
			BOOL bEven = (antline_num % 2) == 0;
			int antline_gap = m_detectorLibData->get_antline_gap();
			int angle = m_detectorInfo->get_angle();

			for (int i = 0; i < antline_num; i++) {
				if (bEven) {
					if (i >= mid) {
						distance = abs(antline_gap * (abs(mid - i)) + antline_gap / 2);
					} else {
						distance = abs(antline_gap * (abs(mid - i)) - antline_gap / 2);
					}
				} else {
					distance = antline_gap * (abs(mid - i));
				}

				if (i < mid) {
					angle = (630 - angle) % 360;
				} else if (i == mid && !bEven) {
					angle = 0;
				} else {
					angle = abs(450 - angle) % 360;
				}

				m_pts[i] = control::CCoordinate::GetRotatedPoint(m_pt, distance, angle);
				ClientToScreen(&m_pts[i]);
			}
		}
	}
}

void CDetector::ReleasePts()
{
	SAFEDELETEARR(m_pts);
}

NAMESPACE_END
