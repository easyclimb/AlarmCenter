#pragma once


// CAddZoneDlg dialog

class CAddZoneDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAddZoneDlg)

public:
	CAddZoneDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAddZoneDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_ADD_ZONE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int m_value;
	afx_msg void OnBnClickedOk();
};
