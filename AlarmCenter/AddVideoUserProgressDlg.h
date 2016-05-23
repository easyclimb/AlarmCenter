#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "VideoManager.h"
#include "VideoUserInfoEzviz.h"

// CAddVideoUserProgressDlg dialog

class CAddVideoUserProgressDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAddVideoUserProgressDlg)

public:
	CAddVideoUserProgressDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CAddVideoUserProgressDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ADD_VIDEO_USER_PROGRESS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_name;
	CString m_phone;
	video::video_manager::VideoEzvizResult m_result;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CProgressCtrl m_progress;
	CStatic m_staticTime;
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DWORD m_dwStart;
};
