// DestroyProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "DestroyProgressDlg.h"
#include "afxdialogex.h"


// CDestroyProgressDlg dialog

IMPLEMENT_DYNAMIC(CDestroyProgressDlg, CDialogEx)

CDestroyProgressDlg::CDestroyProgressDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CDestroyProgressDlg::IDD, pParent)
{

}

CDestroyProgressDlg::~CDestroyProgressDlg()
{
}

void CDestroyProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
}


BEGIN_MESSAGE_MAP(CDestroyProgressDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDestroyProgressDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDestroyProgressDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CDestroyProgressDlg message handlers


void CDestroyProgressDlg::OnBnClickedOk()
{
	return;
	//CDialogEx::OnOK();
}


void CDestroyProgressDlg::OnBnClickedCancel()
{
	return;
	//CDialogEx::OnCancel();
}
