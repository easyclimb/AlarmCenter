// AlarmHandleStep3.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AlarmHandleStep3Dlg.h"
#include "AddSecurityGuardDlg.h"
#include "afxdialogex.h"
#include "alarm_handle_mgr.h"

using namespace gui::control::grid_ctrl;
using namespace core;

namespace detail {

void insert_user_row(CGridCtrl& grid, const security_guard_ptr& guard)
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



// CAlarmHandleStep3 dialog

IMPLEMENT_DYNAMIC(CAlarmHandleStep3Dlg, CDialogEx)

CAlarmHandleStep3Dlg::CAlarmHandleStep3Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_ALARM_HANDLE_3, pParent)
{

}

CAlarmHandleStep3Dlg::~CAlarmHandleStep3Dlg()
{
}

void CAlarmHandleStep3Dlg::init_list()
{
	const int col_count = 7;
	const int row_cnt = 1;

	m_grid.DeleteAllItems();
	m_grid.SetEditable(true);
	//m_grid.SetTextBkColor(RGB(0xFF, 0xFF, 0xE0));//yellow background
	m_grid.SetRowCount(row_cnt + 1);
	m_grid.SetColumnCount(col_count);
	m_grid.SetFixedRowCount(1);
	m_grid.SetFixedColumnCount(1);
	m_grid.SetListMode();

	// 设置表头	
	for (int col = 0; col < col_count; col++) {
		GV_ITEM item;
		item.mask = GVIF_TEXT | GVIF_FORMAT;
		item.row = 0;
		item.col = col;
		item.nFormat = DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS;

		m_grid.SetRowHeight(0, 25); //set row heigh          

		switch (col) {
		case 0: // 序号
			item.strText = TR(IDS_STRING_INDEX);
			m_grid.SetColumnWidth(col, 50);
			break;
		case 1: // ID
			item.strText = L"ID";
			m_grid.SetColumnWidth(col, 50);
			break;
		case 2: // 姓名
			item.strText = TR(IDS_STRING_NAME);
			m_grid.SetColumnWidth(col, 100);
			break;
		case 3: // 手机
			item.strText = TR(IDS_STRING_IDC_STATIC_027);
			m_grid.SetColumnWidth(col, 150);
			break;
		case 4: // 当前状态
			item.strText = TR(IDS_STRING_MACHINE_STATUS);
			m_grid.SetColumnWidth(col, 100);
			break;
		case 5: // 预设处警时间
			item.strText = TR(IDS_STRING_DEFAULT_HANDLE_TIME);
			m_grid.SetColumnWidth(col, 100);
			break;
		case 6: // 备注
			item.strText = TR(IDS_STRING_NOTE);
			m_grid.SetColumnWidth(col, 300);
			break;

		default:
			break;
		}

		m_grid.SetItem(&item);
	}

	
}

void CAlarmHandleStep3Dlg::init_list_data()
{
	auto mgr = alarm_handle_mgr::get_instance();
	cur_editting_handle_id_ = mgr->allocate_alarm_handle_id();
	int row = 1;
	CString txt;

	GV_ITEM item;
	item.mask = GVIF_TEXT | GVIF_FORMAT;
	item.nFormat = DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS;
	item.row = row;

	// 序号
	item.col = 0;
	txt.Format(L"%d", cur_editting_handle_id_);
	item.strText = txt;
	m_grid.SetItem(&item);
	m_grid.SetRowHeight(row, 25); //set row height

	// ID
	item.col++;
	item.strText = L"";
	m_grid.SetItem(&item);

	// name
	item.col++;
	item.strText = L"";
	m_grid.SetItem(&item);

	// phone
	item.col++;
	item.strText = L"";
	m_grid.SetItem(&item);

	// status
	item.col++;
	item.strText = L"";
	m_grid.SetItem(&item);

	// handle time
	item.col++;
	txt.Format(L"%d", cur_editting_handle_time_);
	item.strText = txt;
	m_grid.SetItem(&item);

	// note
	item.col++;
	item.strText = cur_editting_note_.c_str();
	m_grid.SetItem(&item);
}

