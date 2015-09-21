#pragma once
#include "video.h"
#include "VideoPlayerCtrl.h"
#include <queue>
#include "afxwin.h"

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
	WINDOWPLACEMENT m_rcNormal;
	WINDOWPLACEMENT m_rcFullScreen;
	WINDOWPLACEMENT m_rcNormalPlayer;
	video::CVideoDeviceInfo* m_curPlayingDevice;
	CVideoPlayerCtrl m_player;
	CEzvizMsgQueue m_ezvizMsgQueue;
	CLock m_lock4EzvizMsgQueue;
protected:
	void LoadPosition();
	void SavePosition();
	void ShowOtherCtrls(BOOL bShow = TRUE);
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
};
