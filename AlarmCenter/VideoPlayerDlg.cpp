﻿// VideoPlayerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "VideoPlayerDlg.h"
#include "afxdialogex.h"
#include "VideoManager.h"
#include "VideoUserInfoEzviz.h"
#include "VideoDeviceInfoEzviz.h"
#include "PrivateCloudConnector.h"
#include "InputDeviceVerifyCodeDlg.h"
#include "HistoryRecord.h"

#include "json/json.h"
#include "InputDlg.h"
#include "ConfigHelper.h"
#include "UserInfo.h"
#include "ezviz_inc/INS_ErrorCode.h"
#include "ezviz_inc/OpenNetStreamError.h"

using namespace video;
using namespace video::ezviz;


namespace detail {
	const int TIMER_ID_EZVIZ_MSG = 1;
	const int TIMER_ID_REC_VIDEO = 2;
	const int TIMER_ID_PLAY_VIDEO = 3;

	//const int TIMEOUT_4_VIDEO_RECORD = 10; // in minutes

	const int HOTKEY_PTZ = 12;

	const char *const SMSCODE_SECURE_REQ = "{\"method\":\"msg/smsCode/secure\",\"params\":{\"accessToken\":\"%s\"}}";
	const char *const SECUREVALIDATE_REQ = "{\"method\":\"msg/sdk/secureValidate\",\"params\":{\"smsCode\": \"%s\",\"accessToken\": \"%s\"}}";

	auto split_rect = [](CRect rc, int n) {
		std::vector<CRect> v;
		for (int i = 0; i < n; i++) {
			v.push_back(rc);
		}

		double l = sqrt(n);
		int line = int(l);
		if (l - line != 0) {
			return v;
		}

		int col_step = (int)(rc.Width() / line);
		int row_step = (int)(rc.Height() / line);

		for (int i = 0; i < n; i++) {
			v[i].left = rc.left + (i % line) * col_step;
			v[i].right = v[i].left + col_step;
			v[i].top = rc.top + (i / line) * row_step;
			v[i].bottom = v[i].top + row_step;
			//v[i].DeflateRect(5, 5, 5, 5);
		}
		
		/*for (int col = 0; col < line; col++) {
			for (int row = 0; row < line; row++) {
				int ndx = col + row;
				v[ndx].left = rc.left + col_step * col;
				v[ndx].right = rc.left + col_step * (col + 1);
				v[ndx].top = rc.top + row_step * row;
				v[ndx].bottom = rc.top + row_step * (row + 1);
				v[ndx].DeflateRect(5, 5, 5, 5);
			}
		}*/

		return v;
	};


	CVideoPlayerDlg* g_player = nullptr;
};
using namespace detail;

void __stdcall CVideoPlayerDlg::messageHandler(const char *szSessionId,
											   unsigned int iMsgType,
											   unsigned int iErrorCode,
											   const char *pMessageInfo,
											   void *pUser)
{
	AUTO_LOG_FUNCTION;
	JLOGA("(const char *szSessionId, %s\r\n\
unsigned int iMsgType, %d\r\n\
unsigned int iErrorCode, %d\r\n\
const char *pMessageInfo, %p, %s\r\n\
void *pUser, %p)\r\n",
szSessionId, iMsgType, iErrorCode, pMessageInfo, pMessageInfo, pUser);

	CVideoPlayerDlg* dlg = reinterpret_cast<CVideoPlayerDlg*>(pUser); assert(pUser);
	EzvizMessagePtr msg = std::make_shared<EzvizMessage>(iMsgType, iErrorCode, szSessionId, pMessageInfo ? pMessageInfo : "");
	dlg->EnqueEzvizMsg(msg);
}


void __stdcall CVideoPlayerDlg::videoDataHandler(CSdkMgrEzviz::DataType enType,
												 char* const pData,
												 int iLen,
												 void* pUser)
{
	//AUTO_LOG_FUNCTION;
	try {
		if (pUser == nullptr) {
			assert(0);
			JLOG(L"pUser == nullptr");
			return;
		}

		DataCallbackParam* param = reinterpret_cast<DataCallbackParam*>(pUser); assert(param);
		if (!g_player/* || !g_player->is_valid_data_record_param(param)*/) {
			assert(0); 
			JLOG(L"g_player == nullptr");
			return;
		}

		//COleDateTime now = COleDateTime::GetCurrentTime();
		DWORD now = GetTickCount();
		//COleDateTimeSpan span = now - param->_startTime;
		DWORD span = now - param->_start_time;
		if (/*span.GetTotalMinutes()*/ span / 1000 / 60 >= (DWORD)util::CConfigHelper::GetInstance()->get_back_end_record_minutes()) {
			assert(0); 
			JLOG(L"span.GetTotalMinutes() %d", /*span.GetTotalMinutes()*/ 0);
			return;
		}
		std::ofstream file;
		file.open(param->_file_path, std::ios::binary | std::ios::app);
		if (file.is_open()) {
			file.write(pData, iLen);
			file.flush();
			file.close();
		} else {
			assert(0);
			JLOG(L"file.open failed");
			return;
		}
	} catch (...) {
		assert(0);
		JLOGA("Error! CVideoPlayerDlg::videoDataHandler(), enType %d, pData %p, iLen %d\n", enType, pData, iLen);
	}
}



void CVideoPlayerDlg::OnCurUserChangedResult(const core::user_info_ptr& user)
{
	assert(user);
	if (user->get_user_priority() == core::UP_OPERATOR) {
		m_btn_save.EnableWindow(0);
	} else {
		m_btn_save.EnableWindow(1);
	}
}


void CVideoPlayerDlg::EnqueEzvizMsg(EzvizMessagePtr msg)
{
	AUTO_LOG_FUNCTION;
	std::lock_guard<std::mutex> lock(m_lock4EzvizMsgQueue);
	m_ezvizMsgList.push_back(msg);
}


