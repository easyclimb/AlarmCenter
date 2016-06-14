// ChooseVideoDeviceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AppResource.h"
#include "ChooseVideoDeviceDlg.h"
#include "afxdialogex.h"
#include "VideoManager.h"
#include "VideoUserInfoEzviz.h"
#include "VideoDeviceInfoEzviz.h"
#include "VideoUserInfoJovision.h"
#include "VideoDeviceInfoJovision.h"

using namespace video;

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
	ON_WM_DESTROY()
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
		txt.Format(L"%d--%s--%s", usr->get_id(), usr->get_user_name().c_str(), 
				   usr->get_productorInfo().get_formatted_name().c_str());
		int ndx = m_userList.AddString(txt);
		m_userList.SetItemData(ndx, usr->get_id());
	}

	m_btnOk.EnableWindow(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CChooseVideoDeviceDlg::OnLbnSelchangeListUser()
{
	for (int i = 0; i < m_devList.GetCount(); i++) {
		auto data = reinterpret_cast<video::video_device_identifier*>(m_devList.GetItemData(i));
		SAFEDELETEP(data);
	}
	m_devList.ResetContent();
	m_dev = nullptr;
	int ndx = m_userList.GetCurSel();
	if (ndx < 0)return;
	video::video_user_info_ptr user = nullptr;
	user = video::video_manager::get_instance()->GetVideoUserEzviz(m_userList.GetItemData(ndx));
	if (!user) {
		user = video_manager::get_instance()->GetVideoUserJovision(m_userList.GetItemData(ndx));
	}
	assert(user);
	
	CString txt = L"";
	video::video_device_info_list list;
	user->GetDeviceList(list);
	auto productor = user->get_productorInfo().get_productor();
	
	for (auto dev : list) {
		if (productor == video::EZVIZ) {
			auto device = std::dynamic_pointer_cast<video::ezviz::video_device_info_ezviz>(dev);
			txt = device->get_formatted_name().c_str();
			ndx = m_devList.AddString(txt);
		} else if (productor == video::JOVISION) {
			auto device = std::dynamic_pointer_cast<video::jovision::video_device_info_jovision>(dev);
			txt = device->get_formatted_name().c_str();
			ndx = m_devList.AddString(txt);
		} else {
			assert(0);
			continue;
		}

		video::video_device_identifier* data = new video::video_device_identifier();
		data->dev_id = dev->get_id();
		data->productor = productor;
		m_devList.SetItemData(ndx, reinterpret_cast<DWORD_PTR>(data));

	}
	
	m_btnOk.EnableWindow(0);
}


void CChooseVideoDeviceDlg::OnLbnSelchangeListDev()
{
	m_btnOk.EnableWindow(0);
	m_dev = nullptr;
	int ndx = m_devList.GetCurSel(); if (ndx < 0)return;
	auto data = reinterpret_cast<video::video_device_identifier*>(m_devList.GetItemData(ndx)); assert(data);
	m_dev = video::video_manager::get_instance()->GetVideoDeviceInfo(data);
	m_btnOk.EnableWindow();
}


void CChooseVideoDeviceDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	for (int i = 0; i < m_devList.GetCount(); i++) {
		auto data = reinterpret_cast<video::video_device_identifier*>(m_devList.GetItemData(i));
		SAFEDELETEP(data);
	}

}
