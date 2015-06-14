// RetrieveProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "RetrieveProgressDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "AlarmMachineManager.h"

using namespace core;
using namespace ademco;
// CRetrieveProgressDlg dialog
IMPLEMENT_ADEMCO_EVENT_CALL_BACK(CRetrieveProgressDlg, OnAdemcoEvent)
IMPLEMENT_DYNAMIC(CRetrieveProgressDlg, CDialogEx)

CRetrieveProgressDlg::CRetrieveProgressDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CRetrieveProgressDlg::IDD, pParent)
	, m_machine(NULL)
	, m_zone(0)
	, m_gg(0)
	, m_status(0)
	, m_addr(0)
	, m_ok(FALSE)
{

}

CRetrieveProgressDlg::~CRetrieveProgressDlg()
{
}

void CRetrieveProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
}


BEGIN_MESSAGE_MAP(CRetrieveProgressDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CRetrieveProgressDlg::OnBnClickedOk)
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CRetrieveProgressDlg message handlers


void CRetrieveProgressDlg::OnBnClickedOk()
{
	return;
}


void CRetrieveProgressDlg::OnCancel()
{
	CDialogEx::OnCancel();
}


void CRetrieveProgressDlg::OnAdemcoEventResult(const ademco::AdemcoEvent* ademcoEvent)
{
	if (ademcoEvent->_event == ademco::EVENT_RETRIEVE_SUB_MACHINE ||
		ademcoEvent->_event == ademco::EVENT_QUERY_SUB_MACHINE) {
		if (ademcoEvent->_zone == m_zone) {
			m_gg = ademcoEvent->_sub_zone;
			ASSERT(ademcoEvent->_xdata && (ademcoEvent->_xdata_len == 3));
			m_status = ademcoEvent->_xdata[0];
			m_addr = MAKEWORD(ademcoEvent->_xdata[2], ademcoEvent->_xdata[1]);
			m_ok = TRUE;
		}
	}
}


void CRetrieveProgressDlg::OnTimer(UINT_PTR nIDEvent)
{
	int pos = m_progress.GetPos();
	pos += 1;
	if (pos == 10) {
		ShowWindow(SW_SHOW);
#ifndef ENABLE_SEQ_CONFIRM
		CAlarmMachineManager::GetInstance()->RemoteControlAlarmMachine(m_machine,
																	   EVENT_RETRIEVE_SUB_MACHINE,
																	   0, m_zone, NULL, 0, NULL);
#endif
		pos = 0;
	}
	m_progress.SetPos(pos);

	if (m_ok) {
		KillTimer(1);
		m_progress.SetPos(10);
		OnOK();
	}

	CDialogEx::OnTimer(nIDEvent);
}


BOOL CRetrieveProgressDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	ShowWindow(SW_HIDE);
	ASSERT(m_machine);
	m_progress.SetRange32(0, 10);
	SetTimer(1, 250, NULL);
	m_machine->RegisterObserver(this, OnAdemcoEvent);
	CAlarmMachineManager::GetInstance()->RemoteControlAlarmMachine(m_machine,
																   EVENT_RETRIEVE_SUB_MACHINE,
																   0, m_zone, NULL, 0, NULL);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CRetrieveProgressDlg::OnDestroy()
{
	ASSERT(m_machine);
	m_machine->UnRegisterObserver(this);
	CDialogEx::OnDestroy();
}
