// QrcodeViewerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AlarmCenterInfoDlg.h"
#include "afxdialogex.h"
#include "md5.h"
#include "UserInfo.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include "CsrInfo.h"
#include "baidu.h"
#include "AutoSerialPort.h"
#include "Gsm.h"
#include "alarm_center_map_service.h"
#include "NetworkConnector.h"
#include "ConfigHelper.h"
#include "alarm_center_video_service.h"

#pragma comment(lib, "IPHLPAPI.lib")



// CAlarmCenterInfoDlg dialog

class CAlarmCenterInfoDlg::CurUserChangedObserver : public dp::observer<core::user_info_ptr>
{
public:
	explicit CurUserChangedObserver(CAlarmCenterInfoDlg* dlg) : _dlg(dlg) {}
	virtual void on_update(const core::user_info_ptr& ptr) {
		if (_dlg) {
			if (ptr->get_user_priority() == core::UP_OPERATOR) {
				_dlg->m_btnSavePrivateCloud.EnableWindow(0);
				_dlg->m_btnSaveNetworkInfo.EnableWindow(0);
			} else {
				_dlg->m_btnSavePrivateCloud.EnableWindow(1); 
				_dlg->m_btnSaveNetworkInfo.EnableWindow(util::CConfigHelper::get_instance()->get_network_mode() & util::NETWORK_MODE_TRANSMIT);
			}
			_dlg->InitAcct(ptr->get_user_priority());
		}
	}
private:
	CAlarmCenterInfoDlg* _dlg;
};

IMPLEMENT_DYNAMIC(CAlarmCenterInfoDlg, CDialogEx)

CAlarmCenterInfoDlg::CAlarmCenterInfoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CAlarmCenterInfoDlg::IDD, pParent)
	, m_acct_text(_T(""))
{

}

CAlarmCenterInfoDlg::~CAlarmCenterInfoDlg()
{
}

void CAlarmCenterInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_X, m_x);
	DDX_Control(pDX, IDC_EDIT_Y, m_y);
	DDX_Control(pDX, IDC_COMBO_COM, m_cmbCom);
	DDX_Control(pDX, IDC_BUTTON_CHECK_COM, m_btnCheckCom);
	DDX_Control(pDX, IDC_BUTTON_CONN_GSM, m_btnConnCom);
	DDX_Control(pDX, IDC_CHECK2, m_chkRemCom);
	DDX_Control(pDX, IDC_CHECK1, m_chkAutoConnCom);
	DDX_Control(pDX, IDC_EDIT_DTU_PHONE, m_phone);
	DDX_Control(pDX, IDC_IPADDRESS_PRIVATE_CLOUD, m_ip_private_cloud);
	DDX_Control(pDX, IDC_EDIT_PRIVATE_CLOUD, m_port_private_cloud);
	DDX_Control(pDX, IDC_BUTTON_SAVE_PRIVATE_CLOUD, m_btnSavePrivateCloud);
	DDX_Control(pDX, IDC_IPADDRESS_PRIVATE_CLOUD2, m_server_ip);
	DDX_Control(pDX, IDC_EDIT_PRIVATE_CLOUD2, m_server_port);
	DDX_Control(pDX, IDC_IPADDRESS_PRIVATE_CLOUD3, m_server_bk_ip);
	DDX_Control(pDX, IDC_EDIT_PRIVATE_CLOUD3, m_server_bk_port);
	DDX_Control(pDX, IDC_EDIT_PRIVATE_CLOUD4, m_listening_port);
	DDX_Control(pDX, IDC_BUTTON_SAVE_SERVER_INFO, m_btnSaveNetworkInfo);
	DDX_Control(pDX, IDC_EDIT_EZVIZ_APP_KEY, m_ezviz_app_key);
	DDX_Control(pDX, IDC_COMBO_APP_LANGUAGE, m_cmb_switch_language);
	DDX_Control(pDX, IDC_BUTTON_RESTART_APP, m_btnRestartApplication);
}


