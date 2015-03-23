// DetectorBindWizrd.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "DetectorBindWizrd.h"

// CDetectorBindWizrd

IMPLEMENT_DYNAMIC(CDetectorBindWizrd, CPropertySheet)

CDetectorBindWizrd::CDetectorBindWizrd(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	MyConstruct();
}

CDetectorBindWizrd::CDetectorBindWizrd(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	MyConstruct();
}

CDetectorBindWizrd::~CDetectorBindWizrd()
{
	
}

void CDetectorBindWizrd::MyConstruct()
{
	AddPage(&m_pageChooseZone);
	AddPage(&m_pageChooseDet);
	AddPage(&m_pageChooseMap);
	SetActivePage(0);
	SetWizardMode();
}


BEGIN_MESSAGE_MAP(CDetectorBindWizrd, CPropertySheet)
END_MESSAGE_MAP()


// CDetectorBindWizrd message handlers


BOOL CDetectorBindWizrd::OnInitDialog()
{
	BOOL bResult = CPropertySheet::OnInitDialog();

	CWnd *pWnd = GetDlgItem(IDHELP);
	pWnd->ShowWindow(FALSE);


	return bResult;
}
