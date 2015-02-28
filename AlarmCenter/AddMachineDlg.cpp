// AddMachineDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AddMachineDlg.h"
#include "afxdialogex.h"
#include "GroupInfo.h"
#include "AlarmMachineManager.h"


using namespace core;
static const int COMBO_NDX_NO = 0;
static const int COMBO_NDX_YES = 1;

static const int COMBO_NDX_MAP = 0;
static const int COMBO_NDX_VIDEO = 1;
// CAddMachineDlg dialog

IMPLEMENT_DYNAMIC(CAddMachineDlg, CDialogEx)

CAddMachineDlg::CAddMachineDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAddMachineDlg::IDD, pParent)
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
	DDX_Control(pDX, IDC_EDIT2, m_device_id);
	DDX_Control(pDX, IDC_EDIT4, m_alias);
	DDX_Control(pDX, IDC_EDIT5, m_contact);
	DDX_Control(pDX, IDC_EDIT6, m_address);
	DDX_Control(pDX, IDC_EDIT7, m_phone);
	DDX_Control(pDX, IDC_EDIT8, m_phone_bk);
	DDX_Control(pDX, IDC_COMBO2, m_group);
	DDX_Control(pDX, IDC_STATIC_NOTE, m_note);
	DDX_Control(pDX, IDOK, m_ok);
}


BEGIN_MESSAGE_MAP(CAddMachineDlg, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT1, &CAddMachineDlg::OnEnChangeEditAdemcoID)
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

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CAddMachineDlg::OnEnChangeEditAdemcoID()
{
	CString s;
	m_ademco_id.GetWindowTextW(s);
	if (s.IsEmpty()) {
		m_note.SetWindowTextW(L"");
		return;
	}

	int ademco_id = _ttoi(s);
	if (ademco_id < 0 || MAX_MACHINE <= ademco_id) {
		s.LoadStringW(IDS_STRING_ERR_AID);
		m_note.SetWindowTextW(s);
		return;
	}

	m_note.SetWindowTextW(L"");
}
