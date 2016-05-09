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
	ezviz_msg_ptr msg = std::make_shared<ezviz_msg>(iMsgType, iErrorCode, szSessionId, pMessageInfo ? pMessageInfo : "");
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
			JLOG(L"pUser == nullptr");
			return;
		}

		DataCallbackParam* param = reinterpret_cast<DataCallbackParam*>(pUser); assert(param);
		if (!g_player || !g_player->record_op_is_valid(param)) {
			JLOG(L"g_player == nullptr");
			return;
		}

		////COleDateTime now = COleDateTime::GetCurrentTime();
		//DWORD now = GetTickCount();
		////COleDateTimeSpan span = now - param->_startTime;
		//DWORD span = now - param->_start_time;
		//if (/*span.GetTotalMinutes()*/ span / 1000 / 60 >= (DWORD)util::CConfigHelper::GetInstance()->get_back_end_record_minutes()) {
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



void CVideoPlayerDlg::OnCurUserChangedResult(const core::user_info_ptr& user)
{
	assert(user);
	if (user->get_user_priority() == core::UP_OPERATOR) {
		m_btn_save.EnableWindow(0);
	} else {
		m_btn_save.EnableWindow(1);
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
	USES_CONVERSION;

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

	case CSdkMgrEzviz::INS_PLAY_START:
		on_ins_play_start(cur_info);
		break;

	case CSdkMgrEzviz::INS_PLAY_STOP:
		on_ins_play_stop(cur_info);
		break;

	case CSdkMgrEzviz::INS_PLAY_EXCEPTION:
		on_ins_play_exception(msg, cur_info);
		break;

	case CSdkMgrEzviz::INS_PLAY_RECONNECT:
		break;
	case CSdkMgrEzviz::INS_PLAY_RECONNECT_EXCEPTION:
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
	JLOG(sInfo);
}


void CVideoPlayerDlg::on_ins_play_start(const record_ptr& record)
{
	USES_CONVERSION;
	if (record && !record->started_) {
		player_op_bring_player_to_front(record->player_);
		InsertList(record);
		record->started_ = true;
		auto device = record->_device;
		auto zoneUuid = record->_zone;

		CString txt;
		txt.Format(L"%s([%d,%s]%s)-\"%s\"", 
				   GetStringFromAppResource(IDS_STRING_VIDEO_START),
				   device->get_id(),
				   device->get_device_note().c_str(),
				   A2W(device->get_deviceSerial().c_str()),
				   record->_param->_file_path);

		core::history_record_manager* hr = core::history_record_manager::GetInstance();
		hr->InsertRecord(zoneUuid._ademco_id, zoneUuid._zone_value,
						 txt, time(nullptr), core::RECORD_LEVEL_VIDEO);
	}
}


void CVideoPlayerDlg::on_ins_play_stop(const record_ptr& record)
{
	USES_CONVERSION;

	if (record) {
		auto device = record->_device;
		auto zoneUuid = record->_zone;

		CString txt;
		txt.Format(L"%s([%d,%s]%s)-\"%s\"",
				   GetStringFromAppResource(IDS_STRING_VIDEO_STOP),
				   device->get_id(),
				   device->get_device_note().c_str(),
				   A2W(device->get_deviceSerial().c_str()),
				   (record->_param->_file_path));
		
		core::history_record_manager* hr = core::history_record_manager::GetInstance();
		hr->InsertRecord(zoneUuid._ademco_id, zoneUuid._zone_value,
						 txt, time(nullptr), core::RECORD_LEVEL_VIDEO);

		std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_);
		record_list_.remove(record);
		player_op_recycle_player(record->player_);
		delete_from_play_list_by_record(record);
		video::ezviz::CSdkMgrEzviz* mgr = video::ezviz::CSdkMgrEzviz::GetInstance();
		mgr->FreeSession(record->_param->_session_id);
	}
}


