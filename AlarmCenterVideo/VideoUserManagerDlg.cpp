// VideoUserManagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenterVideo.h"
#include "VideoUserManagerDlg.h"
#include "afxdialogex.h"
#include "../AlarmCenter/VideoUserInfoEzviz.h"
#include "../AlarmCenter/VideoUserInfoJovision.h"
#include "../AlarmCenter/VideoDeviceInfoEzviz.h"
#include "../AlarmCenter/VideoDeviceInfoJovision.h"
#include "AddVideoUserEzvizDlg.h"
#include "VideoPlayerDlg.h"
#include "AddVideoUserProgressDlg.h"
#include "ConfigHelper.h"
#include "AddVideoUserJovisionDlg.h"
#include "AddVideoDeviceJovisionDlg.h"
#include "alarm_center_video_client.h"

// CVideoUserManagerDlg dialog

CVideoUserManagerDlg* g_videoUserMgrDlg = nullptr;



class CVideoUserManagerDlg::CurUserChangedObserver : public dp::observer<core::user_info_ptr>
{
public:
	explicit CurUserChangedObserver(CVideoUserManagerDlg* dlg) : _dlg(dlg) {}
	virtual void on_update(const core::user_info_ptr& ptr) {
		/*if (_dlg) {
			if (ptr->get_user_priority() == core::UP_OPERATOR) {
				_dlg->m_btnBindOrUnbind.EnableWindow(0);
				_dlg->m_btnUnbind.EnableWindow(0);
				_dlg->m_chkAutoPlayVideo.EnableWindow(0);
				_dlg->m_btnAddDevice.EnableWindow(0);
				_dlg->m_btnDelDevice.EnableWindow(0);
				_dlg->m_btnSaveDevChange.EnableWindow(0);
				_dlg->m_btnRefreshDev.EnableWindow(0);
				_dlg->m_btnAddUser.EnableWindow(0);
				_dlg->m_btnDelUser.EnableWindow(0);
				_dlg->m_btnUpdateUser.EnableWindow(0);
				_dlg->m_btnRefreshDeviceList.EnableWindow(0);
			} else {
				_dlg->m_btnBindOrUnbind.EnableWindow();
				_dlg->m_btnUnbind.EnableWindow();
				_dlg->m_chkAutoPlayVideo.EnableWindow();
				_dlg->m_btnAddDevice.EnableWindow();
				_dlg->m_btnDelDevice.EnableWindow();
				_dlg->m_btnSaveDevChange.EnableWindow();
				_dlg->m_btnRefreshDev.EnableWindow();
				_dlg->m_btnAddUser.EnableWindow();
				_dlg->m_btnDelUser.EnableWindow();
				_dlg->m_btnUpdateUser.EnableWindow();
				_dlg->m_btnRefreshDeviceList.EnableWindow();
			}

		}*/
	}
private:
	CVideoUserManagerDlg* _dlg;
};

//static const int TIMER_ID_CHECK_USER_ACCTOKEN_TIMEOUT = 1; // check if user's accToken is out of date

IMPLEMENT_DYNAMIC(CVideoUserManagerDlg, CDialogEx)

CVideoUserManagerDlg::CVideoUserManagerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CVideoUserManagerDlg::IDD, pParent)
	, m_curSelUserInfoEzviz(nullptr)
	, m_curSelDeviceInfoEzviz(nullptr)
	, m_curselUserListItemEzviz(-1)
	, m_curselDeviceListItemEzviz(-1)
	, m_observerDlg(nullptr)
{

}

CVideoUserManagerDlg::~CVideoUserManagerDlg()
{
}

void CVideoUserManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_USER, m_listUserEzviz);
	DDX_Control(pDX, IDC_LIST_DEVICE, m_listDeviceEzviz);
	DDX_Control(pDX, IDC_STATIC_DEVICE_LIST, m_groupDevice);
	DDX_Control(pDX, IDC_LIST_DEVICE2, m_listDeviceJovision);
	DDX_Control(pDX, IDC_EDIT_ID, m_id);
	DDX_Control(pDX, IDC_EDIT_PRODUCTOR, m_productor);
	DDX_Control(pDX, IDC_EDIT_NAME, m_name);
	DDX_Control(pDX, IDC_EDIT_PHONE, m_phone);
	DDX_Control(pDX, IDC_BUTTON_DEL_USER, m_btnDelUser);
	DDX_Control(pDX, IDC_BUTTON_SAVE_CHANGE, m_btnUpdateUser);
	DDX_Control(pDX, IDC_BUTTON_ADD_USER, m_btnAddUser);
	DDX_Control(pDX, IDC_BUTTON_REFRESH_DEVICE_LIST, m_btnRefreshDeviceList);
	DDX_Control(pDX, IDC_EDIT_ID2, m_idDev);
	DDX_Control(pDX, IDC_EDIT_CAMERA_NAME, m_nameDev);
	DDX_Control(pDX, IDC_EDIT_DEVICE_NOTE, m_noteDev);
	DDX_Control(pDX, IDC_EDIT_VERIFY_CODE, m_devCode);
	DDX_Control(pDX, IDC_EDIT_ZONE, m_zone);
	DDX_Control(pDX, IDC_BUTTON_BIND_OR_UNBIND, m_btnBindOrUnbind);
	DDX_Control(pDX, IDC_CHECK_AUTO_PLAY_VIDEO, m_chkAutoPlayVideo);
	DDX_Control(pDX, IDC_BUTTON_ADD_DEVICE, m_btnAddDevice);
	DDX_Control(pDX, IDC_BUTTON_DEL_DEVICE, m_btnDelDevice);
	DDX_Control(pDX, IDC_BUTTON_SAVE_DEV, m_btnSaveDevChange);
	DDX_Control(pDX, IDC_BUTTON_REFRESH_DEV, m_btnRefreshDev);
	DDX_Control(pDX, IDC_BUTTON_PLAY, m_btnPlayVideo);
	DDX_Control(pDX, IDC_BUTTON_UNBIND, m_btnUnbind);
	DDX_Control(pDX, IDC_TAB_USERS, m_tab_users);
	DDX_Control(pDX, IDC_STATIC_USER, m_groupUser);
	DDX_Control(pDX, IDC_LIST_USER2, m_listUserJovision);
	DDX_Control(pDX, IDC_EDIT_TOKEN_OR_NAME, m_token_or_name);
	DDX_Control(pDX, IDC_EDIT_TIME_OR_PASSWD, m_time_or_passwd);
	DDX_Control(pDX, IDC_STATIC_TOKEN_OR_NAME, m_static_token_or_name);
	DDX_Control(pDX, IDC_STATIC_TIME_OR_PASSWD, m_static_time_or_passwd);
	DDX_Control(pDX, IDC_STATIC_DEV_NAME, m_static_dev_name);
	DDX_Control(pDX, IDC_STATIC_NOTE, m_static_note);
	DDX_Control(pDX, IDC_STATIC_VERIFY_CODE, m_static_verify_code);
	DDX_Control(pDX, IDC_CHECK_BY_SSE_ID, m_chk_by_sse);
	DDX_Control(pDX, IDC_EDIT_CLOUD_SSE_ID, m_sse);
	DDX_Control(pDX, IDC_STATIC_IP, m_static_ip);
	DDX_Control(pDX, IDC_IPADDRESS1, m_ip);
	DDX_Control(pDX, IDC_STATIC_PORT, m_static_port);
	DDX_Control(pDX, IDC_EDIT_PORT, m_port);
	DDX_Control(pDX, IDC_STATIC_USER_NAME, m_static_user_name);
	DDX_Control(pDX, IDC_EDIT3, m_user_name);
	DDX_Control(pDX, IDC_STATIC_USER_PASSWD, m_static_user_passwd);
	DDX_Control(pDX, IDC_EDIT_USER_PASSWD, m_user_passwd);
	DDX_Control(pDX, IDC_STATIC_SSE_ID, m_static_sse);
}


