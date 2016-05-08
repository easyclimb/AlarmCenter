#pragma once
#include "video.h"
#include "VideoPlayerCtrl.h"
#include "afxwin.h"
#include "SdkMgrEzviz.h"
#include "afxcmn.h"
#include <fstream>
#include <algorithm>
#include "core.h"

// CVideoPlayerDlg dialog

typedef std::shared_ptr<CVideoPlayerCtrl> player;

class CVideoPlayerDlg;
extern CVideoPlayerDlg* g_videoPlayerDlg;
class CVideoPlayerDlg : public CDialogEx
{
protected: // observers
	class CurUserChangedObserver : public dp::observer<core::user_info_ptr>
	{
	public:
		explicit CurUserChangedObserver(CVideoPlayerDlg* dlg) : _dlg(dlg) {}
		virtual void on_update(const core::user_info_ptr& ptr) {
			if (_dlg) {
				_dlg->OnCurUserChangedResult(ptr);
			}
		}
	private:
		CVideoPlayerDlg* _dlg;
	};

	std::shared_ptr<CurUserChangedObserver> m_cur_user_changed_observer;
	void OnCurUserChangedResult(const core::user_info_ptr& user);

protected: // structs

	// ezviz callback define
	typedef struct ezviz_msg
	{
		unsigned int iMsgType;
		unsigned int iErrorCode;
		std::string sessionId;
		std::string messageInfo;
		ezviz_msg() = default;
		ezviz_msg(unsigned int type, unsigned int code, const char* session, const char* msg)
			: iMsgType(type), iErrorCode(code), sessionId(), messageInfo()
		{
			if (session)
				sessionId = session;
			if (msg)
				messageInfo = msg;
		}
	}ezviz_msg;
	typedef std::shared_ptr<ezviz_msg> ezviz_msg_ptr;
	typedef std::list<ezviz_msg_ptr> ezviz_msg_ptr_list;
	ezviz_msg_ptr_list ezviz_msg_list_;
	std::mutex lock_4_ezviz_msg_queue_;

	typedef struct DataCallbackParam
	{
		CVideoPlayerDlg* _dlg;
		char _session_id[1024];
		wchar_t _file_path[4096];
		DWORD _start_time;
		DataCallbackParam() : _dlg(nullptr), _session_id(), _file_path(), _start_time(0) {}
		DataCallbackParam(CVideoPlayerDlg* dlg, const std::string& session_id, DWORD startTime) 
			: _dlg(dlg), _session_id(), _file_path(), _start_time(startTime)
		{
			strcpy(_session_id, session_id.c_str());
		}

		~DataCallbackParam() {}

		CString FormatFilePath(int user_id, const std::wstring& user_name, int dev_id, const std::wstring& dev_note)
		{
			USES_CONVERSION;
			auto name = user_name;
			auto note = dev_note;
			static const wchar_t filter[] = {L'\\', L'/', L':', L'*', L'?', L'"', L'<', L'>', L'|', L' '};
			for (auto c : filter) {
				std::replace(name.begin(), name.end(), c, L'_');
				std::replace(note.begin(), note.end(), c, L'_');
			}
			CString path, user_path; 
			path.Format(L"%s\\data\\video_record", GetModuleFilePath());
			CreateDirectory(path, nullptr);
			user_path.Format(L"\\%d-%s", user_id, name.c_str());
			path += user_path;
			CreateDirectory(path, nullptr);
			CString file; file.Format(L"\\%s-%d-%s.mp4",
									  CTime::GetCurrentTime().Format(L"%Y-%m-%d_%H-%M-%S"),
									  dev_id, dev_note.c_str());
			path += file;
			wcscpy(_file_path, path.LockBuffer()); path.UnlockBuffer();
			return path;
		}
	}DataCallbackParam;

	typedef struct record
	{
		bool started_ = false;
		DataCallbackParam* _param;
		video::ZoneUuid _zone;
		video::ezviz::CVideoDeviceInfoEzvizPtr _device;
		int _level;
		
		player player_;
		record() : _param(nullptr), _zone(), _device(nullptr), player_(nullptr), _level(0) {}
		record(DataCallbackParam* param, const video::ZoneUuid& zone,
						video::ezviz::CVideoDeviceInfoEzvizPtr device, const player& player, int level)
			:_param(param), _zone(zone), _device(device), player_(player), _level(level) {}
		~record() { SAFEDELETEP(_param);  }
	}record;
	typedef std::shared_ptr<record> record_ptr;
	typedef std::list<record_ptr> record_list;

	video::CVideoDeviceInfoPtr m_curPlayingDevice;
	record_list record_list_;
	std::recursive_mutex lock_4_record_list_;

	struct player_ex {
		bool used = false;
		player player = nullptr;
		CRect rc = { 0 };
	};

	typedef std::shared_ptr<player_ex> player_ex_ptr;

	std::vector<player_ex_ptr> player_ex_vector_;
	std::list<player> buffered_players_;

	player player_op_get_free_player();
	player player_op_create_new_player();
	void player_op_recycle_player(const player& player);
	void player_op_bring_player_to_front(const player& player);
	void player_op_update_players_size_with_m_player();
	bool player_op_is_front_end_player(const player& player) const;
	void player_op_set_same_time_play_video_route(const int n);

	record_ptr record_op_get_record_info_by_device(const video::CVideoDeviceInfoPtr& device);
	bool record_op_is_valid(DataCallbackParam* param) {
		AUTO_LOG_FUNCTION;
		std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_);
		for (auto info : record_list_) {
			if (info->_param == param) {
				return true;
			}
		}
		return false;
	}

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
	WINDOWPLACEMENT m_rcNormal;
	WINDOWPLACEMENT m_rcFullScreen;
	WINDOWPLACEMENT m_rcNormalPlayer;
	DWORD m_dwPlayerStyle;
	std::list<video::ezviz::CVideoDeviceInfoEzvizPtr> m_wait2playDevList;
	std::mutex m_lock4Wait2PlayDevList;
	CString m_title;

protected:
	void InsertList(const record_ptr& info);
	void LoadPosition();
	void SavePosition();
	void ShowOtherCtrls(BOOL bShow = TRUE);
	void EnableControlPanel(BOOL bAble = TRUE, int level = 0);
	void PlayVideoEzviz(video::ezviz::CVideoDeviceInfoEzvizPtr device, int speed);
	void StopPlayEzviz(video::ezviz::CVideoDeviceInfoEzvizPtr device);
	void StopPlayByRecordInfo(record_ptr info);
	void EnqueEzvizMsg(const ezviz_msg_ptr& msg);
	void HandleEzvizMsg(const ezviz_msg_ptr& msg);
	void PtzControl(video::ezviz::CSdkMgrEzviz::PTZCommand command, video::ezviz::CSdkMgrEzviz::PTZAction action);
	
public:
	void PlayVideoByDevice(video::CVideoDeviceInfoPtr device, int speed);
	void PlayVideo(const video::ZoneUuid& zone);
	void StopPlayCurselVideo();
	

	static void __stdcall messageHandler(const char *szSessionId,
										 unsigned int iMsgType,
										 unsigned int iErrorCode,
										 const char *pMessageInfo,
										 void *pUser);

	static void __stdcall videoDataHandler(video::ezviz::CSdkMgrEzviz::DataType enType,
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
	afx_msg void OnHdnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
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
	CStatic m_status;
	CStatic m_groupSpeed;
	CStatic m_groupPtz;
	CStatic m_groupControl;
	CListCtrl m_ctrl_play_list;
	CEdit m_ctrl_rerord_minute;
	CButton m_btn_save;
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
};
