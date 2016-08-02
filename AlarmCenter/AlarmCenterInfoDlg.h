#pragma once
#include "StaticBmp.h"
#include "afxwin.h"
#include <iostream>
#include "afxcmn.h"
#include "core.h"

//using std::string;
//namespace gui { namespace control { class CStaticBmp; }; };
// CAlarmCenterInfoDlg dialog

//class CBaiduMapDlg;
class CVideoUserManagerDlg;

class CAlarmCenterInfoDlg : public CDialogEx
{
	class CurUserChangedObserver;
	std::shared_ptr<CurUserChangedObserver> m_cur_user_changed_observer;


	DECLARE_DYNAMIC(CAlarmCenterInfoDlg)

public:
	CAlarmCenterInfoDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CAlarmCenterInfoDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_CSR_ACCT };
	std::string m_acct;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	CString m_md5_path;
	CString m_bmp_path;
	void InitAcct(int user_priority);
	void InitLocation();
	void InitCom();
	void SaveComConfigure(BOOL bRem = FALSE, int nCom = 0, BOOL bAuto = FALSE);
	
	DECLARE_MESSAGE_MAP()
public:
	CString m_acct_text;
	CEdit m_x;
	CEdit m_y;

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
	CEdit m_phone; 
	CIPAddressCtrl m_ip_private_cloud; 
	CButton m_btnSavePrivateCloud;
	CEdit m_port_private_cloud;
	afx_msg void OnBnClickedButtonCheckCom();
	afx_msg void OnBnClickedButtonConnGsm();
	afx_msg void OnBnClickedCheck2();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedButtonMgrVideoUser();
	afx_msg void OnBnClickedButtonSavePrivateCloud();
	afx_msg void OnBnClickedButtonShowMap();
	CIPAddressCtrl m_server_ip;
	CEdit m_server_port;
	CIPAddressCtrl m_server_bk_ip;
	CEdit m_server_bk_port;
	afx_msg void OnBnClickedButtonSaveServerInfo();
	CEdit m_listening_port;
	CButton m_btnSaveNetworkInfo;
	CEdit m_ezviz_app_key;
	CComboBox m_cmb_switch_language;
	CButton m_btnRestartApplication;
	afx_msg void OnCbnSelchangeComboAppLanguage();
	afx_msg void OnBnClickedButtonRestartApp();
	CStatic m_group_center_addr;
	CStatic m_static_center_coor;
	CButton m_btn_see_center_map;
	CStatic m_group_sms_mod;
	CStatic m_static_serial_port;
	CStatic m_group_network;
	CStatic m_static_csr_acct;
	CStatic m_static_note;
	CStatic m_static_listening_port;
	CStatic m_static_server1;
	CStatic m_static_server2;
	CStatic m_group_ezviz;
	CStatic m_static_ezviz_ip_port;
	CStatic m_static_ezviz_app_key;
	CStatic m_group_video;
	CButton m_btn_video_user_mgr;
	CStatic m_group_language;
	CStatic m_static_cur_lang;
	CStatic m_static_note_reboot;
};
