
// TestHikvisionDlg.h : header file
//

#pragma once

#include <string>
#include <list>
#include "afxwin.h"


enum PTZCMD
{
	DIRECT_UP,
	DIRECT_DOWN,
	DIRECT_LEFT,
	DIRECT_RIGHT,
	DIRECT_UPLEFT,
	DIRECT_DOWNLEFT,
	DIRECT_UPRIGHT,
	DIRECT_DOWNRIGHT,
	PTZ_ZOOMIN,
	PTZ_ZOOMOUT,
	PTZ_FOCUSNEAR,
	PTZ_FOCUSFAR,
	PTZ_IRISSTARTUP,
	PTZ_IRISSTOPDOWN,
	PTZ_LIGHT,
	PTZ_WIPER,
	PTZ_AUTO
};

enum PTZACT
{
	ACT_START,
	ACT_STOP,
};

#define PTZ_SPEED 7 //默认使用云台控制速度为7

// CTestHikvisionDlg dialog
class CTestHikvisionDlg : public CDialogEx
{
// Construction
public:
	CTestHikvisionDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_TESTHIKVISION_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
public:
	HICON m_hIcon;
	std::string m_appKey;
	std::string m_appSecret;
	std::string m_accessToken;
	std::string m_sessionId;
	std::string m_videoPath;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

protected:
	std::string getCameraId();
	void StartRealPlay(int iVideoLevel);
	void ptzCtrlStart(PTZCMD cmd);
	void ptzCtrlStop(PTZCMD cmd);
	void ptzCtrl(PTZCMD cmd, PTZACT act);
	void LoginByOAuth();
	void LoginByPrivateCloud();
public:
	CListBox m_list;
	afx_msg void OnLbnSelchangeList1();
	CEdit m_cameraId;
	CEdit m_cameraName;
	CEdit m_cameraNo;
	CEdit m_defence;
	CEdit m_deviceId;
	CEdit m_deviceName;
	CEdit m_deviceSerial;
	CEdit m_isEncrypt;
	CEdit m_isShared;
	CEdit m_picUrl;
	CEdit m_status;
	afx_msg void OnBnClickedButton1();
	
	void SetVideoPath()
	{
		USES_CONVERSION;
		CString cameraId; m_cameraId.GetWindowTextW(cameraId);
		cameraId += CTime::GetCurrentTime().Format(L"-%Y-%m-%d-%H-%M-%S");
		m_videoPath = "d:/hik_rec/my/";
		m_videoPath += W2A(cameraId);
		m_videoPath += ".mp4";
	}
	CStatic m_ctrlVideo;
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	CButton m_btnUp;
	CButton m_btnDown;
	CButton m_btnLeft;
	CButton m_btnRight;
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton7();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedButtonGetDevList();
	CButton m_btnLogin;
};
