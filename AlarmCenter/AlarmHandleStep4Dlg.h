#pragma once
#include "afxwin.h"
#include "core.h"
#include "alarm_handle_mgr.h"

// CAlarmHandleStep4Dlg dialog

class CAlarmHandleStep4Dlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAlarmHandleStep4Dlg)

public:
	CAlarmHandleStep4Dlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAlarmHandleStep4Dlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ALARM_HANDLE_4 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	core::alarm_ptr cur_handling_alarm_info_ = nullptr;
	core::alarm_judgement_ptr judgment_ = nullptr;
	core::alarm_handle_ptr handle_ = nullptr;
	core::alarm_reason_ptr reason_ = nullptr;
	core::alarm_machine_ptr machine_ = nullptr;
protected:
	void update_guard();
	int get_predict_min();

public:
	CEdit m_aid;
	CEdit m_name;
	CEdit m_contact;
	CEdit m_addr;
	CEdit m_phone;
	CEdit m_phone_bk;
	CEdit m_alarm_text;
	CComboBox m_cmb_status;
	CEdit m_assign_time;
	CComboBox m_cmb_guard;
	CEdit m_predict_minutes;
	CEdit m_handle_time;
	CEdit m_user;
	CComboBox m_cmb_alarm_reason;
	CEdit m_reason_detail;
	CEdit m_reason_attach;
	CComboBox m_cmb_judgement;
	CEdit m_judgement_detail;
	CEdit m_judgement_attach1;
	CEdit m_judgement_attach2;
	CButton m_btn_ok;
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonMgrGuard();
	afx_msg void OnBnClickedButtonSwitchUser();
	afx_msg void OnBnClickedButtonAddReasonAttach();
	afx_msg void OnBnClickedButtonAddJudgmentAttach1();
	afx_msg void OnBnClickedButtonAddJudgementAttach2();
	afx_msg void OnEnKillfocusEditPredictMinutes();
	afx_msg void OnCbnSelchangeComboStatus();
	afx_msg void OnBnClickedOk();
	afx_msg void OnCbnSelchangeComboGuard();
	afx_msg void OnEnChangeEditPredictMinutes();
	CButton m_btn_add_judgment_attach_1;
	CButton m_btn_add_judgment_attach_2;
	afx_msg void OnCbnSelchangeComboJudgement();
};
