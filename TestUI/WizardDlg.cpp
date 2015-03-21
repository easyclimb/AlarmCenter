// WizardDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TestUI.h"
#include "WizardDlg.h"
#include "afxdialogex.h"


// CWizardDlg dialog

IMPLEMENT_DYNAMIC(CWizardDlg, CPropertyPage)

CWizardDlg::CWizardDlg()
	: CPropertyPage(CWizardDlg::IDD)
{

}

CWizardDlg::~CWizardDlg()
{
}

void CWizardDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CWizardDlg, CPropertyPage)
END_MESSAGE_MAP()


// CWizardDlg message handlers
