#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "MyListCtrl.h"

#include "video.h"
// CVideoUserManagerDlg dialog
class CVideoUserManagerDlg;
extern CVideoUserManagerDlg* g_videoUserMgrDlg;

class CVideoUserManagerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CVideoUserManagerDlg)
	class CurUserChangedObserver;
	std::shared_ptr<CurUserChangedObserver> m_cur_user_changed_observer;
public:
	CVideoUserManagerDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CVideoUserManagerDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_MGR_VIDEO_USER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	void InitUserList();
	void InsertUserList(video::ezviz::video_user_info_ezviz_ptr userInfo);
	void InsertUserList(video::jovision::video_user_info_jovision_ptr userInfo);
	void InsertDeviceList(video::ezviz::video_device_info_ezviz_ptr deviceInfo);
	void InsertDeviceList(video::jovision::video_device_info_jovision_ptr deviceInfo);
	void UpdateUserList(int nItem, video::ezviz::video_user_info_ezviz_ptr userInfo);
	void UpdateDeviceList(int nItem, video::ezviz::video_device_info_ezviz_ptr deviceInfo);
	void ResetUserListSelectionInfo();
	void ResetDeviceListSelectionInfo();
	void ShowUsersDeviceList(video::video_user_info_ptr userInfo);
	void ShowDeviceInfo(video::ezviz::video_device_info_ezviz_ptr device);
	bool CheckZoneInfoExsist(const video::zone_uuid& zone);
private:
	video::video_user_info_ptr m_curSelUserInfo;
	video::video_device_info_ptr m_curSelDeviceInfo;
	int m_curselUserListItem;
	int m_curselDeviceListItem;
	
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
	CButton m_btnRefreshDeviceList;
	afx_msg void OnBnClickedButtonRefreshDeviceList();
	CEdit m_idDev;
	CEdit m_nameDev;
	CEdit m_noteDev;
	CEdit m_devCode;
	CEdit m_zone;
	CButton m_btnBindOrUnbind;
	CButton m_chkAutoPlayVideo;
	CButton m_btnAddDevice;
	CButton m_btnDelDevice;
	CButton m_btnSaveDevChange;
	CButton m_btnRefreshDev;
	afx_msg void OnLvnItemchangedListDevice(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonBindOrUnbind();
	afx_msg void OnBnClickedCheckAutoPlayVideo();
	afx_msg void OnBnClickedButtonSaveDev();
	CButton m_btnPlayVideo;
	afx_msg void OnBnClickedButtonPlay();
	virtual void OnOK();
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButtonUnbind();
	CWnd* m_observerDlg;
protected:
	afx_msg LRESULT OnVideoInfoChanged(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnBnClickedButtonDelDevice();
	CButton m_btnUnbind;
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnNMDblclkListDevice(NMHDR *pNMHDR, LRESULT *pResult);
	CTabCtrl m_tab_users;
	CStatic m_groupUser;
	CListCtrl m_list_user_jovision;
	afx_msg void OnTcnSelchangeTabUsers(NMHDR *pNMHDR, LRESULT *pResult);
};
