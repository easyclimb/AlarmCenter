// InputGroupNameDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "InputGroupNameDlg.h"
#include "afxdialogex.h"


// CInputGroupNameDlg dialog

IMPLEMENT_DYNAMIC(CInputGroupNameDlg, CDialogEx)

CInputGroupNameDlg::CInputGroupNameDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CInputGroupNameDlg::IDD, pParent)
	, m_value(_T(""))
{

}

CInputGroupNameDlg::~CInputGroupNameDlg()
{
}

void CInputGroupNameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_value);
}


BEGIN_MESSAGE_MAP(CInputGroupNameDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CInputGroupNameDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CInputGroupNameDlg message handlers


void CInputGroupNameDlg::OnBnClickedOk()
{
	UpdateData();
	CDialogEx::OnOK();
}
