// SubMachineExpireManagerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "AppResource.h"
#include "SubMachineExpireManagerDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "AlarmMachineManager.h"
#include "ZoneInfo.h"
#include "ExtendExpireTimeDlg.h"
#include "ConsumerTypeMgrDlg.h"
#include "InputGroupNameDlg.h"

#include <iterator>
#include <odbcinst.h>
#include <afxdb.h>
#include <comdef.h>
#include <set>
using namespace core;
using namespace gui::control::grid_ctrl;
int (WINAPIV * __vsnprintf)(char *, size_t, const char*, va_list) = _vsnprintf;
namespace detail {

static const int DEFAULT_GRID_COLOMN_INDEX_TO_STORAGE_ITEM_DATA = 0;

enum column_header {
	col_aid,
	col_alias,
	col_type,
	col_expire_time,
	col_is_expired,
	col_remind_time,
	col_receivable,
	col_paid,
	col_owed,
	col_is_owed,
	col_contact,
	col_addr,
	col_phone,
	col_phone_bk,
	col_count,
};

};
using namespace detail;

// CMachineExpireManagerDlg 对话框

IMPLEMENT_DYNAMIC(CMachineExpireManagerDlg, CDialogEx)

CMachineExpireManagerDlg::CMachineExpireManagerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CMachineExpireManagerDlg::IDD, pParent)
	//, m_machine(nullptr)
{

}

CMachineExpireManagerDlg::~CMachineExpireManagerDlg()
{
}

void CMachineExpireManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
#ifdef USE_MFC_GRID_CTRL
	DDX_Control(pDX, IDC_CUSTOM_GRID, m_grid);
#else
	DDX_Control(pDX, IDC_LIST1, m_list);
#endif

	DDX_Control(pDX, IDC_STATIC_LINE_NUM, m_staticSeldLineNum);

	DDX_Control(pDX, IDC_BUTTON_ALL, m_btn_all);
	DDX_Control(pDX, IDC_BUTTON_ALL_NOT, m_btn_all_not);
	DDX_Control(pDX, IDC_BUTTON_EXPORT_SEL, m_btn_export_sel_to_excel);
	DDX_Control(pDX, IDC_BUTTON_PRINT_SEL, m_btn_print_sel);
	DDX_Control(pDX, IDC_BUTTON_SET_REMIND_TIME, m_btn_set_sel_remind_time);
	DDX_Control(pDX, IDC_BUTTON_EXTEND, m_btn_extend_sel_expired_time);
	DDX_Control(pDX, IDC_STATIC_SELECTED, m_static_label);
	DDX_Control(pDX, IDC_BUTTON_SET_TYPE, m_btn_set_type);
	DDX_Control(pDX, IDC_BUTTON_TYPE_MANAGER, m_btn_type_manager);
}


BEGIN_MESSAGE_MAP(CMachineExpireManagerDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CMachineExpireManagerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CMachineExpireManagerDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_EXTEND, &CMachineExpireManagerDlg::OnBnClickedButtonExtend)
	ON_BN_CLICKED(IDC_BUTTON_ALL, &CMachineExpireManagerDlg::OnBnClickedButtonAll)
	ON_BN_CLICKED(IDC_BUTTON_ALL_NOT, &CMachineExpireManagerDlg::OnBnClickedButtonAllNot)
	ON_BN_CLICKED(IDC_BUTTON_INVERT, &CMachineExpireManagerDlg::OnBnClickedButtonInvert)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, &CMachineExpireManagerDlg::OnNMClickList1)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT_SEL, &CMachineExpireManagerDlg::OnBnClickedButtonExportSel)
	ON_BN_CLICKED(IDC_BUTTON_PRINT_SEL, &CMachineExpireManagerDlg::OnBnClickedButtonPrintSel)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST1, &CMachineExpireManagerDlg::OnLvnColumnclickList1)
	ON_NOTIFY(GVN_BEGINLABELEDIT, IDC_CUSTOM_GRID, &CMachineExpireManagerDlg::OnGridStartEdit)
	ON_NOTIFY(GVN_ENDLABELEDIT, IDC_CUSTOM_GRID, &CMachineExpireManagerDlg::OnGridEndEdit)
	ON_NOTIFY(GVN_SELCHANGED, IDC_CUSTOM_GRID, &CMachineExpireManagerDlg::OnGridItemChanged)
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED(IDC_BUTTON_SET_REMIND_TIME, &CMachineExpireManagerDlg::OnBnClickedButtonSetRemindTime)
	ON_BN_CLICKED(IDC_BUTTON_TYPE_MANAGER, &CMachineExpireManagerDlg::OnBnClickedButtonTypeManager)
	ON_BN_CLICKED(IDC_BUTTON_SET_TYPE, &CMachineExpireManagerDlg::OnBnClickedButtonSetType)
END_MESSAGE_MAP()


// CMachineExpireManagerDlg 消息处理程序


void CMachineExpireManagerDlg::OnBnClickedOk() 
{
	CDialogEx::OnOK();
}


void CMachineExpireManagerDlg::OnBnClickedCancel() 
{
	CDialogEx::OnCancel();
}


void CMachineExpireManagerDlg::SetExpiredMachineList(std::list<core::alarm_machine_ptr>& list)
{
	std::copy(list.begin(), list.end(), std::back_inserter(m_expiredMachineList));
}


void CMachineExpireManagerDlg::SetExpireTime(CPoint pos)
{
	if (m_grid.GetSelectedCount() == 0)
		return;

	CMenu menu, *sub;
	menu.LoadMenuW(IDR_MENU6);
	sub = menu.GetSubMenu(0); assert(sub); if (!sub) return;
	DWORD ret = sub->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
									pos.x, pos.y, this);
	JLOG(L"TrackPopupMenu ret %d\n", ret);

	if (ret == 0)return;

	auto user_set_date_time = std::chrono::system_clock::now();
	if (ret == ID_EXTEND_SET) {
		CExtendExpireTimeDlg dlg(this); if (IDOK != dlg.DoModal()) return;
		user_set_date_time = dlg.m_dateTime;
	}

	CString syes, sno; syes = GetStringFromAppResource(IDS_STRING_YES); sno = GetStringFromAppResource(IDS_STRING_NO);
	auto mgr = alarm_machine_manager::GetInstance();
	auto set = m_grid.GetSelectedRows();
	for (auto row : set) {
		DWORD data = m_grid.GetItemData(row, detail::DEFAULT_GRID_COLOMN_INDEX_TO_STORAGE_ITEM_DATA);
		alarm_machine_ptr machine;
		if (m_bSubMachine)
			machine = m_machine->GetZone(data)->GetSubMachineInfo();
		else
			machine = mgr->GetMachine(data);

		auto expire_time = machine->get_expire_time();

		switch (ret) {
		case ID_EXTEND_1_MONTH:
		{
			expire_time += std::chrono::hours(24) * 30;
		}
		break;

		case ID_EXTEND_2_MONTH:
		{
			expire_time += std::chrono::hours(24) * 30 * 2;
		}
		break;

		case ID_EXTEND_3_MONTH:
		{
			expire_time += std::chrono::hours(24) * 30 * 3;
		}
		break;

		case ID_EXTEND_6_MONTH:
		{
			expire_time += std::chrono::hours(24) * 30 * 6;
		}
		break;

		case ID_EXTEND_1_YEAR:
		{
			expire_time += std::chrono::hours(24) * 365;
		}
		break;

		case ID_EXTEND_2_YEAR:
		{
			expire_time += std::chrono::hours(24) * 730;
		}
		break;

		case ID_EXTEND_SET:
			expire_time = user_set_date_time;
			break;
		}

		if (machine && machine->execute_update_expire_time(expire_time)) {
			m_grid.SetItemText(row, col_expire_time, time_point_to_wstring(expire_time).c_str());
			m_grid.SetItemText(row, col_is_expired, machine->get_left_service_time_in_minutes() <= 0 ? syes : sno);
		}
	}
	m_grid.Refresh();
}


