#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "video.h"

// CAddVideoDeviceJovisionDlg dialog

class CAddVideoDeviceJovisionDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAddVideoDeviceJovisionDlg)

public:
	CAddVideoDeviceJovisionDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAddVideoDeviceJovisionDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ADD_VIDEO_DEVICE_JOVISION };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	video::jovision::video_user_info_jovision_ptr user_ = nullptr;
	CEdit m_cloud_sse_id;
	CEdit m_user_name;
	CEdit m_user_passwd;
	CIPAddressCtrl m_dev_ip;
	CEdit m_dev_port;
	CButton m_chk_by_sse;
	CButton m_chk_use_default_user_name;
	CButton m_chk_use_default_user_passwd;
	afx_msg void OnBnClickedCheckUseCloudSse();
	afx_msg void OnBnClickedCheckUseDefaultUserName();
	afx_msg void OnBnClickedCheckUseDefaultUserPasswd();
	virtual BOOL OnInitDialog();
	afx_msg void OnEnChangeEditName();
	afx_msg void OnEnChangeEditPasswd();
	afx_msg void OnBnClickedOk();
};
