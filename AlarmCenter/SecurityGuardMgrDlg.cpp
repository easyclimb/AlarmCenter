// SecurityGuardMgrDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "SecurityGuardMgrDlg.h"
#include "AddSecurityGuardDlg.h"
#include "afxdialogex.h"
#include "alarm_handle_mgr.h"


using namespace gui::control::grid_ctrl;
using namespace core;
namespace detail {

void insert_guard_row(CGridCtrl& grid, const security_guard_ptr& guard)
{
	int row = grid.GetRowCount();
	grid.SetRowCount(row + 1);
	CString txt;

	GV_ITEM item;
	item.mask = GVIF_TEXT | GVIF_FORMAT;
	item.nFormat = DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS;
	item.row = row;

	// ID
	item.col = 0;
	txt.Format(L"%d", guard->get_id());
	item.strText = txt;
	grid.SetItem(&item);
	grid.SetRowHeight(row, 25); //set row height

								// name
	item.col++;
	item.strText = guard->get_name().c_str();
	grid.SetItem(&item);

	// phone
	item.col++;
	item.strText = guard->get_phone().c_str();
	grid.SetItem(&item);

	// status
	item.col++;
	txt.Format(L"%d", guard->get_id());
	item.strText = guard->get_status_text().c_str();
	grid.SetItem(&item);

	//grid.SelectRows(CCellID(row, -1), 1, 1);
	//grid.Refresh();
}

}

using namespace detail;

// CSecurityGuardMgrDlg dialog

IMPLEMENT_DYNAMIC(CSecurityGuardMgrDlg, CDialogEx)

CSecurityGuardMgrDlg::CSecurityGuardMgrDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_SECURITY_GUARD_MGR, pParent)
{

}

CSecurityGuardMgrDlg::~CSecurityGuardMgrDlg()
{
}

void CSecurityGuardMgrDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CUSTOM2, m_user_list);
}


BEGIN_MESSAGE_MAP(CSecurityGuardMgrDlg, CDialogEx)
	ON_NOTIFY(GVN_BEGINLABELEDIT, IDC_CUSTOM2, &CSecurityGuardMgrDlg::OnGridStartEditUser)
	ON_NOTIFY(GVN_ENDLABELEDIT, IDC_CUSTOM2, &CSecurityGuardMgrDlg::OnGridEndEditUser)
	ON_NOTIFY(GVN_SELCHANGED, IDC_CUSTOM2, &CSecurityGuardMgrDlg::OnGridItemChangedUser)
	ON_BN_CLICKED(IDC_BUTTON_ADD_GUARD, &CSecurityGuardMgrDlg::OnBnClickedButtonAddGuard)
	ON_BN_CLICKED(IDC_BUTTON_RM_GUARD, &CSecurityGuardMgrDlg::OnBnClickedButtonRmGuard)
END_MESSAGE_MAP()


// CSecurityGuardMgrDlg message handlers


BOOL CSecurityGuardMgrDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowTextW(TR(IDS_STRING_SECURITY_MANAGEMENT));

	SET_WINDOW_TEXT(IDC_BUTTON_ADD_GUARD, IDS_STRING_ADD_SECURITY_GUARD);
	SET_WINDOW_TEXT(IDC_BUTTON_RM_GUARD, IDS_STRING_REMOVE_SECURITY_GUARD);

	SET_WINDOW_TEXT(IDOK, IDS_OK);
	//SET_WINDOW_TEXT(IDCANCEL, IDS_CANCEL);

	auto mgr = alarm_handle_mgr::get_instance();
	auto guard_ids = mgr->get_security_guard_ids();

	const int col_count = 4;
	const int row_cnt = 0;

	m_user_list.DeleteAllItems();
	m_user_list.SetEditable(true);
	m_user_list.SetTextBkColor(RGB(0xFF, 0xFF, 0xE0));//yellow background
	m_user_list.SetRowCount(row_cnt + 1);
	m_user_list.SetColumnCount(col_count);
	m_user_list.SetFixedRowCount(1);
	m_user_list.SetFixedColumnCount(1);
	m_user_list.SetListMode();

	// 设置表头	
	for (int col = 0; col < col_count; col++) {
		GV_ITEM item;
		item.mask = GVIF_TEXT | GVIF_FORMAT;
		item.row = 0;
		item.col = col;
		item.nFormat = DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS;

		m_user_list.SetRowHeight(0, 25); //set row heigh          

		switch (col) {
		case 0: // ID
			item.strText = L"ID";
			m_user_list.SetColumnWidth(col, 50);
			break;
		case 1: // 姓名
			item.strText = TR(IDS_STRING_NAME);
			m_user_list.SetColumnWidth(col, 100);
			break;
		case 2: // 手机
			item.strText = TR(IDS_STRING_IDC_STATIC_027);
			m_user_list.SetColumnWidth(col, 150);
			break;
		case 3: // 当前状态
			item.strText = TR(IDS_STRING_MACHINE_STATUS);
			m_user_list.SetColumnWidth(col, 100);
			break;

		default:
			break;
		}

		m_user_list.SetItem(&item);
	}

	for (auto id : guard_ids) {
		auto guard = mgr->get_security_guard(id); assert(guard);
		if (guard) {
			insert_guard_row(m_user_list, guard);
		}

	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}