void CVideoPlayerDlg::HandleEzvizMsg(EzvizMessagePtr msg)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;

	CString sInfo = L"", sTitle = L"", e = L"";
	RecordVideoInfoPtr cur_info = nullptr;
	{
		std::lock_guard<std::recursive_mutex> lock(m_lock4CurRecordingInfoList);
		for (auto info : m_curRecordingInfoList) {
			if (info->_param->_session_id == msg->sessionId) {
				cur_info = info;
				break;
			}
		}
	}

	switch (msg->iMsgType) {
	case CSdkMgrEzviz::INS_PLAY_EXCEPTION:
	{
		sTitle = GetStringFromAppResource(IDS_STRING_PLAY_EXCEPTION);
		sInfo.Format(L"ErrorCode = %d", msg->iErrorCode);

		switch (msg->iErrorCode) {
		case INS_ERROR_V17_PERMANENTKEY_EXCEPTION:
			e = GetStringFromAppResource(IDS_STRING_VERIFY_CODE_WRONG);
			sInfo.AppendFormat(L"\r\n%s", e);
			break;

		case  INS_ERROR_CASLIB_PLATFORM_CLIENT_REQUEST_NO_PU_FOUNDED:
			e = GetStringFromAppResource(IDS_STRING_DEVICE_OFFLINE);
			sInfo.AppendFormat(L"\r\n%s", e);
			break;

		case INS_ERROR_CASLIB_PLATFORM_CLIENT_NO_SIGN_RELEATED:
		{ 
			// need secure validate
			//bool bVerifyOk = false;
			////int level = 0;
			//video::ezviz::CVideoDeviceInfoEzvizPtr device = nullptr;
			//{
			//	std::lock_guard<std::recursive_mutex> lock(m_lock4CurRecordingInfoList);
			//	for (auto info : m_curRecordingInfoList) {
			//		if (info->_param->_session_id == msg->sessionId) {
			//			video::ezviz::CVideoUserInfoEzvizPtr user = std::dynamic_pointer_cast<video::ezviz::CVideoUserInfoEzviz>(info->_device->get_userInfo());
			//			video::ezviz::CSdkMgrEzviz* mgr = video::ezviz::CSdkMgrEzviz::GetInstance();
			//			/*if (video::ezviz::CSdkMgrEzviz::RESULT_OK != mgr->VerifyUserAccessToken(user, TYPE_HD)) {
			//				e = GetStringFromAppResource(IDS_STRING_PRIVATE_CLOUD_CONN_FAIL_OR_USER_NOT_EXSIST);
			//				MessageBox(e, L"", MB_ICONINFORMATION);
			//			} else {
			//				bVerifyOk = true;
			//				device = info->_device;
			//			}*/

			//			
			//			std::string req_str = "{\"method\":\"msg/smsCode/secure\",\"params\":{\"accessToken\":\"";
			//			req_str += user->get_user_accToken();
			//			req_str += "\",\"phone\":";
			//			req_str += user->get_user_phone();
			//			req_str += "\"}}";
			//			JLOGA(req_str.c_str());
			//			char* buf = nullptr;
			//			int length = 0;
			//			int ret = mgr->m_dll.RequestPassThrough(req_str, &buf, &length);

			//			do {
			//				if (ret != 0) {
			//					JLOG(L"RequestPassThrough %d", ret);
			//					break;
			//				}

			//				buf[length] = 0;
			//				std::string json = buf;
			//				mgr->m_dll.freeData(buf);

			//				Json::Reader reader;
			//				Json::Value	value;
			//				if (!reader.parse(json.c_str(), value)) {
			//					JLOG(L"获取短信验证码解析Json串失败!");
			//					break;
			//				}
			//				Json::Value result = value["result"];
			//				int iResult = 0;
			//				if (result["code"].isString()) {
			//					iResult = atoi(result["code"].asString().c_str());
			//				} else if (result["code"].isInt()) {
			//					iResult = result["code"].asInt();
			//				}
			//				JLOG(L"获取短信验证码解析Json串 result %d", iResult);
			//				if (200 != iResult) {
			//					break;
			//				}

			//				CInputPasswdDlg dlg;
			//				dlg.m_title = GetStringFromAppResource(IDS_STRING_INPUT_PHONE_VERIFY_CODE);
			//				if (IDOK != dlg.DoModal()) { JLOG(L"User didnot input sms code."); break; }
			//				std::string verify_code = W2A(dlg.m_edit);
			//				JLOGA("verify_code=%s, userId=%s", verify_code.c_str(), user->get_user_phone().c_str());

			//				req_str = "{\"method\":\"msg/sdk/secureValidate\",\"params\":{\"smsCode\": \";";
			//				req_str += verify_code;
			//				req_str += "\",\"accessToken\": \"";
			//				req_str += user->get_user_accToken();
			//				req_str += "\"}}";
			//				JLOGA(req_str.c_str());
			//				ret = mgr->m_dll.RequestPassThrough(req_str, &buf, &length);

			//				if (ret != 0) {
			//					JLOG(L"RequestPassThrough %d", ret);
			//					break;
			//				}

			//				buf[length] = 0;
			//				json = buf;
			//				mgr->m_dll.freeData(buf);

			//				if (!reader.parse(json.c_str(), value)) {
			//					JLOG(L"获取短信验证码解析Json串失败!");
			//					break;
			//				}
			//				result = value["result"];
			//				iResult = 0;
			//				if (result["code"].isString()) {
			//					iResult = atoi(result["code"].asString().c_str());
			//				} else if (result["code"].isInt()) {
			//					iResult = result["code"].asInt();
			//				}
			//				JLOG(L"获取短信验证码解析Json串 result %d", iResult);
			//				if (200 != iResult) {
			//					break;
			//				}
			//			} while (false);

			//			StopPlay(info);
			//			m_curRecordingInfoList.remove(info);
			//			break;
			//		}
			//	}
			//}
			//if (bVerifyOk) {
			//	PlayVideoByDevice(device, util::CConfigHelper::GetInstance()->get_default_video_level());
			//}

			JLOG(sInfo);
		}
		break;

		case INS_ERROR_OPERATIONCODE_FAILED:
			e = GetStringFromAppResource(IDS_STRING_OPERATIONCODE_FAILED);
			sInfo.AppendFormat(L"\r\n%s", e);
			break;

		case INS_ERROR_V17_VTDU_TIMEOUT:
		case INS_ERROR_V17_VTDU_STOP:
			e = GetStringFromAppResource(IDS_STRING_VTDU_TIMEOUT);
			sInfo.AppendFormat(L"\r\n%s", e);
			break;

		case NS_ERROR_PRIVATE_VTDU_DISCONNECTED_LINK:
			e = GetStringFromAppResource(IDS_STRING_VTDU_DISCONNECTED_LINK);
			sInfo.AppendFormat(L"\r\n%s", e);
			break;

		case NS_ERROR_PRIVATE_VTDU_STATUS_404:
			e = GetStringFromAppResource(IDS_STRING_VTDU_STATUS_404);
			sInfo.AppendFormat(L"\r\n%s", e);
			break;

		case NS_ERROR_PRIVATE_VTDU_STATUS_405:
			e = GetStringFromAppResource(IDS_STRING_VTDU_STATUS_405);
			sInfo.AppendFormat(L"\r\n%s", e);
			break;

		case NS_ERROR_PRIVATE_VTDU_STATUS_406:
			e = GetStringFromAppResource(IDS_STRING_VTDU_STATUS_406);
			sInfo.AppendFormat(L"\r\n%s", e);
			break;

		case NS_ERROR_PRIVATE_VTDU_STATUS_452:
			e = GetStringFromAppResource(IDS_STRING_VTDU_STATUS_452);
			sInfo.AppendFormat(L"\r\n%s", e);
			break;

		case NS_ERROR_PRIVATE_VTDU_STATUS_454:
			e = GetStringFromAppResource(IDS_STRING_VTDU_STATUS_454);
			sInfo.AppendFormat(L"\r\n%s", e);
			break;

		case NS_ERROR_PRIVATE_VTDU_STATUS_491:
			e = GetStringFromAppResource(IDS_STRING_VTDU_STATUS_491);
			sInfo.AppendFormat(L"\r\n%s", e);
			break;
		}

		if (cur_info) {
			std::lock_guard<std::recursive_mutex> lock(m_lock4CurRecordingInfoList);
			StopPlay(cur_info);
			m_curRecordingInfoList.remove(cur_info);
			recycle_player_ctrl(cur_info->_ctrl);
		}
		
		MessageBox(sInfo, sTitle, MB_ICONINFORMATION);
	}
		break;
	case CSdkMgrEzviz::INS_PLAY_RECONNECT:
		break;
	case CSdkMgrEzviz::INS_PLAY_RECONNECT_EXCEPTION:
		//pInstance->insPlayReconnectException(iErrorCode, pMessageInfo);
		break;

	case CSdkMgrEzviz::INS_PLAY_START:
	{
		if (cur_info && !cur_info->started_) {
			cur_info->started_ = true;
			CString start; start = GetStringFromAppResource(IDS_STRING_VIDEO_START);
			CString record;
			auto device = cur_info->_device;
			record.Format(L"%s([%d,%s]%s)-\"%s\"", start, device->get_id(), 
						  device->get_device_note().c_str(),
						  A2W(device->get_deviceSerial().c_str()), 
						  cur_info->_param->_file_path);

			auto zoneUuid = cur_info->_zone;
			core::history_record_manager* hr = core::history_record_manager::GetInstance();
			hr->InsertRecord(zoneUuid._ademco_id, zoneUuid._zone_value,
							 record, time(nullptr), core::RECORD_LEVEL_VIDEO);
		}
	}
		break;

	case CSdkMgrEzviz::INS_PLAY_STOP:
	{
		if (cur_info) {
			std::lock_guard<std::recursive_mutex> lock(m_lock4CurRecordingInfoList);
			auto device = cur_info->_device;
			CString record, stop; stop = GetStringFromAppResource(IDS_STRING_VIDEO_STOP);
			record.Format(L"%s([%d,%s]%s)-\"%s\"", stop, device->get_id(),
						  device->get_device_note().c_str(),
						  A2W(device->get_deviceSerial().c_str()),
						  (cur_info->_param->_file_path));

			auto zoneUuid = cur_info->_zone;
			core::history_record_manager* hr = core::history_record_manager::GetInstance();
			hr->InsertRecord(zoneUuid._ademco_id, zoneUuid._zone_value,
							 record, time(nullptr), core::RECORD_LEVEL_VIDEO);
			/*video::ezviz::CSdkMgrEzviz* mgr = video::ezviz::CSdkMgrEzviz::GetInstance();
			auto user = std::dynamic_pointer_cast<video::ezviz::CVideoUserInfoEzviz>(device->get_userInfo());
			std::string session_id = mgr->GetSessionId(user->get_user_phone(), device->get_cameraId(), messageHandler, this);
			mgr->m_dll.setDataCallBack(session_id, videoDataHandler, nullptr);*/
			m_curRecordingInfoList.remove(cur_info);		
			recycle_player_ctrl(cur_info->_ctrl);
		}
	}
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
		
		break;
	}

	sInfo.Format(L"MsgType=%d\r\nErrorCode = %d\r\nErrorMsg=%s",
				 msg->iMsgType, msg->iErrorCode, A2W(msg->messageInfo.c_str()));
	//MessageBox(info, L"", MB_ICONINFORMATION);
	JLOG(sInfo);
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
	//, m_level(0)
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
	DDX_Control(pDX, IDC_LIST1, m_ctrl_play_list);
	DDX_Control(pDX, IDC_EDIT_MINUTE, m_ctrl_rerord_minute);
	DDX_Control(pDX, IDC_BUTTON_SAVE, m_btn_save);
	DDX_Control(pDX, IDC_STATIC_VIDEO_LIST, m_group_video_list);
	DDX_Control(pDX, IDC_STATIC_RECORD_SETTINGS, m_group_record_settings);
	DDX_Control(pDX, IDC_STATIC_NOTE, m_static_note);
	DDX_Control(pDX, IDC_STATIC_MINUTE, m_static_minute);
	DDX_Control(pDX, IDC_RADIO_SMOOTH2, m_radioGlobalSmooth);
	DDX_Control(pDX, IDC_RADIO_BALANCE2, m_radioGlobalBalance);
	DDX_Control(pDX, IDC_RADIO_HD2, m_radioGlobalHD);
	DDX_Control(pDX, IDC_STATIC_NOTE2, m_staticNote2);
	DDX_Control(pDX, IDC_RADIO_1_VIDEO, m_chk_1_video);
	DDX_Control(pDX, IDC_RADIO_4_VIDEO, m_chk_4_video);
	DDX_Control(pDX, IDC_RADIO_9_VIDEO, m_chk_9_video);
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
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CVideoPlayerDlg::OnBnClickedButtonSave)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, &CVideoPlayerDlg::OnLvnItemchangedList1)
	ON_NOTIFY(HDN_ITEMCHANGED, 0, &CVideoPlayerDlg::OnHdnItemchangedList1)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CVideoPlayerDlg::OnNMDblclkList1)
	ON_EN_CHANGE(IDC_EDIT_MINUTE, &CVideoPlayerDlg::OnEnChangeEditMinute)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_RADIO_SMOOTH2, &CVideoPlayerDlg::OnBnClickedRadioSmooth2)
	ON_BN_CLICKED(IDC_RADIO_BALANCE2, &CVideoPlayerDlg::OnBnClickedRadioBalance2)
	ON_BN_CLICKED(IDC_RADIO_HD2, &CVideoPlayerDlg::OnBnClickedRadioHd2)
	ON_BN_CLICKED(IDC_RADIO_1_VIDEO, &CVideoPlayerDlg::OnBnClickedRadio1Video)
	ON_BN_CLICKED(IDC_RADIO_4_VIDEO, &CVideoPlayerDlg::OnBnClickedRadio4Video)
	ON_BN_CLICKED(IDC_RADIO_9_VIDEO, &CVideoPlayerDlg::OnBnClickedRadio9Video)
