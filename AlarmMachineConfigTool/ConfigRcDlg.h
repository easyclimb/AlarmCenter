#pragma once


// CConfigRcDlg dialog

class CConfigRcDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CConfigRcDlg)

public:
	CConfigRcDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConfigRcDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_CONFIG_RC };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonDuima();
	afx_msg void OnBnClickedButtonDel();
	afx_msg void OnBnClickedButtonDuima2();
	afx_msg void OnBnClickedButtonDel2();
	afx_msg void OnBnClickedButtonDuima3();
	afx_msg void OnBnClickedButtonDel3();
	afx_msg void OnBnClickedButtonDuima4();
	afx_msg void OnBnClickedButtonDel4();
	afx_msg void OnBnClickedButtonDuima5();
	afx_msg void OnBnClickedButtonDel5();
	afx_msg void OnBnClickedButtonDelAll();
};
