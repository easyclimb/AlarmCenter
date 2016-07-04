#pragma once
#include "afxwin.h"


// CChooseMachineDlg dialog

class CChooseMachineDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CChooseMachineDlg)

public:
	CChooseMachineDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CChooseMachineDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_CHOOSE_MACHINE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int m_ademco_id = -1;
	int m_zone_value = -1;
	bool choosing_sub_machine_ = false;

	CStatic m_staticNote;
	CButton m_btnOK;
	afx_msg void OnEnChangeEditAdemcoId();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	CStatic m_static_input_id;
	CEdit m_edit;
};
