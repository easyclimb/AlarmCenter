#pragma once
#include "afxwin.h"
#include "core.h"


// CAlarmHandleStep2Dlg dialog

class CAlarmHandleStep2Dlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAlarmHandleStep2Dlg)

public:
	CAlarmHandleStep2Dlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAlarmHandleStep2Dlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ALARM_HANDLE_2 };
#endif

public:
	static core::alarm_judgement prev_sel_alarm_judgement_;
	static int prev_user_defined_;

	std::wstring video_ = {};
	std::wstring image_ = {};
	std::wstring note_ = {};

protected:

	void enable_windows();
	bool resolv_alarm_judgement();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CButton m_radio_video_image;
	CButton m_radio_comfirmed_with_consumer;
	CButton m_radio_platform;
	CButton m_radio_user_define;
	CButton m_btn_video;
	CButton m_btn_img;
	CComboBox m_cmb_user_define;
	CEdit m_edit_video_path;
	CEdit m_edit_image_path;
	CEdit m_edit_note;

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedRadio3();
	afx_msg void OnBnClickedRadio4();
	afx_msg void OnBnClickedOk();
	
	afx_msg void OnBnClickedButtonVideo();
	afx_msg void OnBnClickedButtonImage();
};
