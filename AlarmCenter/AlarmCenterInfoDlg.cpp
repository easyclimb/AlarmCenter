// QrcodeViewerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AlarmCenterInfoDlg.h"
#include "afxdialogex.h"
#include "md5.h"
//#include "QrCode.h"
#include "UserInfo.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include "CsrInfo.h"
#include "BaiduMapDlg.h"
#include "baidu.h"
#include "AutoSerialPort.h"
#include "Gsm.h"
#include "VideoUserManagerDlg.h"
#include "VideoManager.h"
#include "PrivateCloudConnector.h"
#include "BaiduMapViewerDlg.h"
//#include "simple_app.h"
//#include "simple_handler.h"
#include "NetworkConnector.h"
#include "ConfigHelper.h"
#include "AppResource.h"

//#ifdef _DEBUG
//#pragma comment(lib, "C:\\dev\\Global\\boost_1_58_0\\libs\\libboost_locale-vc120-mt-sgd-1_58.lib")
//#else
//#pragma comment(lib, "C:\\dev\\Global\\boost_1_58_0\\libs\\libboost_locale-vc120-mt-s-1_58.lib")
//#endif
//#include "C:/dev/Global/boost_1_58_0/boost/locale.hpp"

#include "tinyxml\tinyxml.h"

#ifdef _DEBUG
#include "ademco_func.h"
//#pragma comment(lib, "../Debug/Qrcode.lib")
#else
//#pragma comment(lib, "../Release/Qrcode.lib")
#endif
#pragma comment(lib, "IPHLPAPI.lib")

// CAlarmCenterInfoDlg dialog


