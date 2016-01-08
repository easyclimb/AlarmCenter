// SetupNetworkDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "SetupNetworkDlg.h"
#include "afxdialogex.h"
#include "ConfigHelper.h"
#include <winsock2.h>
#include <ws2tcpip.h>

namespace detail {
	util::NetworkMode g_network_mode = util::NETWORK_MODE_CSR;
	const int TIMEOUT = 3;

	bool is_domain(const CString& name) {
		CString domain = L"http://";
		domain += name;
		auto hr = IsValidURL(nullptr, domain, 0);
		return hr == S_OK;
	}

	std::string get_domain_ip(const std::string& domain) {
		AUTO_LOG_FUNCTION;
		struct addrinfo *result = nullptr;
		struct addrinfo *ptr = nullptr;
		struct addrinfo hints;

		struct sockaddr_in  *sockaddr_ipv4;
		//    struct sockaddr_in6 *sockaddr_ipv6;
		LPSOCKADDR sockaddr_ip;

		char ipstringbuffer[46];
		DWORD ipbufferlength = 46;

		//--------------------------------
		// Setup the hints address info structure
		// which is passed to the getaddrinfo() function
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		std::string ip;
		
		do {

			//--------------------------------
			// Call getaddrinfo(). If the call succeeds,
			// the result variable will hold a linked list
			// of addrinfo structures containing response
			// information
			DWORD dwRetval = getaddrinfo(domain.c_str(), "0", &hints, &result);
			if (dwRetval != 0) {
				JLOGA("getaddrinfo failed with error: %d\n", dwRetval);
				break;
			}

			JLOGA("getaddrinfo returned success\n");

			// Retrieve each address and print out the hex bytes
			int i = 0;
			int iRetval = 0;
			bool ok = false;
			for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
				JLOGA("getaddrinfo response %d\n", i++);
				JLOGA("\tFlags: 0x%x\n", ptr->ai_flags);
				JLOGA("\tFamily: ");
				switch (ptr->ai_family) {
				case AF_UNSPEC:
					JLOGA("Unspecified\n");
					break;
				case AF_INET:
					JLOGA("AF_INET (IPv4)\n");
					sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
					ip = inet_ntoa(sockaddr_ipv4->sin_addr);
					JLOGA("\tIPv4 address %s\n", ip.c_str());
					ok = true;
					break;
				case AF_INET6:
					JLOGA("AF_INET6 (IPv6)\n");
					// the InetNtop function is available on Windows Vista and later
					// sockaddr_ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
					// printf("\tIPv6 address %s\n",
					//    InetNtop(AF_INET6, &sockaddr_ipv6->sin6_addr, ipstringbuffer, 46) );

					// We use WSAAddressToString since it is supported on Windows XP and later
					sockaddr_ip = (LPSOCKADDR)ptr->ai_addr;
					// The buffer length is changed by each call to WSAAddresstoString
					// So we need to set it for each iteration through the loop for safety
					ipbufferlength = 46;
					iRetval = WSAAddressToStringA(sockaddr_ip, (DWORD)ptr->ai_addrlen, NULL,
												 ipstringbuffer, &ipbufferlength);
					if (iRetval)
						JLOGA("WSAAddressToString failed with %u\n", WSAGetLastError());
					else
						JLOGA("\tIPv6 address %s\n", ipstringbuffer);
					break;
				case AF_NETBIOS:
					JLOGA("AF_NETBIOS (NetBIOS)\n");
					break;
				default:
					JLOGA("Other %ld\n", ptr->ai_family);
					break;
				}
				JLOGA("\tSocket type: ");
				switch (ptr->ai_socktype) {
				case 0:
					JLOGA("Unspecified\n");
					break;
				case SOCK_STREAM:
					JLOGA("SOCK_STREAM (stream)\n");
					break;
				case SOCK_DGRAM:
					JLOGA("SOCK_DGRAM (datagram) \n");
					break;
				case SOCK_RAW:
					JLOGA("SOCK_RAW (raw) \n");
					break;
				case SOCK_RDM:
					JLOGA("SOCK_RDM (reliable message datagram)\n");
					break;
				case SOCK_SEQPACKET:
					JLOGA("SOCK_SEQPACKET (pseudo-stream packet)\n");
					break;
				default:
					JLOGA("Other %ld\n", ptr->ai_socktype);
					break;
				}
				JLOGA("\tProtocol: ");
				switch (ptr->ai_protocol) {
				case 0:
					JLOGA("Unspecified\n");
					break;
				case IPPROTO_TCP:
					JLOGA("IPPROTO_TCP (TCP)\n");
					break;
				case IPPROTO_UDP:
					JLOGA("IPPROTO_UDP (UDP) \n");
					break;
				default:
					JLOGA("Other %ld\n", ptr->ai_protocol);
					break;
				}
				JLOGA("\tLength of this sockaddr: %d\n", ptr->ai_addrlen);
				JLOGA("\tCanonical name: %s\n", ptr->ai_canonname);

				if (ok) {
					break;
				}
			}

			freeaddrinfo(result);

			if (ok) {
				return ip;
			}
		}while (false);