END_MESSAGE_MAP()


// CVideoPlayerDlg message handlers


BOOL CVideoPlayerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	GetWindowText(m_title);
	video::CVideoManager* videoMgr = video::CVideoManager::GetInstance();
	videoMgr->LoadFromDB();
	g_player = this;
	//GetWindowRect(m_rcNormal);
	//m_player.GetWindowRect(m_rcNormalPlayer);

	SetTimer(TIMER_ID_EZVIZ_MSG, 1000, nullptr);
	SetTimer(TIMER_ID_REC_VIDEO, 2000, nullptr);
	SetTimer(TIMER_ID_PLAY_VIDEO, 1000, nullptr);

	m_radioSmooth.SetCheck(1);
	EnableOtherCtrls(0, 0);

	m_dwPlayerStyle = m_player.GetStyle();
	m_player.ShowWindow(SW_HIDE);

	RegisterHotKey(GetSafeHwnd(), HOTKEY_PTZ, MOD_ALT, VK_LEFT);
	RegisterHotKey(GetSafeHwnd(), HOTKEY_PTZ, MOD_ALT, VK_RIGHT);
	RegisterHotKey(GetSafeHwnd(), HOTKEY_PTZ, MOD_ALT, VK_UP);
	RegisterHotKey(GetSafeHwnd(), HOTKEY_PTZ, MOD_ALT, VK_DOWN);
	RegisterHotKey(GetSafeHwnd(), HOTKEY_PTZ, MOD_ALT, 'C');

	// init list header
	DWORD dwStyle = m_ctrl_play_list.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
	m_ctrl_play_list.SetExtendedStyle(dwStyle);
	int ndx = -1;
	CString fm;
	fm = GetStringFromAppResource(IDS_STRING_ID);
	m_ctrl_play_list.InsertColumn(++ndx, fm, LVCFMT_LEFT, 50, -1);
	fm = GetStringFromAppResource(IDS_STRING_NAME);
	m_ctrl_play_list.InsertColumn(++ndx, fm, LVCFMT_LEFT, 70, -1);
	fm = GetStringFromAppResource(IDS_STRING_NOTE);
	m_ctrl_play_list.InsertColumn(++ndx, fm, LVCFMT_LEFT, 150, -1);
	fm = GetStringFromAppResource(IDS_STRING_DEVICE_SERIAL);
	m_ctrl_play_list.InsertColumn(++ndx, fm, LVCFMT_LEFT, 100, -1);

	fm.Format(L"%d", util::CConfigHelper::GetInstance()->get_back_end_record_minutes());
	m_ctrl_rerord_minute.SetWindowTextW(fm);

	core::user_manager* mgr = core::user_manager::GetInstance();
	auto user = mgr->GetCurUserInfo();
	OnCurUserChangedResult(user);
	m_cur_user_changed_observer = std::make_shared<CurUserChangedObserver>(this);
	mgr->register_observer(m_cur_user_changed_observer);

	CRect rc;
	m_player.GetWindowRect(rc);
	ScreenToClient(rc);
	const int same_time_play_vidoe_route_count = util::CConfigHelper::GetInstance()->get_show_video_same_time_route_count();
	for (int i = 0; i < same_time_play_vidoe_route_count; i++) {
		auto player = std::make_shared<video_player_ctrl_with_status>();
		player->ctrl = std::make_shared<CVideoPlayerCtrl>();
		player->ctrl->ndx_ = i + 1;
		player->ctrl->Create(nullptr, m_dwPlayerStyle, rc, this, IDC_STATIC_PLAYER);
		player_ctrls_.push_back(player);
	}

	// rebuild ndx
	ndx = 1;
	for (auto player : player_ctrls_) {
		player->ctrl->ndx_ = ndx++;
	}

	m_bInitOver = TRUE;

	LoadPosition();
	
	m_chk_1_video.SetCheck(same_time_play_vidoe_route_count == 1);
	m_chk_4_video.SetCheck(same_time_play_vidoe_route_count == 4);
	m_chk_9_video.SetCheck(same_time_play_vidoe_route_count == 9);
	SetSameTimePlayVideoRoute(same_time_play_vidoe_route_count);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CVideoPlayerDlg::SetSameTimePlayVideoRoute(const int n)
{
	CRect rc;
	m_player.GetWindowRect(rc);
	ScreenToClient(rc);

	auto cfg = util::CConfigHelper::GetInstance();
	const int prev_count = cfg->get_show_video_same_time_route_count();
	cfg->set_show_video_same_time_route_count(n);

	auto v = split_rect(rc, n);

	for (int i = 0; i < n && i < prev_count; i++) {
		player_ctrls_[i]->ctrl->MoveWindow(v[i]); 
		player_ctrls_[i]->ctrl->ShowWindow(SW_SHOW);
		player_ctrls_[i]->rc = v[i];
	}

	if (prev_count < n) {
		for (int i = prev_count; i < n; i++) {
			auto player = std::make_shared<video_player_ctrl_with_status>();
			player->ctrl = create_new_ctrl();
			player->used = true;
			player_ctrls_.push_back(player);
		}

	} else {
		for (int i = n; i < prev_count; i++) {
			recycle_player_ctrl(player_ctrls_[i]->ctrl);
		}

		while (player_ctrls_.size() > (size_t)n) {
			player_ctrls_.pop_back();
		}
	}

	// rebuild ndx
	int ndx = 1;
	for (auto player : player_ctrls_) {
		player->ctrl->ndx_ = ndx++;
	}

	/*if (n == 4) {
		
	} else if (n == 9) {
		util::CConfigHelper::GetInstance()->set_show_video_same_time_route_count(9);
		auto v = split_rect(rc, 9);
		for (int i = 0; i < 9; i++) {
			player_ctrls_[i]->ctrl->MoveWindow(v[i]);
			player_ctrls_[i]->ctrl->ShowWindow(SW_SHOW);
		}
	} else {
		util::CConfigHelper::GetInstance()->set_show_video_same_time_route_count(1);
		player_ctrls_[0]->ctrl->ShowWindow(SW_SHOW);
		for (int i = 1; i < 9; i++) {
			player_ctrls_[i]->ctrl->ShowWindow(SW_HIDE);
		}
	}*/
}


