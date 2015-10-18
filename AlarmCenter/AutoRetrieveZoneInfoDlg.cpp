// AutoRetrieveZoneInfoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AutoRetrieveZoneInfoDlg.h"
#include "afxdialogex.h"
#include "RetrieveProgressDlg.h"
#include "AlarmMachine.h"
#include "ZoneInfo.h"


using namespace core;


// CAutoRetrieveZoneInfoDlg 对话框

IMPLEMENT_DYNAMIC(CAutoRetrieveZoneInfoDlg, CDialogEx)

CAutoRetrieveZoneInfoDlg::CAutoRetrieveZoneInfoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CAutoRetrieveZoneInfoDlg::IDD, pParent)
	, m_machine(nullptr)
	, m_dwStartTime(0)
{

}

CAutoRetrieveZoneInfoDlg::~CAutoRetrieveZoneInfoDlg()
{
}

void CAutoRetrieveZoneInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_START, m_btnStart);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
	DDX_Control(pDX, IDC_STATIC_PROGRESS, m_staticProgress);
	DDX_Control(pDX, IDC_STATIC_TIME, m_staticTime);
	DDX_Control(pDX, IDC_LIST1, m_listctrl);
}


BEGIN_MESSAGE_MAP(CAutoRetrieveZoneInfoDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAutoRetrieveZoneInfoDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_START, &CAutoRetrieveZoneInfoDlg::OnBnClickedButtonStart)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CAutoRetrieveZoneInfoDlg 消息处理程序


void CAutoRetrieveZoneInfoDlg::OnBnClickedOk()
{
	return;
}


BOOL CAutoRetrieveZoneInfoDlg::OnInitDialog() 
{
	CDialogEx::OnInitDialog();

	m_progress.SetRange32(0, 1000);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}


void CAutoRetrieveZoneInfoDlg::OnBnClickedButtonStart() 
{
	m_dwStartTime = GetTickCount();
	SetTimer(1, 1000, nullptr);

	CString msg = L"", str = L"", fmok, fmfail, progress;
	fmok.LoadStringW(IDS_STRING_FM_RETRIEVE_OK);
	fmfail.LoadStringW(IDS_STRING_FM_RETRIEVE_FAILED);
	for (int i = 1; i < MAX_MACHINE_ZONE; i++) {
		msg.Empty();
		if (RetrieveZoneInfo(i, msg)) {
			m_progress.SetPos(i);
			progress.Format(L"%d/1000", i);
			m_staticProgress.SetWindowTextW(progress);
			str.Format(fmok, i);
			int ndx = m_listctrl.InsertString(-1, str + L": " + msg);
			m_listctrl.SetCurSel(ndx);
		} else {
			str.Format(fmfail, i, msg);
			int ndx = m_listctrl.InsertString(-1, str);
			m_listctrl.SetCurSel(ndx);
			break;
		}
	}
	KillTimer(1);
	m_progress.SetPos(0);
	m_staticProgress.SetWindowTextW(L"0/1000");
	m_staticTime.SetWindowTextW(L"00:00");
}


