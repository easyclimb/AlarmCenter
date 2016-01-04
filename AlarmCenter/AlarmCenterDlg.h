
// AlarmCenterDlg.h : header file
//

#pragma once

#include "afxwin.h"
#include "afxcmn.h"
#include <list>
//#include <vector>
#include "BtnST.h"
#include "ListBoxEx.h"
namespace core { class CGroupInfo; };
#include "core.h"
#include "observer.h"

class CAlarmMachineContainerDlg;
class CAlarmCenterInfoDlg;
//class CLoadFromDBProgressDlg;
// CAlarmCenterDlg dialog
class CAlarmCenterDlg : public CDialogEx
{
	class CurUserChangedObserver;
	std::shared_ptr<CurUserChangedObserver> m_cur_user_changed_observer;

	class NewRecordObserver;
	std::shared_ptr<NewRecordObserver> m_new_record_observer;

public:
	class observer : public dp::observer<int> {
	public:

		void on_update(int) {
			//std::shared_ptr<CAlarmCenterDlg> dlg(_dlg.lock());
			if (_dlg)
				_dlg->m_times4GroupOnlineCntChanged++;
		}

		explicit observer(CAlarmCenterDlg* dlg) : _dlg(dlg) {}
		CAlarmCenterDlg* _dlg;
	};
	std::shared_ptr<observer> m_observer;

	typedef struct MachineAlarmOrDisalarm{
		bool alarm;
		core::CAlarmMachinePtr machine;
		MachineAlarmOrDisalarm(bool b, core::CAlarmMachinePtr machine) : alarm(b), machine(machine) {}
	}MachineAlarmOrDisalarm;

	typedef std::shared_ptr<MachineAlarmOrDisalarm> MachineAlarmOrDisalarmPtr;
	typedef std::list<MachineAlarmOrDisalarmPtr> MachineAlarmOrDisalarmList;

// Construction
public:
	CAlarmCenterDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
	enum { IDD = IDD_ALARMCENTER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	std::shared_ptr<CAlarmMachineContainerDlg> m_wndContainer;
	std::shared_ptr<CAlarmMachineContainerDlg> m_wndContainerAlarming;
	HICON m_hIconComputer;
	HICON m_hIconConnection;
	HICON m_hIconInternet;
	std::shared_ptr<CAlarmCenterInfoDlg> m_alarmCenterInfoDlg;
	HTREEITEM m_curselTreeItem;
	DWORD m_curselTreeItemData;
	int m_maxHistory2Show;
	MachineAlarmOrDisalarmList m_machineAlarmOrDialarmList;
	/*std::list<GROUP_TREE_INFO*> m_groupTreeInfoList;
	CLock m_lock4GroupTreeInfoList;*/
public:
	volatile int m_times4GroupOnlineCntChanged;
public:
	CStatic m_groupMachineList;
	CStatic m_staticSysTime; 
	CStatic m_groupControlPanel;
	CStatic m_gNet;
	CStatic m_gCsr;
	CStatic m_sTransmitServerStatus;
	CStatic m_sLocalPort;
	CEdit m_cur_user_id;
	CEdit m_cur_user_name;
	CEdit m_cur_user_phone;
	CEdit m_cur_user_priority;
	CButton m_btnMachineMgr;
	CButton m_btnUserMgr;
	CTreeCtrl m_treeGroup;
	gui::control::CListBoxEx m_listHistory;
	CTabCtrl m_tab;
	CStringList m_recordList;
	CLock m_lock4RecordList;
	CLock m_lock4AdemcoEvent;
	gui::control::CButtonSTUrlLike m_btnSeeMoreHr;
	CStatic m_groupHistory;
protected:
	void InitDisplay();
	void InitAlarmMacineTreeView();
	void TraverseGroup(HTREEITEM hItemGroup, core::CGroupInfoPtr group);
	bool SelectGroupItemOfTree(DWORD data);
	bool SelectGroupItemOfTreeHelper(HTREEITEM hItem, DWORD data);
	void TraverseGroupTree(HTREEITEM hItem);
	void HandleMachineAlarm();
public:
	void MachineAlarm(core::CAlarmMachinePtr machine) { 
		m_lock4AdemcoEvent.Lock(); 
		m_machineAlarmOrDialarmList.push_back(std::make_shared<MachineAlarmOrDisalarm>(true, machine)); 
		m_lock4AdemcoEvent.UnLock();
	}
	void MachineDisalarm(core::CAlarmMachinePtr machine) {
		m_lock4AdemcoEvent.Lock();
		m_machineAlarmOrDialarmList.push_back(std::make_shared<MachineAlarmOrDisalarm>(false, machine));
		m_lock4AdemcoEvent.UnLock();
	}
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnCuruserchangedResult(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTransmitserver(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedButtonSwitchUser();
	afx_msg void OnBnClickedButtonUsermgr();
	afx_msg void OnBnClickedButtonViewQrcode();
	afx_msg void OnClose();
	virtual void OnCancel();
	afx_msg void OnTvnSelchangedTreeMachineGroup(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTcnSelchangeTabContainer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkTreeMachineGroup(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonMachinemgr();
	afx_msg void OnBnClickedButtonSeeMoreHr();
	afx_msg void OnBnClickedButtonMute();
	afx_msg LRESULT OnNeedQuerySubMachine(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNeedToExportHr(WPARAM wParam, LPARAM lParam);
	afx_msg void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);
	afx_msg void OnNMRClickTreeMachineGroup(NMHDR *pNMHDR, LRESULT *pResult);
};
