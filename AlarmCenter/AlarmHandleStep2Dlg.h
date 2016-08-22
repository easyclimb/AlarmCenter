#pragma once
#include "afxwin.h"


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

protected:

	void enable_windows();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CButton m_radio_video_image;
	CButton m_radio_comfirmed_with_consumer;
	CButton m_radio_platform;
	CButton m_radio_user_define;
	CComboBox m_cmb_user_define;

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedRadio1();
	CButton m_btn_video;
	CButton m_btn_img;
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedRadio3();
	afx_msg void OnBnClickedRadio4();
};
