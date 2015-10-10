#pragma once
#include "video.h"
#include "VideoPlayerCtrl.h"
#include "afxwin.h"
#include "SdkMgrEzviz.h"

// CVideoPlayerDlg dialog
class CVideoPlayerDlg;
extern CVideoPlayerDlg* g_videoPlayerDlg;
class CVideoPlayerDlg : public CDialogEx
{
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
	typedef std::list<EzvizMessage*> CEzvizMsgList;

	typedef struct DataCallbackParam
	{
		CVideoPlayerDlg* _dlg;
		std::string _session_id;
		std::string _file_path;
		COleDateTime _startTime;
		DataCallbackParam() : _dlg(nullptr), _session_id(), _file_path(), _startTime(){}
		DataCallbackParam(CVideoPlayerDlg* dlg, const std::string& session_id, const time_t& startTime) 
			: _dlg(dlg), _session_id(session_id), _file_path(), _startTime(startTime)
		{}

		CString FormatFilePath(const std::string& cameraId)
		{
			USES_CONVERSION;
			CString path; path.Format(L"%s\\video_record\\%s-%s.mp4",
									  GetModuleFilePath(),
									  CTime::GetCurrentTime().Format(L"%Y-%m-%d_%H-%M-%S"),
									  A2W(cameraId.c_str()));
			_file_path = W2A(path);
			return path;
		}
	}DataCallbackParam;

	typedef struct RecordVideoInfo
	{
		DataCallbackParam* _param;
		video::ZoneUuid _zone;
		video::ezviz::CVideoDeviceInfoEzviz* _device;
		
		CVideoPlayerCtrl* _ctrl;
		RecordVideoInfo() : _param(nullptr), _zone(), _device(nullptr), _ctrl(nullptr) {}
		RecordVideoInfo(DataCallbackParam* param, const video::ZoneUuid& zone, 
						video::ezviz::CVideoDeviceInfoEzviz* device, CVideoPlayerCtrl* ctrl) 
			:_param(param), _zone(zone), _device(device), _ctrl(ctrl) {}
		~RecordVideoInfo() { SAFEDELETEDLG(_ctrl); }
	}RecordVideoInfo;

	typedef std::list<RecordVideoInfo*> CRecordVideoInfoList;

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
	video::CVideoDeviceInfo* m_curPlayingDevice;
	CRecordVideoInfoList m_curRecordingInfoList;
	CLock m_lock4CurRecordingInfoList;
	CVideoPlayerCtrl m_player;
	DWORD m_dwPlayerStyle;
	//std::list<CVideoPlayerCtrl*> m_playerList;
	CEzvizMsgList m_ezvizMsgList;
	CLock m_lock4EzvizMsgQueue;
	int m_level;
	std::list<video::ezviz::CVideoDeviceInfoEzviz*> m_wait2playDevList;
	CLock m_lock4Wait2PlayDevList;
protected:
	void LoadPosition();
	void SavePosition();
	void ShowOtherCtrls(BOOL bShow = TRUE);
	void EnableOtherCtrls(BOOL bAble = TRUE);
	void PlayVideoEzviz(video::ezviz::CVideoDeviceInfoEzviz* device, int speed);
	void StopPlay(video::ezviz::CVideoDeviceInfoEzviz* device);
	void StopPlay(RecordVideoInfo* info);
	void EnqueEzvizMsg(EzvizMessage* msg);
	void HandleEzvizMsg(EzvizMessage* msg);
	void PtzControl(video::ezviz::CSdkMgrEzviz::PTZCommand command, video::ezviz::CSdkMgrEzviz::PTZAction action);
public:
	void PlayVideoByDevice(video::CVideoDeviceInfo* device, int speed);
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
};
