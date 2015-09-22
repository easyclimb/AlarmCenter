// VideoPlayerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "VideoPlayerDlg.h"
#include "afxdialogex.h"
#include "tinyxml/tinyxml.h"
#include "VideoManager.h"
#include "VideoUserInfoEzviz.h"
#include "VideoDeviceInfoEzviz.h"

#include "PrivateCloudConnector.h"
#include "InputDeviceVerifyCodeDlg.h"

using namespace video;
using namespace video::ezviz;

static const int TIMER_ID_EZVIZ_MSG = 1;
static const int TIMER_ID_REC_VIDEO = 2;

void __stdcall CVideoPlayerDlg::messageHandler(const char *szSessionId,
											   unsigned int iMsgType,
											   unsigned int iErrorCode,
											   const char *pMessageInfo,
											   void *pUser)
{
	AUTO_LOG_FUNCTION;
	LOGA("(const char *szSessionId, %s\r\n\
unsigned int iMsgType, %d\r\n\
unsigned int iErrorCode, %d\r\n\
const char *pMessageInfo, %p, %s\r\n\
void *pUser, %p)\r\n", 
	szSessionId, iMsgType, iErrorCode, pMessageInfo, pMessageInfo, pUser);

	CVideoPlayerDlg* dlg = reinterpret_cast<CVideoPlayerDlg*>(pUser); assert(pUser);
	EzvizMessage* msg = new EzvizMessage(iMsgType, iErrorCode, szSessionId, pMessageInfo ? pMessageInfo : "");
	dlg->EnqueEzvizMsg(msg);
}


void CVideoPlayerDlg::EnqueEzvizMsg(EzvizMessage* msg)
{ 
	AUTO_LOG_FUNCTION;
	m_lock4EzvizMsgQueue.Lock(); 
	m_ezvizMsgList.push_back(msg); 
	m_lock4EzvizMsgQueue.UnLock();
}


void CVideoPlayerDlg::HandleEzvizMsg(EzvizMessage* msg)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	CString info = L"", title = L"", e = L"";
	switch (msg->iMsgType) {
		case CSdkMgrEzviz::INS_PLAY_EXCEPTION:
			//pInstance->insPlayException(iErrorCode, pMessageInfo);
			title.LoadStringW(IDS_STRING_PLAY_EXCEPTION);
			info.Format(L"ErrorCode = %d", msg->iErrorCode);
			if (msg->iErrorCode == 2012) {
				e.LoadStringW(IDS_STRING_VERIFY_CODE_WRONG);
				info.AppendFormat(L"\r\n%s", e);
			} else if (msg->iErrorCode == 3121) {
				e.LoadStringW(IDS_STRING_DEVICE_OFFLINE);
				info.AppendFormat(L"\r\n%s", e);
			}
			MessageBox(info, title, MB_ICONINFORMATION);
			break;
		case CSdkMgrEzviz::INS_PLAY_RECONNECT:
			break;
		case CSdkMgrEzviz::INS_PLAY_RECONNECT_EXCEPTION:
			//pInstance->insPlayReconnectException(iErrorCode, pMessageInfo);
			break;
		case CSdkMgrEzviz::INS_PLAY_START:
			break;
		case CSdkMgrEzviz::INS_PLAY_STOP:
			break;
		case CSdkMgrEzviz::INS_PLAY_ARCHIVE_END:
			break;
		case CSdkMgrEzviz::INS_RECORD_FILE:
			//pInstance->insRecordFile(pMessageInfo);
			break;
		case CSdkMgrEzviz::INS_RECORD_SEARCH_END:
			break;
		case CSdkMgrEzviz::INS_RECORD_SEARCH_FAILED:
			//pInstance->insRecordSearchFailed(iErrorCode, pMessageInfo);
			break;
		case CSdkMgrEzviz::INS_PTZCTRL_SUCCESS:
			break; 
		case CSdkMgrEzviz::INS_PTZCTRL_FAILED:
			break;
		default:
			info.Format(L"MsgType=%d\r\nErrorCode = %d\r\nErrorMsg=%s", 
						msg->iMsgType, msg->iErrorCode, A2W(msg->messageInfo.c_str()));
			MessageBox(info, L"", MB_ICONINFORMATION);
			break;
	}
}



// CVideoPlayerDlg dialog
CVideoPlayerDlg* g_videoPlayerDlg = NULL;
IMPLEMENT_DYNAMIC(CVideoPlayerDlg, CDialogEx)

CVideoPlayerDlg::CVideoPlayerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CVideoPlayerDlg::IDD, pParent)
	, m_bInitOver(FALSE)
	, m_curPlayingDevice(NULL)
	, m_ezvizMsgList()
	, m_lock4EzvizMsgQueue()
	, m_level(0)
{

}

CVideoPlayerDlg::~CVideoPlayerDlg()
{
}

void CVideoPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PLAYER, m_player);
	DDX_Control(pDX, IDC_BUTTON_STOP, m_btnStop);
	DDX_Control(pDX, IDC_BUTTON_CAPTURE, m_btnCapture);
	DDX_Control(pDX, IDC_BUTTON_UP, m_btnUp);
	DDX_Control(pDX, IDC_BUTTON_DOWN, m_btnDown);
	DDX_Control(pDX, IDC_BUTTON_LEFT, m_btnLeft);
	DDX_Control(pDX, IDC_BUTTON_RIGHT, m_btnRight);
	DDX_Control(pDX, IDC_RADIO_SMOOTH, m_radioSmooth);
	DDX_Control(pDX, IDC_RADIO_BALANCE, m_radioBalance);
	DDX_Control(pDX, IDC_RADIO_HD, m_radioHD);
	DDX_Control(pDX, IDC_STATIC_STATUS, m_status);
}


BEGIN_MESSAGE_MAP(CVideoPlayerDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CVideoPlayerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CVideoPlayerDlg::OnBnClickedCancel)
	ON_WM_MOVE()
	ON_MESSAGE(WM_INVERSIONCONTROL, &CVideoPlayerDlg::OnInversioncontrol)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_RADIO_SMOOTH, &CVideoPlayerDlg::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO_BALANCE, &CVideoPlayerDlg::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO_HD, &CVideoPlayerDlg::OnBnClickedRadio3)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CVideoPlayerDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_CAPTURE, &CVideoPlayerDlg::OnBnClickedButtonCapture)
	ON_BN_CLICKED(IDC_BUTTON_UP, &CVideoPlayerDlg::OnBnClickedButtonUp)
	ON_BN_CLICKED(IDC_BUTTON_DOWN, &CVideoPlayerDlg::OnBnClickedButtonDown)
	ON_BN_CLICKED(IDC_BUTTON_LEFT, &CVideoPlayerDlg::OnBnClickedButtonLeft)
	ON_BN_CLICKED(IDC_BUTTON_RIGHT, &CVideoPlayerDlg::OnBnClickedButtonRight)
END_MESSAGE_MAP()


// CVideoPlayerDlg message handlers


