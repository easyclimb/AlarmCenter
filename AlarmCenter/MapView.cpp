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

static void __stdcall TraverseZoneOfMap(void* udata, CZoneInfo* zoneInfo)
{
	CMapView* mapView = reinterpret_cast<CMapView*>(udata); assert(mapView);
	//mapView->TraverseZoneOfMapResult(zoneInfo);
	mapView->SendMessage(WM_TRAVERSEZONE, (WPARAM)zoneInfo);
}

static void __stdcall OnNewAlarmText(void* udata, const AlarmText* at)
{
	CMapView* mapView = reinterpret_cast<CMapView*>(udata); assert(mapView);
	mapView->SendMessage(WM_NEWALARMTEXT, (WPARAM)at);
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
	, m_hDC(NULL)
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
	ON_MESSAGE(WM_ADEMCOEVENT, &CMapView::OnAdemcoEvent)
	ON_MESSAGE(WM_TRAVERSEZONE, &CMapView::OnTraversezone)
	ON_MESSAGE(WM_NEWALARMTEXT, &CMapView::OnNewAlarmTextResult)
END_MESSAGE_MAP()


// CMapView message handlers

BOOL CMapView::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	if (m_mapInfo && m_machine) {

		CString txt;
		txt.Format(L"map: id %d, ademco_id %04d, machine_id %d, type %d", 
				   m_mapInfo->get_id(), m_machine->get_ademco_id(), 
				   m_mapInfo->get_machine_id(), m_mapInfo->get_type());
		m_pAntLine = new gui::CAntLine(txt);

		m_pTextDrawer = new gui::CDesktopTextDrawer();
		m_pTextDrawer->SetOwner(this);

		//m_machine->TraverseZoneOfMap(m_mapInfo->get_id(), this, TraverseZoneOfMap);
		if (m_mapInfo->get_id() != -1) {
			/*CZoneInfo* zoneInfo = m_mapInfo->GetFirstZoneInfo();
			while (zoneInfo) {
				CDetector* detector = new CDetector(zoneInfo, NULL, this);
				if (detector->CreateDetector()) {
					m_detectorList.push_back(detector);
				}
				zoneInfo = m_mapInfo->GetNextZoneInfo();
			}*/
			CZoneInfoList list;
			m_mapInfo->GetAllZoneInfo(list);
			CZoneInfoListIter iter = list.begin();
			while (iter != list.end()) {
				CZoneInfo* zoneInfo = *iter++;
				CDetector* detector = new CDetector(zoneInfo, NULL, this);
				if (detector->CreateDetector()) {
					m_detectorList.push_back(detector);
				}
			}
		}

		m_mapInfo->SetNewAlarmTextCallBack(this, OnNewAlarmText);
		m_mapInfo->TraverseAlarmText(this, OnNewAlarmText);
	}

	return TRUE;  
}


afx_msg LRESULT CMapView::OnTraversezone(WPARAM wParam, LPARAM)
{
	CZoneInfo* zoneInfo = reinterpret_cast<CZoneInfo*>(wParam);
	TraverseZoneOfMapResult(zoneInfo);
	return 0;
}


void CMapView::TraverseZoneOfMapResult(CZoneInfo* zoneInfo)
{
	assert(zoneInfo);
	CDetector* detector = new CDetector(zoneInfo, NULL, this);
	if (detector->CreateDetector()) {
		m_detectorList.push_back(detector);
	}
}


BOOL CMapView::IsThisYourZone(int zone_id)
{
	/*std::list<CDetector*>::iterator iter = m_detectorList.begin();
	while (iter != m_detectorList.end()) {
		CDetector* detector = *iter++;
		if (detector->GetZoneID() == zone) {
			return TRUE;
		}
	}*/

	if (m_mapInfo) {
		CZoneInfo* zone = m_mapInfo->GetZoneInfo(zone_id);
		return (zone != NULL);
	}

	return FALSE;
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
	KillTimer(cTimerIDDrawAntLine);
	KillTimer(cTimerIDFlashSensor);

	SAFEDELETEP(m_pAntLine);
	SAFEDELETEP(m_pTextDrawer);

	if (m_hBmpOrigin) { DeleteObject(m_hBmpOrigin); m_hBmpOrigin = NULL; }
	if (m_hDC)	::ReleaseDC(m_hWnd, m_hDC);	m_hDC = NULL;

	std::list<CDetector*>::iterator iter = m_detectorList.begin();
	while (iter != m_detectorList.end()) {
		CDetector* detector = *iter++;
		SAFEDELETEDLG(detector);
	}
}


void CMapView::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
	if (!m_mapInfo)
		return;

	LOG(L"CMapView::OnShowWindow %d, map: id %d, ademco_id %04d, machine_id %d, type %d",
		bShow, m_mapInfo->get_id(), m_machine->get_ademco_id(),
		m_mapInfo->get_machine_id(), m_mapInfo->get_type());

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
	if (nIDEvent == cTimerIDFlashSensor) {
		FlushDetector();
	} else if (nIDEvent == cTimerIDDrawAntLine) {
		CreateAntLine();
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
	if (m_hDC == NULL) m_hDC = ::GetDC(m_hWnd);
	m_pAntLine->ShowAntLine(m_hDC, TRUE);
}


