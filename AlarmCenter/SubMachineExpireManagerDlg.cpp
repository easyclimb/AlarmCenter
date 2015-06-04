// SubMachineExpireManagerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "SubMachineExpireManagerDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "ZoneInfo.h"
#include "ExtendExpireTimeDlg.h"

using namespace core;

// CSubMachineExpireManagerDlg 对话框

IMPLEMENT_DYNAMIC(CSubMachineExpireManagerDlg, CDialogEx)

CSubMachineExpireManagerDlg::CSubMachineExpireManagerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSubMachineExpireManagerDlg::IDD, pParent)
	, m_machine(NULL)
{

}

CSubMachineExpireManagerDlg::~CSubMachineExpireManagerDlg()
{
}

void CSubMachineExpireManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
}


BEGIN_MESSAGE_MAP(CSubMachineExpireManagerDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CSubMachineExpireManagerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CSubMachineExpireManagerDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_EXTEND, &CSubMachineExpireManagerDlg::OnBnClickedButtonExtend)
	ON_BN_CLICKED(IDC_BUTTON_ALL, &CSubMachineExpireManagerDlg::OnBnClickedButtonAll)
	ON_BN_CLICKED(IDC_BUTTON_ALL_NOT, &CSubMachineExpireManagerDlg::OnBnClickedButtonAllNot)
	ON_BN_CLICKED(IDC_BUTTON_INVERT, &CSubMachineExpireManagerDlg::OnBnClickedButtonInvert)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST1, &CSubMachineExpireManagerDlg::OnNMCustomdrawList1)
END_MESSAGE_MAP()


// CSubMachineExpireManagerDlg 消息处理程序


void CSubMachineExpireManagerDlg::OnBnClickedOk() 
{
	CDialogEx::OnOK();
}


void CSubMachineExpireManagerDlg::OnBnClickedCancel() 
{
	CDialogEx::OnCancel();
}


void CSubMachineExpireManagerDlg::OnBnClickedButtonExtend() 
{
	if (m_list.GetSelectedCount() == 0)
		return;

	CExtendExpireTimeDlg dlg;
	if (dlg.DoModal() != IDOK)
		return;

	int ndx = -1;
	for (UINT i = 0; i < m_list.GetSelectedCount(); i++) {
		ndx = m_list.GetNextItem(ndx, LVNI_SELECTED);
		if (ndx == -1)
			break;
		DWORD data = m_list.GetItemData(ndx);
		CAlarmMachine* subMachine = reinterpret_cast<CAlarmMachine*>(data);
		if (subMachine && subMachine->execute_update_expire_time(dlg.m_dateTime)) {
			m_list.SetItemText(ndx, 2, dlg.m_dateTime.Format(L"%Y-%m-%d %H:%M:%S"));
		}
	}
}


BOOL CSubMachineExpireManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	DWORD dwStyle = m_list.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
	m_list.SetExtendedStyle(dwStyle);
	int i = -1;
	CString fm;
	fm.LoadStringW(IDS_STRING_SUBMACHINE);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 50, -1);
	fm.LoadStringW(IDS_STRING_ALIAS);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 200, -1);
	fm.LoadStringW(IDS_STRING_EXPIRE_TIME);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 1000, -1);

	if (!m_machine)
		return FALSE;

	CZoneInfoList list;
	m_machine->GetAllZoneInfo(list);
	CZoneInfoListIter iter = list.begin();
	while (iter != list.end()) {
		CZoneInfo* zoneInfo = *iter++;
		CAlarmMachine* subMachine = zoneInfo->GetSubMachineInfo();
		if (subMachine) {
			InsertList(subMachine);
		}
	}


	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}


void CSubMachineExpireManagerDlg::InsertList(const core::CAlarmMachine* subMachineInfo)
{
	assert(subMachineInfo);
	int nResult = -1;
	LV_ITEM lvitem = { 0 };
	CString tmp = _T("");

	lvitem.lParam = 0;
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = m_list.GetItemCount();
	lvitem.iSubItem = 0;

	// submachine zone value
	tmp.Format(_T("%03d"), subMachineInfo->get_submachine_zone());
	lvitem.pszText = tmp.LockBuffer();
	nResult = m_list.InsertItem(&lvitem);
	tmp.UnlockBuffer();

	if (nResult != -1) {
		// alias
		lvitem.iItem = nResult;
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), subMachineInfo->get_alias());
		lvitem.pszText = tmp.LockBuffer();
		m_list.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// expire time
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), subMachineInfo->get_expire_time().Format(L"%Y-%m-%d %H:%M:%S"));
		lvitem.pszText = tmp.LockBuffer();
		m_list.SetItem(&lvitem);
		tmp.UnlockBuffer();

		m_list.SetItemData(nResult, reinterpret_cast<DWORD_PTR>(subMachineInfo));
	}
}


void CSubMachineExpireManagerDlg::OnBnClickedButtonAll()
{
	for (int i = 0; i < m_list.GetItemCount(); i++) {
		m_list.SetItemState(i, LVIS_FOCUSED | LVIS_SELECTED,
							LVIS_FOCUSED | LVIS_SELECTED);
	}
	m_list.SetFocus();
}


void CSubMachineExpireManagerDlg::OnBnClickedButtonAllNot() 
{
	for (int i = 0; i < m_list.GetItemCount(); i++) {
		m_list.SetItemState(i, 0, LVIS_FOCUSED | LVIS_SELECTED);
	}
	m_list.SetFocus();
}


void CSubMachineExpireManagerDlg::OnBnClickedButtonInvert()
{
	for (int i = 0; i < m_list.GetItemCount(); i++) {
		if (m_list.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED) {
			m_list.SetItemState(i, 0, LVIS_FOCUSED | LVIS_SELECTED);
		} else {
			m_list.SetItemState(i, LVIS_FOCUSED | LVIS_SELECTED,
								LVIS_FOCUSED | LVIS_SELECTED);
		}
	}
	m_list.SetFocus();
}


void CSubMachineExpireManagerDlg::OnNMCustomdrawList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
	*pResult = CDRF_DODEFAULT;

	if (CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage) {
		*pResult = CDRF_NOTIFYITEMDRAW;
	} else if (CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage) {
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
	} else if ((CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage) {
		COLORREF clrNewTextColor, clrNewBkColor;
		int nItem = static_cast<int>(pLVCD->nmcd.dwItemSpec);
		if (m_list.GetItemState(nItem, LVIS_SELECTED) == LVIS_SELECTED) {
			clrNewTextColor = RGB(255, 255, 255);        //Set the text to white
			clrNewBkColor = RGB(49, 106, 197);        //Set the background color to blue
		} else {
			clrNewTextColor = RGB(0, 0, 0);        //set the text black
			clrNewBkColor = RGB(255, 255, 255);    //leave the background color white
		}
		pLVCD->clrText = clrNewTextColor;
		pLVCD->clrTextBk = clrNewBkColor;
		*pResult = CDRF_DODEFAULT;
	}
}
