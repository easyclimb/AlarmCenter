// SetupNetworkDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "SetupNetworkDlg.h"
#include "afxdialogex.h"
#include "ConfigHelper.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <future>
#include "C:/dev/Global/net.h"
#include "AutoSerialPort.h"
#include "Gsm.h"
#include "congwin_fe100_mgr.h"

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

	/*bool is_domain(const CString& name) {
		CString domain = L"http://";
		domain += name;
		auto hr = IsValidURL(nullptr, domain, 0);
		return hr == S_OK;
	}*/

	std::pair<bool, std::string> get_domain_ip_impl(std::string domain) {
		AUTO_LOG_FUNCTION;
		boost::asio::io_service io_service;
		boost::asio::ip::tcp::resolver resolver(io_service);
		boost::asio::ip::tcp::resolver::query query(domain, "");
		
		try {
			std::string ip;
			std::string fastest_ip;
			long long fastest_ping_ms = 500000000;

			JLOGA("resolving domain:%s", domain.c_str());
			auto iter = resolver.resolve(query);
			boost::asio::ip::tcp::resolver::iterator end;
			while (iter != end) {
				boost::asio::ip::tcp::endpoint endpoint = *iter++;
				ip = endpoint.address().to_string();
				JLOGA("ping ip:%s", ip.c_str());

				pinger p(io_service, ip.c_str(), 3);
				io_service.run();

				auto ms = p.get_average();
				JLOGA("ip:%s 's average delay is %dms", ip.c_str(), ms);
				if (ms < fastest_ping_ms) {
					fastest_ping_ms = ms;
					fastest_ip = ip;
				}
			}

			JLOGA("fastest ip of domain:%s is %s", domain.c_str(), fastest_ip.c_str());

			return std::pair<bool, std::string>(true, fastest_ip);

		} catch (std::exception& e) {
			return std::pair<bool, std::string>(false, e.what());
		}
	}


	bool get_domain_ip(const std::string& domain, std::string& result, progress_cb cb) {
		auto f = std::async(std::launch::async, get_domain_ip_impl, domain);
		while (true) {
			auto status = f.wait_for(std::chrono::milliseconds(1));
			if (status == std::future_status::ready) {
				break;
			} else {
				auto start = std::chrono::steady_clock::now();
				MSG msg;
				while (GetMessage(&msg, nullptr, 0, 0)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);					

					std::this_thread::sleep_for(std::chrono::milliseconds(200));
					auto now = std::chrono::steady_clock::now();
					auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - start);
					if (diff.count() > 0) {
						if (cb) {
							cb();
						}
						break;
					}
				}
			}
		}

		auto ret = f.get();
		result = ret.second;
		return ret.first;
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
	DDX_Control(pDX, IDC_STATIC_CHOOSE_WORK_MODE, m_group_choose_work_mode);
	DDX_Control(pDX, IDC_STATIC_ALARM_CENTER_SETTINGS, m_group_direct_alarm_center_settings);
	DDX_Control(pDX, IDC_STATIC_TRANSMIT_SERVER_SETTINGS, m_group_transmit_server_settings);
	DDX_Control(pDX, IDC_STATIC_ALARM_CENTER_ACCOUNT, m_static_alarm_center_account);
	DDX_Control(pDX, IDC_STATIC_TRANS_1, m_static_trans1);
	DDX_Control(pDX, IDC_STATIC_TRANS2, m_static_trans2);
	DDX_Control(pDX, IDC_STATIC_EZVIZ_SETTINGS, m_group_ezviz_settings);
	DDX_Control(pDX, IDC_STATIC_EZVIZ_DOMAIN, m_static_ezviz_domain);
	DDX_Control(pDX, IDC_STATIC_EZVIZ_APP_KEY, m_static_ezviz_app_key);
	DDX_Control(pDX, IDC_STATIC_LISTENING_PORT, m_static_listening_port);
	DDX_Control(pDX, IDC_STATIC_SMS, m_group_sms_mod);
	DDX_Control(pDX, IDC_STATIC_COM, m_static_serial_port);
	DDX_Control(pDX, IDC_BUTTON_CHECK_COM, m_btnCheckCom);
	DDX_Control(pDX, IDC_BUTTON_CONN_GSM, m_btnConnCom);
	DDX_Control(pDX, IDC_CHECK2, m_chkRemCom);
	DDX_Control(pDX, IDC_CHECK1, m_chkAutoConnCom);
	DDX_Control(pDX, IDC_COMBO_COM, m_cmbCom);
	DDX_Control(pDX, IDC_CHECK8, m_chk_auto_conn_congwin_com);
	DDX_Control(pDX, IDC_CHECK7, m_chk_rem_congwin_com_port);
	DDX_Control(pDX, IDC_COMBO_COM2, m_cmb_congwin_com);
	DDX_Control(pDX, IDC_BUTTON_CHECK_COM2, m_btn_check_com2);
	DDX_Control(pDX, IDC_BUTTON_CONN_GSM2, m_btn_conn_congwin_com);
	DDX_Control(pDX, IDC_CHECK_RUN_AS_ROUTER, m_chk_run_as_router);
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
	ON_BN_CLICKED(IDC_BUTTON_CHECK_COM, &CSetupNetworkDlg::OnBnClickedButtonCheckCom)
	ON_BN_CLICKED(IDC_BUTTON_CONN_GSM, &CSetupNetworkDlg::OnBnClickedButtonConnGsm)
	ON_BN_CLICKED(IDC_CHECK2, &CSetupNetworkDlg::OnBnClickedCheck2)
	ON_BN_CLICKED(IDC_CHECK1, &CSetupNetworkDlg::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_BUTTON_CHECK_COM2, &CSetupNetworkDlg::OnBnClickedButtonCheckCom2)
	ON_BN_CLICKED(IDC_BUTTON_CONN_GSM2, &CSetupNetworkDlg::OnBnClickedButtonConnGsm2)
	ON_BN_CLICKED(IDC_CHECK7, &CSetupNetworkDlg::OnBnClickedCheck7)
	ON_BN_CLICKED(IDC_CHECK8, &CSetupNetworkDlg::OnBnClickedCheck8)
	ON_BN_CLICKED(IDC_CHECK_RUN_AS_ROUTER, &CSetupNetworkDlg::OnBnClickedCheckRunAsRouter)
