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
#include "HistoryRecord.h"
#include <fstream>
#include "json/json.h"
#include "InputDlg.h"

using namespace video;
using namespace video::ezviz;

static const int TIMER_ID_EZVIZ_MSG = 1;
static const int TIMER_ID_REC_VIDEO = 2;
static const int TIMER_ID_PLAY_VIDEO = 3;

static const int TIMEOUT_4_VIDEO_RECORD = 10; // in minutes

#define HOTKEY_PTZ 12

static const char *const SMSCODE_SECURE_REQ = "{\"method\":\"msg/smsCode/secure\",\"params\":{\"accessToken\":\"%s\"}}";
static const char *const SECUREVALIDATE_REQ = "{\"method\":\"msg/sdk/secureValidate\",\"params\":{\"smsCode\": \"%s\",\"accessToken\": \"%s\"}}";

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


void __stdcall CVideoPlayerDlg::videoDataHandler(CSdkMgrEzviz::DataType /*enType*/,
												 char* const pData,
												 int iLen,
												 void* pUser)
{
	//AUTO_LOG_FUNCTION;
	//LOGA("enType %d, pData %p, iLen %d\n", enType, pData, iLen);
	/*CTestHikvisionDlg * mainWins = (CTestHikvisionDlg *)pUser;
	*/

	DataCallbackParam* param = reinterpret_cast<DataCallbackParam*>(pUser); assert(param);
	COleDateTime now = COleDateTime::GetCurrentTime();
	COleDateTimeSpan span = now - param->_startTime;
	if (span.GetTotalMinutes() >= TIMEOUT_4_VIDEO_RECORD) return;

	std::ofstream file;
	file.open(param->_file_path, std::ios::binary | std::ios::app);
	if (file.is_open()) {
		file.write(pData, iLen);
		file.flush();
		file.close();
	}
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
			} else if (msg->iErrorCode == 3128) { // hd sign error
				bool bVerifyOk = false;
				video::ezviz::CVideoDeviceInfoEzviz* device = NULL;
				m_lock4CurRecordingInfoList.Lock();
				for (auto info : m_curRecordingInfoList) {
					if (info->_param->_session_id == msg->sessionId) {
						video::ezviz::CVideoUserInfoEzviz* user = reinterpret_cast<video::ezviz::CVideoUserInfoEzviz*>(info->_device->get_userInfo());
						video::ezviz::CSdkMgrEzviz* mgr = video::ezviz::CSdkMgrEzviz::GetInstance();
						if (video::ezviz::CSdkMgrEzviz::RESULT_OK != mgr->VerifyUserAccessToken(user, TYPE_HD)) {
							e.LoadStringW(IDS_STRING_PRIVATE_CLOUD_CONN_FAIL_OR_USER_NOT_EXSIST);
							MessageBox(e, L"", MB_ICONINFORMATION);
						} else {
							bVerifyOk = true;
							device = info->_device;
						}
						StopPlay(info);
						m_curRecordingInfoList.remove(info);
						delete info;
						break;
					}
				}
				m_lock4CurRecordingInfoList.UnLock();
				if (bVerifyOk) {
					PlayVideoByDevice(device, m_level);
					return;
				}

			} else if (msg->iErrorCode == 2021) {
				e.LoadStringW(IDS_STRING_VTDU_TIMEOUT);
				info.AppendFormat(L"\r\n%s", e);
			}
			MessageBox(info, title, MB_ICONINFORMATION);
			for (auto info : m_curRecordingInfoList) {
				if (info->_param->_session_id == msg->sessionId) {
					StopPlay(info);
					m_curRecordingInfoList.remove(info);
					delete info;
					break;
				}
			}
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
			//MessageBox(info, L"", MB_ICONINFORMATION);
			LOG(info);
			break;
	}
}


// CVideoPlayerDlg dialog
CVideoPlayerDlg* g_videoPlayerDlg = nullptr;
IMPLEMENT_DYNAMIC(CVideoPlayerDlg, CDialogEx)

CVideoPlayerDlg::CVideoPlayerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CVideoPlayerDlg::IDD, pParent)
	, m_bInitOver(FALSE)
	, m_curPlayingDevice(nullptr)
	, m_curRecordingInfoList()
	, m_lock4CurRecordingInfoList()
	, m_ezvizMsgList()
	, m_lock4EzvizMsgQueue()
	, m_level(0)
	, m_dwPlayerStyle(0)
{

}