void CMachineExpireManagerDlg::OnBnClickedButtonExtend() 
{
#ifdef USE_MFC_GRID_CTRL
	CRect rc;
	m_btn_extend_sel_expired_time.GetWindowRect(rc);
	SetExpireTime(CPoint(rc.left, rc.bottom));

#else
	if (m_list.GetSelectedCount() == 0)
		return;

	CExtendExpireTimeDlg dlg(this);
	if (dlg.DoModal() != IDOK)
		return;

	CString syes, sno; syes = GetStringFromAppResource(IDS_STRING_YES); sno = GetStringFromAppResource(IDS_STRING_NO);
	int ndx = -1;
	auto mgr = alarm_machine_manager::GetInstance();
	for (UINT i = 0; i < m_list.GetSelectedCount(); i++) {
		ndx = m_list.GetNextItem(ndx, LVNI_SELECTED);
		if (ndx == -1)
			break;
		DWORD data = m_list.GetItemData(ndx);
		alarm_machine_ptr machine;
		if (m_bSubMachine)
			machine = m_machine->GetZone(data)->GetSubMachineInfo();
		else
			machine = mgr->GetMachine(data);
		if (machine && machine->execute_update_expire_time(dlg.m_dateTime)) {
			m_list.SetItemText(ndx, 2, dlg.m_dateTime.Format(L"%Y-%m-%d %H:%M:%S"));
			m_list.SetItemText(ndx, 3, machine->get_left_service_time_in_minutes() <= 0 ? syes : sno);
		}
	}
#endif
}


void CMachineExpireManagerDlg::InitializeGrid()
{
	m_grid.DeleteAllItems();
	m_grid.SetEditable(true);
	m_grid.SetTextBkColor(RGB(0xFF, 0xFF, 0xE0));//yellow background
	m_grid.SetRowCount(m_expiredMachineList.size() + 1);
	m_grid.SetColumnCount(col_count);
	m_grid.SetFixedRowCount(1);
	//m_grid.SetFixedColumnCount(1); 
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
		case detail::col_aid:
			item.strText = m_bSubMachine ? GetStringFromAppResource(IDS_STRING_SUBMACHINE) : GetStringFromAppResource(IDS_STRING_MACHINE);
			m_grid.SetColumnWidth(col, 50);
			break;
		case detail::col_alias:
			item.strText = GetStringFromAppResource(IDS_STRING_ALIAS);
			m_grid.SetColumnWidth(col, 200);
			break;
		case detail::col_type:
			item.strText = GetStringFromAppResource(IDS_STRING_TYPE);
			m_grid.SetColumnWidth(col, 80);
			break;
		case detail::col_expire_time:
			item.strText = GetStringFromAppResource(IDS_STRING_EXPIRE_TIME);
			m_grid.SetColumnWidth(col, 125);
			break;
		case detail::col_is_expired:
			item.strText = GetStringFromAppResource(IDS_STRING_IF_EXPIRE);
			m_grid.SetColumnWidth(col, 75);
			break;
		case detail::col_remind_time:
			item.strText = GetStringFromAppResource(IDS_STRING_REMIND_TIME);
			m_grid.SetColumnWidth(col, 125);
			break;
		case detail::col_receivable:
			item.strText = GetStringFromAppResource(IDS_STRING_RECEIVABLE);
			m_grid.SetColumnWidth(col, 75);
			break;
		case detail::col_paid:
			item.strText = GetStringFromAppResource(IDS_STRING_PAID);
			m_grid.SetColumnWidth(col, 75);
			break;
		case detail::col_owed:
			item.strText = GetStringFromAppResource(IDS_STRING_OWED);
			m_grid.SetColumnWidth(col, 75);
			break;
		case detail::col_is_owed:
			item.strText = GetStringFromAppResource(IDS_STRING_IS_OWED);
			m_grid.SetColumnWidth(col, 75);
			break;
		case detail::col_contact:
			item.strText = GetStringFromAppResource(IDS_STRING_CONTACT);
			m_grid.SetColumnWidth(col, 100);
			break;
		case detail::col_addr:
			item.strText = GetStringFromAppResource(IDS_STRING_ADDRESS);
			m_grid.SetColumnWidth(col, 225);
			break;
		case detail::col_phone:
			item.strText = GetStringFromAppResource(IDS_STRING_PHONE);
			m_grid.SetColumnWidth(col, 150);
			break;
		case detail::col_phone_bk:
			item.strText = GetStringFromAppResource(IDS_STRING_PHONE_BK);
			m_grid.SetColumnWidth(col, 150);
			break;
		default:
			break;
		}

		m_grid.SetItem(&item);
	}


	// 设置数据
	int row = 1;
	for (auto machine : m_expiredMachineList) {
		GV_ITEM item;
		item.mask = GVIF_TEXT | GVIF_FORMAT;
		item.nFormat = DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS;
		item.row = row;
		m_grid.SetRowHeight(row, 25); //set row height

		m_grid.SetItemData(row, detail::DEFAULT_GRID_COLOMN_INDEX_TO_STORAGE_ITEM_DATA,
						   m_bSubMachine ? machine->get_submachine_zone() : machine->get_ademco_id());

		// ndx
		item.col = 0;
		if (machine->get_is_submachine()) {
			// submachine zone value
			item.strText.Format(_T("%03d"), machine->get_submachine_zone());
		} else {
			// machine ademco id
			item.strText.Format(GetStringFromAppResource(IDS_STRING_FM_ADEMCO_ID), machine->get_ademco_id());
		}
		m_grid.SetItem(&item);

		// alias
		item.col++;
		item.strText = machine->get_machine_name();
		m_grid.SetItem(&item);

		// type
		item.col++;
		item.strText = machine->get_consumer()->type->name;
		m_grid.SetItem(&item);

		// expire time
		item.col++;
		item.strText = time_point_to_wstring(machine->get_expire_time()).c_str();
		m_grid.SetItem(&item);

		// if expire 
		CString syes, sno; syes = GetStringFromAppResource(IDS_STRING_YES); sno = GetStringFromAppResource(IDS_STRING_NO);
		item.col++;
		item.nFormat &= DT_LEFT; item.nFormat |= DT_CENTER;
		item.strText = machine->get_left_service_time_in_minutes() <= 0 ? syes : sno;
		m_grid.SetItem(&item);

		// remind time
		item.col++;
		item.nFormat &= ~DT_CENTER; item.nFormat |= DT_LEFT;
		item.strText = time_point_to_wstring(machine->get_consumer()->remind_time).c_str();
		m_grid.SetItem(&item);

		// receivable
		item.col++; item.nFormat &= DT_LEFT; item.nFormat |= DT_RIGHT;
		item.strText.Format(_T("%d"), machine->get_consumer()->receivable_amount);
		m_grid.SetItem(&item);

		// paid
		item.col++;  item.nFormat &= ~DT_LEFT; item.nFormat |= DT_RIGHT;
		item.strText.Format(_T("%d"), machine->get_consumer()->paid_amount);
		m_grid.SetItem(&item);

		// owed
		item.col++;  item.nFormat &= ~DT_LEFT; item.nFormat |= DT_RIGHT;
		item.strText.Format(_T("%d"), machine->get_consumer()->get_owed_amount());
		m_grid.SetItem(&item);

		// is owed
		item.col++;
		item.nFormat &= ~DT_RIGHT; item.nFormat |= DT_CENTER;
		item.strText.Format(_T("%s"), machine->get_consumer()->get_owed_amount() > 0 ? syes : sno);
		m_grid.SetItem(&item);

		// contact
		item.col++;
		item.nFormat &= ~DT_CENTER; item.nFormat |= DT_LEFT;
		item.strText = machine->get_contact();
		m_grid.SetItem(&item);

		// address
		item.col++;
		item.strText = machine->get_address();
		m_grid.SetItem(&item);

		// phone
		item.col++;  item.nFormat &= ~DT_LEFT; item.nFormat |= DT_RIGHT;
		item.strText = machine->get_phone();
		m_grid.SetItem(&item);

		// phone_bk
		item.col++;  item.nFormat &= ~DT_LEFT; item.nFormat |= DT_RIGHT;
		item.strText = machine->get_phone_bk();
		m_grid.SetItem(&item);

		row++;
	}
}


