// SetupNetworkDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AppResource.h"
#include "SetupNetworkDlg.h"
#include "afxdialogex.h"
#include "ConfigHelper.h"
#include <winsock2.h>
#include <ws2tcpip.h>

#define LIB_BOOST_BASE "D:/dev_libs/boost_1_59_0/stage/lib/"

#ifdef _DEBUG
#pragma comment(lib, LIB_BOOST_BASE "libboost_system-vc140-mt-sgd-1_59.lib")
#pragma comment(lib, LIB_BOOST_BASE "libboost_date_time-vc140-mt-sgd-1_59.lib")
#pragma comment(lib, LIB_BOOST_BASE "libboost_regex-vc140-mt-sgd-1_59.lib")
#else
#pragma comment(lib, LIB_BOOST_BASE "libboost_system-vc140-mt-s-1_59.lib")
#pragma comment(lib, LIB_BOOST_BASE "libboost_date_time-vc140-mt-s-1_59.lib")
#pragma comment(lib, LIB_BOOST_BASE "libboost_regex-vc140-mt-s-1_59.lib")
#endif

#include <boost/asio.hpp>

namespace detail {
	util::NetworkMode g_network_mode = util::NETWORK_MODE_CSR;
	const int TIMEOUT = 3;

	bool is_domain(const CString& name) {
		CString domain = L"http://";
		domain += name;
		auto hr = IsValidURL(nullptr, domain, 0);
		return hr == S_OK;
	}

	bool get_domain_ip(const std::string& domain, std::string& result, progress_cb cb) {
		boost::asio::io_service io_service;
		boost::asio::ip::tcp::resolver resolver(io_service);
		boost::asio::ip::tcp::resolver::query query(domain, "");
		try {
			boost::asio::ip::tcp::resolver::iterator end;
			auto iter = resolver.resolve(query);
			std::string ip;
			std::string fastest_ip;
			long long fastest_ping_ms = 500000000;

			while (iter != end) {
				boost::asio::ip::tcp::endpoint endpoint = *iter++;
				ip = endpoint.address().to_string();

				pinger p(io_service, ip.c_str(), 3);
				
				boost::posix_time::ptime tstart = boost::posix_time::microsec_clock::universal_time();
				boost::asio::deadline_timer timer(io_service);

				std::function<void(const boost::system::error_code&)> handler;
				handler = [&](const boost::system::error_code&) {
					if (cb) {
						cb();
					}

					if (!p.quiting()) {
						tstart = boost::posix_time::microsec_clock::universal_time();
						timer.expires_at(tstart + boost::posix_time::seconds(1));
						timer.async_wait(handler);
					}
				};

				boost::system::error_code ec;
				handler(ec);

				io_service.run();

				auto ms = p.get_average();
				if (ms < fastest_ping_ms) {
					fastest_ping_ms = ms;
					fastest_ip = ip;
				}

				
			}

			result = fastest_ip;
			
			return true;
		} catch (std::exception& e) {
			//MessageBoxA(hWnd, e.what(), "Error", MB_ICONERROR);
			result = e.what();
			return false;
		}

	}



}

// CSetupNetworkDlg dialog

IMPLEMENT_DYNAMIC(CSetupNetworkDlg, CDialogEx)

CSetupNetworkDlg::CSetupNetworkDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CSetupNetworkDlg::IDD, pParent)
	, m_begin(0)
	, m_txtOk(L"")
	, m_csracct(_T(""))
{

}

CSetupNetworkDlg::~CSetupNetworkDlg()
{
}

void CSetupNetworkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_IPADDRESS1, m_server1_ip);
	DDX_Control(pDX, IDC_IPADDRESS2, m_server2_ip);
	//DDX_Text(pDX, IDC_EDIT1, m_listening_port);
	//DDX_Text(pDX, IDC_EDIT2, m_server1_port);
	//DDX_Text(pDX, IDC_EDIT3, m_server2_port);
	DDX_Control(pDX, IDC_RADIO_MODE_CSR, m_radioCsr);
	DDX_Control(pDX, IDC_RADIO_MODE_TRANSMIT, m_radioTransmit);
	DDX_Control(pDX, IDC_RADIO_MODE_DUAL, m_radioDual);
	DDX_Control(pDX, IDC_EDIT1, m_listening_port);
