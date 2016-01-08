// SetupNetworkDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "SetupNetworkDlg.h"
#include "afxdialogex.h"
#include "ConfigHelper.h"

namespace detail {
	util::NetworkMode g_network_mode = util::NETWORK_MODE_CSR;
	const int TIMEOUT = 3;
}

// CSetupNetworkDlg dialog

IMPLEMENT_DYNAMIC(CSetupNetworkDlg, CDialogEx)

CSetupNetworkDlg::CSetupNetworkDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CSetupNetworkDlg::IDD, pParent)
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
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_IPADDRESS1, m_ctrl_server1_ip);
	DDX_Control(pDX, IDC_IPADDRESS2, m_ctrl_server2_ip);
	DDX_Text(pDX, IDC_EDIT1, m_listening_port);
	DDX_Text(pDX, IDC_EDIT2, m_server1_port);
	DDX_Text(pDX, IDC_EDIT3, m_server2_port);
	DDX_Control(pDX, IDC_RADIO_MODE_CSR, m_radioCsr);
	DDX_Control(pDX, IDC_RADIO_MODE_TRANSMIT, m_radioTransmit);
	DDX_Control(pDX, IDC_RADIO_MODE_DUAL, m_radioDual);
	DDX_Control(pDX, IDC_EDIT1, m_ctrl_listening_port);
	DDX_Control(pDX, IDC_EDIT2, m_ctrl_server1_port);
	DDX_Control(pDX, IDC_EDIT3, m_ctrl_server2_port);
	DDX_Control(pDX, IDC_EDIT_SERVER1_DOMAIN, m_server1_domain);
	DDX_Control(pDX, IDC_EDIT_SERVER2_DOMAIN, m_server2_domain);
	DDX_Control(pDX, IDC_CHECK_BY_IPPORT1, m_chkByIpPort1);
	DDX_Control(pDX, IDC_CHECK_BY_IPPORT2, m_chkByIpPort2);
}


BEGIN_MESSAGE_MAP(CSetupNetworkDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CSetupNetworkDlg::OnBnClickedOk)
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_RADIO_MODE_CSR, &CSetupNetworkDlg::OnBnClickedRadioModeCsr)
	ON_BN_CLICKED(IDC_RADIO_MODE_TRANSMIT, &CSetupNetworkDlg::OnBnClickedRadioModeTransmit)
	ON_BN_CLICKED(IDC_RADIO_MODE_DUAL, &CSetupNetworkDlg::OnBnClickedRadioModeDual)
	ON_BN_CLICKED(IDC_CHECK_BY_IPPORT1, &CSetupNetworkDlg::OnBnClickedCheckByIpport1)
	ON_BN_CLICKED(IDC_CHECK_BY_IPPORT2, &CSetupNetworkDlg::OnBnClickedCheckByIpport2)
END_MESSAGE_MAP()


// CSetupNetworkDlg message handlers


void CSetupNetworkDlg::OnBnClickedOk()
{
	UpdateData();
	USES_CONVERSION;
	if (m_listening_port > 65535 || m_server1_port > 65535 || m_server2_port > 65535) {
		return;
	}

	wchar_t wip[32] = { 0 };
	m_ctrl_server1_ip.GetWindowTextW(wip, 32);
	m_server1_ip = utf8::w2a(wip);

	m_ctrl_server2_ip.GetWindowTextW(wip, 32);
	m_server2_ip = utf8::w2a(wip);

	auto cfg = util::CConfigHelper::GetInstance();
	cfg->set_network_mode(detail::g_network_mode);
	cfg->set_listening_port(m_listening_port);
	cfg->set_server1_ip(m_server1_ip);
	cfg->set_server1_port(m_server1_port);
	cfg->set_server2_ip(m_server2_ip);
	cfg->set_server2_port(m_server2_port);
	
	CDialogEx::OnOK();
}


