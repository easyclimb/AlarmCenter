// DesktopTextDrawer.h: interface for the CDesktopTextDrawer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DESKTOPTEXTDRAWER_H__96EDCE70_5D36_4F6D_8193_5EAA4238A5D9__INCLUDED_)
#define AFX_DESKTOPTEXTDRAWER_H__96EDCE70_5D36_4F6D_8193_5EAA4238A5D9__INCLUDED_

#pragma once

#include <memory>
#include <map>

class CAlarmTextDlg;
typedef std::shared_ptr<CAlarmTextDlg> CAlarmTextDlgPtr;

namespace gui {
class CDesktopTextDrawer  
{
	typedef struct AlarmTextInfo
	{
		BOOL bUsed;
		BOOL bProcessStart;
		int zone;
		int subzone;
		ademco::ADEMCO_EVENT ademco_event;
		//DWORD idThread;
		CString string;
		CAlarmTextDlgPtr dlg;
		//time_t _time;
		COLORREF color;
		AlarmTextInfo() : bUsed(FALSE), bProcessStart(FALSE), zone(-1),
			subzone(-1), ademco_event(ademco::EVENT_INVALID_EVENT), string(_T("")), dlg(nullptr)/*, _time()*/
			, color(RGB(255, 0, 0))
		{}

		AlarmTextInfo& operator=(const AlarmTextInfo& rhs)
		{
			bUsed = rhs.bUsed;
			bProcessStart = rhs.bProcessStart;
			zone = rhs.zone;
			subzone = rhs.subzone;
			ademco_event = rhs.ademco_event;
			//idThread = rhs.idThread;
			string = rhs.string;
			dlg = rhs.dlg;
			//_time = rhs._time;
			color = rhs.color;
			return *this;
		}
	}AlarmTextInfo;

	typedef std::shared_ptr<AlarmTextInfo> AlarmTextInfoPtr;

public:
	CDesktopTextDrawer();
	BOOL IsThisZoneAlarming(int zone, int subzone);
	int GetCount();
	BOOL GetZoneEvent(int zone, int subzone, ademco::ADEMCO_EVENT& ademco_event);
	//BOOL IsZoneEventExists(int zone, int subzone, ADEMCO_EVENT ademco_event);
	void DeleteAlarmText(int zone, int subzone, ademco::ADEMCO_EVENT ademco_event);
	void Hide();
	void Show();
	void Quit();
	void AddAlarmText(LPCTSTR szAlarm, int zone, int subzone, ademco::ADEMCO_EVENT ademco_event);
	//CDesktopTextDrawer(const CWnd* pParentWnd);
	virtual ~CDesktopTextDrawer();
	void SetOwner(CWnd* pParentWnd)
	{
		if (pParentWnd) {
			m_pParentWnd = pParentWnd;
		} else {
			m_pParentWnd = AfxGetApp()->m_pMainWnd;
		}
	}
private:
	//CDesktopTextDrawer() 	{}
	int m_nGapID;
	int m_nMaxLine;
	int m_cx, m_cy, m_height;
	//PAlarmTextInfo m_pAlarmTextInfoArr;
	std::map<int, AlarmTextInfoPtr> m_alarmTextMap;
	//CList<PAlarmTextInfo, PAlarmTextInfo&> m_AlarmTextInfoList;
	std::recursive_mutex m_cs;
	CWnd *m_pParentWnd;
protected:
	BOOL ShutdownSubProcess(int id);
	BOOL StartupSubProcess(int id);
};

};
#endif // !defined(AFX_DESKTOPTEXTDRAWER_H__96EDCE70_5D36_4F6D_8193_5EAA4238A5D9__INCLUDED_)
