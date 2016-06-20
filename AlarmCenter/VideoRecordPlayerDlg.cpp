// VideoRecordPlayerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "VideoRecordPlayerDlg.h"
#include "afxdialogex.h"


// CVideoRecordPlayerDlg dialog

IMPLEMENT_DYNAMIC(CVideoRecordPlayerDlg, CDialogEx)

CVideoRecordPlayerDlg::CVideoRecordPlayerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_VIDEO_RECORD_PLAYER, pParent)
{

}

CVideoRecordPlayerDlg::~CVideoRecordPlayerDlg()
{
}

void CVideoRecordPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CVideoRecordPlayerDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CVideoRecordPlayerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CVideoRecordPlayerDlg::OnBnClickedCancel)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CVideoRecordPlayerDlg message handlers


BOOL CVideoRecordPlayerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CenterWindow();

	SetWindowText(GetStringFromAppResource(IDS_DIALOG_VIDEO_RECORD_PLAYER));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CVideoRecordPlayerDlg::OnBnClickedOk()
{
	return;
}


void CVideoRecordPlayerDlg::OnBnClickedCancel()
{
	return;
}


void CVideoRecordPlayerDlg::OnClose()
{
	ShowWindow(SW_HIDE);
}
