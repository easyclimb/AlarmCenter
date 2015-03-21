// DetectorBindWizardChooseDetPage.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "DetectorBindWizardChooseDetPage.h"
#include "afxdialogex.h"


// CDetectorBindWizardChooseDetPage dialog

IMPLEMENT_DYNAMIC(CDetectorBindWizardChooseDetPage, CPropertyPage)

CDetectorBindWizardChooseDetPage::CDetectorBindWizardChooseDetPage()
	: CPropertyPage(CDetectorBindWizardChooseDetPage::IDD)
{

}

CDetectorBindWizardChooseDetPage::~CDetectorBindWizardChooseDetPage()
{
}

void CDetectorBindWizardChooseDetPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDetectorBindWizardChooseDetPage, CPropertyPage)
END_MESSAGE_MAP()


// CDetectorBindWizardChooseDetPage message handlers


BOOL CDetectorBindWizardChooseDetPage::OnSetActive()
{
	CPropertySheet* parent = reinterpret_cast<CPropertySheet*>(GetParent());
	parent->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT | PSWIZB_CANCEL);

	return CPropertyPage::OnSetActive();
}
