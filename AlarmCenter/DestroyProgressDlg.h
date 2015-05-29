#pragma once
#include "afxwin.h"


// CDestroyProgressDlg dialog

class CDestroyProgressDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDestroyProgressDlg)

public:
	CDestroyProgressDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDestroyProgressDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_DESTROY_PROGRESS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CListBox m_list;
};