BEGIN_MESSAGE_MAP(CVideoUserManagerDlg, CDialogEx)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_USER, &CVideoUserManagerDlg::OnLvnItemchangedListUserEzviz)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_CHANGE, &CVideoUserManagerDlg::OnBnClickedButtonSaveChange)
	ON_BN_CLICKED(IDC_BUTTON_DEL_USER, &CVideoUserManagerDlg::OnBnClickedButtonDelUser)
	ON_BN_CLICKED(IDC_BUTTON_ADD_USER, &CVideoUserManagerDlg::OnBnClickedButtonAddUser)
	ON_BN_CLICKED(IDC_BUTTON_REFRESH_DEVICE_LIST, &CVideoUserManagerDlg::OnBnClickedButtonRefreshDeviceList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_DEVICE, &CVideoUserManagerDlg::OnLvnItemchangedListDeviceEzviz)
	ON_BN_CLICKED(IDC_BUTTON_BIND_OR_UNBIND, &CVideoUserManagerDlg::OnBnClickedButtonBindOrUnbind)
	ON_BN_CLICKED(IDC_CHECK_AUTO_PLAY_VIDEO, &CVideoUserManagerDlg::OnBnClickedCheckAutoPlayVideo)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_DEV, &CVideoUserManagerDlg::OnBnClickedButtonSaveDev)
	ON_BN_CLICKED(IDC_BUTTON_PLAY, &CVideoUserManagerDlg::OnBnClickedButtonPlay)
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_UNBIND, &CVideoUserManagerDlg::OnBnClickedButtonUnbind)
	ON_MESSAGE(WM_VIDEO_INFO_CHANGE, &CVideoUserManagerDlg::OnVideoInfoChanged)
	ON_BN_CLICKED(IDC_BUTTON_DEL_DEVICE, &CVideoUserManagerDlg::OnBnClickedButtonDelDevice)
	ON_WM_SHOWWINDOW()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_DEVICE, &CVideoUserManagerDlg::OnNMDblclkListDeviceEzviz)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_USERS, &CVideoUserManagerDlg::OnTcnSelchangeTabUsers)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_USER2, &CVideoUserManagerDlg::OnLvnItemchangedListUserJovision)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_DEVICE2, &CVideoUserManagerDlg::OnLvnItemchangedListDeviceJovision)
	ON_BN_CLICKED(IDC_BUTTON_ADD_DEVICE, &CVideoUserManagerDlg::OnBnClickedButtonAddDevice)
	ON_BN_CLICKED(IDC_CHECK_BY_SSE_ID, &CVideoUserManagerDlg::OnBnClickedCheckBySseId)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_DEVICE2, &CVideoUserManagerDlg::OnNMDblclkListDeviceJovision)
END_MESSAGE_MAP()


// CVideoUserManagerDlg message handlers


BOOL CVideoUserManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CenterWindow(GetParent());

	//m_cur_user_changed_observer = std::make_shared<CurUserChangedObserver>(this);
	//core::user_manager::get_instance()->register_observer(m_cur_user_changed_observer);
	//m_cur_user_changed_observer->on_update(core::user_manager::get_instance()->GetCurUserInfo());

	// set list view style
	DWORD dwStyle = m_listUserEzviz.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
	m_listUserEzviz.SetExtendedStyle(dwStyle);
	m_listUserJovision.SetExtendedStyle(dwStyle);
	m_listDeviceEzviz.SetExtendedStyle(dwStyle);
	m_listDeviceJovision.SetExtendedStyle(dwStyle);

	int i = -1;
	CString fm;
	CRect rc;
	auto& productorEzviz = ProductorEzviz;
	auto& productorJovision = ProductorJovision;

	// set tab's rect
	m_groupUser.GetClientRect(rc);
	m_groupUser.ClientToScreen(rc);
	m_groupUser.ShowWindow(SW_HIDE);
	ScreenToClient(rc);
	rc.InflateRect(0, 10, 0, 0);
	m_tab_users.MoveWindow(rc);
	
	// add 2 list
	fm.Format(L"%s", productorEzviz.get_formatted_name().c_str());
	m_tab_users.InsertItem(0, fm);
	fm.Format(L"%s", productorJovision.get_formatted_name().c_str());
	m_tab_users.InsertItem(1, fm);

	// set user's rect
	rc.DeflateRect(5, 25, 5, 5);
	m_listUserEzviz.MoveWindow(rc);
	m_listUserJovision.MoveWindow(rc);

	// ezviz user list
	fm = TR(IDS_STRING_ID);
	m_listUserEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 38, -1);
	fm = TR(IDS_STRING_NAME);
	m_listUserEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 80, -1);
	fm = TR(IDS_STRING_PHONE);
	m_listUserEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 100, -1);
	fm = TR(IDS_STRING_ACCESS_TOKEN);
	m_listUserEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 100, -1);
	fm = TR(IDS_STRING_TOKEN_TIME);
	m_listUserEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 60, -1);
	fm = TR(IDS_STRING_DEVICE_COUNT);
	m_listUserEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 60, -1);

	// jovision user list
	fm = TR(IDS_STRING_ID);
	m_listUserJovision.InsertColumn(++i, fm, LVCFMT_LEFT, 38, -1);
	fm = TR(IDS_STRING_NAME);
	m_listUserJovision.InsertColumn(++i, fm, LVCFMT_LEFT, 80, -1);
	fm = TR(IDS_STRING_PHONE);
	m_listUserJovision.InsertColumn(++i, fm, LVCFMT_LEFT, 100, -1);
	fm = TR(IDS_STRING_GLOBAL_USER_NAME);
	m_listUserJovision.InsertColumn(++i, fm, LVCFMT_LEFT, 100, -1);
	fm = TR(IDS_STRING_GLOBAL_USER_PASSWD);
	m_listUserJovision.InsertColumn(++i, fm, LVCFMT_LEFT, 60, -1);
	fm = TR(IDS_STRING_DEVICE_COUNT);
	m_listUserJovision.InsertColumn(++i, fm, LVCFMT_LEFT, 60, -1);

	// set device list's rect
	m_groupDevice.GetClientRect(rc);
	m_groupDevice.ClientToScreen(rc);
	ScreenToClient(rc);
	rc.DeflateRect(5, 15, 5, 5);
	m_listDeviceEzviz.MoveWindow(rc);
	m_listDeviceJovision.MoveWindow(rc);
	
	// device list ezviz
	i = -1;
	fm = TR(IDS_STRING_ID);
	m_listDeviceEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 38, -1);
	fm = TR(IDS_STRING_IDC_STATIC_025);
	m_listDeviceEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 100, -1);
	fm = TR(IDS_STRING_CAMERA_ID);
	m_listDeviceEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 80, -1);
	fm = TR(IDS_STRING_CAMERA_NAME);
	m_listDeviceEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 80, -1);
	fm = TR(IDS_STRING_CAMERA_NO);
	m_listDeviceEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 80, -1);
	fm = TR(IDS_STRING_DEFENCE);
	m_listDeviceEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 60, -1);
	fm = TR(IDS_STRING_DEVICE_ID);
	m_listDeviceEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 80, -1);
	fm = TR(IDS_STRING_DEVICE_NAME);
	m_listDeviceEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 80, -1);
	fm = TR(IDS_STRING_DEVICE_SERIAL);
	m_listDeviceEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 80, -1);
	fm = TR(IDS_STRING_IS_ENCRYPT);
	m_listDeviceEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 60, -1);
	fm = TR(IDS_STRING_IS_SHARED);
	m_listDeviceEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 60, -1);
	fm = TR(IDS_STRING_PIC_URL);
	m_listDeviceEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 60, -1);
	fm = TR(IDS_STRING_MACHINE_STATUS);
	m_listDeviceEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 50, -1);
	
	// device list jovision
	i = -1;
	fm = TR(IDS_STRING_ID);
	m_listDeviceJovision.InsertColumn(++i, fm, LVCFMT_LEFT, 38, -1);
	fm = TR(IDS_STRING_IDC_STATIC_025);
	m_listDeviceJovision.InsertColumn(++i, fm, LVCFMT_LEFT, 100, -1);
	fm = TR(IDS_STRING_CONN_BY_SSE);
	m_listDeviceJovision.InsertColumn(++i, fm, LVCFMT_LEFT, 60, -1);
	fm = TR(IDS_STRING_CLOUD_SSE_ID);
	m_listDeviceJovision.InsertColumn(++i, fm, LVCFMT_LEFT, 150, -1);
	fm = TR(IDS_STRING_DEVICE_IP);
	m_listDeviceJovision.InsertColumn(++i, fm, LVCFMT_LEFT, 120, -1);
	fm = TR(IDS_STRING_DEVICE_PORT);
	m_listDeviceJovision.InsertColumn(++i, fm, LVCFMT_LEFT, 60, -1);
	fm = TR(IDS_STRING_IDC_STATIC_021);
	m_listDeviceJovision.InsertColumn(++i, fm, LVCFMT_LEFT, 100, -1);
	fm = TR(IDS_STRING_IDC_STATIC_022);
	m_listDeviceJovision.InsertColumn(++i, fm, LVCFMT_LEFT, 100, -1);

	{
		m_nameDev.GetWindowRect(rc);
		ScreenToClient(rc);

		CRect rc2;
		m_chk_by_sse.GetWindowRect(rc2); ScreenToClient(rc2);
		rc2.top = rc.top; rc2.bottom = rc.bottom;
		m_chk_by_sse.MoveWindow(rc2);

		{
			CRect rc_static;
			m_static_dev_name.GetWindowRect(rc_static); ScreenToClient(rc_static);

			m_static_sse.GetWindowRect(rc2); ScreenToClient(rc2);
			rc2.top = rc_static.top; rc2.bottom = rc_static.bottom;
			m_static_sse.MoveWindow(rc2);
		}

		m_sse.GetWindowRect(rc2); ScreenToClient(rc2);
		rc2.top = rc.top; rc2.bottom = rc.bottom;
		m_sse.MoveWindow(rc2);
		
		{
			CRect rc3;

			{
				CRect rc_static;
				m_static_dev_name.GetWindowRect(rc_static); ScreenToClient(rc_static);

				m_static_ip.GetWindowRect(rc3); ScreenToClient(rc3);
				int h = rc3.Height(); rc3.bottom = rc_static.bottom; rc3.top = rc3.bottom - h;
				m_static_ip.MoveWindow(rc3);

				m_static_port.GetWindowRect(rc3); ScreenToClient(rc3);
				h = rc3.Height(); rc3.bottom = rc_static.bottom; rc3.top = rc3.bottom - h;
				m_static_port.MoveWindow(rc3);
			}
			

			m_ip.GetWindowRect(rc3); ScreenToClient(rc3);
			rc3.top = rc2.top; rc3.bottom = rc2.bottom;
			m_ip.MoveWindow(rc3);

			m_port.GetWindowRect(rc3); ScreenToClient(rc3);
			rc3.top = rc2.top; rc3.bottom = rc2.bottom;
			m_port.MoveWindow(rc3);
		}

		m_devCode.GetWindowRect(rc);
		ScreenToClient(rc);

		{
			CRect rc_static;
			m_static_verify_code.GetWindowRect(rc_static); ScreenToClient(rc_static);

			m_static_user_name.GetWindowRect(rc2); ScreenToClient(rc2);
			int h = rc2.Height(); rc2.bottom = rc_static.bottom; rc2.top = rc2.bottom - h;
			m_static_user_name.MoveWindow(rc2);

			m_static_user_passwd.GetWindowRect(rc2); ScreenToClient(rc2);
			h = rc2.Height(); rc2.bottom = rc_static.bottom; rc2.top = rc2.bottom - h;
			m_static_user_passwd.MoveWindow(rc2);
		}

		m_user_name.GetWindowRect(rc2); ScreenToClient(rc2);
		rc2.top = rc.top; rc2.bottom = rc.bottom;
		m_user_name.MoveWindow(rc2);

		m_user_passwd.GetWindowRect(rc2); ScreenToClient(rc2);
		rc2.top = rc.top; rc2.bottom = rc.bottom;
		m_user_passwd.MoveWindow(rc2);
	}

	InitUserList();

	g_videoUserMgrDlg = this;

	m_tab_users.SetCurSel(0);
	OnTcnSelchangeTabUsers(nullptr, nullptr);

	//SetTimer(TIMER_ID_CHECK_USER_ACCTOKEN_TIMEOUT, 60 * 1000, nullptr);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CVideoUserManagerDlg::ResetUserListSelectionInfoEzviz()
{
	AUTO_LOG_FUNCTION;
	CString txt;
	txt = TR(IDS_STRING_DEVICE_LIST);
	m_groupDevice.SetWindowTextW(txt);
	m_id.SetWindowTextW(L"");
	m_productor.SetWindowTextW(L"");
	m_name.SetWindowTextW(L"");
	m_phone.SetWindowTextW(L"");
	m_curSelDeviceInfoEzviz = nullptr;
	m_curSelUserInfoEzviz = nullptr;
	m_curselUserListItemEzviz = -1;

	ResetDeviceListSelectionInfoEzviz();
}


