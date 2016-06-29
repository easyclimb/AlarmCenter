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
#include "ZoneInfo.h"
#include "AntLine.h"
#include "DesktopTextDrawer.h"
#include "AlarmMachineDlg.h"


using namespace core;

namespace detail {
	static const int cTimerIDDrawAntLine = 1;
	static const int cTimerIDFlashSensor = 2;
	static const int cTimerIDRelayTraverseAlarmText = 3;
	static const int cTimerIDHandleIcmc = 4;

	
	//std::list<icmc_buffer*> g_icmcBufferList;

	/*void OnInversionControlCommand(const CMapViewPtr& mapView,
								   const icmc_buffer_ptr& icmc)
	{
		if (mapView) {
			mapView->AddIcmc(icmc);
		}
	}*/
};

IMPLEMENT_DYNAMIC(CMapView, CDialogEx)

CMapView::CMapView(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CMapView::IDD, pParent)
	, m_machine(nullptr)
	, m_mapInfo(nullptr)
	, m_hBmpOrigin(nullptr)
	, m_bmWidth(0)
	, m_bmHeight(0)
	, m_detectorList()
	, m_pAntLine(nullptr)
	, m_pTextDrawer(nullptr)
	, m_bAlarming(FALSE)
	, m_mode(MODE_NORMAL)
	, m_nFlashTimes(0)
	, m_hDC4AntLine(nullptr)
	, m_pRealParent(nullptr)
{
}


CMapView::~CMapView()
{
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
END_MESSAGE_MAP()


// CMapView message handlers

BOOL CMapView::OnInitDialog()
{
	AUTO_LOG_FUNCTION;
	CDialogEx::OnInitDialog();
	
	if (m_mapInfo && m_machine) {
		m_bAlarming = m_mapInfo->get_alarming();
		m_pAntLine = std::make_unique<gui::CAntLine>();
		m_pTextDrawer = std::make_unique<gui::CDesktopTextDrawer>();
		m_pTextDrawer->SetOwner(this);

		if (m_mapInfo->get_id() != -1) {
			std::list<core::detector_bind_interface_ptr> list;
			m_mapInfo->GetAllInterfaceInfo(list);
			for (auto pInterface : list) {
				//if (DIT_ZONE_INFO == pInterface->GetInterfaceType()) {
				auto detector = std::shared_ptr<CDetector>(new CDetector(pInterface, nullptr), [](CDetector* det) {SAFEDELETEDLG(det); });
				if (detector->CreateDetector(this)) {
					pInterface->SetInversionControlCallback(detector, CDetector::OnInversionControlZone);
					m_detectorList.push_back(detector);
				}
				//}
			}
		}

		m_icmc_observer = std::make_shared<IcmcObserver>(this);
		//m_mapInfo->SetInversionControlCallBack(shared_from_this(), OnInversionControlCommand);
		m_mapInfo->register_observer(m_icmc_observer);
		SetTimer(detail::cTimerIDRelayTraverseAlarmText, 500, nullptr);
		//m_mapInfo->TraverseAlarmText(this, OnNewAlarmText);
		SetTimer(detail::cTimerIDHandleIcmc, 200, nullptr);
	}

	return TRUE;  
}


BOOL CMapView::ImportBmp()
{
	AUTO_LOG_FUNCTION;
	ASSERT(m_mapInfo);
	HINSTANCE hInst = (HINSTANCE)::GetWindowLong(m_hWnd, GWL_HINSTANCE);
	FILE *fp = nullptr;
	_tfopen_s(&fp, m_mapInfo->get_path(), _T("rb"));
	if (fp == nullptr) { return FALSE; }

	BITMAPFILEHEADER bmpFileHeader;
	BITMAPINFOHEADER bmpInfoHeader;
	unsigned int uiRead = fread(&bmpFileHeader, 1, sizeof(bmpFileHeader), fp);

	if (uiRead != sizeof(bmpFileHeader)) {
		JLOG(L"CMapView::ImportBmp() failed, read file header err");
		return FALSE;
	}

	if (bmpFileHeader.bfType != 0x4d42) {
		JLOG(L"CMapView::ImportBmp() failed, not a bmp file");
		return FALSE;
	}

	if ((fread(&bmpInfoHeader, 1, sizeof(bmpInfoHeader), fp)) != sizeof(bmpInfoHeader)) {		//SHOWERROR(_T("read info header err"));
		JLOG(L"CMapView::ImportBmp() failed, read info header err");
		return FALSE;
	}

	if (bmpInfoHeader.biBitCount != 24) {
		JLOG(L"CMapView::ImportBmp() failed, not a 24 bits bitmap");
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
		JLOG(L"CMapView::ImportBmp() failed, code %d", dw);
	}

	return m_hBmpOrigin != nullptr;
}


void CMapView::OnPaint() 
{
	AUTO_LOG_FUNCTION;
	CPaintDC dc(this);

	if (!m_mapInfo) return;
	if (m_hBmpOrigin == nullptr && !ImportBmp()) return;
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
	AUTO_LOG_FUNCTION;
	m_icmc_observer = nullptr;
	m_mapInfo = nullptr;

	KillTimer(detail::cTimerIDDrawAntLine);
	KillTimer(detail::cTimerIDFlashSensor);
	KillTimer(detail::cTimerIDRelayTraverseAlarmText);
	KillTimer(detail::cTimerIDHandleIcmc);

	m_icmcList.clear();

	m_pAntLine = nullptr;
	m_pTextDrawer = nullptr;

	if (m_hBmpOrigin) { DeleteObject(m_hBmpOrigin); m_hBmpOrigin = nullptr; }
	if (m_hDC4AntLine) { ::ReleaseDC(m_hWnd, m_hDC4AntLine); m_hDC4AntLine = nullptr; }

	m_detectorList.clear();
}


void CMapView::OnShowWindow(BOOL bShow, UINT nStatus)
{
	AUTO_LOG_FUNCTION;
	CDialogEx::OnShowWindow(bShow, nStatus);
	if (!m_mapInfo)
		return;

	if (bShow && (MODE_NORMAL == m_mode)) {
		auto_timer timer(m_hWnd, detail::cTimerIDFlashSensor, 500);
		m_pTextDrawer->Hide();
		m_pTextDrawer->Show();
	} else {
		KillTimer(detail::cTimerIDDrawAntLine);
		KillTimer(detail::cTimerIDFlashSensor);
		m_pAntLine->DeleteAllLine();
		m_pTextDrawer->Hide();
	}
}


void CMapView::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent) {
		case detail::cTimerIDFlashSensor:
			FlushDetector();
			break;
		case detail::cTimerIDDrawAntLine:
			CreateAntLine();
			break;
		case detail::cTimerIDRelayTraverseAlarmText:
			KillTimer(detail::cTimerIDRelayTraverseAlarmText);
			if (m_mapInfo) {
				m_mapInfo->TraverseAlarmText(m_icmc_observer);
			}
			break;
		case detail::cTimerIDHandleIcmc:
			if (m_icmcLock.try_lock()){
				std::lock_guard<std::mutex> lock(m_icmcLock, std::adopt_lock);
				for(auto icmc : m_icmcList) {
					OnInversionControlResult(icmc->_icmc, icmc->_at);
				}
				m_icmcList.clear();
			}
			break;
		default:
			break;
	} 

	CDialogEx::OnTimer(nIDEvent);
}