BOOL CSetupNetworkDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_btnOK.GetWindowTextW(m_txtOk);

	auto cfg = util::CConfigHelper::GetInstance();

	m_listening_port = cfg->get_listening_port();
	m_server1_ip = cfg->get_server1_ip();
	m_server1_port = cfg->get_server1_port();
	m_server2_ip = cfg->get_server2_ip();
	m_server2_port = cfg->get_server2_port();

	USES_CONVERSION;
	m_ctrl_server1_ip.SetWindowTextW(A2W(m_server1_ip.c_str()));
	m_ctrl_server2_ip.SetWindowTextW(A2W(m_server2_ip.c_str()));

	detail::g_network_mode = cfg->get_network_mode();
	EnableWindows(detail::g_network_mode);


	SetTimer(1, 1000, nullptr);

	UpdateData(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CSetupNetworkDlg::EnableWindows(int mode)
{
	switch (mode) {
	case util::NETWORK_MODE_TRANSMIT:
		m_ctrl_listening_port.EnableWindow(0);
		m_ctrl_server1_ip.EnableWindow(1);
		m_ctrl_server1_port.EnableWindow(1);
		m_ctrl_server2_ip.EnableWindow(1);
		m_ctrl_server2_port.EnableWindow(1);
		m_radioCsr.SetCheck(0);
		m_radioTransmit.SetCheck(1);
		m_radioDual.SetCheck(0);
		m_chkByIpPort1.EnableWindow();
		m_chkByIpPort2.EnableWindow();
		break;
	case util::NETWORK_MODE_DUAL:
		m_ctrl_listening_port.EnableWindow(1);
		m_ctrl_server1_ip.EnableWindow(1);
		m_ctrl_server1_port.EnableWindow(1);
		m_ctrl_server2_ip.EnableWindow(1);
		m_ctrl_server2_port.EnableWindow(1);
		m_radioCsr.SetCheck(0);
		m_radioTransmit.SetCheck(0);
		m_radioDual.SetCheck(1);
		m_chkByIpPort1.EnableWindow();
		m_chkByIpPort2.EnableWindow();
		break;
	case util::NETWORK_MODE_CSR:
	default:
		m_ctrl_listening_port.EnableWindow(1);
		m_ctrl_server1_ip.EnableWindow(0);
		m_ctrl_server1_port.EnableWindow(0);
		m_ctrl_server2_ip.EnableWindow(0);
		m_ctrl_server2_port.EnableWindow(0);
		m_radioCsr.SetCheck(1);
		m_radioTransmit.SetCheck(0);
		m_radioDual.SetCheck(0);
		m_chkByIpPort1.EnableWindow(0);
		m_chkByIpPort2.EnableWindow(0);
		break;
	}
}



void CSetupNetworkDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (m_begin == 0) {
		m_begin = detail::TIMEOUT;
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


void CSetupNetworkDlg::OnBnClickedRadioModeCsr()
{
	detail::g_network_mode = util::NETWORK_MODE_CSR;
	//util::CConfigHelper::GetInstance()->set_network_mode(detail::g_network_mode);
	EnableWindows(detail::g_network_mode);
}


void CSetupNetworkDlg::OnBnClickedRadioModeTransmit()
{
	detail::g_network_mode = util::NETWORK_MODE_TRANSMIT;
	//util::CConfigHelper::GetInstance()->set_network_mode(detail::g_network_mode);
	EnableWindows(detail::g_network_mode);
}


void CSetupNetworkDlg::OnBnClickedRadioModeDual()
{
	detail::g_network_mode = util::NETWORK_MODE_DUAL;
	//util::CConfigHelper::GetInstance()->set_network_mode(detail::g_network_mode);
	EnableWindows(detail::g_network_mode);
}


void CSetupNetworkDlg::OnBnClickedCheckByIpport1()
{
	BOOL b = m_chkByIpPort1.GetCheck();
	m_server1_domain.EnableWindow(!b);
	m_ctrl_server1_ip.EnableWindow(b);
	m_ctrl_server1_port.EnableWindow(b);
}


void CSetupNetworkDlg::OnBnClickedCheckByIpport2()
{
	BOOL b = m_chkByIpPort2.GetCheck();
	m_server2_domain.EnableWindow(!b);
	m_ctrl_server2_ip.EnableWindow(b);
	m_ctrl_server2_port.EnableWindow(b);
}
