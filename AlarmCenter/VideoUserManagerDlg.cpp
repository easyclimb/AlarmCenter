// VideoUserManagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "VideoUserManagerDlg.h"
#include "afxdialogex.h"
#include "VideoInfo.h"
#include "VideoUserInfoEzviz.h"
#include "VideoUserInfoNormal.h"
#include "VideoDeviceInfoEzviz.h"
#include "VideoDeviceInfoNormal.h"
#include "UserInfo.h"

// CVideoUserManagerDlg dialog

IMPLEMENT_DYNAMIC(CVideoUserManagerDlg, CDialogEx)

CVideoUserManagerDlg::CVideoUserManagerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CVideoUserManagerDlg::IDD, pParent)
	, m_curSelUserInfo(NULL)
	, m_curSelDeviceInfo(NULL)
	, m_privilege(core::UP_OPERATOR)
	, m_curSelUserListItem(-1)
{

}

CVideoUserManagerDlg::~CVideoUserManagerDlg()
{
}

void CVideoUserManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_USER, m_listUser);
	DDX_Control(pDX, IDC_LIST_DEVICE, m_listDevice);
	DDX_Control(pDX, IDC_STATIC_DEVICE_LIST, m_groupDevice);
	DDX_Control(pDX, IDC_LIST_DEVICE2, m_listDevice2);
	DDX_Control(pDX, IDC_EDIT_ID, m_id);
	DDX_Control(pDX, IDC_EDIT_PRODUCTOR, m_productor);
	DDX_Control(pDX, IDC_EDIT_NAME, m_name);
	DDX_Control(pDX, IDC_EDIT_PHONE, m_phone);
	DDX_Control(pDX, IDC_BUTTON_DEL_USER, m_btnDelUser);
	DDX_Control(pDX, IDC_BUTTON_SAVE_CHANGE, m_btnUpdateUser);
	DDX_Control(pDX, IDC_BUTTON_ADD_USER, m_btnAddUser);
}


BEGIN_MESSAGE_MAP(CVideoUserManagerDlg, CDialogEx)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_USER, &CVideoUserManagerDlg::OnLvnItemchangedListUser)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_CHANGE, &CVideoUserManagerDlg::OnBnClickedButtonSaveChange)
	ON_BN_CLICKED(IDC_BUTTON_DEL_USER, &CVideoUserManagerDlg::OnBnClickedButtonDelUser)
	ON_BN_CLICKED(IDC_BUTTON_ADD_USER, &CVideoUserManagerDlg::OnBnClickedButtonAddUser)
END_MESSAGE_MAP()


// CVideoUserManagerDlg message handlers


