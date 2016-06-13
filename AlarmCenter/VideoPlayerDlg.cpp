// VideoPlayerDlg.cpp : implementation file
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
#include "JovisonSdkMgr.h"
#include "VideoUserInfoJovision.h"
#include "VideoDeviceInfoJovision.h"


namespace detail {
	const int TIMER_ID_EZVIZ_MSG = 1;
	const int TIMER_ID_REC_VIDEO = 2;
	const int TIMER_ID_PLAY_VIDEO = 3;
	const int TIMER_ID_JOVISION_MSG = 4;

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
			//v[i].DeflateRect(1, 1, 1, 1);
		}
		
		return v;
	};


	CVideoPlayerDlg* g_player = nullptr;

	void safe_post_msg_to_g_player(UINT message, WPARAM wp = 0U, LPARAM lp = 0L) {
		if (g_player) {
			g_player->PostMessageW(message, wp, lp);
		} else {
			assert(0);
		}
	}

	auto player_deleter = [](CVideoPlayerCtrl* p) {
		SAFEDELETEDLG(p);
	};

	typedef struct play_list_item_data {
		video::productor productor = video::UNKNOWN;
		int dev_id = -1;

	}play_list_item_data;
};

using namespace ::detail;
using namespace video;
using namespace video::ezviz;
using namespace video::jovision;


struct CVideoPlayerDlg::record {
	bool started_ = false;
	video::productor productor_ = video::UNKNOWN;
	DataCallbackParamEzviz* _param = nullptr;
	video::zone_uuid _zone = {};
	video::video_device_info_ptr _device = nullptr;
	int _level = 0;
	video::jovision::JCLink_t link_id_ = -1;

	bool voice_talking_ = false;
	bool sound_opened_ = false;
	bool verified_hd_ = false;
	bool e45_occured_ = false;

	bool connecting_ = false;
	bool decoding_ = false;
	bool previewing_ = false;

	player player_ = nullptr;

	record() {}
	record(DataCallbackParamEzviz* param, const video::zone_uuid& zone,
			video::ezviz::video_device_info_ezviz_ptr device, const player& player, int level)
		: _param(param), _zone(zone), _device(device), player_(player), _level(level)
	{
		productor_ = video::EZVIZ;
	}

	record(DataCallbackParamEzviz* param, const video::zone_uuid& zone,
			video::jovision::video_device_info_jovision_ptr device, const player& player, int level)
		:_param(param), _zone(zone), _device(device), player_(player), _level(level)
	{
		productor_ = video::JOVISION;
	}

	~record() { SAFEDELETEP(_param); player_ = nullptr; }
};


#pragma region ezviz callbacks

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
	ezviz_msg_ptr msg = std::make_shared<ezviz_msg>(iMsgType, iErrorCode, szSessionId, pMessageInfo ? pMessageInfo : "");
	dlg->EnqueEzvizMsg(msg);
}

void __stdcall CVideoPlayerDlg::videoDataHandler(sdk_mgr_ezviz::DataType enType,
												 char* const pData,
												 int iLen,
												 void* pUser)
{
	//AUTO_LOG_FUNCTION;
	try {
		if (pUser == nullptr) {
			JLOG(L"pUser == nullptr");
			return;
		}

		DataCallbackParamEzviz* param = reinterpret_cast<DataCallbackParamEzviz*>(pUser); assert(param);
		if (!g_player || !g_player->record_op_is_valid(param)) {
			JLOG(L"g_player == nullptr");
			return;
		}

		////COleDateTime now = COleDateTime::GetCurrentTime();
		//DWORD now = GetTickCount();
		////COleDateTimeSpan span = now - param->_startTime;
		//DWORD span = now - param->_start_time;
		//if (/*span.GetTotalMinutes()*/ span / 1000 / 60 >= (DWORD)util::CConfigHelper::get_instance()->get_back_end_record_minutes()) {
		//	JLOG(L"span.GetTotalMinutes() %d", /*span.GetTotalMinutes()*/ 0);
		//	return;
		//}
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

void CVideoPlayerDlg::EnqueEzvizMsg(const ezviz_msg_ptr& msg)
{
	AUTO_LOG_FUNCTION;
	std::lock_guard<std::mutex> lock(lock_4_ezviz_msg_queue_);
	ezviz_msg_list_.push_back(msg);
}

void CVideoPlayerDlg::HandleEzvizMsg(const ezviz_msg_ptr& msg)
{
	AUTO_LOG_FUNCTION;
	CString sInfo = L"", sTitle = L"", e = L"";

	record_ptr cur_info = nullptr;

	{
		std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_);
		for (auto info : record_list_) {
			if (info->_param->_session_id == msg->sessionId) {
				cur_info = info;
				break;
			}
		}
	}

	switch (msg->iMsgType) {

	case sdk_mgr_ezviz::INS_PLAY_START:
		on_ins_play_start(cur_info);
		break;

	case sdk_mgr_ezviz::INS_PLAY_STOP:
		on_ins_play_stop(cur_info);
		break;

	case sdk_mgr_ezviz::INS_PLAY_EXCEPTION:
		on_ins_play_exception(msg, cur_info);
		break;

	case sdk_mgr_ezviz::INS_PLAY_RECONNECT:
		break;
	case sdk_mgr_ezviz::INS_PLAY_RECONNECT_EXCEPTION:
		break;
	case sdk_mgr_ezviz::INS_PLAY_ARCHIVE_END:
		break;
	case sdk_mgr_ezviz::INS_RECORD_FILE:
		//pInstance->insRecordFile(pMessageInfo);
		break;
	case sdk_mgr_ezviz::INS_RECORD_SEARCH_END:
		break;
	case sdk_mgr_ezviz::INS_RECORD_SEARCH_FAILED:
		//pInstance->insRecordSearchFailed(iErrorCode, pMessageInfo);
		break;
	case sdk_mgr_ezviz::INS_PTZCTRL_SUCCESS:
		break;
	case sdk_mgr_ezviz::INS_PTZCTRL_FAILED:
		break;
	default:
		break;
	}

	sInfo.Format(L"msg_type=%d\r\nErrorCode = %d\r\nErrorMsg=%s",
				 msg->iMsgType, msg->iErrorCode, utf8::a2w(msg->messageInfo).c_str());
	JLOG(sInfo);
}


void CVideoPlayerDlg::on_ins_play_start(const record_ptr& record)
{
	if (record && !record->started_) {
		player_op_bring_player_to_front(record->player_);
		InsertList(record);
		record->started_ = true;
		auto device = std::dynamic_pointer_cast<video::ezviz::video_device_info_ezviz>(record->_device);
		auto zoneUuid = record->_zone;

		CString txt;
		txt.Format(L"%s([%d,%s]%s)-\"%s\"", 
				   GetStringFromAppResource(IDS_STRING_VIDEO_START),
				   device->get_id(),
				   device->get_device_note().c_str(),
				   utf8::a2w(device->get_deviceSerial()).c_str(),
				   record->_param->_file_path);

		auto hr = core::history_record_manager::get_instance();
		hr->InsertRecord(zoneUuid._ademco_id, zoneUuid._zone_value,
						 txt, time(nullptr), core::RECORD_LEVEL_VIDEO);
	}
}


void CVideoPlayerDlg::on_ins_play_stop(record_ptr record)
{
	if (record) {
		auto device = std::dynamic_pointer_cast<video::ezviz::video_device_info_ezviz>(record->_device);
		auto zoneUuid = record->_zone;

		CString txt;
		txt.Format(L"%s([%d,%s]%s)-\"%s\"",
				   GetStringFromAppResource(IDS_STRING_VIDEO_STOP),
				   device->get_id(),
				   device->get_device_note().c_str(),
				   utf8::a2w(device->get_deviceSerial()).c_str(),
				   (record->_param->_file_path));
		
		auto hr = core::history_record_manager::get_instance();
		hr->InsertRecord(zoneUuid._ademco_id, zoneUuid._zone_value,
						 txt, time(nullptr), core::RECORD_LEVEL_VIDEO);

		std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_);
		record_list_.remove(record);
		player_op_recycle_player(record->player_);
		delete_from_play_list_by_record(record);
		auto mgr = video::ezviz::sdk_mgr_ezviz::get_instance();
		mgr->FreeSession(record->_param->_session_id);

		bool can_re_play = false;
		if (record->e45_occured_) {
			if (!record->verified_hd_) {
				auto user = std::dynamic_pointer_cast<video::ezviz::video_user_info_ezviz>(record->_device->get_userInfo());
				if (do_hd_verify(user)) {
					can_re_play = true;
					record->verified_hd_ = true;
				} else {
					record->verified_hd_ = false;
				}
			}
			record->e45_occured_ = false;
		}

		if (can_re_play) {
			record = nullptr;
			PlayVideoByDevice(record->_device, util::CConfigHelper::get_instance()->get_default_video_level());
		}
	}
}


