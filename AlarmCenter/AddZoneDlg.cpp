// AddZoneDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AddZoneDlg.h"
#include "afxdialogex.h"


// CAddZoneDlg dialog

IMPLEMENT_DYNAMIC(CAddZoneDlg, CDialogEx)

CAddZoneDlg::CAddZoneDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAddZoneDlg::IDD, pParent)
	, m_value(0)
{

}

CAddZoneDlg::~CAddZoneDlg()
{
}

void CAddZoneDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_value);
	DDV_MinMaxInt(pDX, m_value, 1, 1000);
}


BEGIN_MESSAGE_MAP(CAddZoneDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAddZoneDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CAddZoneDlg message handlers


void CAddZoneDlg::OnBnClickedOk()
{
	UpdateData();
	CDialogEx::OnOK();
}
