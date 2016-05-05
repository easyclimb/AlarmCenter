// ConsumerTypeMgrDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "ConsumerTypeMgrDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"

// CConsumerTypeMgrDlg dialog

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

	auto mgr = core::consumer_manager::GetInstance();
	auto types = mgr->get_all_types();
	for (auto type : types) {
		int nResult = -1;
		LV_ITEM lvitem = { 0 };
		CString tmp = _T("");

		lvitem.lParam = 0;
		lvitem.mask = LVIF_TEXT;
		lvitem.iItem = m_list.GetItemCount();
		lvitem.iSubItem = 0;

		//ÐòºÅ
		tmp.Format(_T("%d"), type.first);
		lvitem.pszText = tmp.LockBuffer();
		nResult = m_list.InsertItem(&lvitem);
		tmp.UnlockBuffer();

		if (nResult != -1) {
			// ÀàÐÍ
			lvitem.iSubItem++;
			tmp = type.second->name;
			lvitem.pszText = tmp.LockBuffer();
			m_list.SetItem(&lvitem);
			tmp.UnlockBuffer();
		}

		m_list.SetItemData(nResult, type.first);
	}
	

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CConsumerTypeMgrDlg::OnBnClickedButtonAdd()
{
	// TODO: Add your control notification handler code here
}


void CConsumerTypeMgrDlg::OnBnClickedButtonUpdate()
{
	// TODO: Add your control notification handler code here
}



