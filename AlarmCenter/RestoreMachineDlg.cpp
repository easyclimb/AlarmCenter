// RestoreMachineDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "RestoreMachineDlg.h"
#include "afxdialogex.h"

#include "AlarmMachine.h"
#include "ZoneInfo.h"
#include "AlarmMachineManager.h"


using namespace core;


namespace detail {
	static const int TIMER_ID_TIME = 1;
	static const int TIMER_ID_WORKER = 2;
	static const int MAX_RETRY_TIMES = 2;
#ifdef _DEBUG
	static const int MAX_QUERY_TIME = 20;
#else
	static const int MAX_QUERY_TIME = 20;
#endif
	// CQueryAllSubmachineDlg dialog
	//IMPLEMENT_ADEMCO_EVENT_CALL_BACK(CRestoreMachineDlg, OnAdemcoEvent)
};
using namespace detail;

IMPLEMENT_DYNAMIC(CRestoreMachineDlg, CDialogEx)

CRestoreMachineDlg::CRestoreMachineDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CRestoreMachineDlg::IDD, pParent)
	, m_machine(nullptr)
	, m_bRestoring(FALSE)
	, m_dwStartTime(0)
	, m_dwRestoreStartTime(0)
	, m_nRetryTimes(0)
	, m_nZoneCnt(0)
	, m_strFmRestore(L"")
	, m_strFmRestoreZone(L"")
	, m_strFmRestoreSubmachine(L"")
	, m_strFmRestoreSuccess(L"")
	, m_strRestoreFailed(L"")
	, m_bRestoreSuccess(FALSE)
	, m_curRestoringZoneInfo(nullptr)
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

namespace detail {
	zone_info_list g_zoneInfoList;
};


BOOL CRestoreMachineDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CenterWindow(GetParent());
	ASSERT(m_machine && !m_machine->get_is_submachine());

	Reset();
	CString restore, zone, sensor, submachine, done;
	restore = TR(IDS_STRING_RECONNECT);
	zone = TR(IDS_STRING_ZONE);
	sensor = TR(IDS_STRING_SENSOR);
	submachine = TR(IDS_STRING_SUBMACHINE);
	done = TR(IDS_STRING_DONE);

	m_strFmRestore = restore + zone + L"%03d(%s)";
	m_strFmRestoreZone = restore + zone + L"%03d(%s) " + sensor;
	m_strFmRestoreSubmachine = restore + zone + L"%03d(%s) " + submachine;
	m_strFmRestoreSuccess = restore + done;
	m_strRestoreFailed = TR(IDS_STRING_RESTORE_FAILD);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CRestoreMachineDlg::Reset()
{
	m_bRestoring = FALSE;
	KillTimer(TIMER_ID_TIME);
	KillTimer(TIMER_ID_WORKER);
	m_observer = nullptr;
	m_dwStartTime = 0;
	m_dwRestoreStartTime = 0;
	m_nRetryTimes = 0;
	
	CString txt;
	txt = TR(IDS_STRING_IDC_BUTTON_START);
	m_btnOk.SetWindowTextW(txt);

	detail::g_zoneInfoList.clear();
	zone_info_list list;
	m_machine->GetAllZoneInfo(list);
	for (auto zoneInfo : list) {
		int zoneValue = zoneInfo->get_zone_value();
		if (WIRE_ZONE_RANGE_BEG <= zoneValue && zoneValue <= WIRE_ZONE_RANGE_END)
			continue;
		detail::g_zoneInfoList.push_back(zoneInfo);
	}
	m_bRestoreSuccess = FALSE;

	m_nZoneCnt = detail::g_zoneInfoList.size();
	CString progress;
	progress.Format(L"0/%d", m_nZoneCnt);
	m_staticProgress.SetWindowTextW(progress);
	m_staticTime.SetWindowTextW(L"00:00");
	m_progress.SetRange32(0, m_nZoneCnt);
	m_progress.SetPos(0);
}


void CRestoreMachineDlg::OnAdemcoEventResult(const ademco::AdemcoEventPtr& ademcoEvent)
{
	switch (ademcoEvent->_event) {
		/*case ademco::EVENT_ARM:
			m_bRestoreSuccess = TRUE;
			break;
		case ademco::EVENT_DISARM:
			m_bRestoreSuccess = TRUE;
			break;*/
		case EVENT_RETRIEVE_ZONE_OR_SUB_MACHINE:
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
		txt = TR(IDS_STRING_IDC_BUTTON_STOP);
		m_btnOk.SetWindowTextW(txt);
		m_dwStartTime = GetTickCount();
		m_observer = std::make_shared<ObserverType>(this);
		m_machine->register_observer(m_observer);
		RestoreNextZone();
		SetTimer(TIMER_ID_TIME, 1000, nullptr);
		SetTimer(TIMER_ID_WORKER, 100, nullptr);
	}
}