void CVideoUserManagerDlg::ResetUserListSelectionInfoJovision()
{
	AUTO_LOG_FUNCTION;
	CString txt;
	txt = TR(IDS_STRING_DEVICE_LIST);
	m_groupDevice.SetWindowTextW(txt);
	m_id.SetWindowTextW(L"");
	m_productor.SetWindowTextW(L"");
	m_name.SetWindowTextW(L"");
	m_phone.SetWindowTextW(L"");
	m_curSelDeviceInfoJovision = nullptr;
	m_curSelUserInfoJovision = nullptr;
	m_curselUserListItemJovision = -1;

	ResetDeviceListSelectionInfoJovision();
}


void CVideoUserManagerDlg::ResetDeviceListSelectionInfoEzviz()
{
	AUTO_LOG_FUNCTION;
	m_curselDeviceListItemEzviz = -1;
	m_curSelDeviceInfoEzviz = nullptr;

	m_idDev.SetWindowTextW(L"");
	m_nameDev.SetWindowTextW(L"");
	m_noteDev.SetWindowTextW(L"");
	m_devCode.SetWindowTextW(L"");
	m_zone.SetWindowTextW(L"");

	CString txt; txt = TR(IDS_STRING_BIND_ZONE);
	m_btnBindOrUnbind.SetWindowTextW(txt);
}


void CVideoUserManagerDlg::ResetDeviceListSelectionInfoJovision()
{
	AUTO_LOG_FUNCTION;
	m_curselDeviceListItemJovision = -1;
	m_curSelDeviceInfoJovision = nullptr;

	m_idDev.SetWindowTextW(L"");
	m_nameDev.SetWindowTextW(L"");
	m_noteDev.SetWindowTextW(L"");
	m_devCode.SetWindowTextW(L"");
	m_zone.SetWindowTextW(L"");

	CString txt; txt = TR(IDS_STRING_BIND_ZONE);
	m_btnBindOrUnbind.SetWindowTextW(txt);
}


void CVideoUserManagerDlg::InitUserList()
{
	AUTO_LOG_FUNCTION;
	m_listUserEzviz.DeleteAllItems();
	m_listUserJovision.DeleteAllItems();
	m_listDeviceEzviz.DeleteAllItems();
	m_listDeviceJovision.DeleteAllItems();
	ResetUserListSelectionInfoEzviz();
	ResetUserListSelectionInfoJovision();

	auto client = ipc::alarm_center_video_client::get_instance();
	video::video_user_info_list userList;
	client->get_video_user_list(userList);

	for (auto userInfo : userList) {
		const video::productor_info produtor = userInfo->get_productorInfo();
		if (produtor.get_productor() == video::EZVIZ) {
			auto ezvizUserInfo = std::dynamic_pointer_cast<video::ezviz::video_user_info_ezviz>(userInfo);
			InsertUserListEzviz(ezvizUserInfo);
		} else if (produtor.get_productor() == video::JOVISION) {
			auto normalUserInfo = std::dynamic_pointer_cast<video::jovision::video_user_info_jovision>(userInfo);
			InsertUserListJovision(normalUserInfo);
		}
	}

	if (g_videoPlayerDlg) {
		g_videoPlayerDlg->PostMessageW(WM_VIDEO_INFO_CHANGE);
	}
}


void CVideoUserManagerDlg::InsertUserListEzviz(video::ezviz::video_user_info_ezviz_ptr userInfo)
{
	int nResult = -1;
	LV_ITEM lvitem = { 0 };
	CString tmp = _T("");

	lvitem.lParam = 0;
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = m_listUserEzviz.GetItemCount();
	lvitem.iSubItem = 0;

	// ndx
	tmp.Format(_T("%d"), userInfo->get_id());
	lvitem.pszText = tmp.LockBuffer();
	nResult = m_listUserEzviz.InsertItem(&lvitem);
	tmp.UnlockBuffer();

	if (nResult != -1) {
		// name
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), userInfo->get_user_name().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listUserEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// phone
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), utf8::a2w(userInfo->get_user_phone()).c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listUserEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// access token
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), utf8::a2w(userInfo->get_acc_token()).c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listUserEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// token time
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), time_point_to_wstring(userInfo->get_token_time()).c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listUserEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// device count
		lvitem.iSubItem ++;
		tmp.Format(_T("%d"), userInfo->get_device_count());
		lvitem.pszText = tmp.LockBuffer();
		m_listUserEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		m_listUserEzviz.SetItemData(nResult, userInfo->get_id());
	}

}


void CVideoUserManagerDlg::UpdateUserListEzviz(int nItem, video::ezviz::video_user_info_ezviz_ptr userInfo)
{
	LV_ITEM lvitem = { 0 };
	CString tmp = _T("");

	lvitem.lParam = 0;
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = nItem;
	lvitem.iSubItem = 0;

	if (nItem != -1) {
		// ndx
		tmp.Format(_T("%d"), userInfo->get_id());
		lvitem.pszText = tmp.LockBuffer();
		m_listUserEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// name
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), userInfo->get_user_name().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listUserEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// phone
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), utf8::a2w(userInfo->get_user_phone()).c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listUserEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// access token
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), utf8::a2w(userInfo->get_acc_token()).c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listUserEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// token time
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), time_point_to_wstring(userInfo->get_token_time()).c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listUserEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// device count
		lvitem.iSubItem ++;
		tmp.Format(_T("%d"), userInfo->get_device_count());
		lvitem.pszText = tmp.LockBuffer();
		m_listUserEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		m_listUserEzviz.SetItemData(nItem, userInfo->get_id());
	}
}


void CVideoUserManagerDlg::InsertUserListJovision(video::jovision::video_user_info_jovision_ptr userInfo)
{
	int nResult = -1;
	LV_ITEM lvitem = { 0 };
	CString tmp = _T("");

	lvitem.lParam = 0;
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = m_listUserJovision.GetItemCount();
	lvitem.iSubItem = 0;

	// ndx
	tmp.Format(_T("%d"), userInfo->get_id());
	lvitem.pszText = tmp.LockBuffer();
	nResult = m_listUserJovision.InsertItem(&lvitem);
	tmp.UnlockBuffer();

	if (nResult != -1) {
		// name
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), userInfo->get_user_name().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listUserJovision.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// phone
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), utf8::a2w(userInfo->get_user_phone()).c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listUserJovision.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// global user name
		lvitem.iSubItem ++;
		tmp.Format(_T("%s"), userInfo->get_global_user_name().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listUserJovision.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// global user passwd
		lvitem.iSubItem ++;
		tmp.Format(_T("%s"), utf8::a2w(userInfo->get_global_user_passwd()).c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listUserJovision.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// device count
		lvitem.iSubItem++;
		tmp.Format(_T("%d"), userInfo->get_device_count());
		lvitem.pszText = tmp.LockBuffer();
		m_listUserJovision.SetItem(&lvitem);
		tmp.UnlockBuffer();

		m_listUserJovision.SetItemData(nResult, userInfo->get_id());
	}
}


