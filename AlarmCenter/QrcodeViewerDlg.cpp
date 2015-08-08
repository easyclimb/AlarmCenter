// QrcodeViewerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "QrcodeViewerDlg.h"
#include "afxdialogex.h"
#include "md5.h"
#include "QrCode.h"
#include "UserInfo.h"

#include <iostream>
#include <algorithm>
#include <fstream>
#include "CsrInfo.h"
#include "BaiduMapDlg.h"
#include "baidu.h"
#include "AutoSerialPort.h"
#include "Gsm.h"
//#ifdef _DEBUG
//#pragma comment(lib, "C:\\dev\\Global\\boost_1_58_0\\libs\\libboost_locale-vc120-mt-sgd-1_58.lib")
//#else
//#pragma comment(lib, "C:\\dev\\Global\\boost_1_58_0\\libs\\libboost_locale-vc120-mt-s-1_58.lib")
//#endif
//#include "C:/dev/Global/boost_1_58_0/boost/locale.hpp"

#include "tinyxml\tinyxml.h"

#ifdef _DEBUG
#pragma comment(lib, "../Debug/Qrcode.lib")
#else
#pragma comment(lib, "../Release/Qrcode.lib")
#endif
#pragma comment(lib, "IPHLPAPI.lib")

// CQrcodeViewerDlg dialog

IMPLEMENT_DYNAMIC(CQrcodeViewerDlg, CDialogEx)

CQrcodeViewerDlg::CQrcodeViewerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CQrcodeViewerDlg::IDD, pParent)
	, m_acct_text(_T(""))
	, m_map1(NULL)
{

}

CQrcodeViewerDlg::~CQrcodeViewerDlg()
{
}

void CQrcodeViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_BMP, m_staticEx);
	DDX_Text(pDX, IDC_EDIT_CSR_ACCT, m_acct_text);
	DDX_Control(pDX, IDC_STATIC_BAIDU_MAP, m_staticBaiduMap);
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
}


BEGIN_MESSAGE_MAP(CQrcodeViewerDlg, CDialogEx)
	ON_WM_SHOWWINDOW()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_LOCATE_AUTO, &CQrcodeViewerDlg::OnBnClickedButtonLocateAuto)
	ON_BN_CLICKED(IDC_BUTTON_LOCATE_TO_ADDR, &CQrcodeViewerDlg::OnBnClickedButtonLocateToAddr)
	ON_BN_CLICKED(IDC_BUTTON_LOCATE_TO_COOR, &CQrcodeViewerDlg::OnBnClickedButtonLocateToCoor)
	ON_MESSAGE(WM_CHOSEN_BAIDU_PT, &CQrcodeViewerDlg::OnChosenBaiduPt)
	ON_BN_CLICKED(IDC_BUTTON_CHECK_COM, &CQrcodeViewerDlg::OnBnClickedButtonCheckCom)
	ON_BN_CLICKED(IDC_BUTTON_CONN_GSM, &CQrcodeViewerDlg::OnBnClickedButtonConnGsm)
	ON_BN_CLICKED(IDC_CHECK2, &CQrcodeViewerDlg::OnBnClickedCheck2)
	ON_BN_CLICKED(IDC_CHECK1, &CQrcodeViewerDlg::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_BUTTON2, &CQrcodeViewerDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_PHONE, &CQrcodeViewerDlg::OnBnClickedButtonSavePhone)
END_MESSAGE_MAP()


// CQrcodeViewerDlg message handlers

void __stdcall CQrcodeViewerDlg::OnCurUserChanged(void* udata, const core::CUserInfo* user)
{
	if (!udata || !user)
		return;

	CQrcodeViewerDlg* dlg = reinterpret_cast<CQrcodeViewerDlg*>(udata);
	if (user->get_user_priority() == core::UP_OPERATOR) {
		dlg->m_btnAutoLocate.EnableWindow(0);
	} else {
		dlg->m_btnAutoLocate.EnableWindow(1);
	}
	dlg->InitAcct();
}


