// RestoreMachineDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "RestoreMachineDlg.h"
#include "afxdialogex.h"

#include "AlarmMachine.h"
#include "ZoneInfo.h"
#include "AlarmMachineManager.h"
#include "AppResource.h"

using namespace core;

static const int TIMER_ID_TIME = 1;
static const int TIMER_ID_WORKER = 2;
static const int MAX_RETRY_TIMES = 2;
#ifdef _DEBUG
static const int MAX_QUERY_TIME = 2;
#else
static const int MAX_QUERY_TIME = 20;
#endif
// CQueryAllSubmachineDlg dialog
IMPLEMENT_ADEMCO_EVENT_CALL_BACK(CRestoreMachineDlg, OnAdemcoEvent)
IMPLEMENT_DYNAMIC(CRestoreMachineDlg, CDialogEx)

CRestoreMachineDlg::CRestoreMachineDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CRestoreMachineDlg::IDD, pParent)
	, m_machine(NULL)
	, m_bRestoring(FALSE)
	, m_dwStartTime(0)
	, m_dwRestoreStartTime(0)
	, m_nRetryTimes(0)
	, m_strFmRestore(L"")
	, m_strFmRestoreZone(L"")
	, m_strFmRestoreSubmachine(L"")
	, m_strFmRestoreSuccess(L"")
	, m_strRestoreFailed(L"")
	, m_bRestoreSuccess(FALSE)
	, m_curRestoringZoneInfo(NULL)
{

}

CRestoreMachineDlg::~CRestoreMachineDlg()
{
}

void CRestoreMachineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
	DDX_Control(pDX, IDC_STATIC_PROGRESS, m_staticProgress);
	DDX_Control(pDX, IDC_STATIC_TIME, m_staticTime);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDC_LIST1, m_list);
}


BEGIN_MESSAGE_MAP(CRestoreMachineDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CRestoreMachineDlg::OnBnClickedOk)
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CRestoreMachineDlg message handlers
CZoneInfoList g_zoneInfoList;

BOOL CRestoreMachineDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CenterWindow(GetParent());
	ASSERT(m_machine && !m_machine->get_is_submachine());

	Reset();
	CString restore, zone, sensor, submachine, done;
	restore.LoadStringW(IDS_STRING_RESTORE);
	zone.LoadStringW(IDS_STRING_ZONE);
	sensor.LoadStringW(IDS_STRING_SENSOR);
	submachine.LoadStringW(IDS_STRING_SUBMACHINE);
	done.LoadStringW(IDS_STRING_DONE);

	m_strFmRestore = restore + zone + L"%03d(%s)";
	m_strFmRestoreZone = restore + zone + L"%03d(%s) " + sensor;
	m_strFmRestoreSubmachine = restore + zone + L"%03d(%s) " + submachine;
	m_strFmRestoreSuccess = restore + done;
	m_strRestoreFailed.LoadStringW(IDS_STRING_RESTORE_FAILD);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CRestoreMachineDlg::Reset()
{
	m_bRestoring = FALSE;
	KillTimer(TIMER_ID_TIME);
	KillTimer(TIMER_ID_WORKER);
	if (m_machine) {
		m_machine->UnRegisterObserver(this);
	}
	m_dwStartTime = 0;
	m_dwRestoreStartTime = 0;
	m_nRetryTimes = 0;
	int cnt = m_machine->get_zone_count();
	CString progress;
	progress.Format(L"0/%d", cnt);
	m_staticProgress.SetWindowTextW(progress);
	m_staticTime.SetWindowTextW(L"00:00");
	m_progress.SetRange32(0, cnt);
	m_progress.SetPos(0);
	CString txt;
	txt.LoadStringW(IDS_STRING_START);
	m_btnOk.SetWindowTextW(txt);

	g_zoneInfoList.clear();
	//CZoneInfoList list;
	m_machine->GetAllZoneInfo(g_zoneInfoList);
	/*CZoneInfoListIter iter = list.begin();
	while (iter != list.end()) {
		CZoneInfo* zoneInfo = *iter++;
		CAlarmMachine* subMachine = zoneInfo->GetSubMachineInfo();
		if (subMachine) {
			g_subMachineList.push_back(subMachine);
		}
	}*/
	m_bRestoreSuccess = FALSE;
}


void CRestoreMachineDlg::OnAdemcoEventResult(const ademco::AdemcoEvent* ademcoEvent)
{
	switch (ademcoEvent->_event) {
		/*case ademco::EVENT_ARM:
			m_bRestoreSuccess = TRUE;
			break;
		case ademco::EVENT_DISARM:
			m_bRestoreSuccess = TRUE;
			break;*/
		case EVENT_RETRIEVE_SUB_MACHINE:
			if (m_curRestoringZoneInfo 
				&& m_curRestoringZoneInfo->get_zone_value() == ademcoEvent->_zone) {
				m_bRestoreSuccess = TRUE;
			}
			break;
		default:
			break;
	}
}



