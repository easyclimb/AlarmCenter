#pragma once
#include "afxwin.h"


// CChooseMachineDlg dialog

class CChooseMachineDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CChooseMachineDlg)

public:
	CChooseMachineDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CChooseMachineDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_CHOOSE_MACHINE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int m_ademco_id;
	CStatic m_staticNote;
	CButton m_btnOK;
	afx_msg void OnEnChangeEditAdemcoId();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
};
