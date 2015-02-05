// DesktopTextDrawer.h: interface for the CDesktopTextDrawer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DESKTOPTEXTDRAWER_H__96EDCE70_5D36_4F6D_8193_5EAA4238A5D9__INCLUDED_)
#define AFX_DESKTOPTEXTDRAWER_H__96EDCE70_5D36_4F6D_8193_5EAA4238A5D9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CAlarmTextDlg;
namespace gui {
class CDesktopTextDrawer  
{
	typedef struct _AlarmTextInfo
	{
		BOOL bUsed;
		BOOL bProcessStart;
		int zone;
		int ademco_event;
		//DWORD idThread;
		CString string;
		CAlarmTextDlg *dlg;

		_AlarmTextInfo() : bUsed(FALSE), bProcessStart(FALSE), zone(-1),
			ademco_event(-1), string(_T("")), dlg(NULL)
		{}

		~_AlarmTextInfo()
		{
			
		}

		_AlarmTextInfo& operator=(const _AlarmTextInfo& rhs)
		{
			bUsed = rhs.bUsed;
			bProcessStart = rhs.bProcessStart;
			zone = rhs.zone;
			ademco_event = rhs.ademco_event;
			//idThread = rhs.idThread;
			string = rhs.string;
			dlg = rhs.dlg;
			return *this;
		}
	}AlarmTextInfo, *PAlarmTextInfo;

public:
	CDesktopTextDrawer();
	BOOL IsThisZoneAlarming(int zone);
	int GetCount();
	BOOL GetZoneEvent(int zone, int& ademco_event);
	BOOL IsZoneEventExists(int zone, int ademco_event);
	void DeleteAlarmText(int zone, int ademco_event);
	void Hide();
	void Show();
	void Quit();
	void AddAlarmText(LPCTSTR szAlarm, int zone, int ademco_event);
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
	PAlarmTextInfo m_pAlarmTextInfoArr;
	//CList<PAlarmTextInfo, PAlarmTextInfo&> m_AlarmTextInfoList;
	CRITICAL_SECTION m_cs;
	CWnd *m_pParentWnd;
protected:
	BOOL ShutdownSubProcess(int id);
	BOOL StartupSubProcess(int id);
};

NAMESPACE_END
#endif // !defined(AFX_DESKTOPTEXTDRAWER_H__96EDCE70_5D36_4F6D_8193_5EAA4238A5D9__INCLUDED_)