BOOL CVideoUserManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	core::UserPriority up = core::CUserManager::GetInstance()->GetCurUserInfo()->get_user_priority();
	m_privilege = up;

	DWORD dwStyle = m_listUser.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
	m_listUser.SetExtendedStyle(dwStyle);
	int i = -1;
	CString fm;
	fm.LoadStringW(IDS_STRING_ID);
	m_listUser.InsertColumn(++i, fm, LVCFMT_LEFT, 38, -1);
	fm.LoadStringW(IDS_STRING_PRODUCTOR);
	m_listUser.InsertColumn(++i, fm, LVCFMT_LEFT, 100, -1);
	fm.LoadStringW(IDS_STRING_NAME);
	m_listUser.InsertColumn(++i, fm, LVCFMT_LEFT, 80, -1);
	fm.LoadStringW(IDS_STRING_PHONE);
	m_listUser.InsertColumn(++i, fm, LVCFMT_LEFT, 100, -1);
	//fm.LoadStringW(IDS_STRING_ACCT);
	//m_listUser.InsertColumn(++i, fm, LVCFMT_LEFT, 100, -1);
	fm.LoadStringW(IDS_STRING_DEVICE_COUNT);
	m_listUser.InsertColumn(++i, fm, LVCFMT_LEFT, 60, -1);
	
	// device list ezviz
	dwStyle = m_listDevice.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
	m_listDevice.SetExtendedStyle(dwStyle);
	i = -1;
	fm.LoadStringW(IDS_STRING_ID);
	m_listDevice.InsertColumn(++i, fm, LVCFMT_LEFT, 38, -1);
	fm.LoadStringW(IDS_STRING_NOTE);
	m_listDevice.InsertColumn(++i, fm, LVCFMT_LEFT, 100, -1);
	fm.LoadStringW(IDS_STRING_CAMERA_ID);
	m_listDevice.InsertColumn(++i, fm, LVCFMT_LEFT, 80, -1);
	fm.LoadStringW(IDS_STRING_CAMERA_NAME);
	m_listDevice.InsertColumn(++i, fm, LVCFMT_LEFT, 80, -1);
	fm.LoadStringW(IDS_STRING_CAMERA_NO);
	m_listDevice.InsertColumn(++i, fm, LVCFMT_LEFT, 80, -1);
	fm.LoadStringW(IDS_STRING_DEFENCE);
	m_listDevice.InsertColumn(++i, fm, LVCFMT_LEFT, 60, -1);
	fm.LoadStringW(IDS_STRING_DEVICE_ID);
	m_listDevice.InsertColumn(++i, fm, LVCFMT_LEFT, 80, -1);
	fm.LoadStringW(IDS_STRING_DEVICE_NAME);
	m_listDevice.InsertColumn(++i, fm, LVCFMT_LEFT, 80, -1);
	fm.LoadStringW(IDS_STRING_DEVICE_SERIAL);
	m_listDevice.InsertColumn(++i, fm, LVCFMT_LEFT, 80, -1);
	fm.LoadStringW(IDS_STRING_IS_ENCRYPT);
	m_listDevice.InsertColumn(++i, fm, LVCFMT_LEFT, 60, -1);
	fm.LoadStringW(IDS_STRING_IS_SHARED);
	m_listDevice.InsertColumn(++i, fm, LVCFMT_LEFT, 60, -1);
	fm.LoadStringW(IDS_STRING_PIC_URL);
	m_listDevice.InsertColumn(++i, fm, LVCFMT_LEFT, 60, -1);
	fm.LoadStringW(IDS_STRING_STATUS);
	m_listDevice.InsertColumn(++i, fm, LVCFMT_LEFT, 50, -1);
	
	// device list normal
	dwStyle = m_listDevice2.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
	m_listDevice2.SetExtendedStyle(dwStyle);
	i = -1;
	fm.LoadStringW(IDS_STRING_ID);
	m_listDevice2.InsertColumn(++i, fm, LVCFMT_LEFT, 38, -1);
	fm.LoadStringW(IDS_STRING_NOTE);
	m_listDevice2.InsertColumn(++i, fm, LVCFMT_LEFT, 60, -1);
	fm.LoadStringW(IDS_STRING_DEVICE_DOMAIN);
	m_listDevice2.InsertColumn(++i, fm, LVCFMT_LEFT, 100, -1);
	fm.LoadStringW(IDS_STRING_DEVICE_IP);
	m_listDevice2.InsertColumn(++i, fm, LVCFMT_LEFT, 80, -1);
	fm.LoadStringW(IDS_STRING_DEVICE_PORT);
	m_listDevice2.InsertColumn(++i, fm, LVCFMT_LEFT, 100, -1);

	m_listDevice.ShowWindow(SW_SHOW);
	m_listDevice2.ShowWindow(SW_HIDE);

	InitUserList();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CVideoUserManagerDlg::InitUserList()
{
	USES_CONVERSION;
	m_listUser.DeleteAllItems();
	m_listDevice.DeleteAllItems();
	m_listDevice2.DeleteAllItems();
	core::video::CVideoManager* mgr = core::video::CVideoManager::GetInstance();
	core::video::CVideoUserInfoList userList;
	mgr->GetVideoUserList(userList);
	core::video::CVideoUserInfoListIter userIter = userList.begin();
	while (userIter != userList.end()) {
		core::video::CVideoUserInfo* userInfo = *userIter++;
		const core::video::CProductorInfo produtor = userInfo->get_productorInfo();
		if (produtor.get_productor() == core::video::EZVIZ) {
			core::video::ezviz::CVideoUserInfoEzviz* ezvizUserInfo = reinterpret_cast<core::video::ezviz::CVideoUserInfoEzviz*>(userInfo);
			InsertUserList(ezvizUserInfo);
		} else if (produtor.get_productor() == core::video::NORMAL) {
			core::video::normal::CVideoUserInfoNormal* normalUserInfo = reinterpret_cast<core::video::normal::CVideoUserInfoNormal*>(userInfo);
			InsertUserList(normalUserInfo);
		}
	}
}