BEGIN_MESSAGE_MAP(CAlarmCenterInfoDlg, CDialogEx)
	ON_WM_SHOWWINDOW()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_LOCATE_AUTO, &CAlarmCenterInfoDlg::OnBnClickedButtonLocateAuto)
	ON_MESSAGE(WM_CHOSEN_BAIDU_PT, &CAlarmCenterInfoDlg::OnChosenBaiduPt)
	ON_BN_CLICKED(IDC_BUTTON_CHECK_COM, &CAlarmCenterInfoDlg::OnBnClickedButtonCheckCom)
	ON_BN_CLICKED(IDC_BUTTON_CONN_GSM, &CAlarmCenterInfoDlg::OnBnClickedButtonConnGsm)
	ON_BN_CLICKED(IDC_CHECK2, &CAlarmCenterInfoDlg::OnBnClickedCheck2)
	ON_BN_CLICKED(IDC_CHECK1, &CAlarmCenterInfoDlg::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_BUTTON_MGR_VIDEO_USER, &CAlarmCenterInfoDlg::OnBnClickedButtonMgrVideoUser)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_PRIVATE_CLOUD, &CAlarmCenterInfoDlg::OnBnClickedButtonSavePrivateCloud)
	ON_BN_CLICKED(IDC_BUTTON_SHOW_MAP, &CAlarmCenterInfoDlg::OnBnClickedButtonShowMap)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_SERVER_INFO, &CAlarmCenterInfoDlg::OnBnClickedButtonSaveServerInfo)
	ON_CBN_SELCHANGE(IDC_COMBO_APP_LANGUAGE, &CAlarmCenterInfoDlg::OnCbnSelchangeComboAppLanguage)
	ON_BN_CLICKED(IDC_BUTTON_RESTART_APP, &CAlarmCenterInfoDlg::OnBnClickedButtonRestartApp)
END_MESSAGE_MAP()


BOOL CAlarmCenterInfoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CenterWindow();
	
	//g_baiduMapDlg->m_pCsrInfoWnd = this;
	//InitAcct();
	InitLocation();
	InitCom();

	auto cfg = util::CConfigHelper::get_instance();
	m_ip_private_cloud.SetWindowTextW(utf8::a2w(cfg->get_ezviz_private_cloud_ip()).c_str());
	CString txt; 
	txt.Format(L"%d", cfg->get_ezviz_private_cloud_port());
	m_port_private_cloud.SetWindowTextW(txt);
	m_ezviz_app_key.SetWindowTextW(utf8::a2w(cfg->get_ezviz_private_cloud_app_key()).c_str());

	m_cur_user_changed_observer = std::make_shared<CurUserChangedObserver>(this);
	core::user_manager::get_instance()->register_observer(m_cur_user_changed_observer);
	m_cur_user_changed_observer->on_update(core::user_manager::get_instance()->GetCurUserInfo());

	m_cmb_switch_language.ResetContent();
	m_cmb_switch_language.AddString(L"简体中文");
	m_cmb_switch_language.AddString(L"繁体中文");
	m_cmb_switch_language.AddString(L"English");

	auto lang = cfg->get_current_language();
	m_cmb_switch_language.SetCurSel(lang);
	m_btnRestartApplication.EnableWindow(0);
	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CAlarmCenterInfoDlg::InitCom()
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
	m_cmbCom.SetCurSel(com);

	int auto_conn = cfg->get_auto_conn_com();
	if (rem && auto_conn) {
		m_chkAutoConnCom.SetCheck(1);
		OnBnClickedButtonConnGsm();
	} else {
		m_chkAutoConnCom.SetCheck(0);
	}


}


void CAlarmCenterInfoDlg::InitAcct(int user_priority)
{
	AUTO_LOG_FUNCTION;
	CString acct = utf8::a2w(util::CConfigHelper::get_instance()->get_csr_acct()).c_str();
	if (acct.IsEmpty()) {
	} else {
		m_phone.SetWindowTextW(acct);
		m_phone.SetReadOnly();

		if (user_priority == core::UP_OPERATOR) {
			m_phone.SetReadOnly(1);
		} else {
			m_phone.SetReadOnly(0);
		}
	}

	UpdateData(0);
}


void CAlarmCenterInfoDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
	if (bShow) {
		auto cfg = util::CConfigHelper::get_instance();
		auto listening_port = cfg->get_listening_port();
		auto ip = cfg->get_server1_ip();
		auto port = cfg->get_server1_port();
		auto ip_bk = cfg->get_server2_ip();
		auto port_bk = cfg->get_server2_port();
		
		CString txt;
		txt.Format(L"%d", listening_port);
		m_listening_port.SetWindowTextW(txt);
		m_server_ip.SetWindowTextW(utf8::a2w(ip).c_str());
		txt.Format(L"%d", port);
		m_server_port.SetWindowTextW(txt);
		m_server_bk_ip.SetWindowTextW(utf8::a2w(ip_bk).c_str());
		txt.Format(L"%d", port_bk);
		m_server_bk_port.SetWindowTextW(txt);

		auto mode = cfg->get_network_mode();
		switch (mode) {
		case util::NETWORK_MODE_TRANSMIT:
			m_phone.EnableWindow();
			m_server_ip.EnableWindow();
			m_server_port.EnableWindow();
			m_server_bk_ip.EnableWindow();
			m_server_bk_port.EnableWindow();
			m_btnSaveNetworkInfo.EnableWindow();
			break;
		case util::NETWORK_MODE_DUAL:
			m_phone.EnableWindow();
			m_server_ip.EnableWindow();
			m_server_port.EnableWindow();
			m_server_bk_ip.EnableWindow();
			m_server_bk_port.EnableWindow();
			m_btnSaveNetworkInfo.EnableWindow();
			break;
		case util::NETWORK_MODE_CSR:
			m_phone.EnableWindow(0);
			m_server_ip.EnableWindow(0);
			m_server_port.EnableWindow(0);
			m_server_bk_ip.EnableWindow(0);
			m_server_bk_port.EnableWindow(0);
			m_btnSaveNetworkInfo.EnableWindow(0);
		default:
			break;
		}

		auto csr = core::csr_manager::get_instance();
		auto coor = csr->get_coor();
		CString s;
		s.Format(L"%f", coor.x);
		m_x.SetWindowTextW(s);
		s.Format(L"%f", coor.y);
		m_y.SetWindowTextW(s);

		if (!m_cur_user_changed_observer) {
			m_cur_user_changed_observer = std::make_shared<CurUserChangedObserver>(this);
			core::user_manager::get_instance()->register_observer(m_cur_user_changed_observer);
		}
		m_cur_user_changed_observer->on_update(core::user_manager::get_instance()->GetCurUserInfo());
	}
}


void CAlarmCenterInfoDlg::InitLocation()
{
	AUTO_LOG_FUNCTION;
	auto csr = core::csr_manager::get_instance();
	web::BaiduCoordinate coor = csr->get_coor();
	if (coor.x == 0. && coor.y == 0.) {

	} else {
		CString s;
		s.Format(L"%f", coor.x);
		m_x.SetWindowTextW(s);
		s.Format(L"%f", coor.y);
		m_y.SetWindowTextW(s);
	}
}


void CAlarmCenterInfoDlg::OnBnClickedButtonShowMap()
{
	InitLocation();
	auto csr = core::csr_manager::get_instance();
	//if (g_baiduMapDlg)
	//	g_baiduMapDlg->ShowCsrMap(csr->get_coor(), csr->get_level());
	ipc::alarm_center_map_service::get_instance()->show_csr_map();
}


void CAlarmCenterInfoDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
}


// changed to see baidu map
void CAlarmCenterInfoDlg::OnBnClickedButtonLocateAuto()
{
	/*AUTO_LOG_FUNCTION;
	auto csr = core::csr_manager::get_instance();
	std::wstring addr;
	int city_code;
	web::BaiduCoordinate coor;
	if (web::CBaiduService::get_instance()->locate(addr, city_code, coor)) {
		csr->execute_set_addr(addr.c_str());
		csr->execute_set_city_code(city_code);
		csr->execute_set_coor(coor);
		csr->execute_set_zoom_level(14);

		m_addr.SetWindowTextW(addr.c_str());
		CString s;
		s.Format(L"%f", coor.x);
		m_x.SetWindowTextW(s);
		s.Format(L"%f", coor.y);
		m_y.SetWindowTextW(s); 

		g_baiduMapDlg->ShowCsrMap(coor, 14);
	} else { 
		CString e; e = TR(IDS_STRING_E_AUTO_LACATE_FAILED);
		MessageBox(e, L"", MB_ICONERROR);
	}
	core::user_manager::get_instance()->UnRegisterObserver(this);*/
}


void CAlarmCenterInfoDlg::OnBnClickedButtonLocateToAddr()
{

}


void CAlarmCenterInfoDlg::OnBnClickedButtonLocateToCoor()
{

}


afx_msg LRESULT CAlarmCenterInfoDlg::OnChosenBaiduPt(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	
	return 0;
}


void CAlarmCenterInfoDlg::OnBnClickedButtonCheckCom()
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


void CAlarmCenterInfoDlg::OnBnClickedButtonConnGsm()
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


void CAlarmCenterInfoDlg::SaveComConfigure(BOOL bRem, int nCom, BOOL bAuto)
{
	auto cfg = util::CConfigHelper::get_instance();
	cfg->set_remember_com_port(bRem);
	cfg->set_com_port(nCom);
	cfg->set_auto_conn_com(bAuto);
}


void CAlarmCenterInfoDlg::OnBnClickedCheck2()
{
	BOOL b1 = m_chkRemCom.GetCheck();
	if (!b1) {
		m_chkAutoConnCom.SetCheck(0);
		m_chkAutoConnCom.EnableWindow(0);
	} else {
		m_chkAutoConnCom.EnableWindow(1);
	}
	int ncom = m_cmbCom.GetCurSel();
	BOOL b2 = m_chkAutoConnCom.GetCheck();
	SaveComConfigure(b1, ncom, b2);
}