void CMapView::SetMode(MapViewMode mode)
{
	if (m_mode != mode) {
		m_mode = mode;
	}
}


int CMapView::GetAdemcoID() const
{
	if (m_machine) {
		return m_machine->get_ademco_id();
	}
	return -1;
}

//
//CDetector* CMapView::GetDetector(int zone)
//{
//	CLocalLock lock(&m_csDetectorList);
//	std::list<CDetector*>::iterator iter = m_detectorList.begin();
//	while (iter != m_detectorList.end()) {
//		CDetector* pDet = *iter++;
//		if (zone == pDet->GetZoneID()) {
//			return pDet;
//		}
//	}
//
//	return NULL;
//}


void CMapView::HandleAdemcoEvent(const ademco::AdemcoEvent* ademcoEvent)
{
	if (!IsWindow(m_hWnd) || !ademcoEvent)
		return;

	//ademco::AdemcoEvent* ademcoEvent = new ademco::AdemcoEvent(zone, ademco_event, event_time);
	SendMessage(WM_ADEMCOEVENT, (WPARAM)ademcoEvent);
}


afx_msg LRESULT CMapView::OnRepaint(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	Invalidate(0);
	return 0;
}


afx_msg LRESULT CMapView::OnAdemcoEvent(WPARAM wParam, LPARAM /*lParam*/)
{
	//if (m_machine == NULL)
	//	return 0;

	//ademco::AdemcoEvent* ademcoEvent = reinterpret_cast<ademco::AdemcoEvent*>(wParam);
	//ASSERT(ademcoEvent);

	//int zone = ademcoEvent->_zone;
	//int ademco_event = ademcoEvent->_event;
	//time_t event_time = ademcoEvent->_time;
	//wchar_t wtime[32] = { 0 };
	//struct tm tmtm;
	//localtime_s(&tmtm, &event_time);
	//if (tmtm.tm_year == 1900) {
	//	event_time = time(NULL);
	//	localtime_s(&tmtm, &event_time);
	//} 
	//wcsftime(wtime, 32, L"%H:%M:%S", &tmtm);

	//CString stime = wtime, text, alarmText;
	//
	//if (zone != 0) {
	//	CString fmZone, prefix;
	//	fmZone.LoadStringW(IDS_STRING_ZONE);
	//	prefix.Format(L" %s%03d", fmZone, zone);
	//	text += prefix;
	//}

	//CDetector* detector = GetDetector(zone);
	//if (detector) { // has detector
	//	detector->Alarm(TRUE);
	//	detector->FormatAlarmText(alarmText, ademco_event);
	//} else {		// has not detector
	//	CString strEvent = L"";
	//	CString alias = L""; 
	//	CZoneInfo* zoneInfo = m_mapInfo->GetZoneInfo(zone);
	//	CZonePropertyData* data = NULL;
	//	if (zoneInfo) {
	//		CZonePropertyInfo* info = CZonePropertyInfo::GetInstance();
	//		data = info->GetZonePropertyDataById(zoneInfo->get_property_id());
	//		alias = zoneInfo->get_alias();
	//	} 

	//	if (alias.IsEmpty()) {
	//		CString fmNull;
	//		fmNull.LoadStringW(IDS_STRING_NULL);
	//		alias = fmNull;
	//	}

	//	if (ademco::IsExceptionEvent(ademco_event) || (data == NULL)) { // �쳣��Ϣ������ event ��ʾ����
	//		CAppResource* res = CAppResource::GetInstance();
	//		CString strEvent = res->AdemcoEventToString(ademco_event);
	//		alarmText.Format(L"%s(%s)", strEvent, alias);
	//	} else { // ������Ϣ������ �ֶ����õı������� �� event ��ʾ����
	//		alarmText.Format(L"%s(%s)", data->get_alarm_text(), alias);
	//	}
	//}

	//text += L" " + alarmText;
	//m_pTextDrawer->AddAlarmText(stime + text, zone, ademco_event);
	//m_pTextDrawer->Show();

	// CHistoryRecord *hr = CHistoryRecord::GetInstance();
	// hr->InsertRecord(m_machine->get_ademco_id(), text, event_time, RECORD_LEVEL_ALARM);
	return 0;
}


void CMapView::ClearMsg()
{
	m_pTextDrawer->Quit();

	std::list<CDetector*>::iterator iter = m_detectorList.begin();
	while (iter != m_detectorList.end()) {
		CDetector* detector = *iter++;
		detector->Alarm(FALSE);
	}
}


afx_msg LRESULT CMapView::OnNewAlarmTextResult(WPARAM wParam, LPARAM lParam)
{
	const AlarmText* at = reinterpret_cast<const AlarmText*>(wParam);
	ASSERT(at);

	if (m_pRealParent) {
		m_pRealParent->SendMessage(WM_NEWALARMTEXT, reinterpret_cast<WPARAM>(this));
	}

	m_pTextDrawer->AddAlarmText(at->_txt, at->_zone, at->_subzone, at->_event);
	m_pTextDrawer->Show();

	return 0;
}
