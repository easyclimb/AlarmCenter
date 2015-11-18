// ChooseVideoDeviceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "ChooseVideoDeviceDlg.h"
#include "afxdialogex.h"
#include "VideoManager.h"
#include "VideoUserInfoEzviz.h"
#include "VideoDeviceInfoEzviz.h"

// CChooseVideoDeviceDlg dialog

IMPLEMENT_DYNAMIC(CChooseVideoDeviceDlg, CDialogEx)

CChooseVideoDeviceDlg::CChooseVideoDeviceDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CChooseVideoDeviceDlg::IDD, pParent)
	, m_dev(nullptr)
{

}

CChooseVideoDeviceDlg::~CChooseVideoDeviceDlg()
{
}

void CChooseVideoDeviceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_userList);
	DDX_Control(pDX, IDC_LIST3, m_devList);
	DDX_Control(pDX, IDOK, m_btnOk);
}


BEGIN_MESSAGE_MAP(CChooseVideoDeviceDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CChooseVideoDeviceDlg::OnBnClickedOk)
	ON_LBN_SELCHANGE(IDC_LIST1, &CChooseVideoDeviceDlg::OnLbnSelchangeListUser)
	ON_LBN_SELCHANGE(IDC_LIST3, &CChooseVideoDeviceDlg::OnLbnSelchangeListDev)
END_MESSAGE_MAP()


// CChooseVideoDeviceDlg message handlers


void CChooseVideoDeviceDlg::OnBnClickedOk()
{
	CDialogEx::OnOK();
}


BOOL CChooseVideoDeviceDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString txt = L"";
	video::CVideoUserInfoList list;
	video::CVideoManager::GetInstance()->GetVideoUserList(list);
	for (auto usr : list) {
		if (usr->get_productorInfo().get_productor() == video::EZVIZ) {
			txt.Format(L"%d--%s--%s[%s]", usr->get_id(), usr->get_user_name().c_str(), 
					   usr->get_productorInfo().get_name().c_str(), usr->get_productorInfo().get_description().c_str());
			int ndx = m_userList.AddString(txt);
			m_userList.SetItemData(ndx, reinterpret_cast<DWORD_PTR>(usr));
		}
	}

	m_btnOk.EnableWindow(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CChooseVideoDeviceDlg::OnLbnSelchangeListUser()
{
	USES_CONVERSION;
	m_devList.ResetContent();
	m_dev = nullptr;
	int ndx = m_userList.GetCurSel();
	if (ndx < 0)return;
	video::CVideoUserInfo* user = reinterpret_cast<video::CVideoUserInfo*>(m_userList.GetItemData(ndx));
	assert(user);
	CString txt = L"";
	if (user->get_productorInfo().get_productor() == video::EZVIZ) {
		video::CVideoDeviceInfoList list;
		user->GetDeviceList(list);
		for (auto dev : list) {
			auto device = reinterpret_cast<video::ezviz::CVideoDeviceInfoEzviz*>(dev);
			//if (!device->get_binded()) {
				txt.Format(L"%d--%s--%s", device->get_id(), device->get_device_note().c_str(), A2W(device->get_deviceSerial().c_str()));
				ndx = m_devList.AddString(txt);
				m_devList.SetItemData(ndx, reinterpret_cast<DWORD_PTR>(device));
			//}
		}
	}
	m_btnOk.EnableWindow(0);
}


void CChooseVideoDeviceDlg::OnLbnSelchangeListDev()
{
	m_btnOk.EnableWindow(0);
	m_dev = nullptr;
	int ndx = m_devList.GetCurSel(); if (ndx < 0)return;
	m_dev = reinterpret_cast<video::CVideoDeviceInfo*>(m_devList.GetItemData(ndx));
	m_btnOk.EnableWindow();
}
