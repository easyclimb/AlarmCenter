// UserManagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "UserManagerDlg.h"
#include "afxdialogex.h"
#include "UserInfo.h"
using namespace core;


// CUserManagerDlg dialog

IMPLEMENT_DYNAMIC(CUserManagerDlg, CDialogEx)

CUserManagerDlg::CUserManagerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CUserManagerDlg::IDD, pParent)
	, m_curUser(NULL)
{

}

CUserManagerDlg::~CUserManagerDlg()
{
}

void CUserManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Control(pDX, IDC_EDIT_ID, m_id);
	DDX_Control(pDX, IDC_EDIT_NAME, m_name);
	DDX_Control(pDX, IDC_EDIT_PHONE, m_phone);
	DDX_Control(pDX, IDC_COMBO_PRIORITY, m_priority);
	DDX_Control(pDX, IDC_EDIT_PASSWD, m_passwd);
	DDX_Control(pDX, IDC_BUTTON_DELETE, m_btnDelete);
	DDX_Control(pDX, IDC_BUTTON_UPDATE, m_btnUpdate);
	DDX_Control(pDX, IDC_BUTTON_CHANGE_PASSWD, m_btnChangePasswd);
	DDX_Control(pDX, IDC_BUTTON_ADD, m_btnAdd);
}


BEGIN_MESSAGE_MAP(CUserManagerDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CUserManagerDlg::OnBnClickedButtonClear)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE, &CUserManagerDlg::OnBnClickedButtonUpdate)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CUserManagerDlg::OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, &CUserManagerDlg::OnBnClickedButtonDelete)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, &CUserManagerDlg::OnLvnItemchangedList1)
	ON_BN_CLICKED(IDC_BUTTON_CHANGE_PASSWD, &CUserManagerDlg::OnBnClickedButtonChangePasswd)
END_MESSAGE_MAP()


// CUserManagerDlg message handlers


void CUserManagerDlg::OnBnClickedButtonClear()
{
	CUserManager* mgr = CUserManager::GetInstance();
	int id = mgr->DistributeUserID();
	CString sid;
	if (id == -1)
		sid.Empty();
	else
		sid.Format(L"%d", id);

	m_id.SetWindowTextW(sid);
	m_name.SetWindowTextW(L"");
	m_phone.SetWindowTextW(L"");
	m_priority.SetCurSel(-1);
	m_passwd.SetWindowTextW(L"");
	m_curUser = NULL;

	m_name.EnableWindow();
	m_phone.EnableWindow();
	m_priority.EnableWindow();

	m_btnAdd.EnableWindow();
	m_btnUpdate.EnableWindow(0);
	m_btnDelete.EnableWindow(0);
	m_btnChangePasswd.EnableWindow(0);

	m_list.SetHotItem(-1);
}


void CUserManagerDlg::OnBnClickedButtonUpdate()
{
	UpdateData();
	if (!m_curUser)
		return;

	CString sid, name, phone;
	m_id.GetWindowText(sid);
	int id = _wtoi(sid);
	if (id != m_curUser->get_user_id())
		return;
	m_name.GetWindowTextW(name);
	m_phone.GetWindowTextW(phone);

	UserPriority priority = UP_OPERATOR;

	if (m_curUser->get_user_priority() == UP_SUPER) {
		priority = UP_SUPER;
	} else {
		int ndx = m_priority.GetCurSel();
		if (ndx < 0)
			return;
		if (ndx == 1)
			priority = UP_ADMIN;
		else
			priority = UP_OPERATOR;
	}

	BOOL bUpdated = TRUE;
	do {
		if (wcscmp(name, m_curUser->get_user_name()) != 0)
			break;
		if (wcscmp(phone, m_curUser->get_user_phone()) != 0)
			break;
		if (priority != m_curUser->get_user_priority())
			break;

		bUpdated = FALSE;
	} while (0);

	if (!bUpdated)
		return;

	CUserInfo user;
	user.set_user_name(name);
	user.set_user_phone(phone);
	user.set_user_priority(priority);

	CUserManager* mgr = CUserManager::GetInstance();
	BOOL ok = mgr->UpdateUserInfo(id, user);
	if (ok) {
		CString txt;
		txt.LoadStringW(IDS_STRING_SUCCESS);
		MessageBox(txt, L"");
		LoadAllUserInfo();
	} else {
		CString txt;
		txt.LoadStringW(IDS_STRING_FAILED);
		MessageBox(txt, L"", MB_ICONERROR);
	}
	OnBnClickedButtonClear();
}