class CAlarmCenterInfoDlg::CurUserChangedObserver : public dp::observer<core::CUserInfoPtr>
{
public:
	explicit CurUserChangedObserver(CAlarmCenterInfoDlg* dlg) : _dlg(dlg) {}
	virtual void on_update(const core::CUserInfoPtr& ptr) {
		if (_dlg) {
			if (ptr->get_user_priority() == core::UP_OPERATOR) {
				_dlg->m_btnAutoLocate.EnableWindow(0);
				_dlg->m_btnSavePrivateCloud.EnableWindow(0);
				_dlg->m_btnSaveNetworkInfo.EnableWindow(0);
			} else {
				_dlg->m_btnAutoLocate.EnableWindow(1);
				_dlg->m_btnSavePrivateCloud.EnableWindow(1); 
				_dlg->m_btnSaveNetworkInfo.EnableWindow(util::CConfigHelper::GetInstance()->get_network_mode() & util::NETWORK_MODE_TRANSMIT);
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
	//, m_map1(nullptr)
	, m_videoUserMgrDlg(nullptr)
{

}

CAlarmCenterInfoDlg::~CAlarmCenterInfoDlg()
{
}

void CAlarmCenterInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_ADDR, m_addr);
	DDX_Control(pDX, IDC_EDIT_X, m_x);
	DDX_Control(pDX, IDC_EDIT_Y, m_y);
	DDX_Control(pDX, IDC_COMBO_COM, m_cmbCom);
	DDX_Control(pDX, IDC_BUTTON_CHECK_COM, m_btnCheckCom);
	DDX_Control(pDX, IDC_BUTTON_CONN_GSM, m_btnConnCom);
	DDX_Control(pDX, IDC_CHECK2, m_chkRemCom);
	DDX_Control(pDX, IDC_CHECK1, m_chkAutoConnCom);
	DDX_Control(pDX, IDC_BUTTON_LOCATE_AUTO, m_btnAutoLocate);
	DDX_Control(pDX, IDC_EDIT_DTU_PHONE, m_phone);
	DDX_Control(pDX, IDC_BUTTON_SAVE_PHONE, m_btnSaveCsrAcct);
	DDX_Control(pDX, IDC_BUTTON3, m_btnTest);
	DDX_Control(pDX, IDC_IPADDRESS_PRIVATE_CLOUD, m_ip_private_cloud);
	DDX_Control(pDX, IDC_EDIT_PRIVATE_CLOUD, m_port_private_cloud);
	DDX_Control(pDX, IDC_BUTTON_SAVE_PRIVATE_CLOUD, m_btnSavePrivateCloud);
	DDX_Control(pDX, IDC_IPADDRESS_PRIVATE_CLOUD2, m_server_ip);
	DDX_Control(pDX, IDC_EDIT_PRIVATE_CLOUD2, m_server_port);
	DDX_Control(pDX, IDC_IPADDRESS_PRIVATE_CLOUD3, m_server_bk_ip);
	DDX_Control(pDX, IDC_EDIT_PRIVATE_CLOUD3, m_server_bk_port);
	DDX_Control(pDX, IDC_EDIT_PRIVATE_CLOUD4, m_listening_port);
	DDX_Control(pDX, IDC_BUTTON_SAVE_SERVER_INFO, m_btnSaveNetworkInfo);
}


BEGIN_MESSAGE_MAP(CAlarmCenterInfoDlg, CDialogEx)
	ON_WM_SHOWWINDOW()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_LOCATE_AUTO, &CAlarmCenterInfoDlg::OnBnClickedButtonLocateAuto)
	ON_BN_CLICKED(IDC_BUTTON_LOCATE_TO_ADDR, &CAlarmCenterInfoDlg::OnBnClickedButtonLocateToAddr)
	ON_BN_CLICKED(IDC_BUTTON_LOCATE_TO_COOR, &CAlarmCenterInfoDlg::OnBnClickedButtonLocateToCoor)
	ON_MESSAGE(WM_CHOSEN_BAIDU_PT, &CAlarmCenterInfoDlg::OnChosenBaiduPt)
	ON_BN_CLICKED(IDC_BUTTON_CHECK_COM, &CAlarmCenterInfoDlg::OnBnClickedButtonCheckCom)
	ON_BN_CLICKED(IDC_BUTTON_CONN_GSM, &CAlarmCenterInfoDlg::OnBnClickedButtonConnGsm)
	ON_BN_CLICKED(IDC_CHECK2, &CAlarmCenterInfoDlg::OnBnClickedCheck2)
	ON_BN_CLICKED(IDC_CHECK1, &CAlarmCenterInfoDlg::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_BUTTON2, &CAlarmCenterInfoDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_PHONE, &CAlarmCenterInfoDlg::OnBnClickedButtonSavePhone)
	ON_BN_CLICKED(IDC_BUTTON3, &CAlarmCenterInfoDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON_MGR_VIDEO_DEVICE, &CAlarmCenterInfoDlg::OnBnClickedButtonMgrVideoDevice)
	ON_BN_CLICKED(IDC_BUTTON_MGR_VIDEO_USER, &CAlarmCenterInfoDlg::OnBnClickedButtonMgrVideoUser)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_PRIVATE_CLOUD, &CAlarmCenterInfoDlg::OnBnClickedButtonSavePrivateCloud)
	ON_BN_CLICKED(IDC_BUTTON_SHOW_MAP, &CAlarmCenterInfoDlg::OnBnClickedButtonShowMap)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_SERVER_INFO, &CAlarmCenterInfoDlg::OnBnClickedButtonSaveServerInfo)
END_MESSAGE_MAP()


BOOL CAlarmCenterInfoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CenterWindow();
	
	g_baiduMapDlg->m_pCsrInfoWnd = this;
	//InitAcct();
	InitLocation();
	InitCom();

	USES_CONVERSION;
	video::ezviz::CPrivateCloudConnector* ezvizCloud = video::ezviz::CPrivateCloudConnector::GetInstance();
	m_ip_private_cloud.SetWindowTextW(A2W(ezvizCloud->get_ip().c_str()));
	CString txt; 
	txt.Format(L"%d", ezvizCloud->get_port());
	m_port_private_cloud.SetWindowTextW(txt);

	m_cur_user_changed_observer = std::make_shared<CurUserChangedObserver>(this);
	core::CUserManager::GetInstance()->register_observer(m_cur_user_changed_observer);
	m_cur_user_changed_observer->on_update(core::CUserManager::GetInstance()->GetCurUserInfo());

