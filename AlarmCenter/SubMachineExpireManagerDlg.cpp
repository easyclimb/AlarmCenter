// SubMachineExpireManagerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "SubMachineExpireManagerDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "AlarmMachineManager.h"
#include "ZoneInfo.h"
#include "ExtendExpireTimeDlg.h"
#include <iterator>
#include <odbcinst.h>
#include <afxdb.h>
#include <comdef.h>
#include <set>
using namespace core;
using namespace gui::control::grid_ctrl;

namespace detail {

static const int DEFAULT_GRID_COLOMN_INDEX_TO_STORAGE_ITEM_DATA = 0;


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


void CMachineExpireManagerDlg::OnBnClickedButtonExtend() 
{
#ifdef USE_MFC_GRID_CTRL
	if (m_grid.GetSelectedCount() == 0)
		return;

	CExtendExpireTimeDlg dlg(this);
	if (dlg.DoModal() != IDOK)
		return;

	CString syes, sno; syes = GetStringFromAppResource(IDS_STRING_YES); sno = GetStringFromAppResource(IDS_STRING_NO);
	auto mgr = CAlarmMachineManager::GetInstance();
	auto set = m_grid.GetSelectedRows();
	for (auto row : set) {
		DWORD data = m_grid.GetItemData(row, detail::DEFAULT_GRID_COLOMN_INDEX_TO_STORAGE_ITEM_DATA);
		alarm_machine_ptr machine;
		if (m_bSubMachine)
			machine = m_machine->GetZone(data)->GetSubMachineInfo();
		else
			machine = mgr->GetMachine(data);
		if (machine && machine->execute_update_expire_time(dlg.m_dateTime)) {
			m_grid.SetItemText(row, 2, dlg.m_dateTime.Format(L"%Y-%m-%d %H:%M:%S"));
			m_grid.SetItemText(row, 3, machine->get_left_service_time() <= 0 ? syes : sno);
		}
	}
	m_grid.Refresh();

#else
	if (m_list.GetSelectedCount() == 0)
		return;

	CExtendExpireTimeDlg dlg(this);
	if (dlg.DoModal() != IDOK)
		return;

	CString syes, sno; syes = GetStringFromAppResource(IDS_STRING_YES); sno = GetStringFromAppResource(IDS_STRING_NO);
	int ndx = -1;
	auto mgr = CAlarmMachineManager::GetInstance();
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
			m_list.SetItemText(ndx, 3, machine->get_left_service_time() <= 0 ? syes : sno);
		}
	}
#endif
}


BOOL CMachineExpireManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