void CVideoPlayerDlg::LoadPosition()
{
	AUTO_LOG_FUNCTION;
	auto cfg = util::CConfigHelper::GetInstance();

	do{
		CRect rect = cfg->get_rectVideoPlayerDlg();
		if (rect.IsRectNull() || rect.IsRectEmpty()) {
			break;
		}
		int m = cfg->get_maximizedVideoPlayerDlg();

		if (m) {
			CRect rcNormal;
			GetWindowRect(rcNormal);
			rect.right = rect.left + rcNormal.Width();
			rect.bottom = rect.top + rcNormal.Height();
			MoveWindow(rect);
		} else {
			CRect rc;
			GetWindowRect(rc);
			rect.right = rect.left + rc.Width();
			rect.bottom = rect.top + rc.Height();
			MoveWindow(rect);
			GetWindowPlacement(&m_rcNormal);
			m_player.GetWindowPlacement(&m_rcNormalPlayer);
		}

		m_player.SetMaximized(m);
		OnInversioncontrol(m, 0);

	} while (0);
}


void CVideoPlayerDlg::SavePosition()
{
	auto cfg = util::CConfigHelper::GetInstance();

	CRect rect;
	GetWindowRect(rect);

	cfg->set_rectVideoPlayerDlg(rect);
	cfg->set_maximizedVideoPlayerDlg(m_player.GetMaximized());
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

	m_group_video_list.ShowWindow(sw);
	m_ctrl_play_list.ShowWindow(sw);
	m_group_record_settings.ShowWindow(sw);
	m_static_note.ShowWindow(sw);
	m_ctrl_rerord_minute.ShowWindow(sw);
	m_static_minute.ShowWindow(sw);
	//m_btn_save.ShowWindow(sw);
	m_staticNote2.ShowWindow(sw);
	m_radioGlobalSmooth.ShowWindow(sw);
	m_radioGlobalBalance.ShowWindow(sw);
	m_radioGlobalHD.ShowWindow(sw);

	m_chk_1_video.ShowWindow(sw);
	m_chk_4_video.ShowWindow(sw);
	m_chk_9_video.ShowWindow(sw);
}


