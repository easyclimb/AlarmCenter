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

void insert_row(CGridCtrl& grid, int row, const security_guard_ptr& guard) 
{
	grid.SetRowCount(grid.GetRowCount() + 1);
	CString txt;

	GV_ITEM item;
	item.mask = GVIF_TEXT | GVIF_FORMAT;
	item.nFormat = DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS;
	item.row = row;

	// 序号
	item.col = 0;
	txt.Format(L"%d", row);
	item.strText = txt;
	grid.SetItem(&item);
	grid.SetRowHeight(row, 25); //set row height

	// ID
	item.col++;
	txt.Format(L"%d", guard->get_id());
	item.strText = txt;
	grid.SetItem(&item);

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

	// handle time
	item.col++;
	txt.Format(L"%d", guard->get_id());
	item.strText = txt;
	grid.SetItem(&item);

	// note
	item.col++;
	txt.Format(L"%d", guard->get_id());
	item.strText = txt;
	grid.SetItem(&item);

	//grid.SelectRows(CCellID(row, -1), 1, 1);
	//grid.Refresh();
}

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
	const int row_cnt = 0;

	m_grid.DeleteAllItems();
	m_grid.SetEditable(true);
	m_grid.SetTextBkColor(RGB(0xFF, 0xFF, 0xE0));//yellow background
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

void CAlarmHandleStep3Dlg::init_user_list()
{
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
			insert_user_row(m_user_list, guard);
		}
		
	}
}

void CAlarmHandleStep3Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CUSTOM1, m_grid);
	DDX_Control(pDX, IDC_CUSTOM2, m_user_list);
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

	init_list();
	init_user_list();




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
}


void CAlarmHandleStep3Dlg::OnBnClickedButtonRmGuard()
{

}


