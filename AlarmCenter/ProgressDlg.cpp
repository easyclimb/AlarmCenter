// ProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "ProgressDlg.h"
#include "afxdialogex.h"
#include "AlarmMachineManager.h"
#include "AlarmMachine.h"

namespace {
	void __stdcall OnLoadFromDBProgress(void* udata, bool bmain, const core::ProgressEx* progress)
	{
		AUTO_LOG_FUNCTION;
		CLoadFromDBProgressDlg* dlg = reinterpret_cast<CLoadFromDBProgressDlg*>(udata); assert(dlg);
		//dlg->SendMessage(WM_PROGRESSEX, static_cast<WPARAM>(bmain),
		//				 reinterpret_cast<LPARAM>(progress));
		const core::ProgressEx* pex = bmain ? progress : progress->subProgress;
		dlg->AddProgress(bmain, pex->progress, pex->value, pex->total);
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
	ON_MESSAGE(WM_PROGRESSEX, &CLoadFromDBProgressDlg::OnProgressEx)
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

	for (auto pex : m_progressList) {
		delete pex;
	}
	m_progressList.clear();
}


void CLoadFromDBProgressDlg::AddProgress(bool main, int progress, int value, int total)
{
	m_lock4Progress.Lock();
	PPROGRESS_EX pex = new PROGRESS_EX(main, progress, value, total);
	m_progressList.push_back(pex);
	m_lock4Progress.UnLock();
}

afx_msg LRESULT CLoadFromDBProgressDlg::OnProgressEx(WPARAM wParam, LPARAM lParam)
{
	AUTO_LOG_FUNCTION;
	//int progress = static_cast<int>(wParam);
	//progress = static_cast<int>(progress / core::MAX_MACHINE);
	BOOL bmain = static_cast<BOOL>(wParam);
	const core::ProgressEx* progress = reinterpret_cast<const core::ProgressEx*>(lParam);
	core::ProgressEx* subProgress = progress->subProgress;
	CString note;
	if (bmain) {
		m_progress.SetPos(progress->progress);
		note.Format(L"%d/%d", progress->value, core::MAX_MACHINE);
		m_staticNote.SetWindowTextW(note);
	} else if (subProgress) {
		m_progress2.SetPos(subProgress->progress);
		note.Format(L"%d/%d", subProgress->value, core::MAX_MACHINE_ZONE);
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

	if (bmain && progress->value == progress->total) {
		m_progress.SetPos(progress->progress);
		note.Format(L"%d/%d", core::MAX_MACHINE, core::MAX_MACHINE);
		m_staticNote.SetWindowTextW(note);
		UpdateWindow();
		PostMessage(WM_CLOSE);
	}

	return 0;
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
		PPROGRESS_EX pex = m_progressList.front();
		m_progressList.pop_front();
		CString note;
		if (pex->_main) {
			m_progress.SetPos(pex->_progress);
			note.Format(L"%d/%d", pex->_value, core::MAX_MACHINE);
			m_staticNote.SetWindowTextW(note);
		} else {
			m_progress2.SetPos(pex->_progress);
			note.Format(L"%d/%d", pex->_value, core::MAX_MACHINE_ZONE);
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

		if (pex->_main && pex->_progress == core::MAX_MACHINE) {
			m_progress.SetPos(pex->_progress);
			note.Format(L"%d/%d", core::MAX_MACHINE, core::MAX_MACHINE);
			m_staticNote.SetWindowTextW(note);
			UpdateWindow();
			KillTimer(1);
			PostMessage(WM_CLOSE);
		}
		delete pex;
		m_lock4Progress.UnLock();
	}
	CDialogEx::OnTimer(nIDEvent);
}
