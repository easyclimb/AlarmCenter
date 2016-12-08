// VideoPlayerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenterVideo.h"
#include "VideoPlayerDlg.h"
#include "afxdialogex.h"
#include "../video/ezviz/VideoUserInfoEzviz.h"
#include "../video/ezviz/VideoDeviceInfoEzviz.h"
#include "PrivateCloudConnector.h"
#include "InputDeviceVerifyCodeDlg.h"
#include "../contrib/json/json.h"
#include "InputDlg.h"
#include "ConfigHelper.h"
#include "../video/ezviz/ezviz_inc/INS_ErrorCode.h"
#include "../video/ezviz/ezviz_inc/OpenNetStreamError.h"
#include "JovisonSdkMgr.h"
#include "../video/jovision/VideoUserInfoJovision.h"
#include "../video/jovision/VideoDeviceInfoJovision.h"
#include "VideoRecordPlayerDlg.h"
#include "VideoManager.h"
#include "alarm_center_video_client.h"

namespace detail {
	const int TIMER_ID_EZVIZ_MSG = 1;
	const int TIMER_ID_REC_VIDEO = 2;
	const int TIMER_ID_PLAY_VIDEO = 3;
	const int TIMER_ID_JOVISION_MSG = 4;

	//const int TIMEOUT_4_VIDEO_RECORD = 10; // in minutes

	const int HOTKEY_PTZ = 12;

	const char *const SMSCODE_SECURE_REQ = "{\"method\":\"msg/smsCode/secure\",\"params\":{\"accessToken\":\"%s\"}}";
	const char *const SECUREVALIDATE_REQ = "{\"method\":\"msg/sdk/secureValidate\",\"params\":{\"smsCode\": \"%s\",\"accessToken\": \"%s\"}}";

	//auto split_rect = [](CRect rc, int n) {
	//	std::vector<CRect> v;
	//	for (int i = 0; i < n; i++) {
	//		v.push_back(rc);
	//	}

	//	double l = sqrt(n);
	//	int line = int(l);
	//	if (l - line != 0) {
	//		return v;
	//	}

	//	int col_step = (int)(rc.Width() / line);
	//	int row_step = (int)(rc.Height() / line);

	//	for (int i = 0; i < n; i++) {
	//		v[i].left = rc.left + (i % line) * col_step;
	//		v[i].right = v[i].left + col_step;
	//		v[i].top = rc.top + (i / line) * row_step;
	//		v[i].bottom = v[i].top + row_step;
	//		//v[i].DeflateRect(1, 1, 1, 1);
	//	}
	//	
	//	return v;
	//};


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

	auto rec_player_deleter = [](CVideoRecordPlayerDlg* p) {
		SAFEDELETEDLG(p);
	};

	typedef std::pair<video::zone_uuid, core::alarm_text_ptr> zone_alarm_text_pair;
};

using namespace ::detail;
using namespace video;
using namespace video::ezviz;
using namespace video::jovision;

#pragma region in-class structs

struct CVideoPlayerDlg::player_ex {
	bool used = false;
	player player = nullptr;
	CRect rc = { 0 };

	~player_ex() {
		player = nullptr;
	}
};

struct CVideoPlayerDlg::DataCallbackParamEzviz
{
	CVideoPlayerDlg* _dlg;
	char _session_id[1024];
	wchar_t _file_path[4096];
	DWORD _start_time;
	DataCallbackParamEzviz() : _dlg(nullptr), _session_id(), _file_path(), _start_time(0) {}
	DataCallbackParamEzviz(CVideoPlayerDlg* dlg, const std::string& session_id, DWORD startTime)
		: _dlg(dlg), _session_id(), _file_path(), _start_time(startTime)
	{
		strcpy(_session_id, session_id.c_str());
	}

	~DataCallbackParamEzviz() {}

	CString FormatFilePath(int user_id, const std::wstring& user_name, const std::wstring& dev_formatted_note, const wchar_t* ext = L"mp4")
	{
		auto name = integrate_path(user_name);
		auto note = integrate_path(dev_formatted_note);
		CString path, user_path;
		path.Format(L"%s\\data\\video_record", get_exe_path().c_str());
		CreateDirectory(path, nullptr);
		user_path.Format(L"\\%d-%s", user_id, name.c_str());
		path += user_path;
		CreateDirectory(path, nullptr);
		CString file; file.Format(L"\\%s-(%s).%s",
								  CTime::GetCurrentTime().Format(L"%Y-%m-%d_%H-%M-%S"),
								  integrate_path(dev_formatted_note).c_str(), ext);
		path += file;
		wcscpy(_file_path, path.LockBuffer()); path.UnlockBuffer();
		return path;
	}
};

struct CVideoPlayerDlg::record {
	bool started_ = false;
	video::productor_type productor_ = video::UNKNOWN;
	DataCallbackParamEzviz* _param = nullptr;
	//video::zone_uuid _zone = {};
	video::device_ptr _device = nullptr;
	//core::alarm_text_ptr alarm_text_ = nullptr;
	int _level = 0;
	video::jovision::JCLink_t link_id_ = -1;

	std::list<zone_alarm_text_pair> zone_alarm_text_pairs_ = {};

	bool voice_talking_ = false;
	bool sound_opened_ = false;
	bool verified_hd_ = false;
	bool e45_occured_ = false;

	bool connecting_ = false;
	bool decoding_ = false;
	bool previewing_ = false;

	player player_ = nullptr;
	rec_player rec_player = nullptr;

	record() {}
	record(DataCallbackParamEzviz* param, const video::zone_uuid& zone,
		   const video::ezviz::ezviz_device_ptr& device, 
		   const core::alarm_text_ptr& alarm_text,
		   const player& player, int level)
		: _param(param), /*_zone(zone), */_device(device), /*alarm_text_(alarm_text), */player_(player), _level(level)
	{
		productor_ = video::EZVIZ;
		zone_alarm_text_pairs_.push_back({ zone, alarm_text });
	}

	record(DataCallbackParamEzviz* param, const video::zone_uuid& zone,
		   const video::jovision::jovision_device_ptr& device, 
		   const core::alarm_text_ptr& alarm_text,
		   const player& player, int level)
		:_param(param),/* _zone(zone), */_device(device), /*alarm_text_(alarm_text),*/ player_(player), _level(level)
	{
		productor_ = video::JOVISION;
		zone_alarm_text_pairs_.push_back({ zone, alarm_text });
	}

	~record() { SAFEDELETEP(_param); player_ = nullptr; rec_player = nullptr; }
};

struct CVideoPlayerDlg::wait_to_play_dev {
	video::device_ptr device;
	core::alarm_text_ptr alarm_text;
	video::zone_uuid_ptr zone_uuid;

	wait_to_play_dev(const video::device_ptr& dev, const video::zone_uuid_ptr& zid, const core::alarm_text_ptr& at)
		: device(dev), alarm_text(at), zone_uuid(zid) 
	{}
};

void CVideoPlayerDlg::OnCurUserChangedResult(const core::user_info_ptr& user)
{
	/*assert(user);
	if (user->get_user_priority() == core::UP_OPERATOR) {

	} else {

	}*/
}

class CVideoPlayerDlg::CurUserChangedObserver : public dp::observer<core::user_info_ptr>
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



#pragma endregion


#pragma region ezviz callbacks

struct CVideoPlayerDlg::ezviz_msg
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
};

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
		auto device = std::dynamic_pointer_cast<video::ezviz::ezviz_device>(record->_device);
		auto zoneUuid = record->zone_alarm_text_pairs_.front().first;

		CString txt;
		txt.Format(L"%s(%s)-\"%s\"", 
				   TR(IDS_STRING_VIDEO_START),
				   device->get_formatted_name().c_str(),
				   record->_param->_file_path);

		ipc::alarm_center_video_client::get_instance()->insert_record(zoneUuid._ademco_id, zoneUuid._zone_value, (LPCTSTR)txt);
	}
}


