// SubMachineExpireManagerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "SubMachineExpireManagerDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "ZoneInfo.h"
#include "ExtendExpireTimeDlg.h"
#include <iterator>
#include <odbcinst.h>
#include <afxdb.h>
#include <comdef.h>
using namespace core;

// CMachineExpireManagerDlg 对话框

IMPLEMENT_DYNAMIC(CMachineExpireManagerDlg, CDialogEx)

CMachineExpireManagerDlg::CMachineExpireManagerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CMachineExpireManagerDlg::IDD, pParent)
	//, m_machine(nullptr)
{

}

CMachineExpireManagerDlg::~CMachineExpireManagerDlg()
{
}

void CMachineExpireManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Control(pDX, IDC_STATIC_LINE_NUM, m_staticSeldLineNum);
}


BEGIN_MESSAGE_MAP(CMachineExpireManagerDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CMachineExpireManagerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CMachineExpireManagerDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_EXTEND, &CMachineExpireManagerDlg::OnBnClickedButtonExtend)
	ON_BN_CLICKED(IDC_BUTTON_ALL, &CMachineExpireManagerDlg::OnBnClickedButtonAll)
	ON_BN_CLICKED(IDC_BUTTON_ALL_NOT, &CMachineExpireManagerDlg::OnBnClickedButtonAllNot)
	ON_BN_CLICKED(IDC_BUTTON_INVERT, &CMachineExpireManagerDlg::OnBnClickedButtonInvert)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, &CMachineExpireManagerDlg::OnNMClickList1)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT_SEL, &CMachineExpireManagerDlg::OnBnClickedButtonExportSel)
	ON_BN_CLICKED(IDC_BUTTON_PRINT_SEL, &CMachineExpireManagerDlg::OnBnClickedButtonPrintSel)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST1, &CMachineExpireManagerDlg::OnLvnColumnclickList1)
	ON_NOTIFY(HDN_ITEMCLICK, 0, &CMachineExpireManagerDlg::OnHdnItemclickList1)
END_MESSAGE_MAP()


// CMachineExpireManagerDlg 消息处理程序


void CMachineExpireManagerDlg::OnBnClickedOk() 
{
	CDialogEx::OnOK();
}


void CMachineExpireManagerDlg::OnBnClickedCancel() 
{
	CDialogEx::OnCancel();
}


void CMachineExpireManagerDlg::SetExpiredMachineList(std::list<core::CAlarmMachine*>& list)
{
	std::copy(list.begin(), list.end(), std::back_inserter(m_expiredMachineList));
}


void CMachineExpireManagerDlg::OnBnClickedButtonExtend() 
{
	if (m_list.GetSelectedCount() == 0)
		return;

	CExtendExpireTimeDlg dlg;
	if (dlg.DoModal() != IDOK)
		return;

	CString syes, sno; syes.LoadStringW(IDS_STRING_YES); sno.LoadStringW(IDS_STRING_NO);
	int ndx = -1;
	for (UINT i = 0; i < m_list.GetSelectedCount(); i++) {
		ndx = m_list.GetNextItem(ndx, LVNI_SELECTED);
		if (ndx == -1)
			break;
		DWORD data = m_list.GetItemData(ndx);
		CAlarmMachine* machine = reinterpret_cast<CAlarmMachine*>(data);
		if (machine && machine->execute_update_expire_time(dlg.m_dateTime)) {
			m_list.SetItemText(ndx, 2, dlg.m_dateTime.Format(L"%Y-%m-%d %H:%M:%S"));
			m_list.SetItemText(ndx, 3, machine->get_left_service_time() <= 0 ? syes : sno);
		}
	}
}


BOOL CMachineExpireManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	DWORD dwStyle = m_list.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
	m_list.SetExtendedStyle(dwStyle);
	int i = -1;
	CString fm;
	fm.LoadStringW(IDS_STRING_MACHINE);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 50, -1);
	fm.LoadStringW(IDS_STRING_ALIAS);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 200, -1);
	fm.LoadStringW(IDS_STRING_EXPIRE_TIME);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 125, -1);
	fm.LoadStringW(IDS_STRING_IF_EXPIRE);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 75, -1);
	fm.LoadStringW(IDS_STRING_CONTACT);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 75, -1);
	fm.LoadStringW(IDS_STRING_ADDRESS);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 225, -1);
	fm.LoadStringW(IDS_STRING_PHONE);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 150, -1);
	fm.LoadStringW(IDS_STRING_PHONE_BK);
	m_list.InsertColumn(++i, fm, LVCFMT_LEFT, 150, -1);


	for (auto machine : m_expiredMachineList) {
		InsertList(machine);
	}


	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}