BOOL CMachineExpireManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

#ifdef USE_MFC_GRID_CTRL
	InitializeGrid();
#else
	DWORD dwStyle = m_list.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
	m_list.SetExtendedStyle(dwStyle);
	int i = -1;
	CString fm;
	fm = GetStringFromAppResource(IDS_STRING_MACHINE);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 50, -1);
	fm = GetStringFromAppResource(IDS_STRING_ALIAS);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 200, -1);
	fm = GetStringFromAppResource(IDS_STRING_EXPIRE_TIME);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 125, -1);
	fm = GetStringFromAppResource(IDS_STRING_IF_EXPIRE);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 75, -1);
	fm = GetStringFromAppResource(IDS_STRING_CONTACT);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 75, -1);
	fm = GetStringFromAppResource(IDS_STRING_ADDRESS);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 225, -1);
	fm = GetStringFromAppResource(IDS_STRING_PHONE);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 150, -1);
	fm = GetStringFromAppResource(IDS_STRING_PHONE_BK);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 150, -1);
	
	for (auto machine : m_expiredMachineList) {
		InsertList(machine);
	}
#endif

	

	m_staticSeldLineNum.SetWindowTextW(L"0");

	m_b_initialized_ = true;
	RepositionItems();
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}

//
//void CMachineExpireManagerDlg::InsertList(const core::alarm_machine_ptr& machine)
//{
//	assert(machine);
//	int nResult = -1;
//	LV_ITEM lvitem = { 0 };
//	CString tmp = _T("");
//
//	lvitem.lParam = 0;
//	lvitem.mask = LVIF_TEXT;
//	lvitem.iItem = m_list.GetItemCount();
//	lvitem.iSubItem = 0;
//
//	// ndx
//	if (machine->get_is_submachine()) {
//		// submachine zone value
//		tmp.Format(_T("%03d"), machine->get_submachine_zone());
//	} else {
//		// machine ademco id
//		tmp.Format(GetStringFromAppResource(IDS_STRING_FM_ADEMCO_ID), machine->get_ademco_id());
//	}
//	lvitem.pszText = tmp.LockBuffer();
//	nResult = m_list.InsertItem(&lvitem);
//	tmp.UnlockBuffer();
//
//	if (nResult != -1) {
//		// alias
//		lvitem.iItem = nResult;
//		lvitem.iSubItem++;
//		tmp.Format(_T("%s"), machine->get_formatted_machine_name());
//		lvitem.pszText = tmp.LockBuffer();
//		m_list.SetItem(&lvitem);
//		tmp.UnlockBuffer();
//
//		// expire time
//		lvitem.iSubItem++;
//		tmp.Format(_T("%s"), machine->get_expire_time().Format(L"%Y-%m-%d %H:%M:%S"));
//		lvitem.pszText = tmp.LockBuffer();
//		m_list.SetItem(&lvitem);
//		tmp.UnlockBuffer();
//
//		// if expire 
//		CString syes, sno; syes = GetStringFromAppResource(IDS_STRING_YES); sno = GetStringFromAppResource(IDS_STRING_NO);
//		lvitem.iSubItem++;
//		tmp.Format(_T("%s"), machine->get_left_service_time_in_minutes() <= 0 ? syes : sno);
//		lvitem.pszText = tmp.LockBuffer();
//		m_list.SetItem(&lvitem);
//		tmp.UnlockBuffer();
//
//		// contact
//		lvitem.iSubItem++;
//		tmp.Format(_T("%s"), machine->get_contact());
//		lvitem.pszText = tmp.LockBuffer();
//		m_list.SetItem(&lvitem);
//		tmp.UnlockBuffer();
//
//		// address
//		lvitem.iSubItem++;
//		tmp.Format(_T("%s"), machine->get_address());
//		lvitem.pszText = tmp.LockBuffer();
//		m_list.SetItem(&lvitem);
//		tmp.UnlockBuffer();
//
//		// phone
//		lvitem.iSubItem++;
//		tmp.Format(_T("%s"), machine->get_phone());
//		lvitem.pszText = tmp.LockBuffer();
//		m_list.SetItem(&lvitem);
//		tmp.UnlockBuffer();
//
//		// phone_bk
//		lvitem.iSubItem++;
//		tmp.Format(_T("%s"), machine->get_phone_bk());
//		lvitem.pszText = tmp.LockBuffer();
//		m_list.SetItem(&lvitem);
//		tmp.UnlockBuffer();
//
//		m_list.SetItemData(nResult, m_bSubMachine ? machine->get_submachine_zone() : machine->get_ademco_id());
//	}
//}


void CMachineExpireManagerDlg::OnBnClickedButtonAll()
{
#ifdef USE_MFC_GRID_CTRL
	m_grid.SelectAllCells();
	CString s; s.Format(L"%d", m_grid.GetSelectedRows().size());
	m_staticSeldLineNum.SetWindowTextW(s);
#else
	for (int i = 0; i < m_list.GetItemCount(); i++) {
		m_list.SetItemState(i, LVIS_FOCUSED | LVIS_SELECTED,
							LVIS_FOCUSED | LVIS_SELECTED);
	}
	m_list.SetFocus();
	CString s; s.Format(L"%d", m_list.GetSelectedCount());
	m_staticSeldLineNum.SetWindowTextW(s);
#endif
}


void CMachineExpireManagerDlg::OnBnClickedButtonAllNot() 
{
#ifdef USE_MFC_GRID_CTRL
	m_grid.EnableSelection(FALSE);
	m_grid.EnableSelection(TRUE);
	//auto cell = m_grid.GetCell(0, 0);
	//m_grid.sele
	CString s; s.Format(L"%d", m_grid.GetSelectedRows().size());
	m_staticSeldLineNum.SetWindowTextW(s);
#else
	for (int i = 0; i < m_list.GetItemCount(); i++) {
		m_list.SetItemState(i, 0, LVIS_FOCUSED | LVIS_SELECTED);
	}
	m_list.SetFocus();
	CString s; s.Format(L"%d", m_list.GetSelectedCount());
	m_staticSeldLineNum.SetWindowTextW(s);
#endif
}


void CMachineExpireManagerDlg::OnBnClickedButtonInvert()
{
#ifdef USE_MFC_GRID_CTRL
	for (int row = 1; row < m_grid.GetRowCount(); row++) {
		m_grid.SelectRows(CCellID(row, 0), 1, !m_grid.IsCellSelected(row, 0));
	}
#else
	for (int i = 0; i < m_list.GetItemCount(); i++) {
		if (m_list.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED) {
			m_list.SetItemState(i, 0, LVIS_FOCUSED | LVIS_SELECTED);
		} else {
			m_list.SetItemState(i, LVIS_FOCUSED | LVIS_SELECTED,
								LVIS_FOCUSED | LVIS_SELECTED);
		}
	}
	m_list.SetFocus();
	CString s; s.Format(L"%d", m_list.GetSelectedCount());
	m_staticSeldLineNum.SetWindowTextW(s);
#endif
}


void CMachineExpireManagerDlg::OnNMClickList1(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	//LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
#ifndef USE_MFC_GRID_CTRL
	CString s; s.Format(L"%d", m_list.GetSelectedCount());
	m_staticSeldLineNum.SetWindowTextW(s);
#endif
	*pResult = 0;
}