BOOL CQrcodeViewerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CenterWindow();
	
	m_map1 = new CBaiduMapDlg();
	m_map1->m_pRealParent = this;
	m_map1->Create(IDD_DIALOG_BAIDU_MAP, &m_staticBaiduMap);
	CRect rc;
	m_staticBaiduMap.GetClientRect(rc);
	rc.DeflateRect(5, 25, 5, 5);
	m_map1->MoveWindow(rc, FALSE);
	m_map1->ShowWindow(SW_SHOW);

	//InitAcct();
	InitLocation();
	InitCom();

	core::CUserManager::GetInstance()->RegisterObserver(this, OnCurUserChanged);
	OnCurUserChanged(this, core::CUserManager::GetInstance()->GetCurUserInfo());

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CQrcodeViewerDlg::InitCom()
{
	OnBnClickedButtonCheckCom();
	m_chkAutoConnCom.EnableWindow(0);

	USES_CONVERSION;
	std::string path = W2A(GetModuleFilePath());
	path += "\\config";
	CreateDirectoryA(path.c_str(), NULL);
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


void CQrcodeViewerDlg::InitAcct()
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	//core::CAlarmMachineManager* manager = core::CAlarmMachineManager::GetInstance();
	core::CCsrInfo* csr = core::CCsrInfo::GetInstance();
	CString acct = csr->get_acct();
	if (acct.IsEmpty()) {
		ShowWindow(SW_SHOW);
		CString txt; txt.LoadStringW(IDS_STRING_INPUT_CSR_ACCT);
		m_phone.MessageBox(txt, L"", MB_ICONINFORMATION);
		m_phone.SetFocus();
		//m_phone.SetHighlight(0, 0);
	} else {
		m_phone.SetWindowTextW(acct);
		//m_phone.ModifyStyle(0, ES_READONLY);
		m_phone.SetReadOnly();
		//m_phone.UpdateWindow();
		m_btnSaveCsrAcct.ShowWindow(SW_HIDE);
	}

	//CString path(L"");
	//path.Format(L"%s\\config", GetModuleFilePath());
	//CreateDirectory(path, NULL);
	//m_md5_path.Format(_T("%s\\acct.md5"), path);
	//m_bmp_path.Format(_T("%s\\acct.bmp"), path);
	//if (!CFileOper::PathExists(m_md5_path)) {
	//	DeleteFile(m_bmp_path);
	//	char acct[1024] = { 0 };
	//	GenerateAcct(acct, sizeof(acct));

	//	util::MD5 md5;
	//	md5.update(acct, strnlen_s(acct, 1024));
	//	std::string smd5 = md5.toString();
	//	std::transform(smd5.begin(), smd5.end(), smd5.begin(), ::toupper);

	//	//strcpy_s(manager->m_csr_acct, smd5.c_str());

	//	//manager->SetCsrAcct(smd5.c_str());
	//	
	//	if (strcmp(csr->get_acctA(), smd5.c_str()) != 0) {
	//		csr->execute_set_acct(A2W(smd5.c_str()));
	//	}

	//	wchar_t wacct[1024] = { 0 };
	//	AnsiToUtf16Array(smd5.c_str(), wacct, sizeof(wacct));
	//	m_acct_text = wacct;

	//	CFile file;
	//	CFileException e;
	//	if (file.Open(m_md5_path, CFile::modeCreate | CFile::modeWrite), &e) {
	//		file.Write(smd5.c_str(), smd5.size());
	//		file.Close();
	//	} else {
	//		e.m_cause;
	//	}

	//	//char bmp_path[1024] = { 0 };
	//	//Utf16ToAnsiUseCharArray(m_bmp_path, bmp_path, sizeof(bmp_path));
	//	m_acct = smd5;
	//	BOOL ret = GenerateQrcodeBmp(smd5.c_str(), m_bmp_path);
	//	ret;
	//} else {
	//	char cmd5[1024] = { 0 };
	//	CFile file;
	//	if (file.Open(m_md5_path, CFile::modeRead, NULL)) {
	//		file.Read(cmd5, sizeof(cmd5));
	//		file.Close();
	//	}

	//	//manager->SetCsrAcct(md5);
	//	if (strcmp(csr->get_acctA(), cmd5) != 0) {
	//		csr->execute_set_acct(A2W(cmd5));
	//	}

	//	wchar_t wacct[1024] = { 0 };
	//	AnsiToUtf16Array(cmd5, wacct, sizeof(wacct));
	//	m_acct_text = wacct;

	//	if (!CFileOper::PathExists(m_bmp_path)) {
	//		//char bmp_path[1024] = { 0 };
	//		//Utf16ToAnsiUseCharArray(m_bmp_path, bmp_path, sizeof(bmp_path));
	//		GenerateQrcodeBmp(cmd5, m_bmp_path);
	//	}
	//}

	UpdateData(0);
}