void CMapView::FlushDetector()
{
	AUTO_LOG_FUNCTION;
	if (m_bAlarming) {
		KillTimer(detail::cTimerIDFlashSensor);
		m_nFlashTimes = 0;
		auto_timer timer(m_hWnd, detail::cTimerIDDrawAntLine, 1);
		return;
	}

	std::lock_guard<std::mutex> lock(m_csDetectorList);
	if (m_nFlashTimes++ >= 4) {
		KillTimer(detail::cTimerIDFlashSensor);
		m_nFlashTimes = 0;

		for (auto detector : m_detectorList) {
			if (detector && ::IsWindow(detector->m_hWnd)) {
				detector->SetFocus(FALSE);
			}
		}

		auto_timer timer(m_hWnd, detail::cTimerIDDrawAntLine, 0);
	} else {
		for (auto detector : m_detectorList) {
			if (detector && !detector->IsAlarming() && ::IsWindow(detector->m_hWnd)) {
				detector->SetFocus(m_nFlashTimes % 2 == 0);
			}
		}
	}
}


void CMapView::CreateAntLine()
{
	AUTO_LOG_FUNCTION;
	KillTimer(detail::cTimerIDDrawAntLine);
	std::lock_guard<std::mutex> lock(m_csDetectorList);
	for (auto detector : m_detectorList) {
		if (!detector->m_pPairDetector)
			continue;
		if (!::IsWindow(detector->m_hWnd) || !::IsWindow(detector->m_pPairDetector->m_hWnd)) {
			SetTimer(detail::cTimerIDDrawAntLine, 1000, nullptr);
			return;
		}

		int begs = detector->GetPtn();
		int ends = detector->m_pPairDetector->GetPtn();

		if (begs == ends) {
			CPoint *beg = nullptr;
			CPoint *end = nullptr;
			detector->GetPts(beg);
			detector->m_pPairDetector->GetPts(end);

			if (beg == nullptr || end == nullptr) {
				SetTimer(detail::cTimerIDDrawAntLine, 1000, nullptr);
				return;
			}

			for (int i = 0; i < begs; i++) {
				::ScreenToClient(m_hWnd, &beg[i]);
				::ScreenToClient(m_hWnd, &end[i]);
				m_pAntLine->AddLine(beg[i], end[i], detector);
			}
		}
	}
	if (m_hDC4AntLine == nullptr)
		m_hDC4AntLine = ::GetDC(m_hWnd);
	m_pAntLine->ShowAntLine(m_hDC4AntLine, TRUE);
}


