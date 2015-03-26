// HistoryRecordDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include <odbcinst.h>
#include <afxdb.h>
#include <comdef.h>
#include "HistoryRecordDlg.h"
#include "afxdialogex.h"
#include "HistoryRecord.h"
#include "ChooseMachineDlg.h"

using namespace core;
// CHistoryRecordDlg dialog



void __stdcall CHistoryRecordDlg::OnExportHistoryRecordCB(void* udata,
													const HistoryRecord* record)
{
	CHistoryRecordDlg* dlg = reinterpret_cast<CHistoryRecordDlg*>(udata); ASSERT(dlg);
	ASSERT(dlg->IsKindOf(RUNTIME_CLASS(CHistoryRecordDlg)));
	dlg->OnExportTraverseHistoryRecord(record);
}


void __stdcall CHistoryRecordDlg::OnShowHistoryRecordCB(void* udata,
														const core::HistoryRecord* record)
{
	CHistoryRecordDlg* dlg = reinterpret_cast<CHistoryRecordDlg*>(udata); ASSERT(dlg);
	ASSERT(dlg->IsKindOf(RUNTIME_CLASS(CHistoryRecordDlg)));
	dlg->InsertListContent(record);
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
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_EXPORT_SEL, &CHistoryRecordDlg::OnBnClickedButtonExportSel)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_RECORD, &CHistoryRecordDlg::OnNMCustomdrawListRecord)
	ON_BN_CLICKED(IDC_BUTTON_SEL_BY_USER, &CHistoryRecordDlg::OnBnClickedButtonSelByUser)
	ON_BN_CLICKED(IDC_BUTTON_SEL_BY_MACHINE, &CHistoryRecordDlg::OnBnClickedButtonSelByMachine)
	ON_BN_CLICKED(IDC_BUTTON_SEL_ALL, &CHistoryRecordDlg::OnBnClickedButtonSelAll)
	ON_BN_CLICKED(IDC_BUTTON_SEL_INVERT, &CHistoryRecordDlg::OnBnClickedButtonSelInvert)
	ON_BN_CLICKED(IDC_BUTTON_SEL_NONE, &CHistoryRecordDlg::OnBnClickedButtonSelNone)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHistoryRecordDlg message handlers

void CHistoryRecordDlg::OnOK()
{
	return;
}

void CHistoryRecordDlg::ClearListCtrlAndFreeData()
{
	for (int i = 0; i < m_listCtrlRecord.GetItemCount(); i++) {
		DWORD data = m_listCtrlRecord.GetItemData(i);
		const HistoryRecord* record = reinterpret_cast<const HistoryRecord*>(data);
		SAFEDELETEP(record);
	}
	m_listCtrlRecord.DeleteAllItems();
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
	}
}

BOOL CHistoryRecordDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_startTime = CTime::GetCurrentTime();
	m_currentTime = CTime::GetCurrentTime();
	CenterWindow();
	m_dcList = m_listCtrlRecord.GetDC();
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_HISTORY);
	if (m_hIcon) {
		SendMessage(WM_SETICON, ICON_SMALL, (LPARAM)m_hIcon);
	}


	//ShowWindow(SW_MAXIMIZE);
	RepositionItems();
	DWORD dwStyle = m_listCtrlRecord.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
	m_listCtrlRecord.SetExtendedStyle(dwStyle);
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

	lvitem.lParam = 0;
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = m_listCtrlRecord.GetItemCount();
	lvitem.iSubItem = 0;

	//���
	tmp.Format(_T("%d"), record->id);
	lvitem.pszText = tmp.LockBuffer();
	nResult = m_listCtrlRecord.InsertItem(&lvitem);
	tmp.UnlockBuffer();

	if (nResult != -1) {
		//ʱ��
		lvitem.iItem = nResult;
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), record->record_time);
		lvitem.pszText = tmp.LockBuffer();
		m_listCtrlRecord.SetItem(&lvitem);
		tmp.UnlockBuffer();

		//��¼
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), record->record);
		lvitem.pszText = tmp.LockBuffer();
		m_listCtrlRecord.SetItem(&lvitem);
		tmp.UnlockBuffer();

		m_listCtrlRecord.SetItemData(nResult, 
									 reinterpret_cast<DWORD_PTR>(new HistoryRecord(*record)));
	}
}