CVideoPlayerDlg::~CVideoPlayerDlg()
{}

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
	DDX_Control(pDX, IDC_STATIC_SPEED, m_groupSpeed);
	DDX_Control(pDX, IDC_STATIC_PTZ, m_groupPtz);
	DDX_Control(pDX, IDC_STATIC_CONTROL, m_groupControl);
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
	ON_WM_HOTKEY()
END_MESSAGE_MAP()


// CVideoPlayerDlg message handlers


BOOL CVideoPlayerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	GetWindowText(m_title);
	video::CVideoManager* videoMgr = video::CVideoManager::GetInstance();
	videoMgr->LoadFromDB();

	//GetWindowRect(m_rcNormal);
	//m_player.GetWindowRect(m_rcNormalPlayer);
	

	SetTimer(TIMER_ID_EZVIZ_MSG, 1000, nullptr);
	SetTimer(TIMER_ID_REC_VIDEO, 2000, nullptr);
	SetTimer(TIMER_ID_PLAY_VIDEO, 1000, nullptr);
	

	m_radioSmooth.SetCheck(1);
	EnableOtherCtrls(0);

	m_dwPlayerStyle = m_player.GetStyle();

	RegisterHotKey(GetSafeHwnd(), HOTKEY_PTZ, MOD_ALT, VK_LEFT);
	RegisterHotKey(GetSafeHwnd(), HOTKEY_PTZ, MOD_ALT, VK_RIGHT);
	RegisterHotKey(GetSafeHwnd(), HOTKEY_PTZ, MOD_ALT, VK_UP);
	RegisterHotKey(GetSafeHwnd(), HOTKEY_PTZ, MOD_ALT, VK_DOWN);
	m_bInitOver = TRUE;

	LoadPosition();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CVideoPlayerDlg::LoadPosition()
{
	AUTO_LOG_FUNCTION;
	using namespace tinyxml;
	USES_CONVERSION;
	CString s; s.Format(L"%s\\config", GetModuleFilePath());
	CreateDirectory(s, nullptr);
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

		const char* sl = nullptr;
		const char* sr = nullptr;
		const char* st = nullptr;
		const char* sb = nullptr;
		const char* sm = nullptr;

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

		if (m) {
			CRect rcNormal;
			GetWindowRect(rcNormal);
			rect.right = rect.left + rcNormal.Width();
			rect.bottom = rect.top + rcNormal.Height();
			MoveWindow(rect);
		} else {
			MoveWindow(rect);
			GetWindowPlacement(&m_rcNormal);
			m_player.GetWindowPlacement(&m_rcNormalPlayer);
		}

		

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
	CreateDirectory(s, nullptr);
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
	return;
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

	m_groupSpeed.ShowWindow(sw);
	m_groupPtz.ShowWindow(sw);
	m_groupControl.ShowWindow(sw);
	m_status.ShowWindow(sw);
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
			m_lock4CurRecordingInfoList.Lock();
			for (auto info : m_curRecordingInfoList) {
				info->_ctrl->MoveWindow(rc);
				if (info->_device == m_curPlayingDevice) {
					info->_ctrl->ShowWindow(SW_SHOW);
				} else {
					info->_ctrl->ShowWindow(SW_HIDE);
				}
			}
			m_lock4CurRecordingInfoList.UnLock();
		} else {
			ShowOtherCtrls(1);
			//MoveWindow(m_rcNormal);
			//m_player.MoveWindow(m_rcNormalPlayer);
			SetWindowPlacement(&m_rcNormal);
			m_player.SetWindowPlacement(&m_rcNormalPlayer);
			m_lock4CurRecordingInfoList.Lock();
			for (auto info : m_curRecordingInfoList) {
				info->_ctrl->SetWindowPlacement(&m_rcNormalPlayer);
				if (info->_device == m_curPlayingDevice) {
					info->_ctrl->ShowWindow(SW_SHOW);
				} else {
					info->_ctrl->ShowWindow(SW_HIDE);
				}
			}
			m_lock4CurRecordingInfoList.UnLock();
		}

		SavePosition();
	}
	return 0;
}