void CVideoPlayerDlg::on_ins_play_stop(record_ptr record)
{
	if (record) {
		auto device = std::dynamic_pointer_cast<video::ezviz::ezviz_device>(record->_device);
		auto zoneUuid = record->zone_alarm_text_pairs_.front().first;

		CString txt;
		txt.Format(L"%s(%s)-\"%s\"",
				   TR(IDS_STRING_VIDEO_STOP),
				   device->get_formatted_name().c_str(),
				   (record->_param->_file_path));
		
		ipc::alarm_center_video_client::get_instance()->insert_record(zoneUuid._ademco_id, zoneUuid._zone_value, (LPCTSTR)txt);

		std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_);
		record_list_.remove(record);
		player_op_recycle_player(record->player_);
		delete_from_play_list_by_record(record);
		auto mgr = video::ezviz::sdk_mgr_ezviz::get_instance();
		mgr->FreeSession(record->_param->_session_id);

		bool can_re_play = false;
		if (record->e45_occured_) {
			if (!record->verified_hd_) {
				auto user = std::dynamic_pointer_cast<video::ezviz::ezviz_user>(record->_device->get_userInfo());
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
		sInfo.Format(L"%s-(%s)\r\n", record->_device->get_userInfo()->get_user_name().c_str(), record->_device->get_formatted_name().c_str());
	}
	sInfo.AppendFormat(L"ErrorCode = %d", msg->iErrorCode);

	switch (msg->iErrorCode) {
	case INS_ERROR_V17_PERMANENTKEY_EXCEPTION:
		e = TR(IDS_STRING_VERIFY_CODE_WRONG);
		break;

	case  INS_ERROR_CASLIB_PLATFORM_CLIENT_REQUEST_NO_PU_FOUNDED:
		e = TR(IDS_STRING_DEVICE_OFFLINE);
		break;

	/*case INS_ERROR_CASLIB_PLATFORM_CLIENT_NO_SIGN_RELEATED:
	{
		JLOG(sInfo);
	}
	break;*/

	case INS_ERROR_OPERATIONCODE_FAILED:
		e = TR(IDS_STRING_DEVICE_OFFLINE);
		//record->e45_occured_ = true;
		//return;
		break;

	case INS_ERROR_V17_VTDU_TIMEOUT:
	case INS_ERROR_V17_VTDU_STOP:
		e = TR(IDS_STRING_VTDU_TIMEOUT);
		break;

	case NS_ERROR_PRIVATE_VTDU_DISCONNECTED_LINK:
		e = TR(IDS_STRING_VTDU_DISCONNECTED_LINK);
		break;

	case NS_ERROR_PRIVATE_VTDU_STATUS_404:
		e = TR(IDS_STRING_DEVICE_OFFLINE);
		break;

	case NS_ERROR_PRIVATE_VTDU_STATUS_405:
		e = TR(IDS_STRING_VTDU_STATUS_405);
		break;

	case NS_ERROR_PRIVATE_VTDU_STATUS_406:
		e = TR(IDS_STRING_VTDU_STATUS_406);
		break;

	case NS_ERROR_PRIVATE_VTDU_STATUS_452:
		e = TR(IDS_STRING_VTDU_STATUS_452);
		break;

	case NS_ERROR_PRIVATE_VTDU_STATUS_454:
		e = TR(IDS_STRING_VTDU_STATUS_454);
		break;

	case NS_ERROR_PRIVATE_VTDU_STATUS_491:
		e = TR(IDS_STRING_VTDU_STATUS_491);
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
	MessageBox(sInfo, TR(IDS_STRING_PLAY_EXCEPTION), MB_ICONINFORMATION);
	
	//if (INS_ERROR_OPERATIONCODE_FAILED == msg->iErrorCode) {
		
		//if (!record->verified_hd_) {
		//	auto user = std::dynamic_pointer_cast<video::ezviz::ezviz_user>(record->_device->get_userInfo());
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

void funJCEventCallback(JCLink_t nLinkID, JCEventType etType, DWORD_PTR pData1, DWORD_PTR pData2, LPVOID pUserData)
{
	if (g_player) {
		g_player->EnqueJovisionMsg(std::make_shared<jovision_msg>(nLinkID, etType, pData1, pData2, pUserData));
	}
}

void funJCDataCallback(JCLink_t /*nLinkID*/, PJCStreamFrame /*pFrame*/, LPVOID /*pUserData*/)
{
	/*char acBuffer[32];
	sprintf(acBuffer, "Type:%d\n", pFrame->sType);
	OutputDebugStringA(acBuffer);*/
}

void funJCRawDataCallback(JCLink_t /*nLinkID*/, PJCRawFrame /*pFrame*/, LPVOID /*pUserData*/)
{
}

void funLanSearchCallback(PJCLanDeviceInfo /*pDevice*/)
{
	/*if (pDevice == NULL) {
		PostMessage(g_hFindDeviceWnd, WM_REFLASHDEVICELIST, 0, 0);
	} else {
		g_DeviceInfosList.push_back(*pDevice);
	}*/
}


#pragma endregion



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
	DDX_Control(pDX, IDC_BUTTON_REMOTE_CONFIG, m_btn_remote_config);
	DDX_Control(pDX, IDC_STATIC_ALARM, m_group_alarm);
	DDX_Control(pDX, IDC_LIST_ZONE, m_list_alarm);
	DDX_Control(pDX, IDC_CHECK_AUTO_PLAY_REC, m_chk_auto_play_rec);
	DDX_Control(pDX, IDC_BUTTON_OPEN_REC, m_btn_open_rec);
	DDX_Control(pDX, IDC_BUTTON_STOP_ALL, m_btn_stop_all_videos);
	DDX_Control(pDX, IDC_STATIC_DEVLIST, m_group_all_devs);
	DDX_Control(pDX, IDC_LIST_ALLDEV, m_list_all_devs);
	DDX_Control(pDX, IDC_BUTTON_SAVE_TIME, m_btn_save_time);
}


BEGIN_MESSAGE_MAP(CVideoPlayerDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CVideoPlayerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CVideoPlayerDlg::OnBnClickedCancel)
	ON_WM_MOVE()
	ON_MESSAGE(WM_INVERSIONCONTROL, &CVideoPlayerDlg::OnInversioncontrol)
	ON_MESSAGE(WM_INVERT_FOCUS_CHANGED, &CVideoPlayerDlg::OnInvertFocuseChanged)
	ON_MESSAGE(WM_SHOW_USER_MGR_DLG, &CVideoPlayerDlg::OnMsgShowVideoUserMgrDlg)
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
	//ON_BN_CLICKED(IDC_BUTTON_SAVE, &CVideoPlayerDlg::OnBnClickedButtonSave)
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
	ON_BN_CLICKED(IDC_BUTTON_REMOTE_CONFIG, &CVideoPlayerDlg::OnBnClickedButtonRemoteConfig)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_REC, &CVideoPlayerDlg::OnBnClickedButtonOpenRec)
	ON_BN_CLICKED(IDC_CHECK_AUTO_PLAY_REC, &CVideoPlayerDlg::OnBnClickedCheckAutoPlayRec)
	ON_LBN_SELCHANGE(IDC_LIST_ZONE, &CVideoPlayerDlg::OnLbnSelchangeListZone)
	ON_BN_CLICKED(IDC_BUTTON_STOP_ALL, &CVideoPlayerDlg::OnBnClickedButtonStopAll)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_ALLDEV, &CVideoPlayerDlg::OnNMDblclkListAlldev)
	ON_WM_CLOSE()
	ON_MESSAGE(WM_VIDEO_INFO_CHANGE, &CVideoPlayerDlg::OnMsgVideoChanged)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_TIME, &CVideoPlayerDlg::OnBnClickedButtonSaveTime)
END_MESSAGE_MAP()


// CVideoPlayerDlg message handlers


