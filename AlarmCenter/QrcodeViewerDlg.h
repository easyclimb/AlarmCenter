#pragma once
#include "StaticBmp.h"
#include "afxwin.h"
#include <iostream>
//using std::string;
//namespace gui { namespace control { class CStaticBmp; }; };
// CQrcodeViewerDlg dialog

class CQrcodeViewerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CQrcodeViewerDlg)

public:
	CQrcodeViewerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CQrcodeViewerDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_CSR_ACCT };
	std::string m_acct;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	CString m_md5_path;
	CString m_bmp_path;
	void GenerateAcct(char* buff, int buff_size);
	DECLARE_MESSAGE_MAP()
public:
	gui::control::CStaticBmp m_staticEx;
//	CEdit m_csr_acct;
	virtual BOOL OnInitDialog();
	CString m_acct_text;
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
