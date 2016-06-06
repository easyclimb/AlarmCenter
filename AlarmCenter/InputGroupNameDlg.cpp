// InputGroupNameDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AppResource.h"
#include "InputGroupNameDlg.h"
#include "afxdialogex.h"


// CInputContentDlg dialog

IMPLEMENT_DYNAMIC(CInputContentDlg, CDialogEx)

CInputContentDlg::CInputContentDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CInputContentDlg::IDD, pParent)
	, m_value(_T(""))
{

}

CInputContentDlg::~CInputContentDlg()
{
}

void CInputContentDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_value);
}


BEGIN_MESSAGE_MAP(CInputContentDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CInputContentDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CInputContentDlg message handlers


void CInputContentDlg::OnBnClickedOk()
{
	UpdateData();
	CDialogEx::OnOK();
}


BOOL CInputContentDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	if (!m_title.IsEmpty()) {
		SetWindowText(m_title);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}
