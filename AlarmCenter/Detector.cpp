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
using namespace gui;
#include "AppResource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//namespace gui
//{

/////////////////////////////////////////////////////////////////////////////
// CDetector
//BOOL CDetector::m_bCurColorRed = FALSE;

CDetector::CDetector(CZoneInfo* zoneInfo, 
					 CDetectorInfo* detectorInfo, 
					 CWnd* parentWnd, 
					 BOOL bMainDetector)
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
	//, m_TimerIDRepaint(1)
	//, m_TimerIDAlarm(2)
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
	if (detectorInfo) {
		m_detectorInfo = detectorInfo;
	} else {
		CDetectorInfo* info = m_zoneInfo->GetDetectorInfo();
		m_detectorInfo = new CDetectorInfo();
		m_detectorInfo->set_id(info->get_id());
		m_detectorInfo->set_x(info->get_x());
		m_detectorInfo->set_y(info->get_y());
		m_detectorInfo->set_distance(info->get_distance());
		m_detectorInfo->set_angle(info->get_angle());
		m_detectorInfo->set_detector_lib_id(info->get_detector_lib_id());
	}

	CDetectorLib* lib = CDetectorLib::GetInstance();
	const CDetectorLibData* data = lib->GetDetectorLibData(m_detectorInfo->get_detector_lib_id());
	m_detectorLibData = new CDetectorLibData();
	m_detectorLibData->set_antline_gap(data->get_antline_gap());
	m_detectorLibData->set_antline_num(data->get_antline_num());
	m_detectorLibData->set_detector_name(data->get_detector_name());
	m_detectorLibData->set_id(data->get_id());
	if (m_bMainDetector) {
		m_detectorLibData->set_path(data->get_path());
		m_detectorLibData->set_path_pair(data->get_path_pair());
	} else {
		m_detectorLibData->set_path(data->get_path_pair());
		//m_detectorLibData->set_path_pair(data->get_path_pair());
	}
	m_detectorLibData->set_type(data->get_type());

	InitializeCriticalSection(&m_cs);
}

CDetector::~CDetector()
{
	delete m_detectorLibData;
	delete m_detectorInfo;
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
	ON_WM_SHOWWINDOW()
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

		CPoint ptRtd = control::CCoordinate::GetRotatedPoint(CPoint(rc.left, rc.top),
															 m_detectorInfo->get_distance(),
															 -m_detectorInfo->get_angle());
		rc.left = ptRtd.x;
		rc.top = ptRtd.y;
		rc.right = rc.left + width;
		rc.bottom = rc.top + height;

		CDetectorInfo* detectorInfo = new CDetectorInfo();
		detectorInfo->set_id(m_detectorInfo->get_id());
		detectorInfo->set_x(rc.left);
		detectorInfo->set_y(rc.top);
		detectorInfo->set_distance(m_detectorInfo->get_distance());
		detectorInfo->set_angle(m_detectorInfo->get_angle() % 360);
		detectorInfo->set_detector_lib_id(m_detectorInfo->get_detector_lib_id());
		m_pPairDetector = new CDetector(m_zoneInfo, detectorInfo, m_parentWnd, FALSE);
		

		ok = m_pPairDetector->Create(NULL, WS_CHILD | WS_VISIBLE, rc, m_parentWnd, 0);
		if (!ok) { break; }

	} while (0);

	return ok;
}


int CDetector::GetZoneID() const
{
	if (m_zoneInfo) {
		return m_zoneInfo->get_zone_id();
	}
	return -1;
}


void CDetector::FormatAlarmText(CString& alarmText, int ademco_event)
{
	CString fmZone, fmNull;
	fmZone.LoadStringW(IDS_STRING_ZONE);
	fmNull.LoadStringW(IDS_STRING_NULL);

	CZonePropertyInfo* info = CZonePropertyInfo::GetInstance();
	CZonePropertyData* data = info->GetZonePropertyData(m_zoneInfo->get_detector_property_id());

	CString alias = m_zoneInfo->get_alias();
	if (alias.IsEmpty()) {
		alias = fmNull;
	}

	if (ademco::IsExceptionEvent(ademco_event) || (data == NULL)) { // �쳣��Ϣ������ event ��ʾ����
		CAppResource* res = CAppResource::GetInstance();
		CString strEvent = res->AdemcoEventToString(ademco_event);
		alarmText.Format(L"%s %03d:%s(%s)", fmZone, m_zoneInfo->get_zone_id(),
						 strEvent, alias);
	} else { // ������Ϣ������ �ֶ����õı������� �� event ��ʾ����
		alarmText.Format(L"%s %03d:%s(%s)", fmZone, m_zoneInfo->get_zone_id(),
						 data->get_alarm_text(), alias);
	}
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


void CDetector::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CButton::OnShowWindow(bShow, nStatus);
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
			int detecotr_angle = m_detectorInfo->get_angle();
			int antline_angle = 0;

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
					antline_angle = (630 - detecotr_angle) % 360;
				} else if (i == mid && !bEven) {
					antline_angle = 0;
				} else {
					antline_angle = abs(450 - detecotr_angle) % 360;
				}

				m_pts[i] = control::CCoordinate::GetRotatedPoint(m_pt, distance, 
																 antline_angle);
				ClientToScreen(&m_pts[i]);
			}
		}
	}
}

void CDetector::ReleasePts()
{
	SAFEDELETEARR(m_pts);
}

//NAMESPACE_END