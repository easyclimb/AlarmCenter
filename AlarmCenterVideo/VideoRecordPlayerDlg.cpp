// VideoRecordPlayerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenterVideo.h"
#include "VideoRecordPlayerDlg.h"
#include "afxdialogex.h"
#include "../video/jovision/VideoDeviceInfoJovision.h"
#include "../video/jovision/VideoUserInfoJovision.h"
#include "JovisonSdkMgr.h"
#include "alarm_center_video_client.h"
#include "ConfigHelper.h"

using namespace video;
using namespace video::jovision;


std::string g_szRecFilename = "C:\\Video.mp4";
FILE *g_pRecFile = NULL;
bool s_bDownload = false;
bool s_bPlay = false;

// CVideoRecordPlayerDlg dialog

IMPLEMENT_DYNAMIC(CVideoRecordPlayerDlg, CDialogEx)

CVideoRecordPlayerDlg::CVideoRecordPlayerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_VIDEO_RECORD_PLAYER, pParent)
{

}

CVideoRecordPlayerDlg::~CVideoRecordPlayerDlg()
{
}

void CVideoRecordPlayerDlg::AddLogItem(const CString & log)
{
	int ndx = m_list_log.AddString(log);
	m_list_log.SetCurSel(ndx);
	ipc::alarm_center_video_client::get_instance()->insert_record(0, 0, (LPCTSTR)log);
}

void CVideoRecordPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PLAYER, m_player);
	DDX_Control(pDX, IDC_STATIC_REC_LIST, m_group_rec_list);
	DDX_Control(pDX, IDC_LIST1, m_list_rec);
	DDX_Control(pDX, IDC_DATETIMEPICKER1, m_ctrl_date);
	DDX_Control(pDX, IDC_BUTTON_GET_REC_LIST, m_btn_get_rec_list);
	DDX_Control(pDX, IDC_STATIC_LOGS, m_group_logs);
	DDX_Control(pDX, IDC_LIST3, m_list_log);
	DDX_Control(pDX, IDC_BUTTON_GET_REC_LIST2, m_btn_dl);
	DDX_Control(pDX, IDC_BUTTON1, m_btn_play);
}