void CVideoUserManagerDlg::UpdateUserListJovision(int nItem, video::jovision::video_user_info_jovision_ptr userInfo)
{
	LV_ITEM lvitem = { 0 };
	CString tmp = _T("");

	lvitem.lParam = 0;
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = nItem;
	lvitem.iSubItem = 0;

	if (nItem != -1) {
		// ndx
		tmp.Format(_T("%d"), userInfo->get_id());
		lvitem.pszText = tmp.LockBuffer();
		m_listUserJovision.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// name
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), userInfo->get_user_name().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listUserJovision.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// phone
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), utf8::a2w(userInfo->get_user_phone()).c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listUserJovision.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// global user name
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), userInfo->get_global_user_name().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listUserJovision.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// global user passwd
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), utf8::a2w(userInfo->get_global_user_passwd()).c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listUserJovision.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// device count
		lvitem.iSubItem++;
		tmp.Format(_T("%d"), userInfo->get_device_count());
		lvitem.pszText = tmp.LockBuffer();
		m_listUserJovision.SetItem(&lvitem);
		tmp.UnlockBuffer();

		m_listUserJovision.SetItemData(nItem, userInfo->get_id());
	}
}


void CVideoUserManagerDlg::UpdateDeviceListEzviz(int nItem, video::ezviz::video_device_info_ezviz_ptr deviceInfo)
{
	AUTO_LOG_FUNCTION;
	LV_ITEM lvitem = { 0 };
	CString tmp = _T("");

	lvitem.lParam = 0;
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = nItem;
	lvitem.iSubItem = 0;

	if (nItem != -1) {
		// note
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), deviceInfo->get_device_note().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// cameraId
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), utf8::a2w(deviceInfo->get_cameraId()).c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// cameraName
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), deviceInfo->get_cameraName().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// cameraNo
		lvitem.iSubItem++;
		tmp.Format(_T("%d"), deviceInfo->get_cameraNo());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// defence
		lvitem.iSubItem++;
		tmp.Format(_T("%d"), deviceInfo->get_defence());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// deviceID
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), utf8::a2w(deviceInfo->get_deviceId()).c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// deviceName
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), deviceInfo->get_deviceName().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// deviceSerial
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), utf8::a2w(deviceInfo->get_deviceSerial()).c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// isEncrypt
		lvitem.iSubItem++;
		tmp.Format(_T("%d"), deviceInfo->get_isEncrypt());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// isShared
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), utf8::a2w(deviceInfo->get_isShared()).c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// picUrl
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), utf8::a2w(deviceInfo->get_picUrl()).c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// status
		lvitem.iSubItem++;
		tmp.Format(_T("%d"), deviceInfo->get_status());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		m_listDeviceEzviz.SetItemData(nItem, deviceInfo->get_id());
	}
}


void CVideoUserManagerDlg::InsertDeviceListEzviz(video::ezviz::video_device_info_ezviz_ptr deviceInfo)
{
	AUTO_LOG_FUNCTION;
	int nResult = -1;
	LV_ITEM lvitem = { 0 };
	CString tmp = _T("");

	lvitem.lParam = 0;
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = m_listDeviceEzviz.GetItemCount();
	lvitem.iSubItem = 0;

	// ndx
	tmp.Format(_T("%d"), deviceInfo->get_id());
	lvitem.pszText = tmp.LockBuffer();
	nResult = m_listDeviceEzviz.InsertItem(&lvitem);
	tmp.UnlockBuffer();

	if (nResult != -1) {
		// note
		lvitem.iItem = nResult;
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), deviceInfo->get_device_note().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// cameraId
		lvitem.iItem = nResult;
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), utf8::a2w(deviceInfo->get_cameraId()).c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// cameraName
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), deviceInfo->get_cameraName().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// cameraNo
		lvitem.iSubItem++;
		tmp.Format(_T("%d"), deviceInfo->get_cameraNo());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// defence
		lvitem.iSubItem++;
		tmp.Format(_T("%d"), deviceInfo->get_defence());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// deviceID
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), utf8::a2w(deviceInfo->get_deviceId()).c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// deviceName
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), deviceInfo->get_deviceName().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// deviceSerial
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), utf8::a2w(deviceInfo->get_deviceSerial()).c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// isEncrypt
		lvitem.iSubItem++;
		tmp.Format(_T("%d"), deviceInfo->get_isEncrypt());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// isShared
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), utf8::a2w(deviceInfo->get_isShared()).c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// picUrl
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), utf8::a2w(deviceInfo->get_picUrl()).c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// status
		lvitem.iSubItem++;
		tmp.Format(_T("%d"), deviceInfo->get_status());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		m_listDeviceEzviz.SetItemData(nResult, deviceInfo->get_id());
	}
}


void CVideoUserManagerDlg::InsertDeviceListJovision(video::jovision::video_device_info_jovision_ptr deviceInfo)
{
	AUTO_LOG_FUNCTION;
	int nResult = -1;
	LV_ITEM lvitem = { 0 };
	CString tmp = _T("");

	lvitem.lParam = 0;
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = m_listDeviceJovision.GetItemCount();
	lvitem.iSubItem = 0;

	// ndx
	tmp.Format(_T("%d"), deviceInfo->get_id());
	lvitem.pszText = tmp.LockBuffer();
	nResult = m_listDeviceJovision.InsertItem(&lvitem);
	tmp.UnlockBuffer();

	if (nResult != -1) {
		// note
		lvitem.iItem = nResult;
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), deviceInfo->get_device_note().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceJovision.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// by sse ?
		lvitem.iItem = nResult;
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), TR(deviceInfo->get_by_sse() ? IDS_STRING_YES : IDS_STRING_NO));
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceJovision.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// cloud sse id
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), utf8::a2w(deviceInfo->get_sse()).c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceJovision.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// ip
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), utf8::a2w(deviceInfo->get_ip()).c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceJovision.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// port
		lvitem.iSubItem++;
		tmp.Format(_T("%d"), deviceInfo->get_port());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceJovision.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// user name
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), deviceInfo->get_user_name().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceJovision.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// user passwd
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), utf8::a2w(deviceInfo->get_user_passwd()).c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceJovision.SetItem(&lvitem);
		tmp.UnlockBuffer();

		m_listDeviceJovision.SetItemData(nResult, deviceInfo->get_id());
	}
}


void CVideoUserManagerDlg::UpdateDeviceListJovision(int nItem, video::jovision::video_device_info_jovision_ptr deviceInfo)
{
	AUTO_LOG_FUNCTION;
	LV_ITEM lvitem = { 0 };
	CString tmp = _T("");

	lvitem.lParam = 0;
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = nItem;
	lvitem.iSubItem = 0;

	if (nItem != -1) {

		// note
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), deviceInfo->get_device_note().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceJovision.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// by sse ?
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), TR(deviceInfo->get_by_sse() ? IDS_STRING_YES : IDS_STRING_NO));
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceJovision.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// cloud sse id
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), utf8::a2w(deviceInfo->get_sse()).c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceJovision.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// ip
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), utf8::a2w(deviceInfo->get_ip()).c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceJovision.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// port
		lvitem.iSubItem++;
		tmp.Format(_T("%d"), deviceInfo->get_port());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceJovision.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// user name
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), deviceInfo->get_user_name().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceJovision.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// user passwd
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), utf8::a2w(deviceInfo->get_user_passwd()).c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceJovision.SetItem(&lvitem);
		tmp.UnlockBuffer();

		m_listDeviceJovision.SetItemData(nItem, deviceInfo->get_id());
	}
}


void CVideoUserManagerDlg::OnLvnItemchangedListUserEzviz(NMHDR * pNMHDR, LRESULT * pResult)
{
	AUTO_LOG_FUNCTION;
	if (pResult)
		*pResult = 0;

	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if (pNMLV == nullptr || m_listUserEzviz.GetItemCount() == 0) {
		ResetUserListSelectionInfoEzviz();
		return;
	}
	auto user = video::video_manager::get_instance()->GetVideoUserEzviz(pNMLV->lParam);
	if (m_curSelUserInfoEzviz == user) {
		return;
	}
	m_curSelUserInfoEzviz = user;
	m_curselUserListItemEzviz = pNMLV->iItem;

	ShowUsersDeviceListEzviz(user);
}


void CVideoUserManagerDlg::OnLvnItemchangedListUserJovision(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (pResult) {
		*pResult = 0;
	}

	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	if (pNMLV == nullptr || m_listUserJovision.GetItemCount() == 0) {
		ResetUserListSelectionInfoJovision();
		return;
	}
	auto user = video::video_manager::get_instance()->GetVideoUserJovision(pNMLV->lParam);
	if (m_curSelUserInfoJovision == user) {
		return;
	}
	m_curSelUserInfoJovision = user;
	m_curselUserListItemJovision = pNMLV->iItem;

	ShowUsersDeviceListJovision(user);
	
}


