#pragma once

#include <list>
#include "baidu.h"
#include "afxwin.h"

#include "core.h"

class CBaiduMapDlg;
// CBaiduMapViewerDlg dialog

class CBaiduMapViewerDlg;
extern CBaiduMapViewerDlg* g_baiduMapDlg;

class CBaiduMapViewerDlg : public CDialogEx
{
	class CurUserChangedObserver;
	std::shared_ptr<CurUserChangedObserver> m_cur_user_changed_observer;

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
	int m_maximized_ = 0;
	
	/*typedef struct MachineUuid
	{
		int ademco_id;
		int zone_value;
		MachineUuid() = default;
		MachineUuid(const MachineUuid& rhs) = default;
		MachineUuid(int ademco_id, int zone_value) :ademco_id(ademco_id), zone_value(zone_value) {}
		MachineUuid& operator = (const MachineUuid& rhs)
		{
			ademco_id = rhs.ademco_id;
			zone_value = rhs.zone_value;
			return *this;
		}

		bool operator==(const MachineUuid& rhs) const {
			return ademco_id == rhs.ademco_id &&
				zone_value == rhs.zone_value;
		}
	}MachineUuid;*/

	typedef std::pair<int, int> MachineUuid; // ademco_id, zone_value
	//bool compare_uuid(const MachineUuid&)
	std::list<MachineUuid> m_machineUuidList;
	std::map<int, MachineUuid> m_uuidMap;
	std::mutex m_lock4MachineUuidList;

	bool GetMachineByUuidAndFormatText(const MachineUuid& uuid, core::alarm_machine_ptr& machine, CString& txt);
public:
	core::alarm_machine_ptr m_machine;
	std::shared_ptr<CBaiduMapDlg> m_map;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonAutoLocate();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButtonSetPt();
	afx_msg LRESULT OnChosenBaiduPt(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedButtonShowPath();
	CButton m_btnAutoLocate;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMove(int x, int y);
	void ShowMap(int ademco_id, int zone_value)
	{
		AUTO_LOG_FUNCTION;
		std::lock_guard<std::mutex> lock(m_lock4MachineUuidList);
		MachineUuid uuid(ademco_id, zone_value);
		m_machineUuidList.push_back(uuid);
	}
	void ShowMap(const core::alarm_machine_ptr& machine);
	void ShowCsrMap(const web::BaiduCoordinate& coor, int level);
	afx_msg void OnBnClickedButtonShowMap();
	afx_msg void OnClose();
	afx_msg void OnBnClickedCheckAutoAlarm();
	CButton m_chkAutoAlarm;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CButton m_btnShowDrivingRoute;
	CWnd* m_pCsrInfoWnd;
	afx_msg void OnBnClickedCheckAutoAlarm2();
	CButton m_chkAutoRefresh4NewAlarm;
	CComboBox m_cmbBufferedAlarmList;
	afx_msg void OnCbnSelchangeComboBufferedAlarm();
	afx_msg void OnBnClickedButtonClearCmb();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
};
