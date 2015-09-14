// VideoUserManagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "VideoUserManagerDlg.h"
#include "afxdialogex.h"
#include "VideoManager.h"
#include "VideoUserInfoEzviz.h"
#include "VideoUserInfoNormal.h"
#include "VideoDeviceInfoEzviz.h"
#include "VideoDeviceInfoNormal.h"
#include "UserInfo.h"
#include "AddVideoUserEzvizDlg.h"
#include "AlarmMachineManager.h"
#include "AlarmMachine.h"
#include "ZoneInfo.h"

// CVideoUserManagerDlg dialog

IMPLEMENT_DYNAMIC(CVideoUserManagerDlg, CDialogEx)

CVideoUserManagerDlg::CVideoUserManagerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CVideoUserManagerDlg::IDD, pParent)
	, m_curSelUserInfo(NULL)
	, m_curSelDeviceInfo(NULL)
	, m_privilege(core::UP_OPERATOR)
	, m_curselUserListItem(-1)
	, m_curselDeviceListItem(-1)
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
}


BEGIN_MESSAGE_MAP(CVideoUserManagerDlg, CDialogEx)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_USER, &CVideoUserManagerDlg::OnLvnItemchangedListUser)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_CHANGE, &CVideoUserManagerDlg::OnBnClickedButtonSaveChange)
	ON_BN_CLICKED(IDC_BUTTON_DEL_USER, &CVideoUserManagerDlg::OnBnClickedButtonDelUser)
	ON_BN_CLICKED(IDC_BUTTON_ADD_USER, &CVideoUserManagerDlg::OnBnClickedButtonAddUser)
	ON_BN_CLICKED(IDC_BUTTON_REFRESH_DEVICE_LIST, &CVideoUserManagerDlg::OnBnClickedButtonRefreshDeviceList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_DEVICE, &CVideoUserManagerDlg::OnLvnItemchangedListDevice)
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


void CVideoUserManagerDlg::ResetUserListSelectionInfo()
{
	AUTO_LOG_FUNCTION;
	CString txt;
	txt.LoadStringW(IDS_STRING_DEVICE_LIST);
	m_groupDevice.SetWindowTextW(txt);
	m_id.SetWindowTextW(L"");
	m_productor.SetWindowTextW(L"");
	m_name.SetWindowTextW(L"");
	m_phone.SetWindowTextW(L"");
	m_btnDelUser.EnableWindow(0);
	m_btnUpdateUser.EnableWindow(0);
	m_btnRefreshDeviceList.EnableWindow(0);
	m_curSelDeviceInfo = NULL;
	m_curSelUserInfo = NULL;
	m_curselUserListItem = -1;

	ResetDeviceListSelectionInfo();
}


void CVideoUserManagerDlg::ResetDeviceListSelectionInfo()
{
	AUTO_LOG_FUNCTION;
	m_curselDeviceListItem = -1;
	m_curSelDeviceInfo = NULL;

	m_idDev.SetWindowTextW(L"");
	m_nameDev.SetWindowTextW(L"");
	m_noteDev.SetWindowTextW(L"");
	m_devCode.SetWindowTextW(L"");
	m_zone.SetWindowTextW(L"");

	CString txt; txt.LoadStringW(IDS_STRING_BIND_ZONE);
	m_btnBindOrUnbind.SetWindowTextW(txt);
	m_btnBindOrUnbind.EnableWindow(0);
	m_chkAutoPlayVideo.SetCheck(0);
	m_chkAutoPlayVideo.EnableWindow(0);
	m_btnAddDevice.EnableWindow(0);
	m_btnDelDevice.EnableWindow(0);
	m_btnSaveDevChange.EnableWindow(0);
	m_btnRefreshDev.EnableWindow(0);
}


void CVideoUserManagerDlg::InitUserList()
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	m_listUser.DeleteAllItems();
	m_listDevice.DeleteAllItems();
	m_listDevice2.DeleteAllItems();
	ResetUserListSelectionInfo();

	video::CVideoManager* mgr = video::CVideoManager::GetInstance();
	video::CVideoUserInfoList userList;
	mgr->GetVideoUserList(userList);

	for (auto& userInfo : userList) {
		const video::CProductorInfo produtor = userInfo->get_productorInfo();
		if (produtor.get_productor() == video::EZVIZ) {
			video::ezviz::CVideoUserInfoEzviz* ezvizUserInfo = reinterpret_cast<video::ezviz::CVideoUserInfoEzviz*>(userInfo);
			InsertUserList(ezvizUserInfo);
		} else if (produtor.get_productor() == video::NORMAL) {
			video::normal::CVideoUserInfoNormal* normalUserInfo = reinterpret_cast<video::normal::CVideoUserInfoNormal*>(userInfo);
			InsertUserList(normalUserInfo);
		}
	}
}