BOOL CVideoPlayerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	video::CVideoManager* videoMgr = video::CVideoManager::GetInstance();
	videoMgr->LoadFromDB();

	//GetWindowRect(m_rcNormal);
	//m_player.GetWindowRect(m_rcNormalPlayer);
	LoadPosition();
	
	SetTimer(TIMER_ID_EZVIZ_MSG, 1000, NULL);

	m_radioSmooth.SetCheck(1);
	EnableOtherCtrls(0);

	m_bInitOver = TRUE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CVideoPlayerDlg::LoadPosition()
{
	AUTO_LOG_FUNCTION;
	using namespace tinyxml;
	USES_CONVERSION;
	CString s; s.Format(L"%s\\config", GetModuleFilePath());
	CreateDirectory(s, NULL);
	s += L"\\video.xml";

	TiXmlDocument doc(W2A(s));
	do {
		if (!doc.LoadFile()) {
			break;
		}

		TiXmlElement* root = doc.RootElement();
		if (!root)
			break;

		TiXmlElement* rc = root->FirstChildElement("rc");
		if (!rc)
			break;

		const char* sl = NULL;
		const char* sr = NULL;
		const char* st = NULL;
		const char* sb = NULL;
		const char* sm = NULL;

		sl = rc->Attribute("l");
		sr = rc->Attribute("r");
		st = rc->Attribute("t");
		sb = rc->Attribute("b");
		sm = rc->Attribute("m");

		int l, r, t, b, m;
		l = r = t = b = m = 0;
		if (sl)
			l = atoi(sl);
		if (sr)
			r = atoi(sr);
		if (st)
			t = atoi(st);
		if (sb)
			b = atoi(sb);
		if (sm)
			m = atoi(sm);

		CRect rect(l, t, r, b);
		if (rect.IsRectNull() || rect.IsRectEmpty()) {
			break;
		}

		MoveWindow(rect);

		//if (m) {
			//ShowWindow(SW_SHOWMAXIMIZED);
			m_player.SetMaximized(m);
			OnInversioncontrol(m, 0);
		//}
	} while (0);
}


void CVideoPlayerDlg::SavePosition()
{
	AUTO_LOG_FUNCTION;
	using namespace tinyxml;
	USES_CONVERSION;
	CString s; s.Format(L"%s\\config", GetModuleFilePath());
	CreateDirectory(s, NULL);
	s += L"\\video.xml";

	CRect rect;
	GetWindowRect(rect);

	TiXmlDocument doc;
	TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "", "");
	doc.LinkEndChild(decl);
	TiXmlElement *root = new TiXmlElement("VideoConfig");
	doc.LinkEndChild(root);

	TiXmlElement* rc = new TiXmlElement("rc"); 
	rc->SetAttribute("l", rect.left);
	rc->SetAttribute("r", rect.right);
	rc->SetAttribute("t", rect.top);
	rc->SetAttribute("b", rect.bottom);
	rc->SetAttribute("m", m_player.GetMaximized());
	root->LinkEndChild(rc);

	doc.SaveFile(W2A(s));
}

void CVideoPlayerDlg::OnBnClickedOk()
{
}


void CVideoPlayerDlg::OnBnClickedCancel()
{
	ShowWindow(SW_HIDE);
	//CDialogEx::OnClose();
}


void CVideoPlayerDlg::OnMove(int x, int y)
{
	CDialogEx::OnMove(x, y);

	if (m_bInitOver) {
		//GetWindowRect(m_rcNormal);
		//m_player.GetWindowRect(m_rcNormalPlayer);
		SavePosition();
	}
}


void CVideoPlayerDlg::ShowOtherCtrls(BOOL bShow)
{
	int sw = bShow ? SW_SHOW : SW_HIDE;
	m_radioSmooth.ShowWindow(sw);
	m_radioBalance.ShowWindow(sw);
	m_radioHD.ShowWindow(sw);
	m_btnStop.ShowWindow(sw);
	m_btnCapture.ShowWindow(sw);
	m_btnUp.ShowWindow(sw);
	m_btnDown.ShowWindow(sw);
	m_btnLeft.ShowWindow(sw);
	m_btnRight.ShowWindow(sw);
}


void CVideoPlayerDlg::EnableOtherCtrls(BOOL bAble)
{
	m_radioSmooth.EnableWindow(bAble);
	m_radioBalance.EnableWindow(bAble);
	m_radioHD.EnableWindow(bAble);
	m_btnStop.EnableWindow(bAble);
	m_btnCapture.EnableWindow(bAble);
	m_btnUp.EnableWindow(bAble);
	m_btnDown.EnableWindow(bAble);
	m_btnLeft.EnableWindow(bAble);
	m_btnRight.EnableWindow(bAble);
}


