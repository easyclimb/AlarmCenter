// DetectorBindWizrd.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "DetectorBindWizrd.h"
#include "AlarmMachine.h"

using namespace core;
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
	m_machine = NULL;
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
	m_pageChooseZone.m_machine = m_machine;
	m_pageChooseDet.m_machine = m_machine;
	m_pageChooseMap.m_machine = m_machine;

	CWnd *pWnd = GetDlgItem(IDHELP);
	pWnd->ShowWindow(FALSE);

	ASSERT(m_machine);

	return bResult;
}