void CMachineExpireManagerDlg::InsertList(const core::CAlarmMachine* machine)
{
	assert(machine);
	int nResult = -1;
	LV_ITEM lvitem = { 0 };
	CString tmp = _T("");

	lvitem.lParam = 0;
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = m_list.GetItemCount();
	lvitem.iSubItem = 0;

	// ndx
	if (machine->get_is_submachine()) {
		// submachine zone value
		tmp.Format(_T("%03d"), machine->get_submachine_zone());
	} else {
		// machine ademco id
		tmp.Format(_T("%04d"), machine->get_ademco_id());
	}
	lvitem.pszText = tmp.LockBuffer();
	nResult = m_list.InsertItem(&lvitem);
	tmp.UnlockBuffer();

	if (nResult != -1) {
		// alias
		lvitem.iItem = nResult;
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), machine->get_alias());
		lvitem.pszText = tmp.LockBuffer();
		m_list.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// expire time
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), machine->get_expire_time().Format(L"%Y-%m-%d %H:%M:%S"));
		lvitem.pszText = tmp.LockBuffer();
		m_list.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// if expire 
		CString syes, sno; syes.LoadStringW(IDS_STRING_YES); sno.LoadStringW(IDS_STRING_NO);
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), machine->get_left_service_time() <= 0 ? syes : sno);
		lvitem.pszText = tmp.LockBuffer();
		m_list.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// contact
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), machine->get_contact());
		lvitem.pszText = tmp.LockBuffer();
		m_list.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// address
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), machine->get_address());
		lvitem.pszText = tmp.LockBuffer();
		m_list.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// phone
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), machine->get_phone());
		lvitem.pszText = tmp.LockBuffer();
		m_list.SetItem(&lvitem);
		tmp.UnlockBuffer();

		// phone_bk
		lvitem.iSubItem++;
		tmp.Format(_T("%s"), machine->get_phone_bk());
		lvitem.pszText = tmp.LockBuffer();
		m_list.SetItem(&lvitem);
		tmp.UnlockBuffer();

		m_list.SetItemData(nResult, reinterpret_cast<DWORD_PTR>(machine));
	}
}


void CMachineExpireManagerDlg::OnBnClickedButtonAll()
{
	for (int i = 0; i < m_list.GetItemCount(); i++) {
		m_list.SetItemState(i, LVIS_FOCUSED | LVIS_SELECTED,
							LVIS_FOCUSED | LVIS_SELECTED);
	}
	m_list.SetFocus();
	CString s; s.Format(L"%d", m_list.GetSelectedCount());
	m_staticSeldLineNum.SetWindowTextW(s);
}


void CMachineExpireManagerDlg::OnBnClickedButtonAllNot() 
{
	for (int i = 0; i < m_list.GetItemCount(); i++) {
		m_list.SetItemState(i, 0, LVIS_FOCUSED | LVIS_SELECTED);
	}
	m_list.SetFocus();
	CString s; s.Format(L"%d", m_list.GetSelectedCount());
	m_staticSeldLineNum.SetWindowTextW(s);
}


void CMachineExpireManagerDlg::OnBnClickedButtonInvert()
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
	CString s; s.Format(L"%d", m_list.GetSelectedCount());
	m_staticSeldLineNum.SetWindowTextW(s);
}


void CMachineExpireManagerDlg::OnNMClickList1(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	//LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CString s; s.Format(L"%d", m_list.GetSelectedCount());
	m_staticSeldLineNum.SetWindowTextW(s);
	*pResult = 0;
}


