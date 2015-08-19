// AddMachineDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AddMachineDlg.h"
#include "afxdialogex.h"
#include "GroupInfo.h"
#include "AlarmMachineManager.h"
#include "AlarmMachine.h"
#include "ademco_func.h"


using namespace core;
static const int COMBO_NDX_NO = 0;
static const int COMBO_NDX_YES = 1;

static const int COMBO_NDX_MAP = 0;
static const int COMBO_NDX_VIDEO = 1;
// CAddMachineDlg dialog

IMPLEMENT_DYNAMIC(CAddMachineDlg, CDialogEx)

CAddMachineDlg::CAddMachineDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAddMachineDlg::IDD, pParent)
	, m_machine(NULL)
{

}

CAddMachineDlg::~CAddMachineDlg()
{
}

void CAddMachineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_ademco_id);
	DDX_Control(pDX, IDC_COMBO1, m_banned);
	DDX_Control(pDX, IDC_COMBO_TYPE, m_type);
	//DDX_Control(pDX, IDC_EDIT2, m_device_id);
	DDX_Control(pDX, IDC_EDIT4, m_alias);
	DDX_Control(pDX, IDC_EDIT5, m_contact);
	DDX_Control(pDX, IDC_EDIT6, m_address);
	DDX_Control(pDX, IDC_EDIT7, m_phone);
	DDX_Control(pDX, IDC_EDIT8, m_phone_bk);
	DDX_Control(pDX, IDC_COMBO2, m_group);
	DDX_Control(pDX, IDC_STATIC_NOTE2, m_note);
	DDX_Control(pDX, IDOK, m_ok);
	DDX_Control(pDX, IDC_EDIT9, m_expire_time);
	DDX_Control(pDX, IDC_COMBO3, m_cmb_ademco_id);
}


BEGIN_MESSAGE_MAP(CAddMachineDlg, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT1, &CAddMachineDlg::OnEnChangeEditAdemcoID)
	ON_BN_CLICKED(IDOK, &CAddMachineDlg::OnBnClickedOk)
	ON_EN_KILLFOCUS(IDC_EDIT2, &CAddMachineDlg::OnEnKillfocusEditDeviceID)
	ON_CBN_SELCHANGE(IDC_COMBO3, &CAddMachineDlg::OnCbnSelchangeCombo3)
	ON_CBN_EDITCHANGE(IDC_COMBO3, &CAddMachineDlg::OnCbnEditchangeCombo3)
END_MESSAGE_MAP()


// CAddMachineDlg message handlers


BOOL CAddMachineDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString yes, no;
	yes.LoadStringW(IDS_STRING_YES);
	no.LoadStringW(IDS_STRING_NO);
	int combo_ndx = -1;
	combo_ndx = m_banned.InsertString(COMBO_NDX_NO, no);
	ASSERT(combo_ndx == COMBO_NDX_NO);
	combo_ndx = m_banned.InsertString(COMBO_NDX_YES, yes);
	ASSERT(combo_ndx == COMBO_NDX_YES);
	m_banned.SetCurSel(COMBO_NDX_NO);

	CString normal, video;
	normal.LoadStringW(IDS_STRING_TYPE_MAP);
	video.LoadStringW(IDS_STRING_TYPE_VIDEO);
	combo_ndx = m_type.InsertString(COMBO_NDX_MAP, normal);
	ASSERT(combo_ndx == COMBO_NDX_MAP);
	combo_ndx = m_type.InsertString(COMBO_NDX_VIDEO, video);
	ASSERT(combo_ndx == COMBO_NDX_VIDEO);
	m_type.SetCurSel(COMBO_NDX_MAP);

	CGroupInfoList list;
	CGroupManager* mgr = CGroupManager::GetInstance();
	CGroupInfo* rootGroup = mgr->GetRootGroupInfo();

	int ndx = m_group.InsertString(0, rootGroup->get_name());
	m_group.SetItemData(ndx, rootGroup->get_id());

	rootGroup->GetDescendantGroups(list);
	CGroupInfoListIter iter = list.begin();
	while (iter != list.end()) {
		CGroupInfo* group = *iter++;
		ndx = m_group.AddString(group->get_name());
		m_group.SetItemData(ndx, group->get_id());
	}
	m_group.SetCurSel(0);

	SYSTEMTIME st = { 0 };
	GetLocalTime(&st);
	st.wYear++;
	st.wMinute = 0;
	st.wSecond = 0;
	COleDateTime now(st);
	m_expire_time.SetWindowTextW(now.Format(L"%Y-%m-%d %H:%M:%S"));

	m_machine = new CAlarmMachine();
	m_machine->set_expire_time(now);

	//m_ok.EnableWindow(0);

	CAlarmMachineManager* machine_mgr = CAlarmMachineManager::GetInstance();
	CString txt;
	for (int i = 0; i < MAX_MACHINE; i++)  {
		if (machine_mgr->CheckIfMachineAdemcoIdCanUse(i)) {
			txt.Format(L"%04d", i);
			int ndx = m_cmb_ademco_id.InsertString(-1, txt);
			m_cmb_ademco_id.SetItemData(ndx, i);
		}
	}
	m_cmb_ademco_id.SetCurSel(0);
	OnCbnSelchangeCombo3();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CAddMachineDlg::OnEnChangeEditAdemcoID()
{
	CheckAdemcoID();
}


