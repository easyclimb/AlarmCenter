// VideoRecordPlayerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "VideoRecordPlayerDlg.h"
#include "afxdialogex.h"
#include "VideoDeviceInfoJovision.h"
#include "VideoUserInfoJovision.h"
#include "VideoManager.h"
#include "JovisonSdkMgr.h"

using namespace video;
using namespace video::jovision;

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
END_MESSAGE_MAP()


// CVideoRecordPlayerDlg message handlers


void CVideoRecordPlayerDlg::HandleJovisionMsg(const video::jovision::jovision_msg_ptr & msg)
{
	auto etType = msg->etType;
	auto nLinkID = msg->nLinkID;
	auto pData1 = msg->pData1;
	auto pData2 = msg->pData2;
	auto pUserData = msg->pUserData;
	DWORD dwMsgID = 0;

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

	default:
		return;
		break;
	}

	CString strMsg;
	std::wstring wMsg;
	if (pData1 != NULL) {
		std::string sMsg = (char*)pData1;
		wMsg = std::wstring(sMsg.begin(), sMsg.end());
	}
	strMsg.Format(GetStringFromAppResource(dwMsgID), nLinkID, wMsg.c_str());
	AddLogItem(strMsg);

	if (etType == JCET_ConnectOK) {
		auto jov = sdk_mgr_jovision::get_instance();
		
		if (jov->enable_decoder(link_id_, 1)) {
			dwMsgID = IDS_EnableDecodeOK;
			strMsg.Format(GetStringFromAppResource(dwMsgID), link_id_);
			AddLogItem(strMsg);
		} else {
			dwMsgID = IDS_EnableDecodeError;
			strMsg.Format(GetStringFromAppResource(dwMsgID), link_id_);
			AddLogItem(strMsg);
			return;
		}
		
		JCDateBlock data;
		/*time_t t;
		time(&t);
		tm d = { 0 };
		localtime_s(&d, &t);*/

		CTime ct;
		m_ctrl_date.GetTime(ct);
		
		/*data.nBeginYear = data.nEndYear = d.tm_year + 1900;
		data.nBeginMonth = data.nEndMonth = d.tm_mon + 1;
		data.nBeginDay = d.tm_mday - 1;
		data.nEndDay = d.tm_mday;
*/
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
		strMsg.Format(GetStringFromAppResource(dwMsgID), link_id_);
		AddLogItem(strMsg);
	}
}

BOOL CVideoRecordPlayerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	CenterWindow();

	assert(device_); 
	SetWindowText(GetStringFromAppResource(IDS_DIALOG_VIDEO_RECORD_PLAYER) + L" " + device_->get_formatted_name().c_str());

	m_group_rec_list.SetWindowTextW(GetStringFromAppResource(IDS_REC_LIST));
	m_btn_get_rec_list.SetWindowTextW(GetStringFromAppResource(IDS_GET_REC_LIST));
	m_group_logs.SetWindowTextW(GetStringFromAppResource(IDS_OP_LOG));

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
		AddLogItem(GetStringFromAppResource(IDS_ConnectError));
	} else {
		CString strMsg;
		strMsg.Format(GetStringFromAppResource(IDS_Connecting), link_id_);
		AddLogItem(strMsg);
	}

	m_btn_get_rec_list.EnableWindow(0);
	tp_ = std::chrono::steady_clock::now();
	counter_ = 10;
	SetTimer(1, 1000, nullptr);
}


afx_msg LRESULT CVideoRecordPlayerDlg::OnJcGetRecFileList(WPARAM wParam, LPARAM lParam)
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

	DWORD dwMsgID = 0;
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
	strMsg.Format(GetStringFromAppResource(dwMsgID), link_id_);
	AddLogItem(strMsg);

	KillTimer(1);
	m_btn_get_rec_list.SetWindowTextW(GetStringFromAppResource(IDS_GET_REC_LIST));
	m_btn_get_rec_list.EnableWindow(1);
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
		m_btn_get_rec_list.SetWindowTextW(GetStringFromAppResource(IDS_GET_REC_LIST));
		m_btn_get_rec_list.EnableWindow(1);
		CString strMsg;
		strMsg.Format(GetStringFromAppResource(IDS_GetRecFileListError), link_id_);
		AddLogItem(strMsg);
	} else {
		CString txt;
		txt.Format(L" %d", counter_);
		m_btn_get_rec_list.SetWindowTextW(GetStringFromAppResource(IDS_GET_REC_LIST) + txt);
	}

	counter_--;

	CDialogEx::OnTimer(nIDEvent);
}


void CVideoRecordPlayerDlg::OnLbnDblclkList1()
{
	int ndx = m_list_rec.GetCurSel(); if (ndx < 0) return;

	int nFileID = m_list_rec.GetItemData(ndx);
	if (nFileID >= 0) {
		sdk_mgr_jovision::get_instance()->remote_play(link_id_, nFileID);
	}
}


afx_msg LRESULT CVideoRecordPlayerDlg::OnJcResetStream(WPARAM wParam, LPARAM lParam)
{
	auto jov = sdk_mgr_jovision::get_instance();
	jov->enable_decoder(link_id_, TRUE);

	jov->set_video_preview(link_id_, m_player.GetRealHwnd(), m_player.GetRealRect());
	previewing_ = true;

	return 0;
}
