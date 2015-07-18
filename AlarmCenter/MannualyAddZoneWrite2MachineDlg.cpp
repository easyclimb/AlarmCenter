// MannualyAddZoneWrite2MachineDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "MannualyAddZoneWrite2MachineDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "ZoneInfo.h"
#include "AlarmMachineManager.h"
#include "ademco_func.h"

using namespace core;
// CMannualyAddZoneWrite2MachineDlg dialog
IMPLEMENT_ADEMCO_EVENT_CALL_BACK(CMannualyAddZoneWrite2MachineDlg, OnAdemcoEvent)
IMPLEMENT_DYNAMIC(CMannualyAddZoneWrite2MachineDlg, CDialogEx)

CMannualyAddZoneWrite2MachineDlg::CMannualyAddZoneWrite2MachineDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMannualyAddZoneWrite2MachineDlg::IDD, pParent)
	, m_machine(NULL)
	, m_bRestoreSuccess(FALSE)
	, m_zone(0)
	, m_gg(0)
	, m_waddr(0)
	, m_zs(0)
	, m_dwStartTime(0)
{

}

CMannualyAddZoneWrite2MachineDlg::~CMannualyAddZoneWrite2MachineDlg()
{
}

void CMannualyAddZoneWrite2MachineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_cmbZone);
	DDX_Control(pDX, IDC_EDIT1, m_addr);
	DDX_Control(pDX, IDC_COMBO2, m_type);
	DDX_Control(pDX, IDC_COMBO3, m_property);
	DDX_Control(pDX, IDC_COMBO4, m_buglar_property);
}


BEGIN_MESSAGE_MAP(CMannualyAddZoneWrite2MachineDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CMannualyAddZoneWrite2MachineDlg::OnBnClickedOk)
	ON_CBN_SELCHANGE(IDC_COMBO2, &CMannualyAddZoneWrite2MachineDlg::OnCbnSelchangeComboType)
	ON_CBN_SELCHANGE(IDC_COMBO3, &CMannualyAddZoneWrite2MachineDlg::OnCbnSelchangeComboProperty)
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CMannualyAddZoneWrite2MachineDlg message handlers