void CRestoreMachineDlg::RestoreNextZone()
{
	//if (m_curQueryingSubMachine) {
	//	m_curQueryingSubMachine->UnRegisterObserver(this);
	//}
	m_curRestoringZoneInfo = detail::g_zoneInfoList.front();
	detail::g_zoneInfoList.pop_front();
	CString l;
	l.Format(m_strFmRestore, m_curRestoringZoneInfo->get_zone_value(),
			 m_curRestoringZoneInfo->get_alias());
	int ndx = m_list.InsertString(-1, l);
	m_list.SetCurSel(ndx);
	CString progress;
	progress.Format(L"%d/%d", m_nZoneCnt - detail::g_zoneInfoList.size(),
					m_nZoneCnt);
	m_staticProgress.SetWindowTextW(progress);
	int pos = m_progress.GetPos();
	m_progress.SetPos(++pos);

	//m_curQueryingSubMachine->RegisterObserver(this, OnAdemcoEvent);
	auto manager = alarm_machine_manager::get_instance();
	m_dwRestoreStartTime = GetTickCount();
	m_nRetryTimes = 0;
	bool bSubMachine = (m_curRestoringZoneInfo->GetSubMachineInfo() != nullptr);
	char status_or_property = m_curRestoringZoneInfo->get_status_or_property() & 0xFF;
	WORD addr = m_curRestoringZoneInfo->get_physical_addr() & 0xFFFF;
	ademco::char_array_ptr xdata = std::make_shared<ademco::char_array>();
	xdata->push_back(status_or_property);
	xdata->push_back((char)HIBYTE(addr));
	xdata->push_back((char)LOBYTE(addr));

	/*BOOL ok = */manager->RemoteControlAlarmMachine(m_machine,
												 EVENT_WRITE_TO_MACHINE,
												 bSubMachine ? INDEX_SUB_MACHINE : INDEX_ZONE,
												 m_curRestoringZoneInfo->get_zone_value(),
												 xdata, nullptr, ES_UNKNOWN, this);
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
			//CString l; auto res = CAppResource::get_instance();
			//bool arm = m_curRestoringZoneInfo->get_armed();
			//l.Format(m_strFmRestoreSuccess, m_curRestoringZoneInfo->get_zone_value(),
			//		 m_curRestoringZoneInfo->get_alias(),
			//		 arm ? res->AdemcoEventToString(EVENT_ARM) : res->AdemcoEventToString(EVENT_DISARM));
			int ndx = m_list.InsertString(-1, m_strFmRestoreSuccess);
			m_list.SetCurSel(ndx);
			//m_curQueryingSubMachine->UnRegisterObserver(this);
			m_curRestoringZoneInfo = nullptr;
			if (detail::g_zoneInfoList.size() > 0) {
				m_bRestoreSuccess = FALSE;
				RestoreNextZone();
			} else {
				// 恢复完成
				Reset();
			}
		} else {
			DWORD dwTimeElapsed = GetTickCount() - m_dwRestoreStartTime;
			dwTimeElapsed /= 1000;
			if (dwTimeElapsed >= MAX_QUERY_TIME) {
				if (m_nRetryTimes >= MAX_RETRY_TIMES) {
					// 失败， 停止
					int ndx = m_list.InsertString(-1, m_strRestoreFailed);
					m_list.SetCurSel(ndx);
					//m_curRestoringZoneInfo->set_online(false);
					//m_curQueryingSubMachine->SetAdemcoEvent(EVENT_OFFLINE,
					//										m_curQueryingSubMachine->get_submachine_zone(),
					//										INDEX_SUB_MACHINE,
					//										time(nullptr), nullptr, 0);
					//Reset();
					// 失败后不停止
					if (detail::g_zoneInfoList.size() > 0) {
						m_bRestoreSuccess = FALSE;
						RestoreNextZone();
					} else {
						// 恢复完成
						Reset();
					}
				} else if (dwTimeElapsed >= MAX_QUERY_TIME) {
					// 失败， 重试
					m_nRetryTimes++;
					m_dwRestoreStartTime = GetTickCount();
					CString l, re; re = TR(IDS_STRING_RETRY);
					l.Format(L"%s, %s %d", m_strRestoreFailed, re, m_nRetryTimes);
					int ndx = m_list.InsertString(-1, l);
					m_list.SetCurSel(ndx);
//#ifndef ENABLE_SEQ_CONFIRM
					bool bSubMachine = (m_curRestoringZoneInfo->GetSubMachineInfo() != nullptr);
					char status_or_property = m_curRestoringZoneInfo->get_status_or_property() & 0xFF;
					WORD addr = m_curRestoringZoneInfo->get_physical_addr() & 0xFFFF;
					ademco::char_array_ptr xdata = std::make_shared<ademco::char_array>();
					xdata->push_back(status_or_property);
					xdata->push_back((char)HIBYTE(addr));
					xdata->push_back((char)LOBYTE(addr));
					auto manager = alarm_machine_manager::get_instance();
					manager->RemoteControlAlarmMachine(m_machine,
													   EVENT_WRITE_TO_MACHINE,
													   bSubMachine ? INDEX_SUB_MACHINE : INDEX_ZONE,
													   m_curRestoringZoneInfo->get_zone_value(),
													   xdata, nullptr, ES_UNKNOWN, this);
//#endif
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
