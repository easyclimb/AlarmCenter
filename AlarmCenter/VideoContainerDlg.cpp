// VideoContainerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "VideoContainerDlg.h"
#include "afxdialogex.h"


// CVideoContainerDlg dialog

IMPLEMENT_DYNAMIC(CVideoContainerDlg, CDialogEx)

CVideoContainerDlg::CVideoContainerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CVideoContainerDlg::IDD, pParent)
{

}

CVideoContainerDlg::~CVideoContainerDlg()
{
}

void CVideoContainerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CVideoContainerDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CVideoContainerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CVideoContainerDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CVideoContainerDlg message handlers


void CVideoContainerDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}


void CVideoContainerDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}