void CVideoPlayerDlg::EnableOtherCtrls(BOOL bAble, int level)
{
	m_radioSmooth.EnableWindow(bAble);
	m_radioSmooth.SetCheck(level == 0);
	m_radioBalance.EnableWindow(bAble); m_radioBalance.SetCheck(level == 1);
	m_radioHD.EnableWindow(bAble); m_radioHD.SetCheck(level == 2);
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
			CRect rc;
			GetClientRect(rc);
			m_player.MoveWindow(rc);
			/*std::lock_guard<std::recursive_mutex> lock(m_lock4CurRecordingInfoList);
			for (auto info : m_curRecordingInfoList) {
				info->_ctrl->MoveWindow(rc);
				if (info->_device == m_curPlayingDevice) {
					info->_ctrl->ShowWindow(SW_SHOW);
				} else {
					info->_ctrl->ShowWindow(SW_HIDE);
				}
			}*/
			
		} else {
			ShowOtherCtrls(1);
			SetWindowPlacement(&m_rcNormal);
			m_player.SetWindowPlacement(&m_rcNormalPlayer);
			/*std::lock_guard<std::recursive_mutex> lock(m_lock4CurRecordingInfoList);
			for (auto info : m_curRecordingInfoList) {
				info->_ctrl->SetWindowPlacement(&m_rcNormalPlayer);
				if (info->_device == m_curPlayingDevice) {
					info->_ctrl->ShowWindow(SW_SHOW);
				} else {
					info->_ctrl->ShowWindow(SW_HIDE);
				}
			}*/
		}

		update_players_size_with_m_player();

		SavePosition();
	}
	return 0;
}


void CVideoPlayerDlg::PlayVideoByDevice(video::CVideoDeviceInfoPtr device, int speed)
{
	AUTO_LOG_FUNCTION;
	ShowWindow(SW_SHOWNORMAL);
	assert(device);
	CVideoUserInfoPtr user = device->get_userInfo(); assert(user);
	if (EZVIZ == user->get_productorInfo().get_productor()) {
		PlayVideoEzviz(std::dynamic_pointer_cast<video::ezviz::CVideoDeviceInfoEzviz>(device), speed);
	} else {
		ASSERT(0); m_curPlayingDevice = nullptr;
	}
}


void CVideoPlayerDlg::StopPlay()
{
	AUTO_LOG_FUNCTION;
	if (m_curPlayingDevice) {
		EnableOtherCtrls(0, 0);
		StopPlayEzviz(std::dynamic_pointer_cast<video::ezviz::CVideoDeviceInfoEzviz>(m_curPlayingDevice));
		m_curPlayingDevice = nullptr;
	}
}


