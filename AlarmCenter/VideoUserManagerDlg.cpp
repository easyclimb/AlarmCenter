// VideoUserManagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "VideoUserManagerDlg.h"
#include "afxdialogex.h"
#include "VideoManager.h"
#include "VideoUserInfoEzviz.h"
#include "VideoUserInfoJovision.h"
#include "VideoDeviceInfoEzviz.h"
#include "VideoDeviceInfoJovision.h"
#include "UserInfo.h"
#include "AddVideoUserEzvizDlg.h"
#include "AlarmMachineManager.h"
#include "AlarmMachine.h"
#include "ZoneInfo.h"
#include "ChooseZoneDlg.h"
#include "VideoPlayerDlg.h"
#include "AddVideoUserProgressDlg.h"
#include "ConfigHelper.h"
#include "AddVideoUserJovisionDlg.h"

// CVideoUserManagerDlg dialog

CVideoUserManagerDlg* g_videoUserMgrDlg = nullptr;



class CVideoUserManagerDlg::CurUserChangedObserver : public dp::observer<core::user_info_ptr>
{
public:
	explicit CurUserChangedObserver(CVideoUserManagerDlg* dlg) : _dlg(dlg) {}
	virtual void on_update(const core::user_info_ptr& ptr) {
		if (_dlg) {
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

		}
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
END_MESSAGE_MAP()


// CVideoUserManagerDlg message handlers


BOOL CVideoUserManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//core::user_priority up = core::user_manager::GetInstance()->GetCurUserInfo()->get_user_priority();
	m_cur_user_changed_observer = std::make_shared<CurUserChangedObserver>(this);
	core::user_manager::GetInstance()->register_observer(m_cur_user_changed_observer);
	m_cur_user_changed_observer->on_update(core::user_manager::GetInstance()->GetCurUserInfo());

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
	auto vmgr = video::video_manager::GetInstance();
	auto& productorEzviz = vmgr->ProductorEzviz;
	auto& productorJovision = vmgr->ProductorJovision;

	// set tab's rect
	m_groupUser.GetClientRect(rc);
	m_groupUser.ClientToScreen(rc);
	m_groupUser.ShowWindow(SW_HIDE);
	ScreenToClient(rc);
	
	// add 2 list
	m_tab_users.MoveWindow(rc);
	fm.Format(L"%s[%s]", productorEzviz.get_name().c_str(), productorEzviz.get_description().c_str());
	m_tab_users.InsertItem(0, fm);
	fm.Format(L"%s[%s]", productorJovision.get_name().c_str(), productorJovision.get_description().c_str());
	m_tab_users.InsertItem(1, fm);

	// set user's rect
	rc.DeflateRect(5, 25, 5, 5);
	m_listUserEzviz.MoveWindow(rc);
	m_listUserJovision.MoveWindow(rc);
	m_tab_users.SetCurSel(0);
	m_listUserJovision.ShowWindow(SW_HIDE);

	// ezviz user list
	fm = GetStringFromAppResource(IDS_STRING_ID);
	m_listUserEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 38, -1);
	fm = GetStringFromAppResource(IDS_STRING_NAME);
	m_listUserEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 80, -1);
	fm = GetStringFromAppResource(IDS_STRING_PHONE);
	m_listUserEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 100, -1);
	fm = GetStringFromAppResource(IDS_STRING_ACCESS_TOKEN);
	m_listUserEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 100, -1);
	fm = GetStringFromAppResource(IDS_STRING_TOKEN_TIME);
	m_listUserEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 60, -1);
	fm = GetStringFromAppResource(IDS_STRING_DEVICE_COUNT);
	m_listUserEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 60, -1);

	// jovision user list
	fm = GetStringFromAppResource(IDS_STRING_ID);
	m_listUserJovision.InsertColumn(++i, fm, LVCFMT_LEFT, 38, -1);
	fm = GetStringFromAppResource(IDS_STRING_NAME);
	m_listUserJovision.InsertColumn(++i, fm, LVCFMT_LEFT, 80, -1);
	fm = GetStringFromAppResource(IDS_STRING_PHONE);
	m_listUserJovision.InsertColumn(++i, fm, LVCFMT_LEFT, 100, -1);
	fm = GetStringFromAppResource(IDS_STRING_GLOBAL_USER_NAME);
	m_listUserJovision.InsertColumn(++i, fm, LVCFMT_LEFT, 100, -1);
	fm = GetStringFromAppResource(IDS_STRING_GLOBAL_USER_PASSWD);
	m_listUserJovision.InsertColumn(++i, fm, LVCFMT_LEFT, 60, -1);
	fm = GetStringFromAppResource(IDS_STRING_DEVICE_COUNT);
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
	fm = GetStringFromAppResource(IDS_STRING_ID);
	m_listDeviceEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 38, -1);
	fm = GetStringFromAppResource(IDS_STRING_NOTE);
	m_listDeviceEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 100, -1);
	fm = GetStringFromAppResource(IDS_STRING_CAMERA_ID);
	m_listDeviceEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 80, -1);
	fm = GetStringFromAppResource(IDS_STRING_CAMERA_NAME);
	m_listDeviceEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 80, -1);
	fm = GetStringFromAppResource(IDS_STRING_CAMERA_NO);
	m_listDeviceEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 80, -1);
	fm = GetStringFromAppResource(IDS_STRING_DEFENCE);
	m_listDeviceEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 60, -1);
	fm = GetStringFromAppResource(IDS_STRING_DEVICE_ID);
	m_listDeviceEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 80, -1);
	fm = GetStringFromAppResource(IDS_STRING_DEVICE_NAME);
	m_listDeviceEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 80, -1);
	fm = GetStringFromAppResource(IDS_STRING_DEVICE_SERIAL);
	m_listDeviceEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 80, -1);
	fm = GetStringFromAppResource(IDS_STRING_IS_ENCRYPT);
	m_listDeviceEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 60, -1);
	fm = GetStringFromAppResource(IDS_STRING_IS_SHARED);
	m_listDeviceEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 60, -1);
	fm = GetStringFromAppResource(IDS_STRING_PIC_URL);
	m_listDeviceEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 60, -1);
	fm = GetStringFromAppResource(IDS_STRING_STATUS);
	m_listDeviceEzviz.InsertColumn(++i, fm, LVCFMT_LEFT, 50, -1);
	
	// device list jovision
	i = -1;
	fm = GetStringFromAppResource(IDS_STRING_ID);
	m_listDeviceJovision.InsertColumn(++i, fm, LVCFMT_LEFT, 38, -1);
	fm = GetStringFromAppResource(IDS_STRING_NOTE);
	m_listDeviceJovision.InsertColumn(++i, fm, LVCFMT_LEFT, 60, -1);
	fm = GetStringFromAppResource(IDS_STRING_DEVICE_DOMAIN);
	m_listDeviceJovision.InsertColumn(++i, fm, LVCFMT_LEFT, 100, -1);
	fm = GetStringFromAppResource(IDS_STRING_DEVICE_IP);
	m_listDeviceJovision.InsertColumn(++i, fm, LVCFMT_LEFT, 80, -1);
	fm = GetStringFromAppResource(IDS_STRING_DEVICE_PORT);
	m_listDeviceJovision.InsertColumn(++i, fm, LVCFMT_LEFT, 100, -1);

	m_listDeviceEzviz.ShowWindow(SW_SHOW);
	m_listDeviceJovision.ShowWindow(SW_HIDE);

	InitUserList();

	g_videoUserMgrDlg = this;

	CenterWindow(GetParent());

	//SetTimer(TIMER_ID_CHECK_USER_ACCTOKEN_TIMEOUT, 60 * 1000, nullptr);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CVideoUserManagerDlg::ResetUserListSelectionInfoEzviz()
{
	AUTO_LOG_FUNCTION;
	CString txt;
	txt = GetStringFromAppResource(IDS_STRING_DEVICE_LIST);
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
	txt = GetStringFromAppResource(IDS_STRING_DEVICE_LIST);
	m_groupDevice.SetWindowTextW(txt);
	m_id.SetWindowTextW(L"");
	m_productor.SetWindowTextW(L"");
	m_name.SetWindowTextW(L"");
	m_phone.SetWindowTextW(L"");
	m_curSelDeviceInfoJovision = nullptr;
	m_curSelUserInfoJovision = nullptr;
	m_curselUserListItemJovision = -1;

	ResetDeviceListSelectionInfoJovsion();
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

	CString txt; txt = GetStringFromAppResource(IDS_STRING_BIND_ZONE);
	m_btnBindOrUnbind.SetWindowTextW(txt);
}


