// DetectorBindWizardChooseCameraPage.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "DetectorBindWizardChooseCameraPage.h"
#include "afxdialogex.h"
#include "BmpEx.h"
#include "VideoDeviceInfoEzviz.h"
#include "VideoManager.h"
#include "DetectorLib.h"
#include "VideoUserInfo.h"
#include <sstream>
#include "AlarmMachine.h"
#include "ZoneInfo.h"
// CDetectorBindWizardChooseCameraPage dialog

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
END_MESSAGE_MAP()


// CDetectorBindWizardChooseCameraPage message handlers

void CDetectorBindWizardChooseCameraPage::OnLbnSelchangeList1()
{
	int ndx = m_list.GetCurSel();
	if (ndx < 0) return;
	m_curSelDev = video::video_manager::GetInstance()->GetVideoDeviceInfoEzviz(m_list.GetItemData(ndx));
}


BOOL CDetectorBindWizardChooseCameraPage::OnSetActive()
{
	using namespace core;
	using namespace gui::control;
	CPropertySheet* parent = reinterpret_cast<CPropertySheet*>(GetParent());
	parent->SetWizardButtons(PSWIZB_NEXT | PSWIZB_CANCEL);

	m_list.ResetContent();
	if (m_ImageList.GetSafeHandle() != nullptr) {
		m_ImageList.DeleteImageList();
	}
	if (m_ImageListRotate.GetSafeHandle() != nullptr) {
		m_ImageListRotate.DeleteImageList();
	}

	detector_lib_manager* lib = detector_lib_manager::GetInstance();
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

	video::video_device_info_list devList;
	//video::video_manager::GetInstance()->GetVideoDeviceList(devList);
	

	zone_info_list zoneList;
	m_machine->GetAllZoneInfo(zoneList);
	for (auto zoneInfo : zoneList) {
		video::zone_uuid zoneUuid(m_machine->get_ademco_id(), zoneInfo->get_zone_value(), 0);
		if (m_machine->get_is_submachine()) {
			zoneUuid._gg = zoneInfo->get_sub_zone();
		}
		video::bind_info bi = video::video_manager::GetInstance()->GetBindInfo(zoneUuid);
		if (bi._device) {
			devList.push_back(bi._device);
		}
	}

	if (devList.size() == 0)
		return CPropertyPage::OnSetActive();

	CString txt;
	const detector_lib_data_ptr data = lib->GetDetectorLibData(DI_CAMERA);
	for (auto dev : devList) {
		if (dev->get_userInfo()->get_productorInfo().get_productor() == video::EZVIZ) {
			video::ezviz::video_device_info_ezviz_ptr device = std::dynamic_pointer_cast<video::ezviz::video_device_info_ezviz>(dev);
			std::string serial = device->get_deviceSerial();
			std::wstring wserial;
			utf8::utf8to16(serial.begin(), serial.end(), std::back_inserter(wserial));
			txt.Format(L"%s[%d-%s-%s]", data->get_detector_name(), device->get_id(), wserial.c_str(), device->get_device_note().c_str());
			m_list.InsertString(ndx, txt, DI_CAMERA - 1, -1);
			m_list.SetItemData(ndx, dev->get_id());
		}
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