DDX_Control(pDX, IDC_EDIT2, m_server1_port);
DDX_Control(pDX, IDC_EDIT3, m_server2_port);
DDX_Control(pDX, IDC_EDIT_SERVER1_DOMAIN, m_server1_domain);
DDX_Control(pDX, IDC_EDIT_SERVER2_DOMAIN, m_server2_domain);
DDX_Control(pDX, IDC_CHECK_BY_IPPORT1, m_chkByIpPort1);
DDX_Control(pDX, IDC_CHECK_BY_IPPORT2, m_chkByIpPort2);
DDX_Control(pDX, IDC_BUTTON_TEST_DOMAIN1, m_btnTestDomain1);
DDX_Control(pDX, IDC_BUTTON_TEST_DOMAIN2, m_btnTestDomain2);
DDX_Control(pDX, IDC_EDIT_CSRACCT, m_csr_acct);
DDX_Text(pDX, IDC_EDIT_CSRACCT, m_csracct);
DDV_MaxChars(pDX, m_csracct, 18);
DDX_Control(pDX, IDC_EDIT_EZVIZ_DOMAIN, m_ezviz_domain);
DDX_Control(pDX, IDC_IPADDRESS3, m_ezviz_ip);
DDX_Control(pDX, IDC_EDIT_EZVIZ_PORT, m_ezviz_port);
DDX_Control(pDX, IDC_CHECK_BY_IPPORT3, m_chkByIpPort3);
DDX_Control(pDX, IDC_BUTTON_TEST_DOMAIN3, m_btnTestDomain3);
DDX_Control(pDX, IDC_EDIT_EZVIZ_APP_KEY, m_ezviz_app_key);
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
	ON_BN_CLICKED(IDC_BUTTON_TEST_DOMAIN3, &CSetupNetworkDlg::OnBnClickedButtonTestDomain3)
	ON_BN_CLICKED(IDC_CHECK_BY_IPPORT3, &CSetupNetworkDlg::OnBnClickedCheckByIpport3)
END_MESSAGE_MAP()


// CSetupNetworkDlg message handlers