void CVideoUserManagerDlg::InsertUserList(video::ezviz::CVideoUserInfoEzviz* userInfo)
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


void CVideoUserManagerDlg::UpdateUserList(int nItem, video::ezviz::CVideoUserInfoEzviz* userInfo)
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


void CVideoUserManagerDlg::InsertUserList(video::normal::CVideoUserInfoNormal* userInfo)
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


void CVideoUserManagerDlg::InsertDeviceList(video::ezviz::CVideoDeviceInfoEzviz* deviceInfo)
{
	AUTO_LOG_FUNCTION;
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


void CVideoUserManagerDlg::InsertDeviceList(video::normal::CVideoDeviceInfoNormal* deviceInfo)
{

}


void CVideoUserManagerDlg::OnLvnItemchangedListUser(NMHDR * pNMHDR, LRESULT * pResult)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	if (pResult)
		*pResult = 0;

	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if (pNMLV == NULL || m_listUser.GetItemCount() == 0) {
		ResetUserListSelectionInfo();
		return;
	}
	video::CVideoUserInfo* user = reinterpret_cast<video::CVideoUserInfo*>(pNMLV->lParam);
	if (m_curSelUserInfo == user) {
		return;
	}
	m_curSelUserInfo = user;
	m_curselUserListItem = pNMLV->iItem;

	ShowUsersDeviceList(user);
}


void CVideoUserManagerDlg::OnLvnItemchangedListDevice(NMHDR *pNMHDR, LRESULT *pResult)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	if (pResult)
		*pResult = 0;

	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if (pNMLV == NULL || m_listDevice.GetItemCount() == 0) {
		ResetDeviceListSelectionInfo();
		return;
	}
	video::ezviz::CVideoDeviceInfoEzviz* dev = reinterpret_cast<video::ezviz::CVideoDeviceInfoEzviz*>(pNMLV->lParam);
	if (m_curSelDeviceInfo == dev) {
		return;
	}
	m_curSelDeviceInfo = dev;
	m_curselDeviceListItem = pNMLV->iItem;
	ShowDeviceInfo(dev);
}


void CVideoUserManagerDlg::ShowDeviceInfo(video::ezviz::CVideoDeviceInfoEzviz* device)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;

	if (!device) {
		ResetDeviceListSelectionInfo();
		return;
	}

	if (m_privilege == core::UP_OPERATOR) {
		m_btnBindOrUnbind.EnableWindow(0);
		m_chkAutoPlayVideo.EnableWindow(0);
		m_btnAddDevice.EnableWindow(0);
		m_btnDelDevice.EnableWindow(0);
		m_btnSaveDevChange.EnableWindow(0);
		m_btnRefreshDev.EnableWindow(0);
	} else {
		m_btnBindOrUnbind.EnableWindow();
		m_chkAutoPlayVideo.EnableWindow();
		m_btnAddDevice.EnableWindow();
		m_btnDelDevice.EnableWindow();
		m_btnSaveDevChange.EnableWindow();
		m_btnRefreshDev.EnableWindow();
	}

	CString txt;
	txt.Format(L"%d", device->get_id());
	m_idDev.SetWindowTextW(txt);
	txt.Format(L"%s", A2W(device->get_cameraName().c_str()));
	m_nameDev.SetWindowTextW(txt);
	txt.Format(L"%s", device->get_device_note().c_str());
	m_noteDev.SetWindowTextW(txt);
	txt.Format(L"%s", A2W(device->get_secure_code().c_str()));
	m_devCode.SetWindowTextW(txt);

	bool binded = false;
	video::ZoneUuid zone = device->get_zoneUuid();
	video::BindInfo bi = video::CVideoManager::GetInstance()->GetBindInfo(zone);
	if (device->get_binded()) {
		binded = true;
		if (!CheckZoneInfoExsist(zone) || bi._device != device) {
			video::CVideoManager::GetInstance()->UnbindZoneAndDevice(zone);
			device->set_binded(false);
			binded = false;
		}
	}
	if (binded) {
		if (zone._gg == core::INDEX_ZONE) {
			txt.Format(L"%04d[%03d]", zone._ademco_id, zone._zone_value);
		} else {
			txt.Format(L"%04d[%03d][%02d]", zone._ademco_id, zone._zone_value, zone._gg);
		}
		m_zone.SetWindowTextW(txt);
		txt.LoadStringW(IDS_STRING_UNBIND_ZONE);
		m_btnBindOrUnbind.SetWindowTextW(txt);
		m_chkAutoPlayVideo.SetCheck(bi._auto_play_video);
	} else {
		m_zone.SetWindowTextW(L"");
		txt.LoadStringW(IDS_STRING_BIND_ZONE);
		m_btnBindOrUnbind.SetWindowTextW(txt);
		m_chkAutoPlayVideo.SetCheck(0);
		m_chkAutoPlayVideo.EnableWindow(0);
	}
}