void CVideoUserManagerDlg::OnLvnItemchangedListDeviceEzviz(NMHDR *pNMHDR, LRESULT *pResult)
{
	AUTO_LOG_FUNCTION;
	if (pResult)
		*pResult = 0;

	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if (pNMLV == nullptr || m_listDeviceEzviz.GetItemCount() == 0) {
		ResetDeviceListSelectionInfoEzviz();
		return;
	}
	video::ezviz::video_device_info_ezviz_ptr dev = video::video_manager::get_instance()->GetVideoDeviceInfoEzviz(pNMLV->lParam);
	if (m_curSelDeviceInfoEzviz == dev) {
		return;
	}
	m_curSelDeviceInfoEzviz = dev;
	m_curselDeviceListItemEzviz = pNMLV->iItem;
	ShowDeviceInfoEzviz(dev);
}


void CVideoUserManagerDlg::OnLvnItemchangedListDeviceJovision(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (pResult) {
		*pResult = 0;
	}

	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	if (pNMLV == nullptr || m_listDeviceJovision.GetItemCount() == 0) {
		ResetDeviceListSelectionInfoJovision();
		return;
	}
	auto dev = video::video_manager::get_instance()->GetVideoDeviceInfoJovision(pNMLV->lParam);
	if (m_curSelDeviceInfoJovision == dev) {
		return;
	}
	m_curSelDeviceInfoJovision = dev;
	m_curselDeviceListItemJovision = pNMLV->iItem;

	ShowDeviceInfoJovision(dev);
}


void CVideoUserManagerDlg::ShowDeviceInfoEzviz(video::ezviz::video_device_info_ezviz_ptr device)
{
	AUTO_LOG_FUNCTION;
	if (!device) {
		ResetDeviceListSelectionInfoEzviz();
		return;
	}

	m_btnPlayVideo.EnableWindow();

	CString txt;
	txt.Format(L"%d", device->get_id());
	m_idDev.SetWindowTextW(txt);
	txt.Format(L"%s", device->get_cameraName().c_str());
	m_nameDev.SetWindowTextW(txt);
	txt.Format(L"%s", device->get_device_note().c_str());
	m_noteDev.SetWindowTextW(txt);
	txt.Format(L"%s", utf8::a2w(device->get_secure_code()).c_str());
	m_devCode.SetWindowTextW(txt);

	std::list<video::zone_uuid> zoneList;
	device->get_zoneUuidList(zoneList);
	CString temp = L"";
	txt.Empty();
	if (!zoneList.empty()) {
		size_t ndx = 0;
		for (auto zone : zoneList) {
			if (zone._gg == core::INDEX_ZONE) {
				temp.Format(TR(IDS_STRING_FM_ADEMCO_ID) + L"[%03d]", zone._ademco_id, zone._zone_value);
			} else {
				temp.Format(TR(IDS_STRING_FM_ADEMCO_ID) + L"[%03d][%02d]", zone._ademco_id, zone._zone_value, zone._gg);
			}
			if (ndx != zoneList.size() - 1) {
				temp += L",";
			}
			txt += temp;
		}
		m_zone.SetWindowTextW(txt);
	} else {
		m_zone.SetWindowTextW(L"");
	}
}


void CVideoUserManagerDlg::ShowDeviceInfoJovision(video::jovision::video_device_info_jovision_ptr device)
{
	AUTO_LOG_FUNCTION;

	if (!device) {
		ResetDeviceListSelectionInfoJovision();
		return;
	}

	m_btnPlayVideo.EnableWindow();

	CString txt;
	txt.Format(L"%d", device->get_id());
	m_idDev.SetWindowTextW(txt);
	
	bool by_sse = device->get_by_sse();
	m_chk_by_sse.SetCheck(by_sse);
	m_static_sse.ShowWindow(by_sse ? SW_SHOW : SW_HIDE);
	m_sse.ShowWindow(by_sse ? SW_SHOW : SW_HIDE);
	m_static_ip.ShowWindow(by_sse ? SW_HIDE : SW_SHOW);
	m_ip.ShowWindow(by_sse ? SW_HIDE : SW_SHOW);
	m_static_port.ShowWindow(by_sse ? SW_HIDE : SW_SHOW);
	m_port.ShowWindow(by_sse ? SW_HIDE : SW_SHOW);

	if (by_sse) {
		txt.Format(L"%s", utf8::a2w(device->get_sse()).c_str());
		m_sse.SetWindowTextW(txt);
	} else {
		m_ip.SetWindowTextW(utf8::a2w(device->get_ip()).c_str());
		txt.Format(L"%d", device->get_port());
		m_port.SetWindowTextW(txt);
	}
	
	txt.Format(L"%s", device->get_device_note().c_str());
	m_noteDev.SetWindowTextW(txt);

	m_user_name.SetWindowTextW(device->get_user_name().c_str());
	m_user_passwd.SetWindowTextW(utf8::a2w(device->get_user_passwd()).c_str());

	std::list<video::zone_uuid> zoneList;
	device->get_zoneUuidList(zoneList);
	CString temp = L"";
	txt.Empty();
	if (!zoneList.empty()) {
		size_t ndx = 0;
		for (auto zone : zoneList) {
			if (zone._gg == core::INDEX_ZONE) {
				temp.Format(TR(IDS_STRING_FM_ADEMCO_ID) + L"[%03d]", zone._ademco_id, zone._zone_value);
			} else {
				temp.Format(TR(IDS_STRING_FM_ADEMCO_ID) + L"[%03d][%02d]", zone._ademco_id, zone._zone_value, zone._gg);
			}
			if (ndx != zoneList.size() - 1) {
				temp += L",";
			}
			txt += temp;
		}
		m_zone.SetWindowTextW(txt);
	} else {
		m_zone.SetWindowTextW(L"");
	}
}

//
//bool CVideoUserManagerDlg::CheckZoneInfoExsist(const video::zone_uuid& zone)
//{
//	do {
//		auto mgr = core::alarm_machine_manager::get_instance();
//		core::alarm_machine_ptr machine = mgr->GetMachine(zone._ademco_id);
//		if (!machine)
//			break;
//		core::zone_info_ptr zoneInfo = machine->GetZone(zone._zone_value);
//		if (!zoneInfo)
//			break;
//
//		core::alarm_machine_ptr subMachine = zoneInfo->GetSubMachineInfo();
//		if (zone._gg == core::INDEX_ZONE) {
//			if (subMachine != nullptr)
//				break;
//		} else {
//			if (subMachine == nullptr)
//				break;
//			if (zone._gg == core::INDEX_SUB_MACHINE) 
//				break;
//			if (subMachine->GetZone(zone._gg) == nullptr)
//				break;
//		}
//		
//		return true;
//	} while (0);
//	return false;
//}


void CVideoUserManagerDlg::ShowUsersDeviceListEzviz(video::ezviz::video_user_info_ezviz_ptr user)
{
	AUTO_LOG_FUNCTION;

	if (!user) {
		ResetUserListSelectionInfoEzviz();
		return;
	}

	CString fm, txt;
	fm = TR(IDS_STRING_FM_USERS_DEV_LIST);
	txt.Format(fm, user->get_user_name().c_str(), user->get_device_count());
	m_groupDevice.SetWindowTextW(txt);	

	txt.Format(L"%d", user->get_id());
	m_id.SetWindowTextW(txt);
	txt.Format(L"%s", user->get_productorInfo().get_formatted_name().c_str());
	m_productor.SetWindowTextW(txt);
	txt.Format(L"%s", user->get_user_name().c_str());
	m_name.SetWindowTextW(txt);
	txt.Format(L"%s", utf8::a2w(user->get_user_phone()).c_str());
	m_phone.SetWindowTextW(txt);
	txt.Format(L"%s", utf8::a2w(user->get_acc_token()).c_str());
	m_token_or_name.SetWindowTextW(txt);
	txt.Format(L"%s", time_point_to_wstring(user->get_token_time()).c_str());
	m_time_or_passwd.SetWindowTextW(txt);

	m_listDeviceEzviz.DeleteAllItems();

	video::video_device_info_list list;
	user->GetDeviceList(list);
	
	ResetDeviceListSelectionInfoEzviz();
	for (auto i : list) {
		video::ezviz::video_device_info_ezviz_ptr device = std::dynamic_pointer_cast<video::ezviz::video_device_info_ezviz>(i);
		InsertDeviceListEzviz(device);
	}
	
}


