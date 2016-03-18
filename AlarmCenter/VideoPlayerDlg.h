#pragma once
#include "video.h"
#include "VideoPlayerCtrl.h"
#include "afxwin.h"
#include "SdkMgrEzviz.h"
#include "afxcmn.h"
#include <algorithm>
#include "core.h"

// CVideoPlayerDlg dialog
class CVideoPlayerDlg;
extern CVideoPlayerDlg* g_videoPlayerDlg;
class CVideoPlayerDlg : public CDialogEx
{
	class CurUserChangedObserver : public dp::observer<core::CUserInfoPtr>
	{
	public:
		explicit CurUserChangedObserver(CVideoPlayerDlg* dlg) : _dlg(dlg) {}
		virtual void on_update(const core::CUserInfoPtr& ptr) {
			if (_dlg) {
				_dlg->OnCurUserChangedResult(ptr);
			}
		}
	private:
		CVideoPlayerDlg* _dlg;
	};

	std::shared_ptr<CurUserChangedObserver> m_cur_user_changed_observer;
	void OnCurUserChangedResult(const core::CUserInfoPtr& user);
	typedef struct EzvizMessage
	{
		unsigned int iMsgType;
		unsigned int iErrorCode;
		std::string sessionId;
		std::string messageInfo;
		EzvizMessage() = default;
		EzvizMessage(unsigned int type, unsigned int code, const char* session, const char* msg)
			: iMsgType(type), iErrorCode(code), sessionId(), messageInfo()
		{
			if (session)
				sessionId = session;
			if (msg)
				messageInfo = msg;
		}
	}EzvizMessage;
	typedef std::shared_ptr<EzvizMessage> EzvizMessagePtr;
	typedef std::list<EzvizMessagePtr> CEzvizMsgList;

	typedef struct DataCallbackParam
	{
		const char* _flag = "abcd";
		CVideoPlayerDlg* _dlg;
		std::string _session_id;
		std::wstring _file_path;
		COleDateTime _startTime;
		DataCallbackParam() : _dlg(nullptr), _session_id(), _file_path(), _startTime(){}
		DataCallbackParam(CVideoPlayerDlg* dlg, const std::string& session_id, const time_t& startTime) 
			: _dlg(dlg), _session_id(session_id), _file_path(), _startTime(startTime)
		{}

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
			_file_path = path.LockBuffer(); path.UnlockBuffer();
			return path;
		}
	}DataCallbackParam;


	typedef struct RecordVideoInfo
	{
		DataCallbackParam* _param;
		video::ZoneUuid _zone;
		video::ezviz::CVideoDeviceInfoEzvizPtr _device;
		int _level;
		
		CVideoPlayerCtrl* _ctrl;
		RecordVideoInfo() : _param(nullptr), _zone(), _device(nullptr), _ctrl(nullptr), _level(0) {}
		RecordVideoInfo(DataCallbackParam* param, const video::ZoneUuid& zone, 
						video::ezviz::CVideoDeviceInfoEzvizPtr device, CVideoPlayerCtrl* ctrl, int level) 
			:_param(param), _zone(zone), _device(device), _ctrl(ctrl), _level(level) {}
		~RecordVideoInfo() { SAFEDELETEDLG(_ctrl); SAFEDELETEP(_param); }
	}RecordVideoInfo;
	typedef std::shared_ptr<RecordVideoInfo> RecordVideoInfoPtr;
	typedef std::list<RecordVideoInfoPtr> CRecordVideoInfoList;

	DECLARE_DYNAMIC(CVideoPlayerDlg)

public:

	bool is_valid_data_record_param(DataCallbackParam* param) {
		AUTO_LOG_FUNCTION;
		std::lock_guard<std::recursive_mutex> lock(m_lock4CurRecordingInfoList);
		for (auto info : m_curRecordingInfoList) {
			if (info->_param == param) {
				return true;
			}
		}
		return false;
	}
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
	video::CVideoDeviceInfoPtr m_curPlayingDevice;
	CRecordVideoInfoList m_curRecordingInfoList;
	std::recursive_mutex m_lock4CurRecordingInfoList;
	CVideoPlayerCtrl m_player;
	DWORD m_dwPlayerStyle;
	//std::list<CVideoPlayerCtrl*> m_playerList;
	CEzvizMsgList m_ezvizMsgList;
	std::mutex m_lock4EzvizMsgQueue;
	//int m_level;
	std::list<video::ezviz::CVideoDeviceInfoEzvizPtr> m_wait2playDevList;
	std::mutex m_lock4Wait2PlayDevList;
	CString m_title;
protected:
	void LoadPosition();
	void SavePosition();
	void ShowOtherCtrls(BOOL bShow = TRUE);
	void EnableOtherCtrls(BOOL bAble = TRUE, int level = 0);
	void PlayVideoEzviz(video::ezviz::CVideoDeviceInfoEzvizPtr device, int speed);
	void StopPlayEzviz(video::ezviz::CVideoDeviceInfoEzvizPtr device);
	void StopPlay(RecordVideoInfoPtr info);
	void EnqueEzvizMsg(EzvizMessagePtr msg);
	void HandleEzvizMsg(EzvizMessagePtr msg);
	void PtzControl(video::ezviz::CSdkMgrEzviz::PTZCommand command, video::ezviz::CSdkMgrEzviz::PTZAction action);
public:
	void PlayVideoByDevice(video::CVideoDeviceInfoPtr device, int speed);
	void PlayVideo(const video::ZoneUuid& zone);
	void StopPlay();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnMove(int x, int y);
	static void __stdcall messageHandler(const char *szSessionId,
										 unsigned int iMsgType,
										 unsigned int iErrorCode,
										 const char *pMessageInfo,
										 void *pUser);

	static void __stdcall videoDataHandler(video::ezviz::CSdkMgrEzviz::DataType enType,
										   char* const pData,
										   int iLen,
										   void* pUser);
protected:
	afx_msg LRESULT OnInversioncontrol(WPARAM wParam, LPARAM lParam);
public:
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
	afx_msg void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);
	CStatic m_groupSpeed;
	CStatic m_groupPtz;
	CStatic m_groupControl;
	CListCtrl m_ctrl_play_list;
	CEdit m_ctrl_rerord_minute;
	afx_msg void OnBnClickedButtonSave();
protected:

	void InsertList(const RecordVideoInfoPtr& info);
public:
	afx_msg void OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHdnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
	CButton m_btn_save;
};
