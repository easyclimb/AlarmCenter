// EditMapDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "EditMapDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "MapInfo.h"
#include "UserInfo.h"
#include "AlarmMachineManager.h"

using namespace core;
// CEditMapDlg dialog

IMPLEMENT_DYNAMIC(CEditMapDlg, CDialogEx)

CEditMapDlg::CEditMapDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CEditMapDlg::IDD, pParent)
	, m_prevSelMapInfo(nullptr)
	, m_machine(nullptr)
	, m_rootItem(nullptr)
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
	DDX_Control(pDX, IDC_BUTTON_DEL_MAP, m_btnDeleteMap);
}


BEGIN_MESSAGE_MAP(CEditMapDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CEditMapDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_ADD_MAP, &CEditMapDlg::OnBnClickedButtonAddMap)
	ON_BN_CLICKED(IDC_BUTTON_DEL_MAP, &CEditMapDlg::OnBnClickedButtonDelMap)
	ON_EN_CHANGE(IDC_EDIT_ALIAS, &CEditMapDlg::OnEnChangeEditAlias)
	ON_BN_CLICKED(IDC_BUTTON_CHANGE_FILE, &CEditMapDlg::OnBnClickedButtonChangeFile)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, &CEditMapDlg::OnTvnSelchangedTreeMap)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CEditMapDlg message handlers
BOOL CEditMapDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	assert(m_machine);
	CString sroot;
	sroot = GetStringFromAppResource(IDS_STRING_MAP_INFO);
	HTREEITEM hRoot = m_tree.GetRootItem();
	m_rootItem = m_tree.InsertItem(sroot, hRoot);
	m_tree.SetItemData(m_rootItem, 0);

	CString txt;
	CMapInfoList list;
	m_machine->GetAllMapInfo(list);
	for (auto mapInfo : list) {
		FormatMapText(mapInfo, txt);
		HTREEITEM hItem = m_tree.InsertItem(txt, m_rootItem);
		m_tree.SetItemData(hItem, mapInfo->get_id());
	}

	m_tree.Expand(m_rootItem, TVE_EXPAND);

	CUserManager* userMgr = CUserManager::GetInstance();
	CUserInfoPtr user = userMgr->GetCurUserInfo();
	core::UserPriority user_priority = user->get_user_priority();
	switch (user_priority) {
		case core::UP_SUPER:
		case core::UP_ADMIN:
			m_btnDeleteMap.EnableWindow(1);
			break;
		case core::UP_OPERATOR:
		default:
			m_btnDeleteMap.EnableWindow(0);
			break;
	}

	txt.Format(L"%s\\Maps", GetModuleFilePath());
	CreateDirectory(txt, nullptr);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CEditMapDlg::FormatMapText(const core::CMapInfoPtr& mapInfo, CString& txt)
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

	auto mgr = core::CAlarmMachineManager::GetInstance();
	DWORD data = m_tree.GetItemData(hItem);
	CMapInfoPtr mapInfo = mgr->GetMapInfoById(data);
	if (!mapInfo) {
		m_prevSelMapInfo = nullptr;
		m_alias.SetWindowTextW(L"");
		m_file.SetWindowTextW(L"");
		m_preview.ShowBmp(L"");
		return;
	}
	if (m_prevSelMapInfo == mapInfo)
		return;

	m_alias.SetWindowTextW(mapInfo->get_alias());
	m_file.SetWindowTextW(mapInfo->get_path());
	m_preview.ShowBmp(mapInfo->get_path());
	if (m_prevSelMapInfo) {
		m_prevSelMapInfo->InversionControl(ICMC_MODE_NORMAL);
	}
	m_prevSelMapInfo = mapInfo;
	mapInfo->InversionControl(ICMC_MODE_EDIT);
	mapInfo->InversionControl(ICMC_SHOW);
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
	ofn.lpfnHook = nullptr;

	bResult = GetOpenFileName(&ofn);
	if (bResult == FALSE) {
		dwError = CommDlgExtendedError();
		return FALSE;
	} else {
		if (!CFileOper::PathExists(szFilename)) {
			CString e, fm, title; fm = GetStringFromAppResource(IDS_STRING_FILE_NOT_EXSITS);
			title = GetStringFromAppResource(IDS_STRING_OPEN_FILE);
			e.Format(L"%s\r\n%s", szFilename, fm);
			MessageBox(e, title, MB_ICONINFORMATION);
			return FALSE;
		}
		path = szFilename;
		CString alias = CFileOper::GetFileTitle(path);
		CString newPath;
		int append = 1;
		newPath.Format(L"%s\\Maps\\%s.bmp", GetModuleFilePath(), alias);
		JLOG(L"copying file from %s to %s\n", path, newPath);

		BOOL ret = CopyFile(path, newPath, TRUE);
		while (!ret) {
			newPath.Format(L"%s\\Maps\\%s-%d.bmp", GetModuleFilePath(), alias, append++);
			JLOG(L"copy file failed, recopy: %s\n", newPath);
			ret = CopyFile(path, newPath, TRUE);
		}
		JLOG(L"copy file succeeded.\n");
		path = newPath;
		return TRUE;
	}
}


