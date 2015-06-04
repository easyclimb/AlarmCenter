// SubMachineExpireManagerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "SubMachineExpireManagerDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "ZoneInfo.h"
#include "ExtendExpireTimeDlg.h"
#include <iterator>
#include <odbcinst.h>
#include <afxdb.h>
#include <comdef.h>
using namespace core;

// CMachineExpireManagerDlg 对话框

IMPLEMENT_DYNAMIC(CMachineExpireManagerDlg, CDialogEx)

CMachineExpireManagerDlg::CMachineExpireManagerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMachineExpireManagerDlg::IDD, pParent)
	//, m_machine(NULL)
{

}

CMachineExpireManagerDlg::~CMachineExpireManagerDlg()
{
}

void CMachineExpireManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Control(pDX, IDC_STATIC_LINE_NUM, m_staticSeldLineNum);
}


BEGIN_MESSAGE_MAP(CMachineExpireManagerDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CMachineExpireManagerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CMachineExpireManagerDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_EXTEND, &CMachineExpireManagerDlg::OnBnClickedButtonExtend)
	ON_BN_CLICKED(IDC_BUTTON_ALL, &CMachineExpireManagerDlg::OnBnClickedButtonAll)
	ON_BN_CLICKED(IDC_BUTTON_ALL_NOT, &CMachineExpireManagerDlg::OnBnClickedButtonAllNot)
	ON_BN_CLICKED(IDC_BUTTON_INVERT, &CMachineExpireManagerDlg::OnBnClickedButtonInvert)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST1, &CMachineExpireManagerDlg::OnNMCustomdrawList1)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, &CMachineExpireManagerDlg::OnNMClickList1)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT_SEL, &CMachineExpireManagerDlg::OnBnClickedButtonExportSel)
	ON_BN_CLICKED(IDC_BUTTON_PRINT_SEL, &CMachineExpireManagerDlg::OnBnClickedButtonPrintSel)
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


void CMachineExpireManagerDlg::SetExpiredMachineList(std::list<core::CAlarmMachine*>& list)
{
	std::copy(list.begin(), list.end(), std::back_inserter(m_expiredMachineList));
}


void CMachineExpireManagerDlg::OnBnClickedButtonExtend() 
{
	if (m_list.GetSelectedCount() == 0)
		return;

	CExtendExpireTimeDlg dlg;
	if (dlg.DoModal() != IDOK)
		return;

	int ndx = -1;
	for (UINT i = 0; i < m_list.GetSelectedCount(); i++) {
		ndx = m_list.GetNextItem(ndx, LVNI_SELECTED);
		if (ndx == -1)
			break;
		DWORD data = m_list.GetItemData(ndx);
		CAlarmMachine* machine = reinterpret_cast<CAlarmMachine*>(data);
		if (machine && machine->execute_update_expire_time(dlg.m_dateTime)) {
			m_list.SetItemText(ndx, 2, dlg.m_dateTime.Format(L"%Y-%m-%d %H:%M:%S"));
		}
	}
}


BOOL CMachineExpireManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	DWORD dwStyle = m_list.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
	m_list.SetExtendedStyle(dwStyle);
	int i = -1;
	CString fm;
	fm.LoadStringW(IDS_STRING_SUBMACHINE);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 50, -1);
	fm.LoadStringW(IDS_STRING_ALIAS);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 200, -1);
	fm.LoadStringW(IDS_STRING_EXPIRE_TIME);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 125, -1);
	fm.LoadStringW(IDS_STRING_IF_EXPIRE);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 75, -1);
	fm.LoadStringW(IDS_STRING_CONTACT);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 75, -1);
	fm.LoadStringW(IDS_STRING_ADDRESS);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 225, -1);
	fm.LoadStringW(IDS_STRING_PHONE);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 150, -1);
	fm.LoadStringW(IDS_STRING_PHONE_BK);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 150, -1);

	//if (!m_machine)
	//	return FALSE;

	/*CZoneInfoList list;
	m_machine->GetAllZoneInfo(list);
	CZoneInfoListIter iter = list.begin();
	while (iter != list.end()) {
		CZoneInfo* zoneInfo = *iter++;
		CAlarmMachine* subMachine = zoneInfo->GetSubMachineInfo();
		if (subMachine) {
			InsertList(subMachine);
		}
	}*/

	std::list<core::CAlarmMachine*>::iterator iter = m_expiredMachineList.begin();
	while (iter != m_expiredMachineList.end()) {
		CAlarmMachine* machine = *iter++;
		InsertList(machine);
	}


	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}