#ifndef _DEBUG
	m_btnTest.ShowWindow(SW_HIDE);
#endif

	m_videoUserMgrDlg = std::shared_ptr<CVideoUserManagerDlg>(new CVideoUserManagerDlg(this), 
															  [](CVideoUserManagerDlg* dlg) { SAFEDELETEDLG(dlg); });
	m_videoUserMgrDlg->Create(IDD_DIALOG_MGR_VIDEO_USER, this);

	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CAlarmCenterInfoDlg::InitCom()
{
	OnBnClickedButtonCheckCom();
	m_chkAutoConnCom.EnableWindow(0);

	USES_CONVERSION;
	std::string path = W2A(GetModuleFilePath());
	path += "\\config";
	CreateDirectoryA(path.c_str(), nullptr);
	path += "\\com.xml";
	using namespace tinyxml;
	do {
		TiXmlDocument doc(path.c_str());
		if (!doc.LoadFile(TIXML_ENCODING_UTF8)) {
			const char* e = doc.ErrorDesc(); e;
			break;
		}

		TiXmlElement *root = doc.RootElement();
		if (!root)break;
		root->GetText();

		TiXmlElement* cfg = root->FirstChildElement();
		if (!cfg)break;
		BOOL brem = FALSE;
		if (cfg->Attribute("rem", &brem)) {
			m_chkRemCom.SetCheck(brem);
			if (brem) {
				m_chkAutoConnCom.EnableWindow(1);
			}
		}

		int ncom = 0;
		if (cfg->Attribute("com", &ncom)) {
			m_cmbCom.SetCurSel(ncom);
		}

		BOOL bauto = FALSE;
		if (cfg->Attribute("auto", &bauto)) {
			if (brem && bauto) {
				m_chkAutoConnCom.SetCheck(1);
				OnBnClickedButtonConnGsm();
			} else {
				m_chkAutoConnCom.SetCheck(0);
			}
		}


	} while (0);
}


void CAlarmCenterInfoDlg::InitAcct(int user_priority)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	//core::CAlarmMachineManager* manager = core::CAlarmMachineManager::GetInstance();
	//core::CCsrInfo* csr = core::CCsrInfo::GetInstance();
	CString acct = A2W(util::CConfigHelper::GetInstance()->get_csr_acct().c_str());
	if (acct.IsEmpty()) {
		ShowWindow(SW_SHOW);
		//CString txt; txt = GetStringFromAppResource(IDS_STRING_INPUT_CSR_ACCT);
		//m_phone.MessageBox(txt, L"", MB_ICONINFORMATION);
		//m_phone.SetFocus();
		//m_phone.SetHighlight(0, 0);
	} else {
		m_phone.SetWindowTextW(acct);
		//m_phone.ModifyStyle(0, ES_READONLY);
		m_phone.SetReadOnly();
		//m_phone.UpdateWindow();
		//m_btnSaveCsrAcct.ShowWindow(SW_HIDE);

		if (user_priority == core::UP_OPERATOR) {
			m_phone.SetReadOnly(1);
			m_btnSaveCsrAcct.EnableWindow(0);
		} else {
			m_phone.SetReadOnly(0);
			m_btnSaveCsrAcct.EnableWindow(1);
		}
	}

	UpdateData(0);
}


void CAlarmCenterInfoDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
	USES_CONVERSION;
	if (bShow) {
		auto cfg = util::CConfigHelper::GetInstance();
		auto listening_port = cfg->get_listening_port();
		auto ip = cfg->get_server1_ip();
		auto port = cfg->get_server1_port();
		auto ip_bk = cfg->get_server2_ip();
		auto port_bk = cfg->get_server2_port();
		
		CString txt;
		txt.Format(L"%d", listening_port);
		m_listening_port.SetWindowTextW(txt);
		m_server_ip.SetWindowTextW(A2W(ip.c_str()));
		txt.Format(L"%d", port);
		m_server_port.SetWindowTextW(txt);
		m_server_bk_ip.SetWindowTextW(A2W(ip_bk.c_str()));
		txt.Format(L"%d", port_bk);
		m_server_bk_port.SetWindowTextW(txt);

		auto mode = cfg->get_network_mode();
		switch (mode) {
		case util::NETWORK_MODE_TRANSMIT:
			m_phone.EnableWindow();
			m_btnSaveCsrAcct.EnableWindow();
			m_server_ip.EnableWindow();
			m_server_port.EnableWindow();
			m_server_bk_ip.EnableWindow();
			m_server_bk_port.EnableWindow();
			m_btnSaveNetworkInfo.EnableWindow();
			break;
		case util::NETWORK_MODE_DUAL:
			m_phone.EnableWindow();
			m_btnSaveCsrAcct.EnableWindow();
			m_server_ip.EnableWindow();
			m_server_port.EnableWindow();
			m_server_bk_ip.EnableWindow();
			m_server_bk_port.EnableWindow();
			m_btnSaveNetworkInfo.EnableWindow();
			break;
		case util::NETWORK_MODE_CSR:
			m_phone.EnableWindow(0);
			m_btnSaveCsrAcct.EnableWindow(0);
			m_server_ip.EnableWindow(0);
			m_server_port.EnableWindow(0);
			m_server_bk_ip.EnableWindow(0);
			m_server_bk_port.EnableWindow(0);
			m_btnSaveNetworkInfo.EnableWindow(0);
		default:
			break;
		}
	}
}


void CAlarmCenterInfoDlg::InitLocation()
{
	AUTO_LOG_FUNCTION;
	core::CCsrInfo* csr = core::CCsrInfo::GetInstance();
	CString addr; /*double x, y;*/
	addr = csr->get_addr();
	//city_code = csr->get_city_code();
	web::BaiduCoordinate coor = csr->get_coor();
	//x = csr->get_x();
	//y = csr->get_y();
	if (coor.x == 0. && coor.y == 0.) {
		//OnBnClickedButtonLocateAuto();
	} else {
		m_addr.SetWindowTextW(addr);
		CString s;
		//s.Format(L"%d", city_code);
		s.Format(L"%f", coor.x);
		m_x.SetWindowTextW(s);
		s.Format(L"%f", coor.y);
		m_y.SetWindowTextW(s);
		//g_baiduMapDlg->ShowCsrMap(coor, csr->get_level());
	}
}


void CAlarmCenterInfoDlg::OnBnClickedButtonShowMap()
{
	InitLocation();
	core::CCsrInfo* csr = core::CCsrInfo::GetInstance();
	g_baiduMapDlg->ShowCsrMap(csr->get_coor(), csr->get_level());
}


void CAlarmCenterInfoDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
}


// changed to see baidu map
void CAlarmCenterInfoDlg::OnBnClickedButtonLocateAuto()
{
	/*AUTO_LOG_FUNCTION;
	core::CCsrInfo* csr = core::CCsrInfo::GetInstance();
	std::wstring addr;
	int city_code;
	web::BaiduCoordinate coor;
	if (web::CBaiduService::GetInstance()->locate(addr, city_code, coor)) {
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
		CString e; e = GetStringFromAppResource(IDS_STRING_E_AUTO_LACATE_FAILED);
		MessageBox(e, L"", MB_ICONERROR);
	}
	core::CUserManager::GetInstance()->UnRegisterObserver(this);*/
}


void CAlarmCenterInfoDlg::OnBnClickedButtonLocateToAddr()
{

}


void CAlarmCenterInfoDlg::OnBnClickedButtonLocateToCoor()
{

}


