#pragma once
#include "video.h"
#include "VideoPlayerCtrl.h"
#include "afxwin.h"
#include "afxdtctl.h"

// CVideoRecordPlayerDlg dialog

class CVideoRecordPlayerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CVideoRecordPlayerDlg)

public:
	CVideoRecordPlayerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVideoRecordPlayerDlg();

	video::jovision::video_device_info_jovision_ptr device_ = nullptr;
	video::jovision::JCLink_t link_id_ = -1;
	bool automatic_ = false;
	

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_VIDEO_RECORD_PLAYER };
#endif


private:
	std::vector<video::jovision::JCRecFileInfo> rec_file_infos_;
	std::chrono::steady_clock::time_point tp_;
	int counter_ = 10;
	bool previewing_ = false;

protected:
	void AddLogItem(const CString& log);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	void HandleJovisionMsg(const video::jovision::jovision_msg_ptr& msg);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnClose();
	CVideoPlayerCtrl m_player;
	CStatic m_group_rec_list;
	CListBox m_list_rec;
	CDateTimeCtrl m_ctrl_date;
	CButton m_btn_get_rec_list;
	afx_msg void OnBnClickedButtonGetRecList();
	CStatic m_group_logs;
	CListBox m_list_log;
protected:
	afx_msg LRESULT OnJcGetRecFileList(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLbnDblclkList1();
protected:
	afx_msg LRESULT OnJcResetStream(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnMove(int x, int y);
	afx_msg void OnDestroy();
};