void CMachineExpireManagerDlg::InsertList(const core::CAlarmMachine* machine)
{
	assert(machine);
	int nResult = -1;
	LV_ITEM lvitem = { 0 };
	CString tmp = _T("");

	lvitem.lParam = 0;
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = m_list.GetItemCount();
	lvitem.iSubItem = 0;

	// ndx
	if (machine->get_is_submachine()) {
		// submachine zone value
		tmp.Format(_T("%03d"), machine->get_submachine_zone());
	} else {
		// machine ademco id
		tmp.Format(_T("%04d"), machine->get_ademco_id());
	}
	lvitem.pszText = tmp.LockBuffer();
	nResult = m_list.InsertItem(&lvitem);
	tmp.UnlockBuffer();

	if (nResult != -1) {
		// alias
		lvitem.iItem = nResult;
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), machine->get_alias());
		lvitem.pszText = tmp.LockBuffer();
		m_list.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// expire time
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), machine->get_expire_time().Format(L"%Y-%m-%d %H:%M:%S"));
		lvitem.pszText = tmp.LockBuffer();
		m_list.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// if expire 
		CString syes, sno; syes.LoadStringW(IDS_STRING_YES); sno.LoadStringW(IDS_STRING_NO);
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), machine->get_left_service_time() <= 0 ? syes : sno);
		lvitem.pszText = tmp.LockBuffer();
		m_list.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// contact
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), machine->get_contact());
		lvitem.pszText = tmp.LockBuffer();
		m_list.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// address
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), machine->get_address());
		lvitem.pszText = tmp.LockBuffer();
		m_list.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// phone
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), machine->get_phone());
		lvitem.pszText = tmp.LockBuffer();
		m_list.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// phone_bk
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), machine->get_phone_bk());
		lvitem.pszText = tmp.LockBuffer();
		m_list.SetItem(&lvitem);
		tmp.UnlockBuffer();

		m_list.SetItemData(nResult, reinterpret_cast<DWORD_PTR>(machine));
	}
}


void CMachineExpireManagerDlg::OnBnClickedButtonAll()
{
	for (int i = 0; i < m_list.GetItemCount(); i++) {
		m_list.SetItemState(i, LVIS_FOCUSED | LVIS_SELECTED,
							LVIS_FOCUSED | LVIS_SELECTED);
	}
	m_list.SetFocus();
}


void CMachineExpireManagerDlg::OnBnClickedButtonAllNot() 
{
	for (int i = 0; i < m_list.GetItemCount(); i++) {
		m_list.SetItemState(i, 0, LVIS_FOCUSED | LVIS_SELECTED);
	}
	m_list.SetFocus();
}


void CMachineExpireManagerDlg::OnBnClickedButtonInvert()
{
	for (int i = 0; i < m_list.GetItemCount(); i++) {
		if (m_list.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED) {
			m_list.SetItemState(i, 0, LVIS_FOCUSED | LVIS_SELECTED);
		} else {
			m_list.SetItemState(i, LVIS_FOCUSED | LVIS_SELECTED,
								LVIS_FOCUSED | LVIS_SELECTED);
		}
	}
	m_list.SetFocus();
}


void CMachineExpireManagerDlg::OnNMCustomdrawList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
	*pResult = CDRF_DODEFAULT;

	if (CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage) {
		*pResult = CDRF_NOTIFYITEMDRAW;
	} else if (CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage) {
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
	} else if ((CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage) {
		COLORREF clrNewTextColor, clrNewBkColor;
		int nItem = static_cast<int>(pLVCD->nmcd.dwItemSpec);
		if (m_list.GetItemState(nItem, LVIS_SELECTED) == LVIS_SELECTED) {
			clrNewTextColor = RGB(255, 255, 255);        //Set the text to white
			clrNewBkColor = RGB(49, 106, 197);        //Set the background color to blue
		} else {
			clrNewTextColor = RGB(0, 0, 0);        //set the text black
			clrNewBkColor = RGB(255, 255, 255);    //leave the background color white
		}
		pLVCD->clrText = clrNewTextColor;
		pLVCD->clrTextBk = clrNewBkColor;
		*pResult = CDRF_DODEFAULT;
	}
}


