#pragma once

#include <list>

#include "core.h"
//namespace gui { class CDetector; };


class CDetector;
namespace gui { class CDesktopTextDrawer; class CAntLine; };
class CMapView : public CDialogEx
{
	typedef enum MapViewMode
	{
		MODE_NORMAL = 0,
		MODE_EDIT,
	}MapViewMode;

	DECLARE_DYNAMIC(CMapView)
	friend class CAlarmMachineDlg;
public:
	CMapView(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CMapView();

// Dialog Data
	enum { IDD = IDD_DIALOG_MAPVIEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()
	//CDetector* GetDetector(int zone);
private:
	core::CMapInfoPtr m_mapInfo;
	core::CAlarmMachinePtr m_machine;
	HBITMAP m_hBmpOrigin;
	int m_bmWidth;
	int m_bmHeight;
	std::list<CDetector*> m_detectorList;
	gui::CAntLine* m_pAntLine;
	gui::CDesktopTextDrawer* m_pTextDrawer;
	BOOL m_bAlarming;
	MapViewMode m_mode;
	int m_nFlashTimes;
	CRITICAL_SECTION m_csDetectorList;
	HDC m_hDC4AntLine;
	CWnd* m_pRealParent;
	std::list<void*> m_icmcList;
	CLock m_icmcLock;
public:
	void SetMapInfo(core::CMapInfoPtr mapInfo) { m_mapInfo = mapInfo; }
	void SetMachineInfo(core::CAlarmMachinePtr machine) { m_machine = machine; }
	virtual BOOL OnInitDialog();
	void SetRealParentWnd(CWnd* pWnd) { m_pRealParent = pWnd; }
	void AddIcmc(void* icmc){
		AUTO_LOG_FUNCTION;
		m_icmcLock.Lock();
		m_icmcList.push_back(icmc);
		m_icmcLock.UnLock();
	}
	//void EnterEditMode();
	//void LeaveEditMode();
protected:
	void SetMode(MapViewMode mode);
	BOOL ImportBmp();
	void FlushDetector();
	void CreateAntLine();
	void OnNewDetector();
	void OnDelDetector();
	void TellParent2ShowMyTab(int cmd);
public:
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnRepaint(WPARAM wParam, LPARAM lParam);
	//afx_msg LRESULT OnTraversezone(WPARAM wParam, LPARAM lParam);
	void OnInversionControlResult(WPARAM wParam, core::AlarmTextPtr at);
};

//NAMESPACE_END
