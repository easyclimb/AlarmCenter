
// SerialFilterDlg.h : header file
//

#pragma once
#include "SerialPort.h"
#include "afxwin.h"
#include "GenericBuffer.h"

// CSerialFilterDlg dialog
class CSerialFilterDlg : public CDialogEx, public CSerialPort
{
// Construction
public:
	CSerialFilterDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_SERIALFILTER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnRecv(const char *cmd, WORD wLen);
	virtual BOOL OnSend(IN char* cmd, IN WORD wLen, OUT WORD& wRealLen);
	virtual		void OnConnectionEstablished();
public:
	afx_msg void OnBnClickedButton1();
	CEdit m_port;
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
	CListBox m_list1;
	CListBox m_list2;
	CListBox m_list3;
	CGenericBuffer m_buff;
	CGenericBuffer m_buff2;
	CStringList m_strlist1;
	CStringList m_strlist2;
	CStringList m_strlist3;
	CLock m_lock1;
	CLock m_lock2;
	CLock m_lock3;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	
	afx_msg void OnBnClickedButton5();
};