BOOL CMachineExpireManagerDlg::GetSaveAsFilePath(CString& path) {
	static CString prevPath = _T("");
RE_SAVE_AS:
	TCHAR szFilename[MAX_PATH] = { 0 };
	BOOL bResult = FALSE;
	DWORD dwError = NOERROR;
	OPENFILENAME ofn = { 0 };

	ofn.lStructSize = sizeof (OPENFILENAME);
	ofn.lpstrFilter = _T("Excel File(*.xls)\0*.xls\0\0");
	ofn.lpstrFile = szFilename;
	ofn.nMaxFile = MAX_PATH;
	ofn.hwndOwner = GetSafeHwnd();
	ofn.Flags = OFN_EXPLORER |
		OFN_ENABLEHOOK |
		OFN_HIDEREADONLY |
		OFN_NOCHANGEDIR |
		OFN_PATHMUSTEXIST;
	ofn.lpfnHook = nullptr;

	bResult = GetSaveFileName(&ofn);
	if (bResult == FALSE) {
		dwError = CommDlgExtendedError();
		return FALSE;
	}

	CString fileName = szFilename;
	if (CFileOper::GetFileExt(fileName).CompareNoCase(L"xls") != 0)
		fileName += L".xls";

	if (CFileOper::PathExists(fileName)) {
		CString q;
		q = GetStringFromAppResource(IDS_STRING_QUERY_REPLACE);
		int ret = MessageBox(q, L"", MB_YESNOCANCEL | MB_ICONQUESTION);
		if (ret == IDYES)
			DeleteFile(fileName);
		else if (ret == IDNO)
			goto RE_SAVE_AS;
		else
			return FALSE;
	}
	prevPath = fileName;
	path = fileName;
	return TRUE;
}


