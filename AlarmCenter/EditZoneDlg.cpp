// EditZoneDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "EditZoneDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "ZoneInfo.h"
#include "MapInfo.h"
#include "AlarmMachineManager.h"
#include "AddZoneDlg.h"
#include "DetectorInfo.h"
#include "DetectorLib.h"
#include "ChooseDetDlg.h"
#include "AlarmmachineDlg.h"
#include "RetrieveProgressDlg.h"

using namespace core;

// CEditZoneDlg dialog

IMPLEMENT_DYNAMIC(CEditZoneDlg, CDialogEx)

CEditZoneDlg::CEditZoneDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEditZoneDlg::IDD, pParent)
	, m_machine(NULL)
	, m_rootItem(NULL)
	, m_bNeedReloadMaps(FALSE)
	, m_machineDlg(NULL)
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
	ON_EN_CHANGE(IDC_EDIT_CONTACT, &CEditZoneDlg::OnEnChangeEditContact)
	ON_EN_CHANGE(IDC_EDIT_ADDRESS, &CEditZoneDlg::OnEnChangeEditAddress)
	ON_EN_CHANGE(IDC_EDIT_PHONE, &CEditZoneDlg::OnEnChangeEditPhone)
	ON_EN_CHANGE(IDC_EDIT_PHONE_BK, &CEditZoneDlg::OnEnChangeEditPhoneBk)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_DETECTOR, &CEditZoneDlg::OnBnClickedButtonEditDetector)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CEditZoneDlg message handlers


BOOL CEditZoneDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	assert(m_machine);

	CString txt, ssensor, ssubmachine, sssubmachine;
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
		CZoneInfo* zoneInfo = *iter++;
		FormatZoneInfoText(m_machine, zoneInfo, txt);
		HTREEITEM hChild = m_tree.InsertItem(txt, m_rootItem);
		m_tree.SetItemData(hChild, reinterpret_cast<DWORD_PTR>(zoneInfo));
	}

	m_tree.Expand(m_rootItem, TVE_EXPAND);
	ExpandWindow(false);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CEditZoneDlg::FormatZoneInfoText(const CAlarmMachine* const machine, 
									  const CZoneInfo* const zoneInfo,
									  CString& txt)
{
	AUTO_LOG_FUNCTION;
	CString szone, ssensor, sssubmachine, salias;
	ssensor.LoadStringW(IDS_STRING_SENSOR);
	sssubmachine.LoadStringW(IDS_STRING_SSUBMACHINE);
	
	if (machine->get_is_submachine()) {
		szone.Format(L"%02d", zoneInfo->get_sub_zone());
	} else {
		szone.Format(L"%03d", zoneInfo->get_zone_value());
	}

	salias = zoneInfo->get_alias();
	if (salias.IsEmpty())
		salias.LoadStringW(IDS_STRING_NULL);
	txt.Format(L"%s--%s--%s", szone,
			   zoneInfo->get_type() == ZT_SUB_MACHINE ? sssubmachine : ssensor,
			   salias);
}


void CEditZoneDlg::ExpandWindow(bool expand)
{
	AUTO_LOG_FUNCTION;
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
	AUTO_LOG_FUNCTION;
	*pResult = 0;

	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (!hItem)
		return;

	DWORD data = m_tree.GetItemData(hItem);
	CZoneInfo* zoneInfo = reinterpret_cast<CZoneInfo*>(data);
	if (!zoneInfo) {
		m_zone.SetWindowTextW(L"");
		m_type.SetCurSel(-1);
		m_alias.SetWindowTextW(L"");
		ExpandWindow(false);
		return;
	}
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
	AUTO_LOG_FUNCTION;
	HTREEITEM hItem = m_tree.GetChildItem(m_rootItem);
	while (hItem) {
		if (m_tree.GetItemData(hItem) == zoneInfo) {
			m_tree.SelectItem(hItem); break;
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
		if (dlg->m_machine->get_is_submachine()) 
			return zoneInfo1->get_sub_zone() - zoneInfo2->get_sub_zone();
		else 
			return zoneInfo1->get_zone_value() - zoneInfo2->get_zone_value();
	}

	return 0;
}


void CEditZoneDlg::OnBnClickedButtonAddzone()
{
	AUTO_LOG_FUNCTION;
	int default_zone_value = 0;
	for (int i = 1; i < MAX_MACHINE_ZONE; i++) {
		CZoneInfo* zoneInfo = m_machine->GetZone(i);
		if (!zoneInfo) {
			default_zone_value = i;
			break;
		}
	}
	CAddZoneDlg dlg;
	dlg.m_value = default_zone_value;
	if (dlg.DoModal() != IDOK)
		return;

	int zoneValue = dlg.m_value;
	CZoneInfo* zoneInfo = m_machine->GetZone(zoneValue);
	if (zoneInfo) {
		SelectItem(reinterpret_cast<DWORD_PTR>(zoneInfo));
	} else {
		if (!m_machine->get_is_submachine()) {
			CRetrieveProgressDlg retrieveProgressDlg;
			retrieveProgressDlg.m_machine = m_machine;
			retrieveProgressDlg.m_zone = zoneValue;
			if (retrieveProgressDlg.DoModal() != IDOK)
				return;
			zoneInfo = new CZoneInfo();
			zoneInfo->set_zone_value(zoneValue);
			zoneInfo->set_type(ZT_ZONE);
		} else {
			zoneInfo = new CZoneInfo();
			zoneInfo->set_sub_zone(zoneValue);
			zoneInfo->set_type(ZT_SUB_MACHINE_ZONE);
		}
		if (m_machine->execute_add_zone(zoneInfo)) {
			CString txt;
			FormatZoneInfoText(m_machine, zoneInfo, txt);
			HTREEITEM hItem = m_tree.InsertItem(txt, m_rootItem);
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
	AUTO_LOG_FUNCTION;
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (!hItem)
		return;

	DWORD data = m_tree.GetItemData(hItem);
	CZoneInfo* zoneInfo = reinterpret_cast<CZoneInfo*>(data);
	if (!zoneInfo)
		return;

	bool ok = true;
	if (ZT_SUB_MACHINE == zoneInfo->get_type()) { // 删除分机 (如果存在)
		if (!DeleteSubMachine(zoneInfo)) { 
			ok = false;
		}
	} 

	// 删除防区
	if (ok) {
		bool hasDet = (zoneInfo->GetDetectorInfo() != NULL);
		if (hasDet) {
			CString q; q.LoadStringW(IDS_STRING_Q_CONFIRM_DEL_DET);
			int ret = MessageBox(q, NULL, MB_OKCANCEL | MB_ICONWARNING);
			if (IDOK != ret) {
				LOG(L"user canceled delete zone\n");
				ok = false;
			}
		}

		if (ok)
			ok = m_machine->execute_del_zone(zoneInfo);

		if (ok && (ZT_SUB_MACHINE == zoneInfo->get_type())) {
			m_machine->dec_submachine_count();
		}
	
		if (ok && hasDet) {
			m_bNeedReloadMaps = TRUE;
		}
	}

	if (ok) {
		HTREEITEM hNext = m_tree.GetNextSiblingItem(hItem);
		m_tree.DeleteItem(hItem);
		m_tree.SelectItem(hNext ? hNext : m_rootItem);
		//m_tree.SelectItem(m_rootItem);
	}
}


void CEditZoneDlg::OnCbnSelchangeComboZoneType()
{
	AUTO_LOG_FUNCTION;
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
	do {
		if (ndx == ZT_ZONE) { // 分机变为防区
			// 1.删除分机
			if (!DeleteSubMachine(zoneInfo)) {
				LOG(L"ChangeDetectorImage failed.\n"); ok = false; break;
			}
			m_machine->dec_submachine_count();

			// 2.变更图标 (若原图标存在且为分机图标，则修改为探头图标)
			//if (!ChangeDetectorImage(zoneInfo, DT_SINGLE | DT_DOUBLE)) {
			//	LOG(L"ChangeDetectorImage failed.\n");
			//}
		} else if (ndx == ZT_SUB_MACHINE) { // 防区变为分机
			// 1.创建分机
			CString null;
			null.LoadStringW(IDS_STRING_NULL);
			CAlarmMachine* subMachine = new CAlarmMachine();
			subMachine->set_is_submachine(true);
			subMachine->set_ademco_id(zoneInfo->get_ademco_id());
			subMachine->set_submachine_zone(zoneInfo->get_zone_value());
			subMachine->set_alias(zoneInfo->get_alias());
			subMachine->set_address(null);
			subMachine->set_contact(null);
			subMachine->set_phone(null);
			subMachine->set_phone_bk(null);
			if (!zoneInfo->execute_set_sub_machine(subMachine)) {
				ASSERT(0); LOG(L"execute_set_sub_machine failed.\n"); ok = false; break;
			}
			m_machine->inc_submachine_count();

			// 2.变更图标 (若原图标存在且为探头图标，则修改为分机图标)
			//if (!ChangeDetectorImage(zoneInfo, DT_SUB_MACHINE)) {
			//	LOG(L"ChangeDetectorImage failed.\n");
			//}
			// 2015年4月21日 19:28:21 删除图标
			// 2.1.删除detector
			CMapInfo* mapInfo = zoneInfo->GetMapInfo();
			CDetectorInfo* detInfo = zoneInfo->GetDetectorInfo();
			if (mapInfo && detInfo) {
				mapInfo->SetActiveZoneInfo(zoneInfo);
				mapInfo->InversionControl(ICMC_DEL_DETECTOR);
			}

			// 2.2.更新数据库
			if (detInfo)
				zoneInfo->execute_del_detector_info();
		}
	} while (0);

	if (!ok) {
		m_type.SetCurSel(zoneInfo->get_type());
	} else {
		CString txt;
		FormatZoneInfoText(m_machine, zoneInfo, txt);
		m_tree.SetItemText(hItem, txt);
		m_tree.SelectItem(m_rootItem);
		m_tree.SelectItem(hItem);
		m_bNeedReloadMaps = TRUE;
	}
}


bool CEditZoneDlg::ChangeDetectorImage(CZoneInfo* zoneInfo, int newType)
{
	AUTO_LOG_FUNCTION;
	CDetectorInfo* detInfo = zoneInfo->GetDetectorInfo();
	if (!detInfo) {
		LOG(L"this zone has no detector.\n");
		return true;
	}

	if (newType >= DT_MAX) {
		LOG(L"invalid newType %d.\n", newType);
		return false;
	}

	CDetectorLib* lib = CDetectorLib::GetInstance();
	const CDetectorLibData* libData = lib->GetDetectorLibData(detInfo->get_detector_lib_id());
	if (libData->get_type() & newType) {
		LOG(L"newType is the same as old type.\n");
		return true;
	}

	CString q;
	q.LoadStringW((ZT_SUB_MACHINE == newType) ? IDS_STRING_Q_CHANGE_DET : IDS_STRING_Q_CHANGE_SUBMACHINE);
	int ret = MessageBox(q, NULL, MB_OKCANCEL | MB_ICONQUESTION);
	if (ret != IDOK) {
		LOG(L"user canceled change det type from sensor to submachine\n");
		return true;
	}

	CChooseDetDlg dlg;
	dlg.m_detType2Show = newType;
	if (IDOK != dlg.DoModal()) {
		LOG(L"user canceled choose det type\n");
		return true;
	}

	CString query;
	query.Format(L"update DetectorInfo set detector_lib_id=%d where id=%d",
					dlg.m_chosenDetectorID, detInfo->get_id());
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (mgr->ExecuteSql(query))
		detInfo->set_detector_lib_id(dlg.m_chosenDetectorID);
	else {
		LOG(L"update DetectorInfo failed: %s\n", query);
		ASSERT(0); return false;
	}

	m_bNeedReloadMaps = TRUE;
	return true;
}


bool CEditZoneDlg::DeleteSubMachine(CZoneInfo* zoneInfo)
{
	AUTO_LOG_FUNCTION;
	CAlarmMachine* subMachine = zoneInfo->GetSubMachineInfo();
	if (subMachine) {
		CString q; q.LoadStringW(IDS_STRING_Q_CONFIRM_DEL_SUBMACHINE);
		int ret = MessageBox(q, NULL, MB_OKCANCEL | MB_ICONWARNING);
		if (IDOK != ret) {
			LOG(L"user canceled change submachine to zone\n");
			return false;
		}

		if (!zoneInfo->execute_del_sub_machine()) {
			LOG(L"delete submachine failed\n");
			ASSERT(0); return false;
		}
	}
	return true;
}


void CEditZoneDlg::OnEnChangeEditAlias()
{
	AUTO_LOG_FUNCTION;
	CString alias;
	m_alias.GetWindowTextW(alias);

	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (!hItem)
		return;

	DWORD data = m_tree.GetItemData(hItem);
	CZoneInfo* zoneInfo = reinterpret_cast<CZoneInfo*>(data);
	if (!zoneInfo)
		return;

	if (zoneInfo->execute_update_alias(alias)) {
		CString txt; 
		FormatZoneInfoText(m_machine, zoneInfo, txt);
		m_tree.SetItemText(hItem, txt);
	}
}


void CEditZoneDlg::OnEnChangeEditContact()
{
	AUTO_LOG_FUNCTION;
	CString contact;
	m_contact.GetWindowTextW(contact);

	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (!hItem)
		return;

	DWORD data = m_tree.GetItemData(hItem);
	CZoneInfo* zoneInfo = reinterpret_cast<CZoneInfo*>(data);
	if (!zoneInfo)
		return;

	zoneInfo->execute_update_contact(contact);
}


void CEditZoneDlg::OnEnChangeEditAddress()
{
	AUTO_LOG_FUNCTION;
	CString address;
	m_addr.GetWindowTextW(address);

	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (!hItem)
		return;

	DWORD data = m_tree.GetItemData(hItem);
	CZoneInfo* zoneInfo = reinterpret_cast<CZoneInfo*>(data);
	if (!zoneInfo)
		return;

	zoneInfo->execute_update_address(address);
}


void CEditZoneDlg::OnEnChangeEditPhone()
{
	AUTO_LOG_FUNCTION;
	CString phone;
	m_phone.GetWindowTextW(phone);

	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (!hItem)
		return;

	DWORD data = m_tree.GetItemData(hItem);
	CZoneInfo* zoneInfo = reinterpret_cast<CZoneInfo*>(data);
	if (!zoneInfo)
		return;

	zoneInfo->execute_update_phone(phone);
}


void CEditZoneDlg::OnEnChangeEditPhoneBk()
{
	AUTO_LOG_FUNCTION;
	CString phone_bk;
	m_phone_bk.GetWindowTextW(phone_bk);

	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (!hItem)
		return;

	DWORD data = m_tree.GetItemData(hItem);
	CZoneInfo* zoneInfo = reinterpret_cast<CZoneInfo*>(data);
	if (!zoneInfo)
		return;

	zoneInfo->execute_update_phone_bk(phone_bk);
}


void CEditZoneDlg::OnBnClickedButtonEditDetector()
{
	AUTO_LOG_FUNCTION;
}


void CEditZoneDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	
}
