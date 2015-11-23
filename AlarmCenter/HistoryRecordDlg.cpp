// HistoryRecordDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include <odbcinst.h>
#include <afxdb.h>
#include <comdef.h>
#include "HistoryRecordDlg.h"
#include "afxdialogex.h"
#include "ChooseMachineDlg.h"
#include "UserInfo.h"
#include "ExportHrProcessDlg.h"
#include <vector>
using namespace core;
// CHistoryRecordDlg dialog
#include "PickMachineCoordinateDlg.h"


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
	AUTO_LOG_FUNCTION;
	CHistoryRecordDlg* dlg = reinterpret_cast<CHistoryRecordDlg*>(udata); ASSERT(dlg);
	ASSERT(dlg->IsKindOf(RUNTIME_CLASS(CHistoryRecordDlg)));
	dlg->InsertListContent(record);
}

IMPLEMENT_DYNAMIC(CHistoryRecordDlg, CDialogEx)

CHistoryRecordDlg::CHistoryRecordDlg(CWnd* pParent /*=nullptr*/)
: CDialogEx(CHistoryRecordDlg::IDD, pParent)
, m_ademco_id(-1)
, m_zone_value(-1)
, m_nPageCur(0)
, m_nPageTotal(0)
, m_nPerPage(30)
, m_hIcon(nullptr)
, m_bDraging(FALSE)
, m_dcList(nullptr)
, m_pDatabase(nullptr)
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
	//ON_COMMAND(IDC_BUTTON_PRINT, OnButtonPrint)
	ON_BN_CLICKED(IDC_BUTTON_SEL_BY_DATE, OnButtonSelByDate)
	ON_BN_CLICKED(IDC_BUTTON_SEL_ALARM, OnButtonSelByLevelAndDate)
	ON_BN_CLICKED(IDC_BUTTON_PAGE_FIRST, OnButtonPageFirst)
	ON_BN_CLICKED(IDC_BUTTON_PAGE_PREV, OnButtonPagePrev)
	ON_BN_CLICKED(IDC_BUTTON_PAGE_NEXT, OnButtonPageNext)
	ON_BN_CLICKED(IDC_BUTTON_PAGE_LAST, OnButtonPageLast)
	ON_BN_CLICKED(IDC_BUTTON_CLR_HISTORY, OnButtonDeleteAllRecord)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_EXPORT_SEL, &CHistoryRecordDlg::OnBnClickedButtonExportSel)
	ON_BN_CLICKED(IDC_BUTTON_SEL_BY_USER, &CHistoryRecordDlg::OnBnClickedButtonSelByUser)
	ON_BN_CLICKED(IDC_BUTTON_SEL_BY_MACHINE, &CHistoryRecordDlg::OnBnClickedButtonSelByMachine)
	ON_BN_CLICKED(IDC_BUTTON_SEL_ALL, &CHistoryRecordDlg::OnBnClickedButtonSelAll)
	ON_BN_CLICKED(IDC_BUTTON_SEL_INVERT, &CHistoryRecordDlg::OnBnClickedButtonSelInvert)
	ON_BN_CLICKED(IDC_BUTTON_SEL_NONE, &CHistoryRecordDlg::OnBnClickedButtonSelNone)
	ON_BN_CLICKED(IDC_BUTTON_PRINT, &CHistoryRecordDlg::OnBnClickedButtonPrint)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_RECORD, &CHistoryRecordDlg::OnNMRClickListRecord)
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
		AUTO_LOG_FUNCTION;
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

		/*CHistoryRecord *hr = CHistoryRecord::GetInstance();
		int total = hr->GetRecordCount();
		int pageTotal = total / m_nPerPage;
		if (total % m_nPerPage != 0)
		pageTotal++;
		if (m_nPageTotal != pageTotal) {
		m_nPageTotal = pageTotal;
		m_nPageCur = 1;
		}
		LoadRecordsBasedOnPage(m_nPageCur);*/
	}
}

namespace {
	void __stdcall OnCurUserChanged(void* udata, const core::CUserInfo* user)
	{
		if (!udata || !user)
			return;

		CHistoryRecordDlg* dlg = reinterpret_cast<CHistoryRecordDlg*>(udata);
		if (user->get_user_priority() == core::UP_OPERATOR) {
			dlg->m_btnExport.EnableWindow(0);
		} else {
			dlg->m_btnExport.EnableWindow(1);
		}
	}
};


