#pragma once
#include "afxcmn.h"
#include "afxwin.h"


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
public:
	CTreeCtrl m_tree;
	CEdit m_zone;
	CComboBox m_property;
	CEdit m_alias;
	virtual BOOL OnInitDialog();
};
