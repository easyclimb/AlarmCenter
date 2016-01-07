// ExtendExpireTimeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "ExtendExpireTimeDlg.h"
#include "afxdialogex.h"


// CExtendExpireTimeDlg dialog

IMPLEMENT_DYNAMIC(CExtendExpireTimeDlg, CDialogEx)

CExtendExpireTimeDlg::CExtendExpireTimeDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CExtendExpireTimeDlg::IDD, pParent)
	, m_dateTime()
{

}

CExtendExpireTimeDlg::~CExtendExpireTimeDlg()
{
}

void CExtendExpireTimeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DATETIMEPICKER1, m_date);
	DDX_Control(pDX, IDC_DATETIMEPICKER2, m_time);
}


BEGIN_MESSAGE_MAP(CExtendExpireTimeDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CExtendExpireTimeDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CExtendExpireTimeDlg message handlers


void CExtendExpireTimeDlg::OnBnClickedOk()
{
	UpdateData();
	CTime cDate, cTime;
	DWORD dwResult = m_date.GetTime(cDate);
	CString e;
	if (dwResult != GDT_VALID) {
		e = GetStringFromAppResource(IDS_STRING_INVALID_DATE);
		MessageBox(e, L"", MB_ICONERROR);
		return;
	}
	dwResult = m_time.GetTime(cTime);
	if (dwResult != GDT_VALID) {
		e = GetStringFromAppResource(IDS_STRING_INVALID_TIME);
		MessageBox(e, L"", MB_ICONERROR);
		return;
	}
	/*CString s;
	s = cDate.Format(L"%Y-%m-%d");
	m_dateTime.SetDate(cDate.GetYear(), cDate.GetMonth(), cDate.GetDay());
	s = m_dateTime.Format(L"%Y-%m-%d %H:%M:%S");
	s = cTime.Format(L"%H:%M:%S");
	m_dateTime.SetTime(cTime.GetHour(), cTime.GetMinute(), cTime.GetSecond());
	s = m_dateTime.Format(L"%Y-%m-%d %H:%M:%S");*/
	SYSTEMTIME st = { 0 };
	st.wYear = static_cast<WORD>(cDate.GetYear());
	st.wMonth = static_cast<WORD>(cDate.GetMonth());
	st.wDay = static_cast<WORD>(cDate.GetDay());
	st.wHour = static_cast<WORD>(cTime.GetHour());
	st.wMinute = static_cast<WORD>(cTime.GetMinute());
	st.wSecond = static_cast<WORD>(cTime.GetSecond());
	m_dateTime = st;
	CString s = m_dateTime.Format(L"%Y-%m-%d %H:%M:%S");
	if (m_dateTime.GetStatus() != COleDateTime::valid) {
		e = GetStringFromAppResource(IDS_STRING_INVALID_DATE);
		MessageBox(e, L"", MB_ICONERROR);
		return;
	}
	CDialogEx::OnOK();
}


BOOL CExtendExpireTimeDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	COleDateTime now = COleDateTime::GetCurrentTime();
	m_date.SetTime(now);
	m_time.SetTime(now);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
