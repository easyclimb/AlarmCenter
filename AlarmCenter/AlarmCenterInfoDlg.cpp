﻿// QrcodeViewerDlg.cpp : implementation file
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
#include "congwin_fe100_mgr.h"

#pragma comment(lib, "IPHLPAPI.lib")



// CAlarmCenterInfoDlg dialog

class CAlarmCenterInfoDlg::CurUserChangedObserver : public dp::observer<core::user_info_ptr>
{
public:
	explicit CurUserChangedObserver(CAlarmCenterInfoDlg* dlg) : _dlg(dlg) {}
	virtual void on_update(const core::user_info_ptr& ptr) {
		if (_dlg) {
			if (ptr->get_priority() == core::UP_OPERATOR) {
				_dlg->m_btnSavePrivateCloud.EnableWindow(0);
				_dlg->m_btnSaveNetworkInfo.EnableWindow(0);
			} else {
				_dlg->m_btnSavePrivateCloud.EnableWindow(1); 
				_dlg->m_btnSaveNetworkInfo.EnableWindow(util::CConfigHelper::get_instance()->get_network_mode() & util::NETWORK_MODE_TRANSMIT);
			}
			_dlg->InitAcct(ptr->get_priority());
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
	DDX_Control(pDX, IDC_STATIC_CENTER_ADDR, m_group_center_addr);
	DDX_Control(pDX, IDC_STATIC_COOR, m_static_center_coor);
	DDX_Control(pDX, IDC_BUTTON_SHOW_MAP, m_btn_see_center_map);
	DDX_Control(pDX, IDC_STATIC_SMS, m_group_sms_mod);
	DDX_Control(pDX, IDC_STATIC_COM, m_static_serial_port);
	DDX_Control(pDX, IDC_STATIC_NETWORK, m_group_network);
	DDX_Control(pDX, IDC_STATIC_CSR_ACCT, m_static_csr_acct);
	DDX_Control(pDX, IDC_STATIC_NOTE, m_static_note);
	DDX_Control(pDX, IDC_STATIC_LISTENING_PORT, m_static_listening_port);
	DDX_Control(pDX, IDC_STATIC_SERVER1, m_static_server1);
	DDX_Control(pDX, IDC_STATIC_SERVER2, m_static_server2);
	DDX_Control(pDX, IDC_STATIC_EZVIZ, m_group_ezviz);
	DDX_Control(pDX, IDC_STATIC_EZVIZ_IP_PORT, m_static_ezviz_ip_port);
	DDX_Control(pDX, IDC_STATIC_EZVIZ_APP_KEY, m_static_ezviz_app_key);
	DDX_Control(pDX, IDC_STATIC_VIDEO, m_group_video);
	DDX_Control(pDX, IDC_BUTTON_MGR_VIDEO_USER, m_btn_video_user_mgr);
	DDX_Control(pDX, IDC_STATIC_LANG, m_group_language);
	DDX_Control(pDX, IDC_STATIC_CUR_LANG, m_static_cur_lang);
	DDX_Control(pDX, IDC_STATIC_REBOOT, m_static_note_reboot);
	DDX_Control(pDX, IDC_COMBO_COM2, m_cmb_congwin_com);
	DDX_Control(pDX, IDC_BUTTON_CHECK_COM2, m_btn_check_com2);
	DDX_Control(pDX, IDC_BUTTON_CONN_GSM2, m_btn_conn_congwin_com);
	DDX_Control(pDX, IDC_CHECK7, m_chk_rem_congwin_com_port);
	DDX_Control(pDX, IDC_CHECK8, m_chk_auto_conn_congwin_com);
	DDX_Control(pDX, IDC_CHECK9, m_chk_play_alarm);
	DDX_Control(pDX, IDC_CHECK10, m_chk_play_exception);
	DDX_Control(pDX, IDC_CHECK11, m_chk_play_offline);
	DDX_Control(pDX, IDC_RADIO1, m_radio_alarm_once);
	DDX_Control(pDX, IDC_RADIO2, m_radio_alarm_loop);
	DDX_Control(pDX, IDC_RADIO3, m_radio_ex_once);
	DDX_Control(pDX, IDC_RADIO4, m_radio_ex_loop);
	DDX_Control(pDX, IDC_RADIO5, m_radio_offline_once);
	DDX_Control(pDX, IDC_RADIO6, m_radio_offline_loop);

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
	ON_BN_CLICKED(IDC_BUTTON_CHECK_COM2, &CAlarmCenterInfoDlg::OnBnClickedButtonCheckCom2)
	ON_BN_CLICKED(IDC_BUTTON_CONN_GSM2, &CAlarmCenterInfoDlg::OnBnClickedButtonConnGsm2)
	ON_BN_CLICKED(IDC_CHECK7, &CAlarmCenterInfoDlg::OnBnClickedCheck7)
	ON_BN_CLICKED(IDC_CHECK8, &CAlarmCenterInfoDlg::OnBnClickedCheck8)
	ON_BN_CLICKED(IDC_CHECK9, &CAlarmCenterInfoDlg::OnBnClickedCheckPlayAlarm)
	ON_BN_CLICKED(IDC_RADIO1, &CAlarmCenterInfoDlg::OnBnClickedRadioAlarmOnce)
	ON_BN_CLICKED(IDC_RADIO2, &CAlarmCenterInfoDlg::OnBnClickedRadioAlarmLoop)
	ON_BN_CLICKED(IDC_CHECK10, &CAlarmCenterInfoDlg::OnBnClickedCheckPlayExcpt)
	ON_BN_CLICKED(IDC_RADIO3, &CAlarmCenterInfoDlg::OnBnClickedRadioExcptOnce)
	ON_BN_CLICKED(IDC_RADIO4, &CAlarmCenterInfoDlg::OnBnClickedRadioExcptLoop)
	ON_BN_CLICKED(IDC_CHECK11, &CAlarmCenterInfoDlg::OnBnClickedCheckPlayOffline)
	ON_BN_CLICKED(IDC_RADIO5, &CAlarmCenterInfoDlg::OnBnClickedRadioOfflineOnce)
	ON_BN_CLICKED(IDC_RADIO6, &CAlarmCenterInfoDlg::OnBnClickedRadioOfflineLoop)
END_MESSAGE_MAP()


BOOL CAlarmCenterInfoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CenterWindow();

	SetWindowText(TR(IDS_STRING_IDD_DIALOG_CSR_ACCT));
	m_group_center_addr.SetWindowTextW(TR(IDS_STRING_IDC_STATIC_040));
	m_static_center_coor.SetWindowTextW(TR(IDS_STRING_IDC_STATIC_054));
	m_btn_see_center_map.SetWindowTextW(TR(IDS_STRING_IDC_BUTTON_SHOW_MAP));

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

	m_group_network.SetWindowTextW(TR(IDS_STRING_IDC_STATIC_039));
	m_static_csr_acct.SetWindowTextW(TR(IDS_STRING_IDC_STATIC_047));
	m_static_note.SetWindowTextW(TR(IDS_STRING_IDC_STATIC_043));
	m_static_listening_port.SetWindowTextW(TR(IDS_STRING_IDC_STATIC_001));
	m_static_server1.SetWindowTextW(TR(IDS_STRING_IDC_STATIC_000));
	m_static_server2.SetWindowTextW(TR(IDS_STRING_IDC_STATIC_007));
	m_btnSaveNetworkInfo.SetWindowTextW(TR(IDS_STRING_IDC_BUTTON_SAVE_CHANGE));

	m_group_ezviz.SetWindowTextW(TR(IDS_STRING_IDC_STATIC_049));
	m_static_ezviz_ip_port.SetWindowTextW(TR(IDS_STRING_IDC_STATIC_050));
	SET_WINDOW_TEXT(IDC_STATIC_EZVIZ_APP_KEY, IDS_STRING_IDC_STATIC_051);
	SET_WINDOW_TEXT(IDC_BUTTON_SAVE_PRIVATE_CLOUD, IDS_STRING_IDC_BUTTON_SAVE_CHANGE);

	SET_WINDOW_TEXT(IDC_STATIC_VIDEO, IDS_STRING_IDC_STATIC_048);
	SET_WINDOW_TEXT(IDC_BUTTON_MGR_VIDEO_USER, IDS_STRING_IDC_BUTTON_MGR_VIDEO_USER);

	SET_WINDOW_TEXT(IDC_STATIC_LANG, IDS_STRING_IDC_STATIC_052);
	SET_WINDOW_TEXT(IDC_STATIC_CUR_LANG, IDS_STRING_IDC_STATIC_053);
	SET_CLASSIC_WINDOW_TEXT(IDC_BUTTON_RESTART_APP);// , IDS_STRING_IDC_BUTTON_RESTART_APP);
	SET_CLASSIC_WINDOW_TEXT(IDC_STATIC_REBOOT);

	

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
	m_cur_user_changed_observer->on_update(core::user_manager::get_instance()->get_cur_user_info());

	m_cmb_switch_language.ResetContent();
	m_cmb_switch_language.AddString(L"简体中文");
	m_cmb_switch_language.AddString(L"繁体中文");
	m_cmb_switch_language.AddString(L"English");

	auto lang = cfg->get_current_language();
	m_cmb_switch_language.SetCurSel(lang);
	m_btnRestartApplication.EnableWindow(0);

	//m_cmb_switch_language.EnableWindow(0);
	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CAlarmCenterInfoDlg::InitCom()
{
	auto cfg = util::CConfigHelper::get_instance();

	if (core::gsm_manager::get_instance()->is_open()) {
		int port = core::gsm_manager::get_instance()->get_port();
		CString txt;
		txt.Format(L"COM%d", port);
		int ndx = m_cmbCom.AddString(txt);
		m_cmbCom.SetItemData(ndx, port);
		m_cmbCom.SetCurSel(ndx);
		m_chkRemCom.SetCheck(cfg->get_remember_com_port());
		m_chkAutoConnCom.SetCheck(cfg->get_auto_conn_com());
		OnBnClickedButtonConnGsm();
	} else {
		OnBnClickedButtonCheckCom();
		m_chkAutoConnCom.EnableWindow(0);

		
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
	}

	if (core::congwin_fe100_mgr::get_instance()->is_open()) {
		int port = core::congwin_fe100_mgr::get_instance()->get_port();
		CString txt;
		txt.Format(L"COM%d", port);
		int ndx = m_cmb_congwin_com.AddString(txt);
		m_cmb_congwin_com.SetItemData(ndx, port);
		m_cmb_congwin_com.SetCurSel(ndx);
		m_chk_rem_congwin_com_port.SetCheck(cfg->get_remember_congwin_com_port());
		m_chk_auto_conn_congwin_com.SetCheck(cfg->get_auto_conn_congwin_com());
		OnBnClickedButtonConnGsm2();
	} else {
		OnBnClickedButtonCheckCom2();
		m_chk_auto_conn_congwin_com.EnableWindow(0);

		int rem = cfg->get_remember_congwin_com_port();
		m_chk_rem_congwin_com_port.SetCheck(rem);
		if (rem) {
			m_chk_auto_conn_congwin_com.EnableWindow(1);
		}

		int com = cfg->get_congwin_com_port();
		for (int i = 0; i < m_cmb_congwin_com.GetCount(); i++) {
			if (m_cmb_congwin_com.GetItemData(i) == (DWORD)com) {
				m_cmb_congwin_com.SetCurSel(i);
			}
		}

		int auto_conn = cfg->get_auto_conn_congwin_com();
		if (rem && auto_conn) {
			m_chk_auto_conn_congwin_com.SetCheck(1);
			OnBnClickedButtonConnGsm2();
		} else {
			m_chk_auto_conn_congwin_com.SetCheck(0);
		}
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
		m_cur_user_changed_observer->on_update(core::user_manager::get_instance()->get_cur_user_info());

		// sound settings
		update_alarm_settings();
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

#ifdef _DEBUG
	auto gsm = core::gsm_manager::get_instance();
	//const char* data = "+CMS:0123456789012345678       #000024|1|1110 00 001\r\n\r\n";
	//const char* data = "+CMS:0123456789012345678       #000024x1 1110 00 001\r\n\r\n";
	const char* data = "+CMS:8618658257969       010101#000006*2%1130 00 006%1130 00 006\r\n\r\n";
	gsm->test_input(data, strlen(data));
#endif
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


void CAlarmCenterInfoDlg::SaveComConfigure(BOOL bRem, int nCom, BOOL bAuto)
{
	auto cfg = util::CConfigHelper::get_instance();
	cfg->set_remember_com_port(bRem);
	cfg->set_com_port(nCom);
	cfg->set_auto_conn_com(bAuto);
}

void CAlarmCenterInfoDlg::SaveCongwinComConfigure(BOOL bRem, int nCom, BOOL bAuto)
{
	auto cfg = util::CConfigHelper::get_instance();
	cfg->set_remember_congwin_com_port(bRem);
	cfg->set_congwin_com_port(nCom);
	cfg->set_auto_conn_congwin_com(bAuto);
}

void CAlarmCenterInfoDlg::update_alarm_settings()
{
	auto cfg = util::CConfigHelper::get_instance();

	// alarm
	{
		int play_alarm_sound = cfg->get_play_alarm_sound();
		int play_alarm_loop = cfg->get_play_alarm_loop();
		m_chk_play_alarm.SetCheck(play_alarm_sound);
		m_radio_alarm_once.SetCheck(!play_alarm_loop);
		m_radio_alarm_loop.SetCheck(play_alarm_loop);
		m_radio_alarm_once.EnableWindow(play_alarm_sound);
		m_radio_alarm_loop.EnableWindow(play_alarm_sound);
	}

	// exception
	{
		int play_exception_sound = cfg->get_play_exception_sound();
		int play_exception_loop = cfg->get_play_exception_loop();
		m_chk_play_exception.SetCheck(play_exception_sound);
		m_radio_ex_once.SetCheck(!play_exception_loop);
		m_radio_ex_loop.SetCheck(play_exception_loop);
		m_radio_ex_once.EnableWindow(play_exception_sound);
		m_radio_ex_loop.EnableWindow(play_exception_sound);
	}

	// offline
	{
		int play_offline_sound = cfg->get_play_offline_sound();
		int play_offline_loop = cfg->get_play_offline_loop();
		m_chk_play_offline.SetCheck(play_offline_sound);
		m_radio_offline_once.SetCheck(!play_offline_loop);
		m_radio_offline_loop.SetCheck(play_offline_loop);
		m_radio_offline_once.EnableWindow(play_offline_sound);
		m_radio_offline_loop.EnableWindow(play_offline_sound);
	}
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
	int ndx = m_cmbCom.GetCurSel();
	if (ndx < 0)return;
	int port = m_cmbCom.GetItemData(ndx);
	BOOL b2 = m_chkAutoConnCom.GetCheck();
	SaveComConfigure(b1, port, b2);
}


void CAlarmCenterInfoDlg::OnBnClickedCheck1()
{
	BOOL b1 = m_chkRemCom.GetCheck();
	int ndx = m_cmbCom.GetCurSel();
	if (ndx < 0)return;
	int port = m_cmbCom.GetItemData(ndx);
	BOOL b2 = m_chkAutoConnCom.GetCheck();
	SaveComConfigure(b1, port, b2);


}


void CAlarmCenterInfoDlg::OnBnClickedButtonCheckCom2()
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


void CAlarmCenterInfoDlg::OnBnClickedButtonConnGsm2()
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
		core::congwin_fe100_mgr::get_instance()->Close();
		m_btn_conn_congwin_com.SetWindowTextW(open);
		m_cmb_congwin_com.EnableWindow(1);
		m_btn_check_com2.EnableWindow(1);
		m_chk_rem_congwin_com_port.EnableWindow(1);
		m_chk_auto_conn_congwin_com.EnableWindow(1);
	}
}


void CAlarmCenterInfoDlg::OnBnClickedCheck7()
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


void CAlarmCenterInfoDlg::OnBnClickedCheck8()
{
	BOOL b1 = m_chk_rem_congwin_com_port.GetCheck();
	int ndx = m_cmb_congwin_com.GetCurSel();
	if (ndx < 0)return;
	int port = m_cmb_congwin_com.GetItemData(ndx);
	BOOL b2 = m_chk_auto_conn_congwin_com.GetCheck();
	SaveCongwinComConfigure(b1, port, b2);
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
			core::user_info_ptr user = core::user_manager::get_instance()->get_cur_user_info();
			InitAcct(user->get_priority());
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
	util::CConfigHelper::get_instance()->save_to_file();
	QuitApplication(9959);
}




void CAlarmCenterInfoDlg::OnBnClickedCheckPlayAlarm()
{
	int chk = m_chk_play_alarm.GetCheck();
	util::CConfigHelper::get_instance()->set_play_alarm_sound(chk);
	m_radio_alarm_once.EnableWindow(chk);
	m_radio_alarm_loop.EnableWindow(chk);
}


void CAlarmCenterInfoDlg::OnBnClickedRadioAlarmOnce()
{
	int chk = m_radio_alarm_once.GetCheck();
	util::CConfigHelper::get_instance()->set_play_alarm_loop(!chk);
}


void CAlarmCenterInfoDlg::OnBnClickedRadioAlarmLoop()
{
	int chk = m_radio_alarm_loop.GetCheck();
	util::CConfigHelper::get_instance()->set_play_alarm_loop(chk);
}


void CAlarmCenterInfoDlg::OnBnClickedCheckPlayExcpt()
{
	int chk = m_chk_play_exception.GetCheck();
	util::CConfigHelper::get_instance()->set_play_exception_sound(chk);
	m_radio_ex_once.EnableWindow(chk);
	m_radio_ex_loop.EnableWindow(chk);
}


void CAlarmCenterInfoDlg::OnBnClickedRadioExcptOnce()
{
	int chk = m_radio_ex_once.GetCheck();
	util::CConfigHelper::get_instance()->set_play_exception_loop(!chk);
}


void CAlarmCenterInfoDlg::OnBnClickedRadioExcptLoop()
{
	int chk = m_radio_ex_loop.GetCheck();
	util::CConfigHelper::get_instance()->set_play_exception_loop(chk);
}


void CAlarmCenterInfoDlg::OnBnClickedCheckPlayOffline()
{
	int chk = m_chk_play_offline.GetCheck();
	util::CConfigHelper::get_instance()->set_play_offline_sound(chk);
	m_radio_offline_once.EnableWindow(chk);
	m_radio_offline_loop.EnableWindow(chk);
}


void CAlarmCenterInfoDlg::OnBnClickedRadioOfflineOnce()
{
	int chk = m_radio_offline_once.GetCheck();
	util::CConfigHelper::get_instance()->set_play_offline_loop(!chk);
}


void CAlarmCenterInfoDlg::OnBnClickedRadioOfflineLoop()
{
	int chk = m_radio_offline_loop.GetCheck();
	util::CConfigHelper::get_instance()->set_play_offline_loop(chk);
}
