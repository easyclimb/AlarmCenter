
// AlarmCenterDlg.h : header file
//

#pragma once

#include "afxwin.h"
#include "afxcmn.h"
//#include <vector>

//namespace gui { class CButtonST; };
namespace core { class CGroupInfo; };

class CAlarmMachineContainerDlg;
class CQrcodeViewerDlg;
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
	//JTL::Vector<gui::CButtonST*> m_vectorButtons;
	//std::vector<gui::CButtonST*> m_vectorButtons;
	CAlarmMachineContainerDlg* m_wndContainer;
	HICON m_hIconComputer;
	HICON m_hIconConnection;
	HICON m_hIconInternet;
	CQrcodeViewerDlg* m_qrcodeViewDlg;
public:
	CStatic m_groupMachineList;
	CStatic m_staticSysTime; 
	CStatic m_groupControlPanel;

protected:
	void InitDisplay();
	void InitAlarmMacines();
	void TraverseGroup(HTREEITEM hItemGroup, core::CGroupInfo* group);
	// void ShowMachinesOfGroup(core::CGroupInfo* group);
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	CStatic m_gNet;
	CStatic m_gCsr;
protected:
	afx_msg LRESULT OnTransmitserver(WPARAM wParam, LPARAM lParam);
public:
	CStatic m_sTransmitServerStatus;
	CStatic m_sLocalPort;
	CEdit m_cur_user_id;
	CEdit m_cur_user_name;
protected:
	afx_msg LRESULT OnCuruserchangedResult(WPARAM wParam, LPARAM lParam);
public:
	CEdit m_cur_user_phone;
	afx_msg void OnBnClickedButtonSwitchUser();
	CEdit m_cur_user_priority;
	CButton m_btnUserMgr;
	afx_msg void OnBnClickedButtonUsermgr();
	afx_msg void OnBnClickedButtonViewQrcode();
	afx_msg void OnClose();
	virtual void OnCancel();
	CListBox m_listHistory;
protected:
	afx_msg LRESULT OnNewrecordResult(WPARAM wParam, LPARAM lParam);
public:
	CTreeCtrl m_treeGroup;
	afx_msg void OnTvnSelchangedTreeMachineGroup(NMHDR *pNMHDR, LRESULT *pResult);
};
