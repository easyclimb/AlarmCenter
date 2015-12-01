// CameraBindWizard.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "CameraBindWizard.h"


// CCameraBindWizard

IMPLEMENT_DYNAMIC(CCameraBindWizard, CPropertySheet)

CCameraBindWizard::CCameraBindWizard(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	MyConstruct();
}

CCameraBindWizard::CCameraBindWizard(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	MyConstruct();
}

CCameraBindWizard::~CCameraBindWizard()
{
}



BEGIN_MESSAGE_MAP(CCameraBindWizard, CPropertySheet)
END_MESSAGE_MAP()


// CCameraBindWizard message handlers
void CCameraBindWizard::MyConstruct()
{
	AddPage(&m_pageChooseCamera);
	AddPage(&m_pageChooseMap);
	SetActivePage(0);
	SetWizardMode();
}


BOOL CCameraBindWizard::OnInitDialog()
{
	BOOL bResult = CPropertySheet::OnInitDialog();

	CWnd *pWnd = GetDlgItem(IDHELP);
	pWnd->ShowWindow(FALSE);


	return bResult;
}