void CRestoreMachineDlg::OnBnClickedOk()
{
	if (m_bRestoring) {
		Reset();
	} else {
		Reset();
		m_bRestoring = TRUE;
		CString txt;
		txt.LoadStringW(IDS_STRING_STOP);
		m_btnOk.SetWindowTextW(txt);
		m_dwStartTime = GetTickCount();
		m_machine->RegisterObserver(this, OnAdemcoEvent);
		RestoreNextZone();
		SetTimer(TIMER_ID_TIME, 1000, NULL);
		SetTimer(TIMER_ID_WORKER, 100, NULL);
	}
}

void CRestoreMachineDlg::RestoreNextZone()
{
	//if (m_curQueryingSubMachine) {
	//	m_curQueryingSubMachine->UnRegisterObserver(this);
	//}
	m_curRestoringZoneInfo = g_zoneInfoList.front();
	g_zoneInfoList.pop_front();
	CString l;
	l.Format(m_strFmRestore, m_curRestoringZoneInfo->get_zone_value(),
			 m_curRestoringZoneInfo->get_alias());
	int ndx = m_list.InsertString(-1, l);
	m_list.SetCurSel(ndx);
	CString progress;
	progress.Format(L"%d/%d", m_machine->get_zone_count() - g_zoneInfoList.size(),
					m_machine->get_zone_count());
	m_staticProgress.SetWindowTextW(progress);
	int pos = m_progress.GetPos();
	m_progress.SetPos(++pos);

	//m_curQueryingSubMachine->RegisterObserver(this, OnAdemcoEvent);
	CAlarmMachineManager* manager = CAlarmMachineManager::GetInstance();
	m_dwRestoreStartTime = GetTickCount();
	m_nRetryTimes = 0;
	char status_or_property = m_curRestoringZoneInfo->get_status_or_property() & 0xFF;
	WORD addr = m_curRestoringZoneInfo->get_physical_addr() & 0xFFFF;
	char xdata[3] = { status_or_property, HIBYTE(addr), LOBYTE(addr) };
	int xdata_len = 3;

	/*BOOL ok = */manager->RemoteControlAlarmMachine(m_machine,
												 EVENT_WRITE_TO_MACHINE,
												 INDEX_SUB_MACHINE,
												 m_curRestoringZoneInfo->get_zone_value(),
												 xdata, xdata_len, this);
	//m_bRestoreSuccess = ok;
}



void CRestoreMachineDlg::OnTimer(UINT_PTR nIDEvent)
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
		if (m_bRestoreSuccess) {
			//CString l; CAppResource* res = CAppResource::GetInstance();
			//bool arm = m_curRestoringZoneInfo->get_armed();
			//l.Format(m_strFmRestoreSuccess, m_curRestoringZoneInfo->get_zone_value(),
			//		 m_curRestoringZoneInfo->get_alias(),
			//		 arm ? res->AdemcoEventToString(EVENT_ARM) : res->AdemcoEventToString(EVENT_DISARM));
			int ndx = m_list.InsertString(-1, m_strFmRestoreSuccess);
			m_list.SetCurSel(ndx);
			//m_curQueryingSubMachine->UnRegisterObserver(this);
			m_curRestoringZoneInfo = NULL;
			if (g_zoneInfoList.size() > 0) {
				m_bRestoreSuccess = FALSE;
				RestoreNextZone();
			} else {
				// »Ö¸´Íê³É
				Reset();
			}
		} else {
			DWORD dwTimeElapsed = GetTickCount() - m_dwRestoreStartTime;
			dwTimeElapsed /= 1000;
			if (dwTimeElapsed >= MAX_QUERY_TIME) {
				if (m_nRetryTimes >= MAX_RETRY_TIMES) {
					// Ê§°Ü£¬ Í£Ö¹
					int ndx = m_list.InsertString(-1, m_strRestoreFailed);
					m_list.SetCurSel(ndx);
					//m_curRestoringZoneInfo->set_online(false);
					//m_curQueryingSubMachine->SetAdemcoEvent(EVENT_OFFLINE,
					//										m_curQueryingSubMachine->get_submachine_zone(),
					//										INDEX_SUB_MACHINE,
					//										time(NULL), NULL, 0);
					//Reset();
					// Ê§°Üºó²»Í£Ö¹
					if (g_zoneInfoList.size() > 0) {
						m_bRestoreSuccess = FALSE;
						RestoreNextZone();
					} else {
						// »Ö¸´Íê³É
						Reset();
					}
				} else if (dwTimeElapsed >= MAX_QUERY_TIME) {
					// Ê§°Ü£¬ ÖØÊÔ
					m_nRetryTimes++;
					m_dwRestoreStartTime = GetTickCount();
					CString l, re; re.LoadStringW(IDS_STRING_RETRY);
					l.Format(L"%s, %s %d", m_strRestoreFailed, re, m_nRetryTimes);
					int ndx = m_list.InsertString(-1, l);
					m_list.SetCurSel(ndx);
					CAlarmMachineManager* manager = CAlarmMachineManager::GetInstance();
					manager->RemoteControlAlarmMachine(m_machine,
													   EVENT_WRITE_TO_MACHINE,
													   INDEX_SUB_MACHINE,
													   m_curRestoringZoneInfo->get_zone_value(),
													   NULL, 0, this);
				}
			}
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CRestoreMachineDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	Reset();
}
