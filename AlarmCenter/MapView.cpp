// MapView.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "MapView.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "MapInfo.h"
#include "DetectorInfo.h"
#include "DetectorLib.h"
#include "Detector.h"
#include "ZonePropertyInfo.h"
#include "ZoneInfo.h"
#include "AntLine.h"
#include "DesktopTextDrawer.h"
#include "AppResource.h"
#include "HistoryRecord.h"

using namespace core;
//namespace gui {

static const int cTimerIDDrawAntLine = 1;
static const int cTimerIDFlashSensor = 2;
static const int cTimerIDRelayTraverseAlarmText = 3;



static void __stdcall OnInversionControlCommand(void* udata,
												InversionControlCommand icc,
												const AlarmText* at)
{
	CMapView* mapView = reinterpret_cast<CMapView*>(udata); assert(mapView);
	mapView->SendMessage(WM_NEWALARMTEXT, (WPARAM)icc, (LPARAM)at);
}

IMPLEMENT_DYNAMIC(CMapView, CDialogEx)

CMapView::CMapView(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMapView::IDD, pParent)
	, m_machine(NULL)
	, m_mapInfo(NULL)
	, m_hBmpOrigin(NULL)
	, m_bmWidth(0)
	, m_bmHeight(0)
	, m_detectorList()
	, m_pAntLine(NULL)
	, m_pTextDrawer(NULL)
	, m_bAlarming(FALSE)
	, m_mode(MODE_NORMAL)
	, m_nFlashTimes(0)
	, m_hDC4AntLine(NULL)
	, m_pRealParent(NULL)
{
	::InitializeCriticalSection(&m_csDetectorList);
}


CMapView::~CMapView()
{
	::DeleteCriticalSection(&m_csDetectorList);
}


void CMapView::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMapView, CDialogEx)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
	ON_WM_TIMER()
	ON_MESSAGE(WM_REPAINT, &CMapView::OnRepaint)
	//ON_MESSAGE(WM_TRAVERSEZONE, &CMapView::OnTraversezone)
	ON_MESSAGE(WM_NEWALARMTEXT, &CMapView::OnNewAlarmTextResult)
END_MESSAGE_MAP()


// CMapView message handlers

BOOL CMapView::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	if (m_mapInfo && m_machine) {
		m_bAlarming = m_mapInfo->get_alarming();
		m_pAntLine = new gui::CAntLine();
		m_pTextDrawer = new gui::CDesktopTextDrawer();
		m_pTextDrawer->SetOwner(this);

		if (m_mapInfo->get_id() != -1) {
			CZoneInfoList list;
			m_mapInfo->GetAllZoneInfo(list);
			CZoneInfoListIter iter = list.begin();
			while (iter != list.end()) {
				CZoneInfo* zoneInfo = *iter++;
				CDetector* detector = new CDetector(zoneInfo, NULL);
				if (detector->CreateDetector(this)) {
					m_detectorList.push_back(detector);
				}
			}
		}

		m_mapInfo->SetInversionControlCallBack(this, OnInversionControlCommand);
		SetTimer(cTimerIDRelayTraverseAlarmText, 500, NULL);
		//m_mapInfo->TraverseAlarmText(this, OnNewAlarmText);
	}

	return TRUE;  
}


BOOL CMapView::ImportBmp()
{
	ASSERT(m_mapInfo);
	HINSTANCE hInst = (HINSTANCE)::GetWindowLong(m_hWnd, GWL_HINSTANCE);
	FILE *fp = NULL;
	_tfopen_s(&fp, m_mapInfo->get_path(), _T("rb"));
	if (fp == NULL) { return FALSE; }

	BITMAPFILEHEADER bmpFileHeader;
	BITMAPINFOHEADER bmpInfoHeader;
	unsigned int uiRead = fread(&bmpFileHeader, 1, sizeof(bmpFileHeader), fp);

	if (uiRead != sizeof(bmpFileHeader)) {
		CLog::WriteLog(L"CMapView::ImportBmp() failed, read file header err");
		return FALSE;
	}

	if (bmpFileHeader.bfType != 0x4d42) {
		CLog::WriteLog(L"CMapView::ImportBmp() failed, not a bmp file");
		return FALSE;
	}

	if ((fread(&bmpInfoHeader, 1, sizeof(bmpInfoHeader), fp)) != sizeof(bmpInfoHeader)) {		//SHOWERROR(_T("read info header err"));
		CLog::WriteLog(L"CMapView::ImportBmp() failed, read info header err");
		return FALSE;
	}

	if (bmpInfoHeader.biBitCount != 24) {
		CLog::WriteLog(L"CMapView::ImportBmp() failed, not a 24 bits bitmap");
		return FALSE;
	}

	fclose(fp);

	m_bmWidth = bmpInfoHeader.biWidth;	m_bmHeight = bmpInfoHeader.biHeight;
	if (m_hBmpOrigin) { ::DeleteObject(m_hBmpOrigin); }

	m_hBmpOrigin = (HBITMAP)::LoadImage(hInst, m_mapInfo->get_path(),
										IMAGE_BITMAP, m_bmWidth, m_bmHeight,
										LR_LOADFROMFILE | LR_CREATEDIBSECTION);

	DWORD dw = 0;
	if ((dw = GetLastError()) != 0) {
		CLog::WriteLog(L"CMapView::ImportBmp() failed, code %d", dw);
	}

	return m_hBmpOrigin != NULL;
}


