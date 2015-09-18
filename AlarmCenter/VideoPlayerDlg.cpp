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
#include "SdkMgrEzviz.h"
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
		 const char *pMessageInfo, %s\r\n\
		 void *pUser)\r\n", szSessionId, iMsgType, iErrorCode, pMessageInfo);

	CVideoPlayerDlg* dlg = reinterpret_cast<CVideoPlayerDlg*>(pUser); assert(pUser);
	dlg->EnqueEzvizMsg(new EzvizMessage(iMsgType, iErrorCode, szSessionId, pMessageInfo));

	
}


CString CVideoPlayerDlg::GetEzvzErrorMessage(int errCode)
{
	return L"";
}


// CVideoPlayerDlg dialog
CVideoPlayerDlg* g_videoPlayerDlg = NULL;
IMPLEMENT_DYNAMIC(CVideoPlayerDlg, CDialogEx)

CVideoPlayerDlg::CVideoPlayerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CVideoPlayerDlg::IDD, pParent)
	, m_bInitOver(FALSE)
	, m_curPlayingDevice(NULL)
	, m_ezvizMsgQueue()
{

}

CVideoPlayerDlg::~CVideoPlayerDlg()
{
}

void CVideoPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PLAYER, m_player);
}


BEGIN_MESSAGE_MAP(CVideoPlayerDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CVideoPlayerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CVideoPlayerDlg::OnBnClickedCancel)
	ON_WM_MOVE()
	ON_MESSAGE(WM_INVERSIONCONTROL, &CVideoPlayerDlg::OnInversioncontrol)
	ON_WM_DESTROY()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CVideoPlayerDlg message handlers


BOOL CVideoPlayerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	video::CVideoManager* videoMgr = video::CVideoManager::GetInstance();
	videoMgr->LoadFromDB();

	GetWindowRect(m_rcNormal);
	m_player.GetWindowRect(m_rcNormalPlayer);
	LoadPosition();
	
	m_bInitOver = TRUE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CVideoPlayerDlg::LoadPosition()
{
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
	TiXmlElement *root = new TiXmlElement("VideoConfig"); // 不能有空白符
	doc.LinkEndChild(root);

	TiXmlElement* rc = new TiXmlElement("rc"); // 不能有空白符
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
		GetWindowRect(m_rcNormal);
		m_player.GetWindowRect(m_rcNormalPlayer);
		SavePosition();
	}
}