afx_msg LRESULT CVideoPlayerDlg::OnInversioncontrol(WPARAM wParam, LPARAM /*lParam*/)
{
	AUTO_LOG_FUNCTION;
	if (m_bInitOver) {
		//BOOL bMax = m_player.GetMaximized();
		//bMax = !bMax;
		//m_player.SetMaximized(bMax);
		BOOL bMax = static_cast<BOOL>(wParam);
		if (bMax) {
			ShowOtherCtrls(0);

			//GetWindowRect(m_rcNormal);
			GetWindowPlacement(&m_rcNormal);
			m_player.GetWindowPlacement(&m_rcNormalPlayer);
			HMONITOR hMonitor = MonitorFromWindow(GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
			MONITORINFO mi = { 0 }; mi.cbSize = sizeof(mi);
			GetMonitorInfo(hMonitor, &mi);
			//m_player.SetMonitorRect(mi.rcMonitor);
			m_rcFullScreen.length = sizeof(m_rcFullScreen);
			m_rcFullScreen.flags = 0;
			m_rcFullScreen.showCmd = SW_SHOWNORMAL;
			m_rcFullScreen.rcNormalPosition = mi.rcMonitor;
			SetWindowPlacement(&m_rcFullScreen);
			//MoveWindow(&mi.rcMonitor);
			CRect rc;
			GetClientRect(rc);
			//ClientToScreen(rc);
			m_player.MoveWindow(rc);
		} else {
			ShowOtherCtrls(1);
			//MoveWindow(m_rcNormal);
			//m_player.MoveWindow(m_rcNormalPlayer);
			SetWindowPlacement(&m_rcNormal);
			m_player.SetWindowPlacement(&m_rcNormalPlayer);
		}
		
		SavePosition();
	}
	return 0;
}


void CVideoPlayerDlg::PlayVideo(video::CVideoDeviceInfo* device)
{
	AUTO_LOG_FUNCTION;
	ShowWindow(SW_SHOWNORMAL);
	assert(device);
	//if (device == m_curPlayingDevice) return;
	if (device != m_curPlayingDevice && m_curPlayingDevice) {
		StopPlay();
	}
	m_curPlayingDevice = device;
	CVideoUserInfo* user = device->get_userInfo(); assert(user);
	if (EZVIZ == user->get_productorInfo().get_productor()) {
		PlayVideo(reinterpret_cast<video::ezviz::CVideoDeviceInfoEzviz*>(m_curPlayingDevice), m_level);
	} /*else if (NORMAL == user->get_productorInfo().get_productor())  {

	} */else {
		ASSERT(0); m_curPlayingDevice = NULL;
	}
}


void CVideoPlayerDlg::StopPlay()
{
	AUTO_LOG_FUNCTION;
	if (m_curPlayingDevice) {
		EnableOtherCtrls(0);
		StopPlay(reinterpret_cast<video::ezviz::CVideoDeviceInfoEzviz*>(m_curPlayingDevice));
		m_curPlayingDevice = NULL;
	}
}


void CVideoPlayerDlg::PlayVideo(video::ezviz::CVideoDeviceInfoEzviz* device, int videoLevel)
{
	AUTO_LOG_FUNCTION;
	assert(device);
	do {
		video::ezviz::CVideoUserInfoEzviz* user = reinterpret_cast<video::ezviz::CVideoUserInfoEzviz*>(device->get_userInfo()); assert(user);
		video::ezviz::CSdkMgrEzviz* mgr = video::ezviz::CSdkMgrEzviz::GetInstance();
		CString e;
		if (user->get_user_accToken().size() == 0) {
			if (video::ezviz::CSdkMgrEzviz::RESULT_OK != mgr->VerifyUserAccessToken(user)) {
				e.LoadStringW(IDS_STRING_PRIVATE_CLOUD_CONN_FAIL_OR_USER_NOT_EXSIST);
				MessageBox(e, L"", MB_ICONINFORMATION);
				break;
			}
			user->execute_set_user_token_time(COleDateTime::GetCurrentTime());
		}
		bool bEncrypt = false;
		int ret = mgr->m_dll.UpdateCameraInfo(device->get_cameraId(), user->get_user_accToken(), bEncrypt);
		if (ret != 0) {
			e.LoadStringW(IDS_STRING_UPDATE_CAMERA_INFO_FAILED);
			MessageBox(e, L"", MB_ICONINFORMATION);
			break;
		}

		if (bEncrypt && device->get_secure_code().size() != 6) {
			CInputDeviceVerifyCodeDlg dlg;
			if (dlg.DoModal() != IDOK) {
				break;
			}
			USES_CONVERSION;
			device->set_secure_code(W2A(dlg.m_result));
			device->execute_update_info();
		}
		ret = mgr->m_dll.startRealPlay(mgr->GetSessionId(user->get_user_phone(), messageHandler, this), m_player.m_hWnd,
									   device->get_cameraId(), user->get_user_accToken(), device->get_secure_code(),
									   video::ezviz::CPrivateCloudConnector::GetInstance()->get_appKey(), videoLevel);
		if (ret != 0) {
			LOG(L"startRealPlay failed %d\n", ret);
		}
		LOG(L"PlayVideo ok\n");
		EnableOtherCtrls(1);
		return;
	} while (0);
	LOG(L"PlayVideo failed\n");
}


void CVideoPlayerDlg::StopPlay(video::ezviz::CVideoDeviceInfoEzviz* device)
{
	assert(device);
	video::ezviz::CVideoUserInfoEzviz* user = reinterpret_cast<video::ezviz::CVideoUserInfoEzviz*>(device->get_userInfo()); assert(user);
	video::ezviz::CSdkMgrEzviz* mgr = video::ezviz::CSdkMgrEzviz::GetInstance();
	mgr->m_dll.stopRealPlay(mgr->GetSessionId(user->get_user_phone(), messageHandler, this));
}


void CVideoPlayerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	StopPlay();
	video::CVideoManager::ReleaseObject();
	KillTimer(TIMER_ID_EZVIZ_MSG);
	for (auto& msg : m_ezvizMsgList) {
		SAFEDELETEP(msg);
	}
	m_ezvizMsgList.clear();
}