BOOL CMachineExpireManagerDlg::Export(const CString& excelPath) {
	CString warningStr = _T("");
	CDatabase database;
	CString sDriver = _T("");
	CString sSql = _T("");

	// 检索是否安装有Excel驱动 "Microsoft Excel Driver (*.xls)" 
	sDriver = GetExcelDriver();
	if (sDriver.IsEmpty()) {
		// 没有发现Excel驱动
		CString e;
		e = GetStringFromAppResource(IDS_STRING_E_NO_EXECEL);
		MessageBox(e, L"", MB_ICONERROR);
		return FALSE;
	}

	// 创建进行存取的字符串
	sSql.Format(_T("DRIVER={%s};DSN='';FIRSTROWHASNAMES=1;READONLY=FALSE;CREATE_DB=\")%s\";DBQ=%s"),
				sDriver, excelPath, excelPath);

	// 创建数据库 (既Excel表格文件)
	if (!database.OpenEx(sSql, CDatabase::noOdbcDialog)) {
		CString e;
		e = GetStringFromAppResource(IDS_STRING_E_CREATE_EXCEL);
		MessageBox(e, L"", MB_ICONERROR);
		return FALSE;
	}

	CString sid, salias, sexpire_time, sif_expire, scontact, saddress, sphone, sphone_bk, syes, sno;
	salias = GetStringFromAppResource(IDS_STRING_ALIAS);
	sexpire_time = GetStringFromAppResource(IDS_STRING_EXPIRE_TIME);
	sif_expire = GetStringFromAppResource(IDS_STRING_IF_EXPIRE);
	scontact = GetStringFromAppResource(IDS_STRING_CONTACT);
	saddress = GetStringFromAppResource(IDS_STRING_ADDRESS);
	sphone = GetStringFromAppResource(IDS_STRING_PHONE);
	sphone_bk = GetStringFromAppResource(IDS_STRING_PHONE_BK);
	syes = GetStringFromAppResource(IDS_STRING_YES);
	sno = GetStringFromAppResource(IDS_STRING_NO);

	CString stable;
	stable.Format(L"EXPIRED_MACHINES(Id TEXT,%s TEXT,%s TEXT,%s TEXT,%s TEXT,%s TEXT,%s TEXT, %s TEXT)",
				  salias, sexpire_time, sif_expire, scontact, saddress, sphone, sphone_bk);
	sSql.Format(_T("CREATE TABLE %s"), stable.LockBuffer());
	stable.UnlockBuffer();
	database.ExecuteSQL(sSql);
	
	CString sinsert, svalues;
	sinsert.Format(L"INSERT INTO EXPIRED_MACHINES(Id,%s,%s,%s,%s,%s,%s,%s) ", 
				   salias, sexpire_time, sif_expire, scontact, saddress, sphone, sphone_bk);
	auto mgr = alarm_machine_manager::GetInstance();

#ifdef USE_MFC_GRID_CTRL
	auto set = m_grid.GetSelectedRows();
	for (auto row : set) {
		DWORD data = m_grid.GetItemData(row, detail::DEFAULT_GRID_COLOMN_INDEX_TO_STORAGE_ITEM_DATA);
#else
	int nItem = -1;
	for (UINT i = 0; i < m_list.GetSelectedCount(); i++) {
		nItem = m_list.GetNextItem(nItem, LVNI_SELECTED);
		if (nItem == -1) break;
		DWORD data = m_list.GetItemData(nItem);
#endif
		alarm_machine_ptr machine;
		if (m_bSubMachine)
			machine = m_machine->GetZone(data)->GetSubMachineInfo();
		else
			machine = mgr->GetMachine(data);
		if (machine) {
			auto consumer = machine->get_consumer();
			svalues.Format(_T("VALUES('%d','%s','%s','%s','%s',%d,%d,%d,'%s''%s','%s','%s','%s')"),
						   machine->get_is_submachine() ? machine->get_submachine_zone() : machine->get_ademco_id(),
						   machine->get_machine_name(),
						   time_point_to_wstring(machine->get_expire_time()).c_str(),
						   machine->get_left_service_time_in_minutes() <= 0 ? syes : sno,
						   time_point_to_wstring(consumer->remind_time).c_str(),
						   consumer->receivable_amount,
						   consumer->paid_amount,
						   consumer->get_owed_amount(),
						   consumer->get_owed_amount() > 0 ? syes : sno,
						   machine->get_contact(),
						   machine->get_address(),
						   machine->get_phone(),
						   machine->get_phone_bk());
			database.ExecuteSQL(sinsert + svalues);
		}
	}

	// 关闭数据库
	database.Close();
	CString fm;
	fm = GetStringFromAppResource(IDS_STRING_FM_EXCEL_OK);
	warningStr.Format(fm, excelPath);
	if (IDYES == MessageBox(warningStr, L"", MB_YESNO | MB_ICONQUESTION)) {
		ShellExecute(nullptr, _T("Open"), excelPath, nullptr, nullptr, SW_SHOW);
	}
	return TRUE;
}



CString CMachineExpireManagerDlg::GetExcelDriver() 
{
	TCHAR szBuf[2001];
	WORD cbBufMax = 2000;
	WORD cbBufOut;
	TCHAR *pszBuf = szBuf;
	CString sDriver = _T("");

	// 获取已安装驱动的名称(涵数在odbcinst.h里)
	if (!SQLGetInstalledDrivers(szBuf, cbBufMax, &cbBufOut))
		return _T("");

	// 检索已安装的驱动是否有Excel...
	do {
		if (_tcsstr(pszBuf, _T("Excel")) != 0) {
			//发现 !
			sDriver = CString(pszBuf);
			break;
		}
		pszBuf = _tcschr(pszBuf, _T('\0')) + 1;
	} while (pszBuf[1] != _T('\0'));

	return sDriver;
}


void CMachineExpireManagerDlg::OnBnClickedButtonExportSel()
{
	AUTO_LOG_FUNCTION;
#ifdef USE_MFC_GRID_CTRL
	auto set = m_grid.GetSelectedRows();
	if (set.empty()) {
#else
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos == nullptr) {
#endif
		JLOG(_T("No items were selected!\n"));
		CString e; e = GetStringFromAppResource(IDS_STRING_NO_SELD_CONTENT);
		MessageBox(e, L"", MB_ICONERROR);
		return;
	}

	CString path;
	if (!GetSaveAsFilePath(path))
		return;

	Export(path);
}


BOOL CMachineExpireManagerDlg::PrintRecord(CListCtrl &list) {
	POSITION pos = list.GetFirstSelectedItemPosition();
	if (pos == nullptr) {
		JLOG(_T("No items were selected!\n"));
		CString e; e = GetStringFromAppResource(IDS_STRING_NO_SELD_CONTENT);
		MessageBox(e, L"", MB_ICONERROR);
		return FALSE;
	}

	HFONT hRecordFont;//记录的字体
	HFONT hTopicFont;//标题的字体
	HFONT hCodeFont;//字段的字体

	//创建(输出内容的)字体
	hRecordFont = CreateFont(93, 29, 1, 0, FW_EXTRALIGHT, 0, 0, 0, ANSI_CHARSET,
							 OUT_DEFAULT_PRECIS,
							 CLIP_DEFAULT_PRECIS,
							 DEFAULT_QUALITY,
							 DEFAULT_PITCH,
							 _T("Arial"));

	//创建标题的字体
	hTopicFont = CreateFont(260, 47, 10, 0, FW_BOLD, 0, 0, 0, ANSI_CHARSET,
							OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,
							DEFAULT_QUALITY,
							DEFAULT_PITCH,
							_T("Arial"));

	//创建字段的字体
	hCodeFont = CreateFont(150, 50, 1, 0, FW_BOLD, 0, 0, 0, ANSI_CHARSET,
						   OUT_DEFAULT_PRECIS,
						   CLIP_DEFAULT_PRECIS,
						   DEFAULT_QUALITY,
						   DEFAULT_PITCH,
						   _T("Arial"));

	//设置打印对话框
	PRINTDLG   pd;
	pd.lStructSize = sizeof(PRINTDLG);
	pd.Flags = PD_RETURNDC;
	pd.hDC = nullptr;
	pd.hwndOwner = nullptr;
	pd.hInstance = nullptr;
	pd.nMaxPage = 2;
	pd.nMinPage = 1;
	pd.nFromPage = 1;
	pd.nToPage = 1;
	pd.nCopies = 1;
	pd.hDevMode = nullptr;
	pd.hDevNames = nullptr;

	/////////////////////////////////////////////////////////
	//显示打印对话框，由用户来设定纸张大小等.
	if (!PrintDlg(&pd))   return   FALSE;
	ASSERT(pd.hDC != nullptr);/*断言获取的句柄不为空.*/
	int   nHorRes = GetDeviceCaps(pd.hDC, HORZRES);
	int   nVerRes = GetDeviceCaps(pd.hDC, VERTRES);
	int   nXMargin = 20;//页边的空白   
	int   nYMargin = 6;

	///////////////////////////////////////////////////////////
	TEXTMETRIC  tm;/*映射结构体*/
	GetTextMetrics(pd.hDC, &tm);
	int   nCharWidth = tm.tmAveCharWidth;
	int   ncaps = (tm.tmPitchAndFamily & 1 ? 3 : 2)*nCharWidth / 2;
	int   nCharHeight = tm.tmExternalLeading + tm.tmHeight + ncaps;//

	///////////////////////////////////////////////////////////
	CHeaderCtrl*   pHeader = list.GetHeaderCtrl();
	//获得行，列的个数
	int   nColCount = pHeader->GetItemCount();//获取列头的个数 
	int   nLineCount = list.GetSelectedCount(); //获取ListCtrl的记录行数
	int   ColOrderArray[100];
	COLATT   ca[100];
	list.GetColumnOrderArray(ColOrderArray, nColCount); //存储列头的索引值
	int   nColX = nXMargin*nCharWidth;

	////////////////////////////////////////////////////////////
	//检索各列的信息，确定列标题的内容长度。
	for (int i = 0; i < nColCount; i++) {
		ca[i].nColIndex = ColOrderArray[i];
		LVCOLUMN lvc;
		TCHAR text[100];
		lvc.mask = LVCF_TEXT | LVCF_SUBITEM;
		lvc.pszText = text;
		lvc.cchTextMax = 100;
		list.GetColumn(ca[i].nColIndex, &lvc);
		ca[i].strColText = lvc.pszText;
		ca[i].nSubItemIndex = lvc.iSubItem;
		ca[i].nPrintX = nColX;
		nColX += nCharWidth *  _tcslen(ca[i].strColText);

		/////////////////////////////////////////////////////////////
		if (nColX > nHorRes) {  //表示输出的列头名的位置已经超出了  
			DeleteDC(pd.hDC);
			CString e; e = GetStringFromAppResource(IDS_STRING_E_TOOLMANY_FIELD);
			MessageBox(e, L"", MB_ICONERROR);
			return  FALSE;
		}
	}

	//设置打印文件的保存对话框 
	CString fm;
	DOCINFO   di;
	di.cbSize = sizeof(DOCINFO);
	fm = GetStringFromAppResource(IDS_STRING_EXPIRE_DOC_NAME);
	di.lpszDocName = fm.LockBuffer();
	di.lpszOutput = (LPTSTR)nullptr;
	di.lpszDatatype = (LPTSTR)nullptr;
	di.fwType = 0;
	StartDoc(pd.hDC, &di);
	StartPage(pd.hDC);
	fm.UnlockBuffer();
	SelectObject(pd.hDC, hTopicFont);
	TextOut(pd.hDC, nHorRes / 3, nYMargin, fm, fm.GetLength());

	////////////////////////////////////////////////
	//调整各列的宽度，以使各列在后面的打印输出时更均匀的打印在纸上. 
	int   space = (nHorRes - nXMargin*nCharWidth - nColX) / (nColCount);
	for (int i = 1; i < nColCount; i++) {
		ca[i].nPrintX += i*space;
	}
	SelectObject(pd.hDC, hCodeFont);
	//输出列标题
	for (int i = 0; i < nColCount; i++) {
		TextOut(pd.hDC, ca[i].nPrintX, nYMargin + 260,
				ca[i].strColText, ca[i].strColText.GetLength());
	}

	int   nMaxLinePerPage = nVerRes / nCharHeight - 3;
	int   nCurPage = 1;
	SelectObject(pd.hDC, hRecordFont);//将字体选入设备描述表里

	//HWND hd = ::GetDesktopWindow();
	//HDC ddc = ::GetDC(hd);
	//输出各列的数据   
	int  nItem = -1;
	for (int i = 0; i < nLineCount; i++) {
		nItem = list.GetNextItem(nItem, LVNI_SELECTED);
		ASSERT(nItem != -1);
		for (int j = 0; j<nColCount; j++) {
			if (i + 1 - (nCurPage - 1)*nMaxLinePerPage > nMaxLinePerPage) {
				//新的一页   
				EndPage(pd.hDC);
				StartPage(pd.hDC);
				nCurPage++;
			}
			CString subitem = list.GetItemText(nItem, ca[j].nColIndex);
			JLOG(_T("%s\n"), subitem);
			TextOut(pd.hDC, ca[j].nPrintX,
					nYMargin + 300 + (i + 1 - (nCurPage - 1)*nMaxLinePerPage)*nCharHeight,
					subitem, _tcslen(subitem));
			//  DrawText(ddc, ca[j].nPrintX,     
			//	nYMargin+300+(i+1-(nCurPage-1)*nMaxLinePerPage)*nCharHeight,     
			//	subitem, _tcslen(subitem));   
		}
	}
	//::ReleaseDC(hd, ddc);
	EndPage(pd.hDC);
	EndDoc(pd.hDC);

	//打印结束
	DeleteObject(hTopicFont);
	DeleteObject(hRecordFont);
	DeleteObject(hCodeFont);
	DeleteDC(pd.hDC);

	return  TRUE;
}


void CMachineExpireManagerDlg::PrintRecordOnGrid()
{
	auto set = m_grid.GetSelectedRows();
	if (set.empty()) {
		JLOG(_T("No items were selected!\n"));
		CString e; e = GetStringFromAppResource(IDS_STRING_NO_SELD_CONTENT);
		MessageBox(e, L"", MB_ICONERROR);
		return;
	}

	HFONT hRecordFont;//记录的字体
	HFONT hTopicFont;//标题的字体
	HFONT hCodeFont;//字段的字体

					//创建(输出内容的)字体
	hRecordFont = CreateFont(93, 29, 1, 0, FW_EXTRALIGHT, 0, 0, 0, ANSI_CHARSET,
							 OUT_DEFAULT_PRECIS,
							 CLIP_DEFAULT_PRECIS,
							 DEFAULT_QUALITY,
							 DEFAULT_PITCH,
							 _T("Arial"));

	//创建标题的字体
	hTopicFont = CreateFont(260, 47, 10, 0, FW_BOLD, 0, 0, 0, ANSI_CHARSET,
							OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,
							DEFAULT_QUALITY,
							DEFAULT_PITCH,
							_T("Arial"));

	//创建字段的字体
	hCodeFont = CreateFont(150, 50, 1, 0, FW_BOLD, 0, 0, 0, ANSI_CHARSET,
						   OUT_DEFAULT_PRECIS,
						   CLIP_DEFAULT_PRECIS,
						   DEFAULT_QUALITY,
						   DEFAULT_PITCH,
						   _T("Arial"));

	//设置打印对话框
	PRINTDLG   pd;
	pd.lStructSize = sizeof(PRINTDLG);
	pd.Flags = PD_RETURNDC;
	pd.hDC = nullptr;
	pd.hwndOwner = nullptr;
	pd.hInstance = nullptr;
	pd.nMaxPage = 2;
	pd.nMinPage = 1;
	pd.nFromPage = 1;
	pd.nToPage = 1;
	pd.nCopies = 1;
	pd.hDevMode = nullptr;
	pd.hDevNames = nullptr;

	/////////////////////////////////////////////////////////
	//显示打印对话框，由用户来设定纸张大小等.
	if (!PrintDlg(&pd))   return;
	ASSERT(pd.hDC != nullptr);/*断言获取的句柄不为空.*/
	int   nHorRes = GetDeviceCaps(pd.hDC, HORZRES);
	int   nVerRes = GetDeviceCaps(pd.hDC, VERTRES);
	int   nXMargin = 20;//页边的空白   
	int   nYMargin = 6;

	///////////////////////////////////////////////////////////
	TEXTMETRIC  tm;/*映射结构体*/
	GetTextMetrics(pd.hDC, &tm);
	int   nCharWidth = tm.tmAveCharWidth;
	int   ncaps = (tm.tmPitchAndFamily & 1 ? 3 : 2)*nCharWidth / 2;
	int   nCharHeight = tm.tmExternalLeading + tm.tmHeight + ncaps;//

																   ///////////////////////////////////////////////////////////
	//CHeaderCtrl*   pHeader = list.GetHeaderCtrl();
	//获得行，列的个数
	int   nColCount = m_grid.GetColumnCount();//获取列头的个数 
	int   ColOrderArray[100];
	COLATT   ca[100];
	//CListCtrl::GetColumnOrderArray(ColOrderArray, nColCount); //存储列头的索引值
	for (auto i = 0; i < (int)set.size(); i++) {
		ColOrderArray[i] = i;
	}
	int   nColX = nXMargin*nCharWidth;

	////////////////////////////////////////////////////////////
	//检索各列的信息，确定列标题的内容长度。
	for (int i = 0; i < nColCount; i++) {
		ca[i].nColIndex = ColOrderArray[i];
		//LVCOLUMN lvc;
		//TCHAR text[100];
		//lvc.mask = LVCF_TEXT | LVCF_SUBITEM;
		//lvc.pszText = text;
		//lvc.cchTextMax = 100;
		//list.GetColumn(ca[i].nColIndex, &lvc);
		ca[i].strColText = m_grid.GetCell(0, i)->GetText();
		ca[i].nSubItemIndex = i;
		ca[i].nPrintX = nColX;
		nColX += nCharWidth *  _tcslen(ca[i].strColText);

		/////////////////////////////////////////////////////////////
		if (nColX > nHorRes) {  //表示输出的列头名的位置已经超出了  
			DeleteDC(pd.hDC);
			CString e; e = GetStringFromAppResource(IDS_STRING_E_TOOLMANY_FIELD);
			MessageBox(e, L"", MB_ICONERROR);
			return;
		}
	}

	//设置打印文件的保存对话框 
	CString fm;
	DOCINFO   di;
	di.cbSize = sizeof(DOCINFO);
	fm = GetStringFromAppResource(IDS_STRING_EXPIRE_DOC_NAME);
	di.lpszDocName = fm.LockBuffer();
	di.lpszOutput = (LPTSTR)nullptr;
	di.lpszDatatype = (LPTSTR)nullptr;
	di.fwType = 0;
	StartDoc(pd.hDC, &di);
	StartPage(pd.hDC);
	fm.UnlockBuffer();
	SelectObject(pd.hDC, hTopicFont);
	TextOut(pd.hDC, nHorRes / 3, nYMargin, fm, fm.GetLength());

	////////////////////////////////////////////////
	//调整各列的宽度，以使各列在后面的打印输出时更均匀的打印在纸上. 
	int   space = (nHorRes - nXMargin*nCharWidth - nColX) / (nColCount);
	for (int i = 1; i < nColCount; i++) {
		ca[i].nPrintX += i*space;
	}
	SelectObject(pd.hDC, hCodeFont);
	//输出列标题
	for (int i = 0; i < nColCount; i++) {
		TextOut(pd.hDC, ca[i].nPrintX, nYMargin + 260,
				ca[i].strColText, ca[i].strColText.GetLength());
	}

	int   nMaxLinePerPage = nVerRes / nCharHeight - 3;
	int   nCurPage = 1;
	SelectObject(pd.hDC, hRecordFont);//将字体选入设备描述表里

									  //HWND hd = ::GetDesktopWindow();
									  //HDC ddc = ::GetDC(hd);
									  //输出各列的数据   
	//int  nItem = -1;
	for (auto row : set) {
		//nItem = list.GetNextItem(nItem, LVNI_SELECTED);
		//ASSERT(nItem != -1);

		for (int col = 0; col < nColCount; col++) {
			if (row + 1 - (nCurPage - 1)*nMaxLinePerPage > nMaxLinePerPage) {
				//新的一页   
				EndPage(pd.hDC);
				StartPage(pd.hDC);
				nCurPage++;
			}
			auto cell = m_grid.GetCell(row, col);
			CString txt = cell->GetText();
			TextOut(pd.hDC, ca[col].nPrintX,
					nYMargin + 300 + (row + 1 - (nCurPage - 1)*nMaxLinePerPage)*nCharHeight,
					txt, txt.GetLength());
			//  DrawText(ddc, ca[j].nPrintX,     
			//	nYMargin+300+(i+1-(nCurPage-1)*nMaxLinePerPage)*nCharHeight,     
			//	subitem, _tcslen(subitem));   
		}
	}
	//::ReleaseDC(hd, ddc);
	EndPage(pd.hDC);
	EndDoc(pd.hDC);

	//打印结束
	DeleteObject(hTopicFont);
	DeleteObject(hRecordFont);
	DeleteObject(hCodeFont);
	DeleteDC(pd.hDC);
}


void CMachineExpireManagerDlg::OnBnClickedButtonPrintSel() 
{
	AUTO_LOG_FUNCTION;
	
#ifdef USE_MFC_GRID_CTRL
	PrintRecordOnGrid();
#else
	PrintRecord(m_list);
#endif
}

namespace detail {
	typedef struct my_compare_struct {
		bool bsubmachine;
		bool basc;
		int isubitem;
		int ademco_id;
		//LPARAM machine;
	}my_compare_struct;

	//int __stdcall my_compare_func(LPARAM lp1, LPARAM lp2, LPARAM lp3)
	//{
	//	alarm_machine_ptr machine1;
	//	alarm_machine_ptr machine2;
	//	auto mgr = alarm_machine_manager::GetInstance();
	//	my_compare_struct* m = reinterpret_cast<my_compare_struct*>(lp3);
	//	if (m->bsubmachine) {
	//		auto machine = mgr->GetMachine(m->ademco_id);
	//		machine1 = machine->GetZone(lp1)->GetSubMachineInfo();
	//		machine2 = machine->GetZone(lp2)->GetSubMachineInfo();
	//	} else {
	//		machine1 = mgr->GetMachine(lp1);
	//		machine2 = mgr->GetMachine(lp2);
	//	}
	//	int ret = 0;
	//	column_header header;
	//	switch (header) {
	//	case detail::col_aid:
	//		if (machine1->get_is_submachine()) {
	//			ret = machine1->get_submachine_zone() - machine2->get_submachine_zone();
	//		} else {
	//			ret = machine1->get_ademco_id() - machine2->get_ademco_id();
	//		}
	//		break;
	//	case detail::col_alias:
	//		ret = machine1->get_machine_name().Compare(machine2->get_machine_name());
	//		break;
	//	case detail::col_expire_time:
	//		COleDateTimeSpan span = machine1->get_expire_time() - machine2->get_expire_time();
	//		double minutes = span.GetTotalMinutes();
	//		if (minutes > 0)
	//			ret = 1;
	//		else if (minutes < 0)
	//			ret = -1;
	//		break;
	//	case detail::col_is_expired:
	//		break;
	//	case detail::col_remind_time:
	//		break;
	//	case detail::col_receivable:
	//		break;
	//	case detail::col_paid:
	//		break;
	//	case detail::col_owed:
	//		break;
	//	case detail::col_is_owed:
	//		break;
	//	case detail::col_contact:
	//		break;
	//	case detail::col_addr:
	//		break;
	//	case detail::col_phone:
	//		break;
	//	case detail::col_phone_bk:
	//		break;
	//	case detail::col_count:
	//		break;
	//	default:
	//		break;
	//	}
	//	switch (m->isubitem) {
	//	case 0: // id
	//		
	//		break;
	//	case 1: // alias
	//		
	//		break;
	//	case 2: // expire time
	//	case 3: // if expire
	//		//ret = machine1->get_left_service_time_in_minutes() - machine2->get_left_service_time_in_minutes();
	//	{
	//		
	//	}
	//	break;
	//	case 4: // contact
	//		ret = machine1->get_contact().Compare(machine2->get_contact());
	//		break;
	//	case 5: // address
	//		ret = machine1->get_address().Compare(machine2->get_address());
	//		break;
	//	case 6: // phone
	//		ret = machine1->get_phone().Compare(machine2->get_phone());
	//		break;
	//	case 7: // phone_bk
	//		ret = machine1->get_phone_bk().Compare(machine2->get_phone_bk());
	//		break;
	//	default:
	//		break;
	//	}
	//	ret = m->basc ? ret : -ret;
	//	return ret;
	//}
};

void CMachineExpireManagerDlg::OnLvnColumnclickList1(NMHDR *pNMHDR, LRESULT *pResult) 
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	static bool basc = true;
	detail::my_compare_struct mcs;
	mcs.bsubmachine = m_bSubMachine;
	mcs.basc = basc;
	mcs.isubitem = pNMLV->iSubItem;
	if (m_bSubMachine) {
		mcs.ademco_id = m_machine->get_ademco_id();
	}
	//mcs.machine = pNMLV->lParam;
#ifdef USE_MFC_GRID_CTRL
	assert(0);
	//m_grid.SortItems()
#else
	m_list.SortItems(detail::my_compare_func, reinterpret_cast<DWORD_PTR>(&mcs));
#endif
	basc = !basc;
	*pResult = 0;
}



