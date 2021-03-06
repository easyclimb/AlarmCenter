﻿// AutoQueryDisconnectSubmachineDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AutoQueryDisconnectSubmachineDlg.h"
#include "afxdialogex.h"
#include <algorithm>
#include <iterator>
#include "AlarmMachine.h"
#include "AlarmMachineManager.h"
#include "ZoneInfo.h"
#include "AlarmCenter.h"



namespace detail {
	static const int TIMER_ID_AUTO_START = 1;
	static const int TIMER_ID_AUTO_QUIT = 2;
	static const int TIMER_ID_TIME = 3;
	static const int TIMER_ID_CHECK = 4;

	static const int MAX_RETRY_TIMES = 2;

#ifdef _DEBUG
	static const int MAX_QUERY_TIME = 2;
#else
	static const int MAX_QUERY_TIME = 20;
#endif

	//IMPLEMENT_ADEMCO_EVENT_CALL_BACK(CAutoQueryDisconnectSubmachineDlg, OnAdemcoEvent)

};

using namespace detail;
using namespace core;

// CAutoQueryDisconnectSubmachineDlg dialog
IMPLEMENT_DYNAMIC(CAutoQueryDisconnectSubmachineDlg, CDialogEx)

CAutoQueryDisconnectSubmachineDlg::CAutoQueryDisconnectSubmachineDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CAutoQueryDisconnectSubmachineDlg::IDD, pParent)
	, m_curQueryingSubMachine(nullptr)
	, m_bQuerying(FALSE)
	, m_dwStartTime(0)
	, m_dwQueryStartTime(0)
	, m_nRetryTimes(0)
	, m_strFmQuery(L"")
	, m_strFmQeurySuccess(L"")
	, m_strQueryFailed(L"")
	, m_strCancel(L"")
	, m_bQuerySuccess(FALSE)
	, m_nAutoStartCounter(0)
	, m_nAutoQuitCounter(0)
{

}

CAutoQueryDisconnectSubmachineDlg::~CAutoQueryDisconnectSubmachineDlg()
{
}

void CAutoQueryDisconnectSubmachineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDC_STATIC_TIME, m_staticTime);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Control(pDX, IDC_STATIC_PROGRESS, m_staticProgress);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CAutoQueryDisconnectSubmachineDlg, CDialogEx)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDOK, &CAutoQueryDisconnectSubmachineDlg::OnBnClickedOk)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDCANCEL, &CAutoQueryDisconnectSubmachineDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CAutoQueryDisconnectSubmachineDlg message handlers


BOOL CAutoQueryDisconnectSubmachineDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	Reset();
	CString query, machine, submachine, done;
	query = TR(IDS_STRING_QUERY);
	machine = TR(IDS_STRING_MACHINE);
	submachine = TR(IDS_STRING_SUBMACHINE);
	done = TR(IDS_STRING_DONE);

	m_strFmQuery = query + machine + TR(IDS_STRING_FM_ADEMCO_ID) + L"(%s) " + submachine  + L"%03d(%s)";
	m_strFmQeurySuccess = query + done + L"%03d(%s) %s";
	m_strQueryFailed = TR(IDS_STRING_QUERY_FAILED);

	m_btnCancel.GetWindowTextW(m_strCancel);

	KillTimer(TIMER_ID_AUTO_START);
	OnBnClickedOk();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CAutoQueryDisconnectSubmachineDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (TIMER_ID_TIME == nIDEvent) {
		DWORD cnt = GetTickCount() - m_dwStartTime;
		if (cnt >= 1000) {
			cnt /= 1000;
			int min = cnt / 60;
			int sec = cnt % 60;
			CString t; t.Format(L"%02d:%02d", min, sec);
			m_staticTime.SetWindowTextW(t);
		}
	} else if (TIMER_ID_CHECK == nIDEvent) {
		if (m_bQuerySuccess) {
			CString l; auto res = CAppResource::get_instance();
			ADEMCO_EVENT ademco_event = MachineStatus2AdemcoEvent(m_curQueryingSubMachine->get_machine_status());
			l.Format(m_strFmQeurySuccess, 
					 m_curQueryingSubMachine->get_submachine_zone(),
					 m_curQueryingSubMachine->get_machine_name(),
					 res->AdemcoEventToString(ademco_event));
			int ndx = m_list.InsertString(-1, l);
			m_list.SetCurSel(ndx);
			m_observer = nullptr;
			m_curQueryingSubMachine = nullptr;
			if (m_buffList.size() > 0) {
				m_bQuerySuccess = FALSE;
				QueryNextSubmachine();
			} else {
				// 查询完成
				Reset();
				SetTimer(TIMER_ID_AUTO_QUIT, 1000, nullptr);
			}
		} else {
			DWORD dwTimeElapsed = GetTickCount() - m_dwQueryStartTime;
			dwTimeElapsed /= 1000;
			if (dwTimeElapsed >= MAX_QUERY_TIME) {
				if (m_nRetryTimes >= MAX_RETRY_TIMES) {
					// 失败， 停止
					int ndx = m_list.InsertString(-1, m_strQueryFailed);
					m_list.SetCurSel(ndx);
					m_curQueryingSubMachine->set_online(false);
					auto t = time(nullptr);
					m_curQueryingSubMachine->SetAdemcoEvent(ES_UNKNOWN, EVENT_OFFLINE,
															m_curQueryingSubMachine->get_submachine_zone(),
															INDEX_SUB_MACHINE, t, t);
					//Reset();
					// 失败后不停止
					if (m_buffList.size() > 0) {
						m_bQuerySuccess = FALSE;
						QueryNextSubmachine();
					} else {
						// 查询完成
						Reset();
						SetTimer(TIMER_ID_AUTO_QUIT, 1000, nullptr);
					}
				} else if (dwTimeElapsed >= MAX_QUERY_TIME) {
					// 失败， 重试
					m_nRetryTimes++;
					m_dwQueryStartTime = GetTickCount();
					CString l, re; re = TR(IDS_STRING_RETRY);
					l.Format(L"%s, %s %d", m_strQueryFailed, re, m_nRetryTimes);
					int ndx = m_list.InsertString(-1, l);
					m_list.SetCurSel(ndx);
//#ifndef ENABLE_SEQ_CONFIRM
					auto manager = alarm_machine_manager::get_instance();
					manager->RemoteControlAlarmMachine(m_curQueryingSubMachine,
													   EVENT_QUERY_SUB_MACHINE,
													   INDEX_SUB_MACHINE,
													   m_curQueryingSubMachine->get_submachine_zone(),
													   nullptr, nullptr, ES_UNKNOWN, this);
//#endif
				}
			}
		}
	} else if (TIMER_ID_AUTO_START == nIDEvent) {
		if (--m_nAutoStartCounter > 1) {
			CString s, o; o = TR(IDS_STRING_IDC_BUTTON_START);
			s.Format(L"%s(%d)", o, m_nAutoStartCounter);
			m_btnOk.SetWindowTextW(s);
		} else {
			KillTimer(TIMER_ID_AUTO_START);
			OnBnClickedOk();
		}
	} else if (TIMER_ID_AUTO_QUIT == nIDEvent) {
		if (--m_nAutoQuitCounter > 1) {
			CString s;
			s.Format(L"%s(%d)", m_strCancel, m_nAutoQuitCounter);
			m_btnCancel.SetWindowTextW(s);
		} else {
			KillTimer(TIMER_ID_AUTO_QUIT);
			OnBnClickedCancel();
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CAutoQueryDisconnectSubmachineDlg::Reset()
{
	KillTimer(TIMER_ID_AUTO_START);
	KillTimer(TIMER_ID_TIME);
	KillTimer(TIMER_ID_CHECK); 
	KillTimer(TIMER_ID_AUTO_QUIT);
	m_observer = nullptr;
	if (m_curQueryingSubMachine) {
		//m_curQueryingSubMachine->UnRegisterObserver(this);
		m_curQueryingSubMachine = nullptr;
	}
	int cnt = m_subMachineList.size();
	CString progress;
	progress.Format(L"0/%d", cnt);
	m_staticProgress.SetWindowTextW(progress);
	m_staticTime.SetWindowTextW(L"00:00");
	m_progress.SetRange32(0, cnt);
	m_progress.SetPos(0);
	CString s; s = TR(IDS_STRING_IDC_BUTTON_START);
	m_btnOk.SetWindowTextW(s);
	m_nAutoStartCounter = 11;
	m_nAutoQuitCounter = 11;
	m_dwStartTime = 0;
	m_dwQueryStartTime = 0;
	m_nRetryTimes = 0;
	m_bQuerying = FALSE;
	m_buffList.clear();
	std::copy(m_subMachineList.begin(), m_subMachineList.end(), 
			  std::back_inserter(m_buffList));
	m_bQuerySuccess = FALSE;
}


void CAutoQueryDisconnectSubmachineDlg::OnBnClickedOk()
{
	if (m_bQuerying) {
		Reset();
	} else {
		Reset();
		CString s; s = TR(IDS_STRING_IDC_BUTTON_STOP);
		m_btnOk.SetWindowTextW(s);
		m_dwStartTime = GetTickCount();
		SetTimer(TIMER_ID_TIME, 1000, nullptr);
		m_bQuerying = TRUE;
		QueryNextSubmachine();
		SetTimer(TIMER_ID_CHECK, 100, nullptr);
	}
}


void CAutoQueryDisconnectSubmachineDlg::QueryNextSubmachine()
{
	//if (m_curQueryingSubMachine) {
	//	m_curQueryingSubMachine->UnRegisterObserver(this);
	//}
	m_observer = nullptr;
	m_curQueryingSubMachine = m_buffList.front();
	m_buffList.pop_front();
	CString l;
	auto mgr = alarm_machine_manager::get_instance();
	alarm_machine_ptr machine = mgr->GetMachine(m_curQueryingSubMachine->get_ademco_id());
	l.Format(m_strFmQuery,
			 machine->get_ademco_id(), machine->get_machine_name(),
			 m_curQueryingSubMachine->get_submachine_zone(),
			 m_curQueryingSubMachine->get_machine_name());
	int ndx = m_list.InsertString(-1, l);
	m_list.SetCurSel(ndx);
	CString progress;
	progress.Format(L"%d/%d", m_subMachineList.size() - m_buffList.size(),
					m_subMachineList.size());
	m_staticProgress.SetWindowTextW(progress);
	int pos = m_progress.GetPos();
	m_progress.SetPos(++pos);

	m_observer = std::make_shared<ObserverType>(this);
	m_curQueryingSubMachine->register_observer(m_observer);
	//m_curQueryingSubMachine->RegisterObserver(this, OnAdemcoEvent);
	auto manager = alarm_machine_manager::get_instance();
	m_dwQueryStartTime = GetTickCount();
	m_nRetryTimes = 0;
	manager->RemoteControlAlarmMachine(m_curQueryingSubMachine,
									   EVENT_QUERY_SUB_MACHINE,
									   INDEX_SUB_MACHINE,
									   m_curQueryingSubMachine->get_submachine_zone(),
									   nullptr, nullptr, ES_UNKNOWN, this);
}

void CAutoQueryDisconnectSubmachineDlg::OnAdemcoEventResult(const ademco::AdemcoEventPtr& ademcoEvent)
{
	switch (ademcoEvent->_event) {
		case ademco::EVENT_ARM:
			m_bQuerySuccess = TRUE;
			break;
		case ademco::EVENT_DISARM:
			m_bQuerySuccess = TRUE;
			break;
		default:
			break;
	}
}


void CAutoQueryDisconnectSubmachineDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	Reset();
}


void CAutoQueryDisconnectSubmachineDlg::OnBnClickedCancel()
{
	CDialogEx::OnCancel();
}