void CVideoUserManagerDlg::ShowUsersDeviceListJovision(video::jovision::video_user_info_jovision_ptr user)
{
	AUTO_LOG_FUNCTION;

	if (!user) {
		ResetUserListSelectionInfoJovision();
		return;
	}

	CString fm, txt;
	fm = TR(IDS_STRING_FM_USERS_DEV_LIST);
	txt.Format(fm, user->get_user_name().c_str(), user->get_device_count());
	m_groupDevice.SetWindowTextW(txt);

	txt.Format(L"%d", user->get_id());
	m_id.SetWindowTextW(txt);
	txt.Format(L"%s", user->get_productorInfo().get_formatted_name().c_str());
	m_productor.SetWindowTextW(txt);
	txt.Format(L"%s", user->get_user_name().c_str());
	m_name.SetWindowTextW(txt);
	txt.Format(L"%s", utf8::a2w(user->get_user_phone()).c_str());
	m_phone.SetWindowTextW(txt);
	txt.Format(L"%s", user->get_global_user_name().c_str());
	m_token_or_name.SetWindowTextW(txt);
	txt.Format(L"%s", utf8::a2w(user->get_global_user_passwd()).c_str());
	m_time_or_passwd.SetWindowTextW(txt);

	m_listDeviceJovision.DeleteAllItems();

	video::video_device_info_list list;
	user->GetDeviceList(list);

	ResetDeviceListSelectionInfoJovision();
	for (auto i : list) {
		video::jovision::video_device_info_jovision_ptr device = std::dynamic_pointer_cast<video::jovision::video_device_info_jovision>(i);
		InsertDeviceListJovision(device);
	}
}


void CVideoUserManagerDlg::OnBnClickedButtonSaveChange()
{
	AUTO_LOG_FUNCTION;
	int ndx = m_tab_users.GetCurSel(); if (ndx < 0)return;
	if (ndx == 0) {
		if (m_curSelUserInfoEzviz == nullptr || m_curselUserListItemEzviz == -1) { return; }
	} else if (ndx == 1) {
		if (m_curSelUserInfoJovision == nullptr || m_curselUserListItemJovision == -1) { return; }
	} else {
		assert(0);
		return;
	}

	if (ndx == 0) {
		CString name; m_name.GetWindowTextW(name);
		if (name.Compare(m_curSelUserInfoEzviz->get_user_name().c_str()) != 0) {
			if (m_curSelUserInfoEzviz->execute_set_user_name(name.LockBuffer())) {
				UpdateUserListEzviz(m_curselUserListItemEzviz, m_curSelUserInfoEzviz);
			}
			name.UnlockBuffer();
		}
	} else if (ndx == 1) {
		CString name, passwd;
		m_token_or_name.GetWindowTextW(name);
		m_time_or_passwd.GetWindowTextW(passwd);
		if (name.Compare(m_curSelUserInfoJovision->get_global_user_name().c_str()) == 0
			&& passwd.Compare(utf8::a2w(m_curSelUserInfoJovision->get_global_user_passwd()).c_str()) == 0) {
			return;
		}

		m_curSelUserInfoJovision->execute_set_global_user_name((LPCTSTR)name);
		m_curSelUserInfoJovision->execute_set_global_user_passwd(utf8::w2a((LPCTSTR)passwd));
		UpdateUserListJovision(m_curselUserListItemJovision, m_curSelUserInfoJovision);
	}
}


void CVideoUserManagerDlg::OnBnClickedButtonDelUser()
{
	AUTO_LOG_FUNCTION;
	int ndx = m_tab_users.GetCurSel(); if (ndx < 0)return;
	if (ndx == 0) {
		if (m_curSelUserInfoEzviz == nullptr || m_curselUserListItemEzviz == -1) { return; }
	} else if (ndx == 1) {
		if (m_curSelUserInfoJovision == nullptr || m_curselUserListItemJovision == -1) { return; }
	} else {
		assert(0);
		return;
	}

	CString info; info = TR(IDS_STRING_CONFIRM_DEL_VIDEO_USER);
	int ret = MessageBox(info, L"", MB_OKCANCEL | MB_ICONWARNING);
	if (ret != IDOK)return;

	if (ndx == 0) {
		if (video::video_manager::get_instance()->DeleteVideoUserEzviz(m_curSelUserInfoEzviz)) {
			InitUserList();
			OnLvnItemchangedListUserEzviz(nullptr, nullptr);
		}
	} else if (ndx == 1) {
		if (video::video_manager::get_instance()->DeleteVideoUserJovision(m_curSelUserInfoJovision)) {
			InitUserList();
			OnLvnItemchangedListUserJovision(nullptr, nullptr);
		}
	}
}


void CVideoUserManagerDlg::OnBnClickedButtonAddUser()
{
	AUTO_LOG_FUNCTION;
	auto vmgr = video::video_manager::get_instance();

	int ndx = m_tab_users.GetCurSel(); if (ndx < 0)return;
	int ret = 0;
	if (ndx == 0) {
		ret = 1;
	}else if (ndx == 1) {
		ret = 2;
	} else {
		return;
	}

	if (ret == 1) {
		CAddVideoUserEzvizDlg dlg(this);
		if (IDOK != dlg.DoModal())
			return;

		CAddVideoUserProgressDlg prgdlg(this);
		prgdlg.m_name = dlg.m_strName;
		prgdlg.m_phone = dlg.m_strPhone;
		prgdlg.DoModal();

		auto result = prgdlg.m_result;
		CString e;
		if (result == video::video_manager::RESULT_OK) {
			InitUserList();
		} else if (result == video::video_manager::RESULT_INSERT_TO_DB_FAILED) {

		} else if (result == video::video_manager::RESULT_PRIVATE_CLOUD_CONNECT_FAILED_OR_USER_NOT_EXIST) {
			e = TR(IDS_STRING_PRIVATE_CLOUD_CONN_FAIL_OR_USER_NOT_EXSIST);
			MessageBox(e, L"", MB_ICONERROR);
		} else if (result == video::video_manager::RESULT_USER_ALREADY_EXSIST) {

		} else { assert(0); }
	} else if (ret == 2) {
		CAddVideoUserJovisionDlg dlg(this);
		if (IDOK != dlg.DoModal()) { 
			return; 
		}

		auto user = std::make_shared<video::jovision::video_user_info_jovision>();
		user->set_user_name((LPCTSTR)dlg.m_strName);
		user->set_user_phone(utf8::w2a((LPCTSTR)dlg.m_strPhone));
		user->set_global_user_name((LPCTSTR)dlg.m_strDefaultName);
		user->set_global_user_passwd(utf8::w2a((LPCTSTR)dlg.m_strDefaultPasswd));

		auto result = vmgr->AddVideoUserJovision(user);
		CString e;
		if (result == video::video_manager::RESULT_OK) {
			InitUserList();
		} else if (result == video::video_manager::RESULT_INSERT_TO_DB_FAILED) {

		} else if (result == video::video_manager::RESULT_PRIVATE_CLOUD_CONNECT_FAILED_OR_USER_NOT_EXIST) {
			e = TR(IDS_STRING_PRIVATE_CLOUD_CONN_FAIL_OR_USER_NOT_EXSIST);
			MessageBox(e, L"", MB_ICONERROR);
		} else if (result == video::video_manager::RESULT_USER_ALREADY_EXSIST) {

		} else { assert(0); }
	}
}


void CVideoUserManagerDlg::OnBnClickedButtonRefreshDeviceList()
{
	AUTO_LOG_FUNCTION;
	if (m_curSelUserInfoEzviz == nullptr || m_curselUserListItemEzviz == -1) { return; }
	auto user = std::dynamic_pointer_cast<video::ezviz::video_user_info_ezviz>(m_curSelUserInfoEzviz);
	auto mgr = video::video_manager::get_instance();
	auto result = mgr->RefreshUserEzvizDeviceList(user);
	if (result == video::video_manager::RESULT_OK) {
		ShowUsersDeviceListEzviz(user);
		if (g_videoPlayerDlg) {
			g_videoPlayerDlg->PostMessageW(WM_VIDEO_INFO_CHANGE);
		}
	} else if (result == video::video_manager::RESULT_PRIVATE_CLOUD_CONNECT_FAILED_OR_USER_NOT_EXIST) {
		CString e; e = TR(IDS_STRING_PRIVATE_CLOUD_CONN_FAIL_OR_USER_NOT_EXSIST);
		MessageBox(e, L"", MB_ICONERROR);
	}
}
	

void CVideoUserManagerDlg::OnBnClickedButtonBindOrUnbind()
{
	AUTO_LOG_FUNCTION;
	/*int ndx = m_tab_users.GetCurSel(); if (ndx < 0)return;
	if (ndx == 0) {
		if (m_curSelUserInfoEzviz == nullptr || m_curselUserListItemEzviz == -1) { return; }
		video::ezviz::video_device_info_ezviz_ptr dev = std::dynamic_pointer_cast<video::ezviz::video_device_info_ezviz>(m_curSelDeviceInfoEzviz);
		auto mgr = video::video_manager::get_instance();
		{
			CChooseZoneDlg dlg(this);
			if (IDOK != dlg.DoModal()) return;
			if (mgr->BindZoneAndDevice(dlg.m_zone, dev)) {
				ShowDeviceInfoEzviz(dev);
				if (m_observerDlg) {
					m_observerDlg->PostMessageW(WM_VIDEO_INFO_CHANGE);
				}
			}
		}
	} else if (ndx == 1) {
		if (m_curSelUserInfoJovision == nullptr || m_curselUserListItemJovision == -1) { return; }
		auto dev = std::dynamic_pointer_cast<video::jovision::video_device_info_jovision>(m_curSelDeviceInfoJovision);
		auto mgr = video::video_manager::get_instance();

		CChooseZoneDlg dlg(this);
		if (IDOK != dlg.DoModal()) return;
		if (mgr->BindZoneAndDevice(dlg.m_zone, dev)) {
			ShowDeviceInfoJovision(dev);
			if (m_observerDlg) {
				m_observerDlg->PostMessageW(WM_VIDEO_INFO_CHANGE);
			}
		}

	} else {
		assert(0);
		return;
	}*/

}