void CVideoPlayerDlg::PlayVideoByDevice(video::CVideoDeviceInfo* device, int speed)
{
	AUTO_LOG_FUNCTION;
	ShowWindow(SW_SHOWNORMAL);
	assert(device);
	//if (device == m_curPlayingDevice) return;
	/*if (device == m_curPlayingDevice) {
		if (m_level == speed) {
			return;
		}
		else
			StopPlay();
	}
	m_curPlayingDevice = device;*/
	CVideoUserInfo* user = device->get_userInfo(); assert(user);
	if (EZVIZ == user->get_productorInfo().get_productor()) {
		PlayVideoEzviz(reinterpret_cast<video::ezviz::CVideoDeviceInfoEzviz*>(device), speed);
	} /*else if (NORMAL == user->get_productorInfo().get_productor())  {

	} */else {
		ASSERT(0); m_curPlayingDevice = nullptr;
		SetWindowText(m_title);
	}
}


void CVideoPlayerDlg::StopPlay()
{
	AUTO_LOG_FUNCTION;
	if (m_curPlayingDevice) {
		EnableOtherCtrls(0);
		StopPlay(reinterpret_cast<video::ezviz::CVideoDeviceInfoEzviz*>(m_curPlayingDevice));
		m_curPlayingDevice = nullptr;
		SetWindowText(m_title);
	}
}


