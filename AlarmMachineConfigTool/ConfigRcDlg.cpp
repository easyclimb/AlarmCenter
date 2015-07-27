// ConfigRcDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmMachineConfigTool.h"
#include "ConfigRcDlg.h"
#include "afxdialogex.h"


// CConfigRcDlg dialog

IMPLEMENT_DYNAMIC(CConfigRcDlg, CDialogEx)

CConfigRcDlg::CConfigRcDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CConfigRcDlg::IDD, pParent)
{

}

CConfigRcDlg::~CConfigRcDlg()
{
}

void CConfigRcDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CConfigRcDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CConfigRcDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_DUIMA, &CConfigRcDlg::OnBnClickedButtonDuima)
	ON_BN_CLICKED(IDC_BUTTON_DEL, &CConfigRcDlg::OnBnClickedButtonDel)
	ON_BN_CLICKED(IDC_BUTTON_DUIMA2, &CConfigRcDlg::OnBnClickedButtonDuima2)
	ON_BN_CLICKED(IDC_BUTTON_DEL2, &CConfigRcDlg::OnBnClickedButtonDel2)
	ON_BN_CLICKED(IDC_BUTTON_DUIMA3, &CConfigRcDlg::OnBnClickedButtonDuima3)
	ON_BN_CLICKED(IDC_BUTTON_DEL3, &CConfigRcDlg::OnBnClickedButtonDel3)
	ON_BN_CLICKED(IDC_BUTTON_DUIMA4, &CConfigRcDlg::OnBnClickedButtonDuima4)
	ON_BN_CLICKED(IDC_BUTTON_DEL4, &CConfigRcDlg::OnBnClickedButtonDel4)
	ON_BN_CLICKED(IDC_BUTTON_DUIMA5, &CConfigRcDlg::OnBnClickedButtonDuima5)
	ON_BN_CLICKED(IDC_BUTTON_DEL5, &CConfigRcDlg::OnBnClickedButtonDel5)
	ON_BN_CLICKED(IDC_BUTTON_DEL_ALL, &CConfigRcDlg::OnBnClickedButtonDelAll)
END_MESSAGE_MAP()


// CConfigRcDlg message handlers


void CConfigRcDlg::OnBnClickedOk()
{
}


void CConfigRcDlg::OnBnClickedButtonDuima()
{

}


void CConfigRcDlg::OnBnClickedButtonDel()
{

}


void CConfigRcDlg::OnBnClickedButtonDuima2()
{

}


void CConfigRcDlg::OnBnClickedButtonDel2()
{

}


void CConfigRcDlg::OnBnClickedButtonDuima3()
{

}


void CConfigRcDlg::OnBnClickedButtonDel3()
{

}


void CConfigRcDlg::OnBnClickedButtonDuima4()
{

}


void CConfigRcDlg::OnBnClickedButtonDel4()
{

}


void CConfigRcDlg::OnBnClickedButtonDuima5()
{

}


void CConfigRcDlg::OnBnClickedButtonDel5()
{

}


void CConfigRcDlg::OnBnClickedButtonDelAll()
{

}