#ifdef USE_MFC_GRID_CTRL
	m_grid.SetEditable(true);
	m_grid.SetTextBkColor(RGB(0xFF, 0xFF, 0xE0));//yellow background
	m_grid.SetRowCount(m_expiredMachineList.size() + 1);
	m_grid.SetColumnCount(8); 
	m_grid.SetFixedRowCount(1); 
	//m_grid.SetFixedColumnCount(1); 
	m_grid.SetListMode();

	// 设置表头
	for (int col = 0; col < m_grid.GetColumnCount(); col++) {
		GV_ITEM item;
		item.mask = GVIF_TEXT | GVIF_FORMAT;
		item.row = 0;
		item.col = col;
		item.nFormat = DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS;

		m_grid.SetRowHeight(0, 25); //set row heigh          

		switch (col) {
		case 0:
			item.strText = m_bSubMachine ? GetStringFromAppResource(IDS_STRING_SUBMACHINE) : GetStringFromAppResource(IDS_STRING_MACHINE);
			m_grid.SetColumnWidth(col, 50);
			break;

		case 1:
			item.strText = GetStringFromAppResource(IDS_STRING_ALIAS);
			m_grid.SetColumnWidth(col, 200);
			break;

		case 2:
			item.strText = GetStringFromAppResource(IDS_STRING_EXPIRE_TIME);
			m_grid.SetColumnWidth(col, 125);
			break;

		case 3:
			item.strText = GetStringFromAppResource(IDS_STRING_IF_EXPIRE);
			m_grid.SetColumnWidth(col, 75);
			break;

		case 4:
			item.strText = GetStringFromAppResource(IDS_STRING_CONTACT);
			m_grid.SetColumnWidth(col, 75);
			break;

		case 5:
			item.strText = GetStringFromAppResource(IDS_STRING_ADDRESS);
			m_grid.SetColumnWidth(col, 225);
			break;

		case 6:
			item.strText = GetStringFromAppResource(IDS_STRING_PHONE);
			m_grid.SetColumnWidth(col, 150);
			break;

		case 7:
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
		item.strText.Format(_T("%s"), machine->get_machine_name());
		m_grid.SetItem(&item);

		// expire time
		item.col++;
		item.strText.Format(_T("%s"), machine->get_expire_time().Format(L"%Y-%m-%d %H:%M:%S"));
		m_grid.SetItem(&item);

		// if expire 
		CString syes, sno; syes = GetStringFromAppResource(IDS_STRING_YES); sno = GetStringFromAppResource(IDS_STRING_NO);
		item.col++;
		item.strText.Format(_T("%s"), machine->get_left_service_time() <= 0 ? syes : sno);
		m_grid.SetItem(&item);

		// contact
		item.col++;
		item.strText.Format(_T("%s"), machine->get_contact());
		m_grid.SetItem(&item);

		// address
		item.col++;
		item.strText.Format(_T("%s"), machine->get_address());
		m_grid.SetItem(&item);

		// phone
		item.col++;
		item.strText.Format(_T("%s"), machine->get_phone());
		m_grid.SetItem(&item);

		// phone_bk
		item.col++;
		item.strText.Format(_T("%s"), machine->get_phone_bk());
		m_grid.SetItem(&item);

		row++;
	}
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
//		tmp.Format(_T("%s"), machine->get_left_service_time() <= 0 ? syes : sno);
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
	auto mgr = CAlarmMachineManager::GetInstance();

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
			svalues.Format(_T("VALUES('%d','%s','%s','%s','%s','%s','%s','%s')"), 
						machine->get_is_submachine() ? machine->get_submachine_zone() : machine->get_ademco_id(),
						machine->get_machine_name(), 
						machine->get_expire_time().Format(L"%Y-%m-%d %H:%M:%S"),
						machine->get_left_service_time() <= 0 ? syes : sno,
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

	int __stdcall my_compare_func(LPARAM lp1, LPARAM lp2, LPARAM lp3)
	{
		alarm_machine_ptr machine1;
		alarm_machine_ptr machine2;
		auto mgr = CAlarmMachineManager::GetInstance();
		my_compare_struct* m = reinterpret_cast<my_compare_struct*>(lp3);
		if (m->bsubmachine) {
			auto machine = mgr->GetMachine(m->ademco_id);
			machine1 = machine->GetZone(lp1)->GetSubMachineInfo();
			machine2 = machine->GetZone(lp2)->GetSubMachineInfo();
		} else {
			machine1 = mgr->GetMachine(lp1);
			machine2 = mgr->GetMachine(lp2);
		}
		int ret = 0;
		switch (m->isubitem) {
		case 0: // id
			if (machine1->get_is_submachine()) {
				ret = machine1->get_submachine_zone() - machine2->get_submachine_zone();
			} else {
				ret = machine1->get_ademco_id() - machine2->get_ademco_id();
			}
			break;
		case 1: // alias
			ret = machine1->get_machine_name().Compare(machine2->get_machine_name());
			break;
		case 2: // expire time
		case 3: // if expire
			//ret = machine1->get_left_service_time() - machine2->get_left_service_time();
		{
			COleDateTimeSpan span = machine1->get_expire_time() - machine2->get_expire_time();
			double minutes = span.GetTotalMinutes();
			if (minutes > 0)
				ret = 1;
			else if (minutes < 0)
				ret = -1;
		}
		break;
		case 4: // contact
			ret = machine1->get_contact().Compare(machine2->get_contact());
			break;
		case 5: // address
			ret = machine1->get_address().Compare(machine2->get_address());
			break;
		case 6: // phone
			ret = machine1->get_phone().Compare(machine2->get_phone());
			break;
		case 7: // phone_bk
			ret = machine1->get_phone_bk().Compare(machine2->get_phone_bk());
			break;
		default:
			break;
		}
		ret = m->basc ? ret : -ret;
		return ret;
	}
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
		if (column == 0 || column == 3)
			return FALSE;

		if (column == 2) {
			OnBnClickedButtonExtend();
			return FALSE;
		}

		return TRUE;
	};

	// AllowCellToBeEdited is a fictional routine that should return TRUE 
	// if you want to allow the cell to be edited.
	BOOL bAllowEdit = AllowCellToBeEdited(pItem->iRow, pItem->iColumn);

	*pResult = (bAllowEdit) ? 0 : -1;
}



void CMachineExpireManagerDlg::OnGridEndEdit(NMHDR *pNotifyStruct, LRESULT* pResult)
{
	NM_GRIDVIEW* pItem = (NM_GRIDVIEW*)pNotifyStruct;

	auto AcceptChange = [](/*CGridCtrl& ctrl, */int /*row*/, int column) {
		do {
			if (column == 0 || column == 3) { // ademco id, yes/no
				break;
			}

			/*auto mgr = core::CAlarmMachineManager::GetInstance();
			auto machine = mgr->GetMachine(ctrl.GetItemData(row, 0));
			if (!machine) {
				break;
			}*/


			return true;
		} while (false);

		return false;
	};

	// AcceptChange is a fictional routine that should return TRUE
	// if you want to accept the new value for the cell.
	BOOL bAcceptChange = AcceptChange(/*m_grid, */pItem->iRow, pItem->iColumn);

	auto cell = m_grid.GetCell(pItem->iRow, pItem->iColumn);
	if (bAcceptChange && cell) {
		CString txt = cell->GetText();
		TRACE(L"end edit: %d, %d, %s\n", pItem->iRow, pItem->iColumn, txt);

		bAcceptChange = UpdateMachineInfo(pItem->iRow, pItem->iColumn, txt);
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
		machine = CAlarmMachineManager::GetInstance()->GetMachine(data);
		if (!machine) {
			assert(0); return false;
		}
	}

	bool ok = false;

	switch (col) {
	case 1:
		ok = m_bSubMachine ? zone->execute_update_alias(txt) : machine->execute_set_alias(txt);
		m_bUpdatedMachineName = true;
		break;

	case 4:
		ok = m_bSubMachine ? zone->execute_update_contact(txt) : machine->execute_set_contact(txt);
		break;

	case 5:
		ok = m_bSubMachine ? zone->execute_update_address(txt) : machine->execute_set_address(txt);
		break;

	case 6:
		ok = m_bSubMachine ? zone->execute_update_phone(txt) : machine->execute_set_phone(txt);
		break;

	case 7:
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