void CVideoUserManagerDlg::InsertUserList(core::video::ezviz::CVideoUserInfoEzviz* userInfo)
{
	USES_CONVERSION;
	int nResult = -1;
	LV_ITEM lvitem = { 0 };
	CString tmp = _T("");

	lvitem.lParam = 0;
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = m_listUser.GetItemCount();
	lvitem.iSubItem = 0;

	// ndx
	tmp.Format(_T("%d"), userInfo->get_id());
	lvitem.pszText = tmp.LockBuffer();
	nResult = m_listUser.InsertItem(&lvitem);
	tmp.UnlockBuffer();

	if (nResult != -1) {
		// productor
		lvitem.iItem = nResult;
		lvitem.iSubItem++;
		tmp.Format(_T("%s[%s]"), userInfo->get_productorInfo().get_name().c_str(),
				   userInfo->get_productorInfo().get_description().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listUser.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// name
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), userInfo->get_user_name().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listUser.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// phone
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), A2W(userInfo->get_user_phone().c_str()));
		lvitem.pszText = tmp.LockBuffer();
		m_listUser.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// device count
		lvitem.iSubItem += 1;
		tmp.Format(_T("%d"), userInfo->get_device_count());
		lvitem.pszText = tmp.LockBuffer();
		m_listUser.SetItem(&lvitem);
		tmp.UnlockBuffer();

		m_listUser.SetItemData(nResult, reinterpret_cast<DWORD_PTR>(userInfo));
	}

}


void CVideoUserManagerDlg::UpdateUserList(int nItem, core::video::ezviz::CVideoUserInfoEzviz* userInfo)
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
		m_listUser.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// productor
		lvitem.iSubItem++;
		tmp.Format(_T("%s[%s]"), userInfo->get_productorInfo().get_name().c_str(),
				   userInfo->get_productorInfo().get_description().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listUser.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// name
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), userInfo->get_user_name().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listUser.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// phone
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), A2W(userInfo->get_user_phone().c_str()));
		lvitem.pszText = tmp.LockBuffer();
		m_listUser.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// device count
		lvitem.iSubItem += 1;
		tmp.Format(_T("%d"), userInfo->get_device_count());
		lvitem.pszText = tmp.LockBuffer();
		m_listUser.SetItem(&lvitem);
		tmp.UnlockBuffer();

		m_listUser.SetItemData(nItem, reinterpret_cast<DWORD_PTR>(userInfo));
	}
}


void CVideoUserManagerDlg::InsertUserList(core::video::normal::CVideoUserInfoNormal* userInfo)
{
	USES_CONVERSION;
	int nResult = -1;
	LV_ITEM lvitem = { 0 };
	CString tmp = _T("");

	lvitem.lParam = 0;
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = m_listUser.GetItemCount();
	lvitem.iSubItem = 0;

	// ndx
	tmp.Format(_T("%d"), userInfo->get_id());
	lvitem.pszText = tmp.LockBuffer();
	nResult = m_listUser.InsertItem(&lvitem);
	tmp.UnlockBuffer();

	if (nResult != -1) {
		// productor
		lvitem.iItem = nResult;
		lvitem.iSubItem++;
		tmp.Format(_T("%s[%s]"), userInfo->get_productorInfo().get_name().c_str(),
				   userInfo->get_productorInfo().get_description().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listUser.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// name
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), userInfo->get_user_name().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listUser.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// acct
		lvitem.iSubItem += 2;
		tmp.Format(_T("%s"), A2W(userInfo->get_user_acct().c_str()));
		lvitem.pszText = tmp.LockBuffer();
		m_listUser.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// device count
		lvitem.iSubItem++;
		tmp.Format(_T("%d"), userInfo->get_device_count());
		lvitem.pszText = tmp.LockBuffer();
		m_listUser.SetItem(&lvitem);
		tmp.UnlockBuffer();

		m_listUser.SetItemData(nResult, reinterpret_cast<DWORD_PTR>(userInfo));
	}
}