BEGIN_MESSAGE_MAP(CVideoRecordPlayerDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CVideoRecordPlayerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CVideoRecordPlayerDlg::OnBnClickedCancel)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_GET_REC_LIST, &CVideoRecordPlayerDlg::OnBnClickedButtonGetRecList)
	ON_MESSAGE(WM_JC_GETRECFILELIST, &CVideoRecordPlayerDlg::OnJcGetRecFileList)
	ON_WM_TIMER()
	ON_LBN_DBLCLK(IDC_LIST1, &CVideoRecordPlayerDlg::OnLbnDblclkList1)
	ON_MESSAGE(WM_JC_RESETSTREAM, &CVideoRecordPlayerDlg::OnJcResetStream)
	ON_WM_MOVE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_GET_REC_LIST2, &CVideoRecordPlayerDlg::OnBnClickedButtonGetRecList2)
	ON_BN_CLICKED(IDC_BUTTON1, &CVideoRecordPlayerDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CVideoRecordPlayerDlg message handlers


void CVideoRecordPlayerDlg::load_pos()
{
	auto cfg = util::CConfigHelper::get_instance();
	auto rc = cfg->get_rc_video_rec_dlg();
	if (rc.IsRectEmpty() || rc.IsRectNull()) {

	} else {
		MoveWindow(rc);
	}
}

void CVideoRecordPlayerDlg::save_pos()
{
	auto cfg = util::CConfigHelper::get_instance();
	CRect rc;
	GetWindowRect(rc);
	cfg->set_rc_video_rec_dlg(rc);
}

void CVideoRecordPlayerDlg::HandleJovisionMsg(const video::jovision::jovision_msg_ptr & msg)
{
	auto etType = msg->etType;
	auto nLinkID = msg->nLinkID;
	auto pData1 = msg->pData1;
	auto pData2 = msg->pData2;
	//auto pUserData = msg->pUserData;
	const char* dwMsgID = 0;
	bool bBeginDownload = false;

	switch (etType) {
	case JCET_GetFileListOK://获取远程录像成功
	{
		rec_file_infos_.clear();
		PJCRecFileInfo pInfos = (PJCRecFileInfo)pData1;
		int nCount = (int)pData2;
		for (int i = 0; i < nCount; ++i) {
			rec_file_infos_.push_back(pInfos[i]);
		}
	}
	case JCET_GetFileListError://获取远程录像失败
	{
		PostMessage(WM_JC_GETRECFILELIST, etType == JCET_GetFileListOK);
	}
	return;
	break;

	case JCET_StreamReset://码流重置信号
	{
		sdk_mgr_jovision::get_instance()->enable_decoder(nLinkID, FALSE);
		PostMessage(WM_JC_RESETSTREAM);
	}
	return;
	break;
	}

	switch (etType) {
	case JCET_ConnectOK://连接成功
		dwMsgID = IDS_ConnectOK;
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

	case JCET_DownloadData://录像下载数据
	{
		if (g_pRecFile == NULL && s_bDownload) {
			g_pRecFile = fopen(g_szRecFilename.c_str(), "wb");
			if (g_pRecFile != NULL) {
				bBeginDownload = true;
			} else {
				break;
			}
		}

		if (g_pRecFile != NULL) {
			fwrite((char*)pData1, pData2, 1, g_pRecFile);
		}
	}
	break;

	case JCET_DownloadEnd://录像下载完成
	case JCET_DownloadStop://录像下载停止
	case JCET_DownloadError://远程下载失败
	case JCET_DownloadTimeout://远程下载超时
	{
		if (g_pRecFile != NULL) {
			fclose(g_pRecFile);
			g_pRecFile = NULL;
		}

		m_btn_dl.SetWindowTextW(TR(IDS_STRING_DOWNLOAD));
		s_bDownload = false;
	}
	break;

	default:
		return;
		break;
	}

	CString strMsg;

	if (dwMsgID) {
		std::wstring wMsg;
		if (pData1 != NULL) {
			std::string sMsg = (char*)pData1;
			wMsg = std::wstring(sMsg.begin(), sMsg.end());
		}
		strMsg.Format(TR(dwMsgID), nLinkID, wMsg.c_str());
		AddLogItem(strMsg);
	}

	if (etType == JCET_ConnectOK) {
		auto jov = sdk_mgr_jovision::get_instance();
		
		if (jov->enable_decoder(link_id_, 1)) {
			dwMsgID = IDS_EnableDecodeOK;
			strMsg.Format(TR(dwMsgID), link_id_);
			AddLogItem(strMsg);
		} else {
			dwMsgID = IDS_EnableDecodeError;
			strMsg.Format(TR(dwMsgID), link_id_);
			AddLogItem(strMsg);
			return;
		}
		
		JCDateBlock data;

		CTime ct;
		m_ctrl_date.GetTime(ct);
		
		{
			data.nBeginYear = data.nEndYear = ct.GetYear();
			data.nBeginMonth = data.nEndMonth = ct.GetMonth();
			data.nBeginDay = ct.GetDay();
			data.nEndDay = ct.GetDay();
		}

		if (jov->get_remote_record_file_list(link_id_, &data)) {
			return;
		} else {
			dwMsgID = IDS_GetRecFileListError;
		}
		strMsg.Format(TR(dwMsgID), link_id_);
		AddLogItem(strMsg);
	} else {
		switch (etType) {
		case JCET_DownloadData://录像下载数据
			if (bBeginDownload) {
				dwMsgID = IDS_BeginDownload;
			} else {
				return;
			}
			break;

		case JCET_DownloadEnd://录像下载完成
			dwMsgID = IDS_DownloadOver;
			break;

		case JCET_DownloadStop://录像下载停止
			dwMsgID = IDS_DownloadStop;
			break;

		case JCET_DownloadError://远程下载失败
			dwMsgID = IDS_DownloadFailed;
			break;

		case JCET_DownloadTimeout://远程下载超时
			dwMsgID = IDS_DownloadTimeout;
			break;

		default:
			return;
			break;
		}

		if (dwMsgID) {
			strMsg.Format(TR(dwMsgID), link_id_);
			AddLogItem(strMsg);

			if (JCET_DownloadEnd == etType) {
				auto file = utf8::mbcs_to_u16(g_szRecFilename);
				strMsg += L"\r\n";
				strMsg += file.c_str();
				strMsg += L"\r\n" + TR(IDS_STRING_OPEN_FILE) + L"?";
				int ret = MessageBox(strMsg, L"", MB_YESNO | MB_ICONQUESTION);
				if (ret == IDYES) {
					ShellExecute(GetSafeHwnd(), L"open", file.c_str(), nullptr, nullptr, SW_SHOW);
				}
			}
		}
	}
}

BOOL CVideoRecordPlayerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText(TR(IDS_DIALOG_VIDEO_RECORD_PLAYER));
	
	CenterWindow();

	assert(device_); 
	SetWindowText(TR(IDS_DIALOG_VIDEO_RECORD_PLAYER) + L" " + device_->get_formatted_name().c_str());

	m_group_rec_list.SetWindowTextW(TR(IDS_REC_LIST));
	m_btn_get_rec_list.SetWindowTextW(TR(IDS_GET_REC_LIST));
	m_group_logs.SetWindowTextW(TR(IDS_OP_LOG));
	m_btn_play.SetWindowTextW(TR(IDS_STRING_PLAY_VIDEO));
	m_btn_dl.SetWindowTextW(TR(IDS_STRING_DOWNLOAD));

	if (automatic_) {
		OnBnClickedButtonGetRecList();
	}

	load_pos();

	init_over_ = true;

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CVideoRecordPlayerDlg::OnBnClickedOk()
{
	return;
}


void CVideoRecordPlayerDlg::OnBnClickedCancel()
{
	return;
}


void CVideoRecordPlayerDlg::OnClose()
{
	auto jov = jovision::sdk_mgr_jovision::get_instance();

	if (link_id_ != -1) {
		jov->disconnect(link_id_);
		link_id_ = -1;
	}

	s_bDownload = false;
	s_bPlay = false;

	if (g_pRecFile) {
		fclose(g_pRecFile);
		g_pRecFile = nullptr;
	}

	save_pos();
	ShowWindow(SW_HIDE);
}


void CVideoRecordPlayerDlg::OnBnClickedButtonGetRecList()
{
	auto jov = jovision::sdk_mgr_jovision::get_instance();

	if (link_id_ != -1) {
		jov->disconnect(link_id_);
	}

	if (device_->get_by_sse()) {
		link_id_ = jov->connect(const_cast<char*>(device_->get_sse().c_str()), 0, 1,
							   const_cast<char*>(utf8::w2a(device_->get_user_name()).c_str()),
							   const_cast<char*>(device_->get_user_passwd().c_str()),
							   1, nullptr);
	} else {
		link_id_ = jov->connect(const_cast<char*>(device_->get_ip().c_str()), device_->get_port(), 1,
							   const_cast<char*>(utf8::w2a(device_->get_user_name()).c_str()),
							   const_cast<char*>(device_->get_user_passwd().c_str()),
							   1, nullptr);
	}

	if (link_id_ == -1) {
		AddLogItem(TR(IDS_ConnectError));
	} else {
		CString strMsg;
		strMsg.Format(TR(IDS_Connecting), link_id_);
		AddLogItem(strMsg);
	}

	m_btn_get_rec_list.EnableWindow(0);
	CMenu *pSysMenu = GetSystemMenu(FALSE);
	ASSERT(pSysMenu != NULL);
	pSysMenu->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
	tp_ = std::chrono::steady_clock::now();
	counter_ = 10;
	SetTimer(1, 1000, nullptr);
}


afx_msg LRESULT CVideoRecordPlayerDlg::OnJcGetRecFileList(WPARAM wParam, LPARAM /*lParam*/)
{
	auto translate_file_name = [](const std::wstring& origin) {
		std::wstring res = origin;
		auto npos = std::wstring::size_type(-1);
		auto pos = origin.find_last_of(L'.');
		if (pos != npos) {
			res = origin.substr(pos - 6, 6);
			res.insert(4, L":");
			res.insert(2, L":");
		}
		return res;
	};

	const char* dwMsgID = 0;
	if (wParam) {
		m_list_rec.ResetContent();
		int nCount = rec_file_infos_.size();
		for (int i = 0; i < nCount; ++i) {
			std::string str = rec_file_infos_[i].szPathName;
			std::wstring wstr(str.begin(), str.end());
			auto fmt = translate_file_name(wstr);
			int nItemID = m_list_rec.AddString(fmt.c_str());
			m_list_rec.SetItemData(nItemID, rec_file_infos_[i].nRecFileID);
			m_list_rec.SetCurSel(nItemID);
		}

		dwMsgID = nCount == 0 ? IDS_NoRecFile : IDS_GetRecFileListOK;
	} else {
		dwMsgID = IDS_GetRecFileListError;
	}

	CString strMsg;
	strMsg.Format(TR(dwMsgID), link_id_);
	AddLogItem(strMsg);

	KillTimer(1);
	m_btn_get_rec_list.SetWindowTextW(TR(IDS_GET_REC_LIST));
	m_btn_get_rec_list.EnableWindow(1);
	CMenu *pSysMenu = GetSystemMenu(FALSE);
	ASSERT(pSysMenu != NULL);
	pSysMenu->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND | MF_ENABLED);
	if (automatic_ && wParam && m_list_rec.GetCount() > 0) {
		m_list_rec.SetCurSel(m_list_rec.GetCount() - 1);
		OnLbnDblclkList1();
	}
	return 0;
}


