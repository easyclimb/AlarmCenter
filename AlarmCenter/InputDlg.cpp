// InputDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "InputDlg.h"
#include "afxdialogex.h"


// CInputPasswdDlg dialog

IMPLEMENT_DYNAMIC(CInputPasswdDlg, CDialogEx)

CInputPasswdDlg::CInputPasswdDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CInputPasswdDlg::IDD, pParent)
	, m_title(L"")
	, m_edit(_T(""))
{

}

CInputPasswdDlg::~CInputPasswdDlg()
{
}

void CInputPasswdDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_edit);
	DDV_MaxChars(pDX, m_edit, 6);
	DDX_Control(pDX, IDC_EDIT1, m_edit_control);
}


BEGIN_MESSAGE_MAP(CInputPasswdDlg, CDialogEx)
END_MESSAGE_MAP()


// CInputPasswdDlg message handlers


BOOL CInputPasswdDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	if (m_title.IsEmpty()) {

	} else {
		SetWindowText(m_title);
	}

	m_edit_control.SetFocus();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