BOOL CMachineExpireManagerDlg::GetSaveAsFilePath(CString& path) {
	static CString prevPath = _T("");
RE_SAVE_AS:
	TCHAR szFilename[MAX_PATH] = { 0 };
	BOOL bResult = FALSE;
	DWORD dwError = NOERROR;
	OPENFILENAME ofn = { 0 };

	ofn.lStructSize = sizeof (OPENFILENAME);
	ofn.lpstrFilter = _T("Excel File(*.xls)\0*.xls\0\0");
	ofn.lpstrFile = szFilename;
	ofn.nMaxFile = MAX_PATH;
	ofn.hwndOwner = GetSafeHwnd();
	ofn.Flags = OFN_EXPLORER |
		OFN_ENABLEHOOK |
		OFN_HIDEREADONLY |
		OFN_NOCHANGEDIR |
		OFN_PATHMUSTEXIST;
	ofn.lpfnHook = nullptr;

	bResult = GetSaveFileName(&ofn);
	if (bResult == FALSE) {
		dwError = CommDlgExtendedError();
		return FALSE;
	}

	CString fileName = szFilename;
	if (CFileOper::GetFileExt(fileName).CompareNoCase(L"xls") != 0)
		fileName += L".xls";

	if (CFileOper::PathExists(fileName)) {
		CString q;
		q.LoadStringW(IDS_STRING_QUERY_REPLACE);
		int ret = MessageBox(q, L"", MB_YESNOCANCEL | MB_ICONQUESTION);
		if (ret == IDYES)
			DeleteFile(fileName);
		else if (ret == IDNO)
			goto RE_SAVE_AS;
		else
			return FALSE;
	}
	prevPath = fileName;
	path = fileName;
	return TRUE;
}


BOOL CMachineExpireManagerDlg::Export(const CString& excelPath) {
	CString warningStr = _T("");
	CDatabase database;
	CString sDriver = _T("");
	CString sSql = _T("");

	// 检索是否安装有Excel驱动 "Microsoft Excel Driver (*.xls)" 
	sDriver = GetExcelDriver();
	if (sDriver.IsEmpty()) {
		// 没有发现Excel驱动
		CString e;
		e.LoadStringW(IDS_STRING_E_NO_EXECEL);
		MessageBox(e, L"", MB_ICONERROR);
		return FALSE;
	}

	// 创建进行存取的字符串
	sSql.Format(_T("DRIVER={%s};DSN='';FIRSTROWHASNAMES=1;READONLY=FALSE;CREATE_DB=\")%s\";DBQ=%s"),
				sDriver, excelPath, excelPath);

	// 创建数据库 (既Excel表格文件)
	if (!database.OpenEx(sSql, CDatabase::noOdbcDialog)) {
		CString e;
		e.LoadStringW(IDS_STRING_E_CREATE_EXCEL);
		MessageBox(e, L"", MB_ICONERROR);
		return FALSE;
	}

	CString sid, salias, sexpire_time, sif_expire, scontact, saddress, sphone, sphone_bk, syes, sno;
	salias.LoadStringW(IDS_STRING_ALIAS);
	sexpire_time.LoadStringW(IDS_STRING_EXPIRE_TIME);
	sif_expire.LoadStringW(IDS_STRING_IF_EXPIRE);
	scontact.LoadStringW(IDS_STRING_CONTACT);
	saddress.LoadStringW(IDS_STRING_ADDRESS);
	sphone.LoadStringW(IDS_STRING_PHONE);
	sphone_bk.LoadStringW(IDS_STRING_PHONE_BK);
	syes.LoadStringW(IDS_STRING_YES);
	sno.LoadStringW(IDS_STRING_NO);

	CString stable;
	stable.Format(L"EXPIRED_MACHINES(Id TEXT,%s TEXT,%s TEXT,%s TEXT,%s TEXT,%s TEXT,%s TEXT, %s TEXT)",
				  salias, sexpire_time, sif_expire, scontact, saddress, sphone, sphone_bk);
	sSql.Format(_T("CREATE TABLE %s"), stable.LockBuffer());
	stable.UnlockBuffer();
	database.ExecuteSQL(sSql);
	
	int nItem = -1;
	CString sinsert, svalues;
	sinsert.Format(L"INSERT INTO EXPIRED_MACHINES(Id,%s,%s,%s,%s,%s,%s,%s) ", 
				   salias, sexpire_time, sif_expire, scontact, saddress, sphone, sphone_bk);
	for (UINT i = 0; i < m_list.GetSelectedCount(); i++) {
		nItem = m_list.GetNextItem(nItem, LVNI_SELECTED);
		if (nItem == -1) break;
		DWORD data = m_list.GetItemData(nItem);
		core::CAlarmMachine* machine = reinterpret_cast<core::CAlarmMachine*>(data);
		if (machine) {
			svalues.Format(_T("VALUES('%d','%s','%s','%s','%s','%s','%s','%s')"), 
						machine->get_is_submachine() ? machine->get_submachine_zone() : machine->get_ademco_id(),
						machine->get_alias(), 
						machine->get_expire_time().Format(L"%Y-%m-%d %H:%M:%S"),
						machine->get_left_service_time() <= 0 ? syes : sno,
						machine->get_contact(),
						machine->get_address(),
						machine->get_phone(),
						machine->get_phone_bk());
			database.ExecuteSQL(sinsert + svalues);
		}
	}

	// 关闭数据库
	database.Close();
	CString fm;
	fm.LoadString(IDS_STRING_FM_EXCEL_OK);
	warningStr.Format(fm, excelPath);
	if (IDYES == MessageBox(warningStr, L"", MB_YESNO | MB_ICONQUESTION)) {
		ShellExecute(nullptr, _T("Open"), excelPath, nullptr, nullptr, SW_SHOW);
	}
	return TRUE;
}