void CMapView::OnPaint() 
{
	CPaintDC dc(this);

	if (!m_mapInfo) return;
	if (m_hBmpOrigin == NULL && !ImportBmp()) return;
	ASSERT(m_hBmpOrigin);

	CRect rc;
	GetClientRect(rc);
	HDC hdcMem;	hdcMem = ::CreateCompatibleDC(dc.m_hDC);	ASSERT(hdcMem);
	HGDIOBJ pOld = ::SelectObject(hdcMem, m_hBmpOrigin);
	SetStretchBltMode(dc.m_hDC, HALFTONE);
	StretchBlt(dc.m_hDC, rc.left, rc.top, rc.Width(), rc.Height(),
			   hdcMem, 0, 0, m_bmWidth, m_bmHeight, SRCCOPY);
	SelectObject(hdcMem, pOld);
	DeleteDC(hdcMem);
}


void CMapView::OnDestroy() 
{
	if (m_mapInfo) {
		m_mapInfo->SetInversionControlCallBack(NULL, NULL);
	}

	KillTimer(cTimerIDDrawAntLine);
	KillTimer(cTimerIDFlashSensor);
	KillTimer(cTimerIDRelayTraverseAlarmText);

	SAFEDELETEP(m_pAntLine);
	SAFEDELETEP(m_pTextDrawer);

	if (m_hBmpOrigin) { DeleteObject(m_hBmpOrigin); m_hBmpOrigin = NULL; }
	if (m_hDC4AntLine) { ::ReleaseDC(m_hWnd, m_hDC4AntLine); m_hDC4AntLine = NULL; }

	std::list<CDetector*>::iterator iter = m_detectorList.begin();
	while (iter != m_detectorList.end()) {
		CDetector* detector = *iter++;
		SAFEDELETEDLG(detector);
	}
	m_detectorList.clear();
}


void CMapView::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
	if (!m_mapInfo)
		return;

	/*LOG(L"CMapView::OnShowWindow %d, map: id %d, ademco_id %04d, machine_id %d, type %d",
		bShow, m_mapInfo->get_id(), m_machine->get_ademco_id(),
		m_mapInfo->get_machine_id(), m_mapInfo->get_type());*/

	if (bShow) {
		KillTimer(cTimerIDFlashSensor);
		SetTimer(cTimerIDFlashSensor, 500, NULL);
		m_pTextDrawer->Show();
	} else {
		KillTimer(cTimerIDDrawAntLine);
		KillTimer(cTimerIDFlashSensor);
		m_pAntLine->DeleteAllLine();
		m_pTextDrawer->Hide();
	}
}


void CMapView::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent) {
		case cTimerIDFlashSensor:
			FlushDetector();
			break;
		case cTimerIDDrawAntLine:
			CreateAntLine();
			break;
		case cTimerIDRelayTraverseAlarmText:
			KillTimer(cTimerIDRelayTraverseAlarmText);
			if (m_mapInfo) {
				m_mapInfo->TraverseAlarmText(this, OnInversionControlCommand);
			}
			break;
		default:
			break;
	} 

	CDialogEx::OnTimer(nIDEvent);
}