void CVideoUserManagerDlg::OnBnClickedCheckAutoPlayVideo()
{

}


void CVideoUserManagerDlg::OnBnClickedButtonSaveDev()
{
	AUTO_LOG_FUNCTION;
	int ndx = m_tab_users.GetCurSel(); if (ndx != 0 && ndx != 1) return;

	if (ndx == 0) {
		if (m_curSelDeviceInfoEzviz == nullptr || m_curselDeviceListItemEzviz == -1) { return; }
		do {
			CString note, code;
			m_noteDev.GetWindowTextW(note);
			m_devCode.GetWindowTextW(code);
			if (!code.IsEmpty()) {
				if (!video::ezviz::video_device_info_ezviz::IsValidVerifyCode(utf8::w2a((LPCTSTR)code))) {
					note = TR(IDS_STRING_DEVICE_CODE_INVALID);
					MessageBox(note, L"", MB_ICONERROR);
					break;
				}
			}
			bool changed = false;
			if (note.Compare(m_curSelDeviceInfoEzviz->get_device_note().c_str()) != 0) {
				changed = true;
				m_curSelDeviceInfoEzviz->set_device_note(note.LockBuffer());
				note.UnlockBuffer();
			}
			if (code.Compare(utf8::a2w(m_curSelDeviceInfoEzviz->get_secure_code()).c_str()) != 0) {
				changed = true;
				m_curSelDeviceInfoEzviz->set_secure_code(utf8::w2a((LPCTSTR)code));
			}

			if (changed) {
				m_curSelDeviceInfoEzviz->execute_update_info();
			}
		} while (0);
		UpdateDeviceListEzviz(m_curselDeviceListItemEzviz, m_curSelDeviceInfoEzviz);
		ShowDeviceInfoEzviz(m_curSelDeviceInfoEzviz);

	} else if (ndx == 1) {
		if (m_curSelDeviceInfoJovision == nullptr || m_curselDeviceListItemJovision == -1) { return; }

		do {
			bool changed = false;

			CString txt;
			m_noteDev.GetWindowTextW(txt);

			if (txt.Compare(m_curSelDeviceInfoJovision->get_device_note().c_str()) != 0) {
				changed = true;
				m_curSelDeviceInfoJovision->set_device_note(LPCTSTR(txt));
			}
			
			bool by_sse = m_chk_by_sse.GetCheck() > 0;
			if (by_sse != m_curSelDeviceInfoJovision->get_by_sse()) {
				changed = true;
				m_curSelDeviceInfoJovision->set_by_sse(by_sse);
			}

			if (by_sse) {
				m_sse.GetWindowTextW(txt);
				if (txt.Compare(utf8::a2w(m_curSelDeviceInfoJovision->get_sse()).c_str()) != 0) {
					changed = true;
					m_curSelDeviceInfoJovision->set_sse(utf8::w2a((LPCTSTR)txt));
				}
			} else {
				m_ip.GetWindowTextW(txt);
				if (txt.Compare(utf8::a2w(m_curSelDeviceInfoJovision->get_ip()).c_str()) != 0) {
					changed = true;
					m_curSelDeviceInfoJovision->set_ip(utf8::w2a((LPCTSTR)txt));
				}

				m_port.GetWindowTextW(txt);
				int port = _ttoi(txt);
				if (port != m_curSelDeviceInfoJovision->get_port()) {
					changed = true;
					m_curSelDeviceInfoJovision->set_port(port);
				}
			}

			m_user_name.GetWindowTextW(txt);
			if (txt.Compare(m_curSelDeviceInfoJovision->get_user_name().c_str()) != 0) {
				changed = true;
				m_curSelDeviceInfoJovision->set_user_name((LPCTSTR)txt);
			}

			m_user_passwd.GetWindowTextW(txt);
			if (txt.Compare(utf8::a2w(m_curSelDeviceInfoJovision->get_user_passwd()).c_str()) != 0) {
				changed = true;
				m_curSelDeviceInfoJovision->set_user_passwd(utf8::w2a((LPCTSTR)txt));
			}

			if (changed) {
				m_curSelDeviceInfoJovision->execute_update_info();
			}
		} while (0);
		UpdateDeviceListJovision(m_curselDeviceListItemJovision, m_curSelDeviceInfoJovision);
		ShowDeviceInfoJovision(m_curSelDeviceInfoJovision);

	} else {
		assert(0);
	}
}


void CVideoUserManagerDlg::OnBnClickedButtonPlay()
{
	int ndx = m_tab_users.GetCurSel(); if (ndx != 0 && ndx != 1)return;
	if (ndx == 0) {
		if (m_curSelDeviceInfoEzviz == nullptr || m_curselDeviceListItemEzviz == -1) { return; }
		g_videoPlayerDlg->PlayVideoByDevice(m_curSelDeviceInfoEzviz, util::CConfigHelper::get_instance()->get_default_video_level());
	} else if (ndx == 1) {
		if (m_curSelDeviceInfoJovision == nullptr || m_curselDeviceListItemJovision == -1) { return; }
		g_videoPlayerDlg->PlayVideoByDevice(m_curSelDeviceInfoJovision, util::CConfigHelper::get_instance()->get_default_video_level());
	} else {
		assert(0);
	}
}


void CVideoUserManagerDlg::OnOK()
{
	return;
}


void CVideoUserManagerDlg::OnClose()
{
	ShowWindow(SW_HIDE);
}


void CVideoUserManagerDlg::OnTimer(UINT_PTR nIDEvent)
{
	//if (TIMER_ID_CHECK_USER_ACCTOKEN_TIMEOUT == nIDEvent) {
		//video::video_manager::get_instance()->CheckUserAcctkenTimeout();
	//}

	CDialogEx::OnTimer(nIDEvent);
}


void CVideoUserManagerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	g_videoUserMgrDlg = nullptr;
}


void CVideoUserManagerDlg::OnBnClickedButtonUnbind()
{
	AUTO_LOG_FUNCTION;
	int ndx = m_tab_users.GetCurSel(); if (ndx < 0)return;
	if (ndx == 0) {
		if (m_curSelDeviceInfoEzviz == nullptr || m_curselDeviceListItemEzviz == -1) { return; }
		auto dev = std::dynamic_pointer_cast<video::ezviz::video_device_info_ezviz>(m_curSelDeviceInfoEzviz);
		auto mgr = video::video_manager::get_instance();
		std::list<video::zone_uuid> zoneList;
		dev->get_zoneUuidList(zoneList);
		for (auto zone : zoneList) {
			mgr->UnbindZoneAndDevice(zone);
		}
		ShowDeviceInfoEzviz(dev);
		if (m_observerDlg) {
			m_observerDlg->PostMessageW(WM_VIDEO_INFO_CHANGE);
		}
	} else if (ndx == 1) {
		if (m_curSelDeviceInfoJovision == nullptr || m_curselDeviceListItemEzviz == -1) { return; }
		auto dev = std::dynamic_pointer_cast<video::jovision::video_device_info_jovision>(m_curSelDeviceInfoJovision);
		auto mgr = video::video_manager::get_instance();
		std::list<video::zone_uuid> zoneList;
		dev->get_zoneUuidList(zoneList);
		for (auto zone : zoneList) {
			mgr->UnbindZoneAndDevice(zone);
		}
		ShowDeviceInfoJovision(dev);
		if (m_observerDlg) {
			m_observerDlg->PostMessageW(WM_VIDEO_INFO_CHANGE);
		}
	} else {
		assert(0);
	}
}


afx_msg LRESULT CVideoUserManagerDlg::OnVideoInfoChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	int ndx = m_tab_users.GetCurSel(); if (ndx < 0)return 0;
	if (ndx == 0) {
		if (m_curSelDeviceInfoEzviz == nullptr || m_curselDeviceListItemEzviz == -1) { return 0; }
		auto dev = std::dynamic_pointer_cast<video::ezviz::video_device_info_ezviz>(m_curSelDeviceInfoEzviz);
		ShowDeviceInfoEzviz(dev);
	} else if (ndx == 1) {
		if (m_curSelDeviceInfoJovision == nullptr || m_curselDeviceListItemJovision == -1) { return 0; }
		auto dev = std::dynamic_pointer_cast<video::jovision::video_device_info_jovision>(m_curSelDeviceInfoJovision);
		ShowDeviceInfoJovision(dev);
	} else {
		assert(0);
	}
	return 0;
}