bool CVideoUserManagerDlg::CheckZoneInfoExsist(const video::ZoneUuid& zone)
{
	do {
		core::CAlarmMachineManager* mgr = core::CAlarmMachineManager::GetInstance();
		core::CAlarmMachine* machine = NULL;
		if (!mgr->GetMachine(zone._ademco_id, machine) || machine == NULL)
			break;
		core::CZoneInfo* zoneInfo = machine->GetZone(zone._zone_value);
		if (!zoneInfo)
			break;

		core::CAlarmMachine* subMachine = zoneInfo->GetSubMachineInfo();
		if (zone._gg == core::INDEX_ZONE) {
			if (subMachine != NULL)
				break;
		} else {
			if (subMachine == NULL)
				break;
			if (zone._gg == core::INDEX_SUB_MACHINE) 
				break;
			if (subMachine->GetZone(zone._gg) == NULL)
				break;
		}
		
		return true;
	} while (0);
	return false;
}


void CVideoUserManagerDlg::ShowUsersDeviceList(video::CVideoUserInfo* user)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;

	if (!user) {
		ResetUserListSelectionInfo();
		return;
	}

	if (m_privilege == core::UP_OPERATOR) {
		m_btnAddUser.EnableWindow(0);
		m_btnDelUser.EnableWindow(0);
		m_btnUpdateUser.EnableWindow(0);
		m_btnRefreshDeviceList.EnableWindow(0);
	} else {
		m_btnAddUser.EnableWindow();
		m_btnDelUser.EnableWindow();
		m_btnUpdateUser.EnableWindow();
		m_btnRefreshDeviceList.EnableWindow();
	}

	CString fm, txt;
	fm.LoadStringW(IDS_STRING_FM_USERS_DEV_LIST);
	txt.Format(fm, user->get_user_name().c_str(), user->get_device_count());
	m_groupDevice.SetWindowTextW(txt);
	video::CVideoDeviceInfoList list;

	if (user->get_productorInfo().get_productor() == video::EZVIZ) {
		m_listDevice.DeleteAllItems();
		m_listDevice2.DeleteAllItems();
		m_listDevice.ShowWindow(SW_SHOW);
		m_listDevice2.ShowWindow(SW_HIDE);
		video::ezviz::CVideoUserInfoEzviz* userEzviz = reinterpret_cast<video::ezviz::CVideoUserInfoEzviz*>(user);
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
		//video::CVideoDeviceInfoListIter iter = list.begin();
		//while (iter != list.end()) {
		for (auto &i : list) {
			video::ezviz::CVideoDeviceInfoEzviz* device = reinterpret_cast<video::ezviz::CVideoDeviceInfoEzviz*>(i);
			InsertDeviceList(device);
		}
	} else if (user->get_productorInfo().get_productor() == video::NORMAL) {
		m_listDevice.DeleteAllItems();
		m_listDevice2.DeleteAllItems();
		m_listDevice.ShowWindow(SW_HIDE);
		m_listDevice2.ShowWindow(SW_SHOW);
		video::normal::CVideoUserInfoNormal* uesrNormal = reinterpret_cast<video::normal::CVideoUserInfoNormal*>(user);
		uesrNormal->GetDeviceList(list);
		//video::CVideoDeviceInfoListIter iter = list.begin();
		//while (iter != list.end()) {
		for (auto &i : list) {
			video::normal::CVideoDeviceInfoNormal* device = reinterpret_cast<video::normal::CVideoDeviceInfoNormal*>(i);
			InsertDeviceList(device);
		}
	} else {
		ASSERT(0); return;
	}
}