static bool GetMacByGetAdaptersInfo(std::string& macOUT)
{
	bool ret = false;

	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL)
		return false;
	// Make an initial call to GetAdaptersInfo to get the necessary size into the ulOutBufLen variable
	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		free(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *)malloc(ulOutBufLen);
		if (pAdapterInfo == NULL)
			return false;
	}

	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == NO_ERROR) {
		for (PIP_ADAPTER_INFO pAdapter = pAdapterInfo; pAdapter != NULL; pAdapter = pAdapter->Next) {
			// È·±£ÊÇÒÔÌ«Íø
			if (pAdapter->Type != MIB_IF_TYPE_ETHERNET)
				continue;
			// È·±£MACµØÖ·µÄ³¤¶ÈÎª 00-00-00-00-00-00
			if (pAdapter->AddressLength != 6)
				continue;
			char acMAC[64] = { 0 };
			sprintf_s(acMAC, "%02X-%02X-%02X-%02X-%02X-%02X",
					  int(pAdapter->Address[0]),
					  int(pAdapter->Address[1]),
					  int(pAdapter->Address[2]),
					  int(pAdapter->Address[3]),
					  int(pAdapter->Address[4]),
					  int(pAdapter->Address[5]));
			macOUT = acMAC;
			ret = true;
			break;
		}
	}

	free(pAdapterInfo);
	return ret;
}

static void GetSystemName(std::string& osname)
{
	SYSTEM_INFO info;        //ÓÃSYSTEM_INFO½á¹¹ÅÐ¶Ï64Î»AMD´¦ÀíÆ÷   
	GetSystemInfo(&info);    //µ÷ÓÃGetSystemInfoº¯ÊýÌî³ä½á¹¹   
	OSVERSIONINFOEX os;
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	osname = "unknown OperatingSystem.";

	if (GetVersionEx((OSVERSIONINFO *)&os)) {
		//ÏÂÃæ¸ù¾Ý°æ±¾ÐÅÏ¢ÅÐ¶Ï²Ù×÷ÏµÍ³Ãû³Æ   
		switch (os.dwMajorVersion)//ÅÐ¶ÏÖ÷°æ±¾ºÅ  
		{
			case 4:
				switch (os.dwMinorVersion)//ÅÐ¶Ï´Î°æ±¾ºÅ   
				{
					case 0:
						if (os.dwPlatformId == VER_PLATFORM_WIN32_NT)
							osname = "Microsoft Windows NT 4.0"; //1996Äê7ÔÂ·¢²¼   
						else if (os.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
							osname = "Microsoft Windows 95";
						break;
					case 10:
						osname = "Microsoft Windows 98";
						break;
					case 90:
						osname = "Microsoft Windows Me";
						break;
				}
				break;

			case 5:
				switch (os.dwMinorVersion)   //ÔÙ±È½ÏdwMinorVersionµÄÖµ  
				{
					case 0:
						osname = "Microsoft Windows 2000";//1999Äê12ÔÂ·¢²¼  
						break;

					case 1:
						osname = "Microsoft Windows XP";//2001Äê8ÔÂ·¢²¼  
						break;

					case 2:
						if (os.wProductType == VER_NT_WORKSTATION
							&& info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
							osname = "Microsoft Windows XP Professional x64 Edition";
						} else if (GetSystemMetrics(SM_SERVERR2) == 0)
							osname = "Microsoft Windows Server 2003";//2003Äê3ÔÂ·¢²¼   
						else if (GetSystemMetrics(SM_SERVERR2) != 0)
							osname = "Microsoft Windows Server 2003 R2";
						break;
				}
				break;

			case 6:
				switch (os.dwMinorVersion) {
					case 0:
						if (os.wProductType == VER_NT_WORKSTATION)
							osname = "Microsoft Windows Vista";
						else
							osname = "Microsoft Windows Server 2008";//·þÎñÆ÷°æ±¾   
						break;
					case 1:
						if (os.wProductType == VER_NT_WORKSTATION)
							osname = "Microsoft Windows 7";
						else
							osname = "Microsoft Windows Server 2008 R2";
						break;
				}
				break;
		}
	}//if(GetVersionEx((OSVERSIONINFO *)&os))  

}

//¶ÁÈ¡²Ù×÷ÏµÍ³µÄ°æ±¾Ãû³Æ  
static void GetVersionMark(std::string& vmark)
{
	OSVERSIONINFOEX os;
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	vmark = "";

	if (GetVersionEx((OSVERSIONINFO*)&os)) {
		switch (os.dwMajorVersion) {                //ÏÈÅÐ¶Ï²Ù×÷ÏµÍ³°æ±¾   
			case 5:
				switch (os.dwMinorVersion) {
					case 0:                  //Windows 2000   
						if (os.wSuiteMask == VER_SUITE_ENTERPRISE)
							vmark = "Advanced Server";
						break;
					case 1:                  //Windows XP   
						if (os.wSuiteMask == VER_SUITE_EMBEDDEDNT)
							vmark = "Embedded";
						else if (os.wSuiteMask == VER_SUITE_PERSONAL)
							vmark = "Home Edition";
						else
							vmark = "Professional";
						break;
					case 2:
						if (GetSystemMetrics(SM_SERVERR2) == 0
							&& os.wSuiteMask == VER_SUITE_BLADE)  //Windows Server 2003   
							vmark = "Web Edition";
						else if (GetSystemMetrics(SM_SERVERR2) == 0
								 && os.wSuiteMask == VER_SUITE_COMPUTE_SERVER)
								 vmark = "Compute Cluster Edition";
						else if (GetSystemMetrics(SM_SERVERR2) == 0
								 && os.wSuiteMask == VER_SUITE_STORAGE_SERVER)
								 vmark = "Storage Server";
						else if (GetSystemMetrics(SM_SERVERR2) == 0
								 && os.wSuiteMask == VER_SUITE_DATACENTER)
								 vmark = "Datacenter Edition";
						else if (GetSystemMetrics(SM_SERVERR2) == 0
								 && os.wSuiteMask == VER_SUITE_ENTERPRISE)
								 vmark = "Enterprise Edition";
						else if (GetSystemMetrics(SM_SERVERR2) != 0
								 && os.wSuiteMask == VER_SUITE_STORAGE_SERVER)
								 vmark = "Storage Server";
						break;
				}
				break;

			case 6:
				switch (os.dwMinorVersion) {
					case 0:
						if (os.wProductType != VER_NT_WORKSTATION
							&& os.wSuiteMask == VER_SUITE_DATACENTER)
							vmark = "Datacenter Server";
						else if (os.wProductType != VER_NT_WORKSTATION
								 && os.wSuiteMask == VER_SUITE_ENTERPRISE)
								 vmark = "Enterprise";
						else if (os.wProductType == VER_NT_WORKSTATION
								 && os.wSuiteMask == VER_SUITE_PERSONAL)  //Windows Vista  
								 vmark = "Home";
						break;
					case 1:
						vmark = "Ultimate Edition";
						break;
				}
				break;
		}
	}
}

void CQrcodeViewerDlg::GenerateAcct(char* buff, int buff_size)
{
	std::string mac;
	if (!GetMacByGetAdaptersInfo(mac))
		return;

	SYSTEMTIME st = { 0 };
	GetLocalTime(&st);
	SYSTEM_INFO si = { 0 };
	GetSystemInfo(&si);

	std::string systemname;
	GetSystemName(systemname);

	std::string systemmark;
	GetVersionMark(systemmark);

	char content[1024] = { 0 };
	sprintf_s(content, "HB ALARMCENTER, _%02d:%02d:%02d,%02d-%02d-%04d,  MAC:%s, OS:%s %s",
			  st.wSecond, st.wMinute, st.wHour,
			  st.wDay, st.wMonth, st.wYear, mac.c_str(),
			  systemname.c_str(), systemmark.c_str());
	int content_len = strnlen_s(content, 1024);
	if (content_len <= buff_size) {
		strcpy_s(buff, buff_size, content);
	}
}


void CQrcodeViewerDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	if (bShow) {
		m_staticEx.ShowBmp(m_bmp_path);
	}
}


