#pragma once
#include "afxcmn.h"
#include "afxwin.h"

namespace core { class CAlarmMachine; };
// CEditZoneDlg dialog

class CEditZoneDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CEditZoneDlg)

public:
	CEditZoneDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CEditZoneDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_EDIT_ZONE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
protected:
	void ExpandWindow(bool expand = true);
public:
	CTreeCtrl m_tree;
	CEdit m_zone;
	CComboBox m_type;
	CEdit m_alias;
	core::CAlarmMachine* m_machine;
	virtual BOOL OnInitDialog();
	CStatic m_groupSubMachine;
	afx_msg void OnBnClickedButtonAddzone();
	afx_msg void OnBnClickedButtonDelzone();
	afx_msg void OnCbnSelchangeComboZoneType();
	afx_msg void OnEnChangeEditAlias();
	CEdit m_contact;
	CEdit m_addr;
	CEdit m_phone;
	CEdit m_phone_bk;
	afx_msg void OnTvnSelchangedTreeZone(NMHDR *pNMHDR, LRESULT *pResult);
};
