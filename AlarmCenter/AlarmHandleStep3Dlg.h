#pragma once
#include "C:\dev\AlarmCenter\AlarmCenter\GridCtrl_src\GridCtrl.h"


// CAlarmHandleStep3 dialog

class CAlarmHandleStep3Dlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAlarmHandleStep3Dlg)

public:
	CAlarmHandleStep3Dlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAlarmHandleStep3Dlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ALARM_HANDLE_3 };
#endif

protected:
	int cur_editting_guard_id_ = 0;
	int cur_editting_handle_id_ = 0;
	int cur_editting_handle_time_ = 0;

protected:
	void init_list();
	void init_user_list();
	

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	gui::control::grid_ctrl::CGridCtrl m_grid;
	gui::control::grid_ctrl::CGridCtrl m_user_list;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonAddGuard();
	afx_msg void OnBnClickedButtonRmGuard();
	afx_msg void OnGridStartEdit(NMHDR *pNotifyStruct, LRESULT* pResult);
	afx_msg void OnGridEndEdit(NMHDR *pNotifyStruct, LRESULT* pResult);
	afx_msg void OnGridItemChanged(NMHDR *pNotifyStruct, LRESULT* pResult);
	afx_msg void OnGridStartEditUser(NMHDR *pNotifyStruct, LRESULT* pResult);
	afx_msg void OnGridEndEditUser(NMHDR *pNotifyStruct, LRESULT* pResult);
	afx_msg void OnGridItemChangedUser(NMHDR *pNotifyStruct, LRESULT* pResult);
};