void CVideoUserManagerDlg::ResetDeviceListSelectionInfoJovsion()
{
	AUTO_LOG_FUNCTION;
	m_curselDeviceListItemJovision = -1;
	m_curSelDeviceInfoJovision = nullptr;

	m_idDev.SetWindowTextW(L"");
	m_nameDev.SetWindowTextW(L"");
	m_noteDev.SetWindowTextW(L"");
	m_devCode.SetWindowTextW(L"");
	m_zone.SetWindowTextW(L"");

	CString txt; txt = GetStringFromAppResource(IDS_STRING_BIND_ZONE);
	m_btnBindOrUnbind.SetWindowTextW(txt);
}


void CVideoUserManagerDlg::InitUserList()
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	m_listUserEzviz.DeleteAllItems();
	m_listUserJovision.DeleteAllItems();
	m_listDeviceEzviz.DeleteAllItems();
	m_listDeviceJovision.DeleteAllItems();
	ResetUserListSelectionInfoEzviz();

	video::video_manager* mgr = video::video_manager::GetInstance();
	video::video_user_info_list userList;
	mgr->GetVideoUserList(userList);

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
}


void CVideoUserManagerDlg::InsertUserListEzviz(video::ezviz::video_user_info_ezviz_ptr userInfo)
{
	USES_CONVERSION;
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
		// productor
		/*lvitem.iItem = nResult;
		lvitem.iSubItem++;
		tmp.Format(_T("%s[%s]"), userInfo->get_productorInfo().get_name().c_str(),
				   userInfo->get_productorInfo().get_description().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listUserEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();*/

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
		tmp.Format(_T("%s"), A2W(userInfo->get_acc_token().c_str()));
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
	USES_CONVERSION;
	//int nResult = -1;
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

		// productor
		/*lvitem.iSubItem++;
		tmp.Format(_T("%s[%s]"), userInfo->get_productorInfo().get_name().c_str(),
				   userInfo->get_productorInfo().get_description().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listUserEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();*/

		// name
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), userInfo->get_user_name().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listUserEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// phone
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), A2W(userInfo->get_user_phone().c_str()));
		lvitem.pszText = tmp.LockBuffer();
		m_listUserEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// access token
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), A2W(userInfo->get_acc_token().c_str()));
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
	USES_CONVERSION;
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
		// productor
		/*lvitem.iItem = nResult;
		lvitem.iSubItem++;
		tmp.Format(_T("%s[%s]"), userInfo->get_productorInfo().get_name().c_str(),
				   userInfo->get_productorInfo().get_description().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listUserEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();*/

		// name
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), userInfo->get_user_name().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listUserJovision.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// phone
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), A2W(userInfo->get_user_phone().c_str()));
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


