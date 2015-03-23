// DetectorBindWizardChooseMapPage.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "DetectorBindWizardChooseMapPage.h"
#include "afxdialogex.h"


// CDetectorBindWizardChooseMapPage dialog

IMPLEMENT_DYNAMIC(CDetectorBindWizardChooseMapPage, CPropertyPage)

CDetectorBindWizardChooseMapPage::CDetectorBindWizardChooseMapPage()
	: CPropertyPage(CDetectorBindWizardChooseMapPage::IDD)
	, m_machine(NULL)
{

}

CDetectorBindWizardChooseMapPage::~CDetectorBindWizardChooseMapPage()
{
}

void CDetectorBindWizardChooseMapPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDetectorBindWizardChooseMapPage, CPropertyPage)
END_MESSAGE_MAP()


// CDetectorBindWizardChooseMapPage message handlers


BOOL CDetectorBindWizardChooseMapPage::OnSetActive()
{
	CPropertySheet* parent = reinterpret_cast<CPropertySheet*>(GetParent());
	parent->SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH | PSWIZB_CANCEL);

	return CPropertyPage::OnSetActive();
}


BOOL CDetectorBindWizardChooseMapPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();



	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