void CMachineExpireManagerDlg::OnNMClickList1(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	//LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CString s; s.Format(L"%d", m_list.GetSelectedCount());
	m_staticSeldLineNum.SetWindowTextW(s);
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
	ofn.lpfnHook = NULL;

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
		q.LoadStringW(IDS_STRING_QUERY_REPLACE);
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
		e.LoadStringW(IDS_STRING_E_NO_EXECEL);
		MessageBox(e, L"", MB_ICONERROR);
		return FALSE;
	}

	// 创建进行存取的字符串
	sSql.Format(_T("DRIVER={%s};DSN='';FIRSTROWHASNAMES=1;READONLY=FALSE;CREATE_DB=\")%s\";DBQ=%s"),
				sDriver, excelPath, excelPath);

	// 创建数据库 (既Excel表格文件)
	if (!database.OpenEx(sSql, CDatabase::noOdbcDialog)) {
		CString e;
		e.LoadStringW(IDS_STRING_E_CREATE_EXCEL);
		MessageBox(e, L"", MB_ICONERROR);
		return FALSE;
	}

	CString sid, salias, sexpire_time, sif_expire, scontact, saddress, sphone, sphone_bk, syes, sno;
	salias.LoadStringW(IDS_STRING_ALIAS);
	sexpire_time.LoadStringW(IDS_STRING_EXPIRE_TIME);
	sif_expire.LoadStringW(IDS_STRING_IF_EXPIRE);
	scontact.LoadStringW(IDS_STRING_CONTACT);
	saddress.LoadStringW(IDS_STRING_ADDRESS);
	sphone.LoadStringW(IDS_STRING_PHONE);
	sphone_bk.LoadStringW(IDS_STRING_PHONE_BK);
	syes.LoadStringW(IDS_STRING_YES);
	sno.LoadStringW(IDS_STRING_NO);

	CString stable;
	stable.Format(L"EXPIRED_MACHINES(Id TEXT,%s TEXT,%s TEXT,%s TEXT,%s TEXT,%s TEXT,%s TEXT, %s TEXT)",
				  salias, sexpire_time, sif_expire, scontact, saddress, sphone, sphone_bk);
	sSql.Format(_T("CREATE TABLE %s"), stable.LockBuffer());
	stable.UnlockBuffer();
	database.ExecuteSQL(sSql);
	
	int nItem = -1;
	CString sinsert, svalues;
	sinsert.Format(L"INSERT INTO EXPIRED_MACHINES(Id,%s,%s,%s,%s,%s,%s,%s) ", 
				   salias, sexpire_time, sif_expire, scontact, saddress, sphone, sphone_bk);
	for (UINT i = 0; i < m_list.GetSelectedCount(); i++) {
		nItem = m_list.GetNextItem(nItem, LVNI_SELECTED);
		if (nItem == -1) break;
		DWORD data = m_list.GetItemData(nItem);
		core::CAlarmMachine* machine = reinterpret_cast<core::CAlarmMachine*>(data);
		if (machine) {
			svalues.Format(_T("VALUES('%d','%s','%s','%s','%s','%s','%s','%s')"), 
						machine->get_is_submachine() ? machine->get_submachine_zone() : machine->get_ademco_id(),
						machine->get_alias(), 
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
	fm.LoadString(IDS_STRING_FM_EXCEL_OK);
	warningStr.Format(fm, excelPath);
	if (IDYES == MessageBox(warningStr, L"", MB_YESNO | MB_ICONQUESTION)) {
		ShellExecute(NULL, _T("Open"), excelPath, NULL, NULL, SW_SHOW);
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
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos == NULL) {
		CLog::WriteLog(_T("No items were selected!\n"));
		CString e; e.LoadStringW(IDS_STRING_NO_SELD_CONTENT);
		MessageBox(e, L"", MB_ICONERROR);
		return;
	}

	CString path;
	if (!GetSaveAsFilePath(path))
		return;

	Export(path);
}


void CMachineExpireManagerDlg::OnBnClickedButtonPrintSel() 
{
	AUTO_LOG_FUNCTION;
}