void CVideoPlayerDlg::PlayVideoEzviz(video::ezviz::CVideoDeviceInfoEzviz* device, int videoLevel)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	assert(device);
	do {
		if (m_curPlayingDevice == device) {
			if (videoLevel == m_level) {
				EnableOtherCtrls();
				return;
			} else
				StopPlay();
		} else {
			m_lock4CurRecordingInfoList.Lock();
			bool bFound = false;
			for (auto info : m_curRecordingInfoList) {
				if (info->_device == device) {
					bFound = true;
					info->_param->_startTime = COleDateTime::GetCurrentTime();
					info->_ctrl->ShowWindow(SW_SHOW);
					EnableOtherCtrls();
					break;
				} 
			}
			if (bFound) {
				for (auto info : m_curRecordingInfoList) {
					if (info->_device != device) {
						info->_ctrl->ShowWindow(SW_HIDE);
					}
				}
				m_curPlayingDevice = device;
				CString txt;
				txt.Format(L"%s  ----  %s[%d,%s,%s]", m_title, device->get_userInfo()->get_user_name().c_str(),
						   device->get_id(), device->get_device_note().c_str(), A2W(device->get_deviceSerial().c_str()));
				SetWindowText(txt);
				m_lock4CurRecordingInfoList.UnLock();
				return;
			} else {
				if (m_curRecordingInfoList.size() >= 8) {
					for (auto info : m_curRecordingInfoList) {
						if (info->_device != m_curPlayingDevice) {
							StopPlay(info);
							m_curRecordingInfoList.remove(info);
							delete info;
							break;
						}
					}
				}
			}

			m_lock4CurRecordingInfoList.UnLock();
		}
		
		m_curPlayingDevice = device;
		video::ezviz::CVideoUserInfoEzviz* user = reinterpret_cast<video::ezviz::CVideoUserInfoEzviz*>(device->get_userInfo()); assert(user);
		video::ezviz::CSdkMgrEzviz* mgr = video::ezviz::CSdkMgrEzviz::GetInstance();
		CString e;
		if (user->get_user_accToken().size() == 0) {
			if (video::ezviz::CSdkMgrEzviz::RESULT_OK != mgr->VerifyUserAccessToken(user, TYPE_GET)) {
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
		std::string session_id = mgr->GetSessionId(user->get_user_phone(), device->get_cameraId(), messageHandler, this);
		DataCallbackParam *param = new DataCallbackParam(this, session_id, time(nullptr));
		CString filePath = param->FormatFilePath(device->get_cameraId());
		mgr->m_dll.setDataCallBack(session_id, videoDataHandler, param);
		CVideoPlayerCtrl* ctrl = new CVideoPlayerCtrl();
		CRect rc;
		m_player.GetWindowRect(rc);
		ScreenToClient(rc);
		ctrl->Create(nullptr, m_dwPlayerStyle, rc, this, IDC_STATIC_PLAYER);		
		ret = mgr->m_dll.startRealPlay(session_id, ctrl->m_hWnd, device->get_cameraId(), user->get_user_accToken(),
									   device->get_secure_code(), CPrivateCloudConnector::GetInstance()->get_appKey(), videoLevel);

		if (ret == 20005) { // verify code failed
			/*if (video::ezviz::CSdkMgrEzviz::RESULT_OK == mgr->VerifyUserAccessToken(user, TYPE_HD)) {
				ret = mgr->m_dll.startRealPlay(session_id, ctrl->m_hWnd, device->get_cameraId(), user->get_user_accToken(),
											   device->get_secure_code(), CPrivateCloudConnector::GetInstance()->get_appKey(), videoLevel);
			}*/
			bool ok = false;
			do {
				char reqStr[1024] = { 0 };
				sprintf_s(reqStr, SMSCODE_SECURE_REQ, user->get_user_accToken().c_str());
				char* pOutStr = NULL;
				int iLen = 0;
				ret = mgr->m_dll.RequestPassThrough(reqStr, &pOutStr, &iLen);
				if (ret != 0) {
					LOG(L"调用透传接口失败， 返回错误码为：%d", ret);
					break;
				}
				pOutStr[iLen] = 0;
				std::string json = pOutStr;
				mgr->m_dll.freeData(pOutStr);

				
				Json::Reader reader;
				Json::Value	value;
				if (!reader.parse(json.c_str(), value)) {
					LOG(L"获取短信验证码解析Json串失败!");
					break;
				}
				Json::Value result = value["result"];
				int iResult = 0;
				if (result["code"].isString()) {
					iResult = atoi(result["code"].asString().c_str());
				} else if (result["code"].isInt()) {
					iResult = result["code"].asInt();
				}
				if (200 == iResult) {
					ok = true;
				} else {
					break;
				}
			} while (0);

			do {
				if (!ok) break;
				ok = false;
				CInputDlg dlg;
				if (IDOK != dlg.DoModal())
					break;
				USES_CONVERSION;
				std::string verify_code = W2A(dlg.m_edit);

				char reqStr[1024] = { 0 };
				sprintf_s(reqStr, SECUREVALIDATE_REQ, verify_code.c_str(), user->get_user_accToken().c_str());
				char* pOutStr = NULL;
				int iLen = 0;
				ret = mgr->m_dll.RequestPassThrough(reqStr, &pOutStr, &iLen);
				if (ret != 0) {
					LOG(L"调用透传接口失败， 返回错误码为：%d", ret);
					break;
				}
				pOutStr[iLen] = 0;
				std::string json = pOutStr;
				mgr->m_dll.freeData(pOutStr);


				Json::Reader reader;
				Json::Value	value;
				if (!reader.parse(json.c_str(), value)) {
					LOG(L"验证短信验证码解析Json串失败!");
					break;
				}
				Json::Value result = value["result"];
				int iResult = 0;
				if (result["code"].isString()) {
					iResult = atoi(result["code"].asString().c_str());
				} else if (result["code"].isInt()) {
					iResult = result["code"].asInt();
				}
				if (200 == iResult) {
					ok = true;
				} else {
					break;
				}
			} while (0);
			if (ok)
				ret = mgr->m_dll.startRealPlay(session_id, ctrl->m_hWnd, device->get_cameraId(), user->get_user_accToken(),
					device->get_secure_code(), CPrivateCloudConnector::GetInstance()->get_appKey(), videoLevel);
			else
				ret = -1;
		}

		if (ret != 0) {
			LOG(L"startRealPlay failed %d\n", ret);
			m_curPlayingDevice = nullptr;
			//SetWindowText(m_title);
			SAFEDELETEP(param);
			SAFEDELETEDLG(ctrl);
		} else {
			LOG(L"PlayVideo ok\n");

			EnableOtherCtrls(1);
			m_lock4CurRecordingInfoList.Lock();
			ctrl->ShowWindow(SW_SHOW);
			for (auto info : m_curRecordingInfoList) {
				info->_ctrl->ShowWindow(SW_HIDE);
			}
			core::CHistoryRecord* hr = core::CHistoryRecord::GetInstance();
			CString record, start; start.LoadStringW(IDS_STRING_VIDEO_START);
			record.Format(L"%s-%s-\"%s\"", start, A2W(device->get_cameraId().c_str()), filePath);
			video::ZoneUuid zoneUuid = device->GetActiveZoneUuid();
			hr->InsertRecord(zoneUuid._ademco_id, zoneUuid._zone_value,
							 record, time(nullptr), core::RECORD_LEVEL_VIDEO);
			RecordVideoInfo* info = new RecordVideoInfo(param, zoneUuid, device, ctrl);
			m_curRecordingInfoList.push_back(info);
			record.Format(L"%s  ----  %s[%d,%s,%s]", m_title, device->get_userInfo()->get_user_name().c_str(),
						  device->get_id(), device->get_device_note().c_str(), A2W(device->get_deviceSerial().c_str()));
			SetWindowText(record);
			m_lock4CurRecordingInfoList.UnLock();
			
		}
		UpdateWindow();
		return;
	} while (0);
	LOG(L"PlayVideo failed\n");
	m_curPlayingDevice = nullptr;
	//SetWindowText(m_title);
	UpdateWindow();
}


void CVideoPlayerDlg::StopPlay(video::ezviz::CVideoDeviceInfoEzviz* device)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	assert(device);
	m_lock4CurRecordingInfoList.Lock();
	video::ezviz::CVideoUserInfoEzviz* user = reinterpret_cast<video::ezviz::CVideoUserInfoEzviz*>(device->get_userInfo()); assert(user);
	video::ezviz::CSdkMgrEzviz* mgr = video::ezviz::CSdkMgrEzviz::GetInstance();
	std::string session_id = mgr->GetSessionId(user->get_user_phone(), device->get_cameraId(), messageHandler, this);
	mgr->m_dll.stopRealPlay(session_id);
	core::CHistoryRecord* hr = core::CHistoryRecord::GetInstance();
	CString record, stop; stop.LoadStringW(IDS_STRING_VIDEO_STOP);
	for (const auto info : m_curRecordingInfoList) {
		if (info->_param->_session_id == session_id) {
			m_curRecordingInfoList.remove(info);
			record.Format(L"%s-%s-\"%s\"", stop, A2W(device->get_cameraId().c_str()), A2W(info->_param->_file_path.c_str()));
			video::ZoneUuid zoneUuid = device->GetActiveZoneUuid();
			hr->InsertRecord(zoneUuid._ademco_id, zoneUuid._zone_value,
							 record, time(nullptr), core::RECORD_LEVEL_VIDEO);
			delete info->_param;
			delete info;
			break;
		}
	}
	m_lock4CurRecordingInfoList.UnLock();
}


void CVideoPlayerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	UnregisterHotKey(GetSafeHwnd(), HOTKEY_PTZ);
	StopPlay();
	video::CVideoManager::ReleaseObject();

	KillTimer(TIMER_ID_EZVIZ_MSG);
	KillTimer(TIMER_ID_REC_VIDEO);
	KillTimer(TIMER_ID_PLAY_VIDEO);

	for (auto msg : m_ezvizMsgList) {
		SAFEDELETEP(msg);
	}
	m_ezvizMsgList.clear();

	for (auto info : m_curRecordingInfoList) {
		//StopPlay(info);
		delete info->_param;
		delete info;
	}
	m_curRecordingInfoList.clear();
	m_wait2playDevList.clear();
}


void CVideoPlayerDlg::OnTimer(UINT_PTR nIDEvent)
{
	//AUTO_LOG_FUNCTION;
	if (TIMER_ID_EZVIZ_MSG == nIDEvent) {
		if (m_lock4EzvizMsgQueue.TryLock()) {
			if (m_ezvizMsgList.size() > 0) {
				auto msg = m_ezvizMsgList.front();
				m_ezvizMsgList.pop_front();
				HandleEzvizMsg(msg);
				SAFEDELETEP(msg);
			}
			m_lock4EzvizMsgQueue.UnLock();
		}
	} else if (TIMER_ID_REC_VIDEO == nIDEvent) {
		if (m_lock4CurRecordingInfoList.TryLock()) {
			COleDateTime now = COleDateTime::GetCurrentTime();
			for (const auto info : m_curRecordingInfoList) {
				if (info->_device != m_curPlayingDevice) {
					COleDateTimeSpan span = now - info->_param->_startTime;
					if (span.GetTotalMinutes() >= TIMEOUT_4_VIDEO_RECORD) {
						StopPlay(info);
						m_curRecordingInfoList.remove(info);
						delete info->_param;
						delete info;
						break;
					}
				}
			}
			m_lock4CurRecordingInfoList.UnLock();
		}
	} else if (TIMER_ID_PLAY_VIDEO == nIDEvent) {
		if (m_lock4Wait2PlayDevList.TryLock()) {
			if (!m_wait2playDevList.empty()) {
				auto dev = m_wait2playDevList.front();
				m_wait2playDevList.pop_front();
				PlayVideoByDevice(dev, m_level);
			}
			m_lock4Wait2PlayDevList.UnLock();
		}
	}


	CDialogEx::OnTimer(nIDEvent);
}


