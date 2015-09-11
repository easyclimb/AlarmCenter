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
	void UpdateUserList(int nItem, core::video::ezviz::CVideoUserInfoEzviz* userInfo);
	void ResetUserListSelectionInfo();

private:
	core::video::CVideoUserInfo* m_curSelUserInfo;
	core::video::CVideoDeviceInfo* m_curSelDeviceInfo;
	int m_privilege;
	int m_curSelUserListItem;
public:
	CMyListCtrl m_listUser;
	CMyListCtrl m_listDevice;
	CStatic m_groupDevice;
	virtual BOOL OnInitDialog();
	CMyListCtrl m_listDevice2;
	afx_msg void OnLvnItemchangedListUser(NMHDR *pNMHDR, LRESULT *pResult);
	CEdit m_id;
	CEdit m_productor;
	CEdit m_name;
	CEdit m_phone;
	CButton m_btnDelUser;
	CButton m_btnUpdateUser;
	CButton m_btnAddUser;
	afx_msg void OnBnClickedButtonSaveChange();
	afx_msg void OnBnClickedButtonDelUser();
	afx_msg void OnBnClickedButtonAddUser();
};
