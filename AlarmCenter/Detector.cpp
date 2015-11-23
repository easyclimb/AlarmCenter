// Detector.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "Detector.h"
#include "aarot.hpp"
#include "BmpEx.h"
#include "Coordinate.h"
#include "ZoneInfo.h"
#include "CameraInfo.h"
#include "DetectorInfo.h"
#include "DetectorLib.h"
//#include "SubMachineInfo.h"
#include "AlarmMachine.h"
#include "AlarmMachineDlg.h"
#include "AlarmMachineManager.h"
#include "InputDlg.h"
#include "VideoManager.h"
#include "VideoDeviceInfoEzviz.h"
#include "VideoUserInfoEzviz.h"
#include "VideoPlayerDlg.h"

using namespace ademco;
using namespace core;
using namespace gui;
using namespace gui::control;
#include "AppResource.h"

namespace {
	const UINT cTimerIDRepaint = 1;
	const UINT cTimerIDAlarm = 2;
	//static const UINT cTimerIDRelayGetIsAlarming = 3;
	const UINT cTimerIDHandleIczc = 4;

	const int ALARM_FLICK_GAP = 1500;


	typedef struct IczcBuffer {
		InversionControlZoneCommand _iczc;
		DWORD _extra;
		IczcBuffer(InversionControlZoneCommand iczc, DWORD extra) :_iczc(iczc), _extra(extra) {}
	}IczcBuffer;

	void __stdcall OnInversionControlZone(void* udata,
										  InversionControlZoneCommand iczc,
										  DWORD dwExtra)
	{
		AUTO_LOG_FUNCTION;
		CDetector* detector = reinterpret_cast<CDetector*>(udata); assert(detector);
		//if (detector && IsWindow(detector->m_hWnd))
		//	detector->PostMessageW(WM_INVERSIONCONTROL, iczc, dwExtra);
		if (detector) {
			detector->AddIczc(new IczcBuffer(iczc, dwExtra));
		}
	}
};

/////////////////////////////////////////////////////////////////////////////
// CDetector
CDetector::CDetector(CDetectorBindInterface* pInterface, CDetectorInfo* detectorInfo,
					 BOOL bMainDetector)
	: m_pPairDetector(nullptr)
	, m_hRgn(nullptr)
	//, m_hRgnRotated(nullptr)
	, m_hBitmap(nullptr)
	, m_hBitmapRotated(nullptr)
	, m_interface(nullptr)
	, m_detectorInfo(nullptr)
	, m_detectorLibData(nullptr)
	, m_bFocused(FALSE)
	, m_bManualRotate(TRUE)
	, m_bAlarming(FALSE)
	, m_bCurColorRed(FALSE)
	//, m_TimerIDRepaint(1)
	//, m_TimerIDAlarm(2)
	, m_hBrushFocused(nullptr)
	, m_hBrushAlarmed(nullptr)
	, m_bAntlineGenerated(FALSE)
	, m_pts(nullptr)
	, m_bNeedRecalcPts(FALSE)
	//, m_parentWnd(parentWnd)
	, m_bMainDetector(bMainDetector)
	, m_bMouseIn(FALSE)
	, m_bRbtnDown(FALSE)
{
	ASSERT(pInterface);
	m_interface = pInterface;
	if (detectorInfo) {
		m_detectorInfo = detectorInfo;
	} else {
		CDetectorInfo* info = m_interface->GetDetectorInfo();
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
		m_interface->SetInversionControlCallback(this, OnInversionControlZone);
	} else {
		m_detectorLibData->set_path(data->get_path_pair());
		//m_detectorLibData->set_path_pair(data->get_path_pair());
	}
	m_detectorLibData->set_type(data->get_type());
	m_bAlarming = m_interface->get_alarming();

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
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_WM_CREATE()
	ON_WM_MOUSELEAVE()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
	ON_CONTROL_REFLECT(BN_CLICKED, &CDetector::OnBnClicked)
	ON_CONTROL_REFLECT(BN_DOUBLECLICKED, &CDetector::OnBnDoubleclicked)
	ON_MESSAGE(WM_INVERSIONCONTROL, &CDetector::OnInversionControlResult)
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDetector message handlers

BOOL CDetector::CreateDetector(CWnd* parentWnd)
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
		ok = Create(txt, WS_CHILD | WS_VISIBLE, rc, parentWnd, 0);
#else 
		ok = Create(nullptr, WS_CHILD | WS_VISIBLE, rc, parentWnd, 0);
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
		m_pPairDetector = new CDetector(m_interface, detectorInfo, FALSE);

		ok = m_pPairDetector->Create(nullptr, WS_CHILD | WS_VISIBLE, rc, parentWnd, 0);
		if (!ok) { break; }

	} while (0);

	return ok;
}


