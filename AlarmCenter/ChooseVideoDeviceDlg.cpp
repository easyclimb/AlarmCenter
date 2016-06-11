// ChooseVideoDeviceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AppResource.h"
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
	video::video_user_info_list list;
	video::video_manager::get_instance()->GetVideoUserList(list);
	for (auto usr : list) {
		if (usr->get_productorInfo().get_productor() == video::EZVIZ) {
			txt.Format(L"%d--%s--%s[%s]", usr->get_id(), usr->get_user_name().c_str(), 
					   usr->get_productorInfo().get_name().c_str(), usr->get_productorInfo().get_description().c_str());
			int ndx = m_userList.AddString(txt);
			m_userList.SetItemData(ndx, usr->get_id());
		}
	}

	m_btnOk.EnableWindow(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CChooseVideoDeviceDlg::OnLbnSelchangeListUser()
{
	m_devList.ResetContent();
	m_dev = nullptr;
	int ndx = m_userList.GetCurSel();
	if (ndx < 0)return;
	video::video_user_info_ptr user = video::video_manager::get_instance()->GetVideoUserEzviz(m_userList.GetItemData(ndx));
	assert(user);
	CString txt = L"";
	if (user->get_productorInfo().get_productor() == video::EZVIZ) {
		video::video_device_info_list list;
		user->GetDeviceList(list);
		for (auto dev : list) {
			auto device = std::dynamic_pointer_cast<video::ezviz::video_device_info_ezviz>(dev);
			//if (!device->get_binded()) {
				txt.Format(L"%d--%s--%s", device->get_id(), device->get_device_note().c_str(), utf8::a2w(device->get_deviceSerial()).c_str());
				ndx = m_devList.AddString(txt);
				m_devList.SetItemData(ndx, device->get_id());
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
	m_dev = video::video_manager::get_instance()->GetVideoDeviceInfoEzviz(m_devList.GetItemData(ndx));
	m_btnOk.EnableWindow();
}
