#pragma once

#include <list>

namespace core { class CAlarmMachine; class CMapInfo; class CZoneInfo; };

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
	CMapView(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMapView();

// Dialog Data
	enum { IDD = IDD_DIALOG_MAPVIEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()
	CDetector* GetDetector(int zone);
private:
	core::CMapInfo* m_mapInfo;
	core::CAlarmMachine* m_machine;
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
	HDC m_hDC;
public:
	void SetMapInfo(core::CMapInfo* mapInfo) { m_mapInfo = mapInfo; }
	void SetMachineInfo(core::CAlarmMachine* machine) { m_machine = machine; }
	virtual BOOL OnInitDialog();
	void SetMode(MapViewMode mode);
	int GetAdemcoID() const;
	void HandleAdemcoEvent(const ademco::AdemcoEvent* ademcoEvent);
	void TraverseZoneOfMapResult(core::CZoneInfo* zoneInfo);
protected:
	BOOL ImportBmp();
	void FlushDetector();
	void CreateAntLine();
	BOOL IsThisYourZone(int zone);
public:
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	
protected:
	afx_msg LRESULT OnRepaint(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAdemcoEvent(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTraversezone(WPARAM wParam, LPARAM lParam);
};

//NAMESPACE_END