void CSetupNetworkDlg::OnBnClickedOk()
{
	CString txt;

	m_listening_port.GetWindowText(txt);
	int listening_port = _ttoi(txt);

	m_server1_port.GetWindowTextW(txt);
	int server1_port = _ttoi(txt);

	m_server2_port.GetWindowTextW(txt);
	int server2_port = _ttoi(txt);

	m_ezviz_port.GetWindowTextW(txt);
	int ezviz_port = _ttoi(txt);

	if ((listening_port < 1024 || listening_port > 65535) && (detail::g_network_mode & util::NETWORK_MODE_CSR)) {
		m_listening_port.ShowBalloonTip(GetStringFromAppResource(IDS_STRING_ERROR), GetStringFromAppResource(IDS_STRING_INVALID_PORT), TTI_ERROR);
		return;
	}

	m_csr_acct.GetWindowTextW(m_csracct);
	if (m_csracct.IsEmpty() && (detail::g_network_mode & util::NETWORK_MODE_TRANSMIT)) {
		m_csr_acct.ShowBalloonTip(GetStringFromAppResource(IDS_STRING_ERROR), GetStringFromAppResource(IDS_STRING_INPUT_CSR_ACCT), TTI_ERROR);
		return;
	}

	if ((ezviz_port < 1024 || ezviz_port > 65535)) {
		m_ezviz_port.ShowBalloonTip(GetStringFromAppResource(IDS_STRING_ERROR), GetStringFromAppResource(IDS_STRING_INVALID_PORT), TTI_ERROR);
		return;
	}

	m_server1_ip.GetWindowTextW(txt);
	std::string server1_ip = utf8::w2a((LPCTSTR)(txt));

	m_server2_ip.GetWindowTextW(txt);
	std::string server2_ip = utf8::w2a((LPCTSTR)(txt));

	m_ezviz_ip.GetWindowTextW(txt);
	std::string ezviz_ip = utf8::w2a((LPCTSTR)(txt));

	int b1 = m_chkByIpPort1.GetCheck();
	int b2 = m_chkByIpPort2.GetCheck();
	int b3 = m_chkByIpPort3.GetCheck();

	CString server1_domain, server2_domain, ezviz_domain;
	m_server1_domain.GetWindowTextW(server1_domain);
	m_server2_domain.GetWindowTextW(server2_domain);
	m_ezviz_domain.GetWindowTextW(ezviz_domain);

	auto local_cb = [this]() {
		CString txt = m_txtOk;
		static int step = 1;
		for (int i = 0; i < step; i++) {
			txt += L".";
		}
		m_btnOK.SetWindowTextW(txt);
		step++;
		if (step > 4) {
			step = 1;
		}
	};

	if (detail::g_network_mode & util::NETWORK_MODE_TRANSMIT) {
		if (!b1 || server1_ip.empty()/* || server1_ip == "0.0.0.0"*/) {
			if (!resolve_domain(1, local_cb)) {
				return;
			}
			m_server1_ip.GetWindowTextW(txt);
			server1_ip = utf8::w2a((LPCTSTR)(txt));
		} 
		
		if (!server1_ip.empty() && server1_ip != "0.0.0.0") { // using
			if ((server1_port < 1024 || server1_port > 65535) && (detail::g_network_mode & util::NETWORK_MODE_TRANSMIT)) {
				m_server1_port.ShowBalloonTip(GetStringFromAppResource(IDS_STRING_ERROR), GetStringFromAppResource(IDS_STRING_INVALID_PORT), TTI_ERROR);
				return;
			}
		}

		if (!b2 || server2_ip.empty()/* || server2_ip == "0.0.0.0"*/) {
			if (!resolve_domain(2, local_cb)) {
				return;
			}
			m_server2_ip.GetWindowTextW(txt);
			server2_ip = utf8::w2a((LPCTSTR)(txt));
		}

		if (!server2_ip.empty() && server2_ip != "0.0.0.0") { // using
			if ((server2_port < 1024 || server2_port > 65535) && (detail::g_network_mode & util::NETWORK_MODE_TRANSMIT)) {
				m_server2_port.ShowBalloonTip(GetStringFromAppResource(IDS_STRING_ERROR), GetStringFromAppResource(IDS_STRING_INVALID_PORT), TTI_ERROR);
				return;
			}
		}
	}

	if (!b3 || ezviz_ip.empty()/* || ezviz_ip == "0.0.0.0"*/) {
		if (!resolve_domain(3, local_cb)) {
			return;
		}
		m_ezviz_ip.GetWindowTextW(txt);
		ezviz_ip = utf8::w2a((LPCTSTR)(txt));
	}

	auto cfg = util::CConfigHelper::get_instance();
	cfg->set_network_mode(detail::g_network_mode);
	cfg->set_listening_port(listening_port);
	cfg->set_csr_acct(utf8::w2a((LPCTSTR)(m_csracct)));

	cfg->set_server1_by_ipport(b1);
	cfg->set_server1_domain(utf8::w2a((LPCTSTR)(server1_domain)));
	cfg->set_server1_ip(server1_ip);
	cfg->set_server1_port(server1_port);

	cfg->set_server2_by_ipport(b2);
	cfg->set_server2_domain(utf8::w2a((LPCTSTR)(server2_domain)));
	cfg->set_server2_ip(server2_ip);
	cfg->set_server2_port(server2_port);

	m_ezviz_app_key.GetWindowTextW(txt);
	cfg->set_ezviz_private_cloud_app_key(utf8::w2a((LPCTSTR)(txt)));
	cfg->set_ezviz_private_cloud_by_ipport(b3);
	cfg->set_ezviz_private_cloud_domain(utf8::w2a((LPCTSTR)(ezviz_domain)));
	cfg->set_ezviz_private_cloud_ip(ezviz_ip);
	cfg->set_ezviz_private_cloud_port(ezviz_port);

	CDialogEx::OnOK();
}