void CAlarmCenterInfoDlg::OnBnClickedCheck1()
{
	BOOL b1 = m_chkRemCom.GetCheck();
	int ncom = m_cmbCom.GetCurSel();
	BOOL b2 = m_chkAutoConnCom.GetCheck();
	SaveComConfigure(b1, ncom, b2);
}


void CAlarmCenterInfoDlg::OnBnClickedButtonMgrVideoUser()
{
	//m_videoUserMgrDlg->ShowWindow(SW_SHOW);
	ipc::alarm_center_video_service::get_instance()->show_video_user_mgr_dlg();
}


void CAlarmCenterInfoDlg::OnBnClickedButtonSavePrivateCloud()
{
	CString ip, port;
	m_ip_private_cloud.GetWindowTextW(ip);
	m_port_private_cloud.GetWindowTextW(port);

	bool updated = false;
	auto cfg = util::CConfigHelper::get_instance();
	if (cfg->get_ezviz_private_cloud_app_key() != utf8::w2a((LPCTSTR)ip)) {
		updated = true;
		cfg->set_ezviz_private_cloud_ip(utf8::w2a((LPCTSTR)ip));
	}

	if (cfg->get_ezviz_private_cloud_port() != static_cast<unsigned int>(_ttoi(port))) {
		updated = true;
		cfg->set_ezviz_private_cloud_port(_ttoi(port));
	}

	if (updated) {
		cfg->set_ezviz_private_cloud_by_ipport(0);
	}
}


void CAlarmCenterInfoDlg::OnBnClickedButtonSaveServerInfo()
{
	CString listening_port, ip, port, ip_bk, port_bk;
	m_listening_port.GetWindowTextW(listening_port);
	m_server_ip.GetWindowTextW(ip);
	m_server_port.GetWindowTextW(port);
	m_server_bk_ip.GetWindowTextW(ip_bk);
	m_server_bk_port.GetWindowTextW(port_bk);

	bool updated1 = false, updated2 = false;
	auto cfg = util::CConfigHelper::get_instance();

	if (util::CConfigHelper::get_instance()->get_network_mode() & util::NETWORK_MODE_TRANSMIT) {
		CString phone;
		m_phone.GetWindowTextW(phone);
		if (phone.GetLength() > 32) {
			return;
		}

		std::string phoneA = utf8::w2a((LPCTSTR)phone);

		auto csr_acct = cfg->get_csr_acct();
		if (phoneA.compare(csr_acct) != 0) {
			cfg->set_csr_acct(phoneA);
			core::user_info_ptr user = core::user_manager::get_instance()->GetCurUserInfo();
			InitAcct(user->get_user_priority());
			updated1 = updated2 = true;
		}
	}
	
	unsigned int n = _ttoi(listening_port);
	if (n != cfg->get_listening_port()) {
		updated1 = updated2 = true;
		cfg->set_listening_port(n);
	}
	
	std::string s = utf8::w2a((LPCTSTR)ip);
	if (s != cfg->get_server1_ip()) {
		updated1 = true;
		cfg->set_server1_ip(s);
	}
	
	n = _ttoi(port);
	if (n != cfg->get_server1_port()) {
		updated1 = true;
		cfg->set_server1_port(n);
	}

	s = utf8::w2a((LPCTSTR)(ip_bk));
	if (s != cfg->get_server2_ip()) {
		updated2 = true;
		cfg->set_server2_ip(utf8::w2a((LPCTSTR)(ip_bk)));
	}

	n = _ttoi(port_bk);
	if (n != cfg->get_server2_port()) {
		updated2 = true;
		cfg->set_server2_port(n);
	}

	if (cfg->get_network_mode() & util::NETWORK_MODE_TRANSMIT) {
		if (updated1) {
			cfg->set_server1_by_ipport(1);
			net::CNetworkConnector::get_instance()->RestartClient(net::server_1);
		}
		if (updated2) {
			cfg->set_server2_by_ipport(1);
			net::CNetworkConnector::get_instance()->RestartClient(net::server_2);
		}
	}

}


void CAlarmCenterInfoDlg::OnCbnSelchangeComboAppLanguage()
{
	int ndx = m_cmb_switch_language.GetCurSel(); if (ndx < 0)return;
	auto lang = util::Integer2ApplicationLanguage(ndx);
	auto cfg = util::CConfigHelper::get_instance();
	if (lang != cfg->get_current_language()) {
		cfg->set_language(lang);
		m_btnRestartApplication.EnableWindow();
	} else {
		m_btnRestartApplication.EnableWindow(0);
	}
}



void CAlarmCenterInfoDlg::OnBnClickedButtonRestartApp()
{
	QuitApplication(9959);
}
