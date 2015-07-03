// ChangePswDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmMachineConfigTool.h"
#include "ChangePswDlg.h"
#include "afxdialogex.h"


// CChangePswDlg dialog

IMPLEMENT_DYNAMIC(CChangePswDlg, CDialogEx)

CChangePswDlg::CChangePswDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CChangePswDlg::IDD, pParent)
	, m_title(L"修改密码")
{

}

CChangePswDlg::~CChangePswDlg()
{
}

void CChangePswDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_psw_o);
	DDX_Control(pDX, IDC_EDIT2, m_psw_n);
	DDX_Control(pDX, IDC_EDIT3, m_psw_r);
}


BEGIN_MESSAGE_MAP(CChangePswDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CChangePswDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CChangePswDlg message handlers


void CChangePswDlg::OnBnClickedOk()
{
	//CString psw_o, psw_n, psw_r;
	m_psw_o.GetWindowTextW(psw_o);
	m_psw_n.GetWindowTextW(psw_n); 
	m_psw_r.GetWindowTextW(psw_r);
	if (psw_o.IsEmpty() || psw_n.IsEmpty() || psw_r.IsEmpty()) {
		MessageBox(L"不能为空!");
		return;
	}

	if (psw_o.GetLength() != 6 || psw_n.GetLength() != 6 || psw_r.GetLength() != 6) {
		MessageBox(L"密码长度必须为6!");
		return;
	}

	if (psw_n.Compare(psw_r) != 0) {
		MessageBox(L"两次输入不一致!");
		return;
	}

	CDialogEx::OnOK();
}


BOOL CChangePswDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText(m_title);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