void CVideoPlayerDlg::on_ins_play_exception(const ezviz_msg_ptr& msg, const record_ptr& record)
{
	USES_CONVERSION;

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
		e = GetStringFromAppResource(IDS_STRING_OPERATIONCODE_FAILED);
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
		e = A2W(msg->messageInfo.c_str());
		break;
	}

	/*if (record) {
		on_ins_play_stop(record);
	} else {*/
		auto mgr = video::ezviz::CSdkMgrEzviz::GetInstance();
		mgr->m_dll.stopRealPlay(msg->sessionId);
	//}

	sInfo.AppendFormat(L"\r\n%s", e);
	MessageBox(sInfo, GetStringFromAppResource(IDS_STRING_PLAY_EXCEPTION), MB_ICONINFORMATION);
}


// CVideoPlayerDlg dialog
CVideoPlayerDlg* g_videoPlayerDlg = nullptr;
IMPLEMENT_DYNAMIC(CVideoPlayerDlg, CDialogEx)

CVideoPlayerDlg::CVideoPlayerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CVideoPlayerDlg::IDD, pParent)
	, m_bInitOver(FALSE)
	, m_curPlayingDevice(nullptr)
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
	DDX_Control(pDX, IDC_STATIC_CUR_VIDEO, m_static_group_cur_video);
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
		auto a_player_ex = std::make_shared<player_ex>();
		a_player_ex->player = std::make_shared<CVideoPlayerCtrl>();
		a_player_ex->player->ndx_ = i + 1;
		a_player_ex->player->Create(nullptr, m_dwPlayerStyle, rc, this, IDC_STATIC_PLAYER);
		player_ex_vector_.push_back(a_player_ex);
	}

	// rebuild ndx
	ndx = 1;
	for (auto player_ex : player_ex_vector_) {
		player_ex->player->ndx_ = ndx++;
	}

	m_bInitOver = TRUE;

	LoadPosition();
	
	m_chk_1_video.SetCheck(same_time_play_vidoe_route_count == 1);
	m_chk_4_video.SetCheck(same_time_play_vidoe_route_count == 4);
	m_chk_9_video.SetCheck(same_time_play_vidoe_route_count == 9);
	player_op_set_same_time_play_video_route(same_time_play_vidoe_route_count);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


CVideoPlayerDlg::record_ptr CVideoPlayerDlg::record_op_get_record_info_by_device(const video::CVideoDeviceInfoPtr& device) {
	record_ptr rec_info = nullptr;
	std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_);
	for (auto info : record_list_) {
		if (info->_device == std::dynamic_pointer_cast<video::ezviz::CVideoDeviceInfoEzviz>(device)) {
			rec_info = info;
			break;
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


void CVideoPlayerDlg::player_op_set_same_time_play_video_route(const int n)
{
	CRect rc;
	m_player.GetWindowRect(rc);
	ScreenToClient(rc);

	auto cfg = util::CConfigHelper::GetInstance();
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
			a_player_ex->player = player_op_create_new_player();
			a_player_ex->player->MoveWindow(v[i]);

			a_player_ex->used = true;
			player_ex_vector_.push_back(a_player_ex);
		}

	} else {
		for (int i = n; i < prev_count; i++) {
			//player_op_recycle_player(player_ex_vector_[i]->player);
			auto player = player_ex_vector_[i]->player;
			player->ShowWindow(SW_HIDE);
			delete_from_play_list_by_record(record_op_get_record_info_by_player(player));
		}

		while (player_ex_vector_.size() > (size_t)n) {
			back_end_players_.push_back(player_ex_vector_.back()->player);
			player_ex_vector_.pop_back();
		}
	}

	// rebuild ndx
	int ndx = 1;
	for (auto player_ex : player_ex_vector_) {
		player_ex->player->ndx_ = ndx++;
	}

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
		BOOL bMax = static_cast<BOOL>(wParam);
		if (bMax) {
			ShowOtherCtrls(0);
			GetWindowPlacement(&m_rcNormal);
			m_player.GetWindowPlacement(&m_rcNormalPlayer);
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
		} else {
			ShowOtherCtrls(1);
			SetWindowPlacement(&m_rcNormal);
			m_player.SetWindowPlacement(&m_rcNormalPlayer);
		}

		player_op_update_players_size_with_m_player();

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
		ASSERT(0);
	}
}