BOOL CHistoryRecordDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	core::CUserManager::GetInstance()->RegisterObserver(this, OnCurUserChanged);
	OnCurUserChanged(this, core::CUserManager::GetInstance()->GetCurUserInfo());

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
	int total = 0;
	if (m_ademco_id == -1) {
		total = hr->GetRecordCount();
		m_nPageTotal = total / m_nPerPage;
		if (total % m_nPerPage != 0)
			m_nPageTotal++;
	} else {
		m_nPageTotal = 1;
		m_cmbPerPage.SetCurSel(-1);
		m_cmbPerPage.EnableWindow(0);
		if (m_zone_value == -1) {
			//total = hr->GetRecordConntByMachine(m_ademco_id);
			CString txt, newtxt, smachine;
			smachine.LoadStringW(IDS_STRING_MACHINE);
			GetWindowText(txt);
			newtxt.Format(L"%s %s%04d", txt, smachine, m_ademco_id);
			SetWindowText(newtxt);
		} else {
			//total = hr->GetRecordConntByMachineAndZone(m_ademco_id, m_zone_value);
			CString txt, newtxt, smachine, ssubmachine;
			smachine.LoadStringW(IDS_STRING_MACHINE);
			ssubmachine.LoadStringW(IDS_STRING_SUBMACHINE);
			GetWindowText(txt);
			newtxt.Format(L"%s %s%04d %s%03d", txt, smachine, m_ademco_id,
						  ssubmachine, m_zone_value);
			SetWindowText(newtxt);
		}
	}

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
	m_listCtrlRecord.InsertColumn(++i, fm, LVCFMT_LEFT, 180, -1);
	fm.LoadStringW(IDS_STRING_HRLV);
	m_listCtrlRecord.InsertColumn(++i, fm, LVCFMT_LEFT, 100, -1);
	fm.LoadStringW(IDS_STRING_HISTORY_RECORD);
	m_listCtrlRecord.InsertColumn(++i, fm, LVCFMT_LEFT, 1500, -1);
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

		// 类型
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), GetRecordLevelString(record->level));
		lvitem.pszText = tmp.LockBuffer();
		m_listCtrlRecord.SetItem(&lvitem);
		tmp.UnlockBuffer();

		//记录
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
	if (m_nPageCur == 0)
		m_nPageCur = 1;
	LoadRecordsBasedOnPage(m_nPageCur - 1);
}

void CHistoryRecordDlg::OnUpdateButtonSeperator(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(FALSE);
}

