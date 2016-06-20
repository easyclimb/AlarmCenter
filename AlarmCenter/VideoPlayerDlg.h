#pragma once
#include "video.h"
#include "VideoPlayerCtrl.h"
#include "afxwin.h"
#include "SdkMgrEzviz.h"
#include "afxcmn.h"
#include <fstream>
#include <algorithm>
#include "core.h"
#include <set>

// CVideoPlayerDlg dialog

typedef std::shared_ptr<CVideoPlayerCtrl> player;
class CVideoRecordPlayerDlg;
typedef std::shared_ptr<CVideoRecordPlayerDlg> rec_player;

class CVideoPlayerDlg;
extern CVideoPlayerDlg* g_videoPlayerDlg;

class CVideoPlayerDlg : public CDialogEx
{
protected: 
	// observers
	class CurUserChangedObserver;
	std::shared_ptr<CurUserChangedObserver> m_cur_user_changed_observer;
	void OnCurUserChangedResult(const core::user_info_ptr& user);

protected: 
	// ezviz
	struct ezviz_msg;
	typedef std::shared_ptr<ezviz_msg> ezviz_msg_ptr;
	typedef std::list<ezviz_msg_ptr> ezviz_msg_ptr_list;
	ezviz_msg_ptr_list ezviz_msg_list_;
	std::mutex lock_4_ezviz_msg_queue_;
	struct DataCallbackParamEzviz;
	
public:

	// jovision
	struct jovision_msg;
	typedef std::shared_ptr<jovision_msg> jovision_msg_ptr;
	typedef std::list<jovision_msg_ptr> jovision_msg_ptr_list;
	void EnqueJovisionMsg(const jovision_msg_ptr& msg);
	void HandleJovisionMsg(const jovision_msg_ptr& msg);

protected:

	jovision_msg_ptr_list jovision_msg_list_;
	std::mutex lock_4_jovision_msg_queue_;

	struct record;
	typedef std::shared_ptr<record> record_ptr;
	typedef std::list<record_ptr> record_list;

	video::video_device_info_ptr m_curPlayingDevice;
	record_list record_list_;
	std::recursive_mutex lock_4_record_list_;

	struct player_ex;
	typedef std::shared_ptr<player_ex> player_ex_ptr;

	std::map<int, player_ex_ptr> player_ex_vector_;
	std::set<player> back_end_players_;
	std::list<player> player_buffer_;

	//player player_op_get_free_player();
	player player_op_create_new_player();
	void player_op_recycle_player(const player& player);
	void player_op_bring_player_to_front(const player& player);
	void player_op_update_players_size_with_m_player();
	bool player_op_is_front_end_player(const player& player) const;
	void player_op_set_same_time_play_video_route(const int n);
	void player_op_rebuild();
	void player_op_set_focus(const player& player);

	record_ptr record_op_get_record_info_by_device(const video::video_device_info_ptr& device);
	record_ptr record_op_get_record_info_by_player(const player& player);
	record_ptr record_op_get_record_info_by_link_id(int link_id);

	bool record_op_is_valid(DataCallbackParamEzviz* param);

	void delete_from_play_list_by_record(const record_ptr& record);

	//typedef std::list<std::shared_ptr<CVideoRecordPlayerDlg>> rec_players;
	//rec_players rec_players_ = {};

	DECLARE_DYNAMIC(CVideoPlayerDlg)

public:
	CVideoPlayerDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CVideoPlayerDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_VIDEO_PLAYER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	BOOL m_bInitOver;
	BOOL maximized_ = false;
	WINDOWPLACEMENT m_rcNormal;
	WINDOWPLACEMENT m_rcFullScreen;
	WINDOWPLACEMENT m_rcNormalPlayer;
	DWORD m_dwPlayerStyle;