void CMachineExpireManagerDlg::OnGridStartEdit(NMHDR *pNotifyStruct,
							   LRESULT* pResult)
{
	NM_GRIDVIEW* pItem = (NM_GRIDVIEW*)pNotifyStruct;

	auto AllowCellToBeEdited = [this](int /*row*/, int column) {
		bool b_edit = true;
		CPoint pos;
		GetCursorPos(&pos);
		switch (column) {
		case detail::col_aid:
			b_edit = false;
			break;
		case detail::col_alias:
			break;
		case detail::col_type:
			SetType(pos);
			b_edit = false;
			break;
		case detail::col_expire_time:
			SetExpireTime(pos);
			b_edit = false;
			break;
		case detail::col_is_expired:
			b_edit = false;
			break;
		case detail::col_remind_time:
			SetRemindTime(pos);
			b_edit = false;
			break;
		case detail::col_receivable:
			break;
		case detail::col_paid:
			break;
		case detail::col_owed:
			b_edit = false;
			break;
		case detail::col_is_owed:
			b_edit = false;
			break;
		case detail::col_contact:
			break;
		case detail::col_addr:
			break;
		case detail::col_phone:
			break;
		case detail::col_phone_bk:
			break;
		case detail::col_count:
		default:
			b_edit = false;
			break;
		}

		return b_edit;
	};

	// AllowCellToBeEdited is a fictional routine that should return TRUE 
	// if you want to allow the cell to be edited.
	BOOL bAllowEdit = AllowCellToBeEdited(pItem->iRow, pItem->iColumn);

	*pResult = (bAllowEdit) ? 0 : -1;
}



