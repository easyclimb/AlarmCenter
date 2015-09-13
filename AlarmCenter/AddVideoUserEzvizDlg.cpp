// AddVideoUserEzvizDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AddVideoUserEzvizDlg.h"
#include "afxdialogex.h"
#include "VideoManager.h"

// CAddVideoUserEzvizDlg dialog

IMPLEMENT_DYNAMIC(CAddVideoUserEzvizDlg, CDialogEx)

CAddVideoUserEzvizDlg::CAddVideoUserEzvizDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAddVideoUserEzvizDlg::IDD, pParent)
	, m_strName(L"")
	, m_strPhone(L"")
{

}

CAddVideoUserEzvizDlg::~CAddVideoUserEzvizDlg()
{
}

void CAddVideoUserEzvizDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_name);
	DDX_Control(pDX, IDC_EDIT2, m_phone);
}


BEGIN_MESSAGE_MAP(CAddVideoUserEzvizDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAddVideoUserEzvizDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CAddVideoUserEzvizDlg message handlers


void CAddVideoUserEzvizDlg::OnBnClickedOk()
{
	m_name.GetWindowTextW(m_strName);
	m_phone.GetWindowTextW(m_strPhone);
	if (m_strName.IsEmpty() || m_strPhone.IsEmpty()) {
		return;
	}
	USES_CONVERSION;
	if (video::CVideoManager::GetInstance()->CheckIfUserEzvizPhoneExists(W2A(m_strPhone))) {
		CString e; e.LoadStringW(IDS_STRING_PHONE_ALREADY_EXISTS);
		MessageBox(e, L"", MB_ICONERROR);
		return;
	}

	CDialogEx::OnOK();
}