void CAlarmHandleStep3Dlg::init_user_list()
{
	auto mgr = alarm_handle_mgr::get_instance();
	auto guard_ids = mgr->get_security_guard_ids();

	const int col_count = 4;
	const int row_cnt = 0;

	m_user_list.DeleteAllItems();
	m_user_list.SetEditable(true);
	//m_user_list.SetTextBkColor(RGB(0xFF, 0xFF, 0xE0));//yellow background
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
			insert_user_row(m_user_list, guard);
		}
		
	}
}

void CAlarmHandleStep3Dlg::assign_task_to_guard()
{
	auto mgr = alarm_handle_mgr::get_instance();
	auto guard = mgr->get_security_guard(cur_editting_guard_id_);

	if (guard) {
		CString txt;
		auto cell = m_grid.GetCell(1, 1);
		if (cell) {
			txt.Format(L"%d", guard->get_id());
			cell->SetText(txt);
		}

		cell = m_grid.GetCell(1, 2);
		if (cell) {
			txt = guard->get_name().c_str();
			cell->SetText(txt);
		}

		cell = m_grid.GetCell(1, 3);
		if (cell) {
			txt = guard->get_phone().c_str();
			cell->SetText(txt);
		}

		cell = m_grid.GetCell(1, 4);
		if (cell) {
			txt = guard->get_status_text().c_str();
			cell->SetText(txt);
		}

		m_grid.Refresh();

		check_valid();
	}
}

bool CAlarmHandleStep3Dlg::check_valid()
{
	bool valid = false;
	do {
		if (cur_editting_guard_id_ == 0) {
			break;
		}

		if (cur_editting_handle_id_ == 0) {
			break;
		}

		if (cur_editting_handle_time_ < alarm_handle::handle_time_min) {
			break;
		}

		if (cur_editting_handle_time_ > alarm_handle::handle_time_max) {
			break;
		}
		
		valid = true;
	} while (0);

	m_btn_ok.EnableWindow(valid);
	return valid;
}

void CAlarmHandleStep3Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CUSTOM1, m_grid);
	DDX_Control(pDX, IDC_CUSTOM2, m_user_list);
	DDX_Control(pDX, IDOK, m_btn_ok);
}


BEGIN_MESSAGE_MAP(CAlarmHandleStep3Dlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_ADD_GUARD, &CAlarmHandleStep3Dlg::OnBnClickedButtonAddGuard)
	ON_BN_CLICKED(IDC_BUTTON_RM_GUARD, &CAlarmHandleStep3Dlg::OnBnClickedButtonRmGuard)
	ON_NOTIFY(GVN_BEGINLABELEDIT, IDC_CUSTOM1, &CAlarmHandleStep3Dlg::OnGridStartEdit)
	ON_NOTIFY(GVN_ENDLABELEDIT, IDC_CUSTOM1, &CAlarmHandleStep3Dlg::OnGridEndEdit)
	ON_NOTIFY(GVN_SELCHANGED, IDC_CUSTOM1, &CAlarmHandleStep3Dlg::OnGridItemChanged)
	ON_NOTIFY(GVN_BEGINLABELEDIT, IDC_CUSTOM2, &CAlarmHandleStep3Dlg::OnGridStartEditUser)
	ON_NOTIFY(GVN_ENDLABELEDIT, IDC_CUSTOM2, &CAlarmHandleStep3Dlg::OnGridEndEditUser)
	ON_NOTIFY(GVN_SELCHANGED, IDC_CUSTOM2, &CAlarmHandleStep3Dlg::OnGridItemChangedUser)
	ON_BN_CLICKED(IDOK, &CAlarmHandleStep3Dlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CAlarmHandleStep3Dlg message handlers

BOOL CAlarmHandleStep3Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowTextW(TR(IDS_STRING_ALARM_HANDLE_3));
	SET_WINDOW_TEXT(IDC_BUTTON_ADD_GUARD, IDS_STRING_ADD_SECURITY_GUARD);
	SET_WINDOW_TEXT(IDC_BUTTON_RM_GUARD, IDS_STRING_REMOVE_SECURITY_GUARD);
	SET_WINDOW_TEXT(IDOK, IDS_STRING_ASSIGN_TASK);
	SET_WINDOW_TEXT(IDCANCEL, IDS_CANCEL);

	m_btn_ok.EnableWindow(0);

	cur_editting_handle_time_ = alarm_handle::handle_time_default;

	init_list();
	init_list_data();
	init_user_list();

	m_grid.Refresh();
	m_user_list.Refresh();


	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CAlarmHandleStep3Dlg::OnBnClickedButtonAddGuard()
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

	insert_user_row(m_user_list, guard);
	m_user_list.Refresh();
}