// return true for continue, false for stop
bool CAutoRetrieveZoneInfoDlg::RetrieveZoneInfo(int zoneValue, CString& msg)
{
	do {
		CZoneInfo* zoneInfo = m_machine->GetZone(zoneValue);
		if (zoneInfo) {
			CString fm; fm.LoadStringW(IDS_STRING_FM_ZONE_ALREADY_EXSISTS);
			msg.Format(fm, zoneInfo->get_alias());
			return true;
		}
		bool bNeedCreateSubMachine = false;
		bool bWireZone = WIRE_ZONE_RANGE_BEG <= zoneValue && zoneValue <= WIRE_ZONE_RANGE_END;
		if (!m_machine->get_is_submachine()) {
			if (zoneValue <= 0 || zoneValue >= MAX_MACHINE_ZONE) {
				msg.LoadStringW(IDS_STRING_E_ZONE_RANGE_FAILE);
				MessageBox(msg);
				break;
			}
			if (!bWireZone && MT_NETMOD == m_machine->get_machine_type()) {
				CRetrieveProgressDlg retrieveProgressDlg;
				retrieveProgressDlg.m_machine = m_machine;
				retrieveProgressDlg.m_zone = zoneValue;
				if (retrieveProgressDlg.DoModal() != IDOK) {
					msg.LoadStringW(IDS_STRING_USER_STOP_RESTRIEVE);
					break;
				}
				//int gg = retrieveProgressDlg.m_gg;
				CString alias, fmZone, fmSubMachine;
				fmZone.LoadStringW(IDS_STRING_ZONE);
				fmSubMachine.LoadStringW(IDS_STRING_SUBMACHINE);
				if (0xCC == retrieveProgressDlg.m_gg) { // not registered
					msg.LoadStringW(IDS_STRING_ZONE_NO_DUIMA);
					return true;
				} else if (0xEE == retrieveProgressDlg.m_gg) { // submachine
					zoneInfo = new CZoneInfo();
					zoneInfo->set_ademco_id(m_machine->get_ademco_id());
					zoneInfo->set_zone_value(zoneValue);
					zoneInfo->set_type(ZT_SUB_MACHINE);
					zoneInfo->set_status_or_property(retrieveProgressDlg.m_status);
					zoneInfo->set_physical_addr(retrieveProgressDlg.m_addr);
					alias.Format(L"%s%03d", fmSubMachine, zoneValue);
					zoneInfo->set_alias(alias);
					bNeedCreateSubMachine = true;
				} else if (0x00 == retrieveProgressDlg.m_gg) { // direct
					zoneInfo = new CZoneInfo();
					zoneInfo->set_ademco_id(m_machine->get_ademco_id());
					zoneInfo->set_zone_value(zoneValue);
					zoneInfo->set_type(ZT_ZONE);
					zoneInfo->set_status_or_property(retrieveProgressDlg.m_status);
					zoneInfo->set_physical_addr(retrieveProgressDlg.m_addr);
					alias.Format(L"%s%03d", fmZone, zoneValue);
					zoneInfo->set_alias(alias);
				} else {
					ASSERT(0);
					break;
				}
			} else {
				zoneInfo = new CZoneInfo();
				zoneInfo->set_ademco_id(m_machine->get_ademco_id());
				zoneInfo->set_zone_value(zoneValue);
				zoneInfo->set_type(ZT_ZONE);
			}
		} 

		if (m_machine->execute_add_zone(zoneInfo)) {
			if (bNeedCreateSubMachine) {
				CString null;
				null.LoadStringW(IDS_STRING_NULL);
				CAlarmMachine* subMachine = new CAlarmMachine();
				subMachine->set_is_submachine(true);
				subMachine->set_ademco_id(m_machine->get_ademco_id());
				subMachine->set_submachine_zone(zoneValue);
				subMachine->set_alias(zoneInfo->get_alias());
				subMachine->set_address(null);
				subMachine->set_contact(null);
				subMachine->set_phone(null);
				subMachine->set_phone_bk(null);
				subMachine->set_machine_type(m_machine->get_machine_type());
				subMachine->set_online(true);
				if (!zoneInfo->execute_set_sub_machine(subMachine)) {
					ASSERT(0); JLOG(L"execute_set_sub_machine failed.\n"); break;
				}
				//m_machine->inc_submachine_count();
				char status = zoneInfo->get_status_or_property() & 0xFF;
				ADEMCO_EVENT ademco_event = CZoneInfo::char_to_status(status);
				m_machine->SetAdemcoEvent(ES_UNKNOWN, ademco_event, zoneValue, 0xEE, time(nullptr), time(nullptr), nullptr, 0);
			}
		} else {
			ASSERT(0); JLOG(L"m_machine->execute_add_zone(zoneInfo) failed.\n"); delete zoneInfo; break;
		}

		if (msg.IsEmpty()) {
			//CString szone, ssubmachine; szone.LoadStringW(IDS_STRING_ZONE);
			//ssubmachine.LoadStringW(IDS_STRING_SUBMACHINE);
			msg = zoneInfo->get_alias();
		}
		return true;
	} while (0);
	if (msg.IsEmpty())
		msg.LoadStringW(IDS_STRING_QUERY_FAILED);
	return false;
}


void CAutoRetrieveZoneInfoDlg::OnTimer(UINT_PTR nIDEvent)
{
	DWORD cnt = GetTickCount() - m_dwStartTime;
	if (cnt >= 1000) {
		cnt /= 1000;
		int min = cnt / 60;
		int sec = cnt % 60;
		CString t; t.Format(L"%02d:%02d", min, sec);
		m_staticTime.SetWindowTextW(t);
	}

	CDialogEx::OnTimer(nIDEvent);
}