void CVideoUserManagerDlg::InsertDeviceList(core::video::ezviz::CVideoDeviceInfoEzviz* deviceInfo)
{
	USES_CONVERSION;
	int nResult = -1;
	LV_ITEM lvitem = { 0 };
	CString tmp = _T("");

	lvitem.lParam = 0;
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = m_listDevice.GetItemCount();
	lvitem.iSubItem = 0;

	// ndx
	tmp.Format(_T("%d"), deviceInfo->get_id());
	lvitem.pszText = tmp.LockBuffer();
	nResult = m_listDevice.InsertItem(&lvitem);
	tmp.UnlockBuffer();

	if (nResult != -1) {
		// note
		lvitem.iItem = nResult;
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), deviceInfo->get_device_note().c_str());
		lvitem.pszText = tmp.LockBuffer();
		m_listDevice.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// cameraId
		lvitem.iItem = nResult;
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), A2W(deviceInfo->get_cameraId().c_str()));
		lvitem.pszText = tmp.LockBuffer();
		m_listDevice.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// cameraName
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), A2W(deviceInfo->get_cameraName().c_str()));
		lvitem.pszText = tmp.LockBuffer();
		m_listDevice.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// cameraNo
		lvitem.iSubItem++;
		tmp.Format(_T("%d"), deviceInfo->get_cameraNo());
		lvitem.pszText = tmp.LockBuffer();
		m_listDevice.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// defence
		lvitem.iSubItem++;
		tmp.Format(_T("%d"), deviceInfo->get_defence());
		lvitem.pszText = tmp.LockBuffer();
		m_listDevice.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// deviceID
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), A2W(deviceInfo->get_deviceId().c_str()));
		lvitem.pszText = tmp.LockBuffer();
		m_listDevice.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// deviceName
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), A2W(deviceInfo->get_deviceName().c_str()));
		lvitem.pszText = tmp.LockBuffer();
		m_listDevice.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// deviceSerial
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), A2W(deviceInfo->get_deviceSerial().c_str()));
		lvitem.pszText = tmp.LockBuffer();
		m_listDevice.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// isEncrypt
		lvitem.iSubItem++;
		tmp.Format(_T("%d"), deviceInfo->get_isEncrypt());
		lvitem.pszText = tmp.LockBuffer();
		m_listDevice.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// isShared
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), A2W(deviceInfo->get_isShared().c_str()));
		lvitem.pszText = tmp.LockBuffer();
		m_listDevice.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// picUrl
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), A2W(deviceInfo->get_picUrl().c_str()));
		lvitem.pszText = tmp.LockBuffer();
		m_listDevice.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// status
		lvitem.iSubItem++;
		tmp.Format(_T("%d"), deviceInfo->get_status());
		lvitem.pszText = tmp.LockBuffer();
		m_listDevice.SetItem(&lvitem);
		tmp.UnlockBuffer();

		m_listDevice.SetItemData(nResult, reinterpret_cast<DWORD_PTR>(deviceInfo));
	}
}


void CVideoUserManagerDlg::InsertDeviceList(core::video::normal::CVideoDeviceInfoNormal* deviceInfo)
{

}


