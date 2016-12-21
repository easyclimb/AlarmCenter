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
	DDX_Control(pDX, IDC_BUTTON1, m_btn_search);
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
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CSearchMachineResultDlg::OnTcnSelchangeTab1)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CSearchMachineResultDlg::OnNMDblclkList1)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_CHAR()
	ON_BN_CLICKED(IDC_BUTTON1, &CSearchMachineResultDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CSearchMachineResultDlg message handlers


void CSearchMachineResultDlg::reposition_items()
{
	CRect rc;
	m_input.GetWindowRect(rc);
	int h = rc.Height();
	GetClientRect(rc);
	int b = rc.bottom;
	rc.bottom = rc.top + h;
	{
		CRect rc_btn;
		m_btn_search.GetWindowRect(rc_btn);
		int w = rc_btn.Width();
		rc.right -= w + 10;
		m_input.MoveWindow(rc);

		rc_btn.left = rc.right + 5;
		rc_btn.top = rc.top;
		rc_btn.right = rc.right + w + 5;
		rc_btn.bottom = rc.bottom;
		m_btn_search.MoveWindow(rc_btn);

		rc.right += w + 10;
	}
	
	rc.top = rc.bottom + 5;
	rc.bottom = b;

	m_tab.MoveWindow(rc);
	rc.DeflateRect(5, 25, 5, 5);
	m_list.MoveWindow(rc);

	m_tab.GetClientRect(rc);
	rc.DeflateRect(5, 25, 5, 5);
	container_->MoveWindow(rc);
	
	
}

void CSearchMachineResultDlg::init_list_headers()
{
	DWORD dwStyle = m_list.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
	m_list.SetExtendedStyle(dwStyle);

	int i = -1;
	CString fm;
	fm = TR(IDS_STRING_MACHINE);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 50, -1);
	fm = TR(IDS_STRING_ALIAS);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 200, -1);
	fm = TR(IDS_STRING_CONTACT);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 100, -1);
	fm = TR(IDS_STRING_ADDRESS);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 225, -1);
	fm = TR(IDS_STRING_PHONE);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 150, -1);
	fm = TR(IDS_STRING_PHONE_BK);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 150, -1);
}

void CSearchMachineResultDlg::insert_list_content(const core::alarm_machine_ptr & machine)
{
	int nResult = -1;
	LV_ITEM lvitem = { 0 };
	CString tmp = _T("");

	lvitem.lParam = 0;
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = m_list.GetItemCount();
	lvitem.iSubItem = 0;

	// aid
	tmp.Format(_T("%06d"), machine->get_ademco_id());
	lvitem.pszText = tmp.LockBuffer();
	nResult = m_list.InsertItem(&lvitem);
	tmp.UnlockBuffer();

	if (nResult != -1) {
		// alias
		lvitem.iItem = nResult;
		lvitem.iSubItem++;
		tmp = machine->get_machine_name();
		lvitem.pszText = tmp.LockBuffer();
		m_list.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// contact
		lvitem.iSubItem++;
		tmp = machine->get_contact();
		lvitem.pszText = tmp.LockBuffer();
		m_list.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// address
		lvitem.iSubItem++;
		tmp = machine->get_address();
		lvitem.pszText = tmp.LockBuffer();
		m_list.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// phone
		lvitem.iSubItem++;
		tmp = machine->get_phone();
		lvitem.pszText = tmp.LockBuffer();
		m_list.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// phone_bk
		lvitem.iSubItem++;
		tmp = machine->get_phone_bk();
		lvitem.pszText = tmp.LockBuffer();
		m_list.SetItem(&lvitem);
		tmp.UnlockBuffer();

		m_list.SetItemData(nResult, machine->get_ademco_id());
	}
}

void CSearchMachineResultDlg::do_search()
{
	CString txt;
	m_input.GetWindowTextW(txt);

	if (last_input_content_ != (LPCTSTR)txt) {
		container_->ClearButtonList();
		m_list.DeleteAllItems();

		last_input_content_ = (LPCTSTR)txt;

		if (!last_input_content_.empty()) {
			auto mgr = core::alarm_machine_manager::get_instance();
			auto list = mgr->fuzzy_search_machine(last_input_content_, 10);
			for (auto id : list) {
				auto machine = mgr->GetMachine(id);
				if (machine) {
					container_->InsertMachine(machine, -1, false);
					insert_list_content(machine);
				}
			}
		}
	}

	if (got_focus_) {
		m_input.SetFocus();
		m_input.SetSel((DWORD)-1);
	}
}

void CSearchMachineResultDlg::OnBnClickedOk()
{
	return;
	//CDialogEx::OnOK();
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
	m_btn_search.SetWindowTextW(TR(IDS_STRING_SEARCH_MACHINE));

	auto icon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_SEARCH));
	SetIcon(icon, TRUE);

	container_ = std::shared_ptr<CAlarmMachineContainerDlg>(new CAlarmMachineContainerDlg(this), [](CDialogEx*p) {SAFEDELETEDLG(p); });
	container_->Create(IDD_DIALOG_CONTAINER, &m_tab);

	/*CRect rc;
	m_input.GetWindowRect(rc);
	int h = rc.Height();
	GetClientRect(rc);
	rc.top += h + 5;

	container_->MoveWindow(rc);
	container_->ShowWindow(SW_SHOW);*/

	//SetTimer(1, 2000, nullptr);

	m_tab.InsertItem(0, TR(IDS_STRING_LIST));
	m_tab.InsertItem(1, TR(IDS_STRING_DETAIL));

	reposition_items();

	init_list_headers();

	container_->ShowWindow(SW_SHOW);
	m_list.ShowWindow(SW_HIDE);

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

	reposition_items();
}


void CSearchMachineResultDlg::OnTimer(UINT_PTR nIDEvent)
{
	//auto_timer t(m_hWnd, 1, 2000);

	

	CDialogEx::OnTimer(nIDEvent);
}


void CSearchMachineResultDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize.x = 600;
	lpMMI->ptMinTrackSize.y = 600;

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


void CSearchMachineResultDlg::OnTcnSelchangeTab1(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	*pResult = 0;

	int index = m_tab.GetCurSel();
	if (index < 0) { return; }

	if (index == 0) {
		container_->ShowWindow(SW_SHOW);
		m_list.ShowWindow(SW_HIDE);
	} else if (index == 1) {
		container_->ShowWindow(SW_HIDE);
		m_list.ShowWindow(SW_SHOW);
	}
}


void CSearchMachineResultDlg::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;
	auto data = m_list.GetItemData(pNMItemActivate->iItem);
	container_->PostMessageW(WM_BNCLKEDEX, 0, data);
}


void CSearchMachineResultDlg::OnSetFocus(CWnd* pOldWnd)
{
	CDialogEx::OnSetFocus(pOldWnd);

	got_focus_ = true;
}


void CSearchMachineResultDlg::OnKillFocus(CWnd* pNewWnd)
{
	CDialogEx::OnKillFocus(pNewWnd);

	got_focus_ = false;
}


void CSearchMachineResultDlg::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{


	CDialogEx::OnChar(nChar, nRepCnt, nFlags);
}


BOOL CSearchMachineResultDlg::PreTranslateMessage(MSG* pMsg)
{
	if (m_input.GetSafeHwnd() && pMsg->hwnd == m_input.GetSafeHwnd()) {
		if (pMsg->message == WM_KEYUP) {
			if (pMsg->wParam == VK_RETURN) {
				do_search();
				return TRUE;
			}
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CSearchMachineResultDlg::OnBnClickedButton1()
{
	do_search();
}