void CSecurityGuardMgrDlg::OnGridStartEditUser(NMHDR * pNotifyStruct, LRESULT * pResult)
{
	bool allow_edit = false;

	NM_GRIDVIEW* pItem = (NM_GRIDVIEW*)pNotifyStruct;
	if (pItem->iColumn == 1 || pItem->iColumn == 2) {
		allow_edit = true;
	}

	*pResult = allow_edit ? 0 : -1;
}

void CSecurityGuardMgrDlg::OnGridEndEditUser(NMHDR * pNotifyStruct, LRESULT * pResult)
{
	bool accept_edit = false;
	NM_GRIDVIEW* pItem = (NM_GRIDVIEW*)pNotifyStruct;
	auto cell = m_user_list.GetCell(pItem->iRow, pItem->iColumn);
	if (cell && cur_editting_guard_id_ != 0) {
		auto mgr = alarm_handle_mgr::get_instance();
		auto guard = mgr->get_security_guard(cur_editting_guard_id_);

		if (guard) {

			bool changed = false;
			std::wstring name = guard->get_name();
			std::wstring phone = guard->get_phone();

			switch (pItem->iColumn) {
			case 1: // name
				name = cell->GetText();
				if (guard->get_name() != name) {
					changed = true;
				}
				break;

			case 2: // phone
				phone = cell->GetText();
				if (guard->get_phone() != phone) {
					changed = true;
				}
				break;

			default:
				break;
			}

			if (changed) {
				accept_edit = mgr->execute_update_security_guard_info(guard->get_id(), name, phone);
			}
		}
	}


	*pResult = accept_edit ? 0 : -1;
}

void CSecurityGuardMgrDlg::OnGridItemChangedUser(NMHDR * pNotifyStruct, LRESULT * pResult)
{
	NM_GRIDVIEW* pItem = (NM_GRIDVIEW*)pNotifyStruct;
	auto the_cell = m_user_list.GetCell(pItem->iRow, 0);
	if (the_cell) {
		std::wstring sid = the_cell->GetText();
		cur_editting_guard_id_ = std::stoi(sid);

	}
}



void CSecurityGuardMgrDlg::OnBnClickedButtonAddGuard()
{
	CAddSecurityGuardDlg dlg;
	if (IDOK != dlg.DoModal()) {
		return;
	}

	auto mgr = alarm_handle_mgr::get_instance();
	auto guard = mgr->get_security_guard(dlg.guard_id_); assert(guard);
	if (!guard) {
		return;
	}

	insert_guard_row(m_user_list, guard);
	m_user_list.Refresh();
}


void CSecurityGuardMgrDlg::OnBnClickedButtonRmGuard()
{
	auto set = m_user_list.GetSelectedRows();

	if (!set.empty()) {
		int row = *set.begin();
		auto cell = m_user_list.GetCell(row, 0);
		if (cell) {
			std::wstring sid = cell->GetText();
			int id = std::stoi(sid);
			if (alarm_handle_mgr::get_instance()->execute_rm_security_guard(id)) {
				m_user_list.DeleteRow(row);
				m_user_list.Refresh();
				cur_editting_guard_id_ = 0;
			}
		}
	}
}