void CAddMachineDlg::OnEnKillfocusEditDeviceID()
{
	CheckDeviceID();
}


bool CAddMachineDlg::CheckAdemcoID()
{
	/*CString s;
	m_ademco_id.GetWindowTextW(s);
	if (s.IsEmpty()) {
		m_note.SetWindowTextW(L"");
		m_ok.EnableWindow(0);
		return false;
	}*/
	USES_CONVERSION;

	int ademco_id = -1;
	int ndx = m_cmb_ademco_id.GetCurSel();
	if (ndx < 0) {
		CString num;
		m_cmb_ademco_id.GetWindowTextW(num);
		//ademco_id = ademco::HexCharArrayToDec(W2A(num), num.GetLength());
		ademco_id = _ttoi(num);
		//m_note.SetWindowTextW(L"");
		//m_ok.EnableWindow(0);
		//return false;
	} else {
		ademco_id = m_cmb_ademco_id.GetItemData(ndx);
	}

	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (!mgr->CheckIfMachineAdemcoIdCanUse(ademco_id)) {
		CString s; s.LoadStringW(IDS_STRING_ERR_AID);
		m_note.SetWindowTextW(s);
		m_ok.EnableWindow(0);
		return false;
	}

	m_note.SetWindowTextW(L"");
	m_ok.EnableWindow();
	m_machine->set_ademco_id(ademco_id);
	return true;
}


bool CAddMachineDlg::CheckDeviceID()
{
	//CString s;
	//m_device_id.GetWindowTextW(s);
	//if (s.IsEmpty()) {
	//	m_note.SetWindowTextW(L"");
	//	m_ok.EnableWindow(0);
	//	return false;
	//}

	//CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	///*if (mgr->CheckIfMachineAcctAlreadyInuse(s)) {
	//	s.LoadStringW(IDS_STRING_ACCT_NOT_UNIQUE);
	//	m_note.SetWindowTextW(s);
	//	m_ok.EnableWindow(0);
	//	return false;
	//}*/

	//m_note.SetWindowTextW(L"");
	//m_ok.EnableWindow();
	////m_machine->set_device_id(s);
	return true;
}


void CAddMachineDlg::OnBnClickedOk()
{
	if (!CheckAdemcoID() || !CheckDeviceID())
		return;

	int ndx = m_banned.GetCurSel();
	if (ndx < 0)		return;
	bool banned = ndx == COMBO_NDX_YES;
	m_machine->set_banned(banned);

	ndx = m_type.GetCurSel();
	if (ndx < 0)		return;
	m_machine->set_has_video(ndx == COMBO_NDX_VIDEO);

	CString s;
	m_alias.GetWindowTextW(s);
	m_machine->set_alias(s);

	m_contact.GetWindowTextW(s);
	m_machine->set_contact(s);

	m_address.GetWindowTextW(s);
	m_machine->set_address(s);

	m_phone.GetWindowTextW(s);
	m_machine->set_phone(s);

	m_phone_bk.GetWindowTextW(s);
	m_machine->set_phone_bk(s);

	ndx = m_group.GetCurSel();
	if (ndx < 0)	return;
	m_machine->set_group_id(m_group.GetItemData(ndx));

	m_machine->set_machine_type(MT_UNKNOWN);
	
	CDialogEx::OnOK();
}


void CAddMachineDlg::OnCancel()
{
	SAFEDELETEP(m_machine);
	CDialogEx::OnCancel();
}




void CAddMachineDlg::OnCbnSelchangeCombo3()
{
	int ndx = m_cmb_ademco_id.GetCurSel();
	if (ndx < 0)
		return;

	int ademco_id = m_cmb_ademco_id.GetItemData(ndx);
	CString s, f; f.LoadStringW(IDS_STRING_HEX);
	s.Format(L"%s%04X", f, ademco_id);
	m_note.SetWindowTextW(s);
}


void CAddMachineDlg::OnCbnEditchangeCombo3()
{
	USES_CONVERSION;
	CString t; m_cmb_ademco_id.GetWindowTextW(t);
	if (t.IsEmpty())
		return;
	//int ademco_id = ademco::HexCharArrayToDec(W2A(t), t.GetLength());
	int ademco_id = _ttoi(t);
	if (CheckAdemcoID()) {
		CString s, f; f.LoadStringW(IDS_STRING_HEX);
		s.Format(L"%s%04X", f, ademco_id);
		m_note.SetWindowTextW(s);
	}
}
