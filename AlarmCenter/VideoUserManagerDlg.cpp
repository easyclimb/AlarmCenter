// VideoUserManagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "VideoUserManagerDlg.h"
#include "afxdialogex.h"
#include "VideoInfo.h"
#include "VideoUserInfoEzviz.h"
#include "VideoUserInfoNormal.h"

// CVideoUserManagerDlg dialog

IMPLEMENT_DYNAMIC(CVideoUserManagerDlg, CDialogEx)

CVideoUserManagerDlg::CVideoUserManagerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CVideoUserManagerDlg::IDD, pParent)
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
}


BEGIN_MESSAGE_MAP(CVideoUserManagerDlg, CDialogEx)
END_MESSAGE_MAP()


// CVideoUserManagerDlg message handlers


BOOL CVideoUserManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

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
	fm.LoadStringW(IDS_STRING_ACCT);
	m_listUser.InsertColumn(++i, fm, LVCFMT_LEFT, 100, -1);
	//fm.LoadStringW(IDS_STRING_USER_PASSWD);
	//m_listUser.InsertColumn(++i, fm, LVCFMT_LEFT, 225, -1);
	fm.LoadStringW(IDS_STRING_DEVICE_COUNT);
	m_listUser.InsertColumn(++i, fm, LVCFMT_LEFT, 60, -1);
	InitUserList();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CVideoUserManagerDlg::InitUserList()
{
	USES_CONVERSION;
	m_listUser.DeleteAllItems();
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
		lvitem.iSubItem += 2;
		tmp.Format(_T("%d"), userInfo->get_device_count());
		lvitem.pszText = tmp.LockBuffer();
		m_listUser.SetItem(&lvitem);
		tmp.UnlockBuffer();

		m_listUser.SetItemData(nResult, reinterpret_cast<DWORD_PTR>(userInfo));
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
