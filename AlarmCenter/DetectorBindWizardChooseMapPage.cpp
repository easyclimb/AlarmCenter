// DetectorBindWizardChooseMapPage.cpp : implementation file
//

#include "stdafx.h"
#include "AppResource.h"
#include "DetectorBindWizardChooseMapPage.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "MapInfo.h"
#include "BmpEx.h"

using namespace core;

// CDetectorBindWizardChooseMapPage dialog

IMPLEMENT_DYNAMIC(CDetectorBindWizardChooseMapPage, CPropertyPage)

CDetectorBindWizardChooseMapPage::CDetectorBindWizardChooseMapPage()
	: CPropertyPage(CDetectorBindWizardChooseMapPage::IDD)
	, m_machine(nullptr)
	, m_prevMap(nullptr)
	, m_mapId(-1)
{

}

CDetectorBindWizardChooseMapPage::~CDetectorBindWizardChooseMapPage()
{
}

void CDetectorBindWizardChooseMapPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
}


BEGIN_MESSAGE_MAP(CDetectorBindWizardChooseMapPage, CPropertyPage)
	ON_LBN_SELCHANGE(IDC_LIST1, &CDetectorBindWizardChooseMapPage::OnLbnSelchangeList1)
END_MESSAGE_MAP()


// CDetectorBindWizardChooseMapPage message handlers


BOOL CDetectorBindWizardChooseMapPage::OnSetActive()
{
	using namespace gui::control;
	CPropertySheet* parent = reinterpret_cast<CPropertySheet*>(GetParent());
	parent->SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH | PSWIZB_CANCEL);

	m_list.ResetContent();
	if (m_ImageList.GetSafeHandle() != nullptr) {
		m_ImageList.DeleteImageList();
	}
	static const int width = static_cast<int>(THUMBNAILWIDTH * 1.5);
	static const int height = THUMBNAILWIDTH;
	m_ImageList.Create(width, height, ILC_COLOR24, 0, 1);
	
	map_info_list list;
	m_machine->GetAllMapInfo(list);
	m_ImageList.SetImageCount(list.size());
	int ndx = 0, prev_ndx = 0;
	for (auto mapInfo : list) {
		HBITMAP hBitmap = CBmpEx::GetHBitmapThumbnail(mapInfo->get_path(), 
													  width, height);
		if (hBitmap) {
			auto pImage = std::make_unique<CBitmap>();
			pImage->Attach(hBitmap);
			m_ImageList.Replace(ndx, pImage.get(), nullptr);
			ndx++;
		}
	}
	m_list.SetImageList(&m_ImageList, nullptr);
	ndx = 0;
	for (auto mapInfo : list) {
		m_list.InsertString(ndx, mapInfo->get_alias(), ndx, -1);
		m_list.SetItemData(ndx, mapInfo->get_id());
		if (mapInfo->get_id() == m_mapId) {
			prev_ndx = ndx;
		}
		ndx++;
	}
	m_list.SetCurSel(prev_ndx);
	OnLbnSelchangeList1();

	return CPropertyPage::OnSetActive();
}


BOOL CDetectorBindWizardChooseMapPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();



	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CDetectorBindWizardChooseMapPage::OnLbnSelchangeList1()
{
	if (m_prevMap) {
		m_prevMap->InversionControl(ICMC_MODE_NORMAL);
	}
	int ndx = m_list.GetCurSel();
	if (ndx < 0) return;
	m_mapId = m_list.GetItemData(ndx);
	map_info_ptr mapInfo = m_machine->GetMapInfo(m_mapId);
	if (mapInfo) {
		mapInfo->InversionControl(ICMC_MODE_EDIT);
		mapInfo->InversionControl(ICMC_SHOW);
		m_prevMap = mapInfo;
	}
}
