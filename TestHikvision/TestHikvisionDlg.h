
// TestHikvisionDlg.h : header file
//

#pragma once

#include <string>
#include <list>
#include "afxwin.h"

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
	std::string m_accessToken;
	std::string m_sessonId;
	std::string m_videoPath;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

protected:
	void StartRealPlay(int iVideoLevel);
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
		m_videoPath = "d:/";
		m_videoPath += W2A(cameraId);
		m_videoPath += ".mp4";
	}
	CStatic m_ctrlVideo;
	afx_msg void OnDestroy();
};