void CVideoUserManagerDlg::UpdateDeviceListEzviz(int nItem, video::ezviz::video_device_info_ezviz_ptr deviceInfo)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	int nResult = -1;
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
		tmp.Format(_T("%s"), A2W(deviceInfo->get_cameraId().c_str()));
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
		tmp.Format(_T("%s"), A2W(deviceInfo->get_deviceId().c_str()));
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
		tmp.Format(_T("%s"), A2W(deviceInfo->get_deviceSerial().c_str()));
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
		tmp.Format(_T("%s"), A2W(deviceInfo->get_isShared().c_str()));
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// picUrl
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), A2W(deviceInfo->get_picUrl().c_str()));
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


void CVideoUserManagerDlg::InsertDeviceListEzviz(video::ezviz::video_device_info_ezviz_ptr deviceInfo)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
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
		tmp.Format(_T("%s"), A2W(deviceInfo->get_cameraId().c_str()));
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
		tmp.Format(_T("%s"), A2W(deviceInfo->get_deviceId().c_str()));
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
		tmp.Format(_T("%s"), A2W(deviceInfo->get_deviceSerial().c_str()));
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
		tmp.Format(_T("%s"), A2W(deviceInfo->get_isShared().c_str()));
		lvitem.pszText = tmp.LockBuffer();
		m_listDeviceEzviz.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// picUrl
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), A2W(deviceInfo->get_picUrl().c_str()));
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

}