void CVideoRecordPlayerDlg::OnTimer(UINT_PTR nIDEvent)
{
	/*auto now = std::chrono::steady_clock::now();
	auto diff = now - tp_;
	auto secs = std::chrono::duration_cast<std::chrono::seconds>(diff);
	if (secs.count() > 5) {

	}*/

	if (counter_ == 0) {
		m_btn_get_rec_list.SetWindowTextW(TR(IDS_GET_REC_LIST));
		m_btn_get_rec_list.EnableWindow(1);
		CMenu *pSysMenu = GetSystemMenu(FALSE);
		ASSERT(pSysMenu != NULL);
		pSysMenu->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND | MF_ENABLED);
		CString strMsg;
		strMsg.Format(TR(IDS_GetRecFileListError), link_id_);
		AddLogItem(strMsg);
	} else {
		CString txt;
		txt.Format(L" %d", counter_);
		m_btn_get_rec_list.SetWindowTextW(TR(IDS_GET_REC_LIST) + txt);
	}

	counter_--;

	CDialogEx::OnTimer(nIDEvent);
}


void CVideoRecordPlayerDlg::OnLbnDblclkList1()
{
	/*int ndx = m_list_rec.GetCurSel(); if (ndx < 0) return;

	int nFileID = m_list_rec.GetItemData(ndx);
	if (nFileID >= 0) {
		sdk_mgr_jovision::get_instance()->remote_play(link_id_, nFileID);
	}*/
}


