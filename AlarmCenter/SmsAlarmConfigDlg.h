#pragma once
#include "afxwin.h"


// CSmsAlarmConfigDlg dialog

class CSmsAlarmConfigDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSmsAlarmConfigDlg)

public:
	CSmsAlarmConfigDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CSmsAlarmConfigDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_SMS_CONFIG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedCheck2();
	afx_msg void OnBnClickedCheck3();
	CEdit m_testSms;
	afx_msg void OnBnClickedButton1();
	virtual BOOL OnInitDialog();
};