void CVideoUserManagerDlg::OnLvnItemchangedListUserEzviz(NMHDR * pNMHDR, LRESULT * pResult)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	if (pResult)
		*pResult = 0;

	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if (pNMLV == nullptr || m_listUserEzviz.GetItemCount() == 0) {
		ResetUserListSelectionInfoEzviz();
		return;
	}
	auto user = video::video_manager::GetInstance()->GetVideoUserEzviz(pNMLV->lParam);
	if (m_curSelUserInfoEzviz == user) {
		return;
	}
	m_curSelUserInfoEzviz = user;
	m_curselUserListItemEzviz = pNMLV->iItem;

	ShowUsersDeviceListEzviz(user);
}


void CVideoUserManagerDlg::OnLvnItemchangedListDeviceJovision(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (pResult) {
		*pResult = 0;
	}

	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	if (pNMLV == nullptr || m_listUserJovision.GetItemCount() == 0) {
		ResetUserListSelectionInfoJovision();
		return;
	}
	auto user = video::video_manager::GetInstance()->GetVideoUserJovision(pNMLV->lParam);
	if (m_curSelUserInfoJovision == user) {
		return;
	}
	m_curSelUserInfoJovision = user;
	m_curselUserListItemJovision = pNMLV->iItem;

	ShowUsersDeviceListJovision(user);
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
	auto user = video::video_manager::GetInstance()->GetVideoUserJovision(pNMLV->lParam);
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
	USES_CONVERSION;
	if (pResult)
		*pResult = 0;

	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if (pNMLV == nullptr || m_listDeviceEzviz.GetItemCount() == 0) {
		ResetDeviceListSelectionInfoEzviz();
		return;
	}
	video::ezviz::video_device_info_ezviz_ptr dev = video::video_manager::GetInstance()->GetVideoDeviceInfoEzviz(pNMLV->lParam);
	if (m_curSelDeviceInfoEzviz == dev) {
		return;
	}
	m_curSelDeviceInfoEzviz = dev;
	m_curselDeviceListItemEzviz = pNMLV->iItem;
	ShowDeviceInfoEzviz(dev);
}


void CVideoUserManagerDlg::ShowDeviceInfoEzviz(video::ezviz::video_device_info_ezviz_ptr device)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;

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
	txt.Format(L"%s", A2W(device->get_secure_code().c_str()));
	m_devCode.SetWindowTextW(txt);

	//bool binded = false;
	//video::zone_uuid zone = device->get_zoneUuid();
	std::list<video::zone_uuid> zoneList;
	device->get_zoneUuidList(zoneList);
	/*for (auto zone : zoneList) {
		video::bind_info bi = video::video_manager::GetInstance()->GetBindInfo(zone);
		if (!CheckZoneInfoExsist(zone) || bi._device != device) {
			video::video_manager::GetInstance()->UnbindZoneAndDevice(zone);
			binded = false;
		} else {
			binded = true;
		}
	}*/
	//if (binded) {
	CString temp = L"";
	txt.Empty();
	if (!zoneList.empty()) {
		size_t ndx = 0;
		for (auto zone : zoneList) {
			if (zone._gg == core::INDEX_ZONE) {
				temp.Format(GetStringFromAppResource(IDS_STRING_FM_ADEMCO_ID) + L"[%03d]", zone._ademco_id, zone._zone_value);
			} else {
				temp.Format(GetStringFromAppResource(IDS_STRING_FM_ADEMCO_ID) + L"[%03d][%02d]", zone._ademco_id, zone._zone_value, zone._gg);
			}
			if (ndx != zoneList.size() - 1) {
				temp += L",";
			}
			txt += temp;
		}
		m_zone.SetWindowTextW(txt);
		//txt = GetStringFromAppResource(IDS_STRING_UNBIND_ZONE);
		//m_btnBindOrUnbind.SetWindowTextW(txt);
		//m_chkAutoPlayVideo.SetCheck(bi.auto_play_when_alarm_);
	} else {
		m_zone.SetWindowTextW(L"");
		//txt = GetStringFromAppResource(IDS_STRING_BIND_ZONE);
		//m_btnBindOrUnbind.SetWindowTextW(txt);
		//m_chkAutoPlayVideo.SetCheck(0);
		//m_chkAutoPlayVideo.EnableWindow(0);
	}
}


