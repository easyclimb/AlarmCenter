// SearchMachineResultDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "SearchMachineResultDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "AlarmMachineManager.h"


// CSearchMachineResultDlg dialog

IMPLEMENT_DYNAMIC(CSearchMachineResultDlg, CDialogEx)

CSearchMachineResultDlg::CSearchMachineResultDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_SEARCH_MACHINE_RESULT, pParent)
{

}

CSearchMachineResultDlg::~CSearchMachineResultDlg()
{
}

void CSearchMachineResultDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_input);
	DDX_Control(pDX, IDC_TAB1, m_tab);
	DDX_Control(pDX, IDC_LIST1, m_list);
}


BEGIN_MESSAGE_MAP(CSearchMachineResultDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CSearchMachineResultDlg::OnBnClickedOk)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDCANCEL, &CSearchMachineResultDlg::OnBnClickedCancel)
	ON_WM_TIMER()
	ON_WM_GETMINMAXINFO()
	ON_WM_SHOWWINDOW()
	ON_EN_CHANGE(IDC_EDIT1, &CSearchMachineResultDlg::OnEnChangeEdit1)
END_MESSAGE_MAP()


// CSearchMachineResultDlg message handlers


void CSearchMachineResultDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}

void CSearchMachineResultDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}

void CSearchMachineResultDlg::OnClose()
{
	ShowWindow(SW_HIDE);

	//CDialogEx::OnClose();
}

BOOL CSearchMachineResultDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CenterWindow();

	SetWindowTextW(TR(IDS_STRING_SEARCH_MACHINE));

	auto icon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_SEARCH));
	SetIcon(icon, TRUE);

	container_ = std::shared_ptr<CAlarmMachineContainerDlg>(new CAlarmMachineContainerDlg(this), [](CDialogEx*p) {SAFEDELETEDLG(p); });
	container_->Create(IDD_DIALOG_CONTAINER, this);

	CRect rc;
	m_input.GetWindowRect(rc);
	int h = rc.Height();
	GetClientRect(rc);
	rc.top += h + 5;

	container_->MoveWindow(rc);
	container_->ShowWindow(SW_SHOW);

	//SetTimer(1, 2000, nullptr);

	init_over_ = true;

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CSearchMachineResultDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (!init_over_) {
		return;
	}

	CRect rc;
	m_input.GetWindowRect(rc);
	int h = rc.Height();
	GetClientRect(rc);
	int b = rc.bottom;
	rc.bottom = rc.top + h;
	m_input.MoveWindow(rc);
	rc.top = rc.bottom + 5;
	rc.bottom = b;
	container_->MoveWindow(rc);
}


void CSearchMachineResultDlg::OnTimer(UINT_PTR nIDEvent)
{
	//auto_timer t(m_hWnd, 1, 2000);

	CString txt;
	m_input.GetWindowTextW(txt);

	if (last_input_content_ != (LPCTSTR)txt) {
		container_->ClearButtonList();
		last_input_content_ = (LPCTSTR)txt;

		if (!last_input_content_.empty()) {
			auto mgr = core::alarm_machine_manager::get_instance();
			auto list = mgr->fuzzy_search_machine(last_input_content_, 10);
			for (auto id : list) {
				auto machine = mgr->GetMachine(id);
				if (machine) {
					container_->InsertMachine(machine, -1, false);
				}
			}
		}
	}

	m_input.SetFocus();
	m_input.SetSel((DWORD)-1);

	CDialogEx::OnTimer(nIDEvent);
}


void CSearchMachineResultDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize.x = 600;
	lpMMI->ptMinTrackSize.y = 400;

	CDialogEx::OnGetMinMaxInfo(lpMMI);
}


void CSearchMachineResultDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	if (bShow) {
		auto_timer t(m_hWnd, 1, 2000);
	} else {
		KillTimer(1);
	}
}


void CSearchMachineResultDlg::OnEnChangeEdit1()
{
	auto_timer t(m_hWnd, 1, 2000);
}
