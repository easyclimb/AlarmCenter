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
public:
	unsigned int m_listening_port;

	std::string m_server_ip;
	std::string m_server_ip_bk;
	unsigned int m_server_port;
	unsigned int m_server_port_bk;


	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CButton m_btnOK;
	int m_begin;
	CString m_txtOk;
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	CIPAddressCtrl m_ctrl_server_ip;
	CIPAddressCtrl m_ctrl_server_ip_bk;
};