void CVideoPlayerDlg::StopPlayCurselVideo()
{
	AUTO_LOG_FUNCTION;
	if (m_curPlayingDevice) {
		EnableControlPanel(0, 0);
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

		auto player = player_op_get_free_player();
		{
			video::ezviz::CSdkMgrEzviz::NSCBMsg msg;
			msg.pMessageInfo = nullptr;
			ret = mgr->m_dll.startRealPlay(session_id,
										   player->m_hWnd,
										   device->get_cameraId(),
										   user->get_user_accToken(),
										   device->get_secure_code(),
										   util::CConfigHelper::GetInstance()->get_ezviz_private_cloud_app_key(),
										   videoLevel,
										   &msg);
			if (ret != 0) {
				auto emsg = std::make_shared<ezviz_msg>();
				emsg->iErrorCode = msg.iErrorCode;
				emsg->messageInfo = msg.pMessageInfo ? msg.pMessageInfo : "";
				emsg->iMsgType = video::ezviz::CSdkMgrEzviz::INS_PLAY_EXCEPTION;
				emsg->sessionId = session_id;
				HandleEzvizMsg(emsg);
			}
		}

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
			if (ok) {
				video::ezviz::CSdkMgrEzviz::NSCBMsg msg;
				msg.pMessageInfo = nullptr;
				ret = mgr->m_dll.startRealPlay(session_id,
											   player->m_hWnd,
											   device->get_cameraId(),
											   user->get_user_accToken(),
											   device->get_secure_code(),
											   util::CConfigHelper::GetInstance()->get_ezviz_private_cloud_app_key(),
											   videoLevel,
											   &msg);

				if (ret != 0) {
					auto emsg = std::make_shared<ezviz_msg>();
					emsg->iErrorCode = msg.iErrorCode;
					emsg->messageInfo = msg.pMessageInfo ? msg.pMessageInfo : "";
					emsg->iMsgType = video::ezviz::CSdkMgrEzviz::INS_PLAY_EXCEPTION;
					emsg->sessionId = session_id;
					HandleEzvizMsg(emsg);
				}
			}
		}

		if (ret != 0) {
			JLOG(L"startRealPlay failed %d\n", ret);
			m_curPlayingDevice = nullptr;
			SAFEDELETEP(param);
			player_op_recycle_player(player);
		} else {
			JLOG(L"PlayVideo ok\n");

			EnableControlPanel(TRUE, videoLevel);
			std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_);
			//player->ShowWindow(SW_SHOW);
			video::ZoneUuid zoneUuid = device->GetActiveZoneUuid();			
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


void CVideoPlayerDlg::StopPlayEzviz(video::ezviz::CVideoDeviceInfoEzvizPtr device)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	assert(device);
	std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_);
	auto user = std::dynamic_pointer_cast<video::ezviz::CVideoUserInfoEzviz>(device->get_userInfo()); assert(user);
	video::ezviz::CSdkMgrEzviz* mgr = video::ezviz::CSdkMgrEzviz::GetInstance();
	
	std::string session_id = mgr->GetSessionId(user->get_user_phone(), device->get_cameraId(), messageHandler, this);
	mgr->m_dll.setDataCallBack(session_id, videoDataHandler, nullptr);
	video::ezviz::CSdkMgrEzviz::NSCBMsg msg;
	msg.pMessageInfo = nullptr;
	int ret = mgr->m_dll.stopRealPlay(session_id, &msg);
	if (ret != 0) {
		auto emsg = std::make_shared<ezviz_msg>();
		emsg->iErrorCode = msg.iErrorCode;
		emsg->messageInfo = msg.pMessageInfo ? msg.pMessageInfo : "";
		emsg->iMsgType = video::ezviz::CSdkMgrEzviz::INS_PLAY_EXCEPTION;
		emsg->sessionId = session_id;
		HandleEzvizMsg(emsg);
	} else {
		auto record = record_op_get_record_info_by_device(device);
		on_ins_play_stop(record);
	}

	/*for (auto info : record_list_) {
		if (info->_param->_session_id == session_id) {
			delete_from_play_list_by_record(info);
			break;
		}
	}*/
}


