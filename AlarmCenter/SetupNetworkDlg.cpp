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
	DDX_Control(pDX, IDC_IPADDRESS1, m_ctrl_server_ip);
	DDX_Control(pDX, IDC_IPADDRESS2, m_ctrl_server_ip_bk);
	DDX_Text(pDX, IDC_EDIT1, m_listening_port);
	DDX_Text(pDX, IDC_EDIT2, m_server_port);
	DDX_Text(pDX, IDC_EDIT3, m_server_port_bk);
	DDX_Control(pDX, IDC_RADIO_MODE_CSR, m_radioCsr);
	DDX_Control(pDX, IDC_RADIO_MODE_TRANSMIT, m_radioTransmit);
	DDX_Control(pDX, IDC_RADIO_MODE_DUAL, m_radioDual);
	DDX_Control(pDX, IDC_EDIT1, m_ctrl_listening_port);
	DDX_Control(pDX, IDC_EDIT2, m_ctrl_server_port);
	DDX_Control(pDX, IDC_EDIT3, m_ctrl_server_port_bk);
}


BEGIN_MESSAGE_MAP(CSetupNetworkDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CSetupNetworkDlg::OnBnClickedOk)
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_RADIO_MODE_CSR, &CSetupNetworkDlg::OnBnClickedRadioModeCsr)
	ON_BN_CLICKED(IDC_RADIO_MODE_TRANSMIT, &CSetupNetworkDlg::OnBnClickedRadioModeTransmit)
	ON_BN_CLICKED(IDC_RADIO_MODE_DUAL, &CSetupNetworkDlg::OnBnClickedRadioModeDual)
END_MESSAGE_MAP()


// CSetupNetworkDlg message handlers


void CSetupNetworkDlg::OnBnClickedOk()
{
	UpdateData();
	USES_CONVERSION;
	if (m_listening_port > 65535 || m_server_port > 65535 || m_server_port_bk > 65535) {
		return;
	}

	wchar_t wip[32] = { 0 };
	m_ctrl_server_ip.GetWindowTextW(wip, 32);
	m_server_ip = utf8::w2a(wip);

	m_ctrl_server_ip_bk.GetWindowTextW(wip, 32);
	m_server_ip_bk = utf8::w2a(wip);

	auto cfg = util::CConfigHelper::GetInstance();
	cfg->set_network_mode(detail::g_network_mode);
	cfg->set_listening_port(m_listening_port);
	cfg->set_server_ip(m_server_ip);
	cfg->set_server_port(m_server_port);
	cfg->set_server_ip_bk(m_server_ip_bk);
	cfg->set_server_port_bk(m_server_port_bk);
	
	CDialogEx::OnOK();
}


BOOL CSetupNetworkDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_btnOK.GetWindowTextW(m_txtOk);

	auto cfg = util::CConfigHelper::GetInstance();

	m_listening_port = cfg->get_listening_port();
	m_server_ip = cfg->get_server_ip();
	m_server_port = cfg->get_server_port();
	m_server_ip_bk = cfg->get_server_ip_bk();
	m_server_port_bk = cfg->get_server_port_bk();

	USES_CONVERSION;
	m_ctrl_server_ip.SetWindowTextW(A2W(m_server_ip.c_str()));
	m_ctrl_server_ip_bk.SetWindowTextW(A2W(m_server_ip_bk.c_str()));

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
		m_ctrl_server_ip.EnableWindow(1);
		m_ctrl_server_port.EnableWindow(1);
		m_ctrl_server_ip_bk.EnableWindow(1);
		m_ctrl_server_port_bk.EnableWindow(1);
		m_radioCsr.SetCheck(0);
		m_radioTransmit.SetCheck(1);
		m_radioDual.SetCheck(0);
		break;
	case util::NETWORK_MODE_DUAL:
		m_ctrl_listening_port.EnableWindow(1);
		m_ctrl_server_ip.EnableWindow(1);
		m_ctrl_server_port.EnableWindow(1);
		m_ctrl_server_ip_bk.EnableWindow(1);
		m_ctrl_server_port_bk.EnableWindow(1);
		m_radioCsr.SetCheck(0);
		m_radioTransmit.SetCheck(0);
		m_radioDual.SetCheck(1);
		break;
	case util::NETWORK_MODE_CSR:
	default:
		m_ctrl_listening_port.EnableWindow(1);
		m_ctrl_server_ip.EnableWindow(0);
		m_ctrl_server_port.EnableWindow(0);
		m_ctrl_server_ip_bk.EnableWindow(0);
		m_ctrl_server_port_bk.EnableWindow(0);
		m_radioCsr.SetCheck(1);
		m_radioTransmit.SetCheck(0);
		m_radioDual.SetCheck(0);
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
