#pragma once
#include "StaticBmp.h"
#include "afxwin.h"
#include <iostream>
#include "afxcmn.h"
//using std::string;
//namespace gui { namespace control { class CStaticBmp; }; };
// CQrcodeViewerDlg dialog
namespace core { class CUserInfo; };
//class CBaiduMapDlg;
class CVideoUserManagerDlg;

class CQrcodeViewerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CQrcodeViewerDlg)

public:
	CQrcodeViewerDlg(CWnd* pParent = nullptr);   // standard constructor
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
	CString m_acct_text;
	CEdit m_addr;
	CEdit m_x;
	CEdit m_y;
private:
	CVideoUserManagerDlg* m_videoUserMgrDlg;
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButtonLocateAuto();
	afx_msg void OnBnClickedButtonLocateToAddr();
	afx_msg void OnBnClickedButtonLocateToCoor();
	afx_msg LRESULT OnChosenBaiduPt(WPARAM wParam, LPARAM lParam);
public:
	CComboBox m_cmbCom;
	CButton m_btnCheckCom;
	CButton m_btnConnCom;
	CButton m_chkRemCom;
	CButton m_chkAutoConnCom; 
	CButton m_btnAutoLocate; 
	CEdit m_phone; 
	CButton m_btnTest; 
	CIPAddressCtrl m_ip_private_cloud; 
	CButton m_btnSavePrivateCloud;
	CEdit m_port_private_cloud;
	CButton m_btnSaveCsrAcct;
	afx_msg void OnBnClickedButtonCheckCom();
	afx_msg void OnBnClickedButtonConnGsm();
	afx_msg void OnBnClickedCheck2();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButtonSavePhone();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButtonMgrVideoDevice();
	afx_msg void OnBnClickedButtonMgrVideoUser();
	afx_msg void OnBnClickedButtonSavePrivateCloud();
	afx_msg void OnBnClickedButtonShowMap();
};