void CVideoPlayerDlg::OnTimer(UINT_PTR nIDEvent)
{
	//AUTO_LOG_FUNCTION;
	if (TIMER_ID_EZVIZ_MSG == nIDEvent) {
		if (m_lock4EzvizMsgQueue.TryLock()) {
			/*for (auto& msg : m_ezvizMsgList) {
				HandleEzvizMsg(msg);
				SAFEDELETEP(msg);
			}
			m_ezvizMsgList.clear();*/
			if (m_ezvizMsgList.size() > 0) {
				auto msg = m_ezvizMsgList.front();
				m_ezvizMsgList.pop_front();
				HandleEzvizMsg(msg);
				SAFEDELETEP(msg);
			}
			m_lock4EzvizMsgQueue.UnLock();
		}
	} else if (TIMER_ID_REC_VIDEO == nIDEvent) {

	}


	CDialogEx::OnTimer(nIDEvent);
}


void CVideoPlayerDlg::OnBnClickedRadio1()
{
	if (m_curPlayingDevice) {
		m_level = 0;
		PlayVideo(reinterpret_cast<ezviz::CVideoDeviceInfoEzviz*>(m_curPlayingDevice), 0);
	}
}


void CVideoPlayerDlg::OnBnClickedRadio2()
{
	if (m_curPlayingDevice) {
		m_level = 1;
		PlayVideo(reinterpret_cast<ezviz::CVideoDeviceInfoEzviz*>(m_curPlayingDevice), 1);
	}
}


void CVideoPlayerDlg::OnBnClickedRadio3()
{
	if (m_curPlayingDevice) {
		m_level = 2;
		PlayVideo(reinterpret_cast<ezviz::CVideoDeviceInfoEzviz*>(m_curPlayingDevice), 2);
	}
}


