// DetectorBindWizardChooseZonePage.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "DetectorBindWizardChooseZonePage.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "ZoneInfo.h"
#include "EditZoneDlg.h"

using namespace core;

// CDetectorBindWizardChooseZonePage dialog

IMPLEMENT_DYNAMIC(CDetectorBindWizardChooseZonePage, CPropertyPage)

CDetectorBindWizardChooseZonePage::CDetectorBindWizardChooseZonePage()
	: CPropertyPage(CDetectorBindWizardChooseZonePage::IDD)
	, m_zoneValue(-1)
	, m_machine(NULL)
{

}

CDetectorBindWizardChooseZonePage::~CDetectorBindWizardChooseZonePage()
{
}

void CDetectorBindWizardChooseZonePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
}


BEGIN_MESSAGE_MAP(CDetectorBindWizardChooseZonePage, CPropertyPage)
	ON_LBN_SELCHANGE(IDC_LIST1, &CDetectorBindWizardChooseZonePage::OnLbnSelchangeList1)
END_MESSAGE_MAP()


// CDetectorBindWizardChooseZonePage message handlers


BOOL CDetectorBindWizardChooseZonePage::OnSetActive()
{
	CPropertySheet* parent = reinterpret_cast<CPropertySheet*>(GetParent());
	parent->SetWizardButtons(PSWIZB_NEXT | PSWIZB_CANCEL);

	m_list.ResetContent();
	m_list.SetItemHeight(-1, 20);
	//m_list.SetFont(&m_font);
	int ndx = 0, prev_ndx = 0;
	CString txt;
	CZoneInfoList list;
	m_machine->GetAllZoneInfo(list);
	for (auto zoneInfo : list) {
		if (NULL == zoneInfo->GetDetectorInfo() && NULL == zoneInfo->GetSubMachineInfo()) {
			CEditZoneDlg::FormatZoneInfoText(m_machine, zoneInfo, txt);
			ndx = m_list.InsertString(ndx, txt);
			bool bSubZone = zoneInfo->get_type() == ZT_SUB_MACHINE_ZONE;
			int data = bSubZone ? zoneInfo->get_sub_zone() : zoneInfo->get_zone_value();
			m_list.SetItemData(ndx, data);
			if (m_zoneValue == data) {
				prev_ndx = ndx;
			}
			ndx++;
		}
	}
	m_list.SetCurSel(prev_ndx);
	OnLbnSelchangeList1();
	return CPropertyPage::OnSetActive();
}


void CDetectorBindWizardChooseZonePage::OnLbnSelchangeList1()
{
	int ndx = m_list.GetCurSel();
	if (ndx < 0) return;
	m_zoneValue = m_list.GetItemData(ndx);
}


BOOL CDetectorBindWizardChooseZonePage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_font.CreateFontW(32, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
					   DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
					   DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS,
					   L"Arial");

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