CString CMachineExpireManagerDlg::GetExcelDriver() 
{
	TCHAR szBuf[2001];
	WORD cbBufMax = 2000;
	WORD cbBufOut;
	TCHAR *pszBuf = szBuf;
	CString sDriver = _T("");

	// 获取已安装驱动的名称(涵数在odbcinst.h里)
	if (!SQLGetInstalledDrivers(szBuf, cbBufMax, &cbBufOut))
		return _T("");

	// 检索已安装的驱动是否有Excel...
	do {
		if (_tcsstr(pszBuf, _T("Excel")) != 0) {
			//发现 !
			sDriver = CString(pszBuf);
			break;
		}
		pszBuf = _tcschr(pszBuf, _T('\0')) + 1;
	} while (pszBuf[1] != _T('\0'));

	return sDriver;
}


void CMachineExpireManagerDlg::OnBnClickedButtonExportSel()
{
	AUTO_LOG_FUNCTION;
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos == nullptr) {
		CLog::WriteLog(_T("No items were selected!\n"));
		CString e; e.LoadStringW(IDS_STRING_NO_SELD_CONTENT);
		MessageBox(e, L"", MB_ICONERROR);
		return;
	}

	CString path;
	if (!GetSaveAsFilePath(path))
		return;

	Export(path);
}