void CEditMapDlg::OnBnClickedButtonAddMap()
{
	AUTO_LOG_FUNCTION;
	CString path;
	if (!OpenFile(path)) { return; }
	CString alias = CFileOper::GetFileTitle(path);

	CMapInfoPtr mapInfo = std::make_shared<CMapInfo>();
	mapInfo->set_alias(alias);
	mapInfo->set_path(path);
	if (m_machine->execute_add_map(mapInfo)) {
		CString txt;
		FormatMapText(mapInfo, txt);
		HTREEITEM hItem = m_tree.InsertItem(txt, m_rootItem);
		m_tree.SetItemData(hItem, mapInfo->get_id());
		m_tree.SelectItem(hItem);
		m_bNeedReloadMaps = TRUE;
	} 
}


void CEditMapDlg::OnBnClickedButtonDelMap()
{
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (!hItem)
		return;

	auto mgr = core::CAlarmMachineManager::GetInstance();
	DWORD data = m_tree.GetItemData(hItem);
	CMapInfoPtr mapInfo = mgr->GetMapInfoById(data);
	if (!mapInfo)
		return;

	CString q;
	q = GetStringFromAppResource(IDS_STRING_Q_COMFIRM_DEL_MAP);
	int ret = MessageBox(q, nullptr, MB_OKCANCEL | MB_ICONWARNING);
	if (ret != IDOK) {
		JLOG(L"user canceled delete map.\n");
		return;
	}

	if (m_machine->execute_delete_map(mapInfo)) {
		m_prevSelMapInfo = nullptr;
		HTREEITEM hNext = m_tree.GetNextSiblingItem(hItem);
		m_tree.DeleteItem(hItem);
		m_tree.SelectItem(hNext ? hNext : m_rootItem);
		m_bNeedReloadMaps = TRUE;
	} 
}


void CEditMapDlg::OnEnChangeEditAlias()
{
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (!hItem)
		return;

	auto mgr = core::CAlarmMachineManager::GetInstance();
	DWORD data = m_tree.GetItemData(hItem);
	CMapInfoPtr mapInfo = mgr->GetMapInfoById(data);
	if (!mapInfo)
		return;

	CString alias;
	m_alias.GetWindowTextW(alias);
	if (alias.Compare(mapInfo->get_alias()) == 0)
		return;

	if (m_machine->execute_update_map_alias(mapInfo, alias)) {
		mapInfo->InversionControl(ICMC_RENAME);
	}
	CString txt;
	FormatMapText(mapInfo, txt);
	m_tree.SetItemText(hItem, txt);
}


void CEditMapDlg::OnBnClickedButtonChangeFile()
{
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (!hItem)
		return;

	auto mgr = core::CAlarmMachineManager::GetInstance();
	DWORD data = m_tree.GetItemData(hItem);
	CMapInfoPtr mapInfo = mgr->GetMapInfoById(data);
	if (!mapInfo)
		return;

	CString path;
	if (OpenFile(path)) {
		if (m_machine->execute_update_map_path(mapInfo, path)) {
			//m_bNeedReloadMaps = TRUE;
			mapInfo->InversionControl(ICMC_CHANGE_IMAGE);
		}
		CString txt;
		FormatMapText(mapInfo, txt);
		m_tree.SetItemText(hItem, txt);
		m_tree.SelectItem(m_rootItem);
		m_tree.SelectItem(hItem);
	}	
}


void CEditMapDlg::OnClose()
{
	if (m_prevSelMapInfo) {
		m_prevSelMapInfo->InversionControl(ICMC_MODE_NORMAL);
		m_prevSelMapInfo->InversionControl(ICMC_SHOW);
		m_prevSelMapInfo = nullptr;
	}
	CDialogEx::OnClose();
}
