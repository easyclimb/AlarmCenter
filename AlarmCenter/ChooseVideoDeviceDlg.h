#pragma once
#include "afxwin.h"
#include "video.h"

// CChooseVideoDeviceDlg dialog

class CChooseVideoDeviceDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CChooseVideoDeviceDlg)

public:
	CChooseVideoDeviceDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CChooseVideoDeviceDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_CHOOSE_VIDEO_DEVICE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListBox m_userList;
	CListBox m_devList;
	video::CVideoDeviceInfoPtr m_dev;
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	afx_msg void OnLbnSelchangeListUser();
	afx_msg void OnLbnSelchangeListDev();
	CButton m_btnOk;
};
