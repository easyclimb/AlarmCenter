#pragma once
#include "afxcmn.h"
#include "afxwin.h"

typedef std::function<void(void)> progress_cb;
// CSetupNetworkDlg dialog

class CSetupNetworkDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSetupNetworkDlg)
	
public:
	CSetupNetworkDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CSetupNetworkDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_NETWORK };

protected:

	std::map<std::wstring, std::string> domain_ip_map_ = {};

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	void EnableWindows(int mode);
	bool resolve_domain(int n, progress_cb cb = nullptr);
public:
	//unsigned int m_listening_port;

	//std::string m_server1_ip;
	//std::string m_server2_ip;
	//unsigned int m_server1_port;
	//unsigned int m_server2_port;
	CButton m_btnOK;
	CIPAddressCtrl m_server1_ip;
	CIPAddressCtrl m_server2_ip;
	CButton m_radioCsr;
	CButton m_radioTransmit;
	CButton m_radioDual;
	CEdit m_listening_port;
	CEdit m_server1_port;
	CEdit m_server2_port;
	CEdit m_server1_domain;
	CEdit m_server2_domain;
	CButton m_chkByIpPort1;
	CButton m_chkByIpPort2;
	CButton m_btnTestDomain1;
	CButton m_btnTestDomain2;
	CEdit m_csr_acct;
	CString m_csracct;

	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	int m_begin;
	CString m_txtOk;
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedRadioModeCsr();
	afx_msg void OnBnClickedRadioModeTransmit();
	afx_msg void OnBnClickedRadioModeDual();
	afx_msg void OnBnClickedCheckByIpport1();
	afx_msg void OnBnClickedCheckByIpport2();
	afx_msg void OnBnClickedButtonTestDomain1();
	afx_msg void OnBnClickedButtonTestDomain2();
	
	CEdit m_ezviz_domain;
	CIPAddressCtrl m_ezviz_ip;
	CEdit m_ezviz_port;
	afx_msg void OnBnClickedButtonTestDomain3();
	afx_msg void OnBnClickedCheckByIpport3();
	CButton m_chkByIpPort3;
	CButton m_btnTestDomain3;
	CEdit m_ezviz_app_key;
	CStatic m_group_choose_work_mode;
	CStatic m_group_direct_alarm_center_settings;
	CStatic m_group_transmit_server_settings;
	CStatic m_static_alarm_center_account;
	CStatic m_static_trans1;
	CStatic m_static_trans2;
	CStatic m_group_ezviz_settings;
	CStatic m_static_ezviz_domain;
	CStatic m_static_ezviz_app_key;
	CStatic m_static_listening_port;
};
