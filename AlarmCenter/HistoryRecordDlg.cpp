// HistoryRecordDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "HistoryRecordDlg.h"
#include "afxdialogex.h"
#include "HistoryRecord.h"
#include <odbcinst.h>
#include <afxdb.h>
#include <comdef.h>

using namespace core;
// CHistoryRecordDlg dialog

void __stdcall CHistoryRecordDlg::OnHistoryRecordCB(void* udata,
													const HistoryRecord* record)
{
	CHistoryRecordDlg* dlg = reinterpret_cast<CHistoryRecordDlg*>(udata); ASSERT(dlg);
	ASSERT(dlg->IsKindOf(RUNTIME_CLASS(CHistoryRecordDlg)));
	dlg->OnTraverseHistoryRecord(record);
}


CHistoryRecordDlg::CHistoryRecordDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CHistoryRecordDlg::IDD, pParent)
	, m_nPageCur(0)
	, m_nPageTotal(0)
	, m_nPerPage(30)
	, m_hIcon(NULL)
	, m_bDraging(FALSE)
	, m_dcList(NULL)
	, m_pDatabase(NULL)
{
	//{{AFX_DATA_INIT(CHistoryRecordDlg)
	//}}AFX_DATA_INIT
}

void CHistoryRecordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHistoryRecordDlg)
	DDX_Control(pDX, IDC_DATETIMEPICKER_BEG_DATE, m_begDate);
	DDX_Control(pDX, IDC_DATETIMEPICKER_BEG_TIME, m_begTime);
	DDX_Control(pDX, IDC_DATETIMEPICKER_END_DATE, m_endDate);
	DDX_Control(pDX, IDC_DATETIMEPICKER_END_TIME, m_endTime);
	DDX_Control(pDX, IDC_BUTTON_SEL_BY_DATE, m_btnSelByDate);
	DDX_Control(pDX, IDC_BUTTON_SEL_ALARM, m_btnSelAlarmByDate);
	DDX_Control(pDX, IDC_BUTTON_EXPORT, m_btnExport);
	DDX_Control(pDX, IDC_BUTTON_PAGE_PREV, m_btnPrev);
	DDX_Control(pDX, IDC_BUTTON_PAGE_NEXT, m_btnNext);
	DDX_Control(pDX, IDC_BUTTON_PAGE_LAST, m_btnLast);
	DDX_Control(pDX, IDC_BUTTON_PAGE_FIRST, m_btnFirst);
	DDX_Control(pDX, IDC_BUTTON_CLR_HISTORY, m_btnClr);
	DDX_Control(pDX, IDC_BUTTON_PRINT, m_btnPrint);
	DDX_Control(pDX, IDC_COMBO_PER_PAGE, m_cmbPerPage);
	DDX_Control(pDX, IDC_STATIC_PER_PAGE, m_staticPerPage);
	DDX_Control(pDX, IDC_PAGE, m_page);
	DDX_Control(pDX, IDC_LIST_RECORD, m_listCtrlRecord);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_BUTTON_SEL_BY_USER, m_btnSelByUser);
	DDX_Control(pDX, IDC_BUTTON_SEL_BY_MACHINE, m_btnSelByMachine);
	DDX_Control(pDX, IDC_BUTTON_EXPORT_SEL, m_btnExportSel);
	DDX_Control(pDX, IDC_BUTTON_SEL_ALL, m_btnSelAll);
	DDX_Control(pDX, IDC_BUTTON_SEL_INVERT, m_btnSelInvert);
	DDX_Control(pDX, IDC_BUTTON_SEL_NONE, m_btnSelNone);
}

