// ExportHrProcessDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "ExportHrProcessDlg.h"
#include "afxdialogex.h"
#include "HistoryRecord.h"
#include "../contrib/sqlitecpp/SQLiteCpp.h"
#include "alarm_handle_mgr.h"
#include "UserInfo.h"
#include "HistoryRecordDlg.h"

#include <afxdb.h>
#include <odbcinst.h>
#include <comdef.h>

class CExportHrProcessDlg::TraverseRecordObserver : public dp::observer<core::history_record_ptr>
{
public:
	explicit CExportHrProcessDlg::TraverseRecordObserver(CExportHrProcessDlg* dlg) : _dlg(dlg) {}
	virtual void on_update(const core::history_record_ptr& ptr) {
		if (_dlg) {
			CString sSql;
			sSql.Format(_T("INSERT INTO HISTORY_RECORD (Id,RecordTime,Record) VALUES('%d','%s','%s')"),
						ptr->id, ptr->record_time, SQLite::double_quotes(ptr->record).c_str());
			_dlg->m_pDatabase->ExecuteSQL(sSql);
			_dlg->m_nCurProgress++;
		}
	}
private:
	CExportHrProcessDlg* _dlg;
};

class CExportHrProcessDlg::travers_alarm_observer : public dp::observer<core::alarm_ptr>
{
public:
	explicit CExportHrProcessDlg::travers_alarm_observer(CExportHrProcessDlg* dlg) : _dlg(dlg) {}
	virtual void on_update(const core::alarm_ptr& ptr) {
		if (_dlg) {
			CString sSql;
			std::wstring suser, sguard;
			auto user = core::user_manager::get_instance()->get_user_info(ptr->get_user_id());
			if (user) {
				suser = user->get_name();
			}
			if (ptr->get_handle_id() != 0) {
				auto mgr = core::alarm_handle_mgr::get_instance();
				auto handle = mgr->get_alarm_handle(ptr->get_handle_id());
				if (handle) {
					auto guard = mgr->get_security_guard(handle->get_guard_id());
					if (guard) {
						sguard = guard->get_name();
					}
				}
			}

			sSql.Format(_T("INSERT INTO ALARM_RECORD (Id,Status,SheetMaker,Guard,Record) VALUES('%d','%s','%s','%s','%s')"),
						ptr->get_id(), ptr->get_alarm_status_text(ptr->get_status()).c_str(), SQLite::double_quotes(suser).c_str(),
						SQLite::double_quotes(sguard).c_str(), SQLite::double_quotes(ptr->get_text(false)).c_str());
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


BOOL CExportHrProcessDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowTextW(TR(export_history_ ? IDS_STRING_IDD_DIALOG_EXPORT_HR_PROGRESS : IDS_STRING_IDD_DIALOG_EXPORT_ALARM_PROGRESS));

	if (export_history_) {
		m_traverse_record_observer = std::make_shared<TraverseRecordObserver>(this);
	} else {
		m_traverse_alarm_observer = std::make_shared<travers_alarm_observer>(this);
	}

	m_progress.SetRange32(0, m_nTotalCount);
	CString txt(L"");
	txt.Format(L"0/%d", m_nTotalCount);
	m_staticProgress.SetWindowTextW(txt);

	CString sDriver = _T("");
	CString sSql = _T("");

	// 检索是否安装有Excel驱动 "Microsoft Excel Driver (*.xls)" 
	sDriver = CHistoryRecordDlg::GetExcelDriver();
	if (sDriver.IsEmpty()) {
		// 没有发现Excel驱动
		CString e;
		e = TR(IDS_STRING_E_NO_EXECEL);
		MessageBox(e, L"", MB_ICONERROR);
		return FALSE;
	}

	// 创建进行存取的字符串
	if (m_excelPath.IsEmpty()) {
		m_excelPath.Format(L"%s\\data\\history", get_exe_path().c_str());
		CreateDirectory(m_excelPath, nullptr);
		SYSTEMTIME st = { 0 };
		GetLocalTime(&st);
		CString filen(L"");
		filen.Format(L"\\%04d-%02d-%02d_%02d-%02d-%02d--%s.xls", st.wYear, st.wMonth, st.wDay,
					 st.wHour, st.wMinute, st.wSecond, export_history_ ? TR(IDS_STRING_HISTORY_RECORD) : TR(IDS_STRING_IDC_STATIC_123));
		m_excelPath += filen;
	}
	sSql.Format(_T("DRIVER={%s};DSN='';FIRSTROWHASNAMES=1;READONLY=FALSE;CREATE_DB=\")%s\";DBQ=%s"),
				sDriver, m_excelPath, m_excelPath);

	// 创建数据库 (既Excel表格文件)
	m_pDatabase = std::make_shared<CDatabase>();
	if (!m_pDatabase->OpenEx(sSql, CDatabase::noOdbcDialog)) {
		CString e;
		e = TR(IDS_STRING_E_CREATE_EXCEL);
		MessageBox(e, L"", MB_ICONERROR);
		return FALSE;
	}

	if (export_history_) {
		sSql.Format(_T("CREATE TABLE HISTORY_RECORD(Id TEXT,RecordTime TEXT,Record TEXT)"));
	} else {
		sSql.Format(L"CREATE TABLE ALARM_RECORD(Id TEXT,Status TEXT,SheetMaker TEXT,Guard TEXT,Record TEXT)");
	}
	
	m_pDatabase->ExecuteSQL(sSql);

	m_dwStartTime = GetTickCount();
	SetTimer(1, 100, nullptr);
	
	running_ = true;
	thread_ = std::thread(&CExportHrProcessDlg::ThreadWorker, this);

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
		
		running_ = false;
		thread_.join();

		if (export_history_) {
			auto hr = core::history_record_manager::get_instance();
			hr->DeleteHalfRecored();
		} else {
			core::alarm_handle_mgr::get_instance()->delete_half_record();
		}

		if (m_bOpenAfterExport) {
			CString fm;
			fm = TR(IDS_STRING_FM_EXCEL_OK);
			txt.Format(fm, m_excelPath);
			if (IDYES == MessageBox(txt, L"", MB_YESNO | MB_ICONQUESTION)) {
				ShellExecute(nullptr, _T("Open"), m_excelPath, nullptr, nullptr, SW_SHOW);
			}
		}
		CDialogEx::OnOK();
	}
}


void CExportHrProcessDlg::ThreadWorker()
{
	if (export_history_) {
		auto hr = core::history_record_manager::get_instance();
		hr->TraverseHistoryRecord(m_traverse_record_observer);
	} else {
		core::alarm_handle_mgr::get_instance()->travers_alarm_record(m_traverse_alarm_observer);
	}
	
	m_bOver = TRUE;
}