void CUserManagerDlg::OnBnClickedButtonAdd()
{
	UpdateData();
	CString sid, name, phone;
	m_id.GetWindowText(sid);
	int id = _wtoi(sid);
	if (m_curUser && id == m_curUser->get_user_id()) {
		CString txt;
		txt.LoadStringW(IDS_STRING_CLK_CLR_FST);
		MessageBox(txt, L"", MB_ICONINFORMATION);
		OnBnClickedButtonClear();
		return;
	}

	m_name.GetWindowTextW(name);
	CUserManager* mgr = CUserManager::GetInstance();
	if (mgr->UserExists(name)) {
		CString txt;
		txt.LoadStringW(IDS_STRING_USERNAME_EXISTS);
		MessageBox(txt, L"", MB_ICONERROR);
		OnBnClickedButtonClear();
		return;
	}

	m_phone.GetWindowTextW(phone);
	int ndx = m_priority.GetCurSel();
	if (ndx < 0)
		return;

	UserPriority priority = UP_OPERATOR;
	if (ndx == 1)
		priority = UP_ADMIN;
	//else if (ndx == 2)
	//	priority = UP_SUPER;
	else
		priority = UP_OPERATOR;

	CUserInfo user;
	user.set_user_id(id);
	user.set_user_name(name);
	user.set_user_passwd(L"123456");
	user.set_user_phone(phone);
	user.set_user_priority(priority);

	
	BOOL ok = mgr->AddUser(user);
	if (ok) {
		CString txt;
		txt.LoadStringW(IDS_STRING_SUCCESS);
		MessageBox(txt, L"");
		txt.LoadStringW(IDS_STRING_HURRY_CHANGE_PASSWD);
		MessageBox(txt, L"", MB_ICONINFORMATION);
		LoadAllUserInfo();
	} else {
		CString txt;
		txt.LoadStringW(IDS_STRING_FAILED);
		MessageBox(txt, L"", MB_ICONERROR);
	}
	OnBnClickedButtonClear();
}


void CUserManagerDlg::OnBnClickedButtonDelete()
{
	if (m_curUser == NULL) {
		return;
	}

	if (m_curUser->get_user_id() == 0) {
		CString txt;
		txt.LoadStringW(IDS_STRING_CANT_DEL_SUPER);
		MessageBox(txt, L"", MB_ICONERROR);
		return;
	}

	CUserManager* mgr = CUserManager::GetInstance();
	BOOL ok = mgr->DeleteUser(m_curUser);
	if (ok) {
		CString txt;
		txt.LoadStringW(IDS_STRING_SUCCESS);
		MessageBox(txt, L"");
		LoadAllUserInfo();
	} else {
		CString txt;
		txt.LoadStringW(IDS_STRING_FAILED);
		MessageBox(txt, L"", MB_ICONERROR);
	}
	OnBnClickedButtonClear();
}


void CUserManagerDlg::OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	const CUserInfo* user = reinterpret_cast<const CUserInfo*>(pNMLV->lParam);
	assert(user);
	m_curUser = user;
	CString id;
	id.Format(L"%d", user->get_user_id());
	m_id.SetWindowTextW(id);
	m_name.SetWindowTextW(user->get_user_name());
	m_phone.SetWindowTextW(user->get_user_phone());
	CString super;
	super.LoadStringW(IDS_STRING_USER_SUPER);
	switch (user->get_user_priority()) {
		case UP_SUPER:
			m_priority.SetCurSel(-1);
			m_priority.SetWindowTextW(super);
			break;
		case UP_ADMIN:
			m_priority.SetCurSel(1);
			break;
		default:
			m_priority.SetCurSel(0);
			break;
	}

	BOOL canEdit = m_curUser->get_user_id() != 0;
	m_name.EnableWindow();
	m_phone.EnableWindow();
	m_priority.EnableWindow(canEdit);
	m_btnDelete.EnableWindow(canEdit);
	m_btnUpdate.EnableWindow();
	m_btnAdd.EnableWindow(0);

	*pResult = 0;
}


