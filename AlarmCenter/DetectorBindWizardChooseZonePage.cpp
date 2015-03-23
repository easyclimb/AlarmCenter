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

	int ndx = 0;
	CString txt;
	CZoneInfoList list;
	m_machine->GetAllZoneInfo(list);
	CZoneInfoListIter iter = list.begin();
	while (iter != list.end()) {
		CZoneInfo* zoneInfo = *iter++;
		if (NULL == zoneInfo->GetDetectorInfo()) {
			CEditZoneDlg::FormatZoneInfoText(m_machine, zoneInfo, txt);
			ndx = m_list.InsertString(ndx, txt);
			m_list.SetItemData(ndx, zoneInfo->get_zone_value());
			ndx++;
		}
	}


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