afx_msg LRESULT CVideoRecordPlayerDlg::OnJcResetStream(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	auto jov = sdk_mgr_jovision::get_instance();
	jov->enable_decoder(link_id_, TRUE);

	jov->set_video_preview(link_id_, m_player.GetRealHwnd(), m_player.GetRealRect());
	previewing_ = true;

	return 0;
}


void CVideoRecordPlayerDlg::OnMove(int x, int y)
{
	CDialogEx::OnMove(x, y);

	if (previewing_) {
		auto jov = sdk_mgr_jovision::get_instance();
		jov->set_video_preview(link_id_, m_player.GetRealHwnd(), m_player.GetRealRect());
	}
	
	if (init_over_) {
		save_pos();
	}
}


void CVideoRecordPlayerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	auto jov = jovision::sdk_mgr_jovision::get_instance();

	if (link_id_ != -1) {
		jov->disconnect(link_id_);
		link_id_ = -1;
	}
}


void CVideoRecordPlayerDlg::OnBnClickedButtonGetRecList2()
{
	auto jov = jovision::sdk_mgr_jovision::get_instance();

	if (s_bDownload) {
		jov->download_remote_file(link_id_, -1);

		if (g_pRecFile != NULL) {
			fclose(g_pRecFile);
			g_pRecFile = NULL;

			CString strMsg;
			strMsg.Format(TR(IDS_DownloadStop), link_id_);
			AddLogItem(strMsg);
		}

		m_btn_dl.SetWindowTextW(TR(IDS_STRING_DOWNLOAD));
		s_bDownload = false;

	} else {
		int ndx = m_list_rec.GetCurSel(); if (ndx < 0) return;
		int nFileID = m_list_rec.GetItemData(ndx);
		if (nFileID >= 0) {
			std::wstring path;

			if (!jlib::get_save_as_dialog_path(path, L"mp4", GetSafeHwnd())) {
				return;
			}

			g_szRecFilename = utf8::u16_to_mbcs(path);
			jov->download_remote_file(link_id_, nFileID);
			m_btn_dl.SetWindowTextW(TR(IDS_STRING_STOP_DOWNLOAD));
			s_bDownload = true;
		}
	}

}


void CVideoRecordPlayerDlg::OnBnClickedButton1()
{
	auto jov = jovision::sdk_mgr_jovision::get_instance();

	//if (s_bPlay) {
		//jov->remote_play(link_id_, -1);
		//m_btn_play.SetWindowTextW(TR(IDS_STRING_IDC_BUTTON_PLAY));
		//s_bPlay = false;
	/*} else {
		
	}*/

		int ndx = m_list_rec.GetCurSel(); if (ndx < 0) return;
		int nFileID = m_list_rec.GetItemData(ndx);
		if (nFileID >= 0) {
			jov->remote_play(link_id_, nFileID);
			//m_btn_play.SetWindowTextW(TR(IDS_STRING_IDC_BUTTON_STOP));
			s_bPlay = true;
		}
}
