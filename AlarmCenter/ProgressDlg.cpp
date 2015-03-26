// ProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "ProgressDlg.h"
#include "afxdialogex.h"


// CProgressDlg dialog

IMPLEMENT_DYNAMIC(CProgressDlg, CDialogEx)

CProgressDlg::CProgressDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CProgressDlg::IDD, pParent)
{

}

CProgressDlg::~CProgressDlg()
{
}

void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
	DDX_Control(pDX, IDC_STATIC_NOTE, m_staticNote);
}


BEGIN_MESSAGE_MAP(CProgressDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CProgressDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CProgressDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CProgressDlg message handlers


void CProgressDlg::OnBnClickedOk()
{
	return;
	//CDialogEx::OnOK();
}


void CProgressDlg::OnBnClickedCancel()
{
	return;
	//CDialogEx::OnCancel();
}


BOOL CProgressDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CenterWindow(GetParent());

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
