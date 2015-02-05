// SetupNetworkDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "SetupNetworkDlg.h"
#include "afxdialogex.h"
#include "./tinyxml/tinyxml.h"
using namespace tinyxml;


// CSetupNetworkDlg dialog

IMPLEMENT_DYNAMIC(CSetupNetworkDlg, CDialogEx)

CSetupNetworkDlg::CSetupNetworkDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSetupNetworkDlg::IDD, pParent)
	, m_local_port(0)
	, m_transmit_port(0)
	, m_begin(0)
	, m_txtOk(L"")
{

}

CSetupNetworkDlg::~CSetupNetworkDlg()
{
}

void CSetupNetworkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDRESS1, m_transmit_ip);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Text(pDX, IDC_EDIT1, m_local_port);
	DDX_Text(pDX, IDC_EDIT2, m_transmit_port);
}


BEGIN_MESSAGE_MAP(CSetupNetworkDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CSetupNetworkDlg::OnBnClickedOk)
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


// CSetupNetworkDlg message handlers


void CSetupNetworkDlg::OnBnClickedOk()
{
	UpdateData();
	if (m_local_port > 65535 || m_transmit_port > 65535) {
		return;
	}

	memset(m_tranmit_ipA, 0, sizeof(m_tranmit_ipA));
	wchar_t wip[32] = { 0 };
	m_transmit_ip.GetWindowTextW(wip, 32);
	Utf16ToAnsiUseCharArray(wip, m_tranmit_ipA, 32);
	
	CDialogEx::OnOK();
}


BOOL CSetupNetworkDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_btnOK.GetWindowTextW(m_txtOk);

	m_local_port = 12345;
	m_transmit_ip.SetWindowTextW(L"192.168.0.75");
	m_transmit_port = 7892;

	CString path;
	path.Format(L"%s\\network.xml", GetModuleFilePath());
	USES_CONVERSION;
	TiXmlDocument doc(W2A(path));
	if (doc.LoadFile()) {
		TiXmlElement* root = doc.RootElement();
		TiXmlNode* node = root->FirstChild("local_port");
		if (node) {
			node = node->FirstChild();
			if (node) {
				const char* v = node->Value();
				m_local_port = atoi(v);
			}
		}

		node = root->FirstChild("transmit_server_ip");
		if (node) {
			node = node->FirstChild();
			if (node) {
				m_transmit_ip.SetWindowText(A2W(node->Value()));
			}
		}

		node = root->FirstChild("transmit_server_port");
		if (node) {
			node = node->FirstChild();
			if (node) {
				m_transmit_port = atoi(node->Value());
			}
		}

		SetTimer(1, 1000, NULL);
	}

	UpdateData(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

static const int TIMEOUT = 3;

void CSetupNetworkDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (m_begin == 0) {
		m_begin = TIMEOUT;
	}

	if (m_begin-- <= 1) {
		KillTimer(1);
		m_btnOK.SetWindowTextW(m_txtOk);
		OnBnClickedOk();
	} else {
		CString newTxt;
		newTxt.Format(L"%s %ds", m_txtOk, m_begin);
		m_btnOK.SetWindowTextW(newTxt);
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CSetupNetworkDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	KillTimer(1);
	m_btnOK.SetWindowTextW(m_txtOk);

	CDialogEx::OnMouseMove(nFlags, point);
}
