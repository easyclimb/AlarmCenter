#pragma once
#include "afxcmn.h"
#include "afxwin.h"


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
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	void EnableWindows(int mode);
public:
	unsigned int m_listening_port;

	std::string m_server1_ip;
	std::string m_server2_ip;
	unsigned int m_server1_port;
	unsigned int m_server2_port;


	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CButton m_btnOK;
	int m_begin;
	CString m_txtOk;
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	CIPAddressCtrl m_ctrl_server1_ip;
	CIPAddressCtrl m_ctrl_server2_ip;
	CButton m_radioCsr;
	CButton m_radioTransmit;
	CButton m_radioDual;
	afx_msg void OnBnClickedRadioModeCsr();
	afx_msg void OnBnClickedRadioModeTransmit();
	afx_msg void OnBnClickedRadioModeDual();
	CEdit m_ctrl_listening_port;
	CEdit m_ctrl_server1_port;
	CEdit m_ctrl_server2_port;
	CEdit m_server1_domain;
	CEdit m_server2_domain;
	CButton m_chkByIpPort1;
	CButton m_chkByIpPort2;
	afx_msg void OnBnClickedCheckByIpport1();
	afx_msg void OnBnClickedCheckByIpport2();
	CButton m_btnTestDomain1;
	CButton m_btnTestDomain2;
	afx_msg void OnBnClickedButtonTestDomain1();
	afx_msg void OnBnClickedButtonTestDomain2();
};