void CHistoryRecordDlg::LoadRecordsBasedOnPage(const int nPage)
{
	AUTO_LOG_FUNCTION;
	ClearListCtrlAndFreeData();
	CAutoRedrawListCtrl noname(m_listCtrlRecord);
	CHistoryRecord *hr = CHistoryRecord::GetInstance();
	if (m_ademco_id == -1) {
		long baseID = hr->GetRecordMinimizeID();
		hr->GetTopNumRecordsBasedOnID((m_nPageTotal - nPage)*m_nPerPage + baseID,
									  m_nPerPage, this, OnShowHistoryRecordCB);
	} else {
		if (m_zone_value == -1) {
			/*long baseID = hr->GetRecordMinimizeIDByMachine(m_ademco_id);
			hr->GetTopNumRecordsBasedOnIDByMachine((m_nPageTotal - nPage)*m_nPerPage + baseID,
												   m_nPerPage, m_ademco_id, this,
												   OnShowHistoryRecordCB);*/
			hr->GetTopNumRecordByAdemcoID(1000, m_ademco_id, this, OnShowHistoryRecordCB, FALSE);
		} else {
			/*long baseID = hr->GetRecordMinimizeIDByMachineAndZone(m_ademco_id, m_zone_value);
			hr->GetTopNumRecordsBasedOnIDByMachineAndZone((m_nPageTotal - nPage)*m_nPerPage + baseID,
														  m_nPerPage, m_ademco_id,
														  m_zone_value, this,
														  OnShowHistoryRecordCB);*/
			hr->GetTopNumRecordByAdemcoIDAndZone(1000, m_ademco_id, m_zone_value, this, OnShowHistoryRecordCB, FALSE);
		}
	}
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
		if (m_page.m_hWnd == nullptr)
			break;
		rcItem.right = rcItem.left + 50;
		m_page.MoveWindow(rcItem);

		// 首页
		if (m_btnFirst.m_hWnd == nullptr)
			break;
		rcItem.left = rcItem.right + cBtnGaps * 5;
		rcItem.right = rcItem.left + cBtnWidth;
		m_btnFirst.MoveWindow(rcItem);

		// 上页
		if (m_btnPrev.m_hWnd == nullptr)
			break;
		rcItem.left = rcItem.right + cBtnGaps;
		rcItem.right = rcItem.left + cBtnWidth;
		m_btnPrev.MoveWindow(rcItem);

		// 下页
		if (m_btnNext.m_hWnd == nullptr)
			break;
		rcItem.left = rcItem.right + cBtnGaps;
		rcItem.right = rcItem.left + cBtnWidth;
		m_btnNext.MoveWindow(rcItem);

		// 尾页
		if (m_btnLast.m_hWnd == nullptr)
			break;
		rcItem.left = rcItem.right + cBtnGaps;
		rcItem.right = rcItem.left + cBtnWidth;
		m_btnLast.MoveWindow(rcItem);

		// 行/页
		if (m_staticPerPage.m_hWnd == nullptr)
			break;
		rcItem.left = rcItem.right + cBtnGaps;
		rcItem.right = rcItem.left + int(cBtnWidth * 1.5);
		m_staticPerPage.MoveWindow(rcItem);

		// 行/页
		if (m_cmbPerPage.m_hWnd == nullptr)
			break;
		rcItem.left = rcItem.right + 5;
		rcItem.right = rcItem.left + int(cBtnWidth * 2);
		m_cmbPerPage.MoveWindow(rcItem);

		// 导出全部到Excel
		if (m_btnExport.m_hWnd == nullptr)
			break;
		rcItem.left = rcItem.right + cBtnGaps * 2;
		rcItem.right = rcItem.left + int(cBtnWidth * 3);
		m_btnExport.MoveWindow(rcItem);

		// 导出选中到Excel
		if (m_btnExportSel.m_hWnd == nullptr)
			break;
		rcItem.left = rcItem.right + cBtnGaps;
		rcItem.right = rcItem.left + int(cBtnWidth * 3);
		m_btnExportSel.MoveWindow(rcItem);

		// 打印选中行
		if (m_btnPrint.m_hWnd == nullptr)
			break;
		rcItem.left = rcItem.right + cBtnGaps;
		rcItem.right = rcItem.left + int(cBtnWidth * 2);
		m_btnPrint.MoveWindow(rcItem);

		// 换行
		rcItem.left = rcToolBar.left + cBtnGaps;
		rcItem.top = rcItem.bottom + cBtnGaps;
		rcItem.bottom = rcItem.top + cBtnHeight;

		// 起始日期
		if (m_begDate.m_hWnd == nullptr)
			break;
		CRect rcDateTime;
		m_begDate.GetWindowRect(rcDateTime);
		//rcItem.left = rcItem.right + 15;
		rcItem.right = rcItem.left + rcDateTime.Width();
		m_begDate.MoveWindow(rcItem);

		// 起始时间
		if (m_begTime.m_hWnd == nullptr)
			break;
		rcItem.left = rcItem.right + 2;
		rcItem.right = rcItem.left + rcDateTime.Width();
		m_begTime.MoveWindow(rcItem);

		// 结束日期
		if (m_endDate.m_hWnd == nullptr)
			break;
		rcItem.left = rcItem.right + 5;
		rcItem.right = rcItem.left + rcDateTime.Width();
		m_endDate.MoveWindow(rcItem);

		// 结束时间
		if (m_endTime.m_hWnd == nullptr)
			break;
		rcItem.left = rcItem.right + 2;
		rcItem.right = rcItem.left + rcDateTime.Width();
		m_endTime.MoveWindow(rcItem);

		// 按日期查询
		if (m_btnSelByDate.m_hWnd == nullptr)
			break;
		rcItem.left = rcItem.right + 5;
		rcItem.right = rcItem.left + int(cBtnWidth * 2);
		m_btnSelByDate.MoveWindow(rcItem);

		// 查询报警信息
		if (m_btnSelAlarmByDate.m_hWnd == nullptr)
			break;
		rcItem.left = rcItem.right + 15;
		rcItem.right = rcItem.left + int(cBtnWidth * 2);
		m_btnSelAlarmByDate.MoveWindow(rcItem);

		// 按用户查询
		if (m_btnSelByUser.m_hWnd == nullptr)
			break;
		rcItem.left = rcItem.right + 5;
		rcItem.right = rcItem.left + int(cBtnWidth * 2);
		m_btnSelByUser.MoveWindow(rcItem);

		// 按主机查询
		if (m_btnSelByMachine.m_hWnd == nullptr)
			break;
		rcItem.left = rcItem.right + 5;
		rcItem.right = rcItem.left + int(cBtnWidth * 2);
		m_btnSelByMachine.MoveWindow(rcItem);

		//// 换行
		//rcItem.left = rcToolBar.left + cBtnGaps;
		//rcItem.top = rcItem.bottom + cBtnGaps;
		//rcItem.bottom = rcItem.top + cBtnHeight;

		//// 全选
		//if (m_btnSelAll.m_hWnd == nullptr)
		//	break;
		////rcItem.left = rcItem.right + 5;
		//rcItem.right = rcItem.left + int(cBtnWidth);
		//m_btnSelAll.MoveWindow(rcItem);

		//// 反选
		//if (m_btnSelInvert.m_hWnd == nullptr)
		//	break;
		//rcItem.left = rcItem.right + 5;
		//rcItem.right = rcItem.left + int(cBtnWidth);
		//m_btnSelInvert.MoveWindow(rcItem);

		//// 全不选
		//if (m_btnSelNone.m_hWnd == nullptr)
		//	break;
		//rcItem.left = rcItem.right + 5;
		//rcItem.right = rcItem.left + int(cBtnWidth);
		//m_btnSelNone.MoveWindow(rcItem);

		// 列表
		if (m_listCtrlRecord.m_hWnd == nullptr)
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
	//for (int i = 0; i < dlg->m_listCtrlRecord.GetItemCount(); i++) {
	int nItem = -1;
	for (UINT i = 0; i < dlg->m_listCtrlRecord.GetSelectedCount(); i++) {
		nItem = dlg->m_listCtrlRecord.GetNextItem(nItem, LVNI_SELECTED);
		if (nItem == -1) break;
		DWORD data = dlg->m_listCtrlRecord.GetItemData(nItem);
		const HistoryRecord* record = reinterpret_cast<const HistoryRecord*>(data);
		if (record)
			dlg->OnExportHistoryRecordCB(dlg, record);
	}
}