		return "";
	}



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
	DDX_Control(pDX, IDC_BUTTON_TEST_DOMAIN1, m_btnTestDomain1);
	DDX_Control(pDX, IDC_BUTTON_TEST_DOMAIN2, m_btnTestDomain2);
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
	ON_BN_CLICKED(IDC_BUTTON_TEST_DOMAIN1, &CSetupNetworkDlg::OnBnClickedButtonTestDomain1)
	ON_BN_CLICKED(IDC_BUTTON_TEST_DOMAIN2, &CSetupNetworkDlg::OnBnClickedButtonTestDomain2)
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

	int b1 = m_chkByIpPort1.GetCheck();
	int b2 = m_chkByIpPort2.GetCheck();

	CString server1_domain, server2_domain;
	m_server1_domain.GetWindowTextW(server1_domain);
	m_server2_domain.GetWindowTextW(server2_domain);

	auto cfg = util::CConfigHelper::GetInstance();
	cfg->set_network_mode(detail::g_network_mode);
	cfg->set_listening_port(m_listening_port);

	cfg->set_server1_by_ipport(b1);
	cfg->set_server1_domain(W2A(server1_domain));
	cfg->set_server1_ip(m_server1_ip);
	cfg->set_server1_port(m_server1_port);

	cfg->set_server2_by_ipport(b2);
	cfg->set_server2_domain(W2A(server2_domain));
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
	m_server1_domain.SetWindowTextW(A2W(cfg->get_server1_domain().c_str()));
	m_server2_domain.SetWindowTextW(A2W(cfg->get_server2_domain().c_str()));
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
		m_server1_domain.EnableWindow();
		m_server2_domain.EnableWindow();
		m_chkByIpPort1.EnableWindow();
		m_chkByIpPort2.EnableWindow();
		m_btnTestDomain1.EnableWindow();
		m_btnTestDomain2.EnableWindow();
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
		m_server1_domain.EnableWindow();
		m_server2_domain.EnableWindow();
		m_chkByIpPort1.EnableWindow();
		m_chkByIpPort2.EnableWindow();
		m_btnTestDomain1.EnableWindow();
		m_btnTestDomain2.EnableWindow();
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
		m_server1_domain.EnableWindow(0);
		m_server2_domain.EnableWindow(0);
		m_chkByIpPort1.EnableWindow(0);
		m_chkByIpPort2.EnableWindow(0);
		m_btnTestDomain1.EnableWindow(0);
		m_btnTestDomain2.EnableWindow(0);
		break;
	}

	if (util::NETWORK_MODE_TRANSMIT & mode) {
		auto cfg = util::CConfigHelper::GetInstance();
		int man1 = cfg->get_server1_by_ipport();
		int man2 = cfg->get_server2_by_ipport();
		m_chkByIpPort1.SetCheck(man1);
		m_chkByIpPort2.SetCheck(man2);
		OnBnClickedCheckByIpport1();
		OnBnClickedCheckByIpport2();
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
	m_btnTestDomain1.EnableWindow(!b);
}


void CSetupNetworkDlg::OnBnClickedCheckByIpport2()
{
	BOOL b = m_chkByIpPort2.GetCheck();
	m_server2_domain.EnableWindow(!b);
	m_ctrl_server2_ip.EnableWindow(b);
	m_ctrl_server2_port.EnableWindow(b);
	m_btnTestDomain2.EnableWindow(!b);
}


void CSetupNetworkDlg::OnBnClickedButtonTestDomain1()
{
	USES_CONVERSION;
	CString domain;
	m_server1_domain.GetWindowTextW(domain);
	if (domain.IsEmpty())return;
	//if (detail::is_domain(domain)) {
		auto ip = detail::get_domain_ip(W2A(domain));
		if (ip.empty()) {
			m_ctrl_server1_ip.SetWindowTextW(L"");
			m_ctrl_server1_port.SetWindowTextW(L"");
		} else {
			m_ctrl_server1_ip.SetWindowTextW(A2W(ip.c_str()));
			m_ctrl_server1_port.SetWindowTextW(L"7892");
		}
	//}
}


void CSetupNetworkDlg::OnBnClickedButtonTestDomain2()
{
	USES_CONVERSION;
	CString domain;
	m_server2_domain.GetWindowTextW(domain);
	if (domain.IsEmpty())return;
	//if (detail::is_domain(domain)) {
	auto ip = detail::get_domain_ip(W2A(domain));
	if (ip.empty()) {
		m_ctrl_server2_ip.SetWindowTextW(L"");
		m_ctrl_server2_port.SetWindowTextW(L"");
	} else {
		m_ctrl_server2_ip.SetWindowTextW(A2W(ip.c_str()));
		m_ctrl_server2_port.SetWindowTextW(L"7892");
	}
	//}
}