void CVideoPlayerDlg::on_ins_play_exception(const ezviz_msg_ptr& msg, const record_ptr& record)
{
	CString e, sInfo = L""; 
	if (record) {
		sInfo.Format(L"%s-%s\r\n", record->_device->get_userInfo()->get_user_name().c_str(), record->_device->get_device_note().c_str());
	}
	sInfo.AppendFormat(L"ErrorCode = %d", msg->iErrorCode);

	switch (msg->iErrorCode) {
	case INS_ERROR_V17_PERMANENTKEY_EXCEPTION:
		e = GetStringFromAppResource(IDS_STRING_VERIFY_CODE_WRONG);
		break;

	case  INS_ERROR_CASLIB_PLATFORM_CLIENT_REQUEST_NO_PU_FOUNDED:
		e = GetStringFromAppResource(IDS_STRING_DEVICE_OFFLINE);
		break;

	/*case INS_ERROR_CASLIB_PLATFORM_CLIENT_NO_SIGN_RELEATED:
	{
		JLOG(sInfo);
	}
	break;*/

	case INS_ERROR_OPERATIONCODE_FAILED:
		e = GetStringFromAppResource(IDS_STRING_DEVICE_OFFLINE);
		//record->e45_occured_ = true;
		//return;
		break;

	case INS_ERROR_V17_VTDU_TIMEOUT:
	case INS_ERROR_V17_VTDU_STOP:
		e = GetStringFromAppResource(IDS_STRING_VTDU_TIMEOUT);
		break;

	case NS_ERROR_PRIVATE_VTDU_DISCONNECTED_LINK:
		e = GetStringFromAppResource(IDS_STRING_VTDU_DISCONNECTED_LINK);
		break;

	case NS_ERROR_PRIVATE_VTDU_STATUS_404:
		e = GetStringFromAppResource(IDS_STRING_VTDU_STATUS_404);
		break;

	case NS_ERROR_PRIVATE_VTDU_STATUS_405:
		e = GetStringFromAppResource(IDS_STRING_VTDU_STATUS_405);
		break;

	case NS_ERROR_PRIVATE_VTDU_STATUS_406:
		e = GetStringFromAppResource(IDS_STRING_VTDU_STATUS_406);
		break;

	case NS_ERROR_PRIVATE_VTDU_STATUS_452:
		e = GetStringFromAppResource(IDS_STRING_VTDU_STATUS_452);
		break;

	case NS_ERROR_PRIVATE_VTDU_STATUS_454:
		e = GetStringFromAppResource(IDS_STRING_VTDU_STATUS_454);
		break;

	case NS_ERROR_PRIVATE_VTDU_STATUS_491:
		e = GetStringFromAppResource(IDS_STRING_VTDU_STATUS_491);
		break;

	default:
		e = utf8::a2w(msg->messageInfo).c_str();
		break;
	}

	/*if (record) {
		on_ins_play_stop(record);
	} else {*/
		auto mgr = video::ezviz::sdk_mgr_ezviz::get_instance();
		mgr->m_dll.stopRealPlay(msg->sessionId);
	//}

	sInfo.AppendFormat(L"\r\n%s", e);
	MessageBox(sInfo, GetStringFromAppResource(IDS_STRING_PLAY_EXCEPTION), MB_ICONINFORMATION);
	
	//if (INS_ERROR_OPERATIONCODE_FAILED == msg->iErrorCode) {
		
		//if (!record->verified_hd_) {
		//	auto user = std::dynamic_pointer_cast<video::ezviz::video_user_info_ezviz>(record->_device->get_userInfo());
		//	if (do_hd_verify(user)) {
		//		//PlayVideoByDevice(record->_device, util::CConfigHelper::get_instance()->get_default_video_level());
		//		record->verified_hd_ = true;
		//	} else {
		//		record->verified_hd_ = false;
		//	}
		//}
	//} 

}
#pragma endregion


#pragma region jovision callbacks

#define WM_JC_SDKMSG				(WM_USER + 0x010)
#define WM_JC_GETPICTURE			(WM_USER + 0x011)
#define WM_JC_GETRECFILELIST		(WM_USER + 0x012)
#define WM_JC_RESETSTREAM			(WM_USER + 0x013)

void funJCEventCallback(JCLink_t nLinkID, JCEventType etType, DWORD_PTR pData1, DWORD_PTR pData2, LPVOID pUserData)
{
	if (g_player) {
		g_player->EnqueJovisionMsg(std::make_shared<CVideoPlayerDlg::jovision_msg>(nLinkID, etType, pData1, pData2, pUserData));
	}
}

void funJCDataCallback(JCLink_t nLinkID, PJCStreamFrame pFrame, LPVOID pUserData)
{
	/*char acBuffer[32];
	sprintf(acBuffer, "Type:%d\n", pFrame->sType);
	OutputDebugStringA(acBuffer);*/
}

void funJCRawDataCallback(JCLink_t nLinkID, PJCRawFrame pFrame, LPVOID pUserData)
{
}

void funLanSearchCallback(PJCLanDeviceInfo pDevice)
{
	/*if (pDevice == NULL) {
		PostMessage(g_hFindDeviceWnd, WM_REFLASHDEVICELIST, 0, 0);
	} else {
		g_DeviceInfosList.push_back(*pDevice);
	}*/
}


#pragma endregion

void CVideoPlayerDlg::OnCurUserChangedResult(const core::user_info_ptr& user)
{
	assert(user);
	if (user->get_user_priority() == core::UP_OPERATOR) {

	} else {

	}
}

// CVideoPlayerDlg dialog
CVideoPlayerDlg* g_videoPlayerDlg = nullptr;
IMPLEMENT_DYNAMIC(CVideoPlayerDlg, CDialogEx)

CVideoPlayerDlg::CVideoPlayerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CVideoPlayerDlg::IDD, pParent)
	, m_bInitOver(FALSE)
	, record_list_()
	, lock_4_record_list_()
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
	DDX_Control(pDX, IDC_STATIC_SPEED, m_groupSpeed);
	DDX_Control(pDX, IDC_STATIC_PTZ, m_groupPtz);
	DDX_Control(pDX, IDC_STATIC_CONTROL, m_groupControl);
	DDX_Control(pDX, IDC_LIST1, m_ctrl_play_list);
	DDX_Control(pDX, IDC_EDIT_MINUTE, m_ctrl_rerord_minute);
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
	DDX_Control(pDX, IDC_STATIC_CUR_VIDEO, m_static_group_cur_video);
	DDX_Control(pDX, IDC_SLIDER_VOLUME, m_slider_volume);
	DDX_Control(pDX, IDC_STATIC_VLUME, m_static_volume);
	DDX_Control(pDX, IDC_BUTTON_VOICE_TALK, m_btn_voice_talk);
	DDX_Control(pDX, IDC_CHECK_VOLUME, m_chk_volume);
	DDX_Control(pDX, IDC_STATIC_VOICE_TALK, m_group_voice_talk);
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
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CVideoPlayerDlg::OnNMDblclkList1)
	ON_EN_CHANGE(IDC_EDIT_MINUTE, &CVideoPlayerDlg::OnEnChangeEditMinute)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_RADIO_SMOOTH2, &CVideoPlayerDlg::OnBnClickedRadioSmooth2)
	ON_BN_CLICKED(IDC_RADIO_BALANCE2, &CVideoPlayerDlg::OnBnClickedRadioBalance2)
	ON_BN_CLICKED(IDC_RADIO_HD2, &CVideoPlayerDlg::OnBnClickedRadioHd2)
	ON_BN_CLICKED(IDC_RADIO_1_VIDEO, &CVideoPlayerDlg::OnBnClickedRadio1Video)
	ON_BN_CLICKED(IDC_RADIO_4_VIDEO, &CVideoPlayerDlg::OnBnClickedRadio4Video)
	ON_BN_CLICKED(IDC_RADIO_9_VIDEO, &CVideoPlayerDlg::OnBnClickedRadio9Video)
	ON_BN_CLICKED(IDC_BUTTON_VOICE_TALK, &CVideoPlayerDlg::OnBnClickedButtonVoiceTalk)
	ON_NOTIFY(TRBN_THUMBPOSCHANGING, IDC_SLIDER_VOLUME, &CVideoPlayerDlg::OnTRBNThumbPosChangingSliderVolume)
	ON_BN_CLICKED(IDC_CHECK_VOLUME, &CVideoPlayerDlg::OnBnClickedCheckVolume)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_VOLUME, &CVideoPlayerDlg::OnNMReleasedcaptureSliderVolume)
END_MESSAGE_MAP()


// CVideoPlayerDlg message handlers


BOOL CVideoPlayerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	GetWindowText(m_title);
	g_player = this;

	auto videoMgr = video::video_manager::get_instance();
	videoMgr->LoadFromDB();
	
	auto jov = jovision::sdk_mgr_jovision::get_instance();
	if (!jov->init_sdk(-1)) {
		MessageBox(GetStringFromAppResource(IDS_STRING_INIT_JOVISION_SDK_FAILED), L"Error", MB_ICONERROR);
		QuitApplication(0);
		return TRUE;
	}

	jov->register_call_back(funJCEventCallback,
							funJCDataCallback,
							funJCRawDataCallback,
							funLanSearchCallback);


	//GetWindowRect(m_rcNormal);
	//m_player.GetWindowRect(m_rcNormalPlayer);

	m_radioSmooth.SetCheck(1);
	EnableControlPanel(0, 0);

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
	fm = GetStringFromAppResource(IDS_STRING_PRODUCTOR);
	m_ctrl_play_list.InsertColumn(++ndx, fm, LVCFMT_LEFT, 100, -1);

	fm.Format(L"%d", util::CConfigHelper::get_instance()->get_back_end_record_minutes());
	m_ctrl_rerord_minute.SetWindowTextW(fm);

	auto mgr = core::user_manager::get_instance();
	auto user = mgr->GetCurUserInfo();
	OnCurUserChangedResult(user);
	m_cur_user_changed_observer = std::make_shared<CurUserChangedObserver>(this);
	mgr->register_observer(m_cur_user_changed_observer);

	CRect rc;
	m_player.GetWindowRect(rc);
	ScreenToClient(rc);
	const int same_time_play_vidoe_route_count = util::CConfigHelper::get_instance()->get_show_video_same_time_route_count();
	for (int i = 0; i < same_time_play_vidoe_route_count; i++) {
		auto a_player_ex = std::make_shared<player_ex>();
		a_player_ex->player = std::shared_ptr<CVideoPlayerCtrl>(new CVideoPlayerCtrl(), player_deleter);
		a_player_ex->player->ndx_ = i + 1;
		a_player_ex->player->Create(nullptr, m_dwPlayerStyle, rc, this, IDC_STATIC_PLAYER);
		player_ex_vector_[i] = (a_player_ex);
	}

	m_bInitOver = TRUE;

	LoadPosition();
	
	m_chk_1_video.SetCheck(same_time_play_vidoe_route_count == 1);
	m_chk_4_video.SetCheck(same_time_play_vidoe_route_count == 4);
	m_chk_9_video.SetCheck(same_time_play_vidoe_route_count == 9);
	player_op_set_same_time_play_video_route(same_time_play_vidoe_route_count);

	m_btn_voice_talk.EnableWindow(0);
	m_chk_volume.SetCheck(0);
	m_chk_volume.EnableWindow(0);
	m_slider_volume.SetRange(0, 100, 1);
	m_slider_volume.EnableWindow(0);

	SetTimer(TIMER_ID_EZVIZ_MSG, 1000, nullptr);
	SetTimer(TIMER_ID_REC_VIDEO, 2000, nullptr);
	SetTimer(TIMER_ID_PLAY_VIDEO, 1000, nullptr);
	SetTimer(TIMER_ID_JOVISION_MSG, 1000, nullptr);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