BOOL CVideoPlayerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText(TR(IDS_STRING_IDD_DIALOG_VIDEO_PLAYER));
	SET_WINDOW_TEXT(IDC_STATIC_VIDEO_LIST, IDS_STRING_IDC_STATIC_VIDEO_LIST);
	SET_WINDOW_TEXT(IDC_STATIC_DEVLIST, IDS_STRING_IDC_STATIC_DEVLIST);
	SET_WINDOW_TEXT(IDC_STATIC_RECORD_SETTINGS, IDS_STRING_IDC_STATIC_RECORD_SETTINGS);
	SET_WINDOW_TEXT(IDC_STATIC_SPEED, IDS_STRING_IDC_STATIC_SPEED);
	SET_WINDOW_TEXT(IDC_STATIC_PTZ, IDS_STRING_IDC_STATIC_PTZ);
	SET_WINDOW_TEXT(IDC_STATIC_CONTROL, IDS_STRING_IDC_STATIC_CONTROL);
	SET_WINDOW_TEXT(IDC_STATIC_VOICE_TALK, IDS_STRING_IDC_STATIC_VOICE_TALK);
	SET_WINDOW_TEXT(IDC_STATIC_ALARM, IDS_STRING_HRLV_ALARM);

	SET_WINDOW_TEXT(IDC_STATIC_NOTE, IDS_STRING_Longest_Time_Background_Video_Footage);	//后台视频录像最长时间
	SET_WINDOW_TEXT(IDC_STATIC_MINUTE, IDS_STRING_IDC_STATIC_MINUTE);
	SET_WINDOW_TEXT(IDC_BUTTON_SAVE_TIME, IDS_STRING_IDC_BUTTON_SAVE_CHANGE);
	SET_WINDOW_TEXT(IDC_STATIC_NOTE2_5, IDS_STRING_IDC_STATIC_NOTE2);
	SET_WINDOW_TEXT(IDC_RADIO_SMOOTH2, IDS_STRING_smooth);	//流畅
	SET_WINDOW_TEXT(IDC_RADIO_BALANCE2, IDS_STRING_Balance);	//平衡
	SET_WINDOW_TEXT(IDC_RADIO_HD2, IDS_STRING_Hd);		//高清
	SET_WINDOW_TEXT(IDC_CHECK_AUTO_PLAY_REC, IDS_STRING_If_It_Will_Video_Camera_Support);	//若摄像机支持则播放报警前最新录像
	SET_WINDOW_TEXT(IDC_RADIO_1_VIDEO, IDS_STRING_A_Single_Picture);		//单画面
	SET_WINDOW_TEXT(IDC_RADIO_4_VIDEO, IDS_STRING_Four_Images);		//4画面
	SET_WINDOW_TEXT(IDC_RADIO_9_VIDEO, IDS_STRING_Nine_Images);		//9画面

	SET_WINDOW_TEXT(IDC_RADIO_SMOOTH, IDS_STRING_smooth);		//流畅
	SET_WINDOW_TEXT(IDC_RADIO_BALANCE, IDS_STRING_Balance);		//平衡
	SET_WINDOW_TEXT(IDC_RADIO_HD, IDS_STRING_Hd);		//高清

	SET_WINDOW_TEXT(IDC_BUTTON_UP, IDS_STRING_IDC_BUTTON_MOVE_UP);
	SET_WINDOW_TEXT(IDC_BUTTON_LEFT, IDS_STRING_IDC_BUTTON_LEFT);
	SET_WINDOW_TEXT(IDC_BUTTON_DOWN, IDS_STRING_IDC_BUTTON_DOWN);
	SET_WINDOW_TEXT(IDC_BUTTON_RIGHT, IDS_STRING_IDC_BUTTON_MOVE_RIGHT);

	SET_WINDOW_TEXT(IDC_BUTTON_STOP, IDS_STRING_IDC_BUTTON_STOP);
	SET_WINDOW_TEXT(IDC_BUTTON_CAPTURE, IDS_STRING_IDC_BUTTON_CAPTURE);
	SET_WINDOW_TEXT(IDC_BUTTON_OPEN_REC, IDS_STRING_IDC_BUTTON_OPEN_REC);
	SET_WINDOW_TEXT(IDC_BUTTON_REMOTE_CONFIG, IDS_STRING_IDC_BUTTON_REMOTE_CONFIG);

	SET_WINDOW_TEXT(IDC_BUTTON_VOICE_TALK, IDS_STRING_IDC_BUTTON_VOICE_TALK);

	SET_WINDOW_TEXT(IDC_CHECK_VOLUME, IDS_STRING_Voice);		//声音
	SET_WINDOW_TEXT(IDC_STATIC_VLUME, IDS_STRING_IDC_STATIC_VLUME);


	GetWindowText(m_title);
	g_player = this;
	g_videoPlayerDlg = this;

	
	//video_user_mgr_dlg_->ShowWindow(SW_HIDE);

	{
		auto cfg = util::CConfigHelper::get_instance();
		
		if (!ezviz::sdk_mgr_ezviz::get_instance()->InitLibrary() || !ezviz::sdk_mgr_ezviz::get_instance()->Init(cfg->get_ezviz_app_key())) {
			AfxMessageBox(TR(IDS_STRING_INIT_EZVIZ_SDK_ERROR), MB_ICONEXCLAMATION);
			ExitProcess(8858);
			return FALSE;
		}

		if (!video::ezviz::sdk_mgr_ezviz::get_instance()->InitLibrary()) {
			ExitProcess(8858);
			return FALSE;
		}

		auto videoMgr = video::video_manager::get_instance();
		videoMgr->LoadFromDB();

		auto jov = jovision::sdk_mgr_jovision::get_instance();
		if (!jov->init_sdk(-1)) {
			MessageBox(TR(IDS_STRING_INIT_JOVISION_SDK_FAILED), L"Error", MB_ICONERROR);
			ExitProcess(0);
			return TRUE;
		}

		jov->register_call_back(funJCEventCallback,
								funJCDataCallback,
								funJCRawDataCallback,
								funLanSearchCallback);
	}


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
	m_list_all_devs.SetExtendedStyle(dwStyle);
	int ndx = -1;
	CString fm;
	fm = TR(IDS_STRING_ID);
	m_ctrl_play_list.InsertColumn(++ndx, fm, LVCFMT_LEFT, 50, -1);
	fm = TR(IDS_STRING_USER);
	m_ctrl_play_list.InsertColumn(++ndx, fm, LVCFMT_LEFT, 70, -1);
	fm = TR(IDS_STRING_IDC_STATIC_025);
	m_ctrl_play_list.InsertColumn(++ndx, fm, LVCFMT_LEFT, 150, -1);
	fm = TR(IDS_STRING_PRODUCTOR);
	m_ctrl_play_list.InsertColumn(++ndx, fm, LVCFMT_LEFT, 100, -1);

	ndx = -1;
	fm = TR(IDS_STRING_ID);
	m_list_all_devs.InsertColumn(++ndx, fm, LVCFMT_LEFT, 50, -1);
	fm = TR(IDS_STRING_USER);
	m_list_all_devs.InsertColumn(++ndx, fm, LVCFMT_LEFT, 70, -1);
	fm = TR(IDS_STRING_IDC_STATIC_025);
	m_list_all_devs.InsertColumn(++ndx, fm, LVCFMT_LEFT, 150, -1);
	fm = TR(IDS_STRING_PRODUCTOR);
	m_list_all_devs.InsertColumn(++ndx, fm, LVCFMT_LEFT, 100, -1);

	RefreshDevList();

	fm.Format(L"%d", util::CConfigHelper::get_instance()->get_back_end_record_minutes());
	m_ctrl_rerord_minute.SetWindowTextW(fm);

	//auto mgr = core::user_manager::get_instance();
	//auto user = mgr->GetCurUserInfo();
	//OnCurUserChangedResult(user);
	//m_cur_user_changed_observer = std::make_shared<CurUserChangedObserver>(this);
	//mgr->register_observer(m_cur_user_changed_observer);

	auto cfg = util::CConfigHelper::get_instance();

	m_chk_auto_play_rec.SetCheck(cfg->get_auto_play_rec_if_available());

	CRect rc;
	m_player.GetWindowRect(rc);
	ScreenToClient(rc);
	const int same_time_play_vidoe_route_count = cfg->get_show_video_same_time_route_count();
	for (int i = 0; i < same_time_play_vidoe_route_count; i++) {
		auto a_player_ex = std::make_shared<player_ex>();
		a_player_ex->player = std::shared_ptr<CVideoPlayerCtrl>(new CVideoPlayerCtrl(), player_deleter);
		a_player_ex->player->set_index(i + 1);
		a_player_ex->player->Create(nullptr, m_dwPlayerStyle, rc, this, IDC_STATIC_PLAYER);
		player_ex_vector_[i] = (a_player_ex);
	}

	m_bInitOver = TRUE;

	LoadPosition();

	m_group_all_devs.SetWindowTextW(TR(IDS_STRING_ALL_DEVS));
	
	m_chk_1_video.SetCheck(same_time_play_vidoe_route_count == 1);
	m_chk_4_video.SetCheck(same_time_play_vidoe_route_count == 4);
	m_chk_9_video.SetCheck(same_time_play_vidoe_route_count == 9);
	player_op_set_same_time_play_video_route(same_time_play_vidoe_route_count);
	m_btn_stop_all_videos.SetWindowTextW(TR(IDS_STRING_STOP_ALL_VIDEO));

	m_btn_remote_config.EnableWindow(0);
	m_btn_open_rec.EnableWindow(0);
	m_btn_voice_talk.EnableWindow(0);
	m_chk_volume.SetCheck(0);
	m_chk_volume.EnableWindow(0);
	m_slider_volume.SetRange(0, 100, 1);
	m_slider_volume.EnableWindow(0);

	SetTimer(TIMER_ID_EZVIZ_MSG, 1000, nullptr);
	SetTimer(TIMER_ID_REC_VIDEO, 2000, nullptr);
	SetTimer(TIMER_ID_PLAY_VIDEO, 1000, nullptr);
	SetTimer(TIMER_ID_JOVISION_MSG, 1000, nullptr);

	video_user_mgr_dlg_ = std::make_unique<CVideoUserManagerDlg>(this);
	video_user_mgr_dlg_->Create(IDD_DIALOG_MGR_VIDEO_USER, this);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