void CVideoUserManagerDlg::OnBnClickedButtonSaveChange()
{
	if (m_curSelUserInfo == NULL || m_curselUserListItem == -1) { return; }
	CString name; m_name.GetWindowTextW(name);
	if (name.Compare(m_curSelUserInfo->get_user_name().c_str()) != 0) {
		if (video::EZVIZ == m_curSelUserInfo->get_productorInfo().get_productor()) {
			video::ezviz::CVideoUserInfoEzviz* user = reinterpret_cast<video::ezviz::CVideoUserInfoEzviz*>(m_curSelUserInfo);
			if (user->execute_set_user_name(name.LockBuffer())) {
				UpdateUserList(m_curselUserListItem, user);
			}
			name.UnlockBuffer();
		}
	}
}


void CVideoUserManagerDlg::OnBnClickedButtonDelUser()
{
	AUTO_LOG_FUNCTION;
	if (m_curSelUserInfo == NULL || m_curselUserListItem == -1) { return; }
	CString info; info.LoadStringW(IDS_STRING_CONFIRM_DEL_VIDEO_USER);
	int ret = MessageBox(info, L"", MB_OKCANCEL | MB_ICONWARNING);
	if (ret != IDOK)return;

	if (m_curSelUserInfo->get_productorInfo().get_productor() == video::EZVIZ) {
		video::ezviz::CVideoUserInfoEzviz* user = reinterpret_cast<video::ezviz::CVideoUserInfoEzviz*>(m_curSelUserInfo);
		if (video::CVideoManager::GetInstance()->DeleteVideoUser(user)) {
			InitUserList();
			OnLvnItemchangedListUser(NULL, NULL);
		}
	} else if(m_curSelUserInfo->get_productorInfo().get_productor() == video::NORMAL) {
		video::normal::CVideoUserInfoNormal* user = reinterpret_cast<video::normal::CVideoUserInfoNormal*>(m_curSelDeviceInfo);
		// TODO 2015Äê9ÔÂ11ÈÕ20:50:41
		/*if (video::CVideoManager::GetInstance()->DeleteVideoUser(user)) {

		}*/
	}
}


void CVideoUserManagerDlg::OnBnClickedButtonAddUser()
{
	AUTO_LOG_FUNCTION;
	CAddVideoUserEzvizDlg dlg;
	if (IDOK != dlg.DoModal())
		return;
	USES_CONVERSION;
	video::CVideoManager* mgr = video::CVideoManager::GetInstance();
	video::CVideoManager::VideoEzvizResult result = mgr->AddVideoUserEzviz(dlg.m_strName.LockBuffer(), W2A(dlg.m_strPhone));
	dlg.m_strName.UnlockBuffer();
	CString e;
	if (result == video::CVideoManager::RESULT_OK) {
		InitUserList();
	} else if (result == video::CVideoManager::RESULT_INSERT_TO_DB_FAILED) {

	} else if (result == video::CVideoManager::RESULT_PRIVATE_CLOUD_CONNECT_FAILED_OR_USER_NOT_EXIST) {
		e.LoadStringW(IDS_STRING_PRIVATE_CLOUD_CONN_FAIL_OR_USER_NOT_EXSIST);
		MessageBox(e, L"", MB_ICONERROR);
	} else if (result == video::CVideoManager::RESULT_USER_ALREADY_EXSIST) {

	} else { assert(0); }
	
}


void CVideoUserManagerDlg::OnBnClickedButtonRefreshDeviceList()
{
	AUTO_LOG_FUNCTION;
	if (m_curSelUserInfo == NULL || m_curselUserListItem == -1) { return; }
	if (m_curSelUserInfo->get_productorInfo().get_productor() == video::EZVIZ) {
		video::ezviz::CVideoUserInfoEzviz* user = reinterpret_cast<video::ezviz::CVideoUserInfoEzviz*>(m_curSelUserInfo);
		video::CVideoManager* mgr = video::CVideoManager::GetInstance();
		video::CVideoManager::VideoEzvizResult result = mgr->RefreshUserEzvizDeviceList(user);
		if (result == video::CVideoManager::RESULT_OK) {
			ShowUsersDeviceList(user);
		} else if (result == video::CVideoManager::RESULT_PRIVATE_CLOUD_CONNECT_FAILED_OR_USER_NOT_EXIST) {
			CString e; e.LoadStringW(IDS_STRING_PRIVATE_CLOUD_CONN_FAIL_OR_USER_NOT_EXSIST);
			MessageBox(e, L"", MB_ICONERROR);
		}
	}
}
	