bool CVideoUserManagerDlg::CheckZoneInfoExsist(const video::zone_uuid& zone)
{
	do {
		core::alarm_machine_manager* mgr = core::alarm_machine_manager::GetInstance();
		core::alarm_machine_ptr machine = mgr->GetMachine(zone._ademco_id);
		if (!machine)
			break;
		core::zone_info_ptr zoneInfo = machine->GetZone(zone._zone_value);
		if (!zoneInfo)
			break;

		core::alarm_machine_ptr subMachine = zoneInfo->GetSubMachineInfo();
		if (zone._gg == core::INDEX_ZONE) {
			if (subMachine != nullptr)
				break;
		} else {
			if (subMachine == nullptr)
				break;
			if (zone._gg == core::INDEX_SUB_MACHINE) 
				break;
			if (subMachine->GetZone(zone._gg) == nullptr)
				break;
		}
		
		return true;
	} while (0);
	return false;
}


void CVideoUserManagerDlg::ShowUsersDeviceListEzviz(video::ezviz::video_user_info_ezviz_ptr user)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;

	if (!user) {
		ResetUserListSelectionInfoEzviz();
		return;
	}

	CString fm, txt;
	fm = GetStringFromAppResource(IDS_STRING_FM_USERS_DEV_LIST);
	txt.Format(fm, user->get_user_name().c_str(), user->get_device_count());
	m_groupDevice.SetWindowTextW(txt);	

	txt.Format(L"%d", user->get_id());
	m_id.SetWindowTextW(txt);
	txt.Format(L"%s[%s]", user->get_productorInfo().get_name().c_str(),
			   user->get_productorInfo().get_description().c_str());
	m_productor.SetWindowTextW(txt);
	txt.Format(L"%s", user->get_user_name().c_str());
	m_name.SetWindowTextW(txt);
	txt.Format(L"%s", A2W(user->get_user_phone().c_str()));
	m_phone.SetWindowTextW(txt);

	m_listDeviceEzviz.DeleteAllItems();

	video::video_device_info_list list;
	user->GetDeviceList(list);
	
	for (auto i : list) {
		video::ezviz::video_device_info_ezviz_ptr device = std::dynamic_pointer_cast<video::ezviz::video_device_info_ezviz>(i);
		InsertDeviceListEzviz(device);
	}
	
}


void CVideoUserManagerDlg::ShowUsersDeviceListJovision(video::jovision::video_user_info_jovision_ptr user)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;

	if (!user) {
		ResetUserListSelectionInfoJovision();
		return;
	}

	CString fm, txt;
	fm = GetStringFromAppResource(IDS_STRING_FM_USERS_DEV_LIST);
	txt.Format(fm, user->get_user_name().c_str(), user->get_device_count());
	m_groupDevice.SetWindowTextW(txt);

	txt.Format(L"%d", user->get_id());
	m_id.SetWindowTextW(txt);
	txt.Format(L"%s[%s]", user->get_productorInfo().get_name().c_str(),
			   user->get_productorInfo().get_description().c_str());
	m_productor.SetWindowTextW(txt);
	txt.Format(L"%s", user->get_user_name().c_str());
	m_name.SetWindowTextW(txt);
	txt.Format(L"%s", A2W(user->get_user_phone().c_str()));
	m_phone.SetWindowTextW(txt);

	m_listDeviceJovision.DeleteAllItems();

	video::video_device_info_list list;
	user->GetDeviceList(list);

	ResetDeviceListSelectionInfoJovsion();
	for (auto i : list) {
		video::jovision::video_device_info_jovision_ptr device = std::dynamic_pointer_cast<video::jovision::video_device_info_jovision>(i);
		InsertDeviceListJovision(device);
	}
}


void CVideoUserManagerDlg::OnBnClickedButtonSaveChange()
{
	AUTO_LOG_FUNCTION;
	if (m_curSelUserInfoEzviz == nullptr || m_curselUserListItemEzviz == -1) { return; }
	CString name; m_name.GetWindowTextW(name);
	if (name.Compare(m_curSelUserInfoEzviz->get_user_name().c_str()) != 0) {
		if (video::EZVIZ == m_curSelUserInfoEzviz->get_productorInfo().get_productor()) {
			video::ezviz::video_user_info_ezviz_ptr user = std::dynamic_pointer_cast<video::ezviz::video_user_info_ezviz>(m_curSelUserInfoEzviz);
			if (user->execute_set_user_name(name.LockBuffer())) {
				UpdateUserListEzviz(m_curselUserListItemEzviz, user);
			}
			name.UnlockBuffer();
		}
	}
}


