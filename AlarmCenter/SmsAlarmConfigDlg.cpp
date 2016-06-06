// SmsAlarmConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AppResource.h"
#include "SmsAlarmConfigDlg.h"
#include "afxdialogex.h"


// CSmsAlarmConfigDlg dialog

IMPLEMENT_DYNAMIC(CSmsAlarmConfigDlg, CDialogEx)

CSmsAlarmConfigDlg::CSmsAlarmConfigDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CSmsAlarmConfigDlg::IDD, pParent)
{

}

CSmsAlarmConfigDlg::~CSmsAlarmConfigDlg()
{
}

void CSmsAlarmConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_testSms);
}


BEGIN_MESSAGE_MAP(CSmsAlarmConfigDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK1, &CSmsAlarmConfigDlg::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_CHECK2, &CSmsAlarmConfigDlg::OnBnClickedCheck2)
	ON_BN_CLICKED(IDC_CHECK3, &CSmsAlarmConfigDlg::OnBnClickedCheck3)
	ON_BN_CLICKED(IDC_BUTTON1, &CSmsAlarmConfigDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CSmsAlarmConfigDlg message handlers


void CSmsAlarmConfigDlg::OnBnClickedCheck1()
{
}


void CSmsAlarmConfigDlg::OnBnClickedCheck2()
{
}


void CSmsAlarmConfigDlg::OnBnClickedCheck3()
{
}


void CSmsAlarmConfigDlg::OnBnClickedButton1()
{
}


BOOL CSmsAlarmConfigDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();



	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