void CQrcodeViewerDlg::InitLocation()
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
		OnBnClickedButtonLocateAuto();
	} else {
		m_addr.SetWindowTextW(addr);
		CString s;
		//s.Format(L"%d", city_code);
		s.Format(L"%f", coor.x);
		m_x.SetWindowTextW(s);
		s.Format(L"%f", coor.y);
		m_y.SetWindowTextW(s);

		//std::wstring  url = GetModuleFilePath();
		//url += L"\\baidu.html";
		//if (!CFileOper::PathExists(url.c_str())) {
		CString sAlarmCenter;
		sAlarmCenter.LoadStringW(IDS_STRING_ALARM_CENTER);
		m_map1->ShowCoordinate(csr->get_coor(), sAlarmCenter);
	}
}



void CQrcodeViewerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	SAFEDELETEDLG(m_map1);
}


void CQrcodeViewerDlg::OnBnClickedButtonLocateAuto()
{
	AUTO_LOG_FUNCTION;
	core::CCsrInfo* csr = core::CCsrInfo::GetInstance();
	std::wstring addr;
	int city_code;
	//double x, y;
	web::BaiduCoordinate coor;
	if (web::CBaiduService::GetInstance()->locate(addr, city_code, coor)) {
		csr->execute_set_addr(addr.c_str());
		csr->execute_set_city_code(city_code);
		//csr->execute_set_x(x);
		csr->execute_set_coor(coor);

		m_addr.SetWindowTextW(addr.c_str());
		CString s;
		s.Format(L"%f", coor.x);
		m_x.SetWindowTextW(s);
		s.Format(L"%f", coor.y);
		m_y.SetWindowTextW(s);

		//const wchar_t* fmt = L"http://api.map.baidu.com/marker?location=%f,%f&title=ÎÒµÄÎ»ÖÃ&content=½Ó¾¯ÖÐÐÄ&output=html&src=HB|AlarmCenter";

		CRect rc;
		m_map1->GetClientRect(rc);

		std::wstring  url = GetModuleFilePath();
		url += L"\\config";
		CreateDirectory(url.c_str(), NULL);
		url += L"\\baidu.html";

		CString sAlarmCenter;
		sAlarmCenter.LoadStringW(IDS_STRING_ALARM_CENTER);
		m_map1->ShowCoordinate(coor, sAlarmCenter);
		/*if (m_map1->GenerateHtml(url, coor, sAlarmCenter)) {
			m_map1->Navigate(url.c_str());
		}*/
	} else {
		CString e; e.LoadStringW(IDS_STRING_E_AUTO_LACATE_FAILED);
		MessageBox(e, L"", MB_ICONERROR);
	}

	core::CUserManager::GetInstance()->UnRegisterObserver(this);
}


