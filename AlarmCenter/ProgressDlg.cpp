// ProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "ProgressDlg.h"
#include "afxdialogex.h"
#include "AlarmMachineManager.h"
#include "AlarmMachine.h"

namespace {
	void __stdcall OnLoadFromDBProgress(void* udata, core::ProgressExPtr progress)
	{
		AUTO_LOG_FUNCTION;
		CLoadFromDBProgressDlg* dlg = reinterpret_cast<CLoadFromDBProgressDlg*>(udata); assert(dlg);
		dlg->AddProgress(progress);
	}
};
// CLoadFromDBProgressDlg dialog

IMPLEMENT_DYNAMIC(CLoadFromDBProgressDlg, CDialogEx)

CLoadFromDBProgressDlg::CLoadFromDBProgressDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CLoadFromDBProgressDlg::IDD, pParent)
	, m_dwStartTime(0)
	, m_dwCheckTime(0)
	, m_hThread(INVALID_HANDLE_VALUE)
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

namespace {
	static DWORD WINAPI ThreadWorker(LPVOID lp)
	{
		core::CAlarmMachineManager* mgr = core::CAlarmMachineManager::GetInstance();
		mgr->LoadFromDB(lp, OnLoadFromDBProgress);
		return 0;
	}
};


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

	m_hThread = CreateThread(nullptr, 0, ThreadWorker, this, CREATE_SUSPENDED, nullptr);
	SetThreadPriority(m_hThread, THREAD_PRIORITY_ABOVE_NORMAL);
	ResumeThread(m_hThread);
	SetTimer(1, 1, nullptr);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CLoadFromDBProgressDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	WaitForSingleObject(m_hThread, INFINITE);
	CLOSEHANDLE(m_hThread);

	m_progressList.clear();
}


void CLoadFromDBProgressDlg::AddProgress(core::ProgressExPtr progress)
{
	m_lock4Progress.Lock();
	m_progressList.push_back(progress);
	m_lock4Progress.UnLock();
}


void CLoadFromDBProgressDlg::OnClose()
{
	CDialogEx::OnOK();
}


void CLoadFromDBProgressDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (m_lock4Progress.TryLock()) {
		if (m_progressList.size() == 0) {
			m_lock4Progress.UnLock();
			return;
		}
		auto pex = m_progressList.front();
		m_progressList.pop_front();
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
		m_lock4Progress.UnLock();
	}
	CDialogEx::OnTimer(nIDEvent);
}