CVideoPlayerDlg::record_ptr CVideoPlayerDlg::record_op_get_record_info_by_device(const video::video_device_info_ptr& device) {
	record_ptr rec_info = nullptr;
	std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_);
	for (auto info : record_list_) {
		if (info->productor_ == video::EZVIZ) {
			if (info->_device == std::dynamic_pointer_cast<video::ezviz::video_device_info_ezviz>(device)) {
				rec_info = info;
				break;
			}
		} else if (info->productor_ == video::JOVISION) {
			if (info->_device == std::dynamic_pointer_cast<video::jovision::video_device_info_jovision>(device)) {
				rec_info = info;
				break;
			}
		} else {
			assert(0);
		}
	}
	return rec_info;
}


CVideoPlayerDlg::record_ptr CVideoPlayerDlg::record_op_get_record_info_by_player(const player& player)
{
	record_ptr rec_info = nullptr;
	std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_);
	for (auto info : record_list_) {
		if (info->player_ == player) {
			rec_info = info;
			break;
		}
	}
	return rec_info;
}

CVideoPlayerDlg::record_ptr CVideoPlayerDlg::record_op_get_record_info_by_link_id(int link_id)
{
	record_ptr rec_info = nullptr;
	std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_);
	for (auto info : record_list_) {
		if (info->productor_ == JOVISION && info->link_id_ == link_id) {
			rec_info = info;
			break;
		}
	}
	return rec_info;
}


bool CVideoPlayerDlg::record_op_is_valid(DataCallbackParamEzviz* param)
{
	//AUTO_LOG_FUNCTION;
	std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_);
	for (auto info : record_list_) {
		if (info->_param == param) {
			return true;
		}
	}
	return false;
}


void CVideoPlayerDlg::player_op_set_same_time_play_video_route(const int n)
{
	CRect rc;
	m_player.GetWindowRect(rc);
	ScreenToClient(rc);

	auto cfg = util::CConfigHelper::get_instance();
	const int prev_count = cfg->get_show_video_same_time_route_count();
	cfg->set_show_video_same_time_route_count(n);

	auto v = split_rect(rc, n);

	for (int i = 0; i < n && i < prev_count; i++) {
		player_ex_vector_[i]->player->MoveWindow(v[i]);
		player_ex_vector_[i]->player->ShowWindow(SW_SHOW);
		player_ex_vector_[i]->rc = v[i];
	}

	if (prev_count < n) {
		for (int i = prev_count; i < n; i++) {
			auto a_player_ex = std::make_shared<player_ex>();
			if (!back_end_players_.empty()) {
				a_player_ex->player = *back_end_players_.begin();
				back_end_players_.erase(a_player_ex->player);
				a_player_ex->used = true;
				InsertList(record_op_get_record_info_by_player(a_player_ex->player));
			} else {
				a_player_ex->player = player_op_create_new_player();
				a_player_ex->used = false;
			}
			
			a_player_ex->player->MoveWindow(v[i]);
			a_player_ex->player->ShowWindow(SW_SHOW);
			
			player_ex_vector_[i] = (a_player_ex);
		}

	} else {
		for (int i = n; i < prev_count; i++) {
			auto player = player_ex_vector_[i]->player;
			player->ShowWindow(SW_HIDE);
			delete_from_play_list_by_record(record_op_get_record_info_by_player(player));
		}

		while (player_ex_vector_.size() > (size_t)n) {
			player_ex_vector_[player_ex_vector_.size() - 1]->player->MoveWindow(CRect(0, 0, 1, 1));
			if (player_ex_vector_[player_ex_vector_.size() - 1]->used) {
				back_end_players_.insert(player_ex_vector_[player_ex_vector_.size() - 1]->player);
			} else {
				player_buffer_.push_back(player_ex_vector_[player_ex_vector_.size() - 1]->player);
			}
			player_ex_vector_.erase(player_ex_vector_.size() - 1);
		}
	}

	player_op_rebuild();
}


void CVideoPlayerDlg::LoadPosition()
{
	AUTO_LOG_FUNCTION;
	auto cfg = util::CConfigHelper::get_instance();

	do{
		CRect rect = cfg->get_rectVideoPlayerDlg();
		if (rect.IsRectNull() || rect.IsRectEmpty()) {
			break;
		}
		int m = cfg->get_maximizedVideoPlayerDlg();
		CRect rc;
		GetWindowRect(rc);
		rect.right = rect.left + rc.Width();
		rect.bottom = rect.top + rc.Height();
		MoveWindow(rect);
		GetWindowPlacement(&m_rcNormal);
		m_player.GetWindowPlacement(&m_rcNormalPlayer);
		maximized_ = m;
		OnInversioncontrol(1, 0);

	} while (0);
}


void CVideoPlayerDlg::SavePosition()
{
	CRect rect;
	GetWindowRect(rect);

	auto cfg = util::CConfigHelper::get_instance();
	cfg->set_rectVideoPlayerDlg(rect);
	cfg->set_maximizedVideoPlayerDlg(maximized_);
}

void CVideoPlayerDlg::OnBnClickedOk()
{
	return;
}


void CVideoPlayerDlg::OnBnClickedCancel()
{
	ShowWindow(SW_HIDE);
}


void CVideoPlayerDlg::OnMove(int x, int y)
{
	CDialogEx::OnMove(x, y);

	if (m_bInitOver) {
		SavePosition();

		player_op_rebuild();
	}
}


void CVideoPlayerDlg::ShowOtherCtrls(BOOL bShow)
{
	int sw = bShow ? SW_SHOW : SW_HIDE;
	m_static_group_cur_video.ShowWindow(sw);
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

	m_group_video_list.ShowWindow(sw);
	m_ctrl_play_list.ShowWindow(sw);
	m_group_record_settings.ShowWindow(sw);
	m_static_note.ShowWindow(sw);
	m_ctrl_rerord_minute.ShowWindow(sw);
	m_static_minute.ShowWindow(sw);
	m_staticNote2.ShowWindow(sw);
	m_radioGlobalSmooth.ShowWindow(sw);
	m_radioGlobalBalance.ShowWindow(sw);
	m_radioGlobalHD.ShowWindow(sw);

	m_chk_1_video.ShowWindow(sw);
	m_chk_4_video.ShowWindow(sw);
	m_chk_9_video.ShowWindow(sw);

	m_group_voice_talk.ShowWindow(sw);
	m_btn_voice_talk.ShowWindow(sw);
	m_chk_volume.ShowWindow(sw);
	m_static_volume.ShowWindow(sw);
	m_slider_volume.ShowWindow(sw);

}


void CVideoPlayerDlg::EnableControlPanel(BOOL bAble, int level)
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
		if(!wParam) // 1 for local, don't change maximized_; 0 for player, change it.
			maximized_ = !maximized_;
		if (maximized_) {
			ShowOtherCtrls(0);
			HMONITOR hMonitor = MonitorFromWindow(GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
			MONITORINFO mi = { 0 }; mi.cbSize = sizeof(mi);
			GetMonitorInfo(hMonitor, &mi);
			m_rcFullScreen.length = sizeof(m_rcFullScreen);
			m_rcFullScreen.flags = 0;
			m_rcFullScreen.showCmd = SW_SHOWNORMAL;
			m_rcFullScreen.rcNormalPosition = mi.rcMonitor;
			SetWindowPlacement(&m_rcFullScreen);

			CRect rc;
			GetClientRect(rc);
			m_player.MoveWindow(rc);
			m_player.ShowWindow(SW_HIDE);

		} else {
			ShowOtherCtrls(1);
			SetWindowPlacement(&m_rcNormal);
			m_player.SetWindowPlacement(&m_rcNormalPlayer);
			m_player.ShowWindow(SW_HIDE);
		}

		Invalidate();

		player_op_update_players_size_with_m_player();

		SavePosition();
	}
	return 0;
}


void CVideoPlayerDlg::PlayVideoByDevice(video::video_device_info_ptr device, int speed)
{
	AUTO_LOG_FUNCTION;
	ShowWindow(SW_SHOWNORMAL);
	assert(device);
	if (!device) {
		return;
	}

	video_user_info_ptr user = device->get_userInfo(); assert(user);
	auto productor = user->get_productorInfo().get_productor();
	switch (productor) {
	case video::EZVIZ:
		PlayVideoEzviz(std::dynamic_pointer_cast<video::ezviz::video_device_info_ezviz>(device), speed);
		break;
	case video::JOVISION:
		PlayVideoJovision(std::dynamic_pointer_cast<video::jovision::video_device_info_jovision>(device), speed);
		break;
	default:
		assert(0);
		break;
	}
}


void CVideoPlayerDlg::StopPlayCurselVideo()
{
	AUTO_LOG_FUNCTION;
	if (m_curPlayingDevice) {
		EnableControlPanel(0, 0);
		auto productor = m_curPlayingDevice->get_userInfo()->get_productorInfo().get_productor();
		if (productor == EZVIZ) {
			StopPlayEzviz(std::dynamic_pointer_cast<video::ezviz::video_device_info_ezviz>(m_curPlayingDevice));
		} else if (productor == JOVISION) {
			StopPlayJovision(std::dynamic_pointer_cast<video::jovision::video_device_info_jovision>(m_curPlayingDevice));
		} else {
			assert(0);
		}
		
		m_curPlayingDevice = nullptr;
	}
}


