// DesktopTextDrawer.cpp: implementation of the CDesktopTextDrawer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DesktopTextDrawer.h"
//#include "StaticVarable.h"
//#include "D:/Global/FileOper.h"
#include "AlarmTextDlg.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace gui {

CDesktopTextDrawer::CDesktopTextDrawer()
	: m_nMaxLine(0)
	, m_cx(0)
	, m_cy(0)
	, m_height(detail::ALARM_TEXT_HEIGHT+10)
	, m_nGapID(0)
	, m_pParentWnd(nullptr)
{
	m_cx = ::GetSystemMetrics(SM_CXSCREEN);
	m_cy = ::GetSystemMetrics(SM_CYSCREEN) - ::GetSystemMetrics(SM_CYCAPTION) - ::GetSystemMetrics(SM_CYBORDER);
	m_nMaxLine = m_cy / m_height - 2;
	for (int i = 0; i < m_nMaxLine; i++) {
		m_alarmTextMap[i] = std::make_shared<AlarmTextInfo>();
	}
}

CDesktopTextDrawer::~CDesktopTextDrawer()
{
	Quit();
	m_alarmTextMap.clear();
}


void CDesktopTextDrawer::AddAlarmText(LPCTSTR szAlarm, int zone, int subzone, ADEMCO_EVENT ademco_event)
{
	std::lock_guard<std::mutex> lock(m_cs);
	//if (IsZoneEventExists(zone, subzone, ademco_event))
	//	return;
	CLog::WriteLog(_T("CDesktopTextDrawer::AddAlarmText %s %03d %d\n"), szAlarm,
				   zone, ademco_event);
	BOOL bHasGap = FALSE;
	int idGap = 0;
	for (int i = 0; i < m_nMaxLine; i++)
	if (!m_alarmTextMap[i]->bUsed) {
		bHasGap = TRUE;
		idGap = i;
		break;
	}

	if (!bHasGap) {
		idGap = m_nGapID++ % m_nMaxLine;
		ShutdownSubProcess(idGap);
	}
	m_alarmTextMap[idGap]->bUsed = TRUE;
	m_alarmTextMap[idGap]->bProcessStart = FALSE;
	m_alarmTextMap[idGap]->zone = zone;
	m_alarmTextMap[idGap]->subzone = subzone;
	m_alarmTextMap[idGap]->ademco_event = ademco_event;
	m_alarmTextMap[idGap]->string = szAlarm;
	m_alarmTextMap[idGap]->color = ademco::GetEventLevelColor(ademco::GetEventLevel(ademco_event));
}

BOOL CDesktopTextDrawer::StartupSubProcess(int id)
{
	if (m_alarmTextMap[id]->dlg == nullptr) {
		m_alarmTextMap[id]->dlg = std::shared_ptr<CAlarmTextDlg>(new CAlarmTextDlg(m_pParentWnd), 
																 [](CAlarmTextDlg* dlg) {SAFEDELETEDLG(dlg); });
	}

	m_alarmTextMap[id]->dlg->SetText(m_alarmTextMap[id]->string);
	if (m_alarmTextMap[id]->dlg->Create(IDD_DIALOG_ALARM_TEXT, m_pParentWnd)) {
		m_alarmTextMap[id]->bProcessStart = TRUE;
		CRect rcMap;
		m_pParentWnd->GetClientRect(rcMap);
		m_pParentWnd->ClientToScreen(rcMap);
		CRect rc;
		rc.left = rcMap.left;
		rc.top = rcMap.top + id * m_height;
		rc.right = rc.left + rcMap.Width();
		rc.bottom = rc.top + m_height;
		m_pParentWnd->ScreenToClient(rc);
		m_alarmTextMap[id]->dlg->MoveWindow(rc);
		m_alarmTextMap[id]->dlg->ShowWindow(SW_SHOW);
		m_alarmTextMap[id]->dlg->SetColor(m_alarmTextMap[id]->color);
		return TRUE;
	} else {
		m_alarmTextMap[id]->dlg = nullptr;
		ASSERT(0);
	}

	return FALSE;
}