void CMapView::SetMode(MapViewMode mode)
{
	AUTO_LOG_FUNCTION;
	JLOG(L"mode: %d\n", mode);
	if (m_mode != mode) {
		m_mode = mode;
		if (MODE_EDIT == mode) {
			KillTimer(detail::cTimerIDDrawAntLine);
			KillTimer(detail::cTimerIDFlashSensor);
			KillTimer(detail::cTimerIDRelayTraverseAlarmText);
			m_pAntLine->DeleteAllLine();
			m_pTextDrawer->Hide();
		} else if (MODE_NORMAL == mode) {
			SetTimer(detail::cTimerIDFlashSensor, 500, nullptr);
			m_pTextDrawer->Show();
		} 
	}
}


afx_msg LRESULT CMapView::OnRepaint(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	Invalidate(0);
	return 0;
}


void CMapView::OnInversionControlResult(core::inversion_control_map_command icmc, const core::alarm_text_ptr& at)
{
	AUTO_LOG_FUNCTION;
	switch (icmc) {
		case core::ICMC_ADD_ALARM_TEXT:
			if (at) {
				m_pTextDrawer->AddAlarmText(at->_txt, at->_zone, at->_subzone, at->_event);
				m_pTextDrawer->Show();
				TellParent2ShowMyTab(ICMC_ADD_ALARM_TEXT);
			}
			break;
		case core::ICMC_DEL_ALARM_TEXT:
			if (at) {
				m_pTextDrawer->DeleteAlarmText(at->_zone, at->_subzone, at->_event);
				if (m_pTextDrawer->GetCount() == 0) {
					TellParent2ShowMyTab(ICMC_CLR_ALARM_TEXT);
				}
			}
			break;
		case core::ICMC_SHOW:
			TellParent2ShowMyTab(ICMC_SHOW);
			//OnShowWindow(TRUE, SW_NORMAL);
			break;
		case core::ICMC_CLR_ALARM_TEXT:
			m_pTextDrawer->Quit();
			TellParent2ShowMyTab(ICMC_CLR_ALARM_TEXT);
			break;
		case core::ICMC_MODE_EDIT:
			SetMode(MODE_EDIT);
			break;
		case core::ICMC_MODE_NORMAL:
			SetMode(MODE_NORMAL);
			break;
		case core::ICMC_RENAME:
			if (m_pRealParent && IsWindow(m_pRealParent->m_hWnd)) {
				CAlarmMachineDlg* dlg = static_cast<CAlarmMachineDlg*>(m_pRealParent);
				dlg->OnInversionControl(shared_from_this(), ICMC_RENAME);
			}
			break;
		case core::ICMC_CHANGE_IMAGE:
			if (m_hBmpOrigin) { DeleteObject(m_hBmpOrigin); m_hBmpOrigin = nullptr; }
			Invalidate(0);
			break;
		case core::ICMC_NEW_DETECTOR:
			OnNewDetector();
			break;
		case core::ICMC_DEL_DETECTOR:
			OnDelDetector();
			break;
		//case core::ICMC_DESTROY:
		//	m_mapInfo = nullptr;
		//	break;
		default:
			break;
	}
}


void CMapView::TellParent2ShowMyTab(core::inversion_control_map_command cmd)
{
	if (m_pRealParent && IsWindow(m_pRealParent->m_hWnd)) {
		CAlarmMachineDlg* dlg = static_cast<CAlarmMachineDlg*>(m_pRealParent);
		dlg->OnInversionControl(shared_from_this(), cmd);
	}
}


void CMapView::OnNewDetector()
{
	AUTO_LOG_FUNCTION;
	ASSERT(m_mapInfo);
	detector_bind_interface_ptr pInterface = m_mapInfo->GetActiveInterfaceInfo();
	if (pInterface) {
		auto detector = std::shared_ptr<CDetector>(new CDetector(pInterface, nullptr), [](CDetector* det) {SAFEDELETEDLG(det); });
		if (detector->CreateDetector(this)) {
			pInterface->SetInversionControlCallback(detector, CDetector::OnInversionControlZone);
			m_detectorList.push_back(detector);
		}
	}
}


void CMapView::OnDelDetector()
{
	AUTO_LOG_FUNCTION;
	ASSERT(m_mapInfo);
	detector_bind_interface_ptr pInterface = m_mapInfo->GetActiveInterfaceInfo();
	if (pInterface) {
		for (auto detector : m_detectorList) {
			if (detector->GetInterfaceInfo() == pInterface) {
				detector->DestroyWindow();
				m_detectorList.remove(detector);
				break;
			}
		}

	}
}