afx_msg LRESULT CAlarmCenterInfoDlg::OnChosenBaiduPt(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	web::BaiduCoordinate coor = g_baiduMapDlg->m_map->m_coor;
	int level = g_baiduMapDlg->m_map->m_zoomLevel;

	core::CCsrInfo* csr = core::CCsrInfo::GetInstance();
	web::BaiduCoordinate oldcoor(csr->get_coor());
	if (oldcoor == coor && csr->get_level() == level) {
		return 0;
	}

	if (csr->execute_set_coor(coor) && csr->execute_set_zoom_level(level)) {
		CString s;
		s.Format(L"%f", coor.x);
		m_x.SetWindowTextW(s);
		s.Format(L"%f", coor.y);
		m_y.SetWindowTextW(s);
		g_baiduMapDlg->ShowCsrMap(coor, level);
	}
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
		CString e; e = GetStringFromAppResource(IDS_STRING_NO_COM);
		MessageBox(e, nullptr, MB_ICONINFORMATION);
	}
}


void CAlarmCenterInfoDlg::OnBnClickedButtonConnGsm()
{
	CString open; open = GetStringFromAppResource(IDS_STRING_OPEN_COM);
	CString txt; m_btnConnCom.GetWindowTextW(txt);
	if (txt.Compare(open) == 0) {
		int ndx = m_cmbCom.GetCurSel();
		if (ndx < 0)return;
		int port = m_cmbCom.GetItemData(ndx);
		if (core::CGsm::GetInstance()->Open(port)) {
			m_cmbCom.EnableWindow(0);
			m_btnCheckCom.EnableWindow(0);
			CString close; close = GetStringFromAppResource(IDS_STRING_CLOSE_COM);
			m_btnConnCom.SetWindowTextW(close);
			m_chkRemCom.EnableWindow(0);
			m_chkAutoConnCom.EnableWindow(0);
		}
	} else {
		core::CGsm::GetInstance()->Close();
		m_btnConnCom.SetWindowTextW(open);
		m_cmbCom.EnableWindow(1);
		m_btnCheckCom.EnableWindow(1);
		m_chkRemCom.EnableWindow(1);
		m_chkAutoConnCom.EnableWindow(1);
	}
	
}


void CAlarmCenterInfoDlg::SaveComConfigure(BOOL bRem, int nCom, BOOL bAuto)
{
	USES_CONVERSION;
	
	std::string path = W2A(GetModuleFilePath());
	path += "\\config";
	CreateDirectoryA(path.c_str(), nullptr);
	path += "\\com.xml";
	using namespace tinyxml;
	TiXmlDocument doc;
	TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "", "");
	doc.LinkEndChild(decl);
	TiXmlElement *root = new TiXmlElement("SerialConfig"); // 不能有空白符
	doc.LinkEndChild(root);

	TiXmlElement* com = new TiXmlElement("cfg"); // 不能有空白符
	com->SetAttribute("rem", bRem);
	com->SetAttribute("com", nCom);
	com->SetAttribute("auto", bAuto);
	root->LinkEndChild(com);

	doc.SaveFile(path.c_str());
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


void CAlarmCenterInfoDlg::OnBnClickedButton2()
{
	static wchar_t i = 0;
	std::wstring phone(L"18240888101");
	//std::wstring content(L"Hello world! 我是中国人！实验室实验！");
	//content.push_back(i++ + L'a');
	wchar_t c[128] = { 0 };
	::LoadString(AfxGetInstanceHandle(), IDS_STRING_TEST, c, 128);
	std::wstring content = c;
	content.push_back(i++ + L'a');

	//WideCharToMultiByte()
	USES_CONVERSION_EX;
	//const char* a = W2A_EX(phone.c_str(), CP_UTF8);
	////const char* b = W2A_EX(content.c_str(), CP_UTF8);
	//const char* b = Utf16ToAnsi(content.c_str());

	//CGsm::GetInstance()->SendSms(std::string(a), 
	//							 std::string(b));

	//WideCharToMultiByte(CP_ACP, )
	//Utf8ToUtf16
	return;
	//try {
	//	std::string a_content = boost::locale::conv::from_utf(content, "UTF-16");
	//	//CGsm::GetInstance()->SendSms(std::string("18240888101"), a_content);
	//	//return;

	//	std::wstring u_content = boost::locale::conv::utf_to_utf<wchar_t>(a_content);
	//	std::string gbk_content = boost::locale::conv::between(a_content, "GBK", "UTF-16");

	//	std::fstream f; f.open("a_content", std::ios::out);
	//	if (f.is_open()) {
	//		f << a_content;
	//		f.close();
	//	}

	//	std::wfstream w; w.open("u_content", std::ios::out);
	//	if (w.is_open()) {
	//		w << content;
	//		w.close();
	//	}
	//} catch (...) {

	//}
}


