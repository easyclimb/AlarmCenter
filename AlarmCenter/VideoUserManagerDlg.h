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
	void InsertUserListEzviz(video::ezviz::video_user_info_ezviz_ptr userInfo);
	void InsertUserListJovision(video::jovision::video_user_info_jovision_ptr userInfo);
	void InsertDeviceListEzviz(video::ezviz::video_device_info_ezviz_ptr deviceInfo);
	void InsertDeviceListJovision(video::jovision::video_device_info_jovision_ptr deviceInfo);
	void UpdateUserListEzviz(int nItem, video::ezviz::video_user_info_ezviz_ptr userInfo);
	void UpdateUserListJovision(int nItem, video::jovision::video_user_info_jovision_ptr userInfo);
	void UpdateDeviceListEzviz(int nItem, video::ezviz::video_device_info_ezviz_ptr deviceInfo);
	void UpdateDeviceListJovision(int nItem, video::jovision::video_device_info_jovision_ptr deviceInfo);
	void ResetUserListSelectionInfoEzviz();
	void ResetUserListSelectionInfoJovision();
	void ResetDeviceListSelectionInfoEzviz();
	void ResetDeviceListSelectionInfoJovision();
	void ShowUsersDeviceListEzviz(video::ezviz::video_user_info_ezviz_ptr userInfo);
	void ShowUsersDeviceListJovision(video::jovision::video_user_info_jovision_ptr userInfo);
	void ShowDeviceInfoEzviz(video::ezviz::video_device_info_ezviz_ptr device);
	void ShowDeviceInfoJovision(video::jovision::video_device_info_jovision_ptr device);
	bool CheckZoneInfoExsist(const video::zone_uuid& zone);
private:
	video::ezviz::video_user_info_ezviz_ptr m_curSelUserInfoEzviz;
	video::ezviz::video_device_info_ezviz_ptr m_curSelDeviceInfoEzviz;
	int m_curselUserListItemEzviz;
	int m_curselDeviceListItemEzviz;

	video::jovision::video_user_info_jovision_ptr m_curSelUserInfoJovision = nullptr;
	video::jovision::video_device_info_jovision_ptr m_curSelDeviceInfoJovision = nullptr;
	int m_curselUserListItemJovision = -1;
	int m_curselDeviceListItemJovision = -1;
	
public:
	CMyListCtrl m_listUserEzviz;
	CMyListCtrl m_listDeviceEzviz;
	CStatic m_groupDevice;
	virtual BOOL OnInitDialog();
	CMyListCtrl m_listDeviceJovision;
	afx_msg void OnLvnItemchangedListUserEzviz(NMHDR *pNMHDR, LRESULT *pResult);
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
	afx_msg void OnLvnItemchangedListDeviceEzviz(NMHDR *pNMHDR, LRESULT *pResult);
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
	afx_msg void OnNMDblclkListDeviceEzviz(NMHDR *pNMHDR, LRESULT *pResult);
	CTabCtrl m_tab_users;
	CStatic m_groupUser;
	CListCtrl m_listUserJovision;
	afx_msg void OnTcnSelchangeTabUsers(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchangedListUserJovision(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchangedListDeviceJovision(NMHDR *pNMHDR, LRESULT *pResult);
	CEdit m_token_or_name;
	CEdit m_time_or_passwd;
	CStatic m_static_token_or_name;
	CStatic m_static_time_or_passwd;
	afx_msg void OnBnClickedButtonAddDevice();
	CStatic m_static_dev_name;
	CStatic m_static_note;
	CStatic m_static_verify_code;
	CButton m_chk_by_sse;
	CEdit m_sse;
	CStatic m_static_ip;
	CIPAddressCtrl m_ip;
	CStatic m_static_port;
	CEdit m_port;
	CStatic m_static_user_name;
	CEdit m_user_name;
	CStatic m_static_user_passwd;
	CEdit m_user_passwd;
	afx_msg void OnBnClickedCheckBySseId();
	CStatic m_static_sse;
	afx_msg void OnNMDblclkListDeviceJovision(NMHDR *pNMHDR, LRESULT *pResult);
};