void CVideoPlayerDlg::StopPlay(RecordVideoInfo* info)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	if (m_curPlayingDevice == info->_device) {
		m_curPlayingDevice = nullptr;
		EnableOtherCtrls(0);
	}
	video::ezviz::CSdkMgrEzviz* mgr = video::ezviz::CSdkMgrEzviz::GetInstance();
	mgr->m_dll.stopRealPlay(info->_param->_session_id);
	core::CHistoryRecord* hr = core::CHistoryRecord::GetInstance();
	CString record, stop; stop.LoadStringW(IDS_STRING_VIDEO_STOP);
	record.Format(L"%s-%s-\"%s\"", stop, A2W(info->_device->get_cameraId().c_str()), A2W(info->_param->_file_path.c_str()));
	hr->InsertRecord(info->_zone._ademco_id, info->_zone._zone_value,
					 record, time(nullptr), core::RECORD_LEVEL_VIDEO);
}


void CVideoPlayerDlg::OnBnClickedRadio1()
{
	if (m_curPlayingDevice) {
		//if (m_level != 0) {
		//	m_level = 0;
			PlayVideoByDevice(m_curPlayingDevice, 0);
		//}
	}
}


void CVideoPlayerDlg::OnBnClickedRadio2()
{
	if (m_curPlayingDevice) {
		//if (m_level != 1) {
		//	m_level = 1;
			PlayVideoByDevice(m_curPlayingDevice, 1);
		//}
	}
}


void CVideoPlayerDlg::OnBnClickedRadio3()
{
	if (m_curPlayingDevice) {
		//if (m_level != 2) {
		//	m_level = 2;
			PlayVideoByDevice(m_curPlayingDevice, 2);
		//}
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
		CVideoDeviceInfoEzviz* device = reinterpret_cast<CVideoDeviceInfoEzviz*>(m_curPlayingDevice);
		video::ezviz::CVideoUserInfoEzviz* user = reinterpret_cast<video::ezviz::CVideoUserInfoEzviz*>(m_curPlayingDevice->get_userInfo()); assert(user);
		video::ezviz::CSdkMgrEzviz* mgr = video::ezviz::CSdkMgrEzviz::GetInstance();
		CString path, fm, txt;
		path.Format(L"%s\\video_capture\\%s", GetModuleFilePath(), CTime::GetCurrentTime().Format(L"%Y-%m-%d-%H-%M-%S.jpg"));
		fm.LoadStringW(IDS_STRING_FM_CAPTURE_OK);
		txt.Format(fm, path);
		m_status.SetWindowTextW(txt);
		std::string name = W2A(path);
		mgr->m_dll.capturePicture(mgr->GetSessionId(user->get_user_phone(), device->get_cameraId(), messageHandler, this), name);
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
		mgr->m_dll.PTZCtrl(mgr->GetSessionId(user->get_user_phone(), device->get_cameraId(), messageHandler, this),
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
		//PlayVideoByDevice(device, m_level);
		m_lock4Wait2PlayDevList.Lock();
		device->SetActiveZoneUuid(zone);
		m_wait2playDevList.push_back(device);
		m_lock4Wait2PlayDevList.UnLock();
	}
}


void CVideoPlayerDlg::OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2)
{
	if (nHotKeyId == HOTKEY_PTZ) {
		//if (nKey1 == VK_CONTROL) {
			switch (nKey2) {
				case VK_UP:
					OnBnClickedButtonUp();
					break;
				case VK_DOWN:
					OnBnClickedButtonDown();
					break;
				case VK_LEFT:
					OnBnClickedButtonLeft();
					break;
				case VK_RIGHT:
					OnBnClickedButtonRight();
					break;
				default:
					break;
			}
		//}
	}

	CDialogEx::OnHotKey(nHotKeyId, nKey1, nKey2);
}