void CMachineExpireManagerDlg::OnGridEndEdit(NMHDR *pNotifyStruct, LRESULT* pResult)
{
	NM_GRIDVIEW* pItem = (NM_GRIDVIEW*)pNotifyStruct;

	BOOL bAcceptChange = TRUE;

	auto cell = m_grid.GetCell(pItem->iRow, pItem->iColumn);
	if (bAcceptChange && cell) {
		CString txt = cell->GetText();
		TRACE(L"end edit: %d, %d, %s\n", pItem->iRow, pItem->iColumn, txt);

		bAcceptChange = UpdateMachineInfo(pItem->iRow, pItem->iColumn, txt);
		if (bAcceptChange) {
			m_grid.Refresh();
		}
	}

	*pResult = (bAcceptChange) ? 0 : -1;
}


BOOL CMachineExpireManagerDlg::UpdateMachineInfo(int row, int col, const CString& txt)
{
	int data = m_grid.GetItemData(row, 0);

	alarm_machine_ptr machine = nullptr;
	zone_info_ptr zone = nullptr;
	if (m_bSubMachine) {
		zone = m_machine->GetZone(data);
		if (!zone) {
			assert(0); return false;
		}
	} else {
		machine = alarm_machine_manager::GetInstance()->GetMachine(data);
		if (!machine) {
			assert(0); return false;
		}
	}

	bool ok = false;
	auto consumer_mgr = consumer_manager::GetInstance();

	switch (col) {
	case detail::col_alias:
		ok = m_bSubMachine ? zone->execute_update_alias(txt) : machine->execute_set_alias(txt);
		m_bUpdatedMachineName = true;
		break;
	case detail::col_receivable:
	case detail::col_paid:
	{
		auto a_consumer = machine->get_consumer();
		int receivable_amount = a_consumer->receivable_amount;
		int paid_amount = a_consumer->paid_amount;
		if (col == col_receivable) {
			receivable_amount = _ttoi(txt);
		} else if (col == col_paid) {
			paid_amount = _ttoi(txt);
		}

		if (a_consumer->paid_amount == paid_amount  && a_consumer->receivable_amount == receivable_amount) {
			break;
		}

		auto tmp = std::make_shared<consumer>(*a_consumer);
		tmp->receivable_amount = receivable_amount;
		tmp->paid_amount = paid_amount;

		if (consumer_mgr->execute_update_consumer(tmp)) {
			machine->set_consumer(tmp);
			CString s;
			s.Format(L"%d", tmp->get_owed_amount());
			m_grid.SetItemText(row, col_owed, s);
			m_grid.SetItemText(row, col_is_owed, GetStringFromAppResource(tmp->get_owed_amount() > 0 ? IDS_STRING_YES : IDS_STRING_NO));
			ok = true;
		}
	}
		break;
	case detail::col_contact:
		ok = m_bSubMachine ? zone->execute_update_contact(txt) : machine->execute_set_contact(txt);
		break;
	case detail::col_addr:
		ok = m_bSubMachine ? zone->execute_update_address(txt) : machine->execute_set_address(txt);
		break;
	case detail::col_phone:
		ok = m_bSubMachine ? zone->execute_update_phone(txt) : machine->execute_set_phone(txt);
		break;
	case detail::col_phone_bk:
		ok = m_bSubMachine ? zone->execute_update_phone_bk(txt) : machine->execute_set_phone_bk(txt);
		break;
	default:
		break;
	}

	return ok;
}