BOOL CHistoryRecordDlg::Export(const CString& excelPath, TraverseHistoryRecordCB cb)
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
	m_pDatabase = nullptr;
	// 关闭数据库
	database.Close();
	CString fm;
	fm.LoadString(IDS_STRING_FM_EXCEL_OK);
	warningStr.Format(fm, excelPath);
	if (IDYES == MessageBox(warningStr, L"", MB_YESNO | MB_ICONQUESTION)) {
		ShellExecute(nullptr, _T("Open"), excelPath, nullptr, nullptr, SW_SHOW);
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

	/*if (Export(path, ExportTraverseHistoryRecord)) {
		CHistoryRecord* hr = CHistoryRecord::GetInstance();
		hr->DeleteAllRecored();
		m_nPageTotal = 1;
		LoadRecordsBasedOnPage(1);
		}*/
	CHistoryRecord* hr = CHistoryRecord::GetInstance();
	CExportHrProcessDlg dlg;
	dlg.m_nTotalCount = hr->GetRecordCount();
	dlg.m_excelPath = path;
	dlg.m_bOpenAfterExport = TRUE;
	dlg.DoModal();

	CString s, fm;
	fm.LoadStringW(IDS_STRING_FM_USER_EXPORT_HR);
	const CUserInfo* user = CUserManager::GetInstance()->GetCurUserInfo();
	s.Format(fm, user->get_user_id(), user->get_user_name());
	hr->InsertRecord(-1, -1, s, time(nullptr), RECORD_LEVEL_USERCONTROL);

	m_nPageTotal = 1;
	LoadRecordsBasedOnPage(1);
}

