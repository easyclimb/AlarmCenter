// ChooseMachineDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "ChooseMachineDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "AlarmMachineManager.h"
#include "ZoneInfo.h"

// CChooseMachineDlg dialog

IMPLEMENT_DYNAMIC(CChooseMachineDlg, CDialogEx)

CChooseMachineDlg::CChooseMachineDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CChooseMachineDlg::IDD, pParent)
	//, m_ademco_id(0)
{

}

CChooseMachineDlg::~CChooseMachineDlg()
{
}

void CChooseMachineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//DDX_Text(pDX, IDC_EDIT_ADEMCO_ID, m_ademco_id);
	//DDV_MinMaxInt(pDX, m_ademco_id, 0, 9999);
	DDX_Control(pDX, IDC_STATIC_NOTE, m_staticNote);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_STATIC_MACHINE_ID, m_static_input_id);
	DDX_Control(pDX, IDC_EDIT_ADEMCO_ID, m_edit);
}


BEGIN_MESSAGE_MAP(CChooseMachineDlg, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT_ADEMCO_ID, &CChooseMachineDlg::OnEnChangeEditAdemcoId)
	ON_BN_CLICKED(IDOK, &CChooseMachineDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CChooseMachineDlg message handlers


void CChooseMachineDlg::OnEnChangeEditAdemcoId()
{
	UpdateData();
	do {

		if (!choosing_sub_machine_) {
			CString txt;
			m_edit.GetWindowTextW(txt);
			m_ademco_id = _ttoi(txt);
		}

		core::alarm_machine_ptr machine = core::alarm_machine_manager::get_instance()->GetMachine(m_ademco_id);
		if (!machine) {
			break;
		}

		if (choosing_sub_machine_) {
			CString txt;
			m_edit.GetWindowTextW(txt);
			m_zone_value = _ttoi(txt);

			auto zone = machine->GetZone(m_zone_value);
			if (!zone) {
				break;
			}

			machine = zone->GetSubMachineInfo();
			if (!machine) {
				break;
			}	
		}

		//CString txt, fmAlias; fmAlias = TR(IDS_STRING_ALIAS);
		//txt.Format(L"%s:%s", fmAlias, machine->get_machine_name());
		m_staticNote.SetWindowTextW(machine->get_formatted_name());
		m_btnOK.EnableWindow(1);

		return;
	} while (false);
	
	CString e; 
	e = TR(choosing_sub_machine_ ? IDS_STRING_INVALID_ZONE_VALUE : IDS_STRING_INVALID_ADEMCO_ID);
	m_staticNote.SetWindowTextW(e);
	m_btnOK.EnableWindow(0);
}


BOOL CChooseMachineDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_static_input_id.SetWindowTextW(TR(choosing_sub_machine_ ? IDS_STRING_INPUT_SUBMACHINE_ID : IDS_STRING_INPUT_MACHINE_ID));

	m_btnOK.EnableWindow(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CChooseMachineDlg::OnBnClickedOk()
{
	
	CDialogEx::OnOK();
}