void CQrcodeViewerDlg::OnBnClickedButtonLocateToAddr()
{

}


void CQrcodeViewerDlg::OnBnClickedButtonLocateToCoor()
{

}


afx_msg LRESULT CQrcodeViewerDlg::OnChosenBaiduPt(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	web::BaiduCoordinate coor = m_map1->m_coor;
	core::CCsrInfo* csr = core::CCsrInfo::GetInstance();
	web::BaiduCoordinate oldcoor(csr->get_coor());
	if (oldcoor == coor) {
		return 0;
	}

	if (csr->execute_set_coor(coor)) {
		CString s;
		s.Format(L"%f", coor.x);
		m_x.SetWindowTextW(s);
		s.Format(L"%f", coor.y);
		m_y.SetWindowTextW(s);
	}

	return 0;
}


void CQrcodeViewerDlg::OnBnClickedButtonCheckCom()
{
	m_cmbCom.ResetContent();
	CAutoSerialPort ap;
	std::list<int> list;
	if (ap.CheckValidSerialPorts(list) && list.size() > 0) {
		std::list<int>::iterator iter = list.begin();
		CString str = L"";
		while (iter != list.end()) {
			int port = *iter++;
			str.Format(L"COM%d", port);
			int ndx = m_cmbCom.InsertString(-1, str);
			m_cmbCom.SetItemData(ndx, port);
		}
		m_cmbCom.SetCurSel(0);
	} else {
		CString e; e.LoadStringW(IDS_STRING_NO_COM);
		MessageBox(e, NULL, MB_ICONINFORMATION);
	}
}


void CQrcodeViewerDlg::OnBnClickedButtonConnGsm()
{
	CString open; open.LoadStringW(IDS_STRING_OPEN_COM);
	CString txt; m_btnConnCom.GetWindowTextW(txt);
	if (txt.Compare(open) == 0) {
		int ndx = m_cmbCom.GetCurSel();
		if (ndx < 0)return;
		int port = m_cmbCom.GetItemData(ndx);
		if (core::CGsm::GetInstance()->Open(port)) {
			m_cmbCom.EnableWindow(0);
			m_btnCheckCom.EnableWindow(0);
			CString close; close.LoadStringW(IDS_STRING_CLOSE_COM);
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


void CQrcodeViewerDlg::SaveComConfigure(BOOL bRem, int nCom, BOOL bAuto)
{
	USES_CONVERSION;
	
	std::string path = W2A(GetModuleFilePath());
	path += "\\config";
	CreateDirectoryA(path.c_str(), NULL);
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


void CQrcodeViewerDlg::OnBnClickedCheck2()
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


void CQrcodeViewerDlg::OnBnClickedCheck1()
{
	BOOL b1 = m_chkRemCom.GetCheck();
	int ncom = m_cmbCom.GetCurSel();
	BOOL b2 = m_chkAutoConnCom.GetCheck();
	SaveComConfigure(b1, ncom, b2);
}


void CQrcodeViewerDlg::OnBnClickedButton2()
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


void CQrcodeViewerDlg::OnBnClickedButtonSavePhone()
{
	CString phone;
	m_phone.GetWindowTextW(phone);
	if (phone.IsEmpty() || phone.GetLength() > 32) {
		return;
	}

	core::CCsrInfo* csr = core::CCsrInfo::GetInstance();
	csr->execute_set_acct(phone);
	InitAcct();
}
