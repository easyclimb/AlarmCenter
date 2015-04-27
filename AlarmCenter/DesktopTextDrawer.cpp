// DesktopTextDrawer.cpp: implementation of the CDesktopTextDrawer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DesktopTextDrawer.h"
//#include "StaticVarable.h"
//#include "D:/Global/FileOper.h"
#include "AlarmTextDlg.h"



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace gui {

CDesktopTextDrawer::CDesktopTextDrawer()
	: m_nMaxLine(0)
	, m_cx(0)
	, m_cy(0)
	, m_height(ALARM_TEXT_HEIGHT+10)
	, m_nGapID(0)
	, m_pParentWnd(NULL)
{
	m_cx = ::GetSystemMetrics(SM_CXSCREEN);
	m_cy = ::GetSystemMetrics(SM_CYSCREEN) - ::GetSystemMetrics(SM_CYCAPTION) - ::GetSystemMetrics(SM_CYBORDER);
	m_nMaxLine = m_cy / m_height - 1;
	m_pAlarmTextInfoArr = new AlarmTextInfo[m_nMaxLine];
	InitializeCriticalSection(&m_cs);
}

CDesktopTextDrawer::~CDesktopTextDrawer()
{
	Quit();
	if (m_pAlarmTextInfoArr) {
		for (int i = 0; i < m_nMaxLine; i++) {
			SAFEDELETEDLG(m_pAlarmTextInfoArr[i].dlg);
		}
	}
	SAFEDELETEARR(m_pAlarmTextInfoArr);
	DeleteCriticalSection(&m_cs);
}

//BOOL CDesktopTextDrawer::IsZoneEventExists(int zone, int subzone, ADEMCO_EVENT ademco_event)
//{
//	CLocalLock lock(&m_cs);
//	for (int i = 0; i < m_nMaxLine; i++) {
//		if (m_pAlarmTextInfoArr[i].bUsed
//			&& m_pAlarmTextInfoArr[i].zone == zone
//			&& m_pAlarmTextInfoArr[i].subzone == subzone
//			&& m_pAlarmTextInfoArr[i].ademco_event == ademco_event) {
//			/*time_t now = time(NULL);
//			double seconds = difftime(now, m_pAlarmTextInfoArr[i]._time);
//			if (seconds > 5) {
//				return FALSE;
//			}*/
//			return TRUE;
//		}
//	}
//	return FALSE;
//}

void CDesktopTextDrawer::AddAlarmText(LPCTSTR szAlarm, int zone, int subzone, ADEMCO_EVENT ademco_event)
{
	CLocalLock lock(&m_cs);
	//if (IsZoneEventExists(zone, subzone, ademco_event))
	//	return;
	CLog::WriteLog(_T("CDesktopTextDrawer::AddAlarmText %s %03d %d\n"), szAlarm,
				   zone, ademco_event);
	BOOL bHasGap = FALSE;
	int idGap = 0;
	for (int i = 0; i < m_nMaxLine; i++)
	if (!m_pAlarmTextInfoArr[i].bUsed) {
		bHasGap = TRUE;
		idGap = i;
		break;
	}

	if (!bHasGap) {
		idGap = m_nGapID++ % m_nMaxLine;
		ShutdownSubProcess(idGap);
	}
	//CString alarm = _T("");
	//CTime now = CTime::GetCurrentTime();
	//alarm.Format(_T("%02d:%02d:%02d %s"), now.GetHour(), now.GetMinute(),
	//			 now.GetSecond(), szAlarm);
	m_pAlarmTextInfoArr[idGap].bUsed = TRUE;
	m_pAlarmTextInfoArr[idGap].bProcessStart = FALSE;
	m_pAlarmTextInfoArr[idGap].zone = zone;
	m_pAlarmTextInfoArr[idGap].subzone = subzone;
	m_pAlarmTextInfoArr[idGap].ademco_event = ademco_event;
	m_pAlarmTextInfoArr[idGap].string = szAlarm;
	//m_pAlarmTextInfoArr[idGap]._time = time(NULL);
	m_pAlarmTextInfoArr[idGap].color = ademco::GetEventLevelColor(ademco::GetEventLevel(ademco_event));
}

BOOL CDesktopTextDrawer::StartupSubProcess(int id)
{
	if (m_pAlarmTextInfoArr[id].dlg == NULL) {
		m_pAlarmTextInfoArr[id].dlg = new CAlarmTextDlg(m_pParentWnd);
	}

	m_pAlarmTextInfoArr[id].dlg->SetText(m_pAlarmTextInfoArr[id].string);
	if (m_pAlarmTextInfoArr[id].dlg->Create(IDD_DIALOG_ALARM_TEXT, m_pParentWnd)) {
		m_pAlarmTextInfoArr[id].bProcessStart = TRUE;
		CRect rcMap;
		m_pParentWnd->GetClientRect(rcMap);
		m_pParentWnd->ClientToScreen(rcMap);
		CRect rc;
		rc.left = rcMap.left;
		rc.top = rcMap.top + id * m_height;
		rc.right = rc.left + rcMap.Width();
		rc.bottom = rc.top + m_height;
		m_pParentWnd->ScreenToClient(rc);
		m_pAlarmTextInfoArr[id].dlg->MoveWindow(rc);
		m_pAlarmTextInfoArr[id].dlg->ShowWindow(SW_SHOW);
		m_pAlarmTextInfoArr[id].dlg->SetColor(m_pAlarmTextInfoArr[id].color);
		return TRUE;
	} else {
		delete m_pAlarmTextInfoArr[id].dlg;
		m_pAlarmTextInfoArr[id].dlg = NULL;
		ASSERT(0);
	}

	return FALSE;
}

BOOL CDesktopTextDrawer::ShutdownSubProcess(int id)
{
	if (!m_pAlarmTextInfoArr[id].bUsed)
		return FALSE;
	CLog::WriteLog(_T("CDesktopTextDrawer::ShutdownSubProcess %s %03d %d\n"),
		  m_pAlarmTextInfoArr[id].string, m_pAlarmTextInfoArr[id].zone, 
		  m_pAlarmTextInfoArr[id].ademco_event);
	if (m_pAlarmTextInfoArr[id].dlg) {
		if (::IsWindow(m_pAlarmTextInfoArr[id].dlg->m_hWnd)) {
			m_pAlarmTextInfoArr[id].dlg->DestroyWindow();
		}
		delete m_pAlarmTextInfoArr[id].dlg;
		m_pAlarmTextInfoArr[id].dlg = NULL;
	}
	m_pAlarmTextInfoArr[id].bUsed = FALSE;
	m_pAlarmTextInfoArr[id].bProcessStart = FALSE;
	m_pAlarmTextInfoArr[id].zone = -1;
	m_pAlarmTextInfoArr[id].subzone = -1;
	//m_pAlarmTextInfoArr[id].idThread = 0xffffffff;
	m_pAlarmTextInfoArr[id].string.Empty();
	return TRUE;
}

void CDesktopTextDrawer::Quit()
{
	CLocalLock lock(&m_cs);
	for (int i = 0; i < m_nMaxLine; i++) {
		if (m_pAlarmTextInfoArr[i].bUsed)
			ShutdownSubProcess(i);
	}
	if (m_pParentWnd && ::IsWindow(m_pParentWnd->m_hWnd)) {
		::PostMessage(m_pParentWnd->m_hWnd, WM_REPAINT, 0, 0);
	}
	m_nGapID = 0;
}

void CDesktopTextDrawer::Show()
{
	CLocalLock lock(&m_cs);
	for (int i = 0; i < m_nMaxLine; i++) {
		if (m_pAlarmTextInfoArr[i].bUsed) {
			if (m_pAlarmTextInfoArr[i].bProcessStart) {
				m_pAlarmTextInfoArr[i].dlg->Show();
			} else {
				StartupSubProcess(i);
				m_pAlarmTextInfoArr[i].dlg->Show();
			}

			//if (i % 2 != 0) {
			//	m_pAlarmTextInfoArr[i].dlg->Orange();
			//}
		}
	}
}

void CDesktopTextDrawer::Hide()
{
	CLocalLock lock(&m_cs);
	for (int i = 0; i < m_nMaxLine; i++) {
		if (m_pAlarmTextInfoArr[i].bUsed && m_pAlarmTextInfoArr[i].bProcessStart) {
			m_pAlarmTextInfoArr[i].dlg->DestroyWindow();
			m_pAlarmTextInfoArr[i].bProcessStart = FALSE;
		}
	}
	if (m_pParentWnd && ::IsWindow(m_pParentWnd->m_hWnd)) {
		::PostMessage(m_pParentWnd->m_hWnd, WM_REPAINT, 0, 0);
	}
}

void CDesktopTextDrawer::DeleteAlarmText(int zone, int subzone, ADEMCO_EVENT ademco_event)
{
	CLocalLock lock(&m_cs);
	for (int i = 0; i < m_nMaxLine; i++) {
		if (m_pAlarmTextInfoArr[i].zone == zone 
			&& m_pAlarmTextInfoArr[i].subzone == subzone
			&& m_pAlarmTextInfoArr[i].ademco_event == ademco_event) {
			ShutdownSubProcess(i);
			break;
		}
	}
	if (m_pParentWnd && ::IsWindow(m_pParentWnd->m_hWnd)) {
		::PostMessage(m_pParentWnd->m_hWnd, WM_REPAINT, 0, 0);
	}
}

BOOL CDesktopTextDrawer::GetZoneEvent(int zone, int subzone, int& ademco_event)
{
	CLocalLock lock(&m_cs);
	for (int i = 0; i < m_nMaxLine; i++) {
		if (m_pAlarmTextInfoArr[i].zone == zone 
			&& m_pAlarmTextInfoArr[i].subzone == subzone) {
			ademco_event = m_pAlarmTextInfoArr[i].ademco_event;
			return TRUE;
			break;
		}
	}
	return FALSE;
}

int CDesktopTextDrawer::GetCount()
{
	CLocalLock lock(&m_cs);
	int count = 0;
	for (int i = 0; i < m_nMaxLine; i++) {
		if (m_pAlarmTextInfoArr[i].bUsed) {
			count++;
		}
	}
	return count;
}

BOOL CDesktopTextDrawer::IsThisZoneAlarming(int zone, int subzone)
{
	CLocalLock lock(&m_cs);
	for (int i = 0; i < m_nMaxLine; i++) {
		if (m_pAlarmTextInfoArr[i].zone == zone 
			&& m_pAlarmTextInfoArr[i].subzone == subzone
			&& m_pAlarmTextInfoArr[i].bUsed) {
			return TRUE;
		}
	}
	return FALSE;
}

NAMESPACE_END