BEGIN_MESSAGE_MAP(CHistoryRecordDlg, CDialogEx)
	//{{AFX_MSG_MAP(CHistoryRecordDlg)
	ON_WM_SHOWWINDOW()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_EXPORT, OnButtonExport)
	ON_CBN_SELCHANGE(IDC_COMBO_PER_PAGE, OnSelchangeComboPerpage)
	ON_COMMAND(IDC_BUTTON_PRINT, OnButtonPrint)
	ON_BN_CLICKED(IDC_BUTTON_SEL_BY_DATE, OnButtonSelByDate)
	ON_BN_CLICKED(IDC_BUTTON_SEL_ALARM, OnButtonSelAlarmByDate)
	ON_BN_CLICKED(IDC_BUTTON_PAGE_FIRST, OnButtonPageFirst)
	ON_BN_CLICKED(IDC_BUTTON_PAGE_PREV, OnButtonPagePrev)
	ON_BN_CLICKED(IDC_BUTTON_PAGE_NEXT, OnButtonPageNext)
	ON_BN_CLICKED(IDC_BUTTON_PAGE_LAST, OnButtonPageLast)
	ON_BN_CLICKED(IDC_BUTTON_CLR_HISTORY, OnButtonDeleteAllRecord)
	ON_WM_DESTROY()
	ON_NOTIFY(NM_CLICK, IDC_LIST_RECORD, OnClickListRecord)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHistoryRecordDlg message handlers

void CHistoryRecordDlg::OnOK()
{
	return;
}

void CHistoryRecordDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	if (bShow) {
		m_begDate.SetTime(&m_startTime);
		m_begTime.SetTime(&m_startTime);
		CTime now = CTime::GetCurrentTime();
		m_endDate.SetTime(&now);
		m_endTime.SetTime(&now);
		m_nPerPage = 30;
		for (int i = 0; i < m_cmbPerPage.GetCount(); i++) {
			if (m_nPerPage == (int)m_cmbPerPage.GetItemData(i)) {
				m_cmbPerPage.SetCurSel(i);
				break;
			}
		}

		CHistoryRecord *hr = CHistoryRecord::GetInstance();
		int total = hr->GetRecordCount();
		int pageTotal = total / m_nPerPage;
		if (total % m_nPerPage != 0)
			pageTotal++;
		if (m_nPageTotal != pageTotal) {
			m_nPageTotal = pageTotal;
			m_nPageCur = 1;
		}
		LoadRecordsBasedOnPage(m_nPageCur);
		/*
		CHistoryRecord *hr = CHistoryRecord::GetInstance();
		int total = hr->GetRecordCount();
		int pageTotal = total / MAX_MEMORY_RESIDENT_RECORD_NUMS;
		if(total % MAX_MEMORY_RESIDENT_RECORD_NUMS != 0)
		pageTotal++;
		if(m_nPageTotal != pageTotal)
		{
		m_nPageTotal = pageTotal;
		m_nPageCur = 1;
		}
		LoadRecordsBasedOnPage(m_nPageCur);
		*/
	}
}

BOOL CHistoryRecordDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_startTime = CTime::GetCurrentTime();
	m_currentTime = CTime::GetCurrentTime();
	CenterWindow();
	m_dcList = m_listCtrlRecord.GetDC();
	/*m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_HISTORY);
	if (m_hIcon) {
		SendMessage(WM_SETICON, ICON_SMALL, (LPARAM)m_hIcon);
	}*/


	//ShowWindow(SW_MAXIMIZE);
	RepositionItems();
	DWORD dwStyle = m_listCtrlRecord.GetExtendedStyle(); //获取当前扩展样式
	dwStyle |= LVS_EX_FULLROWSELECT; //选中某行使整行高亮（report风格时）
	dwStyle |= LVS_EX_GRIDLINES; //网格线（report风格时）
	m_listCtrlRecord.SetExtendedStyle(dwStyle); //设置扩展风格
	InitListCtrlHeader();

	int ndx = -1;
	ndx = m_cmbPerPage.AddString(_T("10"));
	m_cmbPerPage.SetItemData(ndx, 10);
	ndx = m_cmbPerPage.AddString(_T("20"));
	m_cmbPerPage.SetItemData(ndx, 20);
	ndx = m_cmbPerPage.AddString(_T("30"));
	m_cmbPerPage.SetItemData(ndx, 30);
	ndx = m_cmbPerPage.AddString(_T("40"));
	m_cmbPerPage.SetItemData(ndx, 40);
	ndx = m_cmbPerPage.AddString(_T("50"));
	m_cmbPerPage.SetItemData(ndx, 50);
	ndx = m_cmbPerPage.AddString(_T("100"));
	m_cmbPerPage.SetItemData(ndx, 100);
	ndx = m_cmbPerPage.AddString(_T("500"));
	m_cmbPerPage.SetItemData(ndx, 500);
	ndx = m_cmbPerPage.AddString(_T("1000"));
	m_cmbPerPage.SetItemData(ndx, 1000);
	for (int i = 0; i < m_cmbPerPage.GetCount(); i++) {
		if (m_nPerPage == (int)m_cmbPerPage.GetItemData(i)) {
			m_cmbPerPage.SetCurSel(i);
			break;
		}
	}

	CHistoryRecord *hr = CHistoryRecord::GetInstance();
	int total = hr->GetRecordCount();
	m_nPageTotal = total / m_nPerPage;
	if (total % m_nPerPage != 0)
		m_nPageTotal++;

	LoadRecordsBasedOnPage(1);

