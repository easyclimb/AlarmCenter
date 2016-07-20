// InputDeviceVerifyCodeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenterVideo.h"
#include "InputDeviceVerifyCodeDlg.h"
#include "afxdialogex.h"
#include "../video/ezviz/VideoDeviceInfoEzviz.h"

// CInputDeviceVerifyCodeDlg dialog

IMPLEMENT_DYNAMIC(CInputDeviceVerifyCodeDlg, CDialogEx)

CInputDeviceVerifyCodeDlg::CInputDeviceVerifyCodeDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CInputDeviceVerifyCodeDlg::IDD, pParent)
	, m_result(L"")
{

}

CInputDeviceVerifyCodeDlg::~CInputDeviceVerifyCodeDlg()
{
}

void CInputDeviceVerifyCodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_code);
}


BEGIN_MESSAGE_MAP(CInputDeviceVerifyCodeDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CInputDeviceVerifyCodeDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CInputDeviceVerifyCodeDlg message handlers


void CInputDeviceVerifyCodeDlg::OnBnClickedOk()
{
	m_code.GetWindowTextW(m_result);
	if (!video::ezviz::ezviz_device::IsValidVerifyCode(utf8::w2a((LPCTSTR)(m_result)))) {
		CString note; note = TR(IDS_STRING_DEVICE_CODE_INVALID);
		MessageBox(note, L"", MB_ICONERROR);
		m_code.SetWindowTextW(L"");
		return;
	}
	CDialogEx::OnOK();
}
