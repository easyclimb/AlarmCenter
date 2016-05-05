#pragma once
#include "afxwin.h"

#include "core.h"

class CAddMachineDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAddMachineDlg)

public:
	CAddMachineDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CAddMachineDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_ADD_MACHINE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	bool CheckAdemcoID();
	bool CheckDeviceID();
	void InitTypes();
	void CalcOwdAmount();
	COleDateTime m_ole_remind_time;
public:
	core::group_info_ptr group_info_ = nullptr;
	CEdit m_ademco_id;
	CComboBox m_banned;
	CComboBox m_type;
	//CEdit m_device_id;
	CEdit m_alias;
	CEdit m_contact;
	CEdit m_address;
	CEdit m_phone;
	CEdit m_phone_bk;
	//CComboBox m_group;
	CStatic m_note;
	CButton m_ok;
	core::alarm_machine_ptr m_machine;
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnEnChangeEditAdemcoID();
	afx_msg void OnBnClickedOk();
	afx_msg void OnEnKillfocusEditDeviceID();
	CEdit m_expire_time;
	CComboBox m_cmb_ademco_id;
	afx_msg void OnCbnSelchangeCombo3();
	afx_msg void OnCbnEditchangeCombo3();
	CButton m_btnGroup;
	CEdit m_edit_group;
	afx_msg void OnBnClickedButtonGroup();
	afx_msg void OnCbnSelchangeComboType();
	CEdit m_receivable_amount;
	CEdit m_paid_amount;
	CEdit m_owd_amount;
	afx_msg void OnEnChangeEditReceivable();
	afx_msg void OnEnChangeEditPaid();
	CEdit m_remind_time;
	afx_msg void OnBnClickedButtonSetExpire();
	afx_msg void OnBnClickedButtonSetAlarm();
	CButton m_btn_set_remind;
	CButton m_btn_extend;
};
