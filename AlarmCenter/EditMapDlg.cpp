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
	m_preview.ShowBmp(mapInfo->get_path());
}


void CEditMapDlg::OnBnClickedOk()
{
	return;
}


BOOL CEditMapDlg::OpenFile(CString& path)
{
	AUTO_LOG_FUNCTION;
	TCHAR szFilename[MAX_PATH] = { 0 };
	BOOL bResult = FALSE;
	DWORD dwError = NOERROR;
	OPENFILENAME ofn = { 0 };

	ofn.lStructSize = sizeof (OPENFILENAME);
	ofn.lpstrFilter = _T("Bitmap(*.bmp)\0*.bmp\0\0");
	ofn.lpstrFile = szFilename;
	ofn.nMaxFile = MAX_PATH;
	ofn.hwndOwner = GetSafeHwnd();
	ofn.Flags = OFN_EXPLORER
		| OFN_ENABLEHOOK
		| OFN_HIDEREADONLY
		| OFN_NOCHANGEDIR
		| OFN_PATHMUSTEXIST;
	ofn.lpfnHook = NULL;

	bResult = GetOpenFileName(&ofn);
	if (bResult == FALSE) {
		dwError = CommDlgExtendedError();
		return FALSE;
	} else {
		path = szFilename;
		return TRUE;
	}
}


void CEditMapDlg::OnBnClickedButtonAddMap()
{
	AUTO_LOG_FUNCTION;
	CString path;
	if (!OpenFile(path)) { return; }
	CString alias = CFileOper::GetFileTitle(path);
	CString newPath;
	int append = 1;
	newPath.Format(L"%s\\%s.bmp", GetModuleFilePath(), alias);
	LOG(L"copying file from %s to %s\n", path, newPath);
	BOOL ret = CopyFile(path, newPath, TRUE);
	while (!ret) {
		newPath.Format(L"%s\\%s-%d.bmp", GetModuleFilePath(), alias, append++);
		LOG(L"copy file failed, recopy: %s\n", newPath);
		ret = CopyFile(path, newPath, TRUE);
	}
	LOG(L"copy file succeeded.\n");
	path = newPath;

	CMapInfo* mapInfo = new CMapInfo();
	mapInfo->set_alias(alias);
	mapInfo->set_path(path);
	if (m_machine->execute_add_map(mapInfo)) {
		CString txt;
		FormatMapText(mapInfo, txt);
		HTREEITEM hItem = m_tree.InsertItem(txt, m_rootItem);
		m_tree.SetItemData(hItem, reinterpret_cast<DWORD_PTR>(mapInfo));
		m_tree.SelectItem(hItem);
		m_bNeedReloadMaps = TRUE;
	} 
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




