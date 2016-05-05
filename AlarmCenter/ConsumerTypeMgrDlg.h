#pragma once
#include "MyListCtrl.h"


// CConsumerTypeMgrDlg dialog

class CConsumerTypeMgrDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CConsumerTypeMgrDlg)

public:
	CConsumerTypeMgrDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConsumerTypeMgrDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CONSUMER_TYPE_MGR };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CMyListCtrl m_list;
	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedButtonUpdate();
	virtual BOOL OnInitDialog();
};