void CAlarmCenterInfoDlg::OnBnClickedButtonSavePhone()
{
	USES_CONVERSION;
	CString phone;
	m_phone.GetWindowTextW(phone);
	if (phone.GetLength() > 32) {
		return;
	}

	std::string phoneA = W2A(phone);

	auto cfg = util::CConfigHelper::GetInstance();
	auto csr_acct = cfg->get_csr_acct();
	if (phoneA.compare(csr_acct) == 0)
		return;
	cfg->set_csr_acct(phoneA);
	core::CUserInfoPtr user = core::CUserManager::GetInstance()->GetCurUserInfo();
	InitAcct(user->get_user_priority());

	if(util::CConfigHelper::GetInstance()->get_network_mode() & util::NETWORK_MODE_TRANSMIT)
		net::CNetworkConnector::GetInstance()->RestartClient();
}


void CAlarmCenterInfoDlg::OnBnClickedButton3()
{
#ifdef _DEBUG
	ademco::AdemcoDataSegment data;
	data.Make(0, 0, 3400, 0);
	core::CGsm::GetInstance()->SendSms(L"18240888101", &data, L"布防");
#endif
}


void CAlarmCenterInfoDlg::OnBnClickedButtonMgrVideoDevice()
{

}


void CAlarmCenterInfoDlg::OnBnClickedButtonMgrVideoUser()
{
	/*CVideoUserManagerDlg dlg;
	dlg.DoModal();*/
	m_videoUserMgrDlg->ShowWindow(SW_SHOW);
}


void CAlarmCenterInfoDlg::OnBnClickedButtonSavePrivateCloud()
{
	USES_CONVERSION;
	CString ip, port;
	m_ip_private_cloud.GetWindowTextW(ip);
	m_port_private_cloud.GetWindowTextW(port);

	if (video::CVideoManager::GetInstance()->UpdatePrivateCloudInfo(W2A(ip), _ttoi(port))) {

	}
}


void CAlarmCenterInfoDlg::OnBnClickedButtonSaveServerInfo()
{
	USES_CONVERSION;
	CString listening_port, ip, port, ip_bk, port_bk;
	m_listening_port.GetWindowTextW(listening_port);
	m_server_ip.GetWindowTextW(ip);
	m_server_port.GetWindowTextW(port);
	m_server_bk_ip.GetWindowTextW(ip_bk);
	m_server_bk_port.GetWindowTextW(port_bk);

	bool updated = false;
	auto cfg = util::CConfigHelper::GetInstance();
	
	unsigned int n = _ttoi(listening_port);
	if (n != cfg->get_listening_port()) {
		updated = true;
		cfg->set_listening_port(n);
	}
	
	std::string s = W2A(ip);
	if (s != cfg->get_server1_ip()) {
		updated = true;
		cfg->set_server1_ip(s);
	}
	
	n = _ttoi(port);
	if (n != cfg->get_server1_port()) {
		updated = true;
		cfg->set_server1_port(n);
	}

	s = W2A(ip_bk);
	if (s != cfg->get_server2_ip()) {
		updated = true;
		cfg->set_server2_ip(W2A(ip_bk));
	}

	n = _ttoi(port_bk);
	if (n != cfg->get_server2_port()) {
		updated = true;
		cfg->set_server2_port(n);
	}

	if (updated && (cfg->get_network_mode() & util::NETWORK_MODE_TRANSMIT)) {
		net::CNetworkConnector::GetInstance()->RestartClient();
	}

}