#ifndef _DEBUG
	::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
#endif
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

int CHistoryRecordDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	lpCreateStruct->x = 0;
	lpCreateStruct->y = 0;
	lpCreateStruct->cx = GetSystemMetrics(SM_CXSCREEN);
	lpCreateStruct->cy = GetSystemMetrics(SM_CYSCREEN);
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

void CHistoryRecordDlg::InitListCtrlHeader()
{
	int i = -1;
	CString fm;
	fm.LoadStringW(IDS_STRING_INDEX);
	m_listCtrlRecord.InsertColumn(++i, fm, LVCFMT_LEFT, 50, -1);
	fm.LoadStringW(IDS_STRING_TIME);
	m_listCtrlRecord.InsertColumn(++i, fm, LVCFMT_LEFT, 200, -1);
	fm.LoadStringW(IDS_STRING_HISTORY_RECORD);
	m_listCtrlRecord.InsertColumn(++i, fm, LVCFMT_LEFT, 1000, -1);
}

void CHistoryRecordDlg::InsertListContent(const core::HistoryRecord* record)
{
	int nResult = -1;
	LV_ITEM lvitem = { 0 };
	CString tmp = _T("");

	lvitem.lParam = record->id;
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = m_listCtrlRecord.GetItemCount();
	lvitem.iSubItem = 0;

	//序号
	tmp.Format(_T("%d"), record->id);
	lvitem.pszText = tmp.LockBuffer();
	nResult = m_listCtrlRecord.InsertItem(&lvitem);
	tmp.UnlockBuffer();

	if (nResult != -1) {
		//时间
		lvitem.iItem = nResult;
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), record->record_time);
		lvitem.pszText = tmp.LockBuffer();
		m_listCtrlRecord.SetItem(&lvitem);
		tmp.UnlockBuffer();

		//记录
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), record->record);
		lvitem.pszText = tmp.LockBuffer();
		m_listCtrlRecord.SetItem(&lvitem);
		tmp.UnlockBuffer();
	}
}

void CHistoryRecordDlg::OnButtonDeleteAllRecord()
{
	CHistoryRecord *hr = CHistoryRecord::GetInstance();
	if (hr->DeleteAllRecored()) {
		m_listCtrlRecord.DeleteAllItems();
		m_nPageCur = m_nPageTotal = 0;
		CString page = _T("");
		page.Format(_T("%d/%d"), m_nPageCur, m_nPageTotal);
		m_page.SetWindowText(page);
	}
}

void CHistoryRecordDlg::OnButtonPageFirst()
{
	if (m_nPageCur == 1)
		return;
	LoadRecordsBasedOnPage(1);
}

void CHistoryRecordDlg::OnButtonPageLast()
{
	if (m_nPageTotal == m_nPageCur)
		return;
	LoadRecordsBasedOnPage(m_nPageTotal);
}

void CHistoryRecordDlg::OnButtonPageNext()
{
	if (m_nPageTotal == m_nPageCur)
		return;
	LoadRecordsBasedOnPage(m_nPageCur + 1);
}