bool CVideoPlayerDlg::do_hd_verify(const video::ezviz::video_user_info_ezviz_ptr& user)
{
	AUTO_LOG_FUNCTION;
	assert(user);

	auto pack_0 = [](char* s) {
		while (isalpha(*s) || isdigit(*s)) {
			*s++;
		}
		*s = 0;
	};

	auto get_result = [](char* s) {
		int ret = 0;
		if (sscanf(s, "{\"result\":{\"code\":\"%d\"", &ret) == 1) {
			return ret;
		}
		return 0;
	};

	bool ok = false;
	auto mgr = video::ezviz::sdk_mgr_ezviz::get_instance();
	do {
		char reqStr[1024] = { 0 };
		sprintf_s(reqStr, SMSCODE_SECURE_REQ, user->get_acc_token().c_str());
		JLOGA("sending req:%s", reqStr);
		char* pOutStr = nullptr;
		int iLen = 0;
		int ret = mgr->m_dll.RequestPassThrough(reqStr, &pOutStr, &iLen);
		if (ret != 0) {
			JLOGA("RequestPassThrough %d", ret);
			break;
		}
		//pOutStr[iLen] = 0;
		//pack_0(pOutStr);
		JLOGA("iLen %d");
		JLOGA(pOutStr);
		/*std::string json = pOutStr;
		mgr->m_dll.freeData(pOutStr);


		Json::Reader reader;
		Json::Value	value;
		if (!reader.parse(json.c_str(), value)) {
			JLOGA("get sms code parse Json failed! json:\n%s", json.c_str());
			break;
		}
		Json::Value result = value["result"];
		int iResult = 0;
		if (result["code"].isString()) {
			iResult = atoi(result["code"].asString().c_str());
		} else if (result["code"].isInt()) {
			iResult = result["code"].asInt();
		}*/
		int iResult = get_result(pOutStr);
		JLOGA("get sms code http result %d", iResult);
		if (200 == iResult) {
			ok = true;
		} else if (1041 == iResult) {
			MessageBox(GetStringFromAppResource(IDS_STRING_OP_IS_TO_FAST), L"", MB_ICONINFORMATION);
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
		std::string verify_code = utf8::w2a((LPCTSTR)dlg.m_edit);

		char reqStr[1024] = { 0 };
		sprintf_s(reqStr, SECUREVALIDATE_REQ, verify_code.c_str(), user->get_acc_token().c_str());
		JLOGA("sending req:%s", reqStr);
		char* pOutStr = nullptr;
		int iLen = 0;
		int ret = mgr->m_dll.RequestPassThrough(reqStr, &pOutStr, &iLen);
		if (ret != 0) {
			JLOG(L"RequestPassThrough %d", ret);
			break;
		}
		pOutStr[iLen] = 0;
		//pack_0(pOutStr);
		JLOGA("iLen %d");
		JLOGA(pOutStr);
		/*std::string json = pOutStr;
		mgr->m_dll.freeData(pOutStr);

		Json::Reader reader;
		Json::Value	value;
		if (!reader.parse(json.c_str(), value)) {
			JLOGA("verify sms code parse Json failed! json:%s", json.c_str());
			break;
		}
		Json::Value result = value["result"];
		int iResult = 0;
		if (result["code"].isString()) {
			iResult = atoi(result["code"].asString().c_str());
		} else if (result["code"].isInt()) {
			iResult = result["code"].asInt();
		}*/
		int iResult = get_result(pOutStr);
		JLOGA("verify sms code http result %d", iResult);
		if (200 == iResult) {
			ok = true;
		} else {
			MessageBox(GetStringFromAppResource(IDS_STRING_SMS_CODE_ERR), L"", MB_ICONERROR);
			break;
		}
	} while (0);

	return ok;
}

void CVideoPlayerDlg::on_jov_play_start(const record_ptr & record)
{
	if (record && !record->started_) {
		player_op_bring_player_to_front(record->player_);
		InsertList(record);
		record->started_ = true;
		auto device = std::dynamic_pointer_cast<video::jovision::video_device_info_jovision>(record->_device);
		auto zoneUuid = record->_zone;

		CString txt;
		txt.Format(L"%s([%d,%s]%s)-\"%s\"",
				   GetStringFromAppResource(IDS_STRING_VIDEO_START),
				   device->get_id(),
				   device->get_device_note().c_str(),
				   utf8::a2w(device->get_sse()).c_str(),
				   record->_param->_file_path);

		auto hr = core::history_record_manager::get_instance();
		hr->InsertRecord(zoneUuid._ademco_id, zoneUuid._zone_value,
						 txt, time(nullptr), core::RECORD_LEVEL_VIDEO);
	}
}

void CVideoPlayerDlg::on_jov_play_stop(const record_ptr & record)
{
	if (record) {
		auto device = std::dynamic_pointer_cast<video::jovision::video_device_info_jovision>(record->_device);
		auto zoneUuid = record->_zone;

		CString txt;
		txt.Format(L"%s([%d,%s]%s)-\"%s\"",
				   GetStringFromAppResource(IDS_STRING_VIDEO_STOP),
				   device->get_id(),
				   device->get_device_note().c_str(),
				   utf8::a2w(device->get_sse()).c_str(),
				   (record->_param->_file_path));

		auto hr = core::history_record_manager::get_instance();
		hr->InsertRecord(zoneUuid._ademco_id, zoneUuid._zone_value,
						 txt, time(nullptr), core::RECORD_LEVEL_VIDEO);

		std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_);
		record_list_.remove(record);
		player_op_recycle_player(record->player_);
		delete_from_play_list_by_record(record);
	}
}

void CVideoPlayerDlg::EnqueJovisionMsg(const jovision_msg_ptr & msg)
{
	AUTO_LOG_FUNCTION;
	std::lock_guard<std::mutex> lock(lock_4_jovision_msg_queue_);
	jovision_msg_list_.push_back(msg);
}

void CVideoPlayerDlg::HandleJovisionMsg(const jovision_msg_ptr & msg)
{
	DWORD dwMsgID = 0;

	std::list<CString> appendix_msg_list = {};

	switch (msg->etType) {
	case JCET_GetFileListOK://获取远程录像成功
	{
		/*g_RecFileInfoList.clear();
		PJCRecFileInfo pInfos = (PJCRecFileInfo)pData1;
		int nCount = (int)pData2;
		for (int i = 0; i < nCount; ++i) {
		g_RecFileInfoList.push_back(pInfos[i]);
		}*/
	}

	case JCET_GetFileListError://获取远程录像失败
	{
		safe_post_msg_to_g_player(WM_JC_GETRECFILELIST, msg->etType == JCET_GetFileListOK);
	}
	return;
	break;

	case JCET_StreamReset://码流重置信号
	{
		sdk_mgr_jovision::get_instance()->enable_decoder(msg->nLinkID, FALSE);
		safe_post_msg_to_g_player(WM_JC_RESETSTREAM);
	}
	return;
	break;
	}

	bool ok = false; // ok for connection established, fail for connection lost
	switch ( msg->etType ) {

	case JCET_ConnectOK://连接成功
		dwMsgID = IDS_ConnectOK;
		{
			auto record = record_op_get_record_info_by_link_id(msg->nLinkID);
			if (record) {
				CString strMsg;
				auto jmgr = sdk_mgr_jovision::get_instance();
				do {
					ok = jmgr->enable_decoder(msg->nLinkID, TRUE);
					if (!ok) {
						strMsg.Format(GetStringFromAppResource(IDS_EnableDecodeError), msg->nLinkID);
						break;
					}

					record->decoding_ = true;
					strMsg.Format(GetStringFromAppResource(IDS_EnableDecodeOK), msg->nLinkID);

					ok = jmgr->set_video_preview(msg->nLinkID, record->player_->GetRealHwnd(), record->player_->GetRealRect());
					if (!ok) {
						strMsg.Format(GetStringFromAppResource(IDS_EnablePreviewError), msg->nLinkID);
						break;
					}

				} while (0);

				if (!ok) {
					record->decoding_ = false;
					record->previewing_ = false;
				}

				appendix_msg_list.push_back(strMsg);
			}
		}
		break;

	case JCET_UserAccessError: //用户验证失败
		dwMsgID = IDS_ConnectAccessError;
		break;

	case JCET_NoChannel://主控通道未开启
		dwMsgID = IDS_ConnectNoChannel;
		break;

	case JCET_ConTypeError://连接类型错误
		dwMsgID = IDS_ConnectTypeError;
		break;

	case JCET_ConCountLimit://超过主控连接最大数
		dwMsgID = IDS_ConnectCountLimit;
		break;

	case JCET_ConTimeout://连接超时
		dwMsgID = IDS_ConnectTimeout;
		break;

	case JCET_DisconOK://断开连接成功
		dwMsgID = IDS_DisconnectOK;
		break;

	case JCET_ConAbout://连接异常断开
		dwMsgID = IDS_DisconnectError;
		break;

	case JCET_ServiceStop://主控断开连接
		dwMsgID = IDS_ServerStop;
		break;

	default:
		return;
		break;
	}

	CString strMsg;
	std::wstring wMsg;
	if (msg->pData1) {
		std::string sMsg = (char*)msg->pData1;
		wMsg = std::wstring(sMsg.begin(), sMsg.end());
	}
	strMsg.Format(GetStringFromAppResource(dwMsgID), msg->nLinkID, wMsg.c_str());
	appendix_msg_list.push_front(strMsg);
	auto hr = core::history_record_manager::get_instance();
	video::zone_uuid zone = { -1,-1,-1 };
	auto record = record_op_get_record_info_by_link_id(msg->nLinkID);
	if (record) {
		zone = record->_zone;
	}

	for (auto s : appendix_msg_list) {
		hr->InsertRecord(zone._ademco_id, zone._zone_value, s, time(nullptr), core::RECORD_LEVEL_VIDEO);
	}

	if (!ok) {
		on_jov_play_stop(record);
	} else {
		on_jov_play_start(record);
	}
}