BOOL CMachineExpireManagerDlg::PrintRecord(CListCtrl &list) {
	POSITION pos = list.GetFirstSelectedItemPosition();
	if (pos == nullptr) {
		CLog::WriteLog(_T("No items were selected!\n"));
		CString e; e.LoadStringW(IDS_STRING_NO_SELD_CONTENT);
		MessageBox(e, L"", MB_ICONERROR);
		return FALSE;
	}

	HFONT hRecordFont;//记录的字体
	HFONT hTopicFont;//标题的字体
	HFONT hCodeFont;//字段的字体

	//创建(输出内容的)字体
	hRecordFont = CreateFont(93, 29, 1, 0, FW_EXTRALIGHT, 0, 0, 0, ANSI_CHARSET,
							 OUT_DEFAULT_PRECIS,
							 CLIP_DEFAULT_PRECIS,
							 DEFAULT_QUALITY,
							 DEFAULT_PITCH,
							 _T("Arial"));

	//创建标题的字体
	hTopicFont = CreateFont(260, 47, 10, 0, FW_BOLD, 0, 0, 0, ANSI_CHARSET,
							OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,
							DEFAULT_QUALITY,
							DEFAULT_PITCH,
							_T("Arial"));

	//创建字段的字体
	hCodeFont = CreateFont(150, 50, 1, 0, FW_BOLD, 0, 0, 0, ANSI_CHARSET,
						   OUT_DEFAULT_PRECIS,
						   CLIP_DEFAULT_PRECIS,
						   DEFAULT_QUALITY,
						   DEFAULT_PITCH,
						   _T("Arial"));

	//设置打印对话框
	PRINTDLG   pd;
	pd.lStructSize = sizeof(PRINTDLG);
	pd.Flags = PD_RETURNDC;
	pd.hDC = nullptr;
	pd.hwndOwner = nullptr;
	pd.hInstance = nullptr;
	pd.nMaxPage = 2;
	pd.nMinPage = 1;
	pd.nFromPage = 1;
	pd.nToPage = 1;
	pd.nCopies = 1;
	pd.hDevMode = nullptr;
	pd.hDevNames = nullptr;

	/////////////////////////////////////////////////////////
	//显示打印对话框，由用户来设定纸张大小等.
	if (!PrintDlg(&pd))   return   FALSE;
	ASSERT(pd.hDC != nullptr);/*断言获取的句柄不为空.*/
	int   nHorRes = GetDeviceCaps(pd.hDC, HORZRES);
	int   nVerRes = GetDeviceCaps(pd.hDC, VERTRES);
	int   nXMargin = 20;//页边的空白   
	int   nYMargin = 6;

	///////////////////////////////////////////////////////////
	TEXTMETRIC  tm;/*映射结构体*/
	GetTextMetrics(pd.hDC, &tm);
	int   nCharWidth = tm.tmAveCharWidth;
	int   ncaps = (tm.tmPitchAndFamily & 1 ? 3 : 2)*nCharWidth / 2;
	int   nCharHeight = tm.tmExternalLeading + tm.tmHeight + ncaps;//

	///////////////////////////////////////////////////////////
	CHeaderCtrl*   pHeader = list.GetHeaderCtrl();
	//获得行，列的个数
	int   nColCount = pHeader->GetItemCount();//获取列头的个数 
	int   nLineCount = list.GetSelectedCount(); //获取ListCtrl的记录行数
	int   ColOrderArray[100];
	COLATT   ca[100];
	list.GetColumnOrderArray(ColOrderArray, nColCount); //存储列头的索引值
	int   nColX = nXMargin*nCharWidth;

	////////////////////////////////////////////////////////////
	//检索各列的信息，确定列标题的内容长度。
	for (int i = 0; i < nColCount; i++) {
		ca[i].nColIndex = ColOrderArray[i];
		LVCOLUMN lvc;
		TCHAR text[100];
		lvc.mask = LVCF_TEXT | LVCF_SUBITEM;
		lvc.pszText = text;
		lvc.cchTextMax = 100;
		list.GetColumn(ca[i].nColIndex, &lvc);
		ca[i].strColText = lvc.pszText;
		ca[i].nSubItemIndex = lvc.iSubItem;
		ca[i].nPrintX = nColX;
		nColX += nCharWidth *  _tcslen(ca[i].strColText);

		/////////////////////////////////////////////////////////////
		if (nColX > nHorRes) {  //表示输出的列头名的位置已经超出了  
			DeleteDC(pd.hDC);
			CString e; e.LoadStringW(IDS_STRING_E_TOOLMANY_FIELD);
			MessageBox(e, L"", MB_ICONERROR);
			return  FALSE;
		}
	}

	//设置打印文件的保存对话框 
	CString fm;
	DOCINFO   di;
	di.cbSize = sizeof(DOCINFO);
	fm.LoadString(IDS_STRING_EXPIRE_DOC_NAME);
	di.lpszDocName = fm.LockBuffer();
	di.lpszOutput = (LPTSTR)nullptr;
	di.lpszDatatype = (LPTSTR)nullptr;
	di.fwType = 0;
	StartDoc(pd.hDC, &di);
	StartPage(pd.hDC);
	fm.UnlockBuffer();
	SelectObject(pd.hDC, hTopicFont);
	TextOut(pd.hDC, nHorRes / 3, nYMargin, fm, fm.GetLength());

	////////////////////////////////////////////////
	//调整各列的宽度，以使各列在后面的打印输出时更均匀的打印在纸上. 
	int   space = (nHorRes - nXMargin*nCharWidth - nColX) / (nColCount);
	for (int i = 1; i < nColCount; i++) {
		ca[i].nPrintX += i*space;
	}
	SelectObject(pd.hDC, hCodeFont);
	//输出列标题
	for (int i = 0; i < nColCount; i++) {
		TextOut(pd.hDC, ca[i].nPrintX, nYMargin + 260,
				ca[i].strColText, ca[i].strColText.GetLength());
	}

	int   nMaxLinePerPage = nVerRes / nCharHeight - 3;
	int   nCurPage = 1;
	SelectObject(pd.hDC, hRecordFont);//将字体选入设备描述表里

	//HWND hd = ::GetDesktopWindow();
	//HDC ddc = ::GetDC(hd);
	//输出各列的数据   
	int  nItem = -1;
	for (int i = 0; i < nLineCount; i++) {
		nItem = list.GetNextItem(nItem, LVNI_SELECTED);
		ASSERT(nItem != -1);
		for (int j = 0; j<nColCount; j++) {
			if (i + 1 - (nCurPage - 1)*nMaxLinePerPage > nMaxLinePerPage) {
				//新的一页   
				EndPage(pd.hDC);
				StartPage(pd.hDC);
				nCurPage++;
			}
			CString subitem = list.GetItemText(nItem, ca[j].nColIndex);
			CLog::WriteLog(_T("%s\n"), subitem);
			TextOut(pd.hDC, ca[j].nPrintX,
					nYMargin + 300 + (i + 1 - (nCurPage - 1)*nMaxLinePerPage)*nCharHeight,
					subitem, _tcslen(subitem));
			//  DrawText(ddc, ca[j].nPrintX,     
			//	nYMargin+300+(i+1-(nCurPage-1)*nMaxLinePerPage)*nCharHeight,     
			//	subitem, _tcslen(subitem));   
		}
	}
	//::ReleaseDC(hd, ddc);
	EndPage(pd.hDC);
	EndDoc(pd.hDC);

	//打印结束
	DeleteObject(hTopicFont);
	DeleteObject(hRecordFont);
	DeleteObject(hCodeFont);
	DeleteDC(pd.hDC);

	return  TRUE;
}