void CHistoryRecordDlg::OnButtonPagePrev()
{
	if (m_nPageCur == 1)
		return;
	LoadRecordsBasedOnPage(m_nPageCur - 1);
}

void CHistoryRecordDlg::OnUpdateButtonSeperator(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(FALSE);
}

void CHistoryRecordDlg::LoadRecordsBasedOnPage(const int nPage)
{
	m_listCtrlRecord.DeleteAllItems();
	
	CHistoryRecord *hr = CHistoryRecord::GetInstance();
	long baseID = hr->GetRecordMinimizeID();
	CRecordList list;
	hr->GetTopNumRecordsBasedOnID((m_nPageTotal - nPage)*m_nPerPage + baseID,
								  m_nPerPage, list);
	CRecordListReverseIter iter = list.rbegin();
	while (iter != list.rend()){
		HistoryRecord *record = *iter++;
		InsertListContent(record);
		SAFEDELETEP(record);
	}
	list.clear();

	m_nPageCur = nPage;
	CString page = _T("");
	page.Format(_T("%d/%d"), m_nPageCur, m_nPageTotal);
	m_page.SetWindowText(page);
}

void CHistoryRecordDlg::RepositionItems()
{
	do {
		CRect rc;
		GetClientRect(rc);
		CRect rcToolBar(rc);
		rcToolBar.bottom = rcToolBar.top + 50;
		//m_wndToolBar.GetWindowRect(rcToolBar);
		//rcToolBar.right = rcToolBar.left + 200;
		//ScreenToClient(rcToolBar);

		// 调整几个按钮的位置
		const int cBtnWidth = 50;
		const int cBtnHeight = 25;
		const int cBtnGaps = 10;
		CRect rcItem = rcToolBar;
		rcItem.top += 10;
		rcItem.bottom = rcItem.top + cBtnHeight;
		//rcItem.left += 50;

		// 页码
		if (m_page.m_hWnd == NULL)
			break;
		rcItem.right = rcItem.left + 50;
		m_page.MoveWindow(rcItem);

		// 首页
		if (m_btnFirst.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + cBtnGaps * 5;
		rcItem.right = rcItem.left + cBtnWidth;
		m_btnFirst.MoveWindow(rcItem);

		// 上页
		if (m_btnPrev.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + cBtnGaps;
		rcItem.right = rcItem.left + cBtnWidth;
		m_btnPrev.MoveWindow(rcItem);

		// 下页
		if (m_btnNext.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + cBtnGaps;
		rcItem.right = rcItem.left + cBtnWidth;
		m_btnNext.MoveWindow(rcItem);

		// 尾页
		if (m_btnLast.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + cBtnGaps;
		rcItem.right = rcItem.left + cBtnWidth;
		m_btnLast.MoveWindow(rcItem);

		// 行/页
		if (m_staticPerPage.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + cBtnGaps;
		rcItem.right = rcItem.left + int(cBtnWidth * 1.5);
		m_staticPerPage.MoveWindow(rcItem);

		// 行/页
		if (m_cmbPerPage.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + 5;
		rcItem.right = rcItem.left + int(cBtnWidth * 1);
		m_cmbPerPage.MoveWindow(rcItem);

		// 导出全部到Excel
		if (m_btnExport.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + cBtnGaps * 2;
		rcItem.right = rcItem.left + int(cBtnWidth * 3);
		m_btnExport.MoveWindow(rcItem);

		// 导出选中到Excel
		if (m_btnExportSel.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + cBtnGaps;
		rcItem.right = rcItem.left + int(cBtnWidth * 3);
		m_btnExportSel.MoveWindow(rcItem);

		// 打印选中行
		if (m_btnPrint.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + cBtnGaps;
		rcItem.right = rcItem.left + int(cBtnWidth * 2);
		m_btnPrint.MoveWindow(rcItem);

		// 换行
		rcItem.left = rcToolBar.left + cBtnGaps;
		rcItem.top = rcItem.bottom + cBtnGaps;
		rcItem.bottom = rcItem.top + cBtnHeight;
		
		// 起始日期
		if (m_begDate.m_hWnd == NULL)
			break;
		CRect rcDateTime;
		m_begDate.GetWindowRect(rcDateTime);
		//rcItem.left = rcItem.right + 15;
		rcItem.right = rcItem.left + rcDateTime.Width();
		m_begDate.MoveWindow(rcItem);

		// 起始时间
		if (m_begTime.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + 2;
		rcItem.right = rcItem.left + rcDateTime.Width();
		m_begTime.MoveWindow(rcItem);

		// 结束日期
		if (m_endDate.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + 5;
		rcItem.right = rcItem.left + rcDateTime.Width();
		m_endDate.MoveWindow(rcItem);

		// 结束时间
		if (m_endTime.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + 2;
		rcItem.right = rcItem.left + rcDateTime.Width();
		m_endTime.MoveWindow(rcItem);

		// 按日期查询
		if (m_btnSelByDate.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + 5;
		rcItem.right = rcItem.left + int(cBtnWidth * 2);
		m_btnSelByDate.MoveWindow(rcItem);

		// 查询报警信息
		if (m_btnSelAlarmByDate.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + 15;
		rcItem.right = rcItem.left + int(cBtnWidth * 2);
		m_btnSelAlarmByDate.MoveWindow(rcItem);

		// 按用户查询
		if (m_btnSelByUser.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + 5;
		rcItem.right = rcItem.left + int(cBtnWidth * 2);
		m_btnSelByUser.MoveWindow(rcItem);

		// 按主机查询
		if (m_btnSelByMachine.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + 5;
		rcItem.right = rcItem.left + int(cBtnWidth * 2);
		m_btnSelByMachine.MoveWindow(rcItem);

		// 换行
		rcItem.left = rcToolBar.left + cBtnGaps;
		rcItem.top = rcItem.bottom + cBtnGaps;
		rcItem.bottom = rcItem.top + cBtnHeight;

		// 全选
		if (m_btnSelAll.m_hWnd == NULL)
			break;
		//rcItem.left = rcItem.right + 5;
		rcItem.right = rcItem.left + int(cBtnWidth);
		m_btnSelAll.MoveWindow(rcItem);

		// 反选
		if (m_btnSelInvert.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + 5;
		rcItem.right = rcItem.left + int(cBtnWidth);
		m_btnSelInvert.MoveWindow(rcItem);

		// 全不选
		if (m_btnSelNone.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + 5;
		rcItem.right = rcItem.left + int(cBtnWidth);
		m_btnSelNone.MoveWindow(rcItem);

		// 列表
		if (m_listCtrlRecord.m_hWnd == NULL)
			break;
		rc.DeflateRect(15, 15, 15, 15);
		//rc.top += rcToolBar.Height() + rcItem.Height() + rcItem.Height() + cBtnGaps;
		rc.top = rcItem.bottom + cBtnGaps;
		m_listCtrlRecord.MoveWindow(rc);
		//m_listCtrlRecord.ShowWindow(SW_HIDE);
	} while (0);
}

void CHistoryRecordDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	RepositionItems();
}

CString CHistoryRecordDlg::GetExcelDriver()
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


void CHistoryRecordDlg::OnTraverseHistoryRecord(const HistoryRecord* record)
{
	static CString sSql;
	sSql.Format(_T("INSERT INTO HISTORY_RECORD (Id,RecordTime,Record) VALUES('%d','%s','%s')"),
				record->id, record->record_time, record->record);
	m_pDatabase->ExecuteSQL(sSql);
}

void __stdcall CHistoryRecordDlg::TraverseHistoryRecord(void* udata)
{
	CHistoryRecord* hr = CHistoryRecord::GetInstance();
	hr->TraverseHistoryRecord(udata, OnHistoryRecordCB);
}


BOOL CHistoryRecordDlg::Export(CString excelPath, TraverseHistoryRecordCB cb)
{
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

	sSql.Format(_T("CREATE TABLE HISTORY_RECORD(Id TEXT,RecordTime TEXT,Record TEXT)"));
	database.ExecuteSQL(sSql);
	m_pDatabase = &database;
	if (cb) { cb(this); }
	m_pDatabase = NULL;
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


BOOL CHistoryRecordDlg::GetSaveAsFilePath(CString& path)
{
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

void CHistoryRecordDlg::OnButtonExport()
{
	CString path;
	if (!GetSaveAsFilePath(path))
		return;

	Export(path, TraverseHistoryRecord);
}

void CHistoryRecordDlg::OnSelchangeComboPerpage()
{
	int ndx = m_cmbPerPage.GetCurSel();
	if (ndx < 0)
		return;
	int nPerPage = m_cmbPerPage.GetItemData(ndx);
	if (nPerPage == m_nPerPage)
		return;
	m_nPerPage = nPerPage;
	CHistoryRecord *hr = CHistoryRecord::GetInstance();
	int total = hr->GetRecordCount();
	int pageTotal = total / nPerPage;
	if (total % nPerPage != 0)
		pageTotal++;
	if (m_nPageTotal != pageTotal) {
		m_nPageTotal = pageTotal;
		m_nPageCur = 1;
	}
	LoadRecordsBasedOnPage(m_nPageCur);
}

BOOL CHistoryRecordDlg::PrintRecord(CListCtrl &list)
{
	POSITION pos = list.GetFirstSelectedItemPosition();
	if (pos == NULL) {
		CLog::WriteLog(_T("No items were selected!\n"));
		CString e; e.LoadStringW(IDS_STRING_NO_SELD_CONTENT);
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
	pd.hDC = NULL;
	pd.hwndOwner = NULL;
	pd.hInstance = NULL;
	pd.nMaxPage = 2;
	pd.nMinPage = 1;
	pd.nFromPage = 1;
	pd.nToPage = 1;
	pd.nCopies = 1;
	pd.hDevMode = NULL;
	pd.hDevNames = NULL;

	/////////////////////////////////////////////////////////
	//显示打印对话框，由用户来设定纸张大小等.
	if (!PrintDlg(&pd))   return   FALSE;
	ASSERT(pd.hDC != NULL);/*断言获取的句柄不为空.*/
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
			AfxMessageBox(_T("字段太多，无法在一行内打印，请试用较大的纸，或横向打印。"));
			return  FALSE;
		}
	}

	//设置打印文件的保存对话框 
	DOCINFO   di;
	di.cbSize = sizeof(DOCINFO);
	di.lpszDocName = _T("报警主机历史记录");
	di.lpszOutput = (LPTSTR)NULL;
	di.lpszDatatype = (LPTSTR)NULL;
	di.fwType = 0;
	StartDoc(pd.hDC, &di);
	StartPage(pd.hDC);
	SelectObject(pd.hDC, hTopicFont);
	TextOut(pd.hDC, nHorRes / 3, nYMargin, _T("报警主机历史记录"),
			_tcslen(_T("报警主机历史记录")));

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
			CLog::WriteLog(_T("%s\n"), subitem);
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

void CHistoryRecordDlg::OnButtonPrint()
{
	PrintRecord(m_listCtrlRecord);
}

void CHistoryRecordDlg::OnButtonSelByDate()
{
	UpdateData();
	CTime begDate, begTime, endDate, endTime;
	if (!GetDateTimeValue(m_begDate, begDate)
		|| !GetDateTimeValue(m_begTime, begTime)
		|| !GetDateTimeValue(m_endDate, endDate)
		|| !GetDateTimeValue(m_endTime, endTime)) {
		//MyErrorMsgBox(m_hWnd, IDS_STRING_TIME_NOT_SET);
		return;
	}

	CTime beg(begDate.GetYear(), begDate.GetMonth(), begDate.GetDay(),
			  begTime.GetHour(), begTime.GetMinute(), begTime.GetSecond());
	CTime end(endDate.GetYear(), endDate.GetMonth(), endDate.GetDay(),
			  endTime.GetHour(), endTime.GetMinute(), endTime.GetSecond());

	//CString strBeg = beg.Format(GetStringTable(IDS_STRING_TIME_FORMAT));
	//CString strEnd = end.Format(GetStringTable(IDS_STRING_TIME_FORMAT));
	//CLog::WriteLog(_T("strBeg:%s strEnd:%s\n"), strBeg, strEnd);

	//CTimeSpan span = end - beg;
	//if (span.GetTotalMinutes() <= 0) {
	//	MyErrorMsgBox(m_hWnd, IDS_STRING_TIME_ERROR);
	//	return;
	//}

	////CString strBeg = beg.Format(GetStringTable(IDS_STRING_TIME_FORMAT));
	////CString strEnd = end.Format(GetStringTable(IDS_STRING_TIME_FORMAT));
	//CRecordList list;
	//if (CDBOper::GetInstance()->GetHistoryRecordListByDate(list, strBeg, strEnd)) {
	//	m_listCtrlRecord.DeleteAllItems();
	//	POSITION pos = list.GetTailPosition();
	//	while (pos) {
	//		CRecord *pRecord = list.GetPrev(pos);
	//		InsertListContent(pRecord->id, pRecord->record_time, pRecord->record);
	//		SAFEDELETEP(pRecord);
	//	}
	//	m_nPageCur = m_nPageTotal = 1;
	//	CString page = _T("");
	//	page.Format(_T("%d/%d"), m_nPageCur, m_nPageTotal);
	//	m_page.SetWindowText(page);
	//	m_cmbPerPage.SetCurSel(-1);
	//} else {
	//	MyMsgBox(m_hWnd, IDS_STRING_NO_DATA);
	//}
}

BOOL CHistoryRecordDlg::GetDateTimeValue(CDateTimeCtrl &ctrl, CTime &value)
{
	//DWORD dwResult = ctrl.GetTime(value);
	//if (dwResult == GDT_VALID) {
	//	// the user checked the box and specified data
	//	CString str;

	//	// is it a time-only control, or a date-only control?
	//	if ((ctrl.GetStyle() & DTS_TIMEFORMAT) == DTS_TIMEFORMAT)
	//		str = value.Format(GetStringTable(IDS_STRING_TIME_FORMAT));
	//	else
	//		str = value.Format(GetStringTable(IDS_STRING_TIME_FORMAT));
	//	//AfxMessageBox(str);
	//	return TRUE;
	//} else {
	//	// the user unmarked the "none" box
	//	//AfxMessageBox(_T("Time not set!"));
	//	return FALSE;
	//}
	return FALSE;
}

void CHistoryRecordDlg::OnButtonSelAlarmByDate()
{
	/*UpdateData();
	CTime begDate, begTime, endDate, endTime;
	if (!GetDateTimeValue(m_begDate, begDate)
		|| !GetDateTimeValue(m_begTime, begTime)
		|| !GetDateTimeValue(m_endDate, endDate)
		|| !GetDateTimeValue(m_endTime, endTime)) {
		MyErrorMsgBox(m_hWnd, IDS_STRING_TIME_NOT_SET);
		return;
	}

	CTime beg(begDate.GetYear(), begDate.GetMonth(), begDate.GetDay(),
			  begTime.GetHour(), begTime.GetMinute(), begTime.GetSecond());
	CTime end(endDate.GetYear(), endDate.GetMonth(), endDate.GetDay(),
			  endTime.GetHour(), endTime.GetMinute(), endTime.GetSecond());

	CTimeSpan span = end - beg;
	if (span.GetTotalMinutes() <= 0) {
		MyErrorMsgBox(m_hWnd, IDS_STRING_TIME_ERROR);
		return;
	}

	CString strBeg = beg.Format(GetStringTable(IDS_STRING_TIME_FORMAT));
	CString strEnd = end.Format(GetStringTable(IDS_STRING_TIME_FORMAT));
	CRecordList list;
	if (CDBOper::GetInstance()->GetHistoryRecordAlarmByDate(list, strBeg, strEnd)) {
		m_listCtrlRecord.DeleteAllItems();
		POSITION pos = list.GetTailPosition();
		while (pos) {
			CRecord *pRecord = list.GetPrev(pos);
			InsertListContent(pRecord->id, pRecord->record_time, pRecord->record);
			SAFEDELETEP(pRecord);
		}
		m_nPageCur = m_nPageTotal = 1;
		CString page = _T("");
		page.Format(_T("%d/%d"), m_nPageCur, m_nPageTotal);
		m_page.SetWindowText(page);
		m_cmbPerPage.SetCurSel(-1);
	} else {
		MyMsgBox(m_hWnd, IDS_STRING_NO_DATA);
	}*/
}

BOOL CHistoryRecordDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->hwnd == m_listCtrlRecord) {
		do {
			if (pMsg->message == WM_LBUTTONDOWN) {
				m_listCtrlRecord.SetFocus();
				m_bDraging = TRUE;
				GetCursorPos(&m_ptBeg);
				m_listCtrlRecord.ScreenToClient(&m_ptBeg);
				int nItem = m_listCtrlRecord.HitTest(m_ptBeg);
				if (nItem == -1) {
					for (int i = 0; i < m_listCtrlRecord.GetItemCount(); i++) {
						m_listCtrlRecord.SetItemState(i, 0, LVIS_SELECTED | LVIS_FOCUSED);
					}
				} else {
					if (m_listCtrlRecord.GetItemState(nItem, LVIS_SELECTED) == LVIS_SELECTED)
						m_listCtrlRecord.SetItemState(nItem,
						0, LVIS_SELECTED | LVIS_FOCUSED);
					else
						m_listCtrlRecord.SetItemState(nItem,
						LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				}
			} else if (pMsg->message == WM_MOUSEMOVE) {
				if (m_bDraging) {
					CPoint pt;
					GetCursorPos(&pt);
					m_listCtrlRecord.ScreenToClient(&pt);
					int nItem = m_listCtrlRecord.HitTest(pt);
					m_listCtrlRecord.SetItemState(nItem, LVIS_SELECTED | LVIS_FOCUSED,
												  LVIS_SELECTED | LVIS_FOCUSED);
					m_ptEnd = pt;
				}
			} else if (pMsg->message == WM_LBUTTONUP) {
				/*if (m_bDraging) {
				GetCursorPos(&m_ptEnd);
				m_listCtrlRecord.ScreenToClient(&m_ptEnd);
				int nItem = m_listCtrlRecord.HitTest(m_ptEnd);
				m_listCtrlRecord.SetItemState(nItem, LVIS_SELECTED | LVIS_FOCUSED,
				LVIS_SELECTED | LVIS_FOCUSED);
				m_bDraging = FALSE;
				}*/
				m_bDraging = FALSE;
				//NMHDR *nm = new NMHDR;
				//	nm->hwndFrom = m_listCtrlRecord.m_hWnd;
				//	nm->idFrom = IDC_LIST_RECORD;
				//	nm->code = NM_CLICK;
				//	PostMessage(WM_NOTIFY, IDC_LIST_RECORD, (DWORD)nm);
			} else if (pMsg->message == WM_KILLFOCUS) {
				//CLog::WriteLog(_T("OnKillFocus...........................\n"));
				//m_bDraging = FALSE;
			} else {
				break;
			}
			return TRUE;
		} while (0);
		return FALSE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

void CHistoryRecordDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	m_listCtrlRecord.ReleaseDC(m_dcList);
}

void CHistoryRecordDlg::OnClickListRecord(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	/*CLog::WriteLog(_T("CHistoryRecordDlg::OnClickListRecord\n"));
	delete pNMHDR;
	CPoint pt;
	GetCursorPos(&pt);
	m_listCtrlRecord.ScreenToClient(&pt);
	int nItem = m_listCtrlRecord.HitTest(pt);
	if (m_listCtrlRecord.GetItemState(nItem, LVIS_SELECTED) == LVIS_SELECTED) {
	int mask = ~(LVIS_SELECTED | LVIS_FOCUSED);
	m_listCtrlRecord.SetItemState(nItem, 0, mask);
	} else {
	m_listCtrlRecord.SetItemState(nItem, LVIS_SELECTED | LVIS_FOCUSED,
	LVIS_SELECTED | LVIS_FOCUSED);
	}
	*pResult = 0;*/
}




