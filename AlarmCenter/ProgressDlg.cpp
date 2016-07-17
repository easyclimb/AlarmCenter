// ProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "ProgressDlg.h"
#include "afxdialogex.h"
#include "AlarmMachineManager.h"
#include "AlarmMachine.h"

namespace detail {

	void __stdcall OnLoadFromDBProgress(void* udata, const core::progress_ex_ptr& progress)
	{
		AUTO_LOG_FUNCTION;
		CLoadFromDBProgressDlg* dlg = reinterpret_cast<CLoadFromDBProgressDlg*>(udata); assert(dlg);
		dlg->AddProgress(progress);
	}

	void ThreadWorker(LPVOID lp)
	{
		auto mgr = core::alarm_machine_manager::get_instance();
		mgr->LoadFromDB(lp, OnLoadFromDBProgress);
	}
};
// CLoadFromDBProgressDlg dialog

IMPLEMENT_DYNAMIC(CLoadFromDBProgressDlg, CDialogEx)

CLoadFromDBProgressDlg::CLoadFromDBProgressDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CLoadFromDBProgressDlg::IDD, pParent)
	, m_dwStartTime(0)
	, m_dwCheckTime(0)
{

}

CLoadFromDBProgressDlg::~CLoadFromDBProgressDlg()
{
}

void CLoadFromDBProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
	DDX_Control(pDX, IDC_STATIC_NOTE, m_staticNote);
	DDX_Control(pDX, IDC_STATIC_NOTE2, m_staticNote2);
	DDX_Control(pDX, IDC_PROGRESS2, m_progress2);
	DDX_Control(pDX, IDC_STATIC_TIME, m_staticTime);
}


BEGIN_MESSAGE_MAP(CLoadFromDBProgressDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CLoadFromDBProgressDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CLoadFromDBProgressDlg::OnBnClickedCancel)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CLoadFromDBProgressDlg message handlers


void CLoadFromDBProgressDlg::OnBnClickedOk()
{
	return;
	//CDialogEx::OnOK();
}


void CLoadFromDBProgressDlg::OnBnClickedCancel()
{
	return;
	//CDialogEx::OnCancel();
}


BOOL CLoadFromDBProgressDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CenterWindow(GetParent());

	CString note;
	m_dwStartTime = GetTickCount();
	m_dwCheckTime = GetTickCount();
	m_staticTime.SetWindowTextW(L"00:00");

	m_progress.SetRange32(0, core::MAX_MACHINE);
	m_progress.SetPos(0);
	note.Format(L"%d/%d", 0, core::MAX_MACHINE);
	m_staticNote.SetWindowTextW(note);

	m_progress2.SetRange32(0, core::MAX_MACHINE_ZONE);
	m_progress2.SetPos(0);
	note.Format(L"%d/%d", 0, core::MAX_MACHINE_ZONE);
	m_staticNote2.SetWindowTextW(note);

	ShowWindow(SW_SHOW);
	UpdateWindow();

	thread_ = std::thread(detail::ThreadWorker, this);

	SetTimer(1, 10, nullptr);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CLoadFromDBProgressDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	
	thread_.join();

	m_progressList.clear();
}


void CLoadFromDBProgressDlg::AddProgress(const core::progress_ex_ptr& progress)
{
	std::lock_guard<std::mutex> lock(m_lock4Progress);
	m_progressList.push_back(progress);
}


void CLoadFromDBProgressDlg::OnClose()
{
	CDialogEx::OnOK();
}


void CLoadFromDBProgressDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (m_lock4Progress.try_lock()) {
		std::lock_guard<std::mutex> lock(m_lock4Progress, std::adopt_lock);
		if (m_progressList.empty())return;
		auto pex = m_progressList.back();
		CString note;
		if (pex->main) {
			m_progress.SetPos(pex->progress);
			note.Format(L"%d/%d", pex->value, core::MAX_MACHINE);
			m_staticNote.SetWindowTextW(note);
		} else {
			m_progress2.SetPos(pex->progress);
			note.Format(L"%d/%d", pex->value, core::MAX_MACHINE_ZONE);
			m_staticNote2.SetWindowTextW(note);
		}

		DWORD now = GetTickCount();
		DWORD elapse = now - m_dwCheckTime;
		if (static_cast<int>(elapse / 1000) > 0) {
			elapse = static_cast<int>((now - m_dwStartTime) / 1000);
			note.Format(L"%02d:%02d", static_cast<int>(elapse / 60), elapse % 60);
			m_staticTime.SetWindowTextW(note);
			m_dwCheckTime = now;
		}

		if (pex->main && pex->progress == core::MAX_MACHINE) {
			m_progress.SetPos(pex->progress);
			note.Format(L"%d/%d", core::MAX_MACHINE, core::MAX_MACHINE);
			m_staticNote.SetWindowTextW(note);
			UpdateWindow();
			KillTimer(1);
			PostMessage(WM_CLOSE);
		}
		m_progressList.clear();
	}
	CDialogEx::OnTimer(nIDEvent);
}