void CMachineExpireManagerDlg::OnGridItemChanged(NMHDR *pNotifyStruct, LRESULT* pResult)
{
	NM_GRIDVIEW* pItem = (NM_GRIDVIEW*)pNotifyStruct;

	auto AcceptChange = [](int row, int /*column*/) {
		return row != 0;
	};

	// AcceptChange is a fictional routine that should return TRUE
	// if you want to accept the new value for the cell.
	//BOOL bAcceptChange = AcceptChange(pItem->iRow, pItem->iColumn);

	auto the_cell = m_grid.GetCell(pItem->iRow, pItem->iColumn);
	if (the_cell) {
		CString txt = the_cell->GetText();
		auto list = m_grid.GetSelectedCellList();
		std::set<int> set;
		for (auto cell : list) {
			set.insert(cell.row);
		}

		TRACE(L"Item changed: %d, %d, %s, count %d\n", pItem->iRow, pItem->iColumn, txt, set.size());

		CString s; s.Format(L"%d", set.size());
		m_staticSeldLineNum.SetWindowTextW(s);
	}

	*pResult = 0;
}


void CMachineExpireManagerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	RepositionItems();
}


void CMachineExpireManagerDlg::RepositionItems()
{
	static const int gap = 10;
	static const int small_button_width = 75;
	static const int big_button_width = 180;

	if (!m_b_initialized_)
		return;

	CRect rc;
	GetClientRect(rc);
	CRect grid_rc(rc);
	grid_rc.bottom -= 40;

	m_grid.MoveWindow(grid_rc);

	CRect line(rc);
	line.top = grid_rc.bottom + gap;
	line.bottom -= gap;

	line.right = line.left + small_button_width;
	m_btn_all.MoveWindow(line);

	line.left = line.right + gap;
	line.right = line.left + small_button_width;
	m_btn_all_not.MoveWindow(line);

	line.left = line.right + gap;
	line.right = line.left + 80;
	m_static_label.MoveWindow(line);

	line.left = line.right + gap;
	line.right = line.left + 50;
	m_staticSeldLineNum.MoveWindow(line);

	line.left = line.right + gap;
	line.right = line.left + big_button_width;
	m_btn_export_sel_to_excel.MoveWindow(line);

	line.left = line.right + gap;
	line.right = line.left + big_button_width;
	m_btn_print_sel.MoveWindow(line);

	line.left = line.right + gap * 2;
	line.right = line.left + small_button_width;
	m_btn_type_manager.MoveWindow(line);

	line.left = line.right + gap;
	line.right = line.left + big_button_width;
	m_btn_set_type.MoveWindow(line);
	
	line.left = line.right + gap;
	line.right = line.left + big_button_width;
	m_btn_set_sel_remind_time.MoveWindow(line);

	line.left = line.right + gap;
	line.right = line.left + big_button_width;
	m_btn_extend_sel_expired_time.MoveWindow(line);
}


void CMachineExpireManagerDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialogEx::OnSizing(fwSide, pRect);

	// TODO: Add your message handler code here
}


void CMachineExpireManagerDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize.x = 800;
	lpMMI->ptMinTrackSize.y = 600;

	CDialogEx::OnGetMinMaxInfo(lpMMI);
}


void CMachineExpireManagerDlg::SetRemindTime(CPoint pos)
{
	if (m_grid.GetSelectedCount() == 0)
		return;

	CMenu menu, *sub;
	menu.LoadMenuW(IDR_MENU5);
	sub = menu.GetSubMenu(0); assert(sub); if (!sub) return;	
	DWORD ret = sub->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
									pos.x, pos.y, this);
	JLOG(L"TrackPopupMenu ret %d\n", ret);

	if (ret == 0)return;

	auto user_set_remind_time = std::chrono::system_clock::now();
	if (ret == ID_S_USER_SET) {
		CExtendExpireTimeDlg dlg(this); if (IDOK != dlg.DoModal()) return;
		user_set_remind_time = dlg.m_dateTime;
	}

	auto mgr = alarm_machine_manager::GetInstance();
	auto set = m_grid.GetSelectedRows();
	for (auto row : set) {
		DWORD data = m_grid.GetItemData(row, detail::DEFAULT_GRID_COLOMN_INDEX_TO_STORAGE_ITEM_DATA);
		alarm_machine_ptr machine;
		if (m_bSubMachine)
			machine = m_machine->GetZone(data)->GetSubMachineInfo();
		else
			machine = mgr->GetMachine(data);

		auto remind_time = machine->get_expire_time();

		switch (ret) {
		case ID_S_DAY:
			break;

		case ID_S_WEEK:
		{
			remind_time -= std::chrono::hours(24) * 7;
		}
		break;

		case ID_S_MONTH:
		{
			remind_time -= std::chrono::hours(24) * 30;
		}
		break;

		case ID_S_USER_SET:
		{
			remind_time = user_set_remind_time;
		}
		break;

		}

		auto a_consumer = machine->get_consumer();
		if (a_consumer->remind_time == remind_time) return;

		auto tmp = std::make_shared<consumer>(*a_consumer);
		tmp->remind_time = remind_time;
		if (consumer_manager::GetInstance()->execute_update_consumer(tmp)) {
			m_grid.SetItemText(row, col_remind_time, time_point_to_wstring(remind_time).c_str());
			machine->set_consumer(tmp);
		}
	}


	m_grid.Refresh();
}


void CMachineExpireManagerDlg::OnBnClickedButtonSetRemindTime()
{
	CRect rc;
	m_btn_set_sel_remind_time.GetWindowRect(rc);
	SetRemindTime(CPoint(rc.left, rc.bottom));
}


void CMachineExpireManagerDlg::OnBnClickedButtonTypeManager()
{
	CConsumerTypeMgrDlg dlg;
	dlg.DoModal();
	InitializeGrid();
	OnBnClickedButtonAllNot();
}


void CMachineExpireManagerDlg::SetType(CPoint pos)
{
	if (m_grid.GetSelectedCount() == 0)
		return;

	CMenu menu;
	menu.CreatePopupMenu();

	std::vector<int> vMenu;
	int ndx = 1;
	vMenu.push_back(0); // place holder
	auto consumer_mgr = consumer_manager::GetInstance();
	auto types = consumer_mgr->get_all_types();
	for (auto type : types) {
		menu.AppendMenuW(MF_STRING, ndx, type.second->name);
		vMenu.push_back(type.second->id);
		ndx++;
	}

	menu.AppendMenuW(MF_STRING, ndx, GetStringFromAppResource(IDS_STRING_USER_DEFINE));

	int ret = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
									pos.x, pos.y, this);
	JLOG(L"TrackPopupMenu ret %d\n", ret);

	if (ret == 0 || ret > (int)vMenu.size())return;

	int type_id = 0;
	if (ret == ndx) { // user define
		CInputContentDlg dlg(this);
		dlg.m_title = GetStringFromAppResource(IDS_STRING_INPUT_TYPE);
		if (dlg.DoModal() != IDOK) return;
		if (!consumer_mgr->execute_add_type(type_id, dlg.m_value)) {
			assert(0); return;
		}
	} else { // choose from existing types
		type_id = vMenu[ret];
	}

	auto type = consumer_mgr->get_consumer_type_by_id(type_id);
	if (!type) {
		assert(0); return;
	}

	auto mgr = alarm_machine_manager::GetInstance();
	auto set = m_grid.GetSelectedRows();
	for (auto row : set) {
		DWORD data = m_grid.GetItemData(row, detail::DEFAULT_GRID_COLOMN_INDEX_TO_STORAGE_ITEM_DATA);
		alarm_machine_ptr machine;
		if (m_bSubMachine)
			machine = m_machine->GetZone(data)->GetSubMachineInfo();
		else
			machine = mgr->GetMachine(data);

		machine->get_consumer()->type = type;

		if (!consumer_mgr->execute_update_consumer(machine->get_consumer())) {
			assert(0);
		}

		m_grid.SetItemText(row, col_type, type->name);
	}

	m_grid.Refresh();
}


void CMachineExpireManagerDlg::OnBnClickedButtonSetType()
{
	CRect rc;
	m_btn_set_type.GetWindowRect(rc);
	SetType(CPoint(rc.left, rc.bottom));
}
