#pragma once
#include "C:\dev\AlarmCenter\AlarmCenter\GridCtrl_src\GridCtrl.h"
#include <afxwin.h>

// CSecurityGuardMgrDlg dialog

class CSecurityGuardMgrDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSecurityGuardMgrDlg)

public:
	CSecurityGuardMgrDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSecurityGuardMgrDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SECURITY_GUARD_MGR };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

protected:
	int cur_editting_guard_id_ = 0;
	gui::control::grid_ctrl::CGridCtrl m_user_list;


public:
	virtual BOOL OnInitDialog();
	afx_msg void OnGridStartEditUser(NMHDR *pNotifyStruct, LRESULT* pResult);
	afx_msg void OnGridEndEditUser(NMHDR *pNotifyStruct, LRESULT* pResult);
	afx_msg void OnGridItemChangedUser(NMHDR *pNotifyStruct, LRESULT* pResult);
	afx_msg void OnBnClickedButtonAddGuard();
	afx_msg void OnBnClickedButtonRmGuard();
};
