
// AlarmCenterDlg.h : header file
//

#pragma once

#include "afxwin.h"
#include "afxcmn.h"
//#include <vector>
#include "BtnST.h"
#include "ListBoxEx.h"
namespace core { class CGroupInfo; };

class CAlarmMachineContainerDlg;
class CQrcodeViewerDlg;
//class CLoadFromDBProgressDlg;
// CAlarmCenterDlg dialog
class CAlarmCenterDlg : public CDialogEx
{
// Construction
public:
	CAlarmCenterDlg(CWnd* pParent = NULL);	// standard constructor

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
	CAlarmMachineContainerDlg* m_wndContainer;
	CAlarmMachineContainerDlg* m_wndContainerAlarming;
	HICON m_hIconComputer;
	HICON m_hIconConnection;
	HICON m_hIconInternet;
	CQrcodeViewerDlg* m_qrcodeViewDlg;
	//CLoadFromDBProgressDlg* m_progressDlg;
	HTREEITEM m_curselTreeItem;
	DWORD m_curselTreeItemData;
	int m_maxHistory2Show;
	
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
	gui::control::CButtonSTUrlLike m_btnSeeMoreHr;
	CStatic m_groupHistory;
protected:
	void InitDisplay();
	void InitAlarmMacines();
	void TraverseGroup(HTREEITEM hItemGroup, core::CGroupInfo* group);
	// void ShowMachinesOfGroup(core::CGroupInfo* group);
	bool SelectGroupItemOfTree(DWORD data);
	bool SelectGroupItemOfTreeHelper(HTREEITEM hItem, DWORD data);
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnCuruserchangedResult(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTransmitserver(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedButtonSwitchUser();
	afx_msg void OnBnClickedButtonUsermgr();
	afx_msg void OnBnClickedButtonViewQrcode();
	afx_msg void OnClose();
	virtual void OnCancel();
	afx_msg LRESULT OnNewrecordResult(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTvnSelchangedTreeMachineGroup(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTcnSelchangeTabContainer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnAdemcoevent(WPARAM wParam, LPARAM lParam);
	afx_msg void OnNMDblclkTreeMachineGroup(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonMachinemgr();
	afx_msg void OnBnClickedButtonSeeMoreHr();
	afx_msg void OnBnClickedButtonMute();
	afx_msg LRESULT OnNeedQuerySubMachine(WPARAM wParam, LPARAM lParam);
};