END_MESSAGE_MAP()


// CSetupNetworkDlg message handlers


void CSetupNetworkDlg::OnBnClickedOk()
{
	if (resolving_) {
		return;
	}

	resolving_ = true;

	bool ok = false;

	do {

		EnableWindow(false);

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
			m_listening_port.ShowBalloonTip(TR(IDS_STRING_ERROR), TR(IDS_STRING_INVALID_PORT), TTI_ERROR);
			break;
		}

		m_csr_acct.GetWindowTextW(m_csracct);
		if (m_csracct.IsEmpty() && (detail::g_network_mode & util::NETWORK_MODE_TRANSMIT)) {
			m_csr_acct.ShowBalloonTip(TR(IDS_STRING_ERROR), TR(IDS_STRING_INPUT_CSR_ACCT), TTI_ERROR);
			break;
		}

		if ((ezviz_port < 1024 || ezviz_port > 65535)) {
			m_ezviz_port.ShowBalloonTip(TR(IDS_STRING_ERROR), TR(IDS_STRING_INVALID_PORT), TTI_ERROR);
			break;
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
					break;
				}
				m_server1_ip.GetWindowTextW(txt);
				server1_ip = utf8::w2a((LPCTSTR)(txt));
			}

			if (!server1_ip.empty() && server1_ip != "0.0.0.0") { // using
				if ((server1_port < 1024 || server1_port > 65535) && (detail::g_network_mode & util::NETWORK_MODE_TRANSMIT)) {
					m_server1_port.ShowBalloonTip(TR(IDS_STRING_ERROR), TR(IDS_STRING_INVALID_PORT), TTI_ERROR);
					break;
				}
			}

			if (!b2 || server2_ip.empty()/* || server2_ip == "0.0.0.0"*/) {
				if (!resolve_domain(2, local_cb)) {
					break;
				}
				m_server2_ip.GetWindowTextW(txt);
				server2_ip = utf8::w2a((LPCTSTR)(txt));
			}

			if (!server2_ip.empty() && server2_ip != "0.0.0.0") { // using
				if ((server2_port < 1024 || server2_port > 65535) && (detail::g_network_mode & util::NETWORK_MODE_TRANSMIT)) {
					m_server2_port.ShowBalloonTip(TR(IDS_STRING_ERROR), TR(IDS_STRING_INVALID_PORT), TTI_ERROR);
					break;
				}
			}
		}

		if (!b3 || ezviz_ip.empty()/* || ezviz_ip == "0.0.0.0"*/) {
			if (!resolve_domain(3, local_cb)) {
				break;
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

		ok = true;

	} while (0);

	resolving_ = false;
	EnableWindow(true);

	if (ok) {
		CDialogEx::OnOK();
	}
}


