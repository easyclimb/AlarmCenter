// ChooseMachineDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "ChooseMachineDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "AlarmMachineManager.h"

// CChooseMachineDlg dialog

IMPLEMENT_DYNAMIC(CChooseMachineDlg, CDialogEx)

CChooseMachineDlg::CChooseMachineDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CChooseMachineDlg::IDD, pParent)
	, m_ademco_id(0)
{

}

CChooseMachineDlg::~CChooseMachineDlg()
{
}

void CChooseMachineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_ADEMCO_ID, m_ademco_id);
	DDV_MinMaxInt(pDX, m_ademco_id, 0, 9999);
	DDX_Control(pDX, IDC_STATIC_NOTE, m_staticNote);
	DDX_Control(pDX, IDOK, m_btnOK);
}


BEGIN_MESSAGE_MAP(CChooseMachineDlg, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT_ADEMCO_ID, &CChooseMachineDlg::OnEnChangeEditAdemcoId)
	ON_BN_CLICKED(IDOK, &CChooseMachineDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CChooseMachineDlg message handlers


void CChooseMachineDlg::OnEnChangeEditAdemcoId()
{
	UpdateData();
	core::CAlarmMachinePtr machine = core::CAlarmMachineManager::GetInstance()->GetMachine(m_ademco_id);
	if (machine) {
		CString txt, fmAlias; fmAlias.LoadStringW(IDS_STRING_ALIAS);
		txt.Format(L"%s:%s", fmAlias, machine->get_alias());
		m_staticNote.SetWindowTextW(txt);
		m_btnOK.EnableWindow(1);
	} else {
		CString e; e.LoadStringW(IDS_STRING_INVALID_ADEMCO_ID);
		m_staticNote.SetWindowTextW(e);
		m_btnOK.EnableWindow(0);
	}
}


BOOL CChooseMachineDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_btnOK.EnableWindow(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CChooseMachineDlg::OnBnClickedOk()
{
	
	CDialogEx::OnOK();
}