void CVideoPlayerDlg::PlayVideoEzviz(video::ezviz::CVideoDeviceInfoEzvizPtr device, int videoLevel)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	assert(device);
	do {
		if (m_curPlayingDevice == device) { // same dev
			std::lock_guard<std::recursive_mutex> lock(m_lock4CurRecordingInfoList);
			bool bFound = false;
			int level = 0;
			for (auto info : m_curRecordingInfoList) {
				if (info->_device == device) {
					bFound = true;
					info->_param->_start_time = /*COleDateTime::GetCurrentTime()*/GetTickCount();
					//info->_ctrl->ShowWindow(SW_SHOW);
					bring_player_to_front(info->_ctrl);
					level = info->_level;
					EnableOtherCtrls(TRUE, level);
					break;
				}
			}
			if (bFound) {
				if (videoLevel == level) {
					EnableOtherCtrls(TRUE, level);
					return;
				} else {
					StopPlay();
				}
			}
			
		} else { // differrent dev
			std::lock_guard<std::recursive_mutex> lock(m_lock4CurRecordingInfoList);
			bool bFound = false;
			int level = 0;
			for (auto info : m_curRecordingInfoList) {
				if (info->_device == device) {
					bFound = true;
					level = info->_level;
					break;
				} 
			}

			if (bFound) { // already playing in back-end
				if (videoLevel == level) { // same level, bring it to front-end
					for (auto info : m_curRecordingInfoList) {
						if (info->_device != device) {
							//info->_ctrl->ShowWindow(SW_HIDE);
						} else {
							info->_param->_start_time = /*COleDateTime::GetCurrentTime()*/ GetTickCount();
							//info->_ctrl->ShowWindow(SW_SHOW);
							bring_player_to_front(info->_ctrl);
							EnableOtherCtrls(TRUE, level);
						}
					}
					m_curPlayingDevice = device;
					return;
				} else {
					StopPlay();
				}				
			} else { // not playing
				if (m_curRecordingInfoList.size() >= 8) {
					for (auto info : m_curRecordingInfoList) {
						if (info->_device != m_curPlayingDevice) {
							StopPlay(info);
							m_curRecordingInfoList.remove(info);
							recycle_player_ctrl(info->_ctrl);
							break;
						}
					}
				}
			}
		}
		
		m_curPlayingDevice = device;
		auto user = std::dynamic_pointer_cast<video::ezviz::CVideoUserInfoEzviz>(device->get_userInfo()); 
		assert(user);
		video::ezviz::CSdkMgrEzviz* mgr = video::ezviz::CSdkMgrEzviz::GetInstance();
		CString e;
		if (user->get_user_accToken().size() == 0) {
			if (video::ezviz::CSdkMgrEzviz::RESULT_OK != mgr->VerifyUserAccessToken(user, TYPE_GET)) {
				e = GetStringFromAppResource(IDS_STRING_PRIVATE_CLOUD_CONN_FAIL_OR_USER_NOT_EXSIST);
				MessageBox(e, L"", MB_ICONINFORMATION);
				break;
			}
			user->execute_set_user_token_time(COleDateTime::GetCurrentTime());
		}
		bool bEncrypt = false;
		int ret = mgr->m_dll.UpdateCameraInfo(device->get_cameraId(), user->get_user_accToken(), bEncrypt);
		if (ret != 0) {
			e = GetStringFromAppResource(IDS_STRING_UPDATE_CAMERA_INFO_FAILED);
			core::history_record_manager::GetInstance()->InsertRecord(-1, 0, e, time(nullptr), core::RECORD_LEVEL_VIDEO);
			MessageBox(e, L"", MB_ICONINFORMATION);
			break;
		}

		if (bEncrypt && device->get_secure_code().size() != 6) {
			CInputDeviceVerifyCodeDlg dlg(this);
			if (dlg.DoModal() != IDOK) {
				break;
			}
			device->set_secure_code(W2A(dlg.m_result));
			device->execute_update_info();
		}
		std::string session_id = mgr->GetSessionId(user->get_user_phone(), device->get_cameraId(), messageHandler, this);
		DataCallbackParam *param = new DataCallbackParam(this, session_id, /*time(nullptr)*/ GetTickCount());
		CString filePath = param->FormatFilePath(device->get_userInfo()->get_id(), device->get_userInfo()->get_user_name(),
												 device->get_id(), device->get_device_note());
		mgr->m_dll.setDataCallBack(session_id, videoDataHandler, param);
		//CVideoPlayerCtrl* ctrl = new CVideoPlayerCtrl();
		auto ctrl = get_free_player_ctrl();
		ret = mgr->m_dll.startRealPlay(session_id, 
									   ctrl->m_hWnd, 
									   device->get_cameraId(), 
									   user->get_user_accToken(),
									   device->get_secure_code(), 
									   util::CConfigHelper::GetInstance()->get_ezviz_private_cloud_app_key(), 
									   videoLevel);

		if (ret == 20005 || ret == OPEN_SDK_IDENTIFY_FAILED) { // 硬件特征码校验失败，需重新进行认证
			bool ok = false;
			do {
				char reqStr[1024] = { 0 };
				sprintf_s(reqStr, SMSCODE_SECURE_REQ, user->get_user_accToken().c_str());
				char* pOutStr = nullptr;
				int iLen = 0;
				ret = mgr->m_dll.RequestPassThrough(reqStr, &pOutStr, &iLen);
				if (ret != 0) {
					JLOG(L"调用透传接口失败， 返回错误码为：%d", ret);
					break;
				}
				pOutStr[iLen] = 0;
				std::string json = pOutStr;
				mgr->m_dll.freeData(pOutStr);

				
				Json::Reader reader;
				Json::Value	value;
				if (!reader.parse(json.c_str(), value)) {
					JLOG(L"获取短信验证码解析Json串失败!");
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
				CInputPasswdDlg dlg(this);
				dlg.m_title = GetStringFromAppResource(IDS_STRING_INPUT_PHONE_VERIFY_CODE);
				if (IDOK != dlg.DoModal())
					break;
				std::string verify_code = W2A(dlg.m_edit);

				char reqStr[1024] = { 0 };
				sprintf_s(reqStr, SECUREVALIDATE_REQ, verify_code.c_str(), user->get_user_accToken().c_str());
				char* pOutStr = nullptr;
				int iLen = 0;
				ret = mgr->m_dll.RequestPassThrough(reqStr, &pOutStr, &iLen);
				if (ret != 0) {
					JLOG(L"调用透传接口失败， 返回错误码为：%d", ret);
					break;
				}
				pOutStr[iLen] = 0;
				std::string json = pOutStr;
				mgr->m_dll.freeData(pOutStr);

				Json::Reader reader;
				Json::Value	value;
				if (!reader.parse(json.c_str(), value)) {
					JLOG(L"验证短信验证码解析Json串失败!");
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
				ret = mgr->m_dll.startRealPlay(session_id,
											   ctrl->m_hWnd,
											   device->get_cameraId(),
											   user->get_user_accToken(),
											   device->get_secure_code(), 
											   util::CConfigHelper::GetInstance()->get_ezviz_private_cloud_app_key(), 
											   videoLevel);
			else
				ret = -1;
		}

		if (ret != 0) {
			JLOG(L"startRealPlay failed %d\n", ret);
			m_curPlayingDevice = nullptr;
			SAFEDELETEP(param);
			recycle_player_ctrl(ctrl);
		} else {
			JLOG(L"PlayVideo ok\n");

			EnableOtherCtrls(TRUE, videoLevel);
			std::lock_guard<std::recursive_mutex> lock(m_lock4CurRecordingInfoList);
			ctrl->ShowWindow(SW_SHOW);
			//for (auto info : m_curRecordingInfoList) {
			//	info->_ctrl->ShowWindow(SW_HIDE);
			//}
			video::ZoneUuid zoneUuid = device->GetActiveZoneUuid();
			CString record;
			
			RecordVideoInfoPtr info = std::make_shared<RecordVideoInfo>(param, zoneUuid, device, ctrl, videoLevel);
			m_curRecordingInfoList.push_back(info);
			record.Format(L"%s  ----  %s[%d-%s-%s]", m_title, device->get_userInfo()->get_user_name().c_str(),
						  device->get_id(), device->get_device_note().c_str(), A2W(device->get_deviceSerial().c_str()));
			InsertList(info);
		}
		UpdateWindow();
		return;
	} while (0);
	JLOG(L"PlayVideo failed\n");
	m_curPlayingDevice = nullptr;
	UpdateWindow();
}


void CVideoPlayerDlg::StopPlayEzviz(video::ezviz::CVideoDeviceInfoEzvizPtr device)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	assert(device);
	std::lock_guard<std::recursive_mutex> lock(m_lock4CurRecordingInfoList);
	auto user = std::dynamic_pointer_cast<video::ezviz::CVideoUserInfoEzviz>(device->get_userInfo()); assert(user);
	video::ezviz::CSdkMgrEzviz* mgr = video::ezviz::CSdkMgrEzviz::GetInstance();
	std::string session_id = mgr->GetSessionId(user->get_user_phone(), device->get_cameraId(), messageHandler, this);
	video::ezviz::CSdkMgrEzviz::NSCBMsg msg;
	//mgr->m_dll.setDataCallBack(session_id, videoDataHandler, nullptr);
	mgr->m_dll.stopRealPlay(session_id, &msg);
	
	for (auto info : m_curRecordingInfoList) {
		if (info->_param->_session_id == session_id) {
			for (int i = 0; i < m_ctrl_play_list.GetItemCount(); i++) {
				int id = m_ctrl_play_list.GetItemData(i);
				if (id == info->_device->get_id()) {
					m_ctrl_play_list.DeleteItem(i);
					break;
				}
			}
			break;
		}
	}
}


void CVideoPlayerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	UnregisterHotKey(GetSafeHwnd(), HOTKEY_PTZ);

	for (auto info : m_curRecordingInfoList) {
		StopPlayEzviz(info->_device);
	}

	MSG msg;
	while (GetMessage(&msg, m_hWnd, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (m_curRecordingInfoList.empty())
			break;
	}

	video::CVideoManager::ReleaseObject();

	KillTimer(TIMER_ID_EZVIZ_MSG);
	KillTimer(TIMER_ID_REC_VIDEO);
	KillTimer(TIMER_ID_PLAY_VIDEO);

	m_ezvizMsgList.clear();	
	m_curRecordingInfoList.clear();
	m_wait2playDevList.clear();

	g_player = nullptr;
}

namespace detail
{
	class autoTimer
	{
	public:
		int m_timer_id;
		DWORD m_time_out;
		HWND m_hWnd;
		autoTimer(HWND hWnd, int timerId, DWORD timeout) : m_hWnd(hWnd), m_timer_id(timerId), m_time_out(timeout)
		{
			KillTimer(hWnd, m_timer_id);
		}
		~autoTimer()
		{
			SetTimer(m_hWnd, m_timer_id, m_time_out, nullptr);
		}
	};
};


void CVideoPlayerDlg::OnTimer(UINT_PTR nIDEvent)
{
	//AUTO_LOG_FUNCTION;
	if (TIMER_ID_EZVIZ_MSG == nIDEvent) {
		detail::autoTimer timer(m_hWnd, TIMER_ID_EZVIZ_MSG, 1000);
		if (m_lock4EzvizMsgQueue.try_lock()) {
			std::lock_guard<std::mutex> lock(m_lock4EzvizMsgQueue, std::adopt_lock);
			if (m_ezvizMsgList.size() > 0) {
				auto msg = m_ezvizMsgList.front();
				m_ezvizMsgList.pop_front();
				HandleEzvizMsg(msg);
			}
		}
	} else if (TIMER_ID_REC_VIDEO == nIDEvent) {
		detail::autoTimer timer(m_hWnd, TIMER_ID_REC_VIDEO, 2000);
		if (m_lock4CurRecordingInfoList.try_lock()) {
			std::lock_guard<std::recursive_mutex> lock(m_lock4CurRecordingInfoList, std::adopt_lock);
			/*COleDateTime now = COleDateTime::GetCurrentTime();*/
			auto now = GetTickCount();
			const int max_minutes = util::CConfigHelper::GetInstance()->get_back_end_record_minutes();
			for (const auto info : m_curRecordingInfoList) {
				if (info->_device != m_curPlayingDevice) {
					/*COleDateTimeSpan span = now - info->_param->_start_time;*/
					DWORD span = (now - info->_param->_start_time) / 1000 / 60;
					if (span/*.GetTotalMinutes()*/ >= (const DWORD)max_minutes) {
						StopPlay(info);
						m_curRecordingInfoList.remove(info);
						break;
					}
				}
			}
		}
	} else if (TIMER_ID_PLAY_VIDEO == nIDEvent) {
		detail::autoTimer timer(m_hWnd, TIMER_ID_PLAY_VIDEO, 5000);
		if (!m_wait2playDevList.empty()) {
			video::ezviz::CVideoDeviceInfoEzvizPtr dev;
			if (m_lock4Wait2PlayDevList.try_lock()) {
				std::lock_guard<std::mutex> lock(m_lock4Wait2PlayDevList, std::adopt_lock);
				dev = m_wait2playDevList.front();
				m_wait2playDevList.pop_front();
			}
			
			if (dev) {
				JLOG(L"ontimer TIMER_ID_PLAY_VIDEO, PlayVideoByDevice");
				PlayVideoByDevice(dev, util::CConfigHelper::GetInstance()->get_default_video_level());
				JLOG(L"ontimer TIMER_ID_PLAY_VIDEO, PlayVideoByDevice over");
			}
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CVideoPlayerDlg::StopPlay(RecordVideoInfoPtr info)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	if (m_curPlayingDevice == info->_device) {
		m_curPlayingDevice = nullptr;
		EnableOtherCtrls(0);
	}
	video::ezviz::CSdkMgrEzviz* mgr = video::ezviz::CSdkMgrEzviz::GetInstance();
	mgr->m_dll.stopRealPlay(info->_param->_session_id);
	core::history_record_manager* hr = core::history_record_manager::GetInstance();
	CString record, stop; stop = GetStringFromAppResource(IDS_STRING_VIDEO_STOP);
	record.Format(L"%s([%d,%s]%s)-\"%s\"", stop, info->_device->get_id(), 
				  info->_device->get_device_note().c_str(),
				  A2W(info->_device->get_deviceSerial().c_str()), 
				  (info->_param->_file_path));
	hr->InsertRecord(info->_zone._ademco_id, info->_zone._zone_value,
					 record, time(nullptr), core::RECORD_LEVEL_VIDEO);
	for (int i = 0; i < m_ctrl_play_list.GetItemCount(); i++) {
		int id = m_ctrl_play_list.GetItemData(i);
		if (id == info->_device->get_id()) {
			m_ctrl_play_list.DeleteItem(i);
			break;
		}
	}
}


void CVideoPlayerDlg::OnBnClickedRadio1()
{
	if (m_curPlayingDevice) {
		PlayVideoByDevice(m_curPlayingDevice, 0);
	}
}


void CVideoPlayerDlg::OnBnClickedRadio2()
{
	if (m_curPlayingDevice) {
		PlayVideoByDevice(m_curPlayingDevice, 1);
	}
}


void CVideoPlayerDlg::OnBnClickedRadio3()
{
	if (m_curPlayingDevice) {
		PlayVideoByDevice(m_curPlayingDevice, 2);
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
		auto device = std::dynamic_pointer_cast<CVideoDeviceInfoEzviz>(m_curPlayingDevice);
		auto user = std::dynamic_pointer_cast<video::ezviz::CVideoUserInfoEzviz>(m_curPlayingDevice->get_userInfo()); assert(user);
		video::ezviz::CSdkMgrEzviz* mgr = video::ezviz::CSdkMgrEzviz::GetInstance();
		CString path, file, fm, txt;
		path.Format(L"%s\\data\\video_capture", GetModuleFilePath());
		file.Format(L"\\%s-%s.jpg", A2W(device->get_deviceSerial().c_str()), 
					CTime::GetCurrentTime().Format(L"%Y-%m-%d-%H-%M-%S"));
		CreateDirectory(path, nullptr);
		path += file;
		fm = GetStringFromAppResource(IDS_STRING_FM_CAPTURE_OK);
		txt.Format(fm, path);
		std::string name = W2A(path);
		if (0 == mgr->m_dll.capturePicture(mgr->GetSessionId(user->get_user_phone(), device->get_cameraId(), messageHandler, this), name)) {
			MessageBox(txt);
		}
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
		auto device = std::dynamic_pointer_cast<video::ezviz::CVideoDeviceInfoEzviz>(m_curPlayingDevice);
		auto user = std::dynamic_pointer_cast<video::ezviz::CVideoUserInfoEzviz>(device->get_userInfo()); assert(user);
		auto mgr = video::ezviz::CSdkMgrEzviz::GetInstance();
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
		auto device = std::dynamic_pointer_cast<video::ezviz::CVideoDeviceInfoEzviz>(bi._device);
		std::lock_guard<std::mutex> lock(m_lock4Wait2PlayDevList);
		device->SetActiveZoneUuid(zone);
		m_wait2playDevList.push_back(device);
	}
}


void CVideoPlayerDlg::OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2)
{
	if (nHotKeyId == HOTKEY_PTZ) {
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
			case 'C':
				OnBnClickedButtonCapture();
				break;
			default:
				break;
		}
	}

	CDialogEx::OnHotKey(nHotKeyId, nKey1, nKey2);
}


void CVideoPlayerDlg::OnBnClickedButtonSave()
{
	CString txt;
	m_ctrl_rerord_minute.GetWindowTextW(txt);
	int minutes = _ttoi(txt);
	if (minutes <= 0) {
		minutes = 10;
	}
	util::CConfigHelper::GetInstance()->set_back_end_record_minutes(minutes);
	txt.Format(L"%d", minutes);
	m_ctrl_rerord_minute.SetWindowTextW(txt);
}


void CVideoPlayerDlg::InsertList(const RecordVideoInfoPtr& info)
{
	USES_CONVERSION;
	int nResult = -1;
	LV_ITEM lvitem = { 0 };
	CString tmp = _T("");

	lvitem.lParam = info->_device->get_id();
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = m_ctrl_play_list.GetItemCount();
	lvitem.iSubItem = 0;

	// ID
	tmp.Format(_T("%d"), info->_device->get_id());
	lvitem.pszText = tmp.LockBuffer();
	nResult = m_ctrl_play_list.InsertItem(&lvitem);
	tmp.UnlockBuffer();

	if (nResult != -1) {
		// 用户
		lvitem.iItem = nResult;
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), info->_device->get_userInfo()->get_user_name().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_ctrl_play_list.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// 备注
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), info->_device->get_device_note().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_ctrl_play_list.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// 设备序列号
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), A2W(info->_device->get_deviceSerial().c_str()));
		lvitem.pszText = tmp.LockBuffer();
		m_ctrl_play_list.SetItem(&lvitem);
		tmp.UnlockBuffer();

		m_ctrl_play_list.SetItemData(nResult, info->_device->get_id());
		m_ctrl_play_list.SetItemState(nResult, LVNI_FOCUSED | LVIS_SELECTED, LVNI_FOCUSED | LVIS_SELECTED);
	}
}