BOOL CSetupNetworkDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText(TR(IDS_STRING_SETUP_NETWORK));
	m_group_choose_work_mode.SetWindowTextW(TR(IDS_STRING_CHOOSE_WORK_MODE));
	m_radioCsr.SetWindowTextW(TR(IDS_STRING_DIRECT_MODE));
	m_radioTransmit.SetWindowTextW(TR(IDS_STRING_TRANSMIT_MODE));
	m_radioDual.SetWindowTextW(TR(IDS_STRING_BLEND_MODE));
	
	m_group_direct_alarm_center_settings.SetWindowTextW(TR(IDS_STRING_DIRECT_MODE_SETTINGS));
	m_static_listening_port.SetWindowTextW(TR(IDS_STRING_LISTENING_PORT));

	m_group_transmit_server_settings.SetWindowTextW(TR(IDS_STRING_TRANSMIT_SERVER_SETTINGS));
	m_static_alarm_center_account.SetWindowTextW(TR(IDS_STRING_ALARM_CENTER_ACCOUNT));
	m_static_trans1.SetWindowTextW(TR(IDS_STRING_TRANSMIT_SERVER_1));
	m_static_trans2.SetWindowTextW(TR(IDS_STRING_TRANSMIT_SERVER_2));
	m_chkByIpPort1.SetWindowTextW(TR(IDS_STRING_USE_IP_PORT));
	m_chkByIpPort2.SetWindowTextW(TR(IDS_STRING_USE_IP_PORT));
	m_chkByIpPort3.SetWindowTextW(TR(IDS_STRING_USE_IP_PORT));

	m_btnTestDomain1.SetWindowTextW(TR(IDS_STRING_TEST));
	m_btnTestDomain2.SetWindowTextW(TR(IDS_STRING_TEST));
	m_btnTestDomain3.SetWindowTextW(TR(IDS_STRING_TEST));

	m_group_ezviz_settings.SetWindowTextW(TR(IDS_STRING_EZVIZ_SETTINGS));
	m_static_ezviz_domain.SetWindowTextW(TR(IDS_STRING_EZVIZ_PRIVATE_CLOUD_DOMAIN));
	m_static_ezviz_app_key.SetWindowTextW(TR(IDS_STRING_EZVIZ_APP_KEY));
	m_btnOK.SetWindowTextW(TR(IDS_STRING_STARTUP_ALARM_CENTER));

	m_group_sms_mod.SetWindowTextW(TR(IDS_STRING_IDC_STATIC_041));
	m_static_serial_port.SetWindowTextW(TR(IDS_STRING_IDC_STATIC_042));
	m_btnCheckCom.SetWindowTextW(TR(IDS_STRING_IDC_BUTTON_CHECK_COM));
	m_btnConnCom.SetWindowTextW(TR(IDS_STRING_IDC_BUTTON_CONN_GSM));
	m_chkRemCom.SetWindowTextW(TR(IDS_STRING_REMEMBER_SERIAL_PORT));
	m_chkAutoConnCom.SetWindowTextW(TR(IDS_STRING_CONN_COM_ON_STARTUP));

	SET_WINDOW_TEXT(IDC_STATIC_FE100, IDS_STRING_CONGWIN_FE100);
	SET_WINDOW_TEXT(IDC_STATIC_COM2, IDS_STRING_IDC_STATIC_042);
	SET_WINDOW_TEXT(IDC_BUTTON_CHECK_COM2, IDS_STRING_IDC_BUTTON_CHECK_COM);
	SET_WINDOW_TEXT(IDC_BUTTON_CONN_GSM2, IDS_STRING_CONNECT_CONGWIN);
	SET_WINDOW_TEXT(IDC_CHECK7, IDS_STRING_REMEMBER_SERIAL_PORT);
	SET_WINDOW_TEXT(IDC_CHECK8, IDS_STRING_CONN_COM_ON_STARTUP);

	m_chk_run_as_router.SetWindowTextW(TR(IDS_STRING_RUN_AS_ROUTER));


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

	InitCom();
	
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
	std::string title = utf8::w2a((LPCTSTR)TR(IDS_STRING_RESOLV_DOMAIN_FAIL));

	if (n == 1) {
		m_server1_domain.GetWindowTextW(domain);
		if (domain.IsEmpty()) {
			m_server1_domain.ShowBalloonTip(TR(IDS_STRING_ERROR), TR(IDS_STRING_CANT_BE_EMPTY), TTI_ERROR);
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
					m_server1_domain.ShowBalloonTip(TR(IDS_STRING_RESOLV_DOMAIN_FAIL), buffer, TTI_ERROR);
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
			m_server2_domain.ShowBalloonTip(TR(IDS_STRING_ERROR), TR(IDS_STRING_CANT_BE_EMPTY), TTI_ERROR);
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
					m_server2_domain.ShowBalloonTip(TR(IDS_STRING_RESOLV_DOMAIN_FAIL), buffer, TTI_ERROR);
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
			m_ezviz_domain.ShowBalloonTip(TR(IDS_STRING_ERROR), TR(IDS_STRING_CANT_BE_EMPTY), TTI_ERROR);
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
					m_ezviz_domain.ShowBalloonTip(TR(IDS_STRING_RESOLV_DOMAIN_FAIL), buffer, TTI_ERROR);
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

void CSetupNetworkDlg::InitCom()
{
	OnBnClickedButtonCheckCom();
	m_chkAutoConnCom.EnableWindow(0);

	auto cfg = util::CConfigHelper::get_instance();
	int rem = cfg->get_remember_com_port();
	m_chkRemCom.SetCheck(rem);
	if (rem) {
		m_chkAutoConnCom.EnableWindow(1);
	}

	int com = cfg->get_com_port();
	for (int i = 0; i < m_cmbCom.GetCount(); i++) {
		if (m_cmbCom.GetItemData(i) == (DWORD)com) {
			m_cmbCom.SetCurSel(i);
		}
	}

	int auto_conn = cfg->get_auto_conn_com();
	if (rem && auto_conn) {
		m_chkAutoConnCom.SetCheck(1);
		OnBnClickedButtonConnGsm();
	} else {
		m_chkAutoConnCom.SetCheck(0);
	}

	OnBnClickedButtonCheckCom2();
	m_chk_auto_conn_congwin_com.EnableWindow(0);

	rem = cfg->get_remember_congwin_com_port();
	m_chk_rem_congwin_com_port.SetCheck(rem);
	if (rem) {
		m_chk_auto_conn_congwin_com.EnableWindow(1);
	}

	com = cfg->get_congwin_com_port();
	for (int i = 0; i < m_cmb_congwin_com.GetCount(); i++) {
		if (m_cmb_congwin_com.GetItemData(i) == (DWORD)com) {
			m_cmb_congwin_com.SetCurSel(i);
		}
	}

	auto_conn = cfg->get_auto_conn_congwin_com();
	if (rem && auto_conn) {
		m_chk_auto_conn_congwin_com.SetCheck(1);
		OnBnClickedButtonConnGsm2();
	} else {
		m_chk_auto_conn_congwin_com.SetCheck(0);
	}

	m_chk_run_as_router.SetCheck(util::CConfigHelper::get_instance()->get_congwin_fe100_router_mode());
}

void CSetupNetworkDlg::SaveComConfigure(BOOL bRem, int nCom, BOOL bAuto)
{
	auto cfg = util::CConfigHelper::get_instance();
	cfg->set_remember_com_port(bRem);
	cfg->set_com_port(nCom);
	cfg->set_auto_conn_com(bAuto);
}

void CSetupNetworkDlg::SaveCongwinComConfigure(BOOL bRem, int nCom, BOOL bAuto)
{
	auto cfg = util::CConfigHelper::get_instance();
	cfg->set_remember_congwin_com_port(bRem);
	cfg->set_congwin_com_port(nCom);
	cfg->set_auto_conn_congwin_com(bAuto);
}

void CSetupNetworkDlg::OnBnClickedButtonTestDomain1()
{
	if (resolving_) {
		return;
	}

	resolving_ = true;
	EnableWindow(false);

	auto local_cb = [this]() {
		CString txt = TR(IDS_STRING_IDC_BUTTON_TEST_DOMAIN1);
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
	m_btnTestDomain1.SetWindowTextW(TR(IDS_STRING_IDC_BUTTON_TEST_DOMAIN1));

	EnableWindow(true);
	resolving_ = false;
}


void CSetupNetworkDlg::OnBnClickedButtonTestDomain2()
{
	if (resolving_) {
		return;
	}

	resolving_ = true;
	EnableWindow(false);

	auto local_cb = [this]() {
		CString txt = TR(IDS_STRING_IDC_BUTTON_TEST_DOMAIN1);
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
	m_btnTestDomain2.SetWindowTextW(TR(IDS_STRING_IDC_BUTTON_TEST_DOMAIN1));

	EnableWindow(true);
	resolving_ = false;
}


void CSetupNetworkDlg::OnBnClickedButtonTestDomain3()
{
	if (resolving_) {
		return;
	}

	resolving_ = true;
	EnableWindow(false);

	auto local_cb = [this]() {
		CString txt = TR(IDS_STRING_IDC_BUTTON_TEST_DOMAIN1);
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
	m_btnTestDomain3.SetWindowTextW(TR(IDS_STRING_IDC_BUTTON_TEST_DOMAIN1));

	EnableWindow(true);
	resolving_ = false;
}


void CSetupNetworkDlg::OnBnClickedCheckByIpport3()
{
	BOOL b = m_chkByIpPort3.GetCheck();
	m_ezviz_domain.EnableWindow(!b);
	m_ezviz_ip.EnableWindow(b);
	m_ezviz_port.EnableWindow(b);
	m_btnTestDomain3.EnableWindow(!b);
}


void CSetupNetworkDlg::OnBnClickedButtonCheckCom()
{
	m_cmbCom.ResetContent();
	util::CAutoSerialPort ap;
	std::list<int> list;
	if (ap.CheckValidSerialPorts(list) && list.size() > 0) {
		CString str = L"";
		for (auto port : list) {
			str.Format(L"COM%d", port);
			int ndx = m_cmbCom.InsertString(-1, str);
			m_cmbCom.SetItemData(ndx, port);
		}
		m_cmbCom.SetCurSel(0);
	} else {
#ifndef _DEBUG
		CString e; e = TR(IDS_STRING_NO_COM);
		int ret = MessageBox(e, nullptr, MB_ICONINFORMATION | MB_OKCANCEL);
		if (IDOK != ret) {
			AfxGetMainWnd()->PostMessageW(WM_CLOSE);
		}
#endif
	}
}


void CSetupNetworkDlg::OnBnClickedButtonConnGsm()
{
	CString open; open = TR(IDS_STRING_IDC_BUTTON_CONN_GSM);
	CString txt; m_btnConnCom.GetWindowTextW(txt);
	if (txt.Compare(open) == 0) {
		int ndx = m_cmbCom.GetCurSel();
		if (ndx < 0)return;
		int port = m_cmbCom.GetItemData(ndx);
		if (core::gsm_manager::get_instance()->Open(port)) {
			m_cmbCom.EnableWindow(0);
			m_btnCheckCom.EnableWindow(0);
			CString close; close = TR(IDS_STRING_CLOSE_COM);
			m_btnConnCom.SetWindowTextW(close);
			m_chkRemCom.EnableWindow(0);
			m_chkAutoConnCom.EnableWindow(0);

			int rem = m_chkRemCom.GetCheck();
			if (rem) {
				SaveComConfigure(rem, port, m_chkAutoConnCom.GetCheck());
			}
		}
	} else {
		core::gsm_manager::get_instance()->Close();
		m_btnConnCom.SetWindowTextW(open);
		m_cmbCom.EnableWindow(1);
		m_btnCheckCom.EnableWindow(1);
		m_chkRemCom.EnableWindow(1);
		m_chkAutoConnCom.EnableWindow(1);
	}
}


void CSetupNetworkDlg::OnBnClickedCheck2()
{
	BOOL b1 = m_chkRemCom.GetCheck();
	if (!b1) {
		m_chkAutoConnCom.SetCheck(0);
		m_chkAutoConnCom.EnableWindow(0);
	} else {
		m_chkAutoConnCom.EnableWindow(1);
	}
	int ndx = m_cmbCom.GetCurSel();
	if (ndx < 0)return;
	int port = m_cmbCom.GetItemData(ndx);
	BOOL b2 = m_chkAutoConnCom.GetCheck();
	SaveComConfigure(b1, port, b2);
}


void CSetupNetworkDlg::OnBnClickedCheck1()
{
	BOOL b1 = m_chkRemCom.GetCheck();
	int ndx = m_cmbCom.GetCurSel();
	if (ndx < 0)return;
	int port = m_cmbCom.GetItemData(ndx);
	BOOL b2 = m_chkAutoConnCom.GetCheck();
	SaveComConfigure(b1, port, b2);
}


void CSetupNetworkDlg::OnBnClickedButtonCheckCom2()
{
	m_cmb_congwin_com.ResetContent();
	util::CAutoSerialPort ap;
	std::list<int> list;
	if (ap.CheckValidSerialPorts(list) && list.size() > 0) {
		CString str = L"";
		for (auto port : list) {
			str.Format(L"COM%d", port);
			int ndx = m_cmb_congwin_com.InsertString(-1, str);
			m_cmb_congwin_com.SetItemData(ndx, port);
		}
		m_cmb_congwin_com.SetCurSel(0);
	} else {
#ifndef _DEBUG
		CString e; e = TR(IDS_STRING_NO_COM);
		int ret = MessageBox(e, nullptr, MB_ICONINFORMATION | MB_OKCANCEL);
		if (IDOK != ret) {
			AfxGetMainWnd()->PostMessageW(WM_CLOSE);
		}
#endif
	}
}


void CSetupNetworkDlg::OnBnClickedButtonConnGsm2()
{
	CString open; open = TR(IDS_STRING_CONNECT_CONGWIN);
	CString txt; m_btn_conn_congwin_com.GetWindowTextW(txt);
	if (txt.Compare(open) == 0) {
		int ndx = m_cmb_congwin_com.GetCurSel();
		if (ndx < 0)return;
		int port = m_cmb_congwin_com.GetItemData(ndx);
		if (core::congwin_fe100_mgr::get_instance()->Open(port)) {
			m_cmb_congwin_com.EnableWindow(0);
			m_btn_check_com2.EnableWindow(0);
			CString close; close = TR(IDS_STRING_CLOSE_COM);
			m_btn_conn_congwin_com.SetWindowTextW(close);
			m_chk_rem_congwin_com_port.EnableWindow(0);
			m_chk_auto_conn_congwin_com.EnableWindow(0);

			int rem = m_chk_rem_congwin_com_port.GetCheck();
			if (rem) {
				SaveCongwinComConfigure(rem, port, m_chk_auto_conn_congwin_com.GetCheck());
			}
		}
	} else {
		core::gsm_manager::get_instance()->Close();
		m_btn_conn_congwin_com.SetWindowTextW(open);
		m_cmb_congwin_com.EnableWindow(1);
		m_btn_check_com2.EnableWindow(1);
		m_chk_rem_congwin_com_port.EnableWindow(1);
		m_chk_auto_conn_congwin_com.EnableWindow(1);
	}
}


void CSetupNetworkDlg::OnBnClickedCheck7()
{
	BOOL b1 = m_chk_rem_congwin_com_port.GetCheck();
	if (!b1) {
		m_chk_auto_conn_congwin_com.SetCheck(0);
		m_chk_auto_conn_congwin_com.EnableWindow(0);
	} else {
		m_chk_auto_conn_congwin_com.EnableWindow(1);
	}
	int ndx = m_cmb_congwin_com.GetCurSel();
	if (ndx < 0)return;
	int port = m_cmb_congwin_com.GetItemData(ndx);
	BOOL b2 = m_chk_auto_conn_congwin_com.GetCheck();
	SaveCongwinComConfigure(b1, port, b2);
}


void CSetupNetworkDlg::OnBnClickedCheck8()
{
	BOOL b1 = m_chk_rem_congwin_com_port.GetCheck();
	int ndx = m_cmb_congwin_com.GetCurSel();
	if (ndx < 0)return;
	int port = m_cmb_congwin_com.GetItemData(ndx);
	BOOL b2 = m_chk_auto_conn_congwin_com.GetCheck();
	SaveCongwinComConfigure(b1, port, b2);
}


void CSetupNetworkDlg::OnBnClickedCheckRunAsRouter()
{
	util::CConfigHelper::get_instance()->set_congwin_fe100_router_mode(m_chk_run_as_router.GetCheck());
}