void CVideoPlayerDlg::OnBnClickedButtonStop()
{
	StopPlay();
}


void CVideoPlayerDlg::OnBnClickedButtonCapture()
{
	USES_CONVERSION;
	if (m_curPlayingDevice) {
		video::ezviz::CVideoUserInfoEzviz* user = reinterpret_cast<video::ezviz::CVideoUserInfoEzviz*>(m_curPlayingDevice->get_userInfo()); assert(user);
		video::ezviz::CSdkMgrEzviz* mgr = video::ezviz::CSdkMgrEzviz::GetInstance();
		CString path, fm, txt;
		path.Format(L"%s\\video_capture\\%s", GetModuleFilePath(), CTime::GetCurrentTime().Format(L"%Y-%m-%d-%H-%M-%S.jpg"));
		fm.LoadStringW(IDS_STRING_FM_CAPTURE_OK);
		txt.Format(fm, path);
		m_status.SetWindowTextW(txt);
		std::string name = W2A(path);
		mgr->m_dll.capturePicture(mgr->GetSessionId(user->get_user_phone(), messageHandler, this), name);
	}
}


void CVideoPlayerDlg::OnBnClickedButtonUp()
{
	PtzControl(video::ezviz::CSdkMgrEzviz::UP, video::ezviz::CSdkMgrEzviz::START);
	PtzControl(video::ezviz::CSdkMgrEzviz::UP, video::ezviz::CSdkMgrEzviz::STOP);
}


void CVideoPlayerDlg::OnBnClickedButtonDown()
{
	PtzControl(video::ezviz::CSdkMgrEzviz::DOWN, video::ezviz::CSdkMgrEzviz::START);
	PtzControl(video::ezviz::CSdkMgrEzviz::DOWN, video::ezviz::CSdkMgrEzviz::STOP);
}


void CVideoPlayerDlg::OnBnClickedButtonLeft()
{
	PtzControl(video::ezviz::CSdkMgrEzviz::LEFT, video::ezviz::CSdkMgrEzviz::START);
	PtzControl(video::ezviz::CSdkMgrEzviz::LEFT, video::ezviz::CSdkMgrEzviz::STOP);
}


void CVideoPlayerDlg::OnBnClickedButtonRight()
{
	PtzControl(video::ezviz::CSdkMgrEzviz::RIGHT, video::ezviz::CSdkMgrEzviz::START);
	PtzControl(video::ezviz::CSdkMgrEzviz::RIGHT, video::ezviz::CSdkMgrEzviz::STOP);
}


void CVideoPlayerDlg::PtzControl(video::ezviz::CSdkMgrEzviz::PTZCommand command, video::ezviz::CSdkMgrEzviz::PTZAction action)
{
	if (m_curPlayingDevice) {
		video::ezviz::CVideoDeviceInfoEzviz* device = reinterpret_cast<video::ezviz::CVideoDeviceInfoEzviz*>(m_curPlayingDevice);
		video::ezviz::CVideoUserInfoEzviz* user = reinterpret_cast<video::ezviz::CVideoUserInfoEzviz*>(device->get_userInfo()); assert(user);
		video::ezviz::CSdkMgrEzviz* mgr = video::ezviz::CSdkMgrEzviz::GetInstance();
		mgr->m_dll.PTZCtrl(mgr->GetSessionId(user->get_user_phone(), messageHandler, this),
						   user->get_user_accToken(),
						   device->get_cameraId(),
						   command,
						   action,
						   PTZ_SPEED);
	}
}


void CVideoPlayerDlg::PlayVideo(const video::ZoneUuid& zone)
{
	AUTO_LOG_FUNCTION;
	video::BindInfo bi = video::CVideoManager::GetInstance()->GetBindInfo(zone);
	if (bi._device && bi._auto_play_video) {
		video::ezviz::CVideoDeviceInfoEzviz* device = reinterpret_cast<video::ezviz::CVideoDeviceInfoEzviz*>(bi._device);
		PlayVideo(device);
	}
}