void CVideoPlayerDlg::PlayVideoEzviz(video::ezviz::video_device_info_ezviz_ptr device, int videoLevel)
{
	AUTO_LOG_FUNCTION;
	assert(device);
	do {
		auto rec_info = record_op_get_record_info_by_device(device);
		if (rec_info) { // playing
			if (videoLevel == rec_info->_level) { // save level
				EnableControlPanel(TRUE, videoLevel);
				rec_info->_param->_start_time = GetTickCount();
				player_op_bring_player_to_front(rec_info->player_);
				m_curPlayingDevice = device;
				return;
			} else { // different level, stop it and re-play it with new level
				StopPlayEzviz(device);
			}
		} else { // not playing
			if (record_list_.size() >= 8) {
				for (auto info : record_list_) {
					if (info->_device != m_curPlayingDevice) {
						StopPlayByRecordInfo(info);
						break;
					}
				}
			}
		}
		
		m_curPlayingDevice = device;
		auto user = std::dynamic_pointer_cast<video::ezviz::video_user_info_ezviz>(device->get_userInfo()); 
		assert(user);
		auto mgr = video::ezviz::sdk_mgr_ezviz::get_instance();
		CString e;
		if (user->get_acc_token().size() == 0) {
			if (video::ezviz::sdk_mgr_ezviz::RESULT_OK != mgr->VerifyUserAccessToken(user, TYPE_GET)) {
				e = GetStringFromAppResource(IDS_STRING_PRIVATE_CLOUD_CONN_FAIL_OR_USER_NOT_EXSIST);
				MessageBox(e, L"", MB_ICONINFORMATION);
				break;
			}
			user->execute_set_acc_token(user->get_acc_token());
		}
		bool bEncrypt = false;
		int ret = mgr->m_dll.UpdateCameraInfo(device->get_cameraId(), user->get_acc_token(), bEncrypt);
		if (ret != 0) {
			e = GetStringFromAppResource(IDS_STRING_UPDATE_CAMERA_INFO_FAILED);
			core::history_record_manager::get_instance()->InsertRecord(-1, 0, e, time(nullptr), core::RECORD_LEVEL_VIDEO);
			MessageBox(e, L"", MB_ICONINFORMATION);
			break;
		}

		if (bEncrypt && device->get_secure_code().size() != 6) {
			CInputDeviceVerifyCodeDlg dlg(this);
			if (dlg.DoModal() != IDOK) {
				break;
			}
			device->set_secure_code(utf8::w2a((LPCTSTR)dlg.m_result));
			device->execute_update_info();
		}
		std::string session_id = mgr->GetSessionId(user->get_user_phone(), device->get_cameraId(), messageHandler, this);
		if (session_id.empty()) {
			assert(0);
			break;
		}
		DataCallbackParamEzviz *param = new DataCallbackParamEzviz(this, session_id, /*time(nullptr)*/ GetTickCount());
		CString filePath = param->FormatFilePath(device->get_userInfo()->get_id(), device->get_userInfo()->get_user_name(),
												 device->get_id(), device->get_device_note());
		mgr->m_dll.setDataCallBack(session_id, videoDataHandler, param);

		auto player = player_op_create_new_player();
		{
			ret = mgr->m_dll.startRealPlay(session_id,
										   player->GetRealHwnd(),
										   device->get_cameraId(),
										   user->get_acc_token(),
										   device->get_secure_code(),
										   util::CConfigHelper::get_instance()->get_ezviz_private_cloud_app_key(),
										   videoLevel/*, &msg*/);
		}

		if (ret == 20005 || ret == OPEN_SDK_IDENTIFY_FAILED) { // 硬件特征码校验失败，需重新进行认证
			if (do_hd_verify(user)) {
				video::ezviz::sdk_mgr_ezviz::NSCBMsg msg;
				msg.pMessageInfo = nullptr;
				ret = mgr->m_dll.startRealPlay(session_id,
											   player->m_hWnd,
											   device->get_cameraId(),
											   user->get_acc_token(),
											   device->get_secure_code(),
											   util::CConfigHelper::get_instance()->get_ezviz_private_cloud_app_key(),
											   videoLevel,
											   &msg);

				if (ret != 0) {
					auto emsg = std::make_shared<ezviz_msg>();
					emsg->iErrorCode = msg.iErrorCode;
					emsg->messageInfo = msg.pMessageInfo ? msg.pMessageInfo : "";
					emsg->iMsgType = video::ezviz::sdk_mgr_ezviz::INS_PLAY_EXCEPTION;
					emsg->sessionId = session_id;
					HandleEzvizMsg(emsg);
				}
			}
		}

		if (ret != 0) {
			JLOG(L"startRealPlay failed %d\n", ret);
			m_curPlayingDevice = nullptr;
			SAFEDELETEP(param);
			player_buffer_.push_back(player);
		} else {
			JLOG(L"PlayVideo ok\n");
			EnableControlPanel(TRUE, videoLevel);
			std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_);
			video::zone_uuid zoneUuid = device->GetActiveZoneUuid();			
			record_ptr info = std::make_shared<record>(param, zoneUuid, device, player, videoLevel);
			record_list_.push_back(info);
		}
		UpdateWindow();
		return;
	} while (0);
	JLOG(L"PlayVideo failed\n");
	m_curPlayingDevice = nullptr;
	UpdateWindow();
}


void CVideoPlayerDlg::PlayVideoJovision(video::jovision::video_device_info_jovision_ptr device, int videoLevel)
{
	AUTO_LOG_FUNCTION;
	assert(device);
	do {
		auto rec_info = record_op_get_record_info_by_device(device);
		if (rec_info) { // playing
			if (rec_info->connecting_) { // last time previewing not done.
				return;
			}

			if (videoLevel == rec_info->_level) { // save level
				EnableControlPanel(TRUE, videoLevel);
				rec_info->_param->_start_time = GetTickCount();
				player_op_bring_player_to_front(rec_info->player_);
				m_curPlayingDevice = device;
				return;
			} else { // different level, stop it and re-play it with new level
				StopPlayJovision(device);
			}
		} else { // not playing
			if (record_list_.size() >= 8) {
				for (auto info : record_list_) {
					if (info->_device != m_curPlayingDevice) {
						StopPlayByRecordInfo(info);
						break;
					}
				}
			}
		}

		m_curPlayingDevice = device;
		auto user = std::dynamic_pointer_cast<video::jovision::video_user_info_jovision>(device->get_userInfo()); assert(user);
		
		auto jov = jovision::sdk_mgr_jovision::get_instance();
		jovision::JCLink_t link_id = -1;

		if (device->get_by_sse()) {
			link_id = jov->connect(const_cast<char*>(device->get_sse().c_str()), 0, 1,
								   const_cast<char*>(utf8::w2a(device->get_user_name()).c_str()),
								   const_cast<char*>(device->get_user_passwd().c_str()),
								   1, nullptr);
		} else {
			link_id = jov->connect(const_cast<char*>(device->get_ip().c_str()), device->get_port(), 1,
								   const_cast<char*>(utf8::w2a(device->get_user_name()).c_str()),
								   const_cast<char*>(device->get_user_passwd().c_str()),
								   1, nullptr);
		}

		video::zone_uuid zoneUuid = device->GetActiveZoneUuid();
		if (link_id == -1) {
			JLOG(L"startRealPlay failed link_id %d\n", link_id);
			m_curPlayingDevice = nullptr;
			//SAFEDELETEP(param);
			//player_buffer_.push_back(player);
			core::history_record_manager::get_instance()->InsertRecord(zoneUuid._ademco_id, zoneUuid._zone_value, 
																	   GetStringFromAppResource(IDS_ConnectError), 
																	   time(nullptr), core::RECORD_LEVEL_VIDEO);
		} else {
			JLOG(L"PlayVideo ok\n");
			EnableControlPanel(TRUE, videoLevel);
			std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_);
			DataCallbackParamEzviz *param = new DataCallbackParamEzviz(this, "", GetTickCount());
			record_ptr info = std::make_shared<record>(param, zoneUuid, device, player_op_create_new_player(), videoLevel);
			info->connecting_ = true;
			info->link_id_ = link_id;
			record_list_.push_back(info);

			CString strMsg;
			strMsg.Format(GetStringFromAppResource(IDS_Connecting), link_id);
			core::history_record_manager::get_instance()->InsertRecord(zoneUuid._ademco_id, zoneUuid._zone_value,
																	   strMsg, time(nullptr), core::RECORD_LEVEL_VIDEO);

		}
		UpdateWindow();
		return;
	} while (0);
	JLOG(L"PlayVideo failed\n");
	m_curPlayingDevice = nullptr;
	UpdateWindow();
}


void CVideoPlayerDlg::StopPlayEzviz(video::ezviz::video_device_info_ezviz_ptr device)
{
	AUTO_LOG_FUNCTION;
	assert(device);
	std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_);
	auto user = std::dynamic_pointer_cast<video::ezviz::video_user_info_ezviz>(device->get_userInfo()); assert(user);
	auto mgr = video::ezviz::sdk_mgr_ezviz::get_instance();
	
	std::string session_id = mgr->GetSessionId(user->get_user_phone(), device->get_cameraId(), messageHandler, this);
	if (session_id.empty()) {
		assert(0);
		auto record = record_op_get_record_info_by_device(device);
		on_ins_play_stop(record);
	} else {
		mgr->m_dll.setDataCallBack(session_id, videoDataHandler, nullptr);
		mgr->m_dll.stopVoiceTalk(session_id);
		mgr->m_dll.closeSound(session_id);
		//video::ezviz::sdk_mgr_ezviz::NSCBMsg msg;
		//msg.pMessageInfo = nullptr;
		int ret = mgr->m_dll.stopRealPlay(session_id/*, &msg*/);
		JLOGA("stopRealPlay ret %d", ret);
		/*if (ret != 0) {
			auto record = record_op_get_record_info_by_device(device);
			on_ins_play_stop(record);
		} else {
			auto record = record_op_get_record_info_by_device(device);
			on_ins_play_stop(record);
		}*/
	}
}


void CVideoPlayerDlg::StopPlayJovision(video::jovision::video_device_info_jovision_ptr device)
{
	AUTO_LOG_FUNCTION;
	assert(device);
	std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_);
	auto record = record_op_get_record_info_by_device(device);
	if (record) {
		sdk_mgr_jovision::get_instance()->disconnect(record->link_id_);
	}
}


