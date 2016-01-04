// ExportHrProcessDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "ExportHrProcessDlg.h"
#include "afxdialogex.h"
#include "HistoryRecord.h"
#include <odbcinst.h>
#include <afxdb.h>
#include <comdef.h>

class CExportHrProcessDlg::TraverseRecordObserver : public dp::observer<core::HistoryRecordPtr>
{
public:
	explicit CExportHrProcessDlg::TraverseRecordObserver(CExportHrProcessDlg* dlg) : _dlg(dlg) {}
	void on_update(const core::HistoryRecordPtr& ptr) {
		if (_dlg) {
			static CString sSql;
			sSql.Format(_T("INSERT INTO HISTORY_RECORD (Id,RecordTime,Record) VALUES('%d','%s','%s')"),
						ptr->id, ptr->record_time, ptr->record);
			_dlg->m_pDatabase->ExecuteSQL(sSql);
			_dlg->m_nCurProgress++;
		}
	}
private:
	CExportHrProcessDlg* _dlg;
};
// CExportHrProcessDlg dialog

IMPLEMENT_DYNAMIC(CExportHrProcessDlg, CDialogEx)

CExportHrProcessDlg::CExportHrProcessDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CExportHrProcessDlg::IDD, pParent)
	, m_pDatabase(nullptr)
	, m_dwStartTime(0)
	, m_bOver(FALSE)
	, m_nCurProgress(0)
	, m_nTotalCount(core::MAX_HISTORY_RECORD)
	, m_excelPath(L"")
	, m_bOpenAfterExport(FALSE)
{

}

CExportHrProcessDlg::~CExportHrProcessDlg()
{
}

void CExportHrProcessDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
	DDX_Control(pDX, IDC_STATIC_PROGRESS, m_staticProgress);
	DDX_Control(pDX, IDC_STATIC_TIME, m_staticTime);
}


BEGIN_MESSAGE_MAP(CExportHrProcessDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CExportHrProcessDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CExportHrProcessDlg::OnBnClickedCancel)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CExportHrProcessDlg message handlers


void CExportHrProcessDlg::OnBnClickedOk()
{
}


void CExportHrProcessDlg::OnBnClickedCancel()
{
	if(m_pDatabase)
		m_pDatabase->Close();
}

namespace {
	CString GetExcelDriver()
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
};

BOOL CExportHrProcessDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_traverse_record_observer = std::make_shared<TraverseRecordObserver>(this);

	m_progress.SetRange32(0, m_nTotalCount);
	CString txt(L"");
	txt.Format(L"0/%d", m_nTotalCount);
	m_staticProgress.SetWindowTextW(txt);

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
	if (m_excelPath.IsEmpty()) {
		m_excelPath.Format(L"%s\\history", GetModuleFilePath());
		CreateDirectory(m_excelPath, nullptr);
		SYSTEMTIME st = { 0 };
		GetLocalTime(&st);
		CString filen(L"");
		filen.Format(L"\\%04d-%02d-%02d_%02d-%02d-%02d.xls", st.wYear, st.wMonth, st.wDay,
					 st.wHour, st.wMinute, st.wSecond);
		m_excelPath += filen;
	}
	sSql.Format(_T("DRIVER={%s};DSN='';FIRSTROWHASNAMES=1;READONLY=FALSE;CREATE_DB=\")%s\";DBQ=%s"),
				sDriver, m_excelPath, m_excelPath);

	// 创建数据库 (既Excel表格文件)
	m_pDatabase = std::make_shared<CDatabase>();
	if (!m_pDatabase->OpenEx(sSql, CDatabase::noOdbcDialog)) {
		CString e;
		e.LoadStringW(IDS_STRING_E_CREATE_EXCEL);
		MessageBox(e, L"", MB_ICONERROR);
		return FALSE;
	}

	sSql.Format(_T("CREATE TABLE HISTORY_RECORD(Id TEXT,RecordTime TEXT,Record TEXT)"));
	m_pDatabase->ExecuteSQL(sSql);

	m_dwStartTime = GetTickCount();
	SetTimer(1, 100, nullptr);
	
	m_hThread = CreateThread(nullptr, 0, ThreadWorker, this, 0, nullptr);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CExportHrProcessDlg::OnTimer(UINT_PTR nIDEvent)
{
	CDialogEx::OnTimer(nIDEvent);

	DWORD now = GetTickCount();
	DWORD span = now - m_dwStartTime;
	CString txt(L"");
	txt.Format(L"%02d:%02d", span / 1000 / 60, span / 1000 % 60);
	m_staticTime.SetWindowTextW(txt);
	txt.Format(L"%d/%d", m_nCurProgress, m_nTotalCount);
	m_staticProgress.SetWindowTextW(txt);
	m_progress.SetPos(m_nCurProgress);

	if (m_bOver) {
		KillTimer(1);
		m_pDatabase->Close();
		m_pDatabase = nullptr;
		WaitForSingleObject(m_hThread, INFINITE);
		CLOSEHANDLE(m_hThread);

		core::CHistoryRecord* hr = core::CHistoryRecord::GetInstance();
		hr->DeleteAllRecored();

		if (m_bOpenAfterExport) {
			CString fm;
			fm.LoadString(IDS_STRING_FM_EXCEL_OK);
			txt.Format(fm, m_excelPath);
			if (IDYES == MessageBox(txt, L"", MB_YESNO | MB_ICONQUESTION)) {
				ShellExecute(nullptr, _T("Open"), m_excelPath, nullptr, nullptr, SW_SHOW);
			}
		}
		CDialogEx::OnOK();
	}
}


DWORD WINAPI CExportHrProcessDlg::ThreadWorker(LPVOID lp)
{
	CExportHrProcessDlg* dlg = reinterpret_cast<CExportHrProcessDlg*>(lp);
	core::CHistoryRecord* hr = core::CHistoryRecord::GetInstance();
	hr->TraverseHistoryRecord(dlg->m_traverse_record_observer);
	dlg->m_bOver = TRUE;
	return 0;
}
