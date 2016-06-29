// ConsumerTypeMgrDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "ConsumerTypeMgrDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "InputGroupNameDlg.h"

// CConsumerTypeMgrDlg dialog

namespace detail {

auto insert_to_list = [](CListCtrl& ctrl, const core::consumer_type_ptr& type) {
	int nResult = -1;
	LV_ITEM lvitem = { 0 };
	CString tmp = _T("");

	lvitem.lParam = type->id;
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = ctrl.GetItemCount();
	lvitem.iSubItem = 0;

	//ÐòºÅ
	tmp.Format(_T("%d"), type->id);
	lvitem.pszText = tmp.LockBuffer();
	nResult = ctrl.InsertItem(&lvitem);
	tmp.UnlockBuffer();

	if (nResult != -1) {
		// ÀàÐÍ
		lvitem.iSubItem++;
		tmp = type->name;
		lvitem.pszText = tmp.LockBuffer();
		ctrl.SetItem(&lvitem);
		tmp.UnlockBuffer();
	}

	ctrl.SetItemData(nResult, type->id);
};

};

IMPLEMENT_DYNAMIC(CConsumerTypeMgrDlg, CDialogEx)

CConsumerTypeMgrDlg::CConsumerTypeMgrDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_CONSUMER_TYPE_MGR, pParent)
{

}

CConsumerTypeMgrDlg::~CConsumerTypeMgrDlg()
{
}

void CConsumerTypeMgrDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
}


BEGIN_MESSAGE_MAP(CConsumerTypeMgrDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CConsumerTypeMgrDlg::OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE, &CConsumerTypeMgrDlg::OnBnClickedButtonUpdate)
END_MESSAGE_MAP()


// CConsumerTypeMgrDlg message handlers

BOOL CConsumerTypeMgrDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	DWORD dwStyle = m_list.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
	m_list.SetExtendedStyle(dwStyle);

	int i = -1;
	m_list.InsertColumn(++i, GetStringFromAppResource(IDS_STRING_INDEX), LVCFMT_LEFT, 50, -1);
	m_list.InsertColumn(++i, GetStringFromAppResource(IDS_STRING_TYPE), LVCFMT_LEFT, 200, -1);

	auto mgr = core::consumer_manager::get_instance();
	auto types = mgr->get_all_types();
	for (auto type : types) {
		detail::insert_to_list(m_list, type.second);
	}
	

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CConsumerTypeMgrDlg::OnBnClickedButtonAdd()
{
	CInputContentDlg dlg(this);
	dlg.m_title = GetStringFromAppResource(IDS_STRING_INPUT_TYPE);
	int ret = dlg.DoModal();
	if (ret != IDOK) return;
	auto mgr = core::consumer_manager::get_instance();
	int id;
	if (mgr->execute_add_type(id, dlg.m_value)) {
		auto type = mgr->get_consumer_type_by_id(id);
		assert(type);
		if (type) {
			detail::insert_to_list(m_list, type);
		}
	}
}


void CConsumerTypeMgrDlg::OnBnClickedButtonUpdate()
{
	auto pos = m_list.GetFirstSelectedItemPosition();
	if (pos == nullptr)return;

	int ndx = m_list.GetNextItem(-1, LVNI_SELECTED);
	if (ndx < 0)return;
	int data = static_cast<int>(m_list.GetItemData(ndx));

	CInputContentDlg dlg;
	dlg.m_title = GetStringFromAppResource(IDS_STRING_INPUT_TYPE);
	int ret = dlg.DoModal();
	if (ret != IDOK) return;
	auto mgr = core::consumer_manager::get_instance();
	auto type = mgr->get_consumer_type_by_id(data); assert(type);
	if (dlg.m_value.IsEmpty() || type->name == dlg.m_value) return;

	if (mgr->execute_rename(data, dlg.m_value)) {
		LV_ITEM lvitem = { 0 };

		lvitem.lParam = 0;
		lvitem.mask = LVIF_TEXT;
		lvitem.iItem = ndx;
		lvitem.iSubItem = 1;

		//m_list.GetItem(&lvitem);
		lvitem.pszText = dlg.m_value.LockBuffer(); 
		m_list.SetItem(&lvitem);
		dlg.m_value.UnlockBuffer();
		
	}

}