void CHistoryRecordDlg::OnBnClickedButtonExportSel()
{
	POSITION pos = m_listCtrlRecord.GetFirstSelectedItemPosition();
	if (pos == nullptr) {
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
	if (pos == nullptr) {
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
			CString e; e.LoadStringW(IDS_STRING_E_TOOLMANY_FIELD);
			MessageBox(e, L"", MB_ICONERROR);
			return  FALSE;
		}
	}

	//设置打印文件的保存对话框 
	CString fm;
	DOCINFO   di;
	di.cbSize = sizeof(DOCINFO);
	fm.LoadString(IDS_STRING_PRINT_DOC_NAME);
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
//
//void CHistoryRecordDlg::OnButtonPrint()
//{
//	PrintRecord(m_listCtrlRecord);
//}


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


CString CHistoryRecordDlg::GetRecordLevelString(RecordLevel level)
{
	CString result = L"level";
	switch (level) {
		case core::RECORD_LEVEL_ONOFFLINE:
			result.LoadStringW(IDS_STRING_HRLV_ONOFFLINE);
			break;
		case core::RECORD_LEVEL_USERLOG:
			result.LoadStringW(IDS_STRING_HRLV_USER_LOG);
			break;
		case core::RECORD_LEVEL_USEREDIT:
			result.LoadStringW(IDS_STRING_HRLV_USER_EDIT);
			break;
		case core::RECORD_LEVEL_USERCONTROL:
			result.LoadStringW(IDS_STRING_HRLV_USER_CONTROL);
			break;
		case core::RECORD_LEVEL_ALARM:
			result.LoadStringW(IDS_STRING_HRLV_ALARM);
			break;
		case core::RECORD_LEVEL_EXCEPTION:
			result.LoadStringW(IDS_STRING_HRLV_EXCEPTION);
			break;
		case core::RECORD_LEVEL_VIDEO:
			result.LoadStringW(IDS_STRING_HRLV_VIDEO);
			break;
		case core::RECORD_LEVEL_SYSTEM:
			result.LoadStringW(IDS_STRING_HRLV_SYSTEM);
			break;
		default:
			break;
	}
	return result;
}


void CHistoryRecordDlg::OnButtonSelByLevelAndDate()
{
	AUTO_LOG_FUNCTION;
	CString strBeg, strEnd;
	if (!GetBegEndDateTime(strBeg, strEnd))
		return;

	CString lvOnoffLine, lvUserLog, lvUserEdit, lvUserControl, lvAlarm, lvException, lvVideo, lvSystem;
	lvOnoffLine.LoadStringW(IDS_STRING_HRLV_ONOFFLINE);
	lvUserLog.LoadStringW(IDS_STRING_HRLV_USER_LOG);
	lvUserEdit.LoadStringW(IDS_STRING_HRLV_USER_EDIT);
	lvUserControl.LoadStringW(IDS_STRING_HRLV_USER_CONTROL);
	lvAlarm.LoadStringW(IDS_STRING_HRLV_ALARM);
	lvException.LoadStringW(IDS_STRING_HRLV_EXCEPTION);
	lvVideo.LoadStringW(IDS_STRING_HRLV_VIDEO);
	lvSystem.LoadStringW(IDS_STRING_HRLV_SYSTEM);

	CMenu menu;
	menu.CreatePopupMenu();
	int ndx = 1;
	menu.AppendMenuW(MF_STRING, ndx++, lvOnoffLine);
	menu.AppendMenuW(MF_STRING, ndx++, lvUserLog);
	menu.AppendMenuW(MF_STRING, ndx++, lvUserEdit);
	menu.AppendMenuW(MF_STRING, ndx++, lvUserControl);
	menu.AppendMenuW(MF_STRING, ndx++, lvAlarm);
	menu.AppendMenuW(MF_STRING, ndx++, lvException);
	menu.AppendMenuW(MF_STRING, ndx++, lvVideo);
	menu.AppendMenuW(MF_STRING, ndx++, lvSystem);

	CRect rc;
	m_btnSelAlarmByDate.GetWindowRect(rc);
	DWORD ret = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
									rc.left, rc.bottom, this);
	JLOG(L"TrackPopupMenu ret %d\n", ret);

	RecordLevel recordLevel = RECORD_LEVEL_CLEARHR;
	switch (ret) {
		case 1:
			recordLevel = RECORD_LEVEL_ONOFFLINE;
			break;
		case 2:
			recordLevel = RECORD_LEVEL_USERLOG;
			break;
		case 3:
			recordLevel = RECORD_LEVEL_USEREDIT;
			break;
		case 4:
			recordLevel = RECORD_LEVEL_USERCONTROL;
			break;
		case 5:
			recordLevel = RECORD_LEVEL_ALARM;
			break;
		case 6:
			recordLevel = RECORD_LEVEL_EXCEPTION;
			break;
		case 7:
			recordLevel = RECORD_LEVEL_VIDEO;
			break;
		case 8:
			recordLevel = RECORD_LEVEL_SYSTEM;
			break;
		default:
			return;
			break;
	}

	ClearListCtrlAndFreeData(); CAutoRedrawListCtrl noname(m_listCtrlRecord);
	CHistoryRecord::GetInstance()->GetHistoryRecordByDateByRecordLevel(strBeg, strEnd, recordLevel, this,
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
	core::CUserManager::GetInstance()->UnRegisterObserver(this);
}


void CHistoryRecordDlg::OnBnClickedButtonSelByUser()
{
	CString strBeg, strEnd;
	if (!GetBegEndDateTime(strBeg, strEnd))
		return;
	CUserManager* mgr = CUserManager::GetInstance();
	const CUserInfo* curUser = mgr->GetCurUserInfo();
	CUserInfo* user = mgr->GetFirstUserInfo();
	bool bDisabled = curUser->get_user_priority() == UP_OPERATOR;
	CMenu menu;
	menu.CreatePopupMenu();
	int ndx = 1;
	CString txt;
	std::vector<int> userIdList;
	userIdList.push_back(0);
	while (user) {
		txt.Format(L"ID:%d(%s)", user->get_user_id(), user->get_user_name());
		menu.AppendMenuW(MF_STRING, ndx, txt);
		if (bDisabled && user != curUser) {
			menu.EnableMenuItem(ndx, MF_DISABLED | MF_GRAYED);
		}
		userIdList.push_back(user->get_user_id());
		user = mgr->GetNextUserInfo();
		ndx++;
	}

	int user_id = -1;
	CRect rc;
	m_btnSelByUser.GetWindowRect(rc);
	DWORD ret = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
									rc.left, rc.bottom, this);
	JLOG(L"TrackPopupMenu ret %d\n", ret);
	if (ret >= 1 && ret < userIdList.size()) {
		user_id = userIdList[ret];
	} else { return; }

	ClearListCtrlAndFreeData(); CAutoRedrawListCtrl noname(m_listCtrlRecord);
	CHistoryRecord::GetInstance()->GetHistoryRecordByDateByUser(strBeg, strEnd, user_id, this,
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


void CHistoryRecordDlg::OnBnClickedButtonPrint()
{
	PrintRecord(m_listCtrlRecord);
}


void CHistoryRecordDlg::OnNMRClickListRecord(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	HistoryRecord* record = reinterpret_cast<HistoryRecord*>(m_listCtrlRecord.GetItemData(pNMItemActivate->iItem));
	if (record && record->level == RECORD_LEVEL_VIDEO) {
		USES_CONVERSION;
		std::string rec = W2A(record->record);
		auto i = rec.find_first_of('"');
		auto j = rec.find_last_of('"');
		if (i > 0 && j > 0 && i < j - 1) {
			std::string spath = rec.substr(i + 1, j - i - 1);
			CString path = A2W(spath.c_str());
			if (CFileOper::PathExists(path)) {
				CMenu menu; menu.CreatePopupMenu();
				CString openFile, openFolder;
				openFile.LoadStringW(IDS_STRING_PLAY_VIDEO);
				openFolder.LoadStringW(IDS_STRING_OPEN_FOLDER);
				menu.AppendMenuW(MF_STRING, 1, openFile);
				menu.AppendMenuW(MF_STRING, 2, openFolder);
				CPoint pt;
				GetCursorPos(&pt);
				int ret = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, this);
				if (ret == 1) {
					ShellExecute(m_hWnd, L"open", path, nullptr, nullptr, SW_SHOW);
				} else if (ret == 2) {
					ShellExecute(m_hWnd, L"open", CFileOper::GetFolderPathFromFilePath(path), nullptr, nullptr, SW_SHOW);
				}
			}
		}
	} else if (record && (record->level == RECORD_LEVEL_ALARM || record->level == RECORD_LEVEL_EXCEPTION)) {
		CMenu menu; menu.CreatePopupMenu();
		CString seeBaiduMap;
		seeBaiduMap.LoadStringW(IDS_STRING_SEE_BAIDU_MAP);
		menu.AppendMenuW(MF_STRING, 1, seeBaiduMap);
		CPoint pt;
		GetCursorPos(&pt);
		int ret = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, this);
		if (ret == 1) {
			g_baiduMapDlg->ShowMap(record->ademco_id, record->zone_value);
		} 
	}
	*pResult = 0;
}