CVideoPlayerDlg::record_ptr CVideoPlayerDlg::record_op_get_record_info_by_device(const video::device_ptr& device) {
	record_ptr rec_info = nullptr;
	std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_);
	for (auto info : record_list_) {
		if (info->productor_ == video::EZVIZ) {
			if (info->_device == std::dynamic_pointer_cast<video::ezviz::ezviz_device>(device)) {
				rec_info = info;
				break;
			}
		} else if (info->productor_ == video::JOVISION) {
			if (info->_device == std::dynamic_pointer_cast<video::jovision::jovision_device>(device)) {
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

	auto v = split_rect(rc, n, 1);

	for (int i = 0; i < n && i < prev_count; i++) {
		player_ex_vector_[i]->player->MoveWindow(&v[i]);
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
			
			a_player_ex->player->MoveWindow(&v[i]);
			a_player_ex->player->ShowWindow(SW_SHOW);
			
			player_ex_vector_[i] = (a_player_ex);
		}

	} else {
		for (int i = n; i < prev_count; i++) {
			auto player = player_ex_vector_[i]->player;
			player->ShowWindow(SW_HIDE);
			//delete_from_play_list_by_record(record_op_get_record_info_by_player(player));
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

	Invalidate();
}


void CVideoPlayerDlg::LoadPosition()
{
	AUTO_LOG_FUNCTION;
	auto cfg = util::CConfigHelper::get_instance();

	do{
		CRect rect = cfg->get_rectVideoPlayerDlg();
		if (rect.IsRectNull() || rect.IsRectEmpty()) {
			GetWindowRect(rect);
		}
		int m = cfg->get_maximizedVideoPlayerDlg();
		CRect rc;
		GetWindowRect(rc);
		rect.right = rect.left + rc.Width();
		rect.bottom = rect.top + rc.Height();
		MoveWindow(rect);

		GetWindowPlacement(&m_rcNormal);
		JLOGA("m_rcNormal l:%d, t:%d, r:%d, b:%d",
			  m_rcNormal.rcNormalPosition.left, m_rcNormal.rcNormalPosition.top,
			  m_rcNormal.rcNormalPosition.right, m_rcNormal.rcNormalPosition.bottom);
		m_player.GetWindowPlacement(&m_rcNormalPlayer);
		maximized_ = m;
		OnInversioncontrol(1, 0);

		return;
	} while (0);


}


void CVideoPlayerDlg::SavePosition()
{
	AUTO_LOG_FUNCTION;
	CRect rect;
	GetWindowRect(rect);

	JLOGA("l:%d, t:%d, r:%d, b:%d", rect.left, rect.top, rect.right, rect.bottom);

	auto cfg = util::CConfigHelper::get_instance();
	cfg->set_rectVideoPlayerDlg(rect);
	cfg->set_maximizedVideoPlayerDlg(maximized_);

	if (!maximized_) {
		GetWindowPlacement(&m_rcNormal);
		JLOGA("m_rcNormal l:%d, t:%d, r:%d, b:%d",
			  m_rcNormal.rcNormalPosition.left, m_rcNormal.rcNormalPosition.top,
			  m_rcNormal.rcNormalPosition.right, m_rcNormal.rcNormalPosition.bottom);
		//m_player.GetWindowPlacement(&m_rcNormalPlayer);
	}
}

void CVideoPlayerDlg::OnBnClickedOk()
{
	return;
}


void CVideoPlayerDlg::OnBnClickedCancel()
{
	//ShowWindow(SW_HIDE);
	CDialogEx::OnCancel();
}


void CVideoPlayerDlg::OnMove(int x, int y)
{
	CDialogEx::OnMove(x, y);

	if (m_bInitOver) {
		player_op_rebuild();
		SavePosition();
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
	m_group_all_devs.ShowWindow(sw);
	m_list_all_devs.ShowWindow(sw);
	m_group_record_settings.ShowWindow(sw);
	m_static_note.ShowWindow(sw);
	m_ctrl_rerord_minute.ShowWindow(sw);
	m_static_minute.ShowWindow(sw);
	m_staticNote2.ShowWindow(sw);
	m_radioGlobalSmooth.ShowWindow(sw);
	m_radioGlobalBalance.ShowWindow(sw);
	m_radioGlobalHD.ShowWindow(sw);
	m_btn_save_time.ShowWindow(sw);

	m_chk_1_video.ShowWindow(sw);
	m_chk_4_video.ShowWindow(sw);
	m_chk_9_video.ShowWindow(sw);
	m_btn_stop_all_videos.ShowWindow(sw);

	//m_btn_remote_config.ShowWindow(sw);
	m_group_voice_talk.ShowWindow(sw);
	m_btn_voice_talk.ShowWindow(sw);
	m_chk_volume.ShowWindow(sw);
	m_static_volume.ShowWindow(sw);
	m_slider_volume.ShowWindow(sw);

	//m_group_alarm.ShowWindow(sw);
	//m_list_alarm.ShowWindow(sw);
	m_chk_auto_play_rec.ShowWindow(sw);
	m_btn_open_rec.ShowWindow(sw);

}


void CVideoPlayerDlg::EnableControlPanel(BOOL bAble, int level)
{
	m_radioSmooth.EnableWindow(bAble);
	m_radioSmooth.SetCheck(level == 0);
	m_radioBalance.EnableWindow(bAble); 
	m_radioBalance.SetCheck(level == 1);
	m_radioHD.EnableWindow(bAble); 
	m_radioHD.SetCheck(level == 2);
	m_btnStop.EnableWindow(bAble);
	m_btnCapture.EnableWindow(bAble);
	m_btnUp.EnableWindow(bAble);
	m_btnDown.EnableWindow(bAble);
	m_btnLeft.EnableWindow(bAble);
	m_btnRight.EnableWindow(bAble);
	m_btn_remote_config.EnableWindow(bAble);
	m_btn_open_rec.EnableWindow(bAble);
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
			JLOGA("m_rcNormal l:%d, t:%d, r:%d, b:%d", 
				  m_rcNormal.rcNormalPosition.left, m_rcNormal.rcNormalPosition.top,
				  m_rcNormal.rcNormalPosition.right, m_rcNormal.rcNormalPosition.bottom);
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

LRESULT CVideoPlayerDlg::OnInvertFocuseChanged(WPARAM wParam, LPARAM lParam)
{
	AUTO_LOG_FUNCTION;
	if (m_bInitOver) {
		size_t ndx = wParam;
		int focused = lParam;

		if (ndx >= 1 && ndx <= player_ex_vector_.size()) {
			for (auto player_ex : player_ex_vector_) {
				if (player_ex.first == ndx - 1) {
					player_ex.second->player->SetFocused(1);
					auto info = record_op_get_record_info_by_player(player_ex.second->player);
					if (info) {
						PlayVideoByDevice(info->_device, info->_level);

						show_one_by_record(info);
						 
						for (int i = 0; i < m_ctrl_play_list.GetItemCount(); i++) {
							auto data = reinterpret_cast<video::video_device_identifier*>(m_ctrl_play_list.GetItemData(i));
							if (data && info->_device->get_userInfo() && data->dev_id == info->_device->get_id() && data->productor_type == info->_device->get_userInfo()->get_productor().get_productor_type()) {
								m_ctrl_play_list.SetItemState(i, LVNI_FOCUSED | LVIS_SELECTED, LVNI_FOCUSED | LVIS_SELECTED);
								break;
							}
						}
					}
				} else {
					player_ex.second->player->SetFocused(0);
				}
			}
		}

	}

	return LRESULT(0);
}


void CVideoPlayerDlg::PlayVideoByDevice(const video::device_ptr& device, int speed, const video::zone_uuid_ptr& zid, const core::alarm_text_ptr& at)
{
	AUTO_LOG_FUNCTION;
	ShowWindow(SW_SHOWNORMAL);
	assert(device);
	if (!device) {
		return;
	}

	user_ptr user = device->get_userInfo(); 
	if (!user) {
		return;
	}
	auto productor = user->get_productor().get_productor_type();
	switch (productor) {
	case video::EZVIZ:
		PlayVideoEzviz(std::dynamic_pointer_cast<video::ezviz::ezviz_device>(device), speed, zid, at);
		break;
	case video::JOVISION:
		PlayVideoJovision(std::dynamic_pointer_cast<video::jovision::jovision_device>(device), speed, zid, at);
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
		auto productor = m_curPlayingDevice->get_userInfo()->get_productor().get_productor_type();
		if (productor == EZVIZ) {
			StopPlayEzviz(std::dynamic_pointer_cast<video::ezviz::ezviz_device>(m_curPlayingDevice));
		} else if (productor == JOVISION) {
			StopPlayJovision(std::dynamic_pointer_cast<video::jovision::jovision_device>(m_curPlayingDevice));
		} else {
			assert(0);
		}
		
		m_curPlayingDevice = nullptr;
	}
}


bool CVideoPlayerDlg::do_hd_verify(const video::ezviz::ezviz_user_ptr& user)
{
	AUTO_LOG_FUNCTION;
	if (!user) {
		return false;
	}

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
			MessageBox(TR(IDS_STRING_OP_IS_TO_FAST), L"", MB_ICONINFORMATION);
		} else {
			break;
		}
	} while (0);

	do {
		if (!ok) break;
		ok = false;
		CInputPasswdDlg dlg(this);
		dlg.m_title = TR(IDS_STRING_INPUT_PHONE_VERIFY_CODE);
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
			MessageBox(TR(IDS_STRING_SMS_CODE_ERR), L"", MB_ICONERROR);
			break;
		}
	} while (0);

	return ok;
}

void CVideoPlayerDlg::on_jov_play_start(const record_ptr & record)
{
	if (record && !record->started_) {
		player_op_bring_player_to_front(record->player_);
		
		// show video record
		do {
			auto cfg = util::CConfigHelper::get_instance();
			if (!cfg->get_auto_play_rec_if_available() || !record->zone_alarm_text_pairs_.back().second) break;

			record->rec_player = nullptr;
			record->rec_player = std::shared_ptr<CVideoRecordPlayerDlg>(new CVideoRecordPlayerDlg(this), rec_player_deleter);
			record->rec_player->device_ = std::dynamic_pointer_cast<video::jovision::jovision_device>(record->_device);
			record->rec_player->automatic_ = true;
			record->rec_player->Create(IDD_DIALOG_VIDEO_RECORD_PLAYER, this);
			record->rec_player->ShowWindow(SW_SHOW);

		} while (0);

		InsertList(record);
		record->connecting_ = false;
		record->started_ = true;
		auto device = std::dynamic_pointer_cast<video::jovision::jovision_device>(record->_device);
		auto zoneUuid = record->zone_alarm_text_pairs_.front().first;

		CString txt;
		txt.Format(L"%s(%s)-\"%s\"",
				   TR(IDS_STRING_VIDEO_START),
				   device->get_formatted_name().c_str(),
				   record->_param->_file_path);

		ipc::alarm_center_video_client::get_instance()->insert_record(zoneUuid._ademco_id, zoneUuid._zone_value, (LPCTSTR)txt);
	}
}

void CVideoPlayerDlg::on_jov_play_stop(const record_ptr & record)
{
	if (record) {
		auto device = std::dynamic_pointer_cast<video::jovision::jovision_device>(record->_device);
		auto zoneUuid = record->zone_alarm_text_pairs_.front().first;

		CString txt;
		txt.Format(L"%s(%s)-\"%s\"",
				   TR(IDS_STRING_VIDEO_STOP),
				   device->get_formatted_name().c_str(),
				   (record->_param->_file_path));

		ipc::alarm_center_video_client::get_instance()->insert_record(zoneUuid._ademco_id, zoneUuid._zone_value, (LPCTSTR)txt);

		std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_);
		record_list_.remove(record);
		player_op_recycle_player(record->player_);
		record->rec_player = nullptr;
		delete_from_play_list_by_record(record);
	}
}

void CVideoPlayerDlg::show_one_by_record(const record_ptr & info)
{
	if (!info->_device->get_userInfo()) {
		return;
	}

	CString txt;
	txt.Format(L"%s-(%s)", info->_device->get_userInfo()->get_user_name().c_str(), info->_device->get_formatted_name().c_str());
	m_static_group_cur_video.SetWindowTextW(txt);
	m_btn_voice_talk.EnableWindow(info->productor_ == video::EZVIZ);
	m_btn_voice_talk.SetWindowTextW(TR(info->voice_talking_ ? IDS_STRING_STOP_VOICE_TALK : IDS_STRING_IDC_BUTTON_VOICE_TALK));
	m_chk_volume.EnableWindow(info->voice_talking_);
	m_chk_volume.SetCheck(info->sound_opened_);
	m_slider_volume.EnableWindow(info->sound_opened_);
	if (info->voice_talking_) {
		int volume = video::ezviz::sdk_mgr_ezviz::get_instance()->m_dll.getVolume(info->_param->_session_id);
		m_slider_volume.SetPos(volume);
		txt.Format(L"%s:%d", TR(IDS_STRING_VOLUME), volume);
		m_static_volume.SetWindowTextW(txt);
	} else {
		m_slider_volume.SetPos(0);
		m_static_volume.SetWindowTextW(L"");
	}

	m_radioBalance.EnableWindow(info->productor_ == video::EZVIZ);
	m_radioSmooth.EnableWindow(info->productor_ == video::EZVIZ);
	m_radioHD.EnableWindow(info->productor_ == video::EZVIZ);
	m_btnUp.EnableWindow(info->productor_ == video::EZVIZ);
	m_btnDown.EnableWindow(info->productor_ == video::EZVIZ);
	m_btnLeft.EnableWindow(info->productor_ == video::EZVIZ);
	m_btnRight.EnableWindow(info->productor_ == video::EZVIZ);
	m_btn_remote_config.EnableWindow(info->productor_ == video::JOVISION);
	m_btn_open_rec.EnableWindow(info->productor_ == video::JOVISION);

	RefreshAlarmList(info);
}

void CVideoPlayerDlg::EnqueJovisionMsg(const jovision_msg_ptr & msg)
{
	AUTO_LOG_FUNCTION;
	std::lock_guard<std::mutex> lock(lock_4_jovision_msg_queue_);
	jovision_msg_list_.push_back(msg);
}

void CVideoPlayerDlg::HandleJovisionMsg(const jovision_msg_ptr & msg)
{
	const char* dwMsgID = 0;

	std::list<CString> appendix_msg_list = {};

	//switch (msg->etType) {
	//case JCET_GetFileListOK://获取远程录像成功
	//{
	//	/*g_RecFileInfoList.clear();
	//	PJCRecFileInfo pInfos = (PJCRecFileInfo)pData1;
	//	int nCount = (int)pData2;
	//	for (int i = 0; i < nCount; ++i) {
	//	g_RecFileInfoList.push_back(pInfos[i]);
	//	}*/
	//}
	//case JCET_GetFileListError://获取远程录像失败
	//{
	//	//safe_post_msg_to_g_player(WM_JC_GETRECFILELIST, msg->etType == JCET_GetFileListOK);
	//}
	////return;
	////break;
	//case JCET_StreamReset://码流重置信号
	////{
	////	sdk_mgr_jovision::get_instance()->enable_decoder(msg->nLinkID, FALSE);
	////	//safe_post_msg_to_g_player(WM_JC_RESETSTREAM);
	////	auto record = record_op_get_record_info_by_link_id(msg->nLinkID);
	////	if (record) {
	////		bool ok = false;
	////		CString strMsg;
	////		auto jmgr = sdk_mgr_jovision::get_instance();
	////		do {
	////			ok = jmgr->enable_decoder(msg->nLinkID, TRUE);
	////			if (!ok) {
	////				strMsg.Format(TR(IDS_EnableDecodeError), msg->nLinkID);
	////				appendix_msg_list.push_back(strMsg);
	////				break;
	////			}
	////			record->decoding_ = true;
	////			//strMsg.Format(TR(IDS_EnableDecodeOK), msg->nLinkID);
	////			//appendix_msg_list.push_back(strMsg);
	////			ok = jmgr->set_video_preview(msg->nLinkID, record->player_->GetRealHwnd(), record->player_->GetRealRect());
	////			if (!ok) {
	////				strMsg.Format(TR(IDS_EnablePreviewError), msg->nLinkID);
	////				appendix_msg_list.push_back(strMsg);
	////				break;
	////			}
	////			// start sound preview
	////			if (jmgr->set_audio_preview(msg->nLinkID, record->player_->GetRealHwnd())) {
	////				//strMsg.Format(TR(IDS_StartAudioOK), msg->nLinkID);
	////			} else {
	////				strMsg.Format(TR(IDS_StartAudioError), msg->nLinkID);
	////			}
	////			appendix_msg_list.push_back(strMsg);
	////			
	////			auto file = record->_param->_file_path;
	////			auto cfile = utf8::u16_to_mbcs(file);
	////			if (jmgr->start_record(msg->nLinkID, (char*)cfile.c_str())) {
	////				strMsg.Format(TR(IDS_StartRecOK), msg->nLinkID);
	////			} else {
	////				strMsg.Format(TR(IDS_StartRecError), msg->nLinkID);
	////			}
	////			appendix_msg_list.push_back(strMsg);
	////		} while (0);
	////		
	////		if (!ok) {
	////			auto hr = core::history_record_manager::get_instance();
	////			video::zone_uuid zone = { -1,-1,-1 };
	////			zone = record->zone_alarm_text_pairs_.front().first;
	////			for (auto s : appendix_msg_list) {
	////				hr->InsertRecord(zone._ademco_id, zone._zone_value, s, time(nullptr), core::RECORD_LEVEL_VIDEO);
	////			}
	////			on_jov_play_stop(record);
	////		} else {
	////			//on_jov_play_start(record);
	////		}
	////	}
	////}
	////return;
	//	// dispatch these msg to video rec dlg
	//{
	//	
	//}
	//
	//break;
	//}

	{
		bool handled = false;
		std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_);
		for (auto info : record_list_) {
			if (info->rec_player && msg->nLinkID == info->rec_player->link_id_) {
				info->rec_player->HandleJovisionMsg(msg);
				handled = true;
				return;
			}
		}

		if (handled) {
			return;
		}
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
						strMsg.Format(TR(IDS_EnableDecodeError), msg->nLinkID);
						appendix_msg_list.push_back(strMsg);
						break;
					}

					record->decoding_ = true;
					strMsg.Format(TR(IDS_EnableDecodeOK), msg->nLinkID);
					appendix_msg_list.push_back(strMsg);

					ok = jmgr->set_video_preview(msg->nLinkID, record->player_->GetRealHwnd(), record->player_->GetRealRect());
					if (!ok) {
						strMsg.Format(TR(IDS_EnablePreviewError), msg->nLinkID);
						appendix_msg_list.push_back(strMsg);
						break;
					}

					// start sound preview
					if (jmgr->set_audio_preview(msg->nLinkID, record->player_->GetRealHwnd())) {
						strMsg.Format(TR(IDS_StartAudioOK), msg->nLinkID);
					} else {
						strMsg.Format(TR(IDS_StartAudioError), msg->nLinkID);
						appendix_msg_list.push_back(strMsg);
					}
					appendix_msg_list.push_back(strMsg);

					// start record video
					/*if (record->zone_alarm_text_pairs_.size() == 1 && !record->zone_alarm_text_pairs_.front().second) {
						JLOGA("no alarm, no record");
						break;
					}*/
					std::string ext;
					JCStreamInfo info;
					if (jmgr->get_stream_info(msg->nLinkID, &info)) {
						switch (info.eRecFileType) {
						case JCRT_SV4:
							ext = "sv4";
							break;

						case JCRT_SV5:
							ext = "sv5";
							break;

						case JCRT_SV6:
							ext = "sv6";
							break;

						case JCRT_MP4:
							ext = "mp4";
							break;
						}
					}

					if (ext.empty()) {
						strMsg.Format(TR(IDS_StartRecError), msg->nLinkID);
					} else {
						auto file = record->_param->FormatFilePath(record->_device->get_userInfo()->get_id(),
																   record->_device->get_userInfo()->get_user_name(),
																   record->_device->get_formatted_name(),
																   utf8::a2w(ext).c_str());
						auto cfile = utf8::u16_to_mbcs((LPCTSTR)file);
						if (jmgr->start_record(msg->nLinkID, (char*)cfile.c_str())) {
							strMsg.Format(TR(IDS_StartRecOK), msg->nLinkID);
						} else {
							strMsg.Format(TR(IDS_StartRecError), msg->nLinkID);
						}

						appendix_msg_list.push_back(strMsg);
					}

				} while (0);

				if (!ok) {
					record->decoding_ = false;
					record->previewing_ = false;
				}
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
	strMsg.Format(TR(dwMsgID), msg->nLinkID, wMsg.c_str());
	appendix_msg_list.push_front(strMsg);

	video::zone_uuid zoneUuid = { -1,-1,-1 };
	auto record = record_op_get_record_info_by_link_id(msg->nLinkID);
	if (record) {
		zoneUuid = record->zone_alarm_text_pairs_.front().first;
	}

	for (auto s : appendix_msg_list) {
		ipc::alarm_center_video_client::get_instance()->insert_record(zoneUuid._ademco_id, zoneUuid._zone_value, (LPCTSTR)s);
	}

	if (!ok) {
		on_jov_play_stop(record);
	} else {
		on_jov_play_start(record);
	}
	busy_ = false;
}