void CVideoUserManagerDlg::OnBnClickedButtonDelUser()
{
	AUTO_LOG_FUNCTION;
	if (m_curSelUserInfoEzviz == nullptr || m_curselUserListItemEzviz == -1) { return; }
	CString info; info = GetStringFromAppResource(IDS_STRING_CONFIRM_DEL_VIDEO_USER);
	int ret = MessageBox(info, L"", MB_OKCANCEL | MB_ICONWARNING);
	if (ret != IDOK)return;

	if (m_curSelUserInfoEzviz->get_productorInfo().get_productor() == video::EZVIZ) {
		video::ezviz::video_user_info_ezviz_ptr user = std::dynamic_pointer_cast<video::ezviz::video_user_info_ezviz>(m_curSelUserInfoEzviz);
		if (video::video_manager::GetInstance()->DeleteVideoUser(user)) {
			InitUserList();
			OnLvnItemchangedListUserEzviz(nullptr, nullptr);
		}
	} else if(m_curSelUserInfoEzviz->get_productorInfo().get_productor() == video::JOVISION) {
		//video::jovision::video_user_info_jovision_ptr user = reinterpret_cast<video::jovision::video_user_info_jovision_ptr>(m_curSelDeviceInfoEzviz);
		// TODO 2015��9��11��20:50:41 video::JOVISION
		/*if (video::video_manager::GetInstance()->DeleteVideoUser(user)) {

		}*/
	}
}


void CVideoUserManagerDlg::OnBnClickedButtonAddUser()
{
	AUTO_LOG_FUNCTION;
	auto vmgr = video::video_manager::GetInstance();
	/*CMenu menu;
	if (!menu.CreatePopupMenu())return;
	
	auto vmgr = video::video_manager::GetInstance();
	auto& productorEzviz = vmgr->ProductorEzviz;
	auto& productorJovision = vmgr->ProductorJovision;

	CString txt;
	txt.Format(L"%s[%s]", productorEzviz.get_name().c_str(), productorEzviz.get_description().c_str());
	menu.AppendMenuW(MF_STRING, 1, txt);
	txt.Format(L"%s[%s]", productorJovision.get_name().c_str(), productorJovision.get_description().c_str());
	menu.AppendMenuW(MF_STRING, 2, txt);

	CRect rc;
	m_btnAddUser.GetWindowRect(rc);
	DWORD ret = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
									rc.right, rc.top, this);*/

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
			e = GetStringFromAppResource(IDS_STRING_PRIVATE_CLOUD_CONN_FAIL_OR_USER_NOT_EXSIST);
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
			e = GetStringFromAppResource(IDS_STRING_PRIVATE_CLOUD_CONN_FAIL_OR_USER_NOT_EXSIST);
			MessageBox(e, L"", MB_ICONERROR);
		} else if (result == video::video_manager::RESULT_USER_ALREADY_EXSIST) {

		} else { assert(0); }
	}
}


void CVideoUserManagerDlg::OnBnClickedButtonRefreshDeviceList()
{
	AUTO_LOG_FUNCTION;
	if (m_curSelUserInfoEzviz == nullptr || m_curselUserListItemEzviz == -1) { return; }
	if (m_curSelUserInfoEzviz->get_productorInfo().get_productor() == video::EZVIZ) {
		video::ezviz::video_user_info_ezviz_ptr user = std::dynamic_pointer_cast<video::ezviz::video_user_info_ezviz>(m_curSelUserInfoEzviz);
		video::video_manager* mgr = video::video_manager::GetInstance();
		video::video_manager::VideoEzvizResult result = mgr->RefreshUserEzvizDeviceList(user);
		if (result == video::video_manager::RESULT_OK) {
			ShowUsersDeviceListEzviz(user);
		} else if (result == video::video_manager::RESULT_PRIVATE_CLOUD_CONNECT_FAILED_OR_USER_NOT_EXIST) {
			CString e; e = GetStringFromAppResource(IDS_STRING_PRIVATE_CLOUD_CONN_FAIL_OR_USER_NOT_EXSIST);
			MessageBox(e, L"", MB_ICONERROR);
		}
	}
}
	