void CAlarmHandleStep3Dlg::OnBnClickedButtonRmGuard()
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
				init_list_data();
				m_user_list.Refresh();
				cur_editting_guard_id_ = 0;
				check_valid();
			}
		}
	}
}

void CAlarmHandleStep3Dlg::OnGridStartEdit(NMHDR * pNotifyStruct, LRESULT * pResult)
{
	bool allow_edit = false;
	NM_GRIDVIEW* pItem = (NM_GRIDVIEW*)pNotifyStruct;
	if (pItem->iColumn == 5 || pItem->iColumn == 6) {
		allow_edit = true;
	}

	*pResult = allow_edit ? 0 : -1;
}

void CAlarmHandleStep3Dlg::OnGridEndEdit(NMHDR * pNotifyStruct, LRESULT * pResult)
{
	bool accept_edit = false;
	NM_GRIDVIEW* pItem = (NM_GRIDVIEW*)pNotifyStruct;
	auto cell = m_grid.GetCell(pItem->iRow, pItem->iColumn);
	if (cell) {
		switch (pItem->iColumn) {
		case 5: // handle time in minutes
			try {
				std::wstring smin = cell->GetText();
				int min = std::stoi(smin);

				if (min < alarm_handle::handle_time_min) {
					break;
				}

				if (min > alarm_handle::handle_time_max) {
					break;
				}

				cur_editting_handle_time_ = min;
				accept_edit = true;
			} catch (...) {
				accept_edit = false;
			} 
		break;

		case 6: // note
			cur_editting_note_ = cell->GetText();
			accept_edit = true;
			break;

		default:
			break;
		}
	}

	*pResult = accept_edit ? 0 : -1;
}

void CAlarmHandleStep3Dlg::OnGridItemChanged(NMHDR * pNotifyStruct, LRESULT * /*pResult*/)
{
	NM_GRIDVIEW* pItem = (NM_GRIDVIEW*)pNotifyStruct;
	auto the_cell = m_grid.GetCell(pItem->iRow, 0);
	if (the_cell) {

	}
}

void CAlarmHandleStep3Dlg::OnGridStartEditUser(NMHDR * pNotifyStruct, LRESULT * pResult)
{
	bool allow_edit = false;

	NM_GRIDVIEW* pItem = (NM_GRIDVIEW*)pNotifyStruct;
	if (pItem->iColumn == 1 || pItem->iColumn == 2) {
		allow_edit = true;
	}

	*pResult = allow_edit ? 0 : -1;
}

void CAlarmHandleStep3Dlg::OnGridEndEditUser(NMHDR * pNotifyStruct, LRESULT * pResult)
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
				if (accept_edit) {
					assign_task_to_guard();
				}
			}
		}
	}


	*pResult = accept_edit ? 0 : -1;
}

void CAlarmHandleStep3Dlg::OnGridItemChangedUser(NMHDR * pNotifyStruct, LRESULT * /*pResult*/)
{
	NM_GRIDVIEW* pItem = (NM_GRIDVIEW*)pNotifyStruct;
	auto the_cell = m_user_list.GetCell(pItem->iRow, 0);
	if (the_cell) {
		std::wstring sid = the_cell->GetText();
		cur_editting_guard_id_ = std::stoi(sid);
		assign_task_to_guard();

	}
}


void CAlarmHandleStep3Dlg::OnBnClickedOk()
{
	if (!check_valid()) {
		return;
	}

	CDialogEx::OnOK();
}