BOOL CDesktopTextDrawer::ShutdownSubProcess(int id)
{
	if (!m_alarmTextMap[id]->bUsed)
		return FALSE;
	CLog::WriteLog(_T("CDesktopTextDrawer::ShutdownSubProcess %s %03d %d\n"),
		  m_alarmTextMap[id]->string, m_alarmTextMap[id]->zone, 
		  m_alarmTextMap[id]->ademco_event);
	if (m_alarmTextMap[id]->dlg) {
		m_alarmTextMap[id]->dlg = nullptr;
	}
	m_alarmTextMap[id]->bUsed = FALSE;
	m_alarmTextMap[id]->bProcessStart = FALSE;
	m_alarmTextMap[id]->zone = -1;
	m_alarmTextMap[id]->subzone = -1;
	m_alarmTextMap[id]->string.Empty();
	return TRUE;
}

void CDesktopTextDrawer::Quit()
{
	std::lock_guard<std::mutex> lock(m_cs);
	for (int i = 0; i < m_nMaxLine; i++) {
		if (m_alarmTextMap[i]->bUsed)
			ShutdownSubProcess(i);
	}
	if (m_pParentWnd && ::IsWindow(m_pParentWnd->m_hWnd)) {
		::PostMessage(m_pParentWnd->m_hWnd, WM_REPAINT, 0, 0);
	}
	m_nGapID = 0;
}

void CDesktopTextDrawer::Show()
{
	std::lock_guard<std::mutex> lock(m_cs);
	for (int i = 0; i < m_nMaxLine; i++) {
		if (m_alarmTextMap[i]->bUsed) {
			if (m_alarmTextMap[i]->bProcessStart) {
				m_alarmTextMap[i]->dlg->Show();
			} else {
				StartupSubProcess(i);
				m_alarmTextMap[i]->dlg->Show();
			}
		}
	}
}

void CDesktopTextDrawer::Hide()
{
	std::lock_guard<std::mutex> lock(m_cs);
	for (int i = 0; i < m_nMaxLine; i++) {
		if (m_alarmTextMap[i]->bUsed && m_alarmTextMap[i]->bProcessStart) {
			m_alarmTextMap[i]->dlg->DestroyWindow();
			m_alarmTextMap[i]->bProcessStart = FALSE;
		}
	}
	if (m_pParentWnd && ::IsWindow(m_pParentWnd->m_hWnd)) {
		::PostMessage(m_pParentWnd->m_hWnd, WM_REPAINT, 0, 0);
	}
}

void CDesktopTextDrawer::DeleteAlarmText(int zone, int subzone, ADEMCO_EVENT ademco_event)
{
	std::lock_guard<std::mutex> lock(m_cs);
	for (int i = 0; i < m_nMaxLine; i++) {
		if (m_alarmTextMap[i]->zone == zone 
			&& m_alarmTextMap[i]->subzone == subzone
			&& m_alarmTextMap[i]->ademco_event == ademco_event) {
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
	std::lock_guard<std::mutex> lock(m_cs);
	for (int i = 0; i < m_nMaxLine; i++) {
		if (m_alarmTextMap[i]->zone == zone 
			&& m_alarmTextMap[i]->subzone == subzone) {
			ademco_event = m_alarmTextMap[i]->ademco_event;
			return TRUE;
			break;
		}
	}
	return FALSE;
}

int CDesktopTextDrawer::GetCount()
{
	std::lock_guard<std::mutex> lock(m_cs);
	int count = 0;
	for (int i = 0; i < m_nMaxLine; i++) {
		if (m_alarmTextMap[i]->bUsed) {
			count++;
		}
	}
	return count;
}

BOOL CDesktopTextDrawer::IsThisZoneAlarming(int zone, int subzone)
{
	std::lock_guard<std::mutex> lock(m_cs);
	for (int i = 0; i < m_nMaxLine; i++) {
		if (m_alarmTextMap[i]->zone == zone 
			&& m_alarmTextMap[i]->subzone == subzone
			&& m_alarmTextMap[i]->bUsed) {
			return TRUE;
		}
	}
	return FALSE;
}

NAMESPACE_END