void CVideoPlayerDlg::delete_from_play_list_by_record(const record_ptr& record)
{
	if (!record)return;

	for (int i = 0; i < m_ctrl_play_list.GetItemCount(); i++) {
		play_list_item_data* data = reinterpret_cast<play_list_item_data*>(m_ctrl_play_list.GetItemData(i));
		if (data->productor == record->_device->get_userInfo()->get_productorInfo().get_productor() 
			&& data->dev_id == record->_device->get_id()) {
			m_static_group_cur_video.SetWindowTextW(L"");
			m_ctrl_play_list.DeleteItem(i);
			delete data;
			break;
		}
	}
}


void CVideoPlayerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	UnregisterHotKey(GetSafeHwnd(), HOTKEY_PTZ);

	/*for (auto info : record_list_) {
		StopPlayEzviz(info->_device);
	}*/



	MSG msg;
	while (GetMessage(&msg, m_hWnd, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (record_list_.empty())
			break;
		else {
			auto info = record_list_.front();
			StopPlayByRecordInfo(info);
		}
	}
	
	KillTimer(TIMER_ID_EZVIZ_MSG);
	KillTimer(TIMER_ID_REC_VIDEO);
	KillTimer(TIMER_ID_PLAY_VIDEO);
	KillTimer(TIMER_ID_JOVISION_MSG);

	record_list_.clear();
	m_wait2playDevList.clear();

	g_player = nullptr;

	player_ex_vector_.clear();
	back_end_players_.clear();
	player_buffer_.clear();

	for (int i = 0; i < m_ctrl_play_list.GetItemCount(); i++) {
		play_list_item_data* data = reinterpret_cast<play_list_item_data*>(m_ctrl_play_list.GetItemData(i));
		delete data;
	}
}


void CVideoPlayerDlg::OnTimer(UINT_PTR nIDEvent)
{
	//AUTO_LOG_FUNCTION;
	if (TIMER_ID_EZVIZ_MSG == nIDEvent) {
		auto_timer timer(m_hWnd, TIMER_ID_EZVIZ_MSG, 1000);
		ezviz_msg_ptr msg = nullptr;
		if (lock_4_ezviz_msg_queue_.try_lock()) {
			std::lock_guard<std::mutex> lock(lock_4_ezviz_msg_queue_, std::adopt_lock);
			if (ezviz_msg_list_.size() > 0) {
				msg = ezviz_msg_list_.front();
				ezviz_msg_list_.pop_front();
			}
		}
		if (msg) {
			HandleEzvizMsg(msg);
		}
	} else if (TIMER_ID_REC_VIDEO == nIDEvent) {
		auto_timer timer(m_hWnd, TIMER_ID_REC_VIDEO, 2000);
		if (lock_4_record_list_.try_lock()) {
			std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_, std::adopt_lock);
			auto now = GetTickCount();
			const int max_minutes = util::CConfigHelper::get_instance()->get_back_end_record_minutes();
			for (const auto info : record_list_) {
				//if (info->_device != m_curPlayingDevice) {
				if (!player_op_is_front_end_player(info->player_)) {
					DWORD span = (now - info->_param->_start_time) / 1000 / 60;
					if (span/*.GetTotalMinutes()*/ >= (const DWORD)max_minutes) {
						StopPlayByRecordInfo(info);
						break;
					}
				}
			}
		}
	} else if (TIMER_ID_PLAY_VIDEO == nIDEvent) {
		auto_timer timer(m_hWnd, TIMER_ID_PLAY_VIDEO, 5000);
		if (!m_wait2playDevList.empty()) {
			video::ezviz::video_device_info_ezviz_ptr dev;
			if (m_lock4Wait2PlayDevList.try_lock()) {
				std::lock_guard<std::mutex> lock(m_lock4Wait2PlayDevList, std::adopt_lock);
				dev = m_wait2playDevList.front();
				m_wait2playDevList.pop_front();
			}
			
			if (dev) {
				JLOG(L"ontimer TIMER_ID_PLAY_VIDEO, PlayVideoByDevice");
				PlayVideoByDevice(dev, util::CConfigHelper::get_instance()->get_default_video_level());
				JLOG(L"ontimer TIMER_ID_PLAY_VIDEO, PlayVideoByDevice over");
			}
		}
	} else if (TIMER_ID_JOVISION_MSG == nIDEvent) {
		auto_timer timer(m_hWnd, TIMER_ID_JOVISION_MSG, 1000);
		jovision_msg_ptr msg = nullptr;
		if (lock_4_jovision_msg_queue_.try_lock()) {
			std::lock_guard<std::mutex> lock(lock_4_jovision_msg_queue_, std::adopt_lock);
			if (jovision_msg_list_.size() > 0) {
				msg = jovision_msg_list_.front();
				jovision_msg_list_.pop_front();
			}
		}
		if(msg)
			HandleJovisionMsg(msg);
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CVideoPlayerDlg::StopPlayByRecordInfo(record_ptr info)
{
	AUTO_LOG_FUNCTION;

	if (m_curPlayingDevice == info->_device) {
		m_curPlayingDevice = nullptr;
		EnableControlPanel(0);
	}

	if (info->productor_ == EZVIZ) {
		StopPlayEzviz(std::dynamic_pointer_cast<video::ezviz::video_device_info_ezviz>(info->_device));
	} else if (info->productor_ == JOVISION) {
		StopPlayJovision(std::dynamic_pointer_cast<video::jovision::video_device_info_jovision>(info->_device));
	} else {
		assert(0);
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
	StopPlayCurselVideo();
}


void CVideoPlayerDlg::OnBnClickedButtonCapture()
{
	if (m_curPlayingDevice) {
		auto device = std::dynamic_pointer_cast<video_device_info_ezviz>(m_curPlayingDevice);
		auto user = std::dynamic_pointer_cast<video::ezviz::video_user_info_ezviz>(m_curPlayingDevice->get_userInfo()); assert(user);
		auto mgr = video::ezviz::sdk_mgr_ezviz::get_instance();
		CString path, file, fm, txt;
		path.Format(L"%s\\data\\video_capture", GetModuleFilePath());
		file.Format(L"\\%s-%s.jpg", utf8::a2w(device->get_deviceSerial()).c_str(),
					CTime::GetCurrentTime().Format(L"%Y-%m-%d-%H-%M-%S"));
		CreateDirectory(path, nullptr);
		path += file;
		fm = GetStringFromAppResource(IDS_STRING_FM_CAPTURE_OK);
		txt.Format(fm, path);
		std::string name = utf8::w2a((LPCTSTR)path);
		auto session_id = mgr->GetSessionId(user->get_user_phone(), device->get_cameraId(), messageHandler, this);
		if (!session_id.empty() && 0 == mgr->m_dll.capturePicture(session_id, name)) {
			MessageBox(txt);
		}
	}
}


void CVideoPlayerDlg::OnBnClickedButtonUp()
{
	PtzControl(video::ezviz::sdk_mgr_ezviz::UP, video::ezviz::sdk_mgr_ezviz::START);
	PtzControl(video::ezviz::sdk_mgr_ezviz::UP, video::ezviz::sdk_mgr_ezviz::STOP);
}


void CVideoPlayerDlg::OnBnClickedButtonDown()
{
	PtzControl(video::ezviz::sdk_mgr_ezviz::DOWN, video::ezviz::sdk_mgr_ezviz::START);
	PtzControl(video::ezviz::sdk_mgr_ezviz::DOWN, video::ezviz::sdk_mgr_ezviz::STOP);
}


void CVideoPlayerDlg::OnBnClickedButtonLeft()
{
	PtzControl(video::ezviz::sdk_mgr_ezviz::LEFT, video::ezviz::sdk_mgr_ezviz::START);
	PtzControl(video::ezviz::sdk_mgr_ezviz::LEFT, video::ezviz::sdk_mgr_ezviz::STOP);
}


void CVideoPlayerDlg::OnBnClickedButtonRight()
{
	PtzControl(video::ezviz::sdk_mgr_ezviz::RIGHT, video::ezviz::sdk_mgr_ezviz::START);
	PtzControl(video::ezviz::sdk_mgr_ezviz::RIGHT, video::ezviz::sdk_mgr_ezviz::STOP);
}


void CVideoPlayerDlg::PtzControl(video::ezviz::sdk_mgr_ezviz::PTZCommand command, video::ezviz::sdk_mgr_ezviz::PTZAction action)
{
	if (m_curPlayingDevice) {
		auto device = std::dynamic_pointer_cast<video::ezviz::video_device_info_ezviz>(m_curPlayingDevice);
		auto user = std::dynamic_pointer_cast<video::ezviz::video_user_info_ezviz>(device->get_userInfo()); assert(user);
		auto mgr = video::ezviz::sdk_mgr_ezviz::get_instance();
		auto session_id = mgr->GetSessionId(user->get_user_phone(), device->get_cameraId(), messageHandler, this);
		if (session_id.empty()) {
			assert(0);
			return;
		}
		mgr->m_dll.PTZCtrl(session_id,
						   user->get_acc_token(),
						   device->get_cameraId(),
						   command,
						   action,
						   PTZ_SPEED);
	}
}


void CVideoPlayerDlg::PlayVideo(const video::zone_uuid& zone)
{
	AUTO_LOG_FUNCTION;
	video::bind_info bi = video::video_manager::get_instance()->GetBindInfo(zone);
	if (bi._device && bi.auto_play_when_alarm_) {
		auto device = std::dynamic_pointer_cast<video::ezviz::video_device_info_ezviz>(bi._device);
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
	util::CConfigHelper::get_instance()->set_back_end_record_minutes(minutes);
	txt.Format(L"%d", minutes);
	m_ctrl_rerord_minute.SetWindowTextW(txt);
}


void CVideoPlayerDlg::InsertList(const record_ptr& info)
{
	if (!info) return;

	for (int i = 0; i < m_ctrl_play_list.GetItemCount(); i++) {
		play_list_item_data* data = reinterpret_cast<play_list_item_data*>(m_ctrl_play_list.GetItemData(i));
		if (data->productor == info->_device->get_userInfo()->get_productorInfo().get_productor()
			&& data->dev_id == info->_device->get_id()) {
			return;
		}
	}

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

		// productor
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), info->_device->get_userInfo()->get_productorInfo().get_formatted_name().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_ctrl_play_list.SetItem(&lvitem);
		tmp.UnlockBuffer();

		play_list_item_data* data = new play_list_item_data();
		data->productor = info->_device->get_userInfo()->get_productorInfo().get_productor();
		data->dev_id = info->_device->get_id();
		m_ctrl_play_list.SetItemData(nResult, reinterpret_cast<DWORD_PTR>(data));
		m_ctrl_play_list.SetItemState(nResult, LVNI_FOCUSED | LVIS_SELECTED, LVNI_FOCUSED | LVIS_SELECTED);

		tmp.Format(L"%s-%s", info->_device->get_userInfo()->get_user_name().c_str(), info->_device->get_device_note().c_str());
		m_static_group_cur_video.SetWindowTextW(tmp);

		CString txt;
		txt.Format(L"%s-%s", info->_device->get_userInfo()->get_user_name().c_str(), info->_device->get_device_note().c_str());
		m_static_group_cur_video.SetWindowTextW(txt);
		m_btn_voice_talk.EnableWindow();
		m_btn_voice_talk.SetWindowTextW(GetStringFromAppResource(info->voice_talking_ ? IDS_STRING_STOP_VOICE_TALK : IDS_STRING_START_VOICE_TALK));
		m_chk_volume.EnableWindow(info->voice_talking_);
		m_chk_volume.SetCheck(info->sound_opened_);
		m_slider_volume.EnableWindow(info->sound_opened_);
		if (info->voice_talking_) {
			int volume = video::ezviz::sdk_mgr_ezviz::get_instance()->m_dll.getVolume(info->_param->_session_id);
			m_slider_volume.SetPos(volume);
			txt.Format(L"%s:%d", GetStringFromAppResource(IDS_STRING_VOLUME), volume);
			m_static_volume.SetWindowTextW(txt);
		} else {
			m_slider_volume.SetPos(0);
			m_static_volume.SetWindowTextW(L"");
		}
	}
}


void CVideoPlayerDlg::OnLvnItemchangedList1(NMHDR * pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;
	if (pNMItemActivate->iItem < 0)return;
	/*int id = m_ctrl_play_list.GetItemData(pNMItemActivate->iItem);
	std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_);
	for (auto info : record_list_) {
		if (info->_device->get_id() == id) {
			PlayVideoByDevice(info->_device, info->_level);
			CString txt;
			txt.Format(L"%s-%s", info->_device->get_userInfo()->get_user_name().c_str(), info->_device->get_device_note().c_str());
			m_static_group_cur_video.SetWindowTextW(txt);
			m_btn_voice_talk.EnableWindow();
			m_btn_voice_talk.SetWindowTextW(GetStringFromAppResource(info->voice_talking_ ? IDS_STRING_STOP_VOICE_TALK : IDS_STRING_START_VOICE_TALK));
			m_chk_volume.EnableWindow(info->voice_talking_);
			m_chk_volume.SetCheck(info->sound_opened_);
			m_slider_volume.EnableWindow(info->sound_opened_);
			if (info->voice_talking_) {
				int volume = video::ezviz::sdk_mgr_ezviz::get_instance()->m_dll.getVolume(info->_param->_session_id);
				m_slider_volume.SetPos(volume);
				txt.Format(L"%s:%d", GetStringFromAppResource(IDS_STRING_VOLUME), volume);
				m_static_volume.SetWindowTextW(txt);
			} else {
				m_slider_volume.SetPos(0);
				m_static_volume.SetWindowTextW(L"");
			}
			break;
		}
	}*/
}


void CVideoPlayerDlg::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;
	if (pNMItemActivate->iItem < 0)return;
	play_list_item_data* data = reinterpret_cast<play_list_item_data*>(m_ctrl_play_list.GetItemData(pNMItemActivate->iItem));
	
	std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_);
	for (auto info : record_list_) {
		if (data->productor == info->_device->get_userInfo()->get_productorInfo().get_productor()
			&& data->dev_id == info->_device->get_id()) {

			PlayVideoByDevice(info->_device, info->_level);
			CString txt;
			txt.Format(L"%s-%s", info->_device->get_userInfo()->get_user_name().c_str(), info->_device->get_device_note().c_str());
			m_static_group_cur_video.SetWindowTextW(txt);
			m_btn_voice_talk.EnableWindow();
			m_btn_voice_talk.SetWindowTextW(GetStringFromAppResource(info->voice_talking_ ? IDS_STRING_STOP_VOICE_TALK : IDS_STRING_START_VOICE_TALK));
			m_chk_volume.EnableWindow(info->voice_talking_);
			m_chk_volume.SetCheck(info->sound_opened_);
			m_slider_volume.EnableWindow(info->sound_opened_);
			if (info->voice_talking_) {
				int volume = video::ezviz::sdk_mgr_ezviz::get_instance()->m_dll.getVolume(info->_param->_session_id);
				m_slider_volume.SetPos(volume);
				txt.Format(L"%s:%d", GetStringFromAppResource(IDS_STRING_VOLUME), volume);
				m_static_volume.SetWindowTextW(txt);
			} else {
				m_slider_volume.SetPos(0);
				m_static_volume.SetWindowTextW(L"");
			}
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
	auto cfg = util::CConfigHelper::get_instance();
	if (minutes != cfg->get_back_end_record_minutes()) {
		util::CConfigHelper::get_instance()->set_back_end_record_minutes(minutes);
		txt.Format(L"%d", minutes);
		m_ctrl_rerord_minute.SetWindowTextW(txt);
	}
}


void CVideoPlayerDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	if (bShow) {
		int global_video_level = util::CConfigHelper::get_instance()->get_default_video_level();
		m_radioGlobalSmooth.SetCheck(global_video_level == 0);
		m_radioGlobalBalance.SetCheck(global_video_level == 1);
		m_radioGlobalHD.SetCheck(global_video_level == 2);
	}
}


