#pragma once
#include "StaticBmp.h"
#include "afxwin.h"
#include <iostream>
#include "afxcmn.h"
//using std::string;
//namespace gui { namespace control { class CStaticBmp; }; };
// CQrcodeViewerDlg dialog
namespace core { class CUserInfo; };
class CBaiduMapDlg;

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
	static void __stdcall OnCurUserChanged(void* udata, const core::CUserInfo* user);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	CString m_md5_path;
	CString m_bmp_path;
	//void GenerateAcct(char* buff, int buff_size);
	void InitAcct(int user_priority);
	void InitLocation();
	void InitCom();
	void SaveComConfigure(BOOL bRem = FALSE, int nCom = 0, BOOL bAuto = FALSE);
	
	DECLARE_MESSAGE_MAP()
public:
	gui::control::CStaticBmp m_staticEx;
//	CEdit m_csr_acct;
	virtual BOOL OnInitDialog();
	CString m_acct_text;
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	CStatic m_staticBaiduMap;
	CEdit m_addr;
	CEdit m_x;
	CEdit m_y;
private:
	CBaiduMapDlg* m_map1;
public:
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButtonLocateAuto();
	afx_msg void OnBnClickedButtonLocateToAddr();
	afx_msg void OnBnClickedButtonLocateToCoor();
protected:
	afx_msg LRESULT OnChosenBaiduPt(WPARAM wParam, LPARAM lParam);
public:
	CComboBox m_cmbCom;
	CButton m_btnCheckCom;
	CButton m_btnConnCom;
	CButton m_chkRemCom;
	CButton m_chkAutoConnCom;
	afx_msg void OnBnClickedButtonCheckCom();
	afx_msg void OnBnClickedButtonConnGsm();
	afx_msg void OnBnClickedCheck2();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedButton2();
	CButton m_btnAutoLocate;
	afx_msg void OnBnClickedButtonSavePhone();
	CEdit m_phone;
	CButton m_btnSaveCsrAcct;
	afx_msg void OnBnClickedButton3();
	CButton m_btnTest;
	afx_msg void OnBnClickedButtonMgrVideoDevice();
	afx_msg void OnBnClickedButtonMgrVideoUser();
	CIPAddressCtrl m_ip_private_cloud;
	CEdit m_port_private_cloud;
	afx_msg void OnBnClickedButtonSavePrivateCloud();
	CButton m_btnSavePrivateCloud;
};