void CMachineExpireManagerDlg::OnBnClickedButtonPrintSel() 
{
	AUTO_LOG_FUNCTION;
	PrintRecord(m_list);
}

namespace {
	typedef struct my_compare_struct {
		bool basc;
		int isubitem;
		//LPARAM machine;
	}my_compare_struct;

	int __stdcall my_compare_func(LPARAM lp1, LPARAM lp2, LPARAM lp3)
	{
		CAlarmMachine* machine1 = reinterpret_cast<CAlarmMachine*>(lp1);
		CAlarmMachine* machine2 = reinterpret_cast<CAlarmMachine*>(lp2);
		my_compare_struct* m = reinterpret_cast<my_compare_struct*>(lp3);
		int ret = 0;
		switch (m->isubitem) {
		case 0: // id
			if (machine1->get_is_submachine()) {
				ret = machine1->get_submachine_zone() - machine2->get_submachine_zone();
			} else {
				ret = machine1->get_ademco_id() - machine2->get_ademco_id();
			}
			break;
		case 1: // alias
			ret = machine1->get_alias().compare(machine2->get_alias());
			break;
		case 2: // expire time
		case 3: // if expire
			//ret = machine1->get_left_service_time() - machine2->get_left_service_time();
		{
			COleDateTimeSpan span = machine1->get_expire_time() - machine2->get_expire_time();
			double minutes = span.GetTotalMinutes();
			if (minutes > 0)
				ret = 1;
			else if (minutes < 0)
				ret = -1;
		}
		break;
		case 4: // contact
			ret = machine1->get_contact().compare(machine2->get_contact());
			break;
		case 5: // address
			ret = machine1->get_address().compare(machine2->get_address());
			break;
		case 6: // phone
			ret = machine1->get_phone().compare(machine2->get_phone());
			break;
		case 7: // phone_bk
			ret = machine1->get_phone_bk().compare(machine2->get_phone_bk());
			break;
		default:
			break;
		}
		ret = m->basc ? ret : -ret;
		return ret;
	}
};

void CMachineExpireManagerDlg::OnLvnColumnclickList1(NMHDR *pNMHDR, LRESULT *pResult) 
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	static bool basc = true;
	my_compare_struct mcs;
	mcs.basc = basc;
	mcs.isubitem = pNMLV->iSubItem;
	//mcs.machine = pNMLV->lParam;
	m_list.SortItems(my_compare_func, reinterpret_cast<DWORD_PTR>(&mcs));
	basc = !basc;
	*pResult = 0;
}



void CMachineExpireManagerDlg::OnHdnItemclickList1(NMHDR *pNMHDR, LRESULT *pResult) 
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	phdr;
	*pResult = 0;
}
