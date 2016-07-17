#pragma once

#include <list>
#include "../AlarmCenter/baidu.h"
#include "afxwin.h"

#include "../AlarmCenter/core.h"

class CBaiduMapDlg;
// CBaiduMapViewerDlg dialog

class CBaiduMapViewerDlg;
extern CBaiduMapViewerDlg* g_baiduMapDlg;

class CBaiduMapViewerDlg : public CDialogEx
{
	//class CurUserChangedObserver;
	//std::shared_ptr<CurUserChangedObserver> m_cur_user_changed_observer;

	enum Mode {
		MODE_MACHINE,
		MODE_CSR,
	};
	volatile Mode m_mode;

	DECLARE_DYNAMIC(CBaiduMapViewerDlg)

public:
	CBaiduMapViewerDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CBaiduMapViewerDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_PICK_MACHINE_COOR };
	HICON m_hIcon;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
protected:
	void InitPosition();
	void SavePosition(BOOL bMaximized = FALSE);
	void ResizeMap();
	BOOL m_bSizing;
	BOOL m_bMoving;
	int m_x, m_y, m_cx, m_cy;
	BOOL m_bInitOver;
	COleDateTime m_lastTimeShowMap;	
	std::list<core::MachineUuid> m_machineUuidList;
	std::map<int, core::MachineUuid> m_uuidMap;
	std::mutex m_lock4MachineUuidList;
	bool GetMachineByUuidAndFormatText(const core::MachineUuid& uuid, core::alarm_machine_ptr& machine, CString& txt);
	void ShowCsrMap(const web::BaiduCoordinate& coor, int level);
	void ShowMap(int ademco_id, int zone_value)
	{
		AUTO_LOG_FUNCTION;
		std::lock_guard<std::mutex> lock(m_lock4MachineUuidList);
		core::MachineUuid uuid(ademco_id, zone_value);
		m_machineUuidList.push_back(uuid);
	}

protected:
	CButton m_btnAutoLocate;
	CButton m_chkAutoAlarm;
	CButton m_btnShowDrivingRoute;
	CButton m_chkAutoRefresh4NewAlarm;
	CComboBox m_cmbBufferedAlarmList;
	std::shared_ptr<CBaiduMapDlg> m_map;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonAutoLocate();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButtonSetPt();
	afx_msg LRESULT OnChosenBaiduPt(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedButtonShowPath();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnBnClickedButtonShowMap();
	afx_msg void OnClose();
	afx_msg void OnBnClickedCheckAutoAlarm();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedCheckAutoAlarm2();
	afx_msg void OnCbnSelchangeComboBufferedAlarm();
	afx_msg void OnBnClickedButtonClearCmb();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg LRESULT OnMsgShowCsrMap(WPARAM wParam, LPARAM lParam);
};