void CMapView::FlushDetector()
{
	if (m_bAlarming) {
		SetMode(MODE_NORMAL);
		KillTimer(cTimerIDFlashSensor);
		m_nFlashTimes = 0;
		KillTimer(cTimerIDDrawAntLine);
		SetTimer(cTimerIDDrawAntLine, 0, NULL);
		return;
	}

	CLocalLock lock(&m_csDetectorList);
	std::list<CDetector*>::iterator iter = m_detectorList.begin();
	if (m_nFlashTimes++ >= 4) {
		KillTimer(cTimerIDFlashSensor);
		m_nFlashTimes = 0;

		while (iter != m_detectorList.end()) {
			CDetector* pDet = *iter++;
			if (pDet && ::IsWindow(pDet->m_hWnd)) {
				pDet->SetFocus(FALSE);
				if (pDet->m_pPairDetector
					&& ::IsWindow(pDet->m_pPairDetector->m_hWnd)) {
					pDet->m_pPairDetector->SetFocus(FALSE);
				}
			}
		}

		KillTimer(cTimerIDDrawAntLine);
		SetTimer(cTimerIDDrawAntLine, 0, NULL);
	} else {
		while (iter != m_detectorList.end()) {
			CDetector* pDet = *iter++;
			if (pDet && !pDet->IsAlarming() && ::IsWindow(pDet->m_hWnd)) {
				pDet->SetFocus(m_nFlashTimes % 2 == 0);
				if (pDet->m_pPairDetector
					&& ::IsWindow(pDet->m_pPairDetector->m_hWnd)) {
					pDet->m_pPairDetector->SetFocus(m_nFlashTimes % 2 == 0);
				}
			}
		}
	}
}


void CMapView::CreateAntLine()
{
	KillTimer(cTimerIDDrawAntLine);
	CLocalLock lock(&m_csDetectorList);
	std::list<CDetector*>::iterator iter = m_detectorList.begin();
	while (iter != m_detectorList.end()) {
		CDetector* pDet = *iter++;
		if (!pDet->m_pPairDetector)
			continue;
		if (!::IsWindow(pDet->m_hWnd) || !::IsWindow(pDet->m_pPairDetector->m_hWnd)) {
			SetTimer(cTimerIDDrawAntLine, 1000, NULL);
			return;
		}

		int begs = pDet->GetPtn();
		int ends = pDet->m_pPairDetector->GetPtn();

		if (begs == ends) {
			CPoint *beg = NULL;
			CPoint *end = NULL;
			pDet->GetPts(beg);
			pDet->m_pPairDetector->GetPts(end);

			if (beg == NULL || end == NULL) {
				SetTimer(cTimerIDDrawAntLine, 1000, NULL);
				return;
			}

			for (int i = 0; i < begs; i++) {
				::ScreenToClient(m_hWnd, &beg[i]);
				::ScreenToClient(m_hWnd, &end[i]);
				m_pAntLine->AddLine(beg[i], end[i], reinterpret_cast<DWORD>(pDet));
			}
		}
	}
	if (m_hDC4AntLine == NULL)
		m_hDC4AntLine = ::GetDC(m_hWnd);
	m_pAntLine->ShowAntLine(m_hDC4AntLine, TRUE);
}


void CMapView::SetMode(MapViewMode mode)
{
	if (m_mode != mode) {
		m_mode = mode;
	}
}


afx_msg LRESULT CMapView::OnRepaint(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	Invalidate(0);
	return 0;
}


afx_msg LRESULT CMapView::OnNewAlarmTextResult(WPARAM wParam, LPARAM lParam)
{
	InversionControlCommand icc = static_cast<InversionControlCommand>(wParam);
	const AlarmText* at = reinterpret_cast<const AlarmText*>(lParam);
	switch (icc) {
		case core::ICC_ADD_ALARM_TEXT:
			if (at) {
				m_pTextDrawer->AddAlarmText(at->_txt, at->_zone, at->_subzone, at->_event);
				m_pTextDrawer->Show();
			}
		case core::ICC_SHOW:
			if (m_pRealParent) {
				m_pRealParent->SendMessage(WM_NEWALARMTEXT, reinterpret_cast<WPARAM>(this));
			}
			break;
		case core::ICC_CLR_ALARM_TEXT:
			m_pTextDrawer->Quit();
			break;
		case core::ICC_RENAME:
			break;
		case core::ICC_CHANGE_IMAGE:
			break;
		case core::ICC_DESTROY:
			m_mapInfo = NULL;
			break;
		default:
			break;
	}
	return 0;
}