BOOL CUserManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString /*super, */admin, operater;
	//super.LoadStringW(IDS_STRING_USER_SUPER);
	admin.LoadStringW(IDS_STRING_USER_ADMIN);
	operater.LoadStringW(IDS_STRING_USER_OPERATOR);
	int ndx = -1;
	m_priority.InsertString(++ndx, operater);
	m_priority.InsertString(++ndx, admin);
	//m_priority.InsertString(++ndx, super);

	CString name, /*passwd, */phone, priority;
	name.LoadStringW(IDS_STRING_USER_NAME);
	//passwd.LoadStringW(IDS_STRING_USER_PASSWD);
	phone.LoadStringW(IDS_STRING_PHONE);
	priority.LoadStringW(IDS_STRING_PRIORITY);

	DWORD dwStyle = m_list.GetExtendedStyle(); //��ȡ��ǰ��չ��ʽ
	dwStyle |= LVS_EX_FULLROWSELECT; //ѡ��ĳ��ʹ���и�����report���ʱ��
	dwStyle |= LVS_EX_GRIDLINES; //�����ߣ�report���ʱ��
	m_list.SetExtendedStyle(dwStyle); //������չ���

	int i = -1;
	m_list.InsertColumn(++i, _T("ID"),	LVCFMT_LEFT, 50, -1);
	m_list.InsertColumn(++i, name,		LVCFMT_LEFT, 100, -1);
	//m_list.InsertColumn(++i, passwd, LVCFMT_LEFT, 100, -1);
	m_list.InsertColumn(++i, phone,		LVCFMT_LEFT, 150, -1);
	m_list.InsertColumn(++i, priority,	LVCFMT_LEFT, 80, -1);

	LoadAllUserInfo();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CUserManagerDlg::Insert2List(const CUserInfo* user)
{
	int nResult = -1;
	LV_ITEM lvitem = { 0 };
	CString tmp = _T("");

	lvitem.lParam = user->get_user_id();
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = user->get_user_id();
	lvitem.iSubItem = 0;

	// ID
	tmp.Format(_T("%d"), user->get_user_id());
	lvitem.pszText = tmp.GetBuffer(tmp.GetLength());
	tmp.ReleaseBuffer();
	nResult = m_list.InsertItem(&lvitem);

	if (nResult != -1) {
		// �û���
		lvitem.iItem = nResult;
		lvitem.iSubItem++;
		tmp = user->get_user_name();
		lvitem.pszText = tmp.GetBuffer(tmp.GetLength());
		tmp.ReleaseBuffer();
		m_list.SetItem(&lvitem);

		// �ֻ�
		lvitem.iSubItem++;
		tmp = user->get_user_phone();
		lvitem.pszText = tmp.GetBuffer(tmp.GetLength());
		tmp.ReleaseBuffer();
		m_list.SetItem(&lvitem);

		// Ȩ��
		switch (user->get_user_priority()) {
			case UP_SUPER:
				tmp.LoadStringW(IDS_STRING_USER_SUPER);
				break;
			case UP_ADMIN:
				tmp.LoadStringW(IDS_STRING_USER_ADMIN);
				break;
			case UP_OPERATOR:
			default:
				tmp.LoadStringW(IDS_STRING_USER_OPERATOR);
				break;
		}
		lvitem.iSubItem++;
		lvitem.pszText = tmp.GetBuffer(tmp.GetLength());
		tmp.ReleaseBuffer();
		m_list.SetItem(&lvitem);

		m_list.SetItemData(nResult, (DWORD_PTR)user);
	}
}


void CUserManagerDlg::LoadAllUserInfo()
{
	m_list.DeleteAllItems();
	m_curUser = NULL;
	CUserManager* mgr = CUserManager::GetInstance();
	CUserInfo* user = mgr->GetFirstUserInfo();
	while (user) {
		Insert2List(user);
		user = mgr->GetNextUserInfo();
	}
}


void CUserManagerDlg::OnBnClickedButtonChangePasswd()
{
	if (m_curUser == NULL)
		return;
	
	CString passwd;
	m_passwd.GetWindowTextW(passwd);
	if (passwd.IsEmpty())
		return;

	CUserManager* mgr = CUserManager::GetInstance();
	BOOL ok = mgr->ChangeUserPasswd(m_curUser, passwd);
	if (ok) {
		CString txt;
		txt.LoadStringW(IDS_STRING_SUCCESS);
		MessageBox(txt, L"");
		LoadAllUserInfo();
	} else {
		CString txt;
		txt.LoadStringW(IDS_STRING_FAILED);
		MessageBox(txt, L"", MB_ICONERROR);
	}
	OnBnClickedButtonClear();
}