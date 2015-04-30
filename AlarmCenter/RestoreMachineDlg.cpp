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
	m_strFmRestoreSuccess = restore + done + L"%03d(%s) %s";
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

	g_zoneInfoList.clear();
	CZoneInfoList list;
	m_machine->GetAllZoneInfo(list);
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
		case ademco::EVENT_ARM:
			m_bRestoreSuccess = TRUE;
			break;
		case ademco::EVENT_DISARM:
			m_bRestoreSuccess = TRUE;
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

	char xdata[3] = { 0 };
	int xdata_len = 3;

	// todo
	manager->RemoteControlAlarmMachine(m_machine,
									   EVENT_WRITE_TO_MACHINE,
									   INDEX_SUB_MACHINE,
									   m_curRestoringZoneInfo->get_zone_value(),
									   xdata, xdata_len, this);
}
