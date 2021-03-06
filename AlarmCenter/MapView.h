#pragma once

#include <list>

#include "core.h"

namespace gui { class CDesktopTextDrawer; class CAntLine; };

class CMapView : public CDialogEx, public std::enable_shared_from_this<CMapView>
{

public:
	class IcmcObserver : public dp::observer<core::icmc_buffer_ptr>
	{
	public:
		explicit IcmcObserver(CMapView* view) : _view(view) {}
		virtual void on_update(const core::icmc_buffer_ptr& ptr) {
			if (_view) {
				_view->AddIcmc(ptr);
			}
		}

	private:
		CMapView* _view;
	};

	std::shared_ptr<IcmcObserver> m_icmc_observer;
protected:
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
	core::map_info_ptr m_mapInfo;
	core::alarm_machine_ptr m_machine;
	HBITMAP m_hBmpOrigin;
	int m_bmWidth;
	int m_bmHeight;
	std::list<core::CDetectorPtr> m_detectorList;
	std::unique_ptr<gui::CAntLine> m_pAntLine;
	std::unique_ptr<gui::CDesktopTextDrawer> m_pTextDrawer;
	BOOL m_bAlarming;
	MapViewMode m_mode;
	int m_nFlashTimes;
	std::mutex m_csDetectorList;
	HDC m_hDC4AntLine;
	CWnd* m_pRealParent;
	
	std::list<core::icmc_buffer_ptr> m_icmcList;
	std::mutex m_icmcLock;
public:
	void SetMapInfo(const core::map_info_ptr& mapInfo) { m_mapInfo = mapInfo; }
	void SetMachineInfo(const core::alarm_machine_ptr& machine) { m_machine = machine; }
	virtual BOOL OnInitDialog();
	void SetRealParentWnd(CWnd* pWnd) { m_pRealParent = pWnd; }
	void AddIcmc(core::icmc_buffer_ptr icmc){
		AUTO_LOG_FUNCTION;
		std::lock_guard<std::mutex> lock(m_icmcLock);
		m_icmcList.push_back(icmc);
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
	void TellParent2ShowMyTab(core::inversion_control_map_command cmd);
public:
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnRepaint(WPARAM wParam, LPARAM lParam);
	//afx_msg LRESULT OnTraversezone(WPARAM wParam, LPARAM lParam);
	void OnInversionControlResult(core::inversion_control_map_command icmc, const core::alarm_text_ptr& at);
};

//;
