#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "MyListCtrl.h"

#include "video.h"
// CVideoUserManagerDlg dialog

class CVideoUserManagerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CVideoUserManagerDlg)

public:
	CVideoUserManagerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVideoUserManagerDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_MGR_VIDEO_USER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	void InitUserList();
	void InsertUserList(core::video::ezviz::CVideoUserInfoEzviz* userInfo);
	void InsertUserList(core::video::normal::CVideoUserInfoNormal* userInfo);
	void InsertDeviceList(core::video::ezviz::CVideoDeviceInfoEzviz* deviceInfo);
	void InsertDeviceList(core::video::normal::CVideoDeviceInfoNormal* deviceInfo);

private:
	core::video::CVideoUserInfo* m_curSelUserInfo;
	core::video::CVideoDeviceInfo* m_curSelDeviceInfo;
public:
	CMyListCtrl m_listUser;
	CMyListCtrl m_listDevice;
	CStatic m_groupDevice;
	virtual BOOL OnInitDialog();
	CMyListCtrl m_listDevice2;
	afx_msg void OnLvnItemchangedListUser(NMHDR *pNMHDR, LRESULT *pResult);
};
