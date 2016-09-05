// AlarmHandleStep3.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AlarmHandleStep3Dlg.h"
#include "afxdialogex.h"
#include "alarm_handle_mgr.h"

// CAlarmHandleStep3 dialog

IMPLEMENT_DYNAMIC(CAlarmHandleStep3Dlg, CDialogEx)

CAlarmHandleStep3Dlg::CAlarmHandleStep3Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_ALARM_HANDLE_3, pParent)
{

}

CAlarmHandleStep3Dlg::~CAlarmHandleStep3Dlg()
{
}

void CAlarmHandleStep3Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CUSTOM1, m_list);
}


BEGIN_MESSAGE_MAP(CAlarmHandleStep3Dlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_ADD_GUARD, &CAlarmHandleStep3Dlg::OnBnClickedButtonAddGuard)
	ON_BN_CLICKED(IDC_BUTTON_RM_GUARD, &CAlarmHandleStep3Dlg::OnBnClickedButtonRmGuard)
END_MESSAGE_MAP()


// CAlarmHandleStep3Dlg message handlers

BOOL CAlarmHandleStep3Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowTextW(TR(IDS_STRING_ALARM_HANDLE_3));
	SET_WINDOW_TEXT(IDC_BUTTON_ADD_GUARD, IDS_STRING_ADD_SECURITY_GUARD);
	SET_WINDOW_TEXT(IDC_BUTTON_RM_GUARD, IDS_STRING_REMOVE_SECURITY_GUARD);
	SET_WINDOW_TEXT(IDOK, IDS_OK);
	SET_WINDOW_TEXT(IDCANCEL, IDS_CANCEL);


	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CAlarmHandleStep3Dlg::OnBnClickedButtonAddGuard()
{
	// TODO: Add your control notification handler code here
}


void CAlarmHandleStep3Dlg::OnBnClickedButtonRmGuard()
{
	// TODO: Add your control notification handler code here
}


