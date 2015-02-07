// InputDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "InputDlg.h"
#include "afxdialogex.h"


// CInputDlg dialog

IMPLEMENT_DYNAMIC(CInputDlg, CDialogEx)

CInputDlg::CInputDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CInputDlg::IDD, pParent)
	, m_edit(_T(""))
{

}

CInputDlg::~CInputDlg()
{
}

void CInputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_edit);
	DDV_MaxChars(pDX, m_edit, 6);
	DDX_Control(pDX, IDC_EDIT1, m_edit_control);
}


BEGIN_MESSAGE_MAP(CInputDlg, CDialogEx)
END_MESSAGE_MAP()


// CInputDlg message handlers


BOOL CInputDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_edit_control.SetFocus();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
