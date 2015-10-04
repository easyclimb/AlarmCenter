// QueryAllSubmachineDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "QueryAllSubmachineDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "ZoneInfo.h"
#include "AlarmMachineManager.h"
#include "AppResource.h"
#include "HistoryRecord.h"

using namespace core;

static const int TIMER_ID_TIME = 1;
static const int TIMER_ID_WORKER = 2;
static const int MAX_RETRY_TIMES = 2;

//#ifndef _DEBUG
//static const int MAX_QUERY_TIME = 2;
//#else
static const int MAX_QUERY_TIME = 20;
//#endif
// CQueryAllSubmachineDlg dialog
IMPLEMENT_ADEMCO_EVENT_CALL_BACK(CQueryAllSubmachineDlg, OnAdemcoEvent)
IMPLEMENT_DYNAMIC(CQueryAllSubmachineDlg, CDialogEx)

CQueryAllSubmachineDlg::CQueryAllSubmachineDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CQueryAllSubmachineDlg::IDD, pParent)
	, m_machine(nullptr)
	, m_bQuerying(FALSE)
	, m_dwStartTime(0)
	, m_dwQueryStartTime(0)
	, m_nRetryTimes(0)
	, m_strFmQuery(L"")
	, m_strFmQeurySuccess(L"")
	, m_strQueryFailed(L"")
	, m_bQuerySuccess(FALSE)
	, m_curQueryingSubMachine(nullptr)
{

}

CQueryAllSubmachineDlg::~CQueryAllSubmachineDlg()
{
}

void CQueryAllSubmachineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDC_STATIC_PROGRESS, m_staticProgress);
	DDX_Control(pDX, IDC_STATIC_TIME, m_staticTime);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
}


BEGIN_MESSAGE_MAP(CQueryAllSubmachineDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CQueryAllSubmachineDlg::OnBnClickedOk)
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CQueryAllSubmachineDlg message handlers
std::list<CAlarmMachine*> g_subMachineList;

void CQueryAllSubmachineDlg::Reset()
{
	m_bQuerying = FALSE;
	KillTimer(TIMER_ID_TIME);
	KillTimer(TIMER_ID_WORKER);
	if (m_curQueryingSubMachine) {
		m_curQueryingSubMachine->UnRegisterObserver(this);
	}
	m_dwStartTime = 0;
	m_dwQueryStartTime = 0;
	m_nRetryTimes = 0;
	int cnt = m_machine->get_submachine_count();
	CString progress;
	progress.Format(L"0/%d", cnt);
	m_staticProgress.SetWindowTextW(progress);
	m_staticTime.SetWindowTextW(L"00:00");
	m_progress.SetRange32(0, cnt);
	m_progress.SetPos(0);
	CString txt;
	txt.LoadStringW(IDS_STRING_START);
	m_btnOk.SetWindowTextW(txt);

	g_subMachineList.clear();
	CZoneInfoList list;
	m_machine->GetAllZoneInfo(list);
	for (auto zoneInfo : list) {
		CAlarmMachine* subMachine = zoneInfo->GetSubMachineInfo();
		if (subMachine) {
			g_subMachineList.push_back(subMachine);
		}
	}
	m_bQuerySuccess = FALSE;
}


BOOL CQueryAllSubmachineDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CenterWindow(GetParent());
	ASSERT(m_machine && !m_machine->get_is_submachine());

	Reset();
	CString query, submachine, done;
	query.LoadStringW(IDS_STRING_QUERY);
	submachine.LoadStringW(IDS_STRING_SUBMACHINE);
	done.LoadStringW(IDS_STRING_DONE);

	m_strFmQuery = query + submachine + L"%03d(%s)";
	m_strFmQeurySuccess = query + done + L"%03d(%s) %s";
	m_strQueryFailed.LoadStringW(IDS_STRING_QUERY_FAILED);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CQueryAllSubmachineDlg::OnBnClickedOk()
{
	if (m_bQuerying) {
		Reset();
	} else {
		Reset();
		m_bQuerying = TRUE;
		CString txt;
		txt.LoadStringW(IDS_STRING_STOP);
		m_btnOk.SetWindowTextW(txt);
		m_dwStartTime = GetTickCount();
		QueryNextSubMachine();
		SetTimer(TIMER_ID_TIME, 1000, nullptr);
		SetTimer(TIMER_ID_WORKER, 100, nullptr);
	}
}


