// DetectorBindWizardChooseZonePage.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "DetectorBindWizardChooseZonePage.h"
#include "afxdialogex.h"


// CDetectorBindWizardChooseZonePage dialog

IMPLEMENT_DYNAMIC(CDetectorBindWizardChooseZonePage, CPropertyPage)

CDetectorBindWizardChooseZonePage::CDetectorBindWizardChooseZonePage()
	: CPropertyPage(CDetectorBindWizardChooseZonePage::IDD)
{

}

CDetectorBindWizardChooseZonePage::~CDetectorBindWizardChooseZonePage()
{
}

void CDetectorBindWizardChooseZonePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDetectorBindWizardChooseZonePage, CPropertyPage)
END_MESSAGE_MAP()


// CDetectorBindWizardChooseZonePage message handlers


BOOL CDetectorBindWizardChooseZonePage::OnSetActive()
{
	CPropertySheet* parent = reinterpret_cast<CPropertySheet*>(GetParent());
	parent->SetWizardButtons(PSWIZB_NEXT | PSWIZB_CANCEL);
	return CPropertyPage::OnSetActive();
}