void CVideoPlayerDlg::delete_from_play_list_by_record(const record_ptr& record)
{
	if (!record)return;

	for (int i = 0; i < m_ctrl_play_list.GetItemCount(); i++) {
		int id = m_ctrl_play_list.GetItemData(i);
		if (id == record->_device->get_id()) {
			m_static_group_cur_video.SetWindowTextW(L"");
			m_ctrl_play_list.DeleteItem(i);
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

	video::CVideoManager::ReleaseObject();

	KillTimer(TIMER_ID_EZVIZ_MSG);
	KillTimer(TIMER_ID_REC_VIDEO);
	KillTimer(TIMER_ID_PLAY_VIDEO);

	record_list_.clear();
	m_wait2playDevList.clear();

	g_player = nullptr;
}

namespace detail
{
	
};


void CVideoPlayerDlg::OnTimer(UINT_PTR nIDEvent)
{
	//AUTO_LOG_FUNCTION;
	if (TIMER_ID_EZVIZ_MSG == nIDEvent) {
		auto_timer timer(m_hWnd, TIMER_ID_EZVIZ_MSG, 1000);
		if (lock_4_ezviz_msg_queue_.try_lock()) {
			std::lock_guard<std::mutex> lock(lock_4_ezviz_msg_queue_, std::adopt_lock);
			if (ezviz_msg_list_.size() > 0) {
				auto msg = ezviz_msg_list_.front();
				ezviz_msg_list_.pop_front();
				HandleEzvizMsg(msg);
			}
		}
	} else if (TIMER_ID_REC_VIDEO == nIDEvent) {
		auto_timer timer(m_hWnd, TIMER_ID_REC_VIDEO, 2000);
		if (lock_4_record_list_.try_lock()) {
			std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_, std::adopt_lock);
			auto now = GetTickCount();
			const int max_minutes = util::CConfigHelper::GetInstance()->get_back_end_record_minutes();
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


void CVideoPlayerDlg::StopPlayByRecordInfo(record_ptr info)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;

	if (m_curPlayingDevice == info->_device) {
		m_curPlayingDevice = nullptr;
		EnableControlPanel(0);
	}

	//video::ezviz::CSdkMgrEzviz* mgr = video::ezviz::CSdkMgrEzviz::GetInstance();
	//mgr->m_dll.stopRealPlay(info->_param->_session_id);
	StopPlayEzviz(info->_device);

	/*core::history_record_manager* hr = core::history_record_manager::GetInstance();
	CString record, stop; stop = GetStringFromAppResource(IDS_STRING_VIDEO_STOP);
	record.Format(L"%s([%d,%s]%s)-\"%s\"", stop, info->_device->get_id(), 
				  info->_device->get_device_note().c_str(),
				  A2W(info->_device->get_deviceSerial().c_str()), 
				  (info->_param->_file_path));

	hr->InsertRecord(info->_zone._ademco_id, info->_zone._zone_value,
					 record, time(nullptr), core::RECORD_LEVEL_VIDEO);*/
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


void CVideoPlayerDlg::InsertList(const record_ptr& info)
{
	if (!info) return;

	for (int i = 0; i < m_ctrl_play_list.GetItemCount(); i++) {
		if (info->_device->get_id() == (int)m_ctrl_play_list.GetItemData(i)) {
			return;
		}
	}

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

		tmp.Format(L"%s-%s", info->_device->get_userInfo()->get_user_name().c_str(), info->_device->get_device_note().c_str());
		m_static_group_cur_video.SetWindowTextW(tmp);
	}
}


void CVideoPlayerDlg::OnLvnItemchangedList1(NMHDR * pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;
	if (pNMItemActivate->iItem < 0)return;
	int id = m_ctrl_play_list.GetItemData(pNMItemActivate->iItem);
	std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_);
	for (auto info : record_list_) {
		if (info->_device->get_id() == id) {
			PlayVideoByDevice(info->_device, info->_level);
			CString txt;
			txt.Format(L"%s-%s", info->_device->get_userInfo()->get_user_name().c_str(), info->_device->get_device_note().c_str());
			m_static_group_cur_video.SetWindowTextW(txt);
			break;
		}
	}
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
	std::lock_guard<std::recursive_mutex> lock(lock_4_record_list_);
	for (auto info : record_list_) {
		if (info->_device->get_id() == id) {
			PlayVideoByDevice(info->_device, info->_level);
			CString txt;
			txt.Format(L"%s-%s", info->_device->get_userInfo()->get_user_name().c_str(), info->_device->get_device_note().c_str());
			m_static_group_cur_video.SetWindowTextW(txt);
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
	CRect rc;
	m_player.GetWindowRect(rc);
	ScreenToClient(rc);
	auto a_player = std::make_shared<CVideoPlayerCtrl>(); 
	a_player->Create(nullptr, m_dwPlayerStyle, rc, this, IDC_STATIC_PLAYER);
	return a_player;
}


player CVideoPlayerDlg::player_op_get_free_player() {
	player player;
	for (int i = 0; i < util::CConfigHelper::GetInstance()->get_show_video_same_time_route_count(); i++) {
		auto iter = player_ex_vector_[i];
		if (!iter->used) { // has free ctrl, show it
			player = iter->player;
			player->ShowWindow(SW_SHOW);
			iter->used = true;
			break;
		}
	}

	if (!player) { // no free ctrl, move current first ctrl to back-end, create new ctrl and bring it to front
		player = player_op_create_new_player();

		assert(player);
	}

	return player;
}


void CVideoPlayerDlg::player_op_bring_player_to_front(const player& player)
{
	// already playing in front-end
	if (player_op_is_front_end_player(player)) {
		player->ShowWindow(SW_SHOW);
		return;
	}

	// not playing
	auto player_ex_0 = player_ex_vector_[0];
	CRect rc;
	player_ex_0->player->GetWindowRect(rc);
	ScreenToClient(rc); // get player 1's rc

	const int player_count = util::CConfigHelper::GetInstance()->get_show_video_same_time_route_count();

	for (int i = 1; i < player_count; i++) { // 后n个player前移1位
		CRect my_rc;
		player_ex_vector_[i]->player->GetWindowRect(my_rc);
		ScreenToClient(my_rc);
		player_ex_vector_[i]->player->MoveWindow(rc);
		player_ex_vector_[i]->rc = rc;
		player_ex_vector_[i]->player->ndx_ = i;
		rc = my_rc;
	}

	player->MoveWindow(rc); // move new ctrl to last place

	player_op_recycle_player(player_ex_0->player); // move prev-player-1 to back-end
	delete_from_play_list_by_record(record_op_get_record_info_by_player(player_ex_0->player));
	player_ex_0->player = player;
	player_ex_0->player->ndx_ = util::CConfigHelper::GetInstance()->get_show_video_same_time_route_count();
	player_ex_0->rc = rc;
	player_ex_0->used = true;

	for (int i = 0; i < player_count - 1; i++) { // rebuild vector
		player_ex_vector_[i] = player_ex_vector_[i + 1];
	}
	player_ex_vector_[player_count - 1] = player_ex_0;

	// show player, add a item to play list
	player->ShowWindow(SW_SHOW);
	InsertList(record_op_get_record_info_by_player(player));

	// rebuild ndx
	int ndx = 1;
	for (auto player_ex : player_ex_vector_) {
		player_ex->player->ndx_ = ndx++;
	}
}


void CVideoPlayerDlg::player_op_recycle_player(const player& player)
{
	assert(player);
	player->ShowWindow(SW_HIDE);

	bool recycled = false;

	for (int i = 0; i < util::CConfigHelper::GetInstance()->get_show_video_same_time_route_count(); i++) {
		auto iter = player_ex_vector_[i];
		if (iter->player == player) { // playing in front-end, delete its list item
			delete_from_play_list_by_record(record_op_get_record_info_by_player(iter->player));
			iter->used = false;
			recycled = true;
			break;
		}
	}

	if (recycled) {
		return;
	}
	
	back_end_players_.push_back(player);
}


void CVideoPlayerDlg::player_op_update_players_size_with_m_player()
{
	CRect rc;
	m_player.GetWindowRect(rc);
	ScreenToClient(rc);

	const int n = util::CConfigHelper::GetInstance()->get_show_video_same_time_route_count();
	auto v = split_rect(rc, n);

	for (int i = 0; i < n; i++) {
		player_ex_vector_[i]->player->MoveWindow(v[i]);
		player_ex_vector_[i]->rc = v[i];
	}
}


bool CVideoPlayerDlg::player_op_is_front_end_player(const player& player) const
{
	for (auto player_ex : player_ex_vector_) {
		if (player_ex->player == player) {
			return true;
		}
	}
	return false;
}