void CDetector::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CLocalLock lock(&m_cs);
	if (m_hBitmap == nullptr)
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
													  nullptr, RGB(255, 255, 255),
													  1, 0, nullptr);
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
	HRGN hRgn = nullptr;

	if (hBmp) {
		// Create a memory DC inside which we will scan the bitmap content
		HDC hMemDC = CreateCompatibleDC(nullptr);
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
											 (BITMAPINFO *)&RGB32BITSBITMAPINFO, DIB_RGB_COLORS, &pbits32, nullptr, 0);
			if (hbm32) {
				HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, hbm32);

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
									HRGN h = ExtCreateRegion(nullptr, sizeof(RGNDATAHEADER)+(sizeof(RECT)* maxRects), pData);
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
					HRGN h = ExtCreateRegion(nullptr, sizeof(RGNDATAHEADER)+(sizeof(RECT)* maxRects), pData);
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

				DeleteObject(SelectObject(hMemDC, hOldBmp));
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
	AUTO_LOG_FUNCTION;
	//if((int)angle % 360 == 0)	return;
	m_bManualRotate = TRUE;
	//KillTimer(m_TimerIDRepaint);
	//SetTimer(m_TimerIDRepaint, 2000, nullptr);
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


void CDetector::OnTimer(UINT nIDEvent)
{
	if (cTimerIDRepaint == nIDEvent) {
		KillTimer(cTimerIDRepaint);
		Invalidate();//Invalidate();
	} else if (cTimerIDAlarm == nIDEvent) {
		if (this->m_pPairDetector)
			m_pPairDetector->SendMessage(WM_TIMER, cTimerIDAlarm);
		m_bCurColorRed = !m_bCurColorRed;
		Invalidate(0);
		//InvalidateRgn(CRgn::FromHandle(m_hRgn));
	} /*else if (cTimerIDRelayGetIsAlarming == nIDEvent) {

	}*/else if (cTimerIDHandleIczc == nIDEvent) {
		if (m_iczcLock.TryLock()) {
			while (m_iczcList.size() > 0) {
				IczcBuffer* iczc = reinterpret_cast<IczcBuffer*>(m_iczcList.front());
				m_iczcList.pop_front();
				OnInversionControlResult(iczc->_iczc, iczc->_extra);
				delete iczc;
			}
			m_iczcLock.UnLock();
		}
	}
	CButton::OnTimer(nIDEvent);
}


void CDetector::SetFocus(BOOL bFocus)
{
	AUTO_LOG_FUNCTION;
	if (m_bFocused ^ bFocus) {
		m_bFocused = bFocus;
		Invalidate(0);
		if (m_bMainDetector 
			&& m_pPairDetector
			&& ::IsWindow(m_pPairDetector->m_hWnd)) {
			m_pPairDetector->SetFocus(bFocus);
		}
		//CLog::WriteLog(_T("CDetector::SetFocus(BOOL bFocus %d) zone %d)"),
		//			   bFocus, m_zoneInfo->get_zone_value());
	}
}