void CVideoUserManagerDlg::OnBnClickedButtonDelDevice()
{
	AUTO_LOG_FUNCTION;
	int ndx = m_tab_users.GetCurSel(); if (ndx < 0)return;
	bool deleted = false;
	if (ndx == 0) {
		if (m_curSelDeviceInfoEzviz == nullptr || m_curselDeviceListItemEzviz == -1) { return; }
		auto dev = std::dynamic_pointer_cast<video::ezviz::video_device_info_ezviz>(m_curSelDeviceInfoEzviz);
		auto user = std::dynamic_pointer_cast<video::ezviz::video_user_info_ezviz>(dev->get_userInfo());
		user->DeleteVideoDevice(dev);
		m_curSelDeviceInfoEzviz = nullptr;
		m_listDeviceEzviz.DeleteItem(m_curselDeviceListItemEzviz);
		m_listDeviceEzviz.SetItemState(m_curselDeviceListItemEzviz, LVNI_FOCUSED | LVIS_SELECTED, LVNI_FOCUSED | LVIS_SELECTED);
		deleted = true;
	} else if (ndx == 1) {
		if (m_curSelDeviceInfoJovision == nullptr || m_curselDeviceListItemJovision == -1) { return; }
		auto dev = std::dynamic_pointer_cast<video::jovision::video_device_info_jovision>(m_curSelDeviceInfoJovision);
		auto user = std::dynamic_pointer_cast<video::jovision::video_user_info_jovision>(dev->get_userInfo());
		user->DeleteVideoDevice(dev);
		m_curSelDeviceInfoJovision = nullptr;
		m_listDeviceJovision.DeleteItem(m_curselDeviceListItemJovision);
		m_listDeviceJovision.SetItemState(m_curselDeviceListItemJovision, LVNI_FOCUSED | LVIS_SELECTED, LVNI_FOCUSED | LVIS_SELECTED);
		deleted = true;
	} else {
		assert(0);
	}
	
	if (deleted && g_videoPlayerDlg) {
		g_videoPlayerDlg->PostMessageW(WM_VIDEO_INFO_CHANGE);
	}
}


void CVideoUserManagerDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	if (bShow) {
		/*if (!m_cur_user_changed_observer) {
			m_cur_user_changed_observer = std::make_shared<CurUserChangedObserver>(this);
			core::user_manager::get_instance()->register_observer(m_cur_user_changed_observer);
		}
		m_cur_user_changed_observer->on_update(core::user_manager::get_instance()->GetCurUserInfo());*/
	}
}


void CVideoUserManagerDlg::OnNMDblclkListDeviceEzviz(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;

	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if (pNMLV == nullptr || m_listDeviceEzviz.GetItemCount() == 0 || pNMLV->iItem == -1) {
		ResetDeviceListSelectionInfoEzviz();
		return;
	}
	auto data = m_listDeviceEzviz.GetItemData(pNMLV->iItem);
	video::ezviz::video_device_info_ezviz_ptr dev = video::video_manager::get_instance()->GetVideoDeviceInfoEzviz(data);
	if (!dev) {
		return;
	}

	m_curSelDeviceInfoEzviz = dev;
	m_curselDeviceListItemEzviz = pNMLV->iItem;

	if (g_videoPlayerDlg)
		g_videoPlayerDlg->PlayVideoByDevice(dev, util::CConfigHelper::get_instance()->get_default_video_level());
}


void CVideoUserManagerDlg::OnNMDblclkListDeviceJovision(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if (pNMLV == nullptr || m_listDeviceJovision.GetItemCount() == 0 || pNMLV->iItem == -1) {
		ResetDeviceListSelectionInfoJovision();
		return;
	}
	auto data = m_listDeviceJovision.GetItemData(pNMLV->iItem);
	video::jovision::video_device_info_jovision_ptr dev = video::video_manager::get_instance()->GetVideoDeviceInfoJovision(data);
	if (!dev) {
		return;
	}

	m_curSelDeviceInfoJovision = dev;
	m_curselDeviceListItemJovision = pNMLV->iItem;

	if (g_videoPlayerDlg)
		g_videoPlayerDlg->PlayVideoByDevice(dev, util::CConfigHelper::get_instance()->get_default_video_level());
}


void CVideoUserManagerDlg::OnTcnSelchangeTabUsers(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	if (pResult) {
		*pResult = 0;
	}

	int ndx = m_tab_users.GetCurSel(); if (ndx < 0)return;
	m_listUserEzviz.ShowWindow(ndx == 0 ? SW_SHOW : SW_HIDE);
	m_btnRefreshDeviceList.ShowWindow(ndx == 0 ? SW_SHOW : SW_HIDE);
	m_listDeviceEzviz.ShowWindow(ndx == 0 ? SW_SHOW : SW_HIDE);

	m_listUserJovision.ShowWindow(ndx == 1 ? SW_SHOW : SW_HIDE);
	m_listDeviceJovision.ShowWindow(ndx == 1 ? SW_SHOW : SW_HIDE);
	m_btnAddDevice.ShowWindow(ndx == 1 ? SW_SHOW : SW_HIDE);

	m_name.SetReadOnly(ndx == 1);
	m_phone.SetReadOnly(ndx == 0);
	m_token_or_name.SetReadOnly(ndx == 0);
	m_time_or_passwd.SetReadOnly(ndx == 0);

	m_static_dev_name.ShowWindow(ndx == 0 ? SW_SHOW : SW_HIDE);
	m_nameDev.ShowWindow(ndx == 0 ? SW_SHOW : SW_HIDE);
	m_static_verify_code.ShowWindow(ndx == 0 ? SW_SHOW : SW_HIDE);
	m_devCode.ShowWindow(ndx == 0 ? SW_SHOW : SW_HIDE);
	m_static_note.ShowWindow(ndx == 0 ? SW_SHOW : SW_HIDE);

	m_chk_by_sse.ShowWindow(ndx == 1 ? SW_SHOW : SW_HIDE);
	m_chk_by_sse.SetCheck(ndx == 1);
	m_static_sse.ShowWindow(ndx == 1 ? SW_SHOW : SW_HIDE);
	m_sse.ShowWindow(ndx == 1 ? SW_SHOW : SW_HIDE);
	m_static_ip.ShowWindow(SW_HIDE);
	m_ip.ShowWindow(SW_HIDE);
	m_static_port.ShowWindow(SW_HIDE);
	m_port.ShowWindow(SW_HIDE);

	m_static_user_name.ShowWindow(ndx == 1 ? SW_SHOW : SW_HIDE);
	m_user_name.ShowWindow(ndx == 1 ? SW_SHOW : SW_HIDE);
	m_static_user_passwd.ShowWindow(ndx == 1 ? SW_SHOW : SW_HIDE);
	m_user_passwd.ShowWindow(ndx == 1 ? SW_SHOW : SW_HIDE);

	if (ndx == 0) {
		m_static_token_or_name.SetWindowTextW(TR(IDS_STRING_ACCESS_TOKEN) + L":");
		m_static_time_or_passwd.SetWindowTextW(TR(IDS_STRING_TOKEN_TIME) + L":");
		ShowUsersDeviceListEzviz(m_curSelUserInfoEzviz);
		//m_listUserEzviz.SetSelectedColumn()
	} else if (ndx == 1) {
		m_static_token_or_name.SetWindowTextW(TR(IDS_STRING_GLOBAL_USER_NAME) + L":");
		m_static_time_or_passwd.SetWindowTextW(TR(IDS_STRING_GLOBAL_USER_PASSWD) + L":");
		ShowUsersDeviceListJovision(m_curSelUserInfoJovision);
	}
}


void CVideoUserManagerDlg::OnBnClickedButtonAddDevice()
{
	int ndx = m_tab_users.GetCurSel(); if (ndx < 0)return;
	if (ndx != 1 || m_curSelUserInfoJovision == nullptr) {
		return;
	}

	CAddVideoDeviceJovisionDlg dlg(this); dlg.user_ = m_curSelUserInfoJovision;
	if (dlg.DoModal() != IDOK) {
		return;
	}

	auto vmgr = video::video_manager::get_instance();
	if (vmgr->AddVideoDeviceJovision(m_curSelUserInfoJovision, dlg.device_)) {
		ShowUsersDeviceListJovision(m_curSelUserInfoJovision);
		
		//CString txt;
		//txt.Format(L"")
		//auto hr = core::history_record_manager::get_instance();

		if (g_videoPlayerDlg) {
			g_videoPlayerDlg->PostMessageW(WM_VIDEO_INFO_CHANGE);
		}
	}
}


void CVideoUserManagerDlg::OnBnClickedCheckBySseId()
{
	int by_sse = m_chk_by_sse.GetCheck();
	m_static_sse.ShowWindow(by_sse ? SW_SHOW : SW_HIDE);
	m_sse.ShowWindow(by_sse ? SW_SHOW : SW_HIDE);
	m_static_ip.ShowWindow(by_sse ? SW_HIDE : SW_SHOW);
	m_ip.ShowWindow(by_sse ? SW_HIDE : SW_SHOW);
	m_static_port.ShowWindow(by_sse ? SW_HIDE : SW_SHOW);
	m_port.ShowWindow(by_sse ? SW_HIDE : SW_SHOW);

	if (m_curSelDeviceInfoJovision) {
		CString txt;
		if (by_sse) {
			txt.Format(L"%s", utf8::a2w(m_curSelDeviceInfoJovision->get_sse()).c_str());
			m_sse.SetWindowTextW(txt);
		} else {
			m_ip.SetWindowTextW(utf8::a2w(m_curSelDeviceInfoJovision->get_ip()).c_str());
			txt.Format(L"%d", m_curSelDeviceInfoJovision->get_port());
			m_port.SetWindowTextW(txt);
		}
	}
}