void CVideoPlayerDlg::OnLvnItemchangedList1(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	//LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;	
}


void CVideoPlayerDlg::OnHdnItemchangedList1(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	//LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	*pResult = 0;	
}


void CVideoPlayerDlg::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;
	if (pNMItemActivate->iItem < 0)return;
	int id = m_ctrl_play_list.GetItemData(pNMItemActivate->iItem);
	std::lock_guard<std::recursive_mutex> lock(m_lock4CurRecordingInfoList);
	for (auto info : m_curRecordingInfoList) {
		if (info->_device->get_id() == id) {
			PlayVideoByDevice(info->_device, info->_level);
			break;
		}
	}
}


void CVideoPlayerDlg::OnEnChangeEditMinute()
{
	CString txt;
	m_ctrl_rerord_minute.GetWindowTextW(txt);
	int minutes = _ttoi(txt);
	if (minutes <= 0) {
		minutes = 10;
	}
	auto cfg = util::CConfigHelper::GetInstance();
	if (minutes != cfg->get_back_end_record_minutes()) {
		util::CConfigHelper::GetInstance()->set_back_end_record_minutes(minutes);
		txt.Format(L"%d", minutes);
		m_ctrl_rerord_minute.SetWindowTextW(txt);
	}
}


void CVideoPlayerDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	if (bShow) {
		int global_video_level = util::CConfigHelper::GetInstance()->get_default_video_level();
		m_radioGlobalSmooth.SetCheck(global_video_level == 0);
		m_radioGlobalBalance.SetCheck(global_video_level == 1);
		m_radioGlobalHD.SetCheck(global_video_level == 2);
	}
}


