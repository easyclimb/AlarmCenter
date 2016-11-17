#pragma once

#include "core.h"
#include "afxwin.h"
#include "alarm_handle_mgr.h"

// CAlarmHandleStep1Dlg dialog

class CAlarmHandleStep1Dlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAlarmHandleStep1Dlg)

public:
	CAlarmHandleStep1Dlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAlarmHandleStep1Dlg();

	core::alarm_machine_ptr machine_ = nullptr;
	class alarm_text_observer;
	std::shared_ptr<alarm_text_observer> alarm_text_observer_ = nullptr;
	std::list<core::alarm_text_ptr> alarm_texts_ = {};
	//core::alarm_text_ptr cur_handling_alarm_text_ = nullptr;
	core::alarm_ptr cur_handling_alarm_info_ = nullptr;

public:
	core::alarm_type get_alarm_type();
	void add_alarm_text(const core::alarm_text_ptr& at);
	void prepair();
	void show_one();
	void handle_one();
	void show_result();

public:

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ALARM_HANDLE_1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	CEdit m_aid;
	CEdit m_name;
	CEdit m_contact;
	CEdit m_addr;
	CEdit m_phone;
	CEdit m_phone_bk;
	CEdit m_alarm_text;
	CButton m_radio_true_alarm;
	CButton m_radio_device_false_positive;
	CButton m_radio_test_device;
	CButton m_radio_man_made_false_posotive;
	CButton m_radio_cannot_determine;
	CButton m_btn_print;
	afx_msg void OnBnClickedButtonPrint();
};