void CVideoPlayerDlg::PlayVideoEzviz(video::ezviz::ezviz_device_ptr device, int videoLevel, const video::zone_uuid_ptr& zid, const core::alarm_text_ptr& at)
{
	AUTO_LOG_FUNCTION;
	assert(device);
	do {
		auto rec_info = record_op_get_record_info_by_device(device);
		if (rec_info) { // playing
			if (videoLevel == rec_info->_level) { // same level
				if (zid && at) {
					rec_info->zone_alarm_text_pairs_.push_back(zone_alarm_text_pair(*zid, at));
				}
				EnableControlPanel(TRUE, videoLevel);
				rec_info->_param->_start_time = GetTickCount();
				player_op_bring_player_to_front(rec_info->player_);
				RefreshAlarmList(rec_info);
				m_curPlayingDevice = device;
				return;
			} else { // different level, stop it and re-play it with new level
				StopPlayEzviz(device);
			}
		} else { // not playing
			if (record_list_.size() > 9) {
				for (auto info : record_list_) {
					if (info->_device != m_curPlayingDevice) {
						StopPlayByRecordInfo(info);
						break;
					}
				}
			}
		}
		
		m_curPlayingDevice = device;
		auto user = std::dynamic_pointer_cast<video::ezviz::ezviz_user>(device->get_userInfo());  assert(user);
		auto mgr = video::ezviz::sdk_mgr_ezviz::get_instance();
		auto vmgr = video::video_manager::get_instance();
		CString e;
		if (user->get_acc_token().size() == 0) {
			if (video::ezviz::sdk_mgr_ezviz::RESULT_OK != mgr->VerifyUserAccessToken(user, TYPE_GET)) {
				e = TR(IDS_STRING_PRIVATE_CLOUD_CONN_FAIL_OR_USER_NOT_EXSIST);
				MessageBox(e, L"", MB_ICONINFORMATION);
				break;
			}
			vmgr->execute_set_ezviz_users_acc_token(user, user->get_acc_token());
		}
		bool bEncrypt = false;
		int ret = mgr->m_dll.UpdateCameraInfo(device->get_cameraId(), user->get_acc_token(), bEncrypt);
		if (ret != 0) {
			e = TR(IDS_STRING_UPDATE_CAMERA_INFO_FAILED);
			ipc::alarm_center_video_client::get_instance()->insert_record(-1, 0, (LPCTSTR)e);
			MessageBox(e, L"", MB_ICONINFORMATION);
			break;
		}

		if (bEncrypt && device->get_secure_code().size() != 6) {
			CInputDeviceVerifyCodeDlg dlg(this);
			if (dlg.DoModal() != IDOK) {
				break;
			}
			device->set_secure_code(utf8::w2a((LPCTSTR)dlg.m_result));
			vmgr->execute_update_dev(device);
		}
		std::string session_id = mgr->GetSessionId(user->get_user_phone(), device->get_cameraId(), messageHandler, this);
		if (session_id.empty()) {
			assert(0);
			break;
		}
		DataCallbackParamEzviz *param = new DataCallbackParamEzviz(this, session_id, /*time(nullptr)*/ GetTickCount());
		CString filePath = param->FormatFilePath(device->get_userInfo()->get_id(), device->get_userInfo()->get_user_name(),
												 device->get_formatted_name());
		mgr->m_dll.setDataCallBack(session_id, videoDataHandler, param);

		auto player = player_op_create_new_player();
		{
			ret = mgr->m_dll.startRealPlay(session_id,
										   player->GetRealHwnd(),
										   device->get_cameraId(),
										   user->get_acc_token(),
										   device->get_secure_code(),
										   util::CConfigHelper::get_instance()->get_ezviz_app_key(),
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
											   util::CConfigHelper::get_instance()->get_ezviz_app_key(),
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
			//video::zone_uuid zoneUuid = device->GetActiveZoneUuid();		
			video::zone_uuid zoneUuid = {};
			if (zid) {
				zoneUuid = *zid;
			} else {
				zoneUuid = device->GetActiveZoneUuid();
			}
			record_ptr info = std::make_shared<record>(param, zoneUuid, device, at, player, videoLevel);
			record_list_.push_back(info);
		}
		UpdateWindow();
		return;
	} while (0);
	JLOG(L"PlayVideo failed\n");
	m_curPlayingDevice = nullptr;
	UpdateWindow();
}


void CVideoPlayerDlg::PlayVideoJovision(video::jovision::jovision_device_ptr device, int videoLevel, const video::zone_uuid_ptr& zid, const core::alarm_text_ptr& at)
{
	AUTO_LOG_FUNCTION;
	assert(device);
	do {
		auto rec_info = record_op_get_record_info_by_device(device);
		if (rec_info) { // playing
			if (rec_info->connecting_) { // last time previewing not done.
				if (zid && at) {
					rec_info->zone_alarm_text_pairs_.push_back(zone_alarm_text_pair(*zid, at));
				}
				RefreshAlarmList(rec_info);
				return;
			}

			if (videoLevel == rec_info->_level) { // same level
				if (zid && at) {
					rec_info->zone_alarm_text_pairs_.push_back(zone_alarm_text_pair(*zid, at));
				}
				EnableControlPanel(TRUE, videoLevel);
				rec_info->_param->_start_time = GetTickCount();
				player_op_bring_player_to_front(rec_info->player_);
				RefreshAlarmList(rec_info);
				m_curPlayingDevice = device;
				return;
			} else { // different level, stop it and re-play it with new level
				StopPlayJovision(device);
			}
		} else { // not playing
			if (record_list_.size() > 9) {
				for (auto info : record_list_) {
					if (info->_device != m_curPlayingDevice) {
						StopPlayByRecordInfo(info);
						break;
					}
				}
			}
		}

		m_curPlayingDevice = device;
		auto user = std::dynamic_pointer_cast<video::jovision::jovision_user>(device->get_userInfo()); assert(user);
		
		auto jov = jovision::sdk_mgr_jovision::get_instance();
		jovision::JCLink_t link_id = -1;

		if (device->get_by_sse()) {
			link_id = jov->connect(const_cast<char*>(device->get_sse().c_str()), 0, device->get_channel_num(),
								   const_cast<char*>(utf8::w2a(device->get_user_name()).c_str()),
								   const_cast<char*>(device->get_user_passwd().c_str()),
								   1, nullptr);
		} else {
			link_id = jov->connect(const_cast<char*>(device->get_ip().c_str()), device->get_port(), device->get_channel_num(),
								   const_cast<char*>(utf8::w2a(device->get_user_name()).c_str()),
								   const_cast<char*>(device->get_user_passwd().c_str()),
								   1, nullptr);
		}

		video::zone_uuid zoneUuid = {};
		if (zid) {
			zoneUuid = *zid;
		} else {
			zoneUuid = device->GetActiveZoneUuid();
		}
		
		if (link_id == -1) {
			JLOG(L"startRealPlay failed link_id %d\n", link_id);
			m_curPlayingDevice = nullptr;
			//SAFEDELETEP(param);
			//player_buffer_.push_back(player);
			ipc::alarm_center_video_client::get_instance()->insert_record(zoneUuid._ademco_id, zoneUuid._zone_value, tr(IDS_ConnectError));
			busy_ = false;
		} else {
			JLOG(L"PlayVideo ok\n");
			EnableControlPanel(TRUE, videoLevel);
			std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_);
			DataCallbackParamEzviz *param = new DataCallbackParamEzviz(this, "", GetTickCount());
			record_ptr info = std::make_shared<record>(param, zoneUuid, device, at, player_op_create_new_player(), videoLevel);
			info->connecting_ = true;
			info->link_id_ = link_id;
			record_list_.push_back(info);

			CString strMsg;
			strMsg.Format(TR(IDS_Connecting), link_id);
			ipc::alarm_center_video_client::get_instance()->insert_record(zoneUuid._ademco_id, zoneUuid._zone_value, (LPCTSTR)strMsg);
			busy_ = true;

		}
		UpdateWindow();
		return;
	} while (0);
	JLOG(L"PlayVideo failed\n");
	m_curPlayingDevice = nullptr;
	UpdateWindow();
}


