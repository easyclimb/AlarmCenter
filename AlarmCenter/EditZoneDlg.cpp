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
#include "DetectorInfo.h"
#include "DetectorLib.h"
#include "ChooseDetDlg.h"

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
	if (!m_machine->get_is_submachine()) {
		ndx = m_type.InsertString(ZT_SUB_MACHINE, ssubmachine);
		VERIFY(ndx == ZT_SUB_MACHINE);
	}

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
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (!hItem)
		return;

	DWORD data = m_tree.GetItemData(hItem);
	CZoneInfo* zoneInfo = reinterpret_cast<CZoneInfo*>(data);
	if (!zoneInfo)
		return;


}


void CEditZoneDlg::OnCbnSelchangeComboZoneType()
{
	LOG_FUNCTION_AUTO;
#pragma region test integrity
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (!hItem)
		return;

	DWORD data = m_tree.GetItemData(hItem);
	CZoneInfo* zoneInfo = reinterpret_cast<CZoneInfo*>(data);
	if (!zoneInfo)
		return;

	int ndx = m_type.GetCurSel();
	if (ndx < 0)
		return;

	if (ndx == zoneInfo->get_type())
		return;
#pragma endregion

	bool ok = true;
	CString query;
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	do {
		if (ndx == ZT_ZONE) { 
			// 分机变为防区
#pragma region submachine --> zone
			CAlarmMachine* subMachine = zoneInfo->GetSubMachineInfo();
			if (subMachine) {

			}
#pragma endregion
		} else if (ndx == ZT_SUB_MACHINE) { 
			// 防区变为分机
#pragma region zone --> submachine
			// 1.修改探头图标 (若原图标存在且为探头图标，则修改为分机图标)
#pragma region reset det type	
			CDetectorInfo* detInfo = zoneInfo->GetDetectorInfo();
			if (detInfo) { // 已绑定探头
				CDetectorLib* lib = CDetectorLib::GetInstance();
				const CDetectorLibData* libData = lib->GetDetectorLibData(detInfo->get_detector_lib_id());
				if (libData->get_type() != DT_SUBMACHINE) { // 探头类型非分机，需更换为分机类型
					CString q;
					q.LoadStringW(IDS_STRING_Q_CHANGE_DET);
					int ret = MessageBox(q, NULL, MB_OKCANCEL | MB_ICONQUESTION);
					if (ret != IDOK) { 
						LOG(L"user canceled change det type from sensor to submachine\n"); 
						ok = false; break; 
					}
					CChooseDetDlg dlg;
					dlg.m_detType2Show = DT_SUBMACHINE;
					if (IDOK != dlg.DoModal()) { 
						LOG(L"user canceled choose det type\n"); 
						ok = false; break; 
					}
					query.Format(L"update DetectorInfo set detector_lib_id=%d where id=%d",
								 dlg.m_chosenDetectorID, detInfo->get_id());
					if (mgr->ExecuteSql(query))
						detInfo->set_detector_lib_id(dlg.m_chosenDetectorID);
					else { 
						LOG(L"update DetectorInfo failed: %s\n", query); 
						ASSERT(0); ok = false; break;
					}
				}
			} 
#pragma endregion	

			// 2.创建分机信息
#pragma region create submachine
			CString null;
			null.LoadStringW(IDS_STRING_NULL);
			CAlarmMachine* subMachine = new CAlarmMachine();
			subMachine->set_is_submachine(true);
			subMachine->set_submachine_zone(zoneInfo->get_zone_value());
			subMachine->set_alias(zoneInfo->get_alias());
			subMachine->set_address(null);
			subMachine->set_contact(null);
			subMachine->set_phone(null);
			subMachine->set_phone_bk(null);
			query.Format(L"insert into SubMachine ([contact],[address],[phone],[phone_bk]) values('%s','%s','%s','%s')",
						 null, null, null, null);
			int id = mgr->AddAutoIndexTableReturnID(query);
			if (-1 == id) {
				LOG(L"add submachine failed: %s\n", query);
				ASSERT(0); ok = false; break;
			}
			subMachine->set_id(id);
#pragma endregion

			// 3.更新防区信息
#pragma region update zone info
			query.Format(L"update ZoneInfo set type=%d,sub_machine_id=%d where id=%d",
						 ZT_SUB_MACHINE, id, zoneInfo->get_id());
			if (!mgr->ExecuteSql(query)) {
				LOG(L"update ZoneInfo type failed: %s\n", query);
				ASSERT(0); ok = false; break;
			}
			zoneInfo->set_type(ZT_SUB_MACHINE);
			zoneInfo->set_sub_machine_id(id);
			zoneInfo->SetSubMachineInfo(subMachine);
#pragma endregion
#pragma endregion
		}
	} while (0);

	if (!ok) {
		m_type.SetCurSel(zoneInfo->get_type());
	} else {
		m_tree.SelectItem(m_rootItem);
		m_tree.SelectItem(hItem);
	}
}


void CEditZoneDlg::OnEnChangeEditAlias()
{
	
}


