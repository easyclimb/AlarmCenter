// DetectorBindWizardChooseDetPage.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "DetectorBindWizardChooseDetPage.h"
#include "afxdialogex.h"
#include "DetectorBindWizardChooseZonePage.h"
#include "DetectorLib.h"
#include "DetectorInfo.h"
#include "BmpEx.h"
#include "ZoneInfo.h"
#include "AlarmMachine.h"

using namespace core;
// CDetectorBindWizardChooseDetPage dialog

IMPLEMENT_DYNAMIC(CDetectorBindWizardChooseDetPage, CPropertyPage)

CDetectorBindWizardChooseDetPage::CDetectorBindWizardChooseDetPage()
	: CPropertyPage(CDetectorBindWizardChooseDetPage::IDD)
	, m_detLibID(-1)
	, m_machine(nullptr)
{

}

CDetectorBindWizardChooseDetPage::~CDetectorBindWizardChooseDetPage()
{
}

void CDetectorBindWizardChooseDetPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
}


BEGIN_MESSAGE_MAP(CDetectorBindWizardChooseDetPage, CPropertyPage)
	ON_LBN_SELCHANGE(IDC_LIST1, &CDetectorBindWizardChooseDetPage::OnLbnSelchangeList1)
END_MESSAGE_MAP()


// CDetectorBindWizardChooseDetPage message handlers


BOOL CDetectorBindWizardChooseDetPage::OnSetActive()
{
	using namespace gui::control;
	CPropertySheet* parent = reinterpret_cast<CPropertySheet*>(GetParent());
	parent->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT | PSWIZB_CANCEL);

	m_list.ResetContent();
	if (m_ImageList.GetSafeHandle() != nullptr) {
		m_ImageList.DeleteImageList();
	}
	if (m_ImageListRotate.GetSafeHandle() != nullptr) {
		m_ImageListRotate.DeleteImageList();
	}

	CDetectorLib* lib = CDetectorLib::GetInstance();
	CDetectorBindWizardChooseZonePage *page0 = reinterpret_cast<CDetectorBindWizardChooseZonePage*>(parent->GetPage(0));
	int zoneValue = page0->m_zoneValue;
	int detType2Show = DT_MAX;
	zone_info_ptr zoneInfo = m_machine->GetZone(zoneValue);
	if (zoneInfo) {
		ZoneType zt = zoneInfo->get_type();
		if (ZT_SUB_MACHINE == zt) {
			detType2Show = DT_SUB_MACHINE;
		} else {
			detType2Show = DT_SINGLE | DT_DOUBLE;
		}
	}

	std::list<detector_lib_data_ptr> list;
	lib->GetAllLibData(list);
	m_ImageList.Create(THUMBNAILWIDTH, THUMBNAILWIDTH, ILC_COLOR24, 0, 1);
	m_ImageListRotate.Create(THUMBNAILWIDTH, THUMBNAILWIDTH, ILC_COLOR24, 0, 1);

	bool bSettedType = (detType2Show < DT_MAX);

	CString path = _T("");
	int ndx = 0, prev_ndx = 0;

	for (auto data : list) {
		if (bSettedType && ((data->get_type() & detType2Show) == 0))
			continue;
		ndx++;
	}
	m_ImageList.SetImageCount(ndx);
	m_ImageListRotate.SetImageCount(ndx);

	ndx = 0;
	for (auto data : list) {
		if (bSettedType && ((data->get_type() & detType2Show) == 0))
			continue;

		path = data->get_path();
		HBITMAP hBitmap = CBmpEx::GetHBitmapThumbnail(path, THUMBNAILWIDTH, THUMBNAILWIDTH);
		if (hBitmap) {
			auto pImage = std::make_unique<CBitmap>();
			pImage->Attach(hBitmap);
			m_ImageList.Replace(ndx, pImage.get(), nullptr);
			if (data->get_type() == DT_DOUBLE) {
				HBITMAP hBitmapPair = CBmpEx::GetHBitmapThumbnail(data->get_path_pair(),
																  THUMBNAILWIDTH,
																  THUMBNAILWIDTH);
				auto pImagePair = std::make_unique<CBitmap>();
				pImagePair->Attach(hBitmapPair);
				m_ImageListRotate.Replace(ndx, pImagePair.get(), nullptr);
			} else {
				m_ImageListRotate.Replace(ndx, pImage.get(), nullptr);
			}
			ndx++;
		}
	}
	m_list.SetImageList(&m_ImageList, &m_ImageListRotate);

	ndx = 0;
	for (auto data : list) {
		if (bSettedType && ((data->get_type() & detType2Show) == 0))
			continue;
		m_list.InsertString(ndx, data->get_detector_name(), ndx,
							(data->get_type() == DT_DOUBLE) ? ndx : -1);
		m_list.SetItemData(ndx, data->get_id());
		if (data->get_id() == m_detLibID)
			prev_ndx = ndx;
		ndx++;
	}
	m_list.SetCurSel(prev_ndx);
	OnLbnSelchangeList1();

	return CPropertyPage::OnSetActive();
}


BOOL CDetectorBindWizardChooseDetPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CDetectorBindWizardChooseDetPage::OnLbnSelchangeList1()
{
	int ndx = m_list.GetCurSel();
	if (ndx < 0) return;
	m_detLibID = m_list.GetItemData(ndx);
}
