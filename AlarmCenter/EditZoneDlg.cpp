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
#include "AddZoneDlg.h"

using namespace core;

// CEditZoneDlg dialog

IMPLEMENT_DYNAMIC(CEditZoneDlg, CDialogEx)

CEditZoneDlg::CEditZoneDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEditZoneDlg::IDD, pParent)
	, m_machine(NULL)
	, m_rootItem(NULL)
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

	CString txt, szone, ssensor, ssubmachine, sssubmachine;
	ssensor.LoadStringW(IDS_STRING_SENSOR);
	ssubmachine.LoadStringW(IDS_STRING_SUBMACHINE);
	sssubmachine.LoadStringW(IDS_STRING_SSUBMACHINE);

	int ndx = m_type.InsertString(ZT_ZONE, ssensor);
	VERIFY(ndx == ZT_ZONE);
	ndx = m_type.InsertString(ZT_SUB_MACHINE, ssubmachine);
	VERIFY(ndx == ZT_SUB_MACHINE);

	CString sroot;
	sroot.LoadStringW(IDS_STRING_ZONE_INFO);
	HTREEITEM hRoot = m_tree.GetRootItem();
	m_rootItem = m_tree.InsertItem(sroot, hRoot);
	m_tree.SetItemData(m_rootItem, NULL);
	m_tree.Expand(m_rootItem, TVE_EXPAND);

	CZoneInfoList list;
	m_machine->GetAllZoneInfo(list);
	CZoneInfoListIter iter = list.begin();
	while (iter != list.end()) {
		CZoneInfo* zone = *iter++;
		if (m_machine->get_is_submachine()) {
			szone.Format(L"%02d", zone->get_sub_zone());
		} else {
			szone.Format(L"%03d", zone->get_zone_value());
		}
		txt.Format(L"--%s--%s",
				   zone->get_type() == ZT_SUB_MACHINE ? sssubmachine : ssensor,
				   zone->get_alias());
		HTREEITEM hChild = m_tree.InsertItem(szone + txt, m_rootItem);
		m_tree.SetItemData(hChild, reinterpret_cast<DWORD_PTR>(zone));
	}

	m_tree.Expand(m_rootItem, TVE_EXPAND);
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
		szone.Format(L"%02d", zoneInfo->get_sub_zone());
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


void CEditZoneDlg::SelectItem(DWORD_PTR zoneInfo)
{
	HTREEITEM hItem = m_tree.GetChildItem(m_rootItem);
	while (hItem) {
		DWORD_PTR data = m_tree.GetItemData(hItem);
		if (zoneInfo == data) {
			m_tree.SelectItem(hItem);
			break;
		} 

		hItem = m_tree.GetNextSiblingItem(hItem);
	}
}


int __stdcall CEditZoneDlg::MyTreeCompareProc(LPARAM lp1, LPARAM lp2, LPARAM lpSort)
{
	CEditZoneDlg* dlg = reinterpret_cast<CEditZoneDlg*>(lpSort);
	CZoneInfo* zoneInfo1 = reinterpret_cast<CZoneInfo*>(lp1);
	CZoneInfo* zoneInfo2 = reinterpret_cast<CZoneInfo*>(lp2);

	if (dlg && zoneInfo1 && zoneInfo2) {
		if (dlg->m_machine->get_is_submachine()) {
			return zoneInfo1->get_sub_zone() - zoneInfo2->get_sub_zone();
		} else {
			return zoneInfo1->get_zone_value() - zoneInfo2->get_zone_value();
		}
	}

	return 0;
}


void CEditZoneDlg::OnBnClickedButtonAddzone()
{
	CAddZoneDlg dlg;
	if (dlg.DoModal() != IDOK)
		return;

	int zoneValue = dlg.m_value;
	CZoneInfo* zoneInfo = m_machine->GetZone(zoneValue);
	if (zoneInfo) {
		SelectItem(reinterpret_cast<DWORD_PTR>(zoneInfo));
	} else {
		zoneInfo = new CZoneInfo();
		if (m_machine->get_is_submachine()) {
			zoneInfo->set_sub_zone(zoneValue);
			zoneInfo->set_type(ZT_SUB_MACHINE_ZONE);
		} else {
			zoneInfo->set_zone_value(zoneValue);
			zoneInfo->set_type(ZT_ZONE);
		}

		if (m_machine->execute_add_zone(zoneInfo)) {
			CString szone, txt, ssensor, ssubmachine;
			ssensor.LoadStringW(IDS_STRING_SENSOR);
			ssubmachine.LoadStringW(IDS_STRING_SSUBMACHINE);

			if (m_machine->get_is_submachine()) {
				szone.Format(L"%02d", zoneInfo->get_sub_zone());
			} else {
				szone.Format(L"%03d", zoneInfo->get_zone_value());
			}
			txt.Format(L"--%s--%s",
					   zoneInfo->get_type() == ZT_SUB_MACHINE ? ssubmachine : ssensor,
					   zoneInfo->get_alias());
			HTREEITEM hItem = m_tree.InsertItem(szone + txt, m_rootItem);
			m_tree.SetItemData(hItem, reinterpret_cast<DWORD_PTR>(zoneInfo));

			TVSORTCB tvs;
			tvs.hParent = TVI_ROOT;
			tvs.lpfnCompare = MyTreeCompareProc;
			tvs.lParam = reinterpret_cast<LPARAM>(this);
			m_tree.SortChildrenCB(&tvs);

			m_tree.SelectItem(hItem);
		} else {
			delete zoneInfo;
		}
	}
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


