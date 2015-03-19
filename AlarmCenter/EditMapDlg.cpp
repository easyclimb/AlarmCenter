// EditMapDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "EditMapDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "MapInfo.h"


using namespace core;
// CEditMapDlg dialog

IMPLEMENT_DYNAMIC(CEditMapDlg, CDialogEx)

CEditMapDlg::CEditMapDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEditMapDlg::IDD, pParent)
	, m_machine(NULL)
	, m_rootItem(NULL)
	, m_bNeedReloadMaps(FALSE)
{

}

CEditMapDlg::~CEditMapDlg()
{
}

void CEditMapDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_ALIAS, m_alias);
	DDX_Control(pDX, IDC_EDIT_FILE, m_file);
	DDX_Control(pDX, IDC_TREE1, m_tree);
	DDX_Control(pDX, IDC_STATIC_PREVIEW, m_preview);
}


BEGIN_MESSAGE_MAP(CEditMapDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CEditMapDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_ADD_MAP, &CEditMapDlg::OnBnClickedButtonAddMap)
	ON_BN_CLICKED(IDC_BUTTON_DEL_MAP, &CEditMapDlg::OnBnClickedButtonDelMap)
	ON_EN_CHANGE(IDC_EDIT_ALIAS, &CEditMapDlg::OnEnChangeEditAlias)
	ON_BN_CLICKED(IDC_BUTTON_CHANGE_FILE, &CEditMapDlg::OnBnClickedButtonChangeFile)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, &CEditMapDlg::OnTvnSelchangedTreeMap)
END_MESSAGE_MAP()


// CEditMapDlg message handlers
BOOL CEditMapDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	assert(m_machine);
	CString sroot;
	sroot.LoadStringW(IDS_STRING_MAP_INFO);
	HTREEITEM hRoot = m_tree.GetRootItem();
	m_rootItem = m_tree.InsertItem(sroot, hRoot);
	m_tree.SetItemData(m_rootItem, NULL);

	CString txt;
	CMapInfoList list;
	m_machine->GetAllMapInfo(list);
	CMapInfoListIter iter = list.begin();
	while (iter != list.end()) {
		CMapInfo* mapInfo = *iter++;
		FormatMapText(mapInfo, txt);
		HTREEITEM hItem = m_tree.InsertItem(txt, m_rootItem);
		m_tree.SetItemData(hItem, reinterpret_cast<DWORD_PTR>(mapInfo));
	}

	m_tree.Expand(m_rootItem, TVE_EXPAND);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CEditMapDlg::FormatMapText(CMapInfo* mapInfo, CString& txt)
{
	txt = mapInfo->get_alias();
	if (txt.IsEmpty()) {
		txt = mapInfo->get_path();
	}
}


void CEditMapDlg::OnBnClickedOk()
{

	CDialogEx::OnOK();
}


void CEditMapDlg::OnBnClickedButtonAddMap()
{

}


void CEditMapDlg::OnBnClickedButtonDelMap()
{

}


void CEditMapDlg::OnEnChangeEditAlias()
{
	
}


void CEditMapDlg::OnBnClickedButtonChangeFile()
{

}




void CEditMapDlg::OnTvnSelchangedTreeMap(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	*pResult = 0;

	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (!hItem)
		return;

	DWORD data = m_tree.GetItemData(hItem);
	CMapInfo* mapInfo = reinterpret_cast<CMapInfo*>(data);
	if (!mapInfo)
		return;

	m_alias.SetWindowTextW(mapInfo->get_alias());
	m_file.SetWindowTextW(mapInfo->get_path());
}
