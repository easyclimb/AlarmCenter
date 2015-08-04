// InputVerifyCodeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TestHikvision.h"
#include "InputVerifyCodeDlg.h"
#include "afxdialogex.h"


// CInputVerifyCodeDlg dialog

IMPLEMENT_DYNAMIC(CInputVerifyCodeDlg, CDialogEx)

CInputVerifyCodeDlg::CInputVerifyCodeDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CInputVerifyCodeDlg::IDD, pParent)
{

}

CInputVerifyCodeDlg::~CInputVerifyCodeDlg()
{
}

void CInputVerifyCodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_code);
}


BEGIN_MESSAGE_MAP(CInputVerifyCodeDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CInputVerifyCodeDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CInputVerifyCodeDlg message handlers


void CInputVerifyCodeDlg::OnBnClickedOk()
{
	m_code.GetWindowTextW(m_result);
	CDialogEx::OnOK();
}
