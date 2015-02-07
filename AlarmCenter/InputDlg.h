#pragma once
#include "afxwin.h"


// CInputDlg dialog

class CInputDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CInputDlg)

public:
	CInputDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CInputDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_INPUT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_edit;
	CEdit m_edit_control;
	virtual BOOL OnInitDialog();
};