	video::video_device_info_list m_wait2playDevList;
	std::mutex m_lock4Wait2PlayDevList;
	CString m_title;	

protected:
	void InsertList(const record_ptr& info);
	void LoadPosition();
	void SavePosition();
	void ShowOtherCtrls(BOOL bShow = TRUE);
	void EnableControlPanel(BOOL bAble = TRUE, int level = 0);
	void PlayVideoEzviz(video::ezviz::video_device_info_ezviz_ptr device, int speed);
	void PlayVideoJovision(video::jovision::video_device_info_jovision_ptr device, int speed);
	void StopPlayEzviz(video::ezviz::video_device_info_ezviz_ptr device);
	void StopPlayJovision(video::jovision::video_device_info_jovision_ptr device);
	void StopPlayByRecordInfo(record_ptr info);
	void EnqueEzvizMsg(const ezviz_msg_ptr& msg);
	void HandleEzvizMsg(const ezviz_msg_ptr& msg);
	void PtzControl(video::ezviz::sdk_mgr_ezviz::PTZCommand command, video::ezviz::sdk_mgr_ezviz::PTZAction action);
	void on_ins_play_start(const record_ptr& record);
	void on_ins_play_stop(record_ptr record);
	void on_ins_play_exception(const ezviz_msg_ptr& msg, const record_ptr& record);
	bool do_hd_verify(const video::ezviz::video_user_info_ezviz_ptr& user);

	void on_jov_play_start(const record_ptr& record);
	void on_jov_play_stop(const record_ptr& record);
	
public:
	
	void PlayVideoByDevice(video::video_device_info_ptr device, int speed);
	void PlayVideo(const video::zone_uuid& zone, const core::alarm_text_ptr at);
	void StopPlayCurselVideo();
	

	static void __stdcall messageHandler(const char *szSessionId,
										 unsigned int iMsgType,
										 unsigned int iErrorCode,
										 const char *pMessageInfo,
										 void *pUser);

	static void __stdcall videoDataHandler(video::ezviz::sdk_mgr_ezviz::DataType enType,
										   char* const pData,
										   int iLen,
										   void* pUser);

	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnMove(int x, int y);
	afx_msg LRESULT OnInversioncontrol(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedRadio3();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonCapture();
	afx_msg void OnBnClickedButtonUp();
	afx_msg void OnBnClickedButtonDown();
	afx_msg void OnBnClickedButtonLeft();
	afx_msg void OnBnClickedButtonRight();
	afx_msg void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeEditMinute();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBnClickedRadioSmooth2();
	afx_msg void OnBnClickedRadioBalance2();
	afx_msg void OnBnClickedRadioHd2();
	afx_msg void OnBnClickedRadio1Video();
	afx_msg void OnBnClickedRadio4Video();
	afx_msg void OnBnClickedRadio9Video();

	CVideoPlayerCtrl m_player;
	CButton m_btnStop;
	CButton m_btnCapture;
	CButton m_btnUp;
	CButton m_btnDown;
	CButton m_btnLeft;
	CButton m_btnRight;
	CButton m_radioSmooth;
	CButton m_radioBalance;
	CButton m_radioHD;
	CStatic m_groupSpeed;
	CStatic m_groupPtz;
	CStatic m_groupControl;
	CListCtrl m_ctrl_play_list;
	CEdit m_ctrl_rerord_minute;
	CStatic m_group_video_list;
	CStatic m_group_record_settings;
	CStatic m_static_note;
	CStatic m_static_minute;
	CButton m_radioGlobalSmooth;
	CButton m_radioGlobalBalance;
	CButton m_radioGlobalHD;
	CStatic m_staticNote2;
	CButton m_chk_1_video;
	CButton m_chk_4_video;
	CButton m_chk_9_video;
	CStatic m_static_group_cur_video;
	CSliderCtrl m_slider_volume;
	CStatic m_static_volume;
	CButton m_btn_voice_talk;
	afx_msg void OnBnClickedButtonVoiceTalk();
	afx_msg void OnTRBNThumbPosChangingSliderVolume(NMHDR *pNMHDR, LRESULT *pResult);
	CButton m_chk_volume;
	afx_msg void OnBnClickedCheckVolume();
	CStatic m_group_voice_talk;
	afx_msg void OnNMReleasedcaptureSliderVolume(NMHDR *pNMHDR, LRESULT *pResult);
	CButton m_btn_remote_config;
	afx_msg void OnBnClickedButtonRemoteConfig();
	CStatic m_group_alarm;
	CListBox m_list_alarm;
	CButton m_chk_auto_play_rec;
	afx_msg void OnBnClickedButtonOpenRec();
	afx_msg void OnBnClickedCheckAutoPlayRec();
};