void CVideoUserManagerDlg::OnBnClickedButtonBindOrUnbind()
{
	AUTO_LOG_FUNCTION;
	if (m_curSelDeviceInfoEzviz == nullptr || m_curselDeviceListItemEzviz == -1) { return; }
	if (m_curSelDeviceInfoEzviz->get_userInfo()->get_productorInfo().get_productor() == video::EZVIZ) {
		video::ezviz::video_device_info_ezviz_ptr dev = std::dynamic_pointer_cast<video::ezviz::video_device_info_ezviz>(m_curSelDeviceInfoEzviz);
		video::video_manager* mgr = video::video_manager::GetInstance();
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
	}
}


void CVideoUserManagerDlg::OnBnClickedCheckAutoPlayVideo()
{
	/*AUTO_LOG_FUNCTION;
	if (m_curSelDeviceInfoEzviz == nullptr || m_curselDeviceListItemEzviz == -1) { return; }
	if (m_curSelDeviceInfoEzviz->get_userInfo()->get_productorInfo().get_productor() == video::EZVIZ) {
		video::ezviz::video_device_info_ezviz_ptr dev = reinterpret_cast<video::ezviz::video_device_info_ezviz_ptr>(m_curSelDeviceInfoEzviz);
		video::video_manager* mgr = video::video_manager::GetInstance();
		bool checked = m_chkAutoPlayVideo.GetCheck() > 0 ? true : false;
		if (checked == dev->get_binded()) return;
		if (mgr->SetBindInfoAutoPlayVideoOnAlarm(dev->get_zoneUuid(), checked)) {
			ShowDeviceInfo(dev);
		}
	}*/
}


void CVideoUserManagerDlg::OnBnClickedButtonSaveDev()
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	if (m_curSelDeviceInfoEzviz == nullptr || m_curselDeviceListItemEzviz == -1) { return; }
	if (m_curSelDeviceInfoEzviz->get_userInfo()->get_productorInfo().get_productor() == video::EZVIZ) {
		video::ezviz::video_device_info_ezviz_ptr dev = std::dynamic_pointer_cast<video::ezviz::video_device_info_ezviz>(m_curSelDeviceInfoEzviz);
		//video::video_manager* mgr = video::video_manager::GetInstance();
		do {
			CString note, code;
			m_noteDev.GetWindowTextW(note);
			m_devCode.GetWindowTextW(code);
			if (!code.IsEmpty()) {
				if (!video::ezviz::video_device_info_ezviz::IsValidVerifyCode(W2A(code))) {
					note = GetStringFromAppResource(IDS_STRING_DEVICE_CODE_INVALID);
					MessageBox(note, L"", MB_ICONERROR);
					break;
				}
			}
			bool changed = false;
			if (note.Compare(dev->get_device_note().c_str()) != 0) {
				changed = true;
				dev->set_device_note(note.LockBuffer());
				note.UnlockBuffer();
			}
			if (code.Compare(A2W(dev->get_secure_code().c_str())) != 0) {
				changed = true;
				dev->set_secure_code(W2A(code));
			}

			if (changed) {
				dev->execute_update_info();
			}
		} while (0);
		UpdateDeviceListEzviz(m_curselDeviceListItemEzviz, dev);
		ShowDeviceInfoEzviz(dev);
	}
}


void CVideoUserManagerDlg::OnBnClickedButtonPlay()
{
	if (m_curSelDeviceInfoEzviz == nullptr || m_curselDeviceListItemEzviz == -1) { return; }
	if (m_curSelDeviceInfoEzviz->get_userInfo()->get_productorInfo().get_productor() == video::EZVIZ) {
		//video::ezviz::video_device_info_ezviz_ptr dev = reinterpret_cast<video::ezviz::video_device_info_ezviz_ptr>(m_curSelDeviceInfoEzviz);
		g_videoPlayerDlg->PlayVideoByDevice(m_curSelDeviceInfoEzviz, util::CConfigHelper::GetInstance()->get_default_video_level());
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
		//video::video_manager::GetInstance()->CheckUserAcctkenTimeout();
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
	if (m_curSelDeviceInfoEzviz == nullptr || m_curselDeviceListItemEzviz == -1) { return; }
	if (m_curSelDeviceInfoEzviz->get_userInfo()->get_productorInfo().get_productor() == video::EZVIZ) {
		video::ezviz::video_device_info_ezviz_ptr dev = std::dynamic_pointer_cast<video::ezviz::video_device_info_ezviz>(m_curSelDeviceInfoEzviz);
		video::video_manager* mgr = video::video_manager::GetInstance();
		std::list<video::zone_uuid> zoneList;
		dev->get_zoneUuidList(zoneList);
		for(auto zone : zoneList) {
			mgr->UnbindZoneAndDevice(zone);
		} 
		ShowDeviceInfoEzviz(dev);
		if (m_observerDlg) {
			m_observerDlg->PostMessageW(WM_VIDEO_INFO_CHANGE);
		}
	}
}


afx_msg LRESULT CVideoUserManagerDlg::OnVideoInfoChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (m_curSelDeviceInfoEzviz == nullptr || m_curselDeviceListItemEzviz == -1) { return 0; }
	if (m_curSelDeviceInfoEzviz->get_userInfo()->get_productorInfo().get_productor() == video::EZVIZ) {
		video::ezviz::video_device_info_ezviz_ptr dev = std::dynamic_pointer_cast<video::ezviz::video_device_info_ezviz>(m_curSelDeviceInfoEzviz);
		ShowDeviceInfoEzviz(dev);
	}
	return 0;
}