void CVideoPlayerDlg::OnBnClickedRadioSmooth2()
{
	util::CConfigHelper::get_instance()->set_default_video_level(0);
}


void CVideoPlayerDlg::OnBnClickedRadioBalance2()
{
	util::CConfigHelper::get_instance()->set_default_video_level(1);
}


void CVideoPlayerDlg::OnBnClickedRadioHd2()
{
	util::CConfigHelper::get_instance()->set_default_video_level(2);
}


void CVideoPlayerDlg::OnBnClickedRadio1Video()
{
	player_op_set_same_time_play_video_route(1);
}


void CVideoPlayerDlg::OnBnClickedRadio4Video()
{
	player_op_set_same_time_play_video_route(4);
}


void CVideoPlayerDlg::OnBnClickedRadio9Video()
{
	player_op_set_same_time_play_video_route(9);
}


player CVideoPlayerDlg::player_op_create_new_player()
{
	player a_player = nullptr;
	if (!player_buffer_.empty()) {
		a_player = player_buffer_.front();
		player_buffer_.pop_front();
	} else {
		CRect rc;
		m_player.GetWindowRect(rc);
		ScreenToClient(rc);
		
		a_player = std::shared_ptr<CVideoPlayerCtrl>(new CVideoPlayerCtrl(), player_deleter);
		a_player->Create(nullptr, m_dwPlayerStyle, rc, this, IDC_STATIC_PLAYER);
	}
	
	return a_player;
}


//player CVideoPlayerDlg::player_op_get_free_player() {
//	player player;
//	for (int i = 0; i < util::CConfigHelper::get_instance()->get_show_video_same_time_route_count(); i++) {
//		auto& player_ex = player_ex_vector_[i];
//		if (!player_ex->used) { // has free ctrl, show it
//			player = player_ex->player;
//			player->ShowWindow(SW_SHOW);
//			player_ex->used = true;
//			break;
//		}
//	}
//
//	if (!player) { // no free ctrl, move current first ctrl to back-end, create new ctrl and bring it to front
//		player = player_op_create_new_player();
//
//		assert(player);
//	}
//
//	return player;
//}


void CVideoPlayerDlg::player_op_bring_player_to_front(const player& player)
{
	// already playing in front-end
	if (player_op_is_front_end_player(player)) {
		player_op_rebuild();
		player_op_set_focus(player);
		return;
	}

	// not playing
	const int player_count = util::CConfigHelper::get_instance()->get_show_video_same_time_route_count();

	// have a gap
	for (int i = 0; i < player_count; i++) {
		auto player_ex = player_ex_vector_[i];
		if (!player_ex->used) {
			CRect rc;
			player_ex->player->GetWindowRect(rc);
			ScreenToClient(rc);
			player_ex->player = player;
			player->MoveWindow(rc);
			player->ShowWindow(SW_SHOW);
			player_ex->used = true;
			player_op_rebuild();
			player_op_set_focus(player);
			return;
		}
	}

	// no gap
	auto player_ex_0 = player_ex_vector_[0];
	player_ex_vector_.erase(0);
	//CRect rc;
	//player_ex_0->player->GetWindowRect(rc);
	//ScreenToClient(rc); // get player 1's rc

	//for (int i = 1; i < player_count; i++) { // 后n个player前移1位
	//	CRect my_rc;
	//	player_ex_vector_[i]->player->GetWindowRect(my_rc);
	//	ScreenToClient(my_rc);
	//	player_ex_vector_[i]->player->MoveWindow(rc);
	//	player_ex_vector_[i]->rc = rc;
	//	player_ex_vector_[i]->player->ndx_ = i;
	//	rc = my_rc;
	//}

	//player->MoveWindow(rc); // move new ctrl to last place
	CRect rc;
	m_player.GetWindowRect(rc);
	ScreenToClient(rc);
	auto v = split_rect(rc, player_count);
	for (int i = 0; i < player_count - 1; i++) {
		player_ex_vector_[i] = player_ex_vector_[i + 1];
		player_ex_vector_[i]->player->MoveWindow(v[i]);
		player_ex_vector_[i]->player->ShowWindow(SW_SHOW);
	}

	//player_op_recycle_player(player_ex_0->player); // move prev-player-1 to back-end
	//back_end_players_.push_back(player_ex_0->player);
	player_ex_0->player->ShowWindow(SW_HIDE);
	delete_from_play_list_by_record(record_op_get_record_info_by_player(player_ex_0->player));
	back_end_players_.insert(player_ex_0->player);

	player->MoveWindow(v[player_count - 1]);
	player->ShowWindow(SW_SHOW);
	player_ex_0->player = player;
	player_ex_0->used = true;

	player_ex_vector_[player_count - 1] = player_ex_0;

	// show player, add a item to play list
	InsertList(record_op_get_record_info_by_player(player));

	/*for (int i = 0; i < player_count; i++) {
		player_ex_vector_[i]->player->ShowWindow(SW_SHOW);
	}*/

	// set focus
	player_op_rebuild();
	player_op_set_focus(player);
}

