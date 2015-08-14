#pragma once
#include "afxwin.h"

namespace core { class CAlarmMachine; };
// CAddMachineDlg dialog

class CAddMachineDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAddMachineDlg)

public:
	CAddMachineDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAddMachineDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_ADD_MACHINE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	bool CheckAdemcoID();
	bool CheckDeviceID();
public:
	CEdit m_ademco_id;
	CComboBox m_banned;
	CComboBox m_type;
	//CEdit m_device_id;
	CEdit m_alias;
	CEdit m_contact;
	CEdit m_address;
	CEdit m_phone;
	CEdit m_phone_bk;
	CComboBox m_group;
	CStatic m_note;
	CButton m_ok;
	core::CAlarmMachine* m_machine;
	virtual BOOL OnInitDialog();
	afx_msg void OnEnChangeEditAdemcoID();
	afx_msg void OnBnClickedOk();
	afx_msg void OnEnKillfocusEditDeviceID();
	CEdit m_expire_time;
};