void CVideoPlayerDlg::OnBnClickedRadioSmooth2()
{
	util::CConfigHelper::GetInstance()->set_default_video_level(0);
}


void CVideoPlayerDlg::OnBnClickedRadioBalance2()
{
	util::CConfigHelper::GetInstance()->set_default_video_level(1);
}


void CVideoPlayerDlg::OnBnClickedRadioHd2()
{
	util::CConfigHelper::GetInstance()->set_default_video_level(2);
}


void CVideoPlayerDlg::OnBnClickedRadio1Video()
{
	SetSameTimePlayVideoRoute(1);
}


void CVideoPlayerDlg::OnBnClickedRadio4Video()
{
	SetSameTimePlayVideoRoute(4);
}


void CVideoPlayerDlg::OnBnClickedRadio9Video()
{
	SetSameTimePlayVideoRoute(9);
}


CVideoPlayerCtrlPtr CVideoPlayerDlg::create_new_ctrl()
{
	CRect rc;
	m_player.GetWindowRect(rc);
	ScreenToClient(rc);
	auto ctrl = std::make_shared<CVideoPlayerCtrl>();
	ctrl->Create(nullptr, m_dwPlayerStyle, rc, this, IDC_STATIC_PLAYER);
	return ctrl;
}


CVideoPlayerCtrlPtr CVideoPlayerDlg::get_free_player_ctrl() {
	CVideoPlayerCtrlPtr ctrl;
	for (int i = 0; i < util::CConfigHelper::GetInstance()->get_show_video_same_time_route_count(); i++) {
		auto iter = player_ctrls_[i];
		if (!iter->used) { // has free ctrl, show it
			ctrl = iter->ctrl;
			ctrl->ShowWindow(SW_SHOW);
			iter->used = true;
			break;
		}
	}

	if (!ctrl) { // no free ctrl, move current first ctrl to back-end, create new ctrl and bring it to front
		if (!buffered_player_ctrls_.empty()) {
			ctrl = buffered_player_ctrls_.front();
			buffered_player_ctrls_.pop_front();
		} else {
			ctrl = create_new_ctrl();
		}

		assert(ctrl);

		bring_player_to_front(ctrl);
	}

	return ctrl;
}


void CVideoPlayerDlg::bring_player_to_front(const CVideoPlayerCtrlPtr& ctrl)
{
	// already playing
	for (auto iter : player_ctrls_) {
		if (iter->ctrl == ctrl) {
			return;
		}
	}

	// not playing
	auto& player_0 = player_ctrls_[0];
	CRect rc;
	player_0->ctrl->GetWindowRect(rc);
	ScreenToClient(rc); // get player 1's rc

	for (int i = 1; i < util::CConfigHelper::GetInstance()->get_show_video_same_time_route_count(); i++) { // 后n个player前移1位
		CRect my_rc;
		player_ctrls_[i]->ctrl->GetWindowRect(my_rc);
		ScreenToClient(my_rc);
		player_ctrls_[i]->ctrl->MoveWindow(rc);
		player_ctrls_[i]->rc = rc;
		player_ctrls_[i]->ctrl->ndx_ = i;
		rc = my_rc;
	}

	ctrl->MoveWindow(rc); // move new ctrl to last place

	recycle_player_ctrl(player_0->ctrl); // move prev-player-1 to back-end
	player_0->ctrl = ctrl;
	player_0->ctrl->ndx_ = util::CConfigHelper::GetInstance()->get_show_video_same_time_route_count();
	player_0->rc = rc;

	// rebuild ndx
	int ndx = 1;
	for (auto player : player_ctrls_) {
		player->ctrl->ndx_ = ndx++;
	}
}


void CVideoPlayerDlg::recycle_player_ctrl(const CVideoPlayerCtrlPtr& ctrl)
{
	assert(ctrl);
	ctrl->ShowWindow(SW_HIDE);

	bool recycled = false;

	for (int i = 0; i < util::CConfigHelper::GetInstance()->get_show_video_same_time_route_count(); i++) {
		auto iter = player_ctrls_[i];
		if (!iter->used) {
			if (iter->ctrl == ctrl) {
				iter->used = false;
				recycled = true;
				break;
			}
		}
	}

	if (recycled) {
		return;
	}
	
	buffered_player_ctrls_.push_back(ctrl);
}


void CVideoPlayerDlg::update_players_size_with_m_player()
{
	CRect rc;
	m_player.GetWindowRect(rc);
	ScreenToClient(rc);

	const int n = util::CConfigHelper::GetInstance()->get_show_video_same_time_route_count();
	auto v = split_rect(rc, n);

	for (int i = 0; i < n; i++) {
		player_ctrls_[i]->ctrl->MoveWindow(v[i]);
		player_ctrls_[i]->rc = v[i];
	}
}
