// EditZoneDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "EditZoneDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "ZoneInfo.h"
#include "ZonePropertyInfo.h"
#include "AlarmMachineManager.h"

using namespace core;

// CEditZoneDlg dialog

IMPLEMENT_DYNAMIC(CEditZoneDlg, CDialogEx)

CEditZoneDlg::CEditZoneDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEditZoneDlg::IDD, pParent)
	, m_machine(NULL)
{

}

CEditZoneDlg::~CEditZoneDlg()
{
}

void CEditZoneDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE1, m_tree);
	DDX_Control(pDX, IDC_EDIT_ZONE, m_zone);
	DDX_Control(pDX, IDC_COMBO_TYPE, m_type);
	DDX_Control(pDX, IDC_EDIT_ALIAS, m_alias);
	DDX_Control(pDX, IDC_STATIC_SUBMACHINE, m_groupSubMachine);
	DDX_Control(pDX, IDC_EDIT_CONTACT, m_contact);
	DDX_Control(pDX, IDC_EDIT_ADDRESS, m_addr);
	DDX_Control(pDX, IDC_EDIT_PHONE, m_phone);
	DDX_Control(pDX, IDC_EDIT_PHONE_BK, m_phone_bk);
}


BEGIN_MESSAGE_MAP(CEditZoneDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_ADDZONE, &CEditZoneDlg::OnBnClickedButtonAddzone)
	ON_BN_CLICKED(IDC_BUTTON_DELZONE, &CEditZoneDlg::OnBnClickedButtonDelzone)
	ON_CBN_SELCHANGE(IDC_COMBO_TYPE, &CEditZoneDlg::OnCbnSelchangeComboZoneType)
	ON_EN_CHANGE(IDC_EDIT_ALIAS, &CEditZoneDlg::OnEnChangeEditAlias)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, &CEditZoneDlg::OnTvnSelchangedTreeZone)
END_MESSAGE_MAP()


// CEditZoneDlg message handlers


BOOL CEditZoneDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	assert(m_machine);

	// 2015年3月3日 14:29:42 添加combobox 属性的项
	/*CString null;
	null.LoadStringW(IDS_STRING_NULL);
	int ndx = m_property.InsertString(0, null);
	m_property.SetItemData(ndx, ZP_MAX);
	CZonePropertyInfo* mgr = CZonePropertyInfo::GetInstance();
	ZoneProperty zp = ZP_BURGLAR;
	while (zp != ZP_MAX) {
		CZonePropertyData* data = mgr->GetZonePropertyDataByProperty(zp);
		ndx = m_property.InsertString(0, data->get_property_text());
		m_property.SetItemData(ndx, zp);
	}

	
	CZonePropertyData* data = NULL;
	*/

	CString txt, ssensor, ssubmachine;
	ssensor.LoadStringW(IDS_STRING_SENSOR);
	ssubmachine.LoadStringW(IDS_STRING_SUBMACHINE);

	int ndx = m_type.InsertString(ZT_ZONE, ssensor);
	VERIFY(ndx == ZT_ZONE);
	ndx = m_type.InsertString(ZT_SUB_MACHINE, ssubmachine);
	VERIFY(ndx == ZT_SUB_MACHINE);

	HTREEITEM hRoot = m_tree.GetRootItem();
	CZoneInfoList list;
	m_machine->GetAllZoneInfo(list);
	CZoneInfoListIter iter = list.begin();
	while (iter != list.end()) {
		CZoneInfo* zone = *iter++;
		txt.Format(L"%03d--%s--%s", zone->get_zone_value(), 
				   zone->get_type() == ZT_SUB_MACHINE ? ssubmachine : ssensor,
				   zone->get_alias());
		HTREEITEM hChild = m_tree.InsertItem(txt, hRoot);
		m_tree.SetItemData(hChild, reinterpret_cast<DWORD_PTR>(zone));
	}

	ExpandWindow(false);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CEditZoneDlg::ExpandWindow(bool expand)
{
	CRect rc, rcSub;
	GetWindowRect(rc);
	m_groupSubMachine.GetWindowRect(rcSub);

	if (expand) {
		rc.right = rcSub.right + 5;
	} else {
		rc.right = rcSub.left;
	}

	MoveWindow(rc);
}


void CEditZoneDlg::OnBnClickedButtonAddzone()
{

}


void CEditZoneDlg::OnBnClickedButtonDelzone()
{

}


void CEditZoneDlg::OnCbnSelchangeComboZoneType()
{

}


void CEditZoneDlg::OnEnChangeEditAlias()
{
	
}


void CEditZoneDlg::OnTvnSelchangedTreeZone(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	*pResult = 0;

	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (!hItem)
		return;

	DWORD data = m_tree.GetItemData(hItem);
	CZoneInfo* zoneInfo = reinterpret_cast<CZoneInfo*>(data);
	if (!zoneInfo)
		return;

	bool bsub = (ZT_SUB_MACHINE == zoneInfo->get_type());
	ExpandWindow(bsub);

	CString szone, salias, scontact, saddr;
	if (m_machine->get_is_submachine()) {
		szone.Format(L"%02d", zoneInfo->get_zone_value());
	} else {
		szone.Format(L"%03d", zoneInfo->get_zone_value());
	}

	m_zone.SetWindowTextW(szone);
	m_type.SetCurSel(bsub ? ZT_SUB_MACHINE : ZT_ZONE);
	m_alias.SetWindowTextW(zoneInfo->get_alias());
	if (bsub) {
		CAlarmMachine* subMachine = zoneInfo->GetSubMachineInfo();
		if (subMachine) {
			m_contact.SetWindowTextW(subMachine->get_contact());
			m_addr.SetWindowTextW(subMachine->get_address());
			m_phone.SetWindowTextW(subMachine->get_phone());
			m_phone_bk.SetWindowTextW(subMachine->get_phone_bk());
		}
	}
}