void CQueryAllSubmachineDlg::QueryNextSubMachine()
{
	if (m_curQueryingSubMachine) {
		m_curQueryingSubMachine->UnRegisterObserver(this);
	}
	m_curQueryingSubMachine = g_subMachineList.front();
	g_subMachineList.pop_front();
	CString l;
	l.Format(m_strFmQuery, m_curQueryingSubMachine->get_submachine_zone(),
			 m_curQueryingSubMachine->get_alias());
	int ndx = m_list.InsertString(-1, l);
	m_list.SetCurSel(ndx);
	CString progress;
	progress.Format(L"%d/%d", m_machine->get_submachine_count() - g_subMachineList.size(), 
					m_machine->get_submachine_count());
	m_staticProgress.SetWindowTextW(progress);
	int pos = m_progress.GetPos();
	m_progress.SetPos(++pos);

	m_curQueryingSubMachine->RegisterObserver(this, OnAdemcoEvent);
	CAlarmMachineManager* manager = CAlarmMachineManager::GetInstance();
	m_dwQueryStartTime = GetTickCount();
	m_nRetryTimes = 0;
	manager->RemoteControlAlarmMachine(m_curQueryingSubMachine, 
									   EVENT_QUERY_SUB_MACHINE,
									   INDEX_SUB_MACHINE,
									   m_curQueryingSubMachine->get_submachine_zone(),
									   nullptr, 0, this);
}


void CQueryAllSubmachineDlg::OnAdemcoEventResult(const ademco::AdemcoEvent* ademcoEvent)
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


void CQueryAllSubmachineDlg::OnTimer(UINT_PTR nIDEvent)
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
	} else if (TIMER_ID_WORKER == nIDEvent) {
		if (m_bQuerySuccess) {
			CString i; i.LoadStringW(IDS_STRING_QUERY_SUCCESS);
			CHistoryRecord::GetInstance()->InsertRecord(m_machine->get_ademco_id(),
														m_curQueryingSubMachine->get_submachine_zone(),
														i, time(nullptr), RECORD_LEVEL_USERCONTROL);
			CString l; CAppResource* res = CAppResource::GetInstance();
			bool arm = m_curQueryingSubMachine->get_armed();
			l.Format(m_strFmQeurySuccess, m_curQueryingSubMachine->get_submachine_zone(),
					 m_curQueryingSubMachine->get_alias(), 
					 arm ? res->AdemcoEventToString(EVENT_ARM) : res->AdemcoEventToString(EVENT_DISARM));
			int ndx = m_list.InsertString(-1, l);
			m_list.SetCurSel(ndx);
			m_curQueryingSubMachine->UnRegisterObserver(this);
			m_curQueryingSubMachine = nullptr;
			if (g_subMachineList.size() > 0) {
				m_bQuerySuccess = FALSE;
				QueryNextSubMachine();
			} else {
				// 查询完成
				Reset();
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
					m_curQueryingSubMachine->SetAdemcoEvent(ES_UNKNOWN, EVENT_OFFLINE,
															m_curQueryingSubMachine->get_submachine_zone(),
															INDEX_SUB_MACHINE,
															time(nullptr), time(nullptr), nullptr, 0);
					//Reset();
					// 失败后不停止
					CString i; i.LoadStringW(IDS_STRING_QUERY_FAILED);
					CHistoryRecord::GetInstance()->InsertRecord(m_curQueryingSubMachine->get_ademco_id(),
																m_curQueryingSubMachine->get_submachine_zone(),
																i, time(nullptr), RECORD_LEVEL_USERCONTROL);
					if (g_subMachineList.size() > 0) {
						m_bQuerySuccess = FALSE;
						QueryNextSubMachine();
					} else {
						// 查询完成
						Reset();
					}
				} else if (dwTimeElapsed >= MAX_QUERY_TIME) {
					// 失败， 重试
					m_nRetryTimes++;
					m_dwQueryStartTime = GetTickCount();
					CString l, re; re.LoadStringW(IDS_STRING_RETRY);
					l.Format(L"%s, %s %d", m_strQueryFailed, re, m_nRetryTimes);
					int ndx = m_list.InsertString(-1, l);
					m_list.SetCurSel(ndx);
//#ifndef ENABLE_SEQ_CONFIRM
					CAlarmMachineManager* manager = CAlarmMachineManager::GetInstance();
					manager->RemoteControlAlarmMachine(m_curQueryingSubMachine,
													   EVENT_QUERY_SUB_MACHINE,
													   INDEX_SUB_MACHINE,
													   m_curQueryingSubMachine->get_submachine_zone(),
													   nullptr, 0, this);
//#endif
				}
			}
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CQueryAllSubmachineDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	Reset();
}