void CDetector::Alarm(BOOL bAlarm)
{
	AUTO_LOG_FUNCTION;
	if (m_bAlarming != bAlarm) {
		m_bAlarming = bAlarm;
		if (m_bAlarming) {
			//CLog::WriteLog(_T("#%d Alarm init+++++++++++++++++++++++++++++\n"), 
			//			   m_zoneInfo->get_zone_value());
			if (::IsWindow(m_hWnd)) {
				KillTimer(cTimerIDAlarm);
				SetTimer(cTimerIDAlarm, ALARM_FLICK_GAP, nullptr);
			}
			if (this->m_pPairDetector)
				m_pPairDetector->m_bAlarming = TRUE;
		} else {
			if (::IsWindow(m_hWnd)) {
				KillTimer(cTimerIDAlarm);
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
	KillTimer(cTimerIDAlarm);
	KillTimer(cTimerIDRepaint);
	//KillTimer(cTimerIDRelayGetIsAlarming);
	if (m_bMainDetector && m_interface) {
		m_interface->SetInversionControlCallback(nullptr, nullptr);
	}

	ReleasePts();
	if (m_hRgn) { 
		::DeleteObject(m_hRgn); m_hRgn = nullptr;
	}
	if (m_hBitmap) {
		::DeleteObject(m_hBitmap);	m_hBitmap = nullptr;
	}
	if (m_hBrushFocused) {
		DeleteObject(m_hBrushFocused);	m_hBrushFocused = nullptr;
	}
	if (m_hBrushAlarmed) {
		DeleteObject(m_hBrushAlarmed);	m_hBrushAlarmed = nullptr;
	}

	if (m_bMainDetector && m_pPairDetector) {
		SAFEDELETEDLG(m_pPairDetector);
	}

	m_iczcLock.Lock();
	for (auto iczc_voidp : m_iczcList) {
		IczcBuffer* iczc = reinterpret_cast<IczcBuffer*>(iczc_voidp);
		delete iczc;
	}
	m_iczcList.clear();
	m_iczcLock.UnLock();
}


void CDetector::InitToolTip()
{
	if (m_ToolTip.m_hWnd == nullptr) {
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
	// We cannot accept nullptr pointer
	if (lpszText == nullptr) return;

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
	if (!m_bMouseIn && m_interface) {
		TRACKMOUSEEVENT csTME;
		csTME.cbSize = sizeof (csTME);
		csTME.dwFlags = TME_LEAVE | TME_HOVER;
		csTME.hwndTrack = m_hWnd;// 指定要 追踪 的窗口 
		csTME.dwHoverTime = 10;  // 鼠标在按钮上停留超过 10ms ，才认为状态为 HOVER
		::_TrackMouseEvent(&csTME);
		m_bMouseIn = TRUE;
		ShowToolTip();
	}
	CButton::OnMouseMove(nFlags, point);
}


std::wstring CZoneInfo::FormatTooltip() const
{
	std::wstring tooltip;
	CString tip, fmzone, fmproperty, fmalias, szone, fmsubmachine;
	fmzone.LoadString(IDS_STRING_ZONE);
	fmalias.LoadString(IDS_STRING_ALIAS);
	fmsubmachine.LoadStringW(IDS_STRING_SUBMACHINE);
	ZoneType zt = get_type();

	if (zt == ZT_SUB_MACHINE_ZONE) {
		szone.Format(L"%s:%02d", fmzone, get_sub_zone());
	} else if (ZT_SUB_MACHINE == zt) {
		szone.Format(L"%s:%03d", fmsubmachine, get_zone_value());
	} else {
		szone.Format(L"%s:%03d", fmzone, get_zone_value());
	}

	tip.Format(_T("%s\r\n%s:%s"), szone,
			   /*fmproperty, data ? data->get_property_text() : sproperty,\r\n%s:%s*/
			   fmalias, get_alias());

	if (zt == ZT_SUB_MACHINE) {
		CAlarmMachine* subMachine = GetSubMachineInfo();
		if (subMachine) {
			CString extra, sstatus, scontact, saddress, sphone, sphone_bk;
			sstatus.LoadStringW(IDS_STRING_MACHINE_STATUS);
			scontact.LoadStringW(IDS_STRING_CONTACT);
			saddress.LoadStringW(IDS_STRING_ADDRESS);
			sphone.LoadStringW(IDS_STRING_PHONE);
			sphone_bk.LoadStringW(IDS_STRING_PHONE_BK);
			int status = (subMachine->get_armed() == true) ? EVENT_ARM : EVENT_DISARM;
			CAppResource* res = CAppResource::GetInstance();
			extra.Format(L"\r\n%s:%s\r\n%s:%s\r\n%s:%s\r\n%s:%s\r\n%s:%s\r\n",
						 sstatus, res->AdemcoEventToString(status),
						 scontact, subMachine->get_contact(),
						 saddress, subMachine->get_address(),
						 sphone, subMachine->get_phone(),
						 sphone_bk, subMachine->get_phone_bk());
			tip += extra;
		}
	}
	tooltip = tip;
	return tooltip;
}


std::wstring CCameraInfo::FormatTooltip() const
{
	USES_CONVERSION;
	using namespace video;
	if (_productor != EZVIZ) return L"";
	CVideoDeviceInfo* dev = nullptr;
	if (CVideoManager::GetInstance()->GetVideoDeviceInfo(_device_info_id, EZVIZ, dev) && dev) {
		ezviz::CVideoDeviceInfoEzviz* device = reinterpret_cast<ezviz::CVideoDeviceInfoEzviz*>(dev);
		CString note, user;
		note.LoadStringW(IDS_STRING_NOTE);
		user.LoadStringW(IDS_STRING_USER);
		CString tip;
		tip.Format(L"%s:%s\r\n%s:%s\r\nID:%d\r\nSerial:%s", 
				   note, device->get_device_note().c_str(),
				   user, device->get_userInfo()->get_user_name().c_str(),
				   device->get_id(),
				   A2W(device->get_deviceSerial().c_str()));
		return std::wstring(tip);
	}
	return L"camera";
}


void CDetector::ShowToolTip()
{
	if (m_interface)
		SetTooltipText(m_interface->FormatTooltip().c_str());
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

	if (m_bAlarming && m_bMainDetector){
		SetTimer(cTimerIDAlarm, ALARM_FLICK_GAP, nullptr);
	}
	if (m_bMainDetector) {
		SetTimer(cTimerIDHandleIczc, 50, nullptr);
	}
	return 0;
}


void CDetector::GenerateAntlinePts()
{
	CLocalLock lock(&m_cs);
	SAFEDELETEARR(m_pts);
	if (m_pts == nullptr) {
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


void CDetector::OnBnClicked()
{
	/*if (m_interface) {
		if (DIT_ZONE_INFO == m_interface->GetInterfaceType())
			m_interface->DoClick();
		else if (DIT_CAMERA_INFO == m_interface->GetInterfaceType())
			OnClick();
	}*/
	OnClick();
}


void CDetector::OnClick()
{
	if (DIT_ZONE_INFO == m_interface->GetInterfaceType()) {
		core::CZoneInfo* zoneInfo = reinterpret_cast<core::CZoneInfo*>(m_interface);
		ZoneType zt = zoneInfo->get_type();
		if (ZT_SUB_MACHINE == zt) {
			CAlarmMachine* subMachine = zoneInfo->GetSubMachineInfo();
			if (subMachine) {
				CAlarmMachineDlg dlg;
				dlg.SetMachineInfo(subMachine);
				dlg.DoModal();
			}
		} else {
			ShowToolTip();
		}
	} else if (DIT_CAMERA_INFO == m_interface->GetInterfaceType()) {
		using namespace video;
		CVideoDeviceInfo* dev = nullptr;
		CCameraInfo* camera = reinterpret_cast<CCameraInfo*>(m_interface);
		if ((camera->get_productor() == EZVIZ) && CVideoManager::GetInstance()->GetVideoDeviceInfo(camera->get_device_info_id(), EZVIZ, dev) && (dev != nullptr) && (g_videoPlayerDlg != nullptr)) {
			g_videoPlayerDlg->PlayVideoByDevice(dev, 0);
		}
	}
}


void CDetector::OnBnDoubleclicked()
{

}


void CDetector::OnRotate()
{
	AUTO_LOG_FUNCTION;
	CDetectorInfo* detInfo = m_interface->GetDetectorInfo();
	int angle = detInfo->get_angle();
	if (m_detectorInfo->get_angle() == angle)
		return;

	Rotate(angle);
	if (m_bMainDetector && m_pPairDetector && IsWindow(m_pPairDetector->m_hWnd)) {
		CRect rc, rc0;
		GetWindowRect(rc0);
		m_pPairDetector->GetWindowRect(rc);
		int width = rc.Width();
		int height = rc.Height();
		CPoint ptRtd = CCoordinate::GetRotatedPoint(CPoint(rc0.left, rc0.top),
													detInfo->get_distance(), 
													-angle);
		rc.left = ptRtd.x;
		rc.right = rc.left + width;
		rc.top = ptRtd.y;
		rc.bottom = rc.top + height;
		CWnd *parent = GetParent();
		parent->ScreenToClient(rc);
		m_pPairDetector->MoveWindow(rc);
		m_pPairDetector->m_detectorInfo->set_x(rc.left);
		m_pPairDetector->m_detectorInfo->set_y(rc.top);
		m_pPairDetector->Rotate(angle);
	}
}


void CDetector::OnDistance()
{
	AUTO_LOG_FUNCTION;
	CDetectorInfo* detInfo = m_interface->GetDetectorInfo();
	int distance = detInfo->get_distance();
	if (m_detectorInfo->get_distance() == distance)
		return;

	m_detectorInfo->set_distance(distance);
	if (m_bMainDetector && m_pPairDetector && IsWindow(m_pPairDetector->m_hWnd)) {
		CRect rcPair, rc;
		GetWindowRect(rc);
		m_pPairDetector->GetWindowRect(rcPair);
		int width = rcPair.Width();
		int height = rcPair.Height();

		CPoint ptRtd = CCoordinate::GetRotatedPoint(CPoint(rc.left, rc.top),
													distance,
													-(m_detectorInfo->get_angle()) % 360);
		rcPair.left = ptRtd.x;
		rcPair.right = rcPair.left + width;
		rcPair.top = ptRtd.y;
		rcPair.bottom = rcPair.top + height;
		CWnd *parent = GetParent();
		parent->ScreenToClient(rcPair);
		m_pPairDetector->m_detectorInfo->set_x(rcPair.left);
		m_pPairDetector->m_detectorInfo->set_y(rcPair.top);
		m_pPairDetector->MoveWindow(rcPair);
	}
}


void CDetector::OnMoveWithDirection()
{
	AUTO_LOG_FUNCTION;
	CDetectorInfo* detInfo = m_interface->GetDetectorInfo();
	int offset_x = detInfo->get_x() - m_detectorInfo->get_x();
	int offset_y = detInfo->get_y() - m_detectorInfo->get_y();
	if (offset_x == 0 && offset_y == 0)
		return;

	m_detectorInfo->set_x(detInfo->get_x());
	m_detectorInfo->set_y(detInfo->get_y());

	CRect rc, rcPair;
	GetWindowRect(rc);
	if (m_bMainDetector && m_pPairDetector && IsWindow(m_pPairDetector->m_hWnd)) {
		m_pPairDetector->GetWindowRect(rcPair);
	}

	int width = rc.Width();
	int height = rc.Height();

	rc.left += offset_x;
	rc.top += offset_y;
	rc.right = rc.left + width;
	rc.bottom = rc.top + height;

	rcPair.left += offset_x;
	rcPair.top += offset_y;
	rcPair.right = rcPair.left + width;
	rcPair.bottom = rcPair.top + height;

	CWnd *parent = GetParent();
	parent->ScreenToClient(rc);
	parent->ScreenToClient(rcPair);

	MoveWindow(rc);
	if (m_bMainDetector && m_pPairDetector && IsWindow(m_pPairDetector->m_hWnd)) {
		m_pPairDetector->MoveWindow(rcPair);
	}
}


afx_msg LRESULT CDetector::OnInversionControlResult(WPARAM wParam, LPARAM lParam)
{
	AUTO_LOG_FUNCTION;
	InversionControlZoneCommand iczc = static_cast<InversionControlZoneCommand>(wParam);
	COLORREF clr = static_cast<COLORREF>(lParam);
	switch (iczc) {
		case core::ICZC_ALARM_START:
			SetAlarmingColor(clr);
			Alarm(TRUE);
			break;
		case core::ICZC_ALARM_STOP:
			Alarm(FALSE);
			break;
		case core::ICZC_SET_FOCUS:
			SetFocus(TRUE);
			break;
		case core::ICZC_KILL_FOCUS:
			SetFocus(FALSE);
			break;
		case core::ICZC_ROTATE:
			OnRotate();
			break;
		case core::ICZC_DISTANCE:
			OnDistance();
			break;
		case core::ICZC_MOVE:
			OnMoveWithDirection();
			break;
		case core::ICZC_CLICK:
			OnClick();
			break;
		case core::ICZC_RCLICK:
			OnRClick();
			break;
		case core::ICZC_DESTROY:
			m_interface = nullptr;
			break;
		//case core::ICZC_ALIAS_CHANGED:
		//	if (m_zoneInfo) {
		//		CAlarmMachine* subMachineInfo = m_zoneInfo->GetSubMachineInfo();
		//		if (subMachineInfo) {
		//			SetWindowText(subMachineInfo->get_alias());
		//		}
		//	}
		//	break;
		default:
			break;
	}
	
	return 0;
}


void CDetector::SetAlarmingColor(COLORREF clr)
{
	if (m_hBrushAlarmed) {
		DeleteObject(m_hBrushAlarmed);	m_hBrushAlarmed = nullptr;
	}
	m_hBrushAlarmed = CreateSolidBrush(clr);
	if (this->m_pPairDetector)
		m_pPairDetector->SetAlarmingColor(clr);
}


void CDetector::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_bRbtnDown = TRUE;
	CButton::OnRButtonDown(nFlags, point);
}


void CDetector::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (m_bRbtnDown) {
		m_bRbtnDown = FALSE;

		if (m_interface) {
			m_interface->DoRClick();
		}
	}
	CButton::OnRButtonUp(nFlags, point);
}


void CDetector::OnRClick()
{
	CAlarmMachine* subMachine = nullptr;
	core::CZoneInfo* zoneInfo = reinterpret_cast<core::CZoneInfo*>(m_interface);
	if (zoneInfo && zoneInfo->get_type() == ZT_SUB_MACHINE) {
		subMachine = zoneInfo->GetSubMachineInfo();
	} else {
		return;
	}

	CMenu menu, *subMenu;
	menu.LoadMenuW(IDR_MENU1);
	subMenu = menu.GetSubMenu(0);

	//CRect rc;
	//GetWindowRect(rc);
	//ScreenToClient(&point);
	CPoint point;
	GetCursorPos(&point);
	//ClientToScreen(&point);
	int ret = subMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
									  /*rc.left, rc.bottom, */
									  point.x, point.y,
									  this);

	core::CAlarmMachineManager* manager = core::CAlarmMachineManager::GetInstance();

	switch (ret) {
	case ID_DDD_32771: // open
		OnBnClicked();
		break;
	case ID_DDD_32772: // arm
		manager->RemoteControlAlarmMachine(subMachine,
										   ademco::EVENT_ARM,
										   INDEX_SUB_MACHINE,
										   subMachine->get_submachine_zone(),
										   nullptr, 0, this);
		break;
	case ID_DDD_32773:
	{ // disarm
		char xdata[64] = { 0 };
		int xdata_len = 0;
		if (!subMachine->get_is_submachine()) {
			CInputDlg dlg(this);
			if (dlg.DoModal() != IDOK)
				return;
			if (dlg.m_edit.GetLength() != 6)
				return;

			USES_CONVERSION;
			strcpy_s(xdata, W2A(dlg.m_edit));
			xdata_len = strlen(xdata);
		}
		manager->RemoteControlAlarmMachine(subMachine,
										   ademco::EVENT_DISARM,
										   INDEX_SUB_MACHINE,
										   subMachine->get_submachine_zone(),
										   xdata, xdata_len, this);
	}
	break;
	case ID_DDD_32774: // emergency
		manager->RemoteControlAlarmMachine(subMachine,
										   ademco::EVENT_EMERGENCY,
										   INDEX_SUB_MACHINE,
										   subMachine->get_submachine_zone(),
										   nullptr, 0, this);
		break;
	case ID_DDD_32775: // clear msg
		if (subMachine) {
			subMachine->clear_ademco_event_list();
		}
		break;
	default:
		break;

	}
}
