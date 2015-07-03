// InputPswDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmMachineConfigTool.h"
#include "InputPswDlg.h"
#include "afxdialogex.h"


// CInputPswDlg dialog

IMPLEMENT_DYNAMIC(CInputPswDlg, CDialogEx)

CInputPswDlg::CInputPswDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CInputPswDlg::IDD, pParent)
	, m_strPsw(L"")
{

}

CInputPswDlg::~CInputPswDlg()
{
}

void CInputPswDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_psw);
}


BEGIN_MESSAGE_MAP(CInputPswDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CInputPswDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CInputPswDlg message handlers


void CInputPswDlg::OnBnClickedOk()
{
	m_psw.GetWindowText(m_strPsw);
	while (m_strPsw.GetLength() < 6) {
		m_strPsw = L"0" + m_strPsw;
	}
	CDialogEx::OnOK();
}