void CVideoUserManagerDlg::OnLvnItemchangedListUser(NMHDR * pNMHDR, LRESULT * pResult)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	*pResult = 0;
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	core::video::CVideoUserInfo* user = reinterpret_cast<core::video::CVideoUserInfo*>(pNMLV->lParam);
	if (m_curSelUserInfo == user) {
		return;
	}
	m_curSelUserInfo = user;
	m_curSelUserListItem = pNMLV->iItem;

	CString txt, fm;
	if (!user) {
		txt.LoadStringW(IDS_STRING_DEVICE_LIST);
		m_groupDevice.SetWindowTextW(txt);
		m_btnDelUser.EnableWindow(0);
		m_btnUpdateUser.EnableWindow(0);
		m_curSelUserListItem = -1;
		return;
	}

	if (m_privilege == core::UP_OPERATOR) {
		m_btnAddUser.EnableWindow(0);
		m_btnDelUser.EnableWindow(0);
		m_btnUpdateUser.EnableWindow(0);
	} else {
		m_btnAddUser.EnableWindow();
		m_btnDelUser.EnableWindow();
		m_btnUpdateUser.EnableWindow();
	}

	fm.LoadStringW(IDS_STRING_FM_USERS_DEV_LIST);
	txt.Format(fm, user->get_user_name().c_str(), user->get_device_count());
	m_groupDevice.SetWindowTextW(txt);
	core::video::CVideoDeviceInfoList list;
	
	if (user->get_productorInfo().get_productor() == core::video::EZVIZ) {
		m_listDevice.DeleteAllItems();
		m_listDevice2.DeleteAllItems();
		m_listDevice.ShowWindow(SW_SHOW);
		m_listDevice2.ShowWindow(SW_HIDE);
		core::video::ezviz::CVideoUserInfoEzviz* userEzviz = reinterpret_cast<core::video::ezviz::CVideoUserInfoEzviz*>(user);
		CString txt;
		txt.Format(L"%d", userEzviz->get_id());
		m_id.SetWindowTextW(txt);
		txt.Format(L"%s[%s]", userEzviz->get_productorInfo().get_name().c_str(),
				   userEzviz->get_productorInfo().get_description().c_str());
		m_productor.SetWindowTextW(txt);
		txt.Format(L"%s", userEzviz->get_user_name().c_str());
		m_name.SetWindowTextW(txt);
		txt.Format(L"%s", A2W(userEzviz->get_user_phone().c_str()));
		m_phone.SetWindowTextW(txt);
		

		userEzviz->GetDeviceList(list);
		core::video::CVideoDeviceInfoListIter iter = list.begin();
		while (iter != list.end()) {
			core::video::ezviz::CVideoDeviceInfoEzviz* device = reinterpret_cast<core::video::ezviz::CVideoDeviceInfoEzviz*>(*iter++);
			InsertDeviceList(device);
		}
	} else if (user->get_productorInfo().get_productor() == core::video::NORMAL) {
		m_listDevice.DeleteAllItems();
		m_listDevice2.DeleteAllItems(); 
		m_listDevice.ShowWindow(SW_HIDE);
		m_listDevice2.ShowWindow(SW_SHOW);
		core::video::normal::CVideoUserInfoNormal* uesrNormal = reinterpret_cast<core::video::normal::CVideoUserInfoNormal*>(user);
		uesrNormal->GetDeviceList(list);
		core::video::CVideoDeviceInfoListIter iter = list.begin();
		while (iter != list.end()) {
			core::video::normal::CVideoDeviceInfoNormal* device = reinterpret_cast<core::video::normal::CVideoDeviceInfoNormal*>(*iter++);
			InsertDeviceList(device);
		}
	} else {
		ASSERT(0); return;
	}

}


void CVideoUserManagerDlg::OnBnClickedButtonSaveChange()
{
	if (m_curSelUserInfo == NULL || m_curSelUserListItem == -1) { return; }
	CString name; m_name.GetWindowTextW(name);
	if (name.Compare(m_curSelUserInfo->get_user_name().c_str()) != 0) {
		if (core::video::EZVIZ == m_curSelUserInfo->get_productorInfo().get_productor()) {
			core::video::ezviz::CVideoUserInfoEzviz* user = reinterpret_cast<core::video::ezviz::CVideoUserInfoEzviz*>(m_curSelUserInfo);
			if (user->execute_set_user_name(name.LockBuffer())) {
				UpdateUserList(m_curSelUserListItem, user);
			}
			name.UnlockBuffer();
		}
	}
}


void CVideoUserManagerDlg::OnBnClickedButtonDelUser()
{

}


void CVideoUserManagerDlg::OnBnClickedButtonAddUser()
{

}
