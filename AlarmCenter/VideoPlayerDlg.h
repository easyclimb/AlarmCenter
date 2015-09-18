#pragma once
#include "video.h"
#include "VideoPlayerCtrl.h"
#include <queue>

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
	typedef std::queue<EzvizMessage*> CEzvizMsgQueue;


	DECLARE_DYNAMIC(CVideoPlayerDlg)

public:
	CVideoPlayerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVideoPlayerDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_VIDEO_PLAYER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	BOOL m_bInitOver;
	CRect m_rcNormal;
	CRect m_rcNormalPlayer;
	video::CVideoDeviceInfo* m_curPlayingDevice;
	CVideoPlayerCtrl m_player;
	CEzvizMsgQueue m_ezvizMsgQueue;
	CLock m_lock4EzvizMsgQueue;
protected:
	void LoadPosition();
	void SavePosition();
	void CVideoPlayerDlg::PlayVideo(video::ezviz::CVideoDeviceInfoEzviz* device, int speed);
	void CVideoPlayerDlg::StopPlay(video::ezviz::CVideoDeviceInfoEzviz* device);
	CString GetEzvzErrorMessage(int errCode);
	void EnqueEzvizMsg(EzvizMessage* msg) { m_lock4EzvizMsgQueue.Lock(); m_ezvizMsgQueue.push(msg); m_lock4EzvizMsgQueue.UnLock(); }
	void HandleEzvizMsg(EzvizMessage* msg);
public:
	void PlayVideo(video::CVideoDeviceInfo* device);
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
protected:
	afx_msg LRESULT OnInversioncontrol(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
