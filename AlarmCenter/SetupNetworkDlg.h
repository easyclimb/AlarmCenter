#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CSetupNetworkDlg dialog

class CSetupNetworkDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSetupNetworkDlg)
	
public:
	CSetupNetworkDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSetupNetworkDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_NETWORK };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	int m_local_port;
	CIPAddressCtrl m_transmit_ip;
	int m_transmit_port;
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CButton m_btnOK;
	int m_begin;
	CString m_txtOk;
	char m_tranmit_ipA[32];
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};
