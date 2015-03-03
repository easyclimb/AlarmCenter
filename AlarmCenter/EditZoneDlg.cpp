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
	DDX_Control(pDX, IDC_EDIT1, m_zone);
	DDX_Control(pDX, IDC_COMBO1, m_property);
	DDX_Control(pDX, IDC_EDIT2, m_alias);
}


BEGIN_MESSAGE_MAP(CEditZoneDlg, CDialogEx)
END_MESSAGE_MAP()


// CEditZoneDlg message handlers


BOOL CEditZoneDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	assert(m_machine);

	// 2015年3月3日 14:29:42 添加combobox 属性的项
	CString null;
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

	CString txt;
	CZonePropertyData* data = NULL;
	HTREEITEM hRoot = m_tree.GetRootItem();
	CZoneInfoList list;
	m_machine->GetAllZoneInfo(list);
	CZoneInfoListIter iter = list.begin();
	while (iter != list.end()) {
		CZoneInfo* zone = *iter++;
		data = mgr->GetZonePropertyDataByProperty(zone->get_property_id());
		txt.Format(L"%03d--%s--%s", zone->get_zone(), 
				   data ? data->get_property_text() : null, 
				   zone->get_alias());
		HTREEITEM hChild = m_tree.InsertItem(txt, hRoot);
		m_tree.SetItemData(hChild, reinterpret_cast<DWORD_PTR>(zone));
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