bool CVideoPlayerDlg::StopPlayEzviz(video::ezviz::ezviz_device_ptr device)
{
	AUTO_LOG_FUNCTION;
	assert(device);
	std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_);
	auto user = std::dynamic_pointer_cast<video::ezviz::ezviz_user>(device->get_userInfo()); 
	if (!user) {
		return false;
	}
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
	return true;
}


bool CVideoPlayerDlg::StopPlayJovision(video::jovision::jovision_device_ptr device)
{
	AUTO_LOG_FUNCTION;
	assert(device);
	std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_);
	auto record = record_op_get_record_info_by_device(device);
	if (record) {
		sdk_mgr_jovision::get_instance()->stop_record(record->link_id_);
		sdk_mgr_jovision::get_instance()->disconnect(record->link_id_);
		return true;
	}

	return false;
}


void CVideoPlayerDlg::delete_from_play_list_by_record(const record_ptr& record)
{
	if (!record)return;

	for (int i = 0; i < m_ctrl_play_list.GetItemCount(); i++) {
		video_device_identifier* data = reinterpret_cast<video_device_identifier*>(m_ctrl_play_list.GetItemData(i));
		if (data && record->_device && data->productor_type == record->_device->get_userInfo()->get_productor().get_productor_type() 
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
	g_videoPlayerDlg = nullptr;

	player_ex_vector_.clear();
	back_end_players_.clear();
	player_buffer_.clear();

	for (int i = 0; i < m_ctrl_play_list.GetItemCount(); i++) {
		video_device_identifier* data = reinterpret_cast<video_device_identifier*>(m_ctrl_play_list.GetItemData(i));
		delete data;
	}
	
	for (int i = 0; i < m_list_all_devs.GetItemCount(); i++) {
		video_device_identifier* data = reinterpret_cast<video_device_identifier*>(m_list_all_devs.GetItemData(i));
		delete data;
	}

	video::video_manager::release_singleton();
	video::ezviz::sdk_mgr_ezviz::release_singleton();
	video::jovision::sdk_mgr_jovision::release_singleton();
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
		auto_timer timer(m_hWnd, TIMER_ID_PLAY_VIDEO, 2000);
		if (!busy_ && !m_wait2playDevList.empty()) {
			wait_to_play_dev_ptr waiting_dev = nullptr;
			if (m_lock4Wait2PlayDevList.try_lock()) {
				std::lock_guard<std::mutex> lock(m_lock4Wait2PlayDevList, std::adopt_lock);
				waiting_dev = m_wait2playDevList.front();
				m_wait2playDevList.pop_front();
			}
			
			if (waiting_dev) {
				JLOG(L"ontimer TIMER_ID_PLAY_VIDEO, PlayVideoByDevice");
				PlayVideoByDevice(waiting_dev->device, 
								  util::CConfigHelper::get_instance()->get_default_video_level(), 
								  waiting_dev->zone_uuid, 
								  waiting_dev->alarm_text);
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


bool CVideoPlayerDlg::StopPlayByRecordInfo(record_ptr info)
{
	AUTO_LOG_FUNCTION;

	if (m_curPlayingDevice == info->_device) {
		m_curPlayingDevice = nullptr;
		EnableControlPanel(0);
	}

	if (info->productor_ == EZVIZ) {
		return StopPlayEzviz(std::dynamic_pointer_cast<video::ezviz::ezviz_device>(info->_device));
	} else if (info->productor_ == JOVISION) {
		return StopPlayJovision(std::dynamic_pointer_cast<video::jovision::jovision_device>(info->_device));
	} else {
		assert(0);
	}

	return false;
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
		auto user = m_curPlayingDevice->get_userInfo();
		auto productor = user->get_productor().get_productor_type();
		if (video::EZVIZ == productor) {
			auto device = std::dynamic_pointer_cast<ezviz_device>(m_curPlayingDevice);
			auto mgr = video::ezviz::sdk_mgr_ezviz::get_instance();
			CString path, file, fm, txt;
			path.Format(L"%s\\data\\video_capture", get_exe_path().c_str());
			file.Format(L"\\%s-%s.jpg", device->get_formatted_name().c_str(),
						CTime::GetCurrentTime().Format(L"%Y-%m-%d-%H-%M-%S"));
			CreateDirectory(path, nullptr);
			path += file;
			fm = TR(IDS_STRING_FM_CAPTURE_OK);
			txt.Format(fm, path);
			std::string name = utf8::w2a((LPCTSTR)path);
			auto session_id = mgr->GetSessionId(user->get_user_phone(), device->get_cameraId(), messageHandler, this);
			if (!session_id.empty() && 0 == mgr->m_dll.capturePicture(session_id, name)) {
				MessageBox(txt);
			}
		} else if (video::JOVISION == productor) {
			auto device = std::dynamic_pointer_cast<jovision_device>(m_curPlayingDevice);
			auto jmgr = video::jovision::sdk_mgr_jovision::get_instance();
			CString path, file, fm, txt;
			path.Format(L"%s\\data\\video_capture", get_exe_path().c_str());
			file.Format(L"\\%s-%s.bmp", integrate_path(device->get_formatted_name()).c_str(),
						CTime::GetCurrentTime().Format(L"%Y-%m-%d-%H-%M-%S"));
			CreateDirectory(path, nullptr);
			path += file;
			fm = TR(IDS_STRING_FM_CAPTURE_OK);
			txt.Format(fm, path);
			std::string name = utf8::w2a((LPCTSTR)path);

			auto info = record_op_get_record_info_by_device(device);
			if(info && jmgr->save_bitmap(info->link_id_, const_cast<char*>(name.c_str()))) {
				MessageBox(txt);
			}
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
		auto device = std::dynamic_pointer_cast<video::ezviz::ezviz_device>(m_curPlayingDevice);
		if (!device) { return; }
		auto user = std::dynamic_pointer_cast<video::ezviz::ezviz_user>(device->get_userInfo()); assert(user);
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


void CVideoPlayerDlg::PlayVideo(const video::zone_uuid_ptr& zone, const core::alarm_text_ptr& at)
{
	AUTO_LOG_FUNCTION;
	assert(zone);
	video::bind_info bi = video::video_manager::get_instance()->GetBindInfo(*zone);
	if (bi._device && bi.auto_play_when_alarm_) {
		std::lock_guard<std::mutex> lock(m_lock4Wait2PlayDevList);
		bi._device->SetActiveZoneUuid(*zone);
		m_wait2playDevList.push_back(std::make_shared<wait_to_play_dev>(bi._device, zone, at));
	}
}

void CVideoPlayerDlg::PlayVideo(const video::device_ptr & device)
{
	AUTO_LOG_FUNCTION;
	assert(device);
	if (device) {
		std::lock_guard<std::mutex> lock(m_lock4Wait2PlayDevList);
		m_wait2playDevList.push_back(std::make_shared<wait_to_play_dev>(device, nullptr, nullptr));
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


//void CVideoPlayerDlg::OnBnClickedButtonSave()
//{
//	CString txt;
//	m_ctrl_rerord_minute.GetWindowTextW(txt);
//	int minutes = _ttoi(txt);
//	if (minutes <= 0) {
//		minutes = 10;
//	}
//	util::CConfigHelper::get_instance()->set_back_end_record_minutes(minutes);
//	txt.Format(L"%d", minutes);
//	m_ctrl_rerord_minute.SetWindowTextW(txt);
//}


void CVideoPlayerDlg::ClearAlarmList()
{
	for (int i = 0; i < m_list_alarm.GetCount(); i++) {
		auto zid = reinterpret_cast<video::zone_uuid*>(m_list_alarm.GetItemData(i));
		delete zid;
	}
	m_list_alarm.ResetContent();
}

void CVideoPlayerDlg::RefreshAlarmList(const record_ptr& info)
{
	ClearAlarmList();
	CString txt;
	for (auto iter : info->zone_alarm_text_pairs_) {
		if (!iter.second) continue;
		auto zid = new video::zone_uuid(iter.first);

		txt.Format(L"%06d", zid->_ademco_id);

		txt += iter.second->_txt;
		int ndx = m_list_alarm.AddString(txt);
		m_list_alarm.SetItemData(ndx, reinterpret_cast<DWORD_PTR>(zid));
		m_list_alarm.SetCurSel(ndx);
	}
}

void CVideoPlayerDlg::InsertList(const record_ptr& info)
{
	if (!info) return;

	for (int i = 0; i < m_ctrl_play_list.GetItemCount(); i++) {
		video_device_identifier* data = reinterpret_cast<video_device_identifier*>(m_ctrl_play_list.GetItemData(i));
		if (info->_device->get_userInfo() && data->productor_type == info->_device->get_userInfo()->get_productor().get_productor_type()
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
		tmp.Format(_T("%s"), info->_device->get_userInfo()->get_productor().get_formatted_name().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_ctrl_play_list.SetItem(&lvitem);
		tmp.UnlockBuffer();		
	}

	video_device_identifier* data = new video_device_identifier();
	data->productor_type = info->_device->get_userInfo()->get_productor().get_productor_type();
	data->dev_id = info->_device->get_id();
	m_ctrl_play_list.SetItemData(nResult, reinterpret_cast<DWORD_PTR>(data));
	m_ctrl_play_list.SetItemState(nResult, LVNI_FOCUSED | LVIS_SELECTED, LVNI_FOCUSED | LVIS_SELECTED);

	show_one_by_record(info);
}


void CVideoPlayerDlg::OnLvnItemchangedList1(NMHDR * pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;
	if (pNMItemActivate->iItem < 0)return;
}


void CVideoPlayerDlg::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;
	if (pNMItemActivate->iItem < 0)return;
	video_device_identifier* data = reinterpret_cast<video_device_identifier*>(m_ctrl_play_list.GetItemData(pNMItemActivate->iItem));
	
	std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_);
	for (auto info : record_list_) {
		if (data->productor_type == info->_device->get_userInfo()->get_productor().get_productor_type()
			&& data->dev_id == info->_device->get_id()) {

			PlayVideoByDevice(info->_device, info->_level);

			show_one_by_record(info);

			break;
		}
	}
	
}


void CVideoPlayerDlg::OnNMDblclkListAlldev(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	if (pNMItemActivate->iItem < 0)return;
	video_device_identifier* data = reinterpret_cast<video_device_identifier*>(m_list_all_devs.GetItemData(pNMItemActivate->iItem));

	auto dev = video::video_manager::get_instance()->GetVideoDeviceInfo(data);
	if (dev) {
		PlayVideoByDevice(dev, util::CConfigHelper::get_instance()->get_default_video_level());
	}
}


void CVideoPlayerDlg::OnEnChangeEditMinute()
{
	
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

	CRect rc;
	m_player.GetWindowRect(rc);
	ScreenToClient(rc);
	auto v = split_rect(rc, player_count, 1);
	for (int i = 0; i < player_count - 1; i++) {
		player_ex_vector_[i] = player_ex_vector_[i + 1];
		player_ex_vector_[i]->player->MoveWindow(&v[i]);
		player_ex_vector_[i]->player->ShowWindow(SW_SHOW);
	}

	player_ex_0->player->ShowWindow(SW_HIDE);
	//delete_from_play_list_by_record(record_op_get_record_info_by_player(player_ex_0->player));
	back_end_players_.insert(player_ex_0->player);

	player->MoveWindow(&v[player_count - 1]);
	player->ShowWindow(SW_SHOW);
	player_ex_0->player = player;
	player_ex_0->used = true;

	player_ex_vector_[player_count - 1] = player_ex_0;

	// show player, add a item to play list
	InsertList(record_op_get_record_info_by_player(player));

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

	bool recycled = false;
	const int n = util::CConfigHelper::get_instance()->get_show_video_same_time_route_count();
	for (int i = 0; i < n; i++) {
		auto& player_ex = player_ex_vector_[i];
		if (player_ex->player == player) { // playing in front-end, delete its list item
			//delete_from_play_list_by_record(record_op_get_record_info_by_player(player_ex->player));

			player_ex->player->Invalidate();
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
	int ndx = 0;
	for (auto player_ex : player_ex_vector_) {
		if (player_ex.second->used) {
			use_count++;
		}
		player_ex.second->player->set_index(++ndx);
	}
	if (use_count == 0)return;
	use_count = 0;
	auto v = split_rect(rc, n, 1);
	for (size_t i = 0; i < n; i++) {
		auto& player_ex = player_ex_vector_[i];
		if (player_ex->used) {
			player_ex->player->MoveWindow(&v[use_count]);
			player_ex->player->ShowWindow(SW_SHOW);
			player_ex->player->set_index(i + 1);
			use_count++;

			auto record = record_op_get_record_info_by_player(player_ex->player);
			if (record && record->productor_ == video::JOVISION) {
				video::jovision::sdk_mgr_jovision::get_instance()->set_video_preview(record->link_id_, 
																					 player_ex->player->GetRealHwnd(), 
																					 player_ex->player->GetRealRect());
			}

		} else {
			//delete_from_play_list_by_record(record_op_get_record_info_by_player(player_ex->player));
		}
	}

	for (size_t i = use_count; i < n; i++) {
		auto& player_ex = player_ex_vector_[i];
		player_ex->player->MoveWindow(&v[i]);
		player_ex->player->ShowWindow(SW_SHOW);
		player_ex->player->set_index(i + 1);
		player_ex_vector_[i]->used = false;
	}
}


void CVideoPlayerDlg::player_op_update_players_size_with_m_player()
{
	CRect rc;
	m_player.GetWindowRect(rc);
	ScreenToClient(rc);

	const int n = util::CConfigHelper::get_instance()->get_show_video_same_time_route_count();
	auto v = split_rect(rc, n, 1);

	auto jmgr = video::jovision::sdk_mgr_jovision::get_instance();
	for (int i = 0; i < n; i++) {
		player_ex_vector_[i]->player->MoveWindow(&v[i]);
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

	auto user = std::dynamic_pointer_cast<video::ezviz::ezviz_user>(record->_device->get_userInfo());
	if (!user)return;

	auto& mgr = video::ezviz::sdk_mgr_ezviz::get_instance()->m_dll;
	if (record->voice_talking_) {
		int ret = mgr.stopVoiceTalk(record->_param->_session_id);
		if (ret == 0) {
			mgr.closeSound(record->_param->_session_id);
			m_btn_voice_talk.SetWindowTextW(TR(IDS_STRING_IDC_BUTTON_VOICE_TALK));
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
			MessageBox(e + TR(IDS_STRING_STOP_VOICE_TALK_FAIL), L"", MB_ICONERROR);
		}
	} else {
		int ret = mgr.startVoiceTalk(record->_param->_session_id, user->get_acc_token(), std::dynamic_pointer_cast<video::ezviz::ezviz_device>(record->_device)->get_cameraId());
		if (ret == 0 || ret == -2) {
			mgr.openSound(record->_param->_session_id);
			record->voice_talking_ = true;
			record->sound_opened_ = true;
			JLOG(L"start voice talk ok");
			m_btn_voice_talk.SetWindowTextW(TR(IDS_STRING_STOP_VOICE_TALK));
			m_chk_volume.EnableWindow();
			m_chk_volume.SetCheck(1);
			int vol = mgr.getVolume(record->_param->_session_id);
			m_slider_volume.SetPos(vol);
			m_slider_volume.EnableWindow();
			CString txt;
			txt.Format(L"%s:%d", TR(IDS_STRING_VOLUME), vol);
			m_static_volume.SetWindowTextW(txt);
		} else if (ret != 0) {
			CString e;
			e.Format(L"ErrorCode:%d\r\n", ret);
			MessageBox(e + TR(IDS_STRING_START_VOICE_TALK_FAIL), L"", MB_ICONERROR);
		}
	}
}


void CVideoPlayerDlg::OnTRBNThumbPosChangingSliderVolume(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMTRBTHUMBPOSCHANGING *pNMTPC = reinterpret_cast<NMTRBTHUMBPOSCHANGING *>(pNMHDR);
	CString txt;
	txt.Format(L"%s:%d", TR(IDS_STRING_VOLUME), pNMTPC->dwPos);
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

	auto user = std::dynamic_pointer_cast<video::ezviz::ezviz_user>(record->_device->get_userInfo());
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
		txt.Format(L"%s:%d", TR(IDS_STRING_VOLUME), vol);
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
	txt.Format(L"%s:%d", TR(IDS_STRING_VOLUME), pos);
	m_static_volume.SetWindowTextW(txt);
}


void CVideoPlayerDlg::OnBnClickedButtonRemoteConfig()
{
	if (!m_curPlayingDevice) {
		return;
	}

	auto record = record_op_get_record_info_by_device(m_curPlayingDevice);
	if (!record || record->productor_ != video::JOVISION)return;

	int l = 0;
	auto lang = util::CConfigHelper::get_instance()->get_language();
	switch (lang) {
	case util::AL_ENGLISH:
		l = 1;
		break;
	}

	try {
		sdk_mgr_jovision::get_instance()->remote_config(record->link_id_, l);
	} catch (...) {

	}
}


void CVideoPlayerDlg::OnBnClickedCheckAutoPlayRec()
{
	int b = m_chk_auto_play_rec.GetCheck();
	util::CConfigHelper::get_instance()->set_auto_play_rec_if_available(b);
}


void CVideoPlayerDlg::OnBnClickedButtonOpenRec()
{
	if (!m_curPlayingDevice) {
		return;
	}

	auto record = record_op_get_record_info_by_device(m_curPlayingDevice);
	if (!record || record->productor_ != video::JOVISION)return;

	if (!record->rec_player) {
		record->rec_player = std::shared_ptr<CVideoRecordPlayerDlg>(new CVideoRecordPlayerDlg(this), rec_player_deleter);
		record->rec_player->device_ = std::dynamic_pointer_cast<video::jovision::jovision_device>(record->_device);
		record->rec_player->Create(IDD_DIALOG_VIDEO_RECORD_PLAYER, this);
	}

	record->rec_player->ShowWindow(SW_SHOW);

	// test
	//player_op_bring_player_to_front(record->player_);
}


void CVideoPlayerDlg::OnLbnSelchangeListZone()
{
	//int ndx = m_list_alarm.GetCurSel(); if (ndx < 0) return;
	//auto zid = reinterpret_cast<video::zone_uuid*>(m_list_alarm.GetItemData(ndx));

}


void CVideoPlayerDlg::OnBnClickedButtonStopAll()
{
	bool failed = false;
	for (auto info : record_list_) {
		if (!StopPlayByRecordInfo(info)) {
			failed = true;
		}
	}
#ifdef _DEBUG
	//video_user_mgr_dlg_->ShowWindow(SW_SHOW);
#endif // _DEBUG

	if (failed) {
		record_list_.clear();
		m_curPlayingDevice = nullptr;
		player_ex_vector_.clear();
		back_end_players_.clear();
		player_buffer_.clear();
		CRect rc;
		m_player.GetWindowRect(rc);
		ScreenToClient(rc);
		const int same_time_play_vidoe_route_count = util::CConfigHelper::get_instance()->get_show_video_same_time_route_count();
		for (int i = 0; i < same_time_play_vidoe_route_count; i++) {
			auto a_player_ex = std::make_shared<player_ex>();
			a_player_ex->player = std::shared_ptr<CVideoPlayerCtrl>(new CVideoPlayerCtrl(), player_deleter);
			a_player_ex->player->set_index(i + 1);
			a_player_ex->player->Create(nullptr, m_dwPlayerStyle, rc, this, IDC_STATIC_PLAYER);
			player_ex_vector_[i] = (a_player_ex);
		}
		player_op_set_same_time_play_video_route(util::CConfigHelper::get_instance()->get_show_video_same_time_route_count());
		for (int i = 0; i < m_ctrl_play_list.GetItemCount(); i++) {
			video_device_identifier* data = reinterpret_cast<video_device_identifier*>(m_ctrl_play_list.GetItemData(i));
			delete data;
		}
		m_ctrl_play_list.DeleteAllItems();
	}
}


void CVideoPlayerDlg::RefreshDevList()
{
	for (int i = 0; i < m_list_all_devs.GetItemCount(); i++) {
		video_device_identifier* data = reinterpret_cast<video_device_identifier*>(m_list_all_devs.GetItemData(i));
		delete data;
	}
	m_list_all_devs.DeleteAllItems();
	auto mgr = video::video_manager::get_instance();
	video::user_list user_list;
	mgr->GetVideoUserList(user_list);
	for (auto user : user_list) {
		auto productor_type = user->get_productor().get_productor_type();
		for (auto dev : user->get_device_list()) {
			int nResult = -1;
			LV_ITEM lvitem = { 0 };
			CString tmp = _T("");

			lvitem.lParam = dev->get_id();
			lvitem.mask = LVIF_TEXT;
			lvitem.iItem = m_list_all_devs.GetItemCount();
			lvitem.iSubItem = 0;

			// ID
			tmp.Format(_T("%d"), dev->get_id());
			lvitem.pszText = tmp.LockBuffer();
			nResult = m_list_all_devs.InsertItem(&lvitem);
			tmp.UnlockBuffer();

			if (nResult != -1) {
				// 用户
				lvitem.iItem = nResult;
				lvitem.iSubItem++;
				tmp.Format(_T("%s"), user->get_user_name().c_str());
				lvitem.pszText = tmp.LockBuffer();
				m_list_all_devs.SetItem(&lvitem);
				tmp.UnlockBuffer();

				// 备注
				lvitem.iSubItem++;
				tmp.Format(_T("%s"), dev->get_device_note().c_str());
				lvitem.pszText = tmp.LockBuffer();
				m_list_all_devs.SetItem(&lvitem);
				tmp.UnlockBuffer();

				// productor
				lvitem.iSubItem++;
				tmp.Format(_T("%s"), user->get_productor().get_formatted_name().c_str());
				lvitem.pszText = tmp.LockBuffer();
				m_list_all_devs.SetItem(&lvitem);
				tmp.UnlockBuffer();
			}

			video_device_identifier* data = new video_device_identifier();
			data->productor_type = productor_type;
			data->dev_id = dev->get_id();
			m_list_all_devs.SetItemData(nResult, reinterpret_cast<DWORD_PTR>(data));
			m_list_all_devs.SetItemState(nResult, LVNI_FOCUSED | LVIS_SELECTED, LVNI_FOCUSED | LVIS_SELECTED);
		}
	}
}


afx_msg LRESULT CVideoPlayerDlg::OnMsgVideoChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	AUTO_LOG_FUNCTION;
	RefreshDevList();
	return 0;
}


void CVideoPlayerDlg::OnClose()
{
#ifdef _DEBUG
	CDialogEx::OnClose();
#else
	ShowWindow(SW_HIDE);
#endif
}

LRESULT CVideoPlayerDlg::OnMsgShowVideoUserMgrDlg(WPARAM wParam, LPARAM lParam)
{
	if (wParam) {
		ShowWindow(SW_SHOW);
		video_user_mgr_dlg_->ShowWindow(SW_SHOW);
	}
	
	video_user_mgr_dlg_->OnUserLevelChanged(lParam);

	return 0;
}


void CVideoPlayerDlg::OnBnClickedButtonSaveTime()
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