BOOL CSetupNetworkDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_btnOK.GetWindowTextW(m_txtOk);

	auto cfg = util::CConfigHelper::get_instance();

	CString txt;
	txt.Format(L"%d", cfg->get_listening_port());
	m_listening_port.SetWindowTextW(txt);

	txt.Format(L"%d", cfg->get_server1_port());
	m_server1_port.SetWindowTextW(txt);
	
	txt.Format(L"%d", cfg->get_server2_port());
	m_server2_port.SetWindowTextW(txt);

	txt.Format(L"%d", cfg->get_ezviz_private_cloud_port());
	m_ezviz_port.SetWindowTextW(txt);

	m_csr_acct.SetWindowTextW(utf8::a2w(cfg->get_csr_acct()).c_str());
	m_server1_domain.SetWindowTextW(utf8::a2w(cfg->get_server1_domain()).c_str());
	m_server2_domain.SetWindowTextW(utf8::a2w(cfg->get_server2_domain()).c_str());
	m_ezviz_domain.SetWindowTextW(utf8::a2w(cfg->get_ezviz_private_cloud_domain()).c_str());
	m_server1_ip.SetWindowTextW(utf8::a2w(cfg->get_server1_ip()).c_str());
	m_server2_ip.SetWindowTextW(utf8::a2w(cfg->get_server2_ip()).c_str());
	m_ezviz_ip.SetWindowTextW(utf8::a2w(cfg->get_ezviz_private_cloud_ip()).c_str());
	m_ezviz_app_key.SetWindowTextW(utf8::a2w(cfg->get_ezviz_private_cloud_app_key()).c_str());
	
	detail::g_network_mode = cfg->get_network_mode();
	EnableWindows(detail::g_network_mode);
	
	SetTimer(1, 1000, nullptr);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CSetupNetworkDlg::EnableWindows(int mode)
{
	switch (mode) {
	case util::NETWORK_MODE_TRANSMIT:
		m_listening_port.EnableWindow(0);
		m_csr_acct.EnableWindow();
		m_server1_ip.EnableWindow();
		m_server1_port.EnableWindow();
		m_server2_ip.EnableWindow();
		m_server2_port.EnableWindow();
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
		m_listening_port.EnableWindow();
		m_csr_acct.EnableWindow();
		m_server1_ip.EnableWindow();
		m_server1_port.EnableWindow();
		m_server2_ip.EnableWindow();
		m_server2_port.EnableWindow();
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
		m_listening_port.EnableWindow();
		m_csr_acct.EnableWindow(0);
		m_server1_ip.EnableWindow(0);
		m_server1_port.EnableWindow(0);
		m_server2_ip.EnableWindow(0);
		m_server2_port.EnableWindow(0);
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

	auto cfg = util::CConfigHelper::get_instance();
	if (util::NETWORK_MODE_TRANSMIT & mode) {
		int man1 = cfg->get_server1_by_ipport();
		int man2 = cfg->get_server2_by_ipport();
		m_chkByIpPort1.SetCheck(man1);
		m_chkByIpPort2.SetCheck(man2);
		OnBnClickedCheckByIpport1();
		OnBnClickedCheckByIpport2();
	}

	int man3 = cfg->get_ezviz_private_cloud_by_ipport();
	m_chkByIpPort3.SetCheck(man3);
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
	//util::CConfigHelper::get_instance()->set_network_mode(detail::g_network_mode);
	EnableWindows(detail::g_network_mode);
}


void CSetupNetworkDlg::OnBnClickedRadioModeTransmit()
{
	detail::g_network_mode = util::NETWORK_MODE_TRANSMIT;
	//util::CConfigHelper::get_instance()->set_network_mode(detail::g_network_mode);
	EnableWindows(detail::g_network_mode);
}


void CSetupNetworkDlg::OnBnClickedRadioModeDual()
{
	detail::g_network_mode = util::NETWORK_MODE_DUAL;
	//util::CConfigHelper::get_instance()->set_network_mode(detail::g_network_mode);
	EnableWindows(detail::g_network_mode);
}


void CSetupNetworkDlg::OnBnClickedCheckByIpport1()
{
	BOOL b = m_chkByIpPort1.GetCheck();
	m_server1_domain.EnableWindow(!b);
	m_server1_ip.EnableWindow(b);
	m_server1_port.EnableWindow(b);
	m_btnTestDomain1.EnableWindow(!b);
}


void CSetupNetworkDlg::OnBnClickedCheckByIpport2()
{
	BOOL b = m_chkByIpPort2.GetCheck();
	m_server2_domain.EnableWindow(!b);
	m_server2_ip.EnableWindow(b);
	m_server2_port.EnableWindow(b);
	m_btnTestDomain2.EnableWindow(!b);
}

bool CSetupNetworkDlg::resolve_domain(int n, progress_cb cb)
{
	CString domain;
	wchar_t buffer[1024] = {};
	std::string title = utf8::w2a((LPCTSTR)GetStringFromAppResource(IDS_STRING_RESOLV_DOMAIN_FAIL));

	if (n == 1) {
		m_server1_domain.GetWindowTextW(domain);
		if (domain.IsEmpty()) {
			m_server1_domain.ShowBalloonTip(GetStringFromAppResource(IDS_STRING_ERROR), GetStringFromAppResource(IDS_STRING_CANT_BE_EMPTY), TTI_ERROR);
			return false;
		}
		std::string result;
		auto iter = domain_ip_map_.find((LPCTSTR)domain);
		if (iter != domain_ip_map_.end()) {
			result = iter->second;
		} else {
			if (!detail::get_domain_ip(utf8::w2a((LPCTSTR)domain), result, cb)) {
				if (!utf8::mbcs_to_u16(result.c_str(), buffer, 1024)) {
					MessageBoxA(m_hWnd, result.c_str(), title.c_str(), MB_ICONERROR);
				} else {
					m_server1_domain.ShowBalloonTip(GetStringFromAppResource(IDS_STRING_RESOLV_DOMAIN_FAIL), buffer, TTI_ERROR);
				}
				m_server1_ip.SetWindowText(L"");
				m_server1_port.SetWindowTextW(L"7892");
				return false;
			} else {
				domain_ip_map_[(LPCTSTR)domain] = result;
			}
		}

		m_server1_ip.SetWindowTextW(utf8::a2w(result).c_str());
		m_server1_port.SetWindowTextW(L"7892");

	} else if (n == 2) {
		m_server2_domain.GetWindowTextW(domain);
		if (domain.IsEmpty()) {
			m_server2_domain.ShowBalloonTip(GetStringFromAppResource(IDS_STRING_ERROR), GetStringFromAppResource(IDS_STRING_CANT_BE_EMPTY), TTI_ERROR);
			return false;
		}

		std::string result;
		auto iter = domain_ip_map_.find((LPCTSTR)domain);
		if (iter != domain_ip_map_.end()) {
			result = iter->second;
		} else {
			if (!detail::get_domain_ip(utf8::w2a((LPCTSTR)domain), result, cb)) {
				if (!utf8::mbcs_to_u16(result.c_str(), buffer, 1024)) {
					MessageBoxA(m_hWnd, result.c_str(), title.c_str(), MB_ICONERROR);
				} else {
					m_server2_domain.ShowBalloonTip(GetStringFromAppResource(IDS_STRING_RESOLV_DOMAIN_FAIL), buffer, TTI_ERROR);
				}
				m_server2_ip.SetWindowTextW(L"");
				m_server2_port.SetWindowTextW(L"7892");
				return false;
			} else {
				domain_ip_map_[(LPCTSTR)domain] = result;
			}
		}
		 
		m_server2_ip.SetWindowTextW(utf8::a2w(result).c_str());
		m_server2_port.SetWindowTextW(L"7892");

	} else if (n == 3) {
		m_ezviz_domain.GetWindowTextW(domain);
		if (domain.IsEmpty()) {
			m_ezviz_domain.ShowBalloonTip(GetStringFromAppResource(IDS_STRING_ERROR), GetStringFromAppResource(IDS_STRING_CANT_BE_EMPTY), TTI_ERROR);
			return false;
		}

		std::string result;
		auto iter = domain_ip_map_.find((LPCTSTR)domain);
		if (iter != domain_ip_map_.end()) {
			result = iter->second;
		} else {
			if (!detail::get_domain_ip(utf8::w2a((LPCTSTR)domain), result, cb)) {
				if (!utf8::mbcs_to_u16(result.c_str(), buffer, 1024)) {
					MessageBoxA(m_hWnd, result.c_str(), title.c_str(), MB_ICONERROR);
				} else {
					m_ezviz_domain.ShowBalloonTip(GetStringFromAppResource(IDS_STRING_RESOLV_DOMAIN_FAIL), buffer, TTI_ERROR);
				}
				m_ezviz_ip.SetWindowTextW(L"");
				m_ezviz_port.SetWindowTextW(L"12346");
				return false;
			} else {
				domain_ip_map_[(LPCTSTR)domain] = result;
			}
		}

		m_ezviz_ip.SetWindowTextW(utf8::a2w(result).c_str());
		m_ezviz_port.SetWindowTextW(L"12346");
	}

	return true;
}

void CSetupNetworkDlg::OnBnClickedButtonTestDomain1()
{
	auto local_cb = [this]() {
		CString txt = GetStringFromAppResource(IDS_STRING_TEST);
		static int step = 1;
		for (int i = 0; i < step; i++) {
			txt += L".";
		}
		m_btnTestDomain1.SetWindowTextW(txt);
		step++;
		if (step > 4) {
			step = 1;
		}
	};
	resolve_domain(1, local_cb);
	m_btnTestDomain1.SetWindowTextW(GetStringFromAppResource(IDS_STRING_TEST));
}


void CSetupNetworkDlg::OnBnClickedButtonTestDomain2()
{
	auto local_cb = [this]() {
		CString txt = GetStringFromAppResource(IDS_STRING_TEST);
		static int step = 1;
		for (int i = 0; i < step; i++) {
			txt += L".";
		}
		m_btnTestDomain2.SetWindowTextW(txt);
		step++;
		if (step > 4) {
			step = 1;
		}
	};
	resolve_domain(2);
	m_btnTestDomain2.SetWindowTextW(GetStringFromAppResource(IDS_STRING_TEST));
}


void CSetupNetworkDlg::OnBnClickedButtonTestDomain3()
{
	auto local_cb = [this]() {
		CString txt = GetStringFromAppResource(IDS_STRING_TEST);
		static int step = 1;
		for (int i = 0; i < step; i++) {
			txt += L".";
		}
		m_btnTestDomain3.SetWindowTextW(txt);
		step++;
		if (step > 4) {
			step = 1;
		}
	};
	resolve_domain(3);
	m_btnTestDomain3.SetWindowTextW(GetStringFromAppResource(IDS_STRING_TEST));
}


void CSetupNetworkDlg::OnBnClickedCheckByIpport3()
{
	BOOL b = m_chkByIpPort3.GetCheck();
	m_ezviz_domain.EnableWindow(!b);
	m_ezviz_ip.EnableWindow(b);
	m_ezviz_port.EnableWindow(b);
	m_btnTestDomain3.EnableWindow(!b);
}