void CVideoUserManagerDlg::OnBnClickedButtonDelDevice()
{
	AUTO_LOG_FUNCTION;
	if (m_curSelDeviceInfoEzviz == nullptr || m_curselDeviceListItemEzviz == -1) { return; }
	if (m_curSelDeviceInfoEzviz->get_userInfo()->get_productorInfo().get_productor() == video::EZVIZ) {
		video::ezviz::video_device_info_ezviz_ptr dev = std::dynamic_pointer_cast<video::ezviz::video_device_info_ezviz>(m_curSelDeviceInfoEzviz);
		auto user = std::dynamic_pointer_cast<video::ezviz::video_user_info_ezviz>(dev->get_userInfo());
		user->DeleteVideoDevice(dev);
		m_curSelDeviceInfoEzviz = nullptr;
		m_listDeviceEzviz.DeleteItem(m_curselDeviceListItemEzviz);
		m_listDeviceEzviz.SetItemState(m_curselDeviceListItemEzviz, LVNI_FOCUSED | LVIS_SELECTED, LVNI_FOCUSED | LVIS_SELECTED);
	}
}


void CVideoUserManagerDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	if (bShow) {
		if (!m_cur_user_changed_observer) {
			m_cur_user_changed_observer = std::make_shared<CurUserChangedObserver>(this);
			core::user_manager::GetInstance()->register_observer(m_cur_user_changed_observer);
		}
		m_cur_user_changed_observer->on_update(core::user_manager::GetInstance()->GetCurUserInfo());
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
	video::ezviz::video_device_info_ezviz_ptr dev = video::video_manager::GetInstance()->GetVideoDeviceInfoEzviz(data);
	if (!dev) {
		return;
	}

	m_curSelDeviceInfoEzviz = dev;
	m_curselDeviceListItemEzviz = pNMLV->iItem;

	if (g_videoPlayerDlg)
		g_videoPlayerDlg->PlayVideoByDevice(dev, util::CConfigHelper::GetInstance()->get_default_video_level());
}


void CVideoUserManagerDlg::OnTcnSelchangeTabUsers(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	*pResult = 0;

	int ndx = m_tab_users.GetCurSel(); if (ndx < 0)return;
	m_listUserEzviz.ShowWindow(ndx == 0 ? SW_SHOW : SW_HIDE);
	m_btnRefreshDeviceList.ShowWindow(ndx == 0 ? SW_SHOW : SW_HIDE);
	m_listDeviceEzviz.ShowWindow(ndx == 0 ? SW_SHOW : SW_HIDE);

	m_listUserJovision.ShowWindow(ndx == 1 ? SW_SHOW : SW_HIDE);
	m_listDeviceJovision.ShowWindow(ndx == 1 ? SW_SHOW : SW_HIDE);
	m_btnAddDevice.ShowWindow(ndx == 1 ? SW_SHOW : SW_HIDE);

	//OnLvnItemchangedListUserEzviz(nullptr, nullptr);

	if (ndx == 0) {
		ShowUsersDeviceListEzviz(m_curSelUserInfoEzviz);
	} else if (ndx == 1) {
		ShowUsersDeviceListJovision(m_curSelUserInfoJovision);
	}
}