void CHistoryRecordDlg::OnButtonDeleteAllRecord()
{
	CHistoryRecord *hr = CHistoryRecord::GetInstance();
	if (hr->DeleteAllRecored()) {
		ClearListCtrlAndFreeData();
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
	ClearListCtrlAndFreeData();
	
	CHistoryRecord *hr = CHistoryRecord::GetInstance();
	long baseID = hr->GetRecordMinimizeID();
	CAutoRedrawListCtrl noname(m_listCtrlRecord);
	hr->GetTopNumRecordsBasedOnID((m_nPageTotal - nPage)*m_nPerPage + baseID,
								  m_nPerPage, this, OnShowHistoryRecordCB);
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

		// ����������ť��λ��
		const int cBtnWidth = 50;
		const int cBtnHeight = 25;
		const int cBtnGaps = 10;
		CRect rcItem = rcToolBar;
		rcItem.top += 10;
		rcItem.bottom = rcItem.top + cBtnHeight;
		//rcItem.left += 50;

		// ҳ��
		if (m_page.m_hWnd == NULL)
			break;
		rcItem.right = rcItem.left + 50;
		m_page.MoveWindow(rcItem);

		// ��ҳ
		if (m_btnFirst.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + cBtnGaps * 5;
		rcItem.right = rcItem.left + cBtnWidth;
		m_btnFirst.MoveWindow(rcItem);

		// ��ҳ
		if (m_btnPrev.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + cBtnGaps;
		rcItem.right = rcItem.left + cBtnWidth;
		m_btnPrev.MoveWindow(rcItem);

		// ��ҳ
		if (m_btnNext.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + cBtnGaps;
		rcItem.right = rcItem.left + cBtnWidth;
		m_btnNext.MoveWindow(rcItem);

		// βҳ
		if (m_btnLast.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + cBtnGaps;
		rcItem.right = rcItem.left + cBtnWidth;
		m_btnLast.MoveWindow(rcItem);

		// ��/ҳ
		if (m_staticPerPage.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + cBtnGaps;
		rcItem.right = rcItem.left + int(cBtnWidth * 1.5);
		m_staticPerPage.MoveWindow(rcItem);

		// ��/ҳ
		if (m_cmbPerPage.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + 5;
		rcItem.right = rcItem.left + int(cBtnWidth * 1);
		m_cmbPerPage.MoveWindow(rcItem);

		// ����ȫ����Excel
		if (m_btnExport.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + cBtnGaps * 2;
		rcItem.right = rcItem.left + int(cBtnWidth * 3);
		m_btnExport.MoveWindow(rcItem);

		// ����ѡ�е�Excel
		if (m_btnExportSel.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + cBtnGaps;
		rcItem.right = rcItem.left + int(cBtnWidth * 3);
		m_btnExportSel.MoveWindow(rcItem);

		// ��ӡѡ����
		if (m_btnPrint.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + cBtnGaps;
		rcItem.right = rcItem.left + int(cBtnWidth * 2);
		m_btnPrint.MoveWindow(rcItem);

		// ����
		rcItem.left = rcToolBar.left + cBtnGaps;
		rcItem.top = rcItem.bottom + cBtnGaps;
		rcItem.bottom = rcItem.top + cBtnHeight;
		
		// ��ʼ����
		if (m_begDate.m_hWnd == NULL)
			break;
		CRect rcDateTime;
		m_begDate.GetWindowRect(rcDateTime);
		//rcItem.left = rcItem.right + 15;
		rcItem.right = rcItem.left + rcDateTime.Width();
		m_begDate.MoveWindow(rcItem);

		// ��ʼʱ��
		if (m_begTime.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + 2;
		rcItem.right = rcItem.left + rcDateTime.Width();
		m_begTime.MoveWindow(rcItem);

		// ��������
		if (m_endDate.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + 5;
		rcItem.right = rcItem.left + rcDateTime.Width();
		m_endDate.MoveWindow(rcItem);

		// ����ʱ��
		if (m_endTime.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + 2;
		rcItem.right = rcItem.left + rcDateTime.Width();
		m_endTime.MoveWindow(rcItem);

		// �����ڲ�ѯ
		if (m_btnSelByDate.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + 5;
		rcItem.right = rcItem.left + int(cBtnWidth * 2);
		m_btnSelByDate.MoveWindow(rcItem);

		// ��ѯ������Ϣ
		if (m_btnSelAlarmByDate.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + 15;
		rcItem.right = rcItem.left + int(cBtnWidth * 2);
		m_btnSelAlarmByDate.MoveWindow(rcItem);

		// ���û���ѯ
		if (m_btnSelByUser.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + 5;
		rcItem.right = rcItem.left + int(cBtnWidth * 2);
		m_btnSelByUser.MoveWindow(rcItem);

		// ��������ѯ
		if (m_btnSelByMachine.m_hWnd == NULL)
			break;
		rcItem.left = rcItem.right + 5;
		rcItem.right = rcItem.left + int(cBtnWidth * 2);
		m_btnSelByMachine.MoveWindow(rcItem);

		//// ����
		//rcItem.left = rcToolBar.left + cBtnGaps;
		//rcItem.top = rcItem.bottom + cBtnGaps;
		//rcItem.bottom = rcItem.top + cBtnHeight;

		//// ȫѡ
		//if (m_btnSelAll.m_hWnd == NULL)
		//	break;
		////rcItem.left = rcItem.right + 5;
		//rcItem.right = rcItem.left + int(cBtnWidth);
		//m_btnSelAll.MoveWindow(rcItem);

		//// ��ѡ
		//if (m_btnSelInvert.m_hWnd == NULL)
		//	break;
		//rcItem.left = rcItem.right + 5;
		//rcItem.right = rcItem.left + int(cBtnWidth);
		//m_btnSelInvert.MoveWindow(rcItem);

		//// ȫ��ѡ
		//if (m_btnSelNone.m_hWnd == NULL)
		//	break;
		//rcItem.left = rcItem.right + 5;
		//rcItem.right = rcItem.left + int(cBtnWidth);
		//m_btnSelNone.MoveWindow(rcItem);

		// �б�
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

	// ��ȡ�Ѱ�װ����������(������odbcinst.h��)
	if (!SQLGetInstalledDrivers(szBuf, cbBufMax, &cbBufOut))
		return _T("");

	// �����Ѱ�װ�������Ƿ���Excel...
	do {
		if (_tcsstr(pszBuf, _T("Excel")) != 0) {
			//���� !
			sDriver = CString(pszBuf);
			break;
		}
		pszBuf = _tcschr(pszBuf, _T('\0')) + 1;
	} while (pszBuf[1] != _T('\0'));

	return sDriver;
}


void CHistoryRecordDlg::OnExportTraverseHistoryRecord(const HistoryRecord* record)
{
	static CString sSql;
	sSql.Format(_T("INSERT INTO HISTORY_RECORD (Id,RecordTime,Record) VALUES('%d','%s','%s')"),
				record->id, record->record_time, record->record);
	m_pDatabase->ExecuteSQL(sSql);
}

void __stdcall CHistoryRecordDlg::ExportTraverseHistoryRecord(void* udata)
{
	CHistoryRecord* hr = CHistoryRecord::GetInstance();
	hr->TraverseHistoryRecord(udata, OnExportHistoryRecordCB);
}

void __stdcall CHistoryRecordDlg::ExportTraverseSeledHistoryRecord(void* udata)
{
	CHistoryRecordDlg* dlg = reinterpret_cast<CHistoryRecordDlg*>(udata); ASSERT(dlg);
	ASSERT(dlg->IsKindOf(RUNTIME_CLASS(CHistoryRecordDlg)));
	for (int i = 0; i < dlg->m_listCtrlRecord.GetItemCount(); i++) {
		DWORD data = dlg->m_listCtrlRecord.GetItemData(i);
		const HistoryRecord* record = reinterpret_cast<const HistoryRecord*>(data);
		dlg->OnExportHistoryRecordCB(dlg, record);
	}
}


BOOL CHistoryRecordDlg::Export(const CString& excelPath, TraverseHistoryRecordCB cb)
{
	CString warningStr = _T("");
	CDatabase database;
	CString sDriver = _T("");
	CString sSql = _T("");

	// �����Ƿ�װ��Excel���� "Microsoft Excel Driver (*.xls)" 
	sDriver = GetExcelDriver();
	if (sDriver.IsEmpty()) {
		// û�з���Excel����
		CString e;
		e.LoadStringW(IDS_STRING_E_NO_EXECEL);
		MessageBox(e, L"", MB_ICONERROR);
		return FALSE;
	}

	// �������д�ȡ���ַ���
	sSql.Format(_T("DRIVER={%s};DSN='';FIRSTROWHASNAMES=1;READONLY=FALSE;CREATE_DB=\")%s\";DBQ=%s"),
				sDriver, excelPath, excelPath);

	// �������ݿ� (��Excel�����ļ�)
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
	// �ر����ݿ�
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

	Export(path, ExportTraverseHistoryRecord);
}

void CHistoryRecordDlg::OnBnClickedButtonExportSel()
{
	POSITION pos = m_listCtrlRecord.GetFirstSelectedItemPosition();
	if (pos == NULL) {
		CLog::WriteLog(_T("No items were selected!\n"));
		CString e; e.LoadStringW(IDS_STRING_NO_SELD_CONTENT);
		MessageBox(e, L"", MB_ICONERROR);
		return;
	}

	CString path;
	if (!GetSaveAsFilePath(path))
		return;

	Export(path, ExportTraverseSeledHistoryRecord);
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

	HFONT hRecordFont;//��¼������
	HFONT hTopicFont;//���������
	HFONT hCodeFont;//�ֶε�����

	//����(������ݵ�)����
	hRecordFont = CreateFont(93, 29, 1, 0, FW_EXTRALIGHT, 0, 0, 0, ANSI_CHARSET,
							 OUT_DEFAULT_PRECIS,
							 CLIP_DEFAULT_PRECIS,
							 DEFAULT_QUALITY,
							 DEFAULT_PITCH,
							 _T("Arial"));

	//�������������
	hTopicFont = CreateFont(260, 47, 10, 0, FW_BOLD, 0, 0, 0, ANSI_CHARSET,
							OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,
							DEFAULT_QUALITY,
							DEFAULT_PITCH,
							_T("Arial"));

	//�����ֶε�����
	hCodeFont = CreateFont(150, 50, 1, 0, FW_BOLD, 0, 0, 0, ANSI_CHARSET,
						   OUT_DEFAULT_PRECIS,
						   CLIP_DEFAULT_PRECIS,
						   DEFAULT_QUALITY,
						   DEFAULT_PITCH,
						   _T("Arial"));

	//���ô�ӡ�Ի���
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
	//��ʾ��ӡ�Ի������û����趨ֽ�Ŵ�С��.
	if (!PrintDlg(&pd))   return   FALSE;
	ASSERT(pd.hDC != NULL);/*���Ի�ȡ�ľ����Ϊ��.*/
	int   nHorRes = GetDeviceCaps(pd.hDC, HORZRES);
	int   nVerRes = GetDeviceCaps(pd.hDC, VERTRES);
	int   nXMargin = 20;//ҳ�ߵĿհ�   
	int   nYMargin = 6;

	///////////////////////////////////////////////////////////
	TEXTMETRIC  tm;/*ӳ��ṹ��*/
	GetTextMetrics(pd.hDC, &tm);
	int   nCharWidth = tm.tmAveCharWidth;
	int   ncaps = (tm.tmPitchAndFamily & 1 ? 3 : 2)*nCharWidth / 2;
	int   nCharHeight = tm.tmExternalLeading + tm.tmHeight + ncaps;//

	///////////////////////////////////////////////////////////
	CHeaderCtrl*   pHeader = list.GetHeaderCtrl();
	//����У��еĸ���
	int   nColCount = pHeader->GetItemCount();//��ȡ��ͷ�ĸ��� 
	int   nLineCount = list.GetSelectedCount(); //��ȡListCtrl�ļ�¼����
	int   ColOrderArray[100];
	COLATT   ca[100];
	list.GetColumnOrderArray(ColOrderArray, nColCount); //�洢��ͷ������ֵ
	int   nColX = nXMargin*nCharWidth;

	////////////////////////////////////////////////////////////
	//�������е���Ϣ��ȷ���б�������ݳ��ȡ�
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
		if (nColX > nHorRes) {  //��ʾ�������ͷ����λ���Ѿ�������  
			DeleteDC(pd.hDC);
			CString e; e.LoadStringW(IDS_STRING_E_TOOLMANY_FIELD);
			MessageBox(e, L"", MB_ICONERROR);
			return  FALSE;
		}
	}

	//���ô�ӡ�ļ��ı���Ի��� 
	CString fm;
	DOCINFO   di;
	di.cbSize = sizeof(DOCINFO);
	fm.LoadString(IDS_STRING_PRINT_DOC_NAME);
	di.lpszDocName = fm.LockBuffer();
	di.lpszOutput = (LPTSTR)NULL;
	di.lpszDatatype = (LPTSTR)NULL;
	di.fwType = 0;
	StartDoc(pd.hDC, &di);
	StartPage(pd.hDC);
	fm.UnlockBuffer();
	SelectObject(pd.hDC, hTopicFont);
	TextOut(pd.hDC, nHorRes / 3, nYMargin, fm, fm.GetLength());

	////////////////////////////////////////////////
	//�������еĿ��ȣ���ʹ�����ں���Ĵ�ӡ���ʱ�����ȵĴ�ӡ��ֽ��. 
	int   space = (nHorRes - nXMargin*nCharWidth - nColX) / (nColCount);
	for (int i = 1; i < nColCount; i++) {
		ca[i].nPrintX += i*space;
	}
	SelectObject(pd.hDC, hCodeFont);
	//����б���
	for (int i = 0; i < nColCount; i++) {
		TextOut(pd.hDC, ca[i].nPrintX, nYMargin + 260,
				ca[i].strColText, ca[i].strColText.GetLength());
	}

	int   nMaxLinePerPage = nVerRes / nCharHeight - 3;
	int   nCurPage = 1;
	SelectObject(pd.hDC, hRecordFont);//������ѡ���豸��������

	//HWND hd = ::GetDesktopWindow();
	//HDC ddc = ::GetDC(hd);
	//������е�����   
	int  nItem = -1;
	for (int i = 0; i < nLineCount; i++) {
		nItem = list.GetNextItem(nItem, LVNI_SELECTED);
		ASSERT(nItem != -1);
		for (int j = 0; j<nColCount; j++) {
			if (i + 1 - (nCurPage - 1)*nMaxLinePerPage > nMaxLinePerPage) {
				//�µ�һҳ   
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

	//��ӡ����
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


BOOL CHistoryRecordDlg::GetBegEndDateTime(CString& strBeg, CString& strEnd)
{
	UpdateData();
	CTime begDate, begTime, endDate, endTime;
	if (!GetDateTimeValue(m_begDate, begDate)
		|| !GetDateTimeValue(m_begTime, begTime)
		|| !GetDateTimeValue(m_endDate, endDate)
		|| !GetDateTimeValue(m_endTime, endTime)) {
		CString e; e.LoadStringW(IDS_STRING_TIME_NOT_SET);
		MessageBox(e, L"", MB_ICONERROR);
		return FALSE;
	}

	CTime beg(begDate.GetYear(), begDate.GetMonth(), begDate.GetDay(),
			  begTime.GetHour(), begTime.GetMinute(), begTime.GetSecond());
	CTime end(endDate.GetYear(), endDate.GetMonth(), endDate.GetDay(),
			  endTime.GetHour(), endTime.GetMinute(), endTime.GetSecond());

	CString fmTime; fmTime.LoadStringW(IDS_STRING_TIME_FORMAT);
	strBeg = beg.Format(fmTime);
	strEnd = end.Format(fmTime);
	CLog::WriteLog(_T("strBeg:%s strEnd:%s\n"), strBeg, strEnd);

	CTimeSpan span = end - beg;
	if (span.GetTotalMinutes() <= 0) {
		CString e; e.LoadStringW(IDS_STRING_TIME_ERROR);
		MessageBox(e, L"", MB_ICONERROR);
		return FALSE;
	}
	return TRUE;
}

void CHistoryRecordDlg::OnButtonSelByDate()
{
	CString strBeg, strEnd;
	if (!GetBegEndDateTime(strBeg, strEnd))
		return;

	ClearListCtrlAndFreeData(); CAutoRedrawListCtrl noname(m_listCtrlRecord);
	CHistoryRecord::GetInstance()->GetHistoryRecordByDate(strBeg, strEnd, this,
														  OnShowHistoryRecordCB);
	m_nPageCur = m_nPageTotal = 1;
	CString page = _T("");
	page.Format(_T("%d/%d"), m_nPageCur, m_nPageTotal);
	m_page.SetWindowText(page);
	m_cmbPerPage.SetCurSel(-1);
}

BOOL CHistoryRecordDlg::GetDateTimeValue(CDateTimeCtrl &ctrl, CTime &value)
{
	DWORD dwResult = ctrl.GetTime(value);
	if (dwResult == GDT_VALID) {
		//// the user checked the box and specified data
		//CString str;
		//CString fmTime; fmTime.LoadStringW(IDS_STRING_TIME_FORMAT);
		//// is it a time-only control, or a date-only control?
		//if ((ctrl.GetStyle() & DTS_TIMEFORMAT) == DTS_TIMEFORMAT)
		//	str = value.Format(fmTime);
		//else
		//	str = value.Format(fmTime);
		////AfxMessageBox(str);
		return TRUE;
	} else {
		// the user unmarked the "none" box
		//AfxMessageBox(_T("Time not set!"));
		return FALSE;
	}
}

void CHistoryRecordDlg::OnButtonSelAlarmByDate()
{
	CString strBeg, strEnd;
	if (!GetBegEndDateTime(strBeg, strEnd))
		return;
	ClearListCtrlAndFreeData(); CAutoRedrawListCtrl noname(m_listCtrlRecord);
	CHistoryRecord::GetInstance()->GetHistoryRecordByDateByAlarm(strBeg, strEnd, this,
																 OnShowHistoryRecordCB);
	m_nPageCur = m_nPageTotal = 1;
	CString page = _T("");
	page.Format(_T("%d/%d"), m_nPageCur, m_nPageTotal);
	m_page.SetWindowText(page);
	m_cmbPerPage.SetCurSel(-1);
}

void CHistoryRecordDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	ClearListCtrlAndFreeData();
	m_listCtrlRecord.ReleaseDC(m_dcList);
}


void CHistoryRecordDlg::OnNMCustomdrawListRecord(NMHDR *pNMHDR, LRESULT *pResult)
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
		POSITION pos = m_listCtrlRecord.GetFirstSelectedItemPosition();
		int index = m_listCtrlRecord.GetNextSelectedItem(pos);
		if (index == nItem) { //���Ҫˢ�µ���Ϊ��ǰѡ������������Ϊ��ɫ������ɫ��Ϊ��ɫ
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


void CHistoryRecordDlg::OnBnClickedButtonSelByUser()
{
	CString strBeg, strEnd;
	if (!GetBegEndDateTime(strBeg, strEnd))
		return;
	ClearListCtrlAndFreeData(); CAutoRedrawListCtrl noname(m_listCtrlRecord);
	CHistoryRecord::GetInstance()->GetHistoryRecordByDateByUser(strBeg, strEnd, this,
																OnShowHistoryRecordCB);
	m_nPageCur = m_nPageTotal = 1;
	CString page = _T("");
	page.Format(_T("%d/%d"), m_nPageCur, m_nPageTotal);
	m_page.SetWindowText(page);
	m_cmbPerPage.SetCurSel(-1);
}


void CHistoryRecordDlg::OnBnClickedButtonSelByMachine()
{
	CString strBeg, strEnd;
	if (!GetBegEndDateTime(strBeg, strEnd))
		return;

	CChooseMachineDlg dlg;
	if (IDOK != dlg.DoModal())
		return;

	ClearListCtrlAndFreeData();
	CAutoRedrawListCtrl noname(m_listCtrlRecord);
	CHistoryRecord::GetInstance()->GetHistoryRecordByDateByMachine(dlg.m_ademco_id,
																   strBeg, strEnd, this,
																   OnShowHistoryRecordCB);
	m_nPageCur = m_nPageTotal = 1;
	CString page = _T("");
	page.Format(_T("%d/%d"), m_nPageCur, m_nPageTotal);
	m_page.SetWindowText(page);
	m_cmbPerPage.SetCurSel(-1);
}


void CHistoryRecordDlg::OnBnClickedButtonSelAll()
{
	for (int i = 0; i < m_listCtrlRecord.GetItemCount(); i++) {
		m_listCtrlRecord.SetItemState(i, LVIS_FOCUSED | LVIS_SELECTED, 
									  LVIS_FOCUSED | LVIS_SELECTED);
	}
}


void CHistoryRecordDlg::OnBnClickedButtonSelInvert()
{
	for (int i = 0; i < m_listCtrlRecord.GetItemCount(); i++) {
		if (m_listCtrlRecord.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED) {
			m_listCtrlRecord.SetItemState(i, 0, LVIS_FOCUSED | LVIS_SELECTED);
		} else {
			m_listCtrlRecord.SetItemState(i, LVIS_FOCUSED | LVIS_SELECTED,
										  LVIS_FOCUSED | LVIS_SELECTED);
		}
	}
}


void CHistoryRecordDlg::OnBnClickedButtonSelNone()
{
	for (int i = 0; i < m_listCtrlRecord.GetItemCount(); i++) {
		m_listCtrlRecord.SetItemState(i, 0, LVIS_FOCUSED | LVIS_SELECTED);
	}
}