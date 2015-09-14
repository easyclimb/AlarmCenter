// ChooseZoneDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "ChooseZoneDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "AlarmMachineManager.h"
#include "ZoneInfo.h"
#include "GroupInfo.h"

// CChooseZoneDlg dialog

IMPLEMENT_DYNAMIC(CChooseZoneDlg, CDialogEx)

CChooseZoneDlg::CChooseZoneDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CChooseZoneDlg::IDD, pParent)
	, m_zone()
{

}

CChooseZoneDlg::~CChooseZoneDlg()
{
}

void CChooseZoneDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDC_TREE1, m_tree);
	DDX_Control(pDX, IDC_LIST_ZONE, m_listZone);
	DDX_Control(pDX, IDC_LIST_SUBZONE, m_listSubMachine);
	DDX_Control(pDX, IDC_STATIC_NOTE, m_staticNote);
}


BEGIN_MESSAGE_MAP(CChooseZoneDlg, CDialogEx)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, &CChooseZoneDlg::OnTvnSelchangedTree1)
	ON_LBN_SELCHANGE(IDC_LIST_ZONE, &CChooseZoneDlg::OnLbnSelchangeListZone)
	ON_LBN_SELCHANGE(IDC_LIST_SUBZONE, &CChooseZoneDlg::OnLbnSelchangeListSubzone)
	ON_BN_CLICKED(IDOK, &CChooseZoneDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CChooseZoneDlg message handlers


BOOL CChooseZoneDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	using namespace core;
	//core::CAlarmMachineManager* mgr = core::CAlarmMachineManager::GetInstance();
	
	CGroupManager* mgr = CGroupManager::GetInstance();
	CGroupInfo* rootGroup = mgr->GetRootGroupInfo();
	if (rootGroup) {
		CString txt;
		txt.Format(L"%s[%d]", rootGroup->get_name(), rootGroup->get_child_machine_count());
		HTREEITEM hRoot = m_tree.GetRootItem();
		HTREEITEM hRootGroup = m_tree.InsertItem(txt, hRoot);
		m_tree.SetItemData(hRootGroup, (DWORD_PTR)rootGroup);

		TraverseGroup(hRootGroup, rootGroup);

		m_tree.Expand(hRootGroup, TVE_EXPAND);
	}
	ResetCurselZoneUudi();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CChooseZoneDlg::TraverseGroup(HTREEITEM hItemGroup, core::CGroupInfo* group)
{
	using namespace core;
	//CGroupManager* mgr = CGroupManager::GetInstance();
	CString txt;
	CGroupInfoList groupList;
	group->GetChildGroups(groupList);

	std::list<CGroupInfo*>::iterator group_iter = groupList.begin();
	while (group_iter != groupList.end()) {
		CGroupInfo* child_group = *group_iter++;
		txt.Format(L"%s[%d]", child_group->get_name(), child_group->get_child_machine_count());
		HTREEITEM hChildItem = m_tree.InsertItem(txt, hItemGroup);
		TraverseGroup(hChildItem, child_group);
	}

	CAlarmMachineList machineList;
	group->GetChildMachines(machineList);
	std::list<CAlarmMachine*>::iterator machine_iter = machineList.begin();
	while (machine_iter != machineList.end()) {
		CAlarmMachine* machine = *machine_iter++;
		txt.Format(L"%s(%04d)", machine->get_alias(), machine->get_ademco_id());
		HTREEITEM hChildItem = m_tree.InsertItem(txt, hItemGroup);
		m_tree.SetItemData(hChildItem, (DWORD_PTR)machine);
	}
}


void CChooseZoneDlg::ResetCurselZoneUudi()
{
	m_staticNote.SetWindowTextW(L"");
	m_btnOk.EnableWindow(0);
}


void CChooseZoneDlg::OnTvnSelchangedTree1(NMHDR * /*pNMHDR*/, LRESULT * /*pResult*/)
{
	m_listZone.ResetContent();
	m_listSubMachine.ResetContent();
	HTREEITEM hItem = m_tree.GetSelectedItem();
	do {
		if (hItem == NULL) break;
		if (hItem == m_tree.GetRootItem()) break;
		if (m_tree.GetChildItem(hItem) != NULL) break;
		core::CAlarmMachine* machine = reinterpret_cast<core::CAlarmMachine*>(m_tree.GetItemData(hItem));
		if (machine == NULL) break;
		core::CZoneInfoList list;
		machine->GetAllZoneInfo(list);
		CString txt;
		for (auto& zone : list) {
			txt.Format(L"%03d(%s)", zone->get_zone_value(), zone->get_alias());
			int ndx = m_listZone.AddString(txt);
			m_listZone.SetItemData(ndx, reinterpret_cast<DWORD_PTR>(zone));
		}
		return;
	} while (0);
	ResetCurselZoneUudi();
}


void CChooseZoneDlg::OnLbnSelchangeListZone()
{
	m_listSubMachine.ResetContent();
	do {
		int ndx = m_listZone.GetCurSel();
		if (ndx < 0) break;
		core::CZoneInfo* zone = reinterpret_cast<core::CZoneInfo*>(m_listZone.GetItemData(ndx));
		if (zone == NULL) break;
		m_zone._ademco_id = zone->get_ademco_id();
		m_zone._zone_value = zone->get_zone_value();
		m_zone._gg = core::INDEX_ZONE;

		CString txt;
		core::CAlarmMachine* subMachine = zone->GetSubMachineInfo();
		if (subMachine) {
			core::CZoneInfoList list;
			subMachine->GetAllZoneInfo(list);
			for (auto& subZone : list) {
				txt.Format(L"%02d(%s)", subZone->get_sub_zone(), subZone->get_alias());
				int ndx = m_listSubMachine.AddString(txt);
				m_listSubMachine.SetItemData(ndx, reinterpret_cast<DWORD_PTR>(subZone));
			}
			m_staticNote.SetWindowTextW(L"");
			m_btnOk.EnableWindow(0);
		} else {
			txt.Format(L"%04d--%03d", zone->get_ademco_id(), zone->get_zone_value());
			m_staticNote.SetWindowTextW(txt);
			m_btnOk.EnableWindow();
		}
		return;
	} while (0);
	ResetCurselZoneUudi();
}


void CChooseZoneDlg::OnLbnSelchangeListSubzone()
{
	int ndx = m_listSubMachine.GetCurSel();
	if (ndx < 0) return;
	core::CZoneInfo* subZone = reinterpret_cast<core::CZoneInfo*>(m_listSubMachine.GetItemData(ndx));
	if (subZone == NULL) return;
	m_zone._gg = subZone->get_sub_zone();
	CString txt;
	txt.Format(L"%04d--%03d--%02d", subZone->get_ademco_id(), subZone->get_zone_value(), subZone->get_sub_zone());
	m_staticNote.SetWindowTextW(txt);
	m_btnOk.EnableWindow();
}


void CChooseZoneDlg::OnBnClickedOk()
{
	CDialogEx::OnOK();
}