afx_msg LRESULT CVideoPlayerDlg::OnInversioncontrol(WPARAM wParam, LPARAM /*lParam*/)
{
	if (m_bInitOver) {
		//BOOL bMax = m_player.GetMaximized();
		//bMax = !bMax;
		//m_player.SetMaximized(bMax);
		BOOL bMax = static_cast<BOOL>(wParam);
		if (bMax) {
			HMONITOR hMonitor = MonitorFromWindow(GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
			MONITORINFO mi = { 0 }; mi.cbSize = sizeof(mi);
			GetMonitorInfo(hMonitor, &mi);
			//m_player.SetMonitorRect(mi.rcMonitor);
			
			//MoveWindow(&mi.rcMonitor);
			CRect rc;
			GetClientRect(rc);
			//ClientToScreen(rc);
			//m_player.MoveWindow(rc);
		} else {
			//MoveWindow(m_rcNormal);
			//m_player.MoveWindow(m_rcNormalPlayer);
		}
		
		SavePosition();
	}
	return 0;
}


void CVideoPlayerDlg::PlayVideo(video::CVideoDeviceInfo* device)
{
	ShowWindow(SW_SHOWNORMAL);
	assert(device);
	if (device == m_curPlayingDevice) return;
	if (m_curPlayingDevice) {
		StopPlay();
	}
	m_curPlayingDevice = device;
	CVideoUserInfo* user = device->get_userInfo(); assert(user);
	if (EZVIZ == user->get_productorInfo().get_productor()) {
		PlayVideo(reinterpret_cast<video::ezviz::CVideoDeviceInfoEzviz*>(m_curPlayingDevice), 0);
	} /*else if (NORMAL == user->get_productorInfo().get_productor())  {

	} */else {
		ASSERT(0); m_curPlayingDevice = NULL;
	}
}


void CVideoPlayerDlg::StopPlay()
{
	if (m_curPlayingDevice) {
		StopPlay(reinterpret_cast<video::ezviz::CVideoDeviceInfoEzviz*>(m_curPlayingDevice));
		m_curPlayingDevice = NULL;
	}
}


void CVideoPlayerDlg::PlayVideo(video::ezviz::CVideoDeviceInfoEzviz* device, int videoLevel)
{
	assert(device);
	do {
		video::ezviz::CVideoUserInfoEzviz* user = reinterpret_cast<video::ezviz::CVideoUserInfoEzviz*>(device->get_userInfo()); assert(user);
		video::ezviz::CSdkMgrEzviz* mgr = video::ezviz::CSdkMgrEzviz::GetInstance();
		CString e;
		if (user->get_user_accToken().size() == 0) {
			if (video::ezviz::CSdkMgrEzviz::RESULT_OK != mgr->VerifyUserAccessToken(user)) {
				e.LoadStringW(IDS_STRING_PRIVATE_CLOUD_CONN_FAIL_OR_USER_NOT_EXSIST);
				MessageBox(L"", e, MB_ICONINFORMATION);
				break;
			}
		}
		bool bEncrypt = false;
		int ret = mgr->m_dll.UpdateCameraInfo(device->get_cameraId(), user->get_user_accToken(), bEncrypt);
		if (ret != 0) {
			e.LoadStringW(IDS_STRING_UPDATE_CAMERA_INFO_FAILED);
			MessageBox(L"", e, MB_ICONINFORMATION);
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

		}

	} while (0);
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

	video::CVideoManager::ReleaseObject();
}


void CVideoPlayerDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (TIMER_ID_EZVIZ_MSG == nIDEvent) {
		if (m_lock4EzvizMsgQueue.TryLock()) {
			while (m_ezvizMsgQueue.size() > 0) {
				auto msg = m_ezvizMsgQueue.front();
				m_ezvizMsgQueue.pop();
				HandleEzvizMsg(msg);
				SAFEDELETEP(msg);
			}
			m_lock4EzvizMsgQueue.UnLock();
		}
	} else if (TIMER_ID_REC_VIDEO == nIDEvent) {

	}


	CDialogEx::OnTimer(nIDEvent);
}


void CVideoPlayerDlg::HandleEzvizMsg(EzvizMessage* msg)
{
	switch (msg->iMsgType) {
		case CSdkMgrEzviz::INS_PLAY_EXCEPTION: // 播放异常
			//pInstance->insPlayException(iErrorCode, pMessageInfo);
			break;
		case CSdkMgrEzviz::INS_PLAY_RECONNECT:
			break;
		case CSdkMgrEzviz::INS_PLAY_RECONNECT_EXCEPTION: // 重连异常
			//pInstance->insPlayReconnectException(iErrorCode, pMessageInfo);
			break;
		case CSdkMgrEzviz::INS_PLAY_START:
			break;
		case CSdkMgrEzviz::INS_PLAY_STOP:
			break;
		case CSdkMgrEzviz::INS_PLAY_ARCHIVE_END:
			break;
		case CSdkMgrEzviz::INS_RECORD_FILE: // 查询回放成功，返回回放列表
			//pInstance->insRecordFile(pMessageInfo);
			break;
		case CSdkMgrEzviz::INS_RECORD_SEARCH_END:
			break;
		case CSdkMgrEzviz::INS_RECORD_SEARCH_FAILED: // 查询回放失败
			//pInstance->insRecordSearchFailed(iErrorCode, pMessageInfo);
			break;
	}
}
