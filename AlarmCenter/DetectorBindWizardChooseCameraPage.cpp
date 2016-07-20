// DetectorBindWizardChooseCameraPage.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "DetectorBindWizardChooseCameraPage.h"
#include "afxdialogex.h"
#include "BmpEx.h"
#include "../video/ezviz/VideoDeviceInfoEzviz.h"
#include "../video/VideoUserInfo.h"
#include "DetectorLib.h"
#include <sstream>
#include "AlarmMachine.h"
#include "ZoneInfo.h"
#include "alarm_center_video_service.h"


IMPLEMENT_DYNAMIC(CDetectorBindWizardChooseCameraPage, CPropertyPage)

CDetectorBindWizardChooseCameraPage::CDetectorBindWizardChooseCameraPage()
	: CPropertyPage(IDD_PROPPAGE_LARGE_CHOOSE_CAMERA)
{

}

CDetectorBindWizardChooseCameraPage::~CDetectorBindWizardChooseCameraPage()
{
}

void CDetectorBindWizardChooseCameraPage::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_LIST1, m_list);
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDetectorBindWizardChooseCameraPage, CPropertyPage)
	ON_LBN_SELCHANGE(IDC_LIST1, &CDetectorBindWizardChooseCameraPage::OnLbnSelchangeList1)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CDetectorBindWizardChooseCameraPage message handlers

void CDetectorBindWizardChooseCameraPage::OnLbnSelchangeList1()
{
	int ndx = m_list.GetCurSel();
	if (ndx < 0) return;
	auto data = reinterpret_cast<video::video_device_identifier*>(m_list.GetItemData(ndx));
	m_curSelDev = ipc::alarm_center_video_service::get_instance()->get_device(data);
}


BOOL CDetectorBindWizardChooseCameraPage::OnSetActive()
{
	using namespace core;
	using namespace gui::control;
	CPropertySheet* parent = reinterpret_cast<CPropertySheet*>(GetParent());
	parent->SetWizardButtons(PSWIZB_NEXT | PSWIZB_CANCEL);

	for (int i = 0; i < m_list.GetCount(); i++) {
		auto data = reinterpret_cast<video::video_device_identifier*>(m_list.GetItemData(i)); assert(data);
		SAFEDELETEP(data);
	}
	m_list.ResetContent();
	if (m_ImageList.GetSafeHandle() != nullptr) {
		m_ImageList.DeleteImageList();
	}
	if (m_ImageListRotate.GetSafeHandle() != nullptr) {
		m_ImageListRotate.DeleteImageList();
	}

	auto lib = detector_lib_manager::get_instance();
	std::list<detector_lib_data_ptr> detectorLiblist;
	lib->GetAllLibData(detectorLiblist);
	m_ImageList.Create(THUMBNAILWIDTH, THUMBNAILWIDTH, ILC_COLOR24, 0, 1);
	m_ImageListRotate.Create(THUMBNAILWIDTH, THUMBNAILWIDTH, ILC_COLOR24, 0, 1);

	CString path = _T("");
	int ndx = 0, prev_ndx = 0;
	m_ImageList.SetImageCount(detectorLiblist.size());
	m_ImageListRotate.SetImageCount(detectorLiblist.size());

	ndx = 0;
	for (auto data : detectorLiblist) {
		path = data->get_path();
		HBITMAP hBitmap = CBmpEx::GetHBitmapThumbnail(path, THUMBNAILWIDTH, THUMBNAILWIDTH);
		if (hBitmap) {
			auto pImage = std::make_unique<CBitmap>();
			pImage->Attach(hBitmap);
			m_ImageList.Replace(ndx, pImage.get(), nullptr);
			m_ImageListRotate.Replace(ndx, pImage.get(), nullptr);
			ndx++;
		}
	}
	m_list.SetImageList(&m_ImageList, &m_ImageListRotate);

	ndx = 0;

	video::device_list devList;
	//video::video_manager::get_instance()->GetVideoDeviceList(devList);
	

	zone_info_list zoneList;
	m_machine->GetAllZoneInfo(zoneList);
	for (auto zoneInfo : zoneList) {
		video::zone_uuid zoneUuid(m_machine->get_ademco_id(), zoneInfo->get_zone_value(), 0);
		if (m_machine->get_is_submachine()) {
			zoneUuid._gg = zoneInfo->get_sub_zone();
		}
		video::bind_info bi = ipc::alarm_center_video_service::get_instance()->get_bind_info(zoneUuid);
		if (bi._device) {
			devList.push_back(bi._device);
		}
	}

	if (devList.size() == 0)
		return CPropertyPage::OnSetActive();

	ndx = 0;
	CString txt;
	const detector_lib_data_ptr data = lib->GetDetectorLibData(DI_CAMERA);
	for (auto dev : devList) {
		txt.Format(L"%s[%s]", data->get_detector_name(), dev->get_formatted_name().c_str());
		ndx = m_list.InsertString(ndx, txt, DI_CAMERA - 1, -1);
		m_list.SetItemData(ndx, reinterpret_cast<DWORD_PTR>(dev->create_identifier()));
		ndx++;
	}
	m_list.SetCurSel(prev_ndx);
	OnLbnSelchangeList1();

	return CPropertyPage::OnSetActive();
}


BOOL CDetectorBindWizardChooseCameraPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();


	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}



void CDetectorBindWizardChooseCameraPage::OnDestroy()
{
	CPropertyPage::OnDestroy();

	for (int i = 0; i < m_list.GetCount(); i++) {
		auto data = reinterpret_cast<video::video_device_identifier*>(m_list.GetItemData(i)); assert(data);
		SAFEDELETEP(data);
	}
	m_list.ResetContent();
}