BOOL CMannualyAddZoneWrite2MachineDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	ASSERT(m_machine && !m_machine->get_is_submachine());
	
	CString txt(L"");
	for (int i = WIRE_ZONE_RANGE_END + 1; i < MAX_MACHINE_ZONE; i++) {
		if (NULL == m_machine->GetZone(i)) {
			txt.Format(L"%03d", i);
			int ndx = m_cmbZone.AddString(txt);
			m_cmbZone.SetItemData(ndx, i);
		}
	}

	CString type;
	type.LoadStringW(IDS_STRING_SUBMACHINE);
	int ndx = m_type.AddString(type);
	m_type.SetItemData(ndx, INDEX_SUB_MACHINE);
	type.LoadStringW(IDS_STRING_ZONE);
	ndx = m_type.AddString(type);
	m_type.SetItemData(ndx, INDEX_ZONE);
	m_type.SetCurSel(0);

	CString prop;
	prop.LoadStringW(IDS_STRING_BUGLAR_ZONE);
	ndx = m_property.AddString(prop);
	m_property.SetItemData(ndx, ZSOP_INVALID);// ����

	prop.LoadStringW(IDS_STRING_GLOBAL_ZONE);
	ndx = m_buglar_property.AddString(prop);
	m_property.SetItemData(ndx, ZP_GLOBAL);

	prop.LoadStringW(IDS_STRING_HALF_ZONE);
	ndx = m_buglar_property.AddString(prop);
	m_property.SetItemData(ndx, ZP_HALF);

	prop.LoadStringW(IDS_STRING_EMERGENCY_ZONE);
	ndx = m_buglar_property.AddString(prop);
	m_property.SetItemData(ndx, ZP_EMERGE);

	prop.LoadStringW(IDS_STRING_SHIELD_ZONE);
	ndx = m_buglar_property.AddString(prop);
	m_property.SetItemData(ndx, ZP_SHIELD);

	prop.LoadStringW(IDS_STRING_DOORRING_ZONE);
	ndx = m_buglar_property.AddString(prop);
	m_property.SetItemData(ndx, ZP_DOOR);


	prop.LoadStringW(IDS_STRING_FIRE_ZONE);
	ndx = m_property.AddString(prop);
	m_property.SetItemData(ndx, ZP_FIRE);

	prop.LoadStringW(IDS_STRING_DURESS_ZONE);
	ndx = m_property.AddString(prop);
	m_property.SetItemData(ndx, ZP_DURESS);

	prop.LoadStringW(IDS_STRING_GAS_ZONE);
	ndx = m_property.AddString(prop);
	m_property.SetItemData(ndx, ZP_GAS);

	prop.LoadStringW(IDS_STRING_WATER_ZONE);
	ndx = m_property.AddString(prop);
	m_property.SetItemData(ndx, ZP_WATER);


	m_property.SetCurSel(0); 
	m_buglar_property.SetCurSel(0);
	m_property.EnableWindow(0);
	m_buglar_property.EnableWindow(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CMannualyAddZoneWrite2MachineDlg::OnBnClickedOk()
{
	CString szone, saddr;
	m_cmbZone.GetWindowTextW(szone);
	m_addr.GetWindowTextW(saddr);
	if (szone.IsEmpty() || saddr.IsEmpty())
		return;
	m_zone = _ttoi(szone);
	CZoneInfo* zoneInfo = m_machine->GetZone(m_zone);
	if (NULL != zoneInfo) {
		CString e, fm; fm.LoadStringW(IDS_STRING_FM_ZONE_ALREADY_EXSISTS);
		e.Format(fm, zoneInfo->get_alias());
		MessageBox(e, L"", MB_ICONINFORMATION);
		m_cmbZone.SetFocus();
		return;
	}

	if (saddr.GetLength() != 4) {
		m_addr.SetFocus();
		return;
	}

	USES_CONVERSION;
	const char* addr = W2A(saddr);
	m_waddr = 0;
	try {
		m_waddr = ademco::HexCharArrayToDec(addr, 4) & 0xFFFF;
	} catch (...) {
		m_addr.SetFocus();
		return;
	}

	int ndx = m_type.GetCurSel();
	if (ndx < 0)return;
	m_gg = m_type.GetItemData(ndx);

	if (m_gg != INDEX_SUB_MACHINE) {
		ndx = m_property.GetCurSel();
		if (ndx < 0) {
			m_property.SetFocus();
			return;
		}

		m_zs = m_property.GetItemData(ndx);
		if (m_zs == ZSOP_INVALID) {
			ndx = m_buglar_property.GetCurSel();
			if (ndx < 0) {
				m_buglar_property.SetFocus();
				return;
			}
			m_zs = m_buglar_property.GetItemData(ndx);
		}
	}

	char xdata[3] = { m_zs & 0xFF, HIBYTE(m_waddr), LOBYTE(m_waddr) };
	int xdata_len = 3;

	m_dwStartTime = GetTickCount();
	//SetTimer(1, 100, NULL);

	m_machine->RegisterObserver(this, OnAdemcoEvent);

	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	mgr->RemoteControlAlarmMachine(m_machine,
								   EVENT_WRITE_TO_MACHINE,
								   m_gg, m_zone,
								   xdata, xdata_len, this);

	while (1) {
		if (m_bRestoreSuccess) {
			//KillTimer(1);
			CString e; e.LoadStringW(IDS_STRING_SUCCESS);
			MessageBox(e);
			CDialogEx::OnOK();
			return;
		} else {
			if (GetTickCount() - m_dwStartTime > 5000) {
				KillTimer(1);
				CString e; e.LoadStringW(IDS_STRING_FAILED);
				MessageBox(e);
				CDialogEx::OnCancel();
				return;
			}
		}
	}


}


void CMannualyAddZoneWrite2MachineDlg::OnCbnSelchangeComboType()
{
	int ndx = m_type.GetCurSel();
	if (ndx < 0)return;
	int gg = m_type.GetItemData(ndx);
	if (gg == INDEX_SUB_MACHINE) {
		m_property.EnableWindow(0);
		m_buglar_property.EnableWindow(0);
	} else if (gg == INDEX_ZONE) {
		m_property.EnableWindow(1);
		m_buglar_property.EnableWindow(1);
	}
}


void CMannualyAddZoneWrite2MachineDlg::OnCbnSelchangeComboProperty()
{
	int ndx = m_property.GetCurSel();
	if (ndx < 0)return;

	int prop = m_property.GetItemData(ndx);
	if (prop == ZSOP_INVALID) {
		m_buglar_property.EnableWindow(1);
	} else {
		m_buglar_property.EnableWindow(0);
	}
}


void CMannualyAddZoneWrite2MachineDlg::OnAdemcoEventResult(const ademco::AdemcoEvent* ademcoEvent)
{
	switch (ademcoEvent->_event) {
		case EVENT_RETRIEVE_SUB_MACHINE:
			if (m_zone == ademcoEvent->_zone && m_gg == ademcoEvent->_sub_zone) {
				m_bRestoreSuccess = TRUE;
			}
			break;
		default:
			break;
	}
}


void CMannualyAddZoneWrite2MachineDlg::OnTimer(UINT_PTR nIDEvent)
{
	

	CDialogEx::OnTimer(nIDEvent);
}


void CMannualyAddZoneWrite2MachineDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	KillTimer(1);
	m_machine->UnRegisterObserver(this);
}