// PickMachineCoordinateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "PickMachineCoordinateDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "BaiduMapDlg.h"
#include "CsrInfo.h"
#include "UserInfo.h"

using namespace core;
// CPickMachineCoordinateDlg dialog

IMPLEMENT_DYNAMIC(CPickMachineCoordinateDlg, CDialogEx)

CPickMachineCoordinateDlg::CPickMachineCoordinateDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPickMachineCoordinateDlg::IDD, pParent)
	, m_machine(NULL)
	, m_map(NULL)
{

}

CPickMachineCoordinateDlg::~CPickMachineCoordinateDlg()
{
}

void CPickMachineCoordinateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_AUTO_LOCATE, m_btnAutoLocate);
}


BEGIN_MESSAGE_MAP(CPickMachineCoordinateDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CPickMachineCoordinateDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_LOCATE, &CPickMachineCoordinateDlg::OnBnClickedButtonAutoLocate)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_SET_PT, &CPickMachineCoordinateDlg::OnBnClickedButtonSetPt)
	ON_MESSAGE(WM_CHOSEN_BAIDU_PT, &CPickMachineCoordinateDlg::OnChosenBaiduPt)
	ON_BN_CLICKED(IDC_BUTTON_SHOW_PATH, &CPickMachineCoordinateDlg::OnBnClickedButtonShowPath)
END_MESSAGE_MAP()


// CPickMachineCoordinateDlg message handlers


void CPickMachineCoordinateDlg::OnBnClickedOk()
{

	CDialogEx::OnOK();
}
// m_btnAutoLocate
static void __stdcall OnCurUserChanged(void* udata, const core::CUserInfo* user)
{
	if (!udata || !user)
		return;

	CPickMachineCoordinateDlg* dlg = reinterpret_cast<CPickMachineCoordinateDlg*>(udata);
	if (user->get_user_priority() == core::UP_OPERATOR) {
		dlg->m_btnAutoLocate.EnableWindow(0);
	} else {
		dlg->m_btnAutoLocate.EnableWindow(1);
	}
}

BOOL CPickMachineCoordinateDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	assert(m_machine);

	core::CUserManager::GetInstance()->RegisterObserver(this, OnCurUserChanged);
	OnCurUserChanged(this, core::CUserManager::GetInstance()->GetCurUserInfo());

	m_map = new CBaiduMapDlg(this);
	m_map->m_pRealParent = this;
	m_map->Create(IDD_DIALOG_BAIDU_MAP, this);
	CRect rc;
	GetClientRect(rc);
	rc.DeflateRect(0, 25, 0, 0);
	m_map->MoveWindow(rc);
	m_map->ShowWindow(SW_SHOW);

	web::BaiduCoordinate coor = m_machine->get_coor();
	if (coor.x == 0. && coor.y == 0.) {
		OnBnClickedButtonAutoLocate();
	} else {
		std::wstring  url = GetModuleFilePath();
		url += L"\\baidu.html";
		CString title, smachine; smachine.LoadStringW(IDS_STRING_MACHINE);
		title.Format(L"%s%04d(%s)", smachine, m_machine->get_ademco_id(), m_machine->get_alias());
		m_map->ShowCoordinate(coor, title);
		/*if (m_map->GenerateHtml(url, coor, title)) {
			m_map->Navigate(url.c_str());
		}*/
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CPickMachineCoordinateDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	SAFEDELETEDLG(m_map);
	core::CUserManager::GetInstance()->UnRegisterObserver(this);
}


void CPickMachineCoordinateDlg::OnBnClickedButtonAutoLocate()
{
	std::wstring addr;
	int city_code;
	web::BaiduCoordinate coor;
	if (web::CBaiduService::GetInstance()->locate(addr, city_code, coor)) {
		m_machine->execute_set_coor(coor);
		std::wstring  url = GetModuleFilePath();
		url += L"\\baidu.html";
		CString title, smachine; smachine.LoadStringW(IDS_STRING_MACHINE);
		title.Format(L"%s%04d(%s)", smachine, m_machine->get_ademco_id(), m_machine->get_alias());
		m_map->ShowCoordinate(coor, title);
	} else {
		CString e; e.LoadStringW(IDS_STRING_E_AUTO_LACATE_FAILED);
		MessageBox(e, L"", MB_ICONERROR);
	}
}


void CPickMachineCoordinateDlg::OnBnClickedButtonSetPt()
{
	
}


afx_msg LRESULT CPickMachineCoordinateDlg::OnChosenBaiduPt(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	AUTO_LOG_FUNCTION;
	web::BaiduCoordinate coor = m_map->m_coor;
	if (!(coor == m_machine->get_coor())) {
		if (m_machine->execute_set_coor(coor)) {
			LOG(L"succeed.\n");
		}
	}
	return 0;
}


void CPickMachineCoordinateDlg::OnBnClickedButtonShowPath()
{
	web::BaiduCoordinate coor_csr = CCsrInfo::GetInstance()->get_coor();
	web::BaiduCoordinate coor_cli = m_machine->get_coor();
	CString scsr; scsr.LoadStringW(IDS_STRING_ALARM_CENTER);
	std::wstring csr = scsr.LockBuffer();
	scsr.UnlockBuffer();
	CString sdst, smachine; smachine.LoadStringW(IDS_STRING_MACHINE);
	sdst.Format(L"%s%04d(%s)", smachine, m_machine->get_ademco_id(), 
				m_machine->get_alias());
	std::wstring dst = sdst.LockBuffer();
	sdst.UnlockBuffer();
	m_map->ShowDrivingRoute(coor_csr, coor_cli, csr, dst);
}
