// QrcodeViewerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "QrcodeViewerDlg.h"
#include "afxdialogex.h"
#include "md5.h"
#include "QrCode.h"

#include <iostream>

#include <algorithm>

#include "CsrInfo.h"
#include "BaiduMapDlg.h"
#include "baidu.h"

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
}


BEGIN_MESSAGE_MAP(CQrcodeViewerDlg, CDialogEx)
	ON_WM_SHOWWINDOW()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_LOCATE_AUTO, &CQrcodeViewerDlg::OnBnClickedButtonLocateAuto)
	ON_BN_CLICKED(IDC_BUTTON_LOCATE_TO_ADDR, &CQrcodeViewerDlg::OnBnClickedButtonLocateToAddr)
	ON_BN_CLICKED(IDC_BUTTON_LOCATE_TO_COOR, &CQrcodeViewerDlg::OnBnClickedButtonLocateToCoor)
	ON_MESSAGE(WM_CHOSEN_BAIDU_PT, &CQrcodeViewerDlg::OnChosenBaiduPt)
END_MESSAGE_MAP()


// CQrcodeViewerDlg message handlers


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

	InitAcct();
	InitLocation();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CQrcodeViewerDlg::InitAcct()
{
	USES_CONVERSION;
	//core::CAlarmMachineManager* manager = core::CAlarmMachineManager::GetInstance();
	core::CCsrInfo* csr = core::CCsrInfo::GetInstance();

	m_md5_path.Format(_T("%s\\acct.md5"), GetModuleFilePath());
	m_bmp_path.Format(_T("%s\\acct.bmp"), GetModuleFilePath());
	if (!CFileOper::PathExists(m_md5_path)) {
		DeleteFile(m_bmp_path);
		char acct[1024] = { 0 };
		GenerateAcct(acct, sizeof(acct));

		util::MD5 md5;
		md5.update(acct, strnlen_s(acct, 1024));
		std::string smd5 = md5.toString();
		std::transform(smd5.begin(), smd5.end(), smd5.begin(), ::toupper);

		//strcpy_s(manager->m_csr_acct, smd5.c_str());

		//manager->SetCsrAcct(smd5.c_str());
		
		if (strcmp(csr->get_acctA(), smd5.c_str()) != 0) {
			csr->execute_set_acct(A2W(smd5.c_str()));
		}

		wchar_t wacct[1024] = { 0 };
		AnsiToUtf16Array(smd5.c_str(), wacct, sizeof(wacct));
		m_acct_text = wacct;

		CFile file;
		CFileException e;
		if (file.Open(m_md5_path, CFile::modeCreate | CFile::modeWrite), &e) {
			file.Write(smd5.c_str(), smd5.size());
			file.Close();
		} else {
			e.m_cause;
		}

		char bmp_path[1024] = { 0 };
		Utf16ToAnsiUseCharArray(m_bmp_path, bmp_path, sizeof(bmp_path));
		m_acct = smd5;
		BOOL ret = GenerateQrcodeBmp(smd5.c_str(), bmp_path);
		ret;
	} else {
		char md5[1024] = { 0 };
		CFile file;
		if (file.Open(m_md5_path, CFile::modeRead, NULL)) {
			file.Read(md5, sizeof(md5));
			file.Close();
		}

		//manager->SetCsrAcct(md5);
		if (strcmp(csr->get_acctA(), md5) != 0) {
			csr->execute_set_acct(A2W(md5));
		}

		wchar_t wacct[1024] = { 0 };
		AnsiToUtf16Array(md5, wacct, sizeof(wacct));
		m_acct_text = wacct;

		if (!CFileOper::PathExists(m_bmp_path)) {
			char bmp_path[1024] = { 0 };
			Utf16ToAnsiUseCharArray(m_bmp_path, bmp_path, sizeof(bmp_path));
			GenerateQrcodeBmp(md5, bmp_path);
		}
	}

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

		std::wstring  url = GetModuleFilePath();
		url += L"\\baidu.html";
		//if (!CFileOper::PathExists(url.c_str())) {
		CString sAlarmCenter;
		sAlarmCenter.LoadStringW(IDS_STRING_ALARM_CENTER);
		m_map1->ShowCoordinate(csr->get_coor(), sAlarmCenter);
		/*if (m_map1->GenerateHtml(url, csr->get_coor(), sAlarmCenter)) {
			m_map1->Navigate(url.c_str());
		}*/
	}
}



void CQrcodeViewerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	SAFEDELETEDLG(m_map1);
}


void CQrcodeViewerDlg::OnBnClickedButtonLocateAuto()
{
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