void CVideoPlayerDlg::player_op_set_focus(const player& player)
{
	const int player_count = util::CConfigHelper::get_instance()->get_show_video_same_time_route_count();
	for (int i = 0; i < player_count; i++) {
		auto a_player = player_ex_vector_[i];
		if (a_player->player == player) {
			a_player->player->SetFocused();
		} else {
			a_player->player->SetFocused(0);
		}
	}
}


void CVideoPlayerDlg::player_op_recycle_player(const player& player)
{
	assert(player);
	player->ShowWindow(SW_HIDE);

	bool recycled = false;
	const int n = util::CConfigHelper::get_instance()->get_show_video_same_time_route_count();
	for (int i = 0; i < n; i++) {
		auto& player_ex = player_ex_vector_[i];
		if (player_ex->player == player) { // playing in front-end, delete its list item
			delete_from_play_list_by_record(record_op_get_record_info_by_player(player_ex->player));
			/*for (int j = i; j < n - 1; j++) {
				player_ex_vector_[j] = player_ex_vector_[j + 1];
			}
			player_ex_vector_.erase(n - 1);*/
			player_ex->player->ShowWindow(SW_HIDE);
			player_ex->used = false;
			recycled = true;
			break;
		}
	}	
	
	if (!recycled) {
		player_buffer_.push_back(player);
	}
}


void CVideoPlayerDlg::player_op_rebuild()
{
	CRect rc;
	m_player.GetWindowRect(rc);
	ScreenToClient(rc);
	const size_t n = util::CConfigHelper::get_instance()->get_show_video_same_time_route_count();
	
	size_t use_count = 0;
	for (auto player_ex : player_ex_vector_) {
		if (player_ex.second->used) {
			use_count++;
		}
	}
	if (use_count == 0)return;
	use_count = 0;
	auto v = split_rect(rc, n);
	for (size_t i = 0; i < n; i++) {
		auto& player_ex = player_ex_vector_[i];
		if (player_ex->used) {
			player_ex->player->MoveWindow(v[use_count]);
			player_ex->player->ShowWindow(SW_SHOW);
			use_count++;

			auto record = record_op_get_record_info_by_player(player_ex->player);
			if (record && record->productor_ == video::JOVISION) {
				video::jovision::sdk_mgr_jovision::get_instance()->set_video_preview(record->link_id_, 
																					 player_ex->player->GetRealHwnd(), 
																					 player_ex->player->GetRealRect());
			}

		} else {
			delete_from_play_list_by_record(record_op_get_record_info_by_player(player_ex->player));
			//for (size_t j = i; j < n - 1; j++) {
			//	player_ex_vector_[j] = player_ex_vector_[j + 1];
			//}
		}
	}

	for (size_t i = use_count; i < n; i++) {
		//player_ex_vector_[i]->player->ShowWindow(SW_HIDE);
		player_ex_vector_[i]->used = false;
	}
}


void CVideoPlayerDlg::player_op_update_players_size_with_m_player()
{
	CRect rc;
	m_player.GetWindowRect(rc);
	ScreenToClient(rc);

	const int n = util::CConfigHelper::get_instance()->get_show_video_same_time_route_count();
	auto v = split_rect(rc, n);

	auto jmgr = video::jovision::sdk_mgr_jovision::get_instance();
	for (int i = 0; i < n; i++) {
		player_ex_vector_[i]->player->MoveWindow(v[i]);
		player_ex_vector_[i]->rc = v[i];
		if (player_ex_vector_[i]->used) {
			auto record = record_op_get_record_info_by_player(player_ex_vector_[i]->player);
			if (record && record->productor_ == video::JOVISION) {
				CRect rc_player = v[i];
				ClientToScreen(rc_player);
				jmgr->set_video_preview(record->link_id_,
										player_ex_vector_[i]->player->GetRealHwnd(), 
										rc_player);
			}
		}

	}


}


bool CVideoPlayerDlg::player_op_is_front_end_player(const player& player) const
{
	for (auto player_ex : player_ex_vector_) {
		if (player_ex.second->player == player) {
			return true;
		}
	}
	return false;
}


void CVideoPlayerDlg::OnBnClickedButtonVoiceTalk()
{
	if (!m_curPlayingDevice) {
		return;
	}

	auto record = record_op_get_record_info_by_device(m_curPlayingDevice);
	if (!record)return;

	auto user = std::dynamic_pointer_cast<video::ezviz::video_user_info_ezviz>(record->_device->get_userInfo());
	if (!user)return;

	auto& mgr = video::ezviz::sdk_mgr_ezviz::get_instance()->m_dll;
	if (record->voice_talking_) {
		int ret = mgr.stopVoiceTalk(record->_param->_session_id);
		if (ret == 0) {
			mgr.closeSound(record->_param->_session_id);
			m_btn_voice_talk.SetWindowTextW(GetStringFromAppResource(IDS_STRING_START_VOICE_TALK));
			m_chk_volume.EnableWindow(0);
			m_chk_volume.SetCheck(0);
			m_slider_volume.SetPos(0);
			m_slider_volume.EnableWindow(0);
			m_static_volume.SetWindowTextW(L"");
			record->voice_talking_ = false;
			record->sound_opened_ = false;
		} else {
			CString e;
			e.Format(L"ErrorCode:%d\r\n", ret);
			MessageBox(e + GetStringFromAppResource(IDS_STRING_STOP_VOICE_TALK_FAIL), L"", MB_ICONERROR);
		}
	} else {
		int ret = mgr.startVoiceTalk(record->_param->_session_id, user->get_acc_token(), std::dynamic_pointer_cast<video::ezviz::video_device_info_ezviz>(record->_device)->get_cameraId());
		if (ret == 0 || ret == -2) {
			mgr.openSound(record->_param->_session_id);
			record->voice_talking_ = true;
			record->sound_opened_ = true;
			JLOG(L"start voice talk ok");
			m_btn_voice_talk.SetWindowTextW(GetStringFromAppResource(IDS_STRING_STOP_VOICE_TALK));
			m_chk_volume.EnableWindow();
			m_chk_volume.SetCheck(1);
			int vol = mgr.getVolume(record->_param->_session_id);
			m_slider_volume.SetPos(vol);
			m_slider_volume.EnableWindow();
			CString txt;
			txt.Format(L"%s:%d", GetStringFromAppResource(IDS_STRING_VOLUME), vol);
			m_static_volume.SetWindowTextW(txt);
		} else if (ret != 0) {
			CString e;
			e.Format(L"ErrorCode:%d\r\n", ret);
			MessageBox(e + GetStringFromAppResource(IDS_STRING_START_VOICE_TALK_FAIL), L"", MB_ICONERROR);
		}
	}
}


void CVideoPlayerDlg::OnTRBNThumbPosChangingSliderVolume(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMTRBTHUMBPOSCHANGING *pNMTPC = reinterpret_cast<NMTRBTHUMBPOSCHANGING *>(pNMHDR);
	CString txt;
	txt.Format(L"%s:%d", GetStringFromAppResource(IDS_STRING_VOLUME), pNMTPC->dwPos);
	m_static_volume.SetWindowTextW(txt);
	*pResult = 0;
}


void CVideoPlayerDlg::OnBnClickedCheckVolume()
{
	if (!m_curPlayingDevice) {
		return;
	}

	auto record = record_op_get_record_info_by_device(m_curPlayingDevice);
	if (!record || !record->voice_talking_)return;

	auto user = std::dynamic_pointer_cast<video::ezviz::video_user_info_ezviz>(record->_device->get_userInfo());
	if (!user)return;

	auto& mgr = video::ezviz::sdk_mgr_ezviz::get_instance()->m_dll;
	BOOL open = m_chk_volume.GetCheck();
	if (open) {
		mgr.openSound(record->_param->_session_id);
		record->sound_opened_ = true;
		m_slider_volume.EnableWindow();
		int vol = mgr.getVolume(record->_param->_session_id);
		m_slider_volume.SetPos(vol);
		CString txt;
		txt.Format(L"%s:%d", GetStringFromAppResource(IDS_STRING_VOLUME), vol);
		m_static_volume.SetWindowTextW(txt);
	} else {
		mgr.closeSound(record->_param->_session_id);
		record->sound_opened_ = false;
		m_slider_volume.EnableWindow(0);
		m_static_volume.SetWindowTextW(L"");
	}
}


void CVideoPlayerDlg::OnNMReleasedcaptureSliderVolume(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	*pResult = 0;

	int pos = m_slider_volume.GetPos();

	if (!m_curPlayingDevice) {
		return;
	}

	auto record = record_op_get_record_info_by_device(m_curPlayingDevice);
	if (!record || !record->voice_talking_)return;

	auto& mgr = video::ezviz::sdk_mgr_ezviz::get_instance()->m_dll;
	mgr.setVolume(record->_param->_session_id, pos & 0xFFFF);
	CString txt;
	txt.Format(L"%s:%d", GetStringFromAppResource(IDS_STRING_VOLUME), pos);
	m_static_volume.SetWindowTextW(txt);
}
