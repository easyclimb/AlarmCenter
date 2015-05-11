// QrcodeViewerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "QrcodeViewerDlg.h"
#include "afxdialogex.h"
#include "md5.h"
#include "QrCode.h"

#include <iostream>
#include <sstream>
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
	
	InitAcct();
	InitLocation();
	m_map1 = new CBaiduMapDlg();
	m_map1->m_pRealParent = this;
	m_map1->Create(IDD_DIALOG_BAIDU_MAP, &m_staticBaiduMap);
	CRect rc;
	m_staticBaiduMap.GetClientRect(rc);
	rc.DeflateRect(5, 25, 5, 5);
	m_map1->MoveWindow(rc, FALSE);
	m_map1->ShowWindow(SW_SHOW);
	std::wstring  url = GetModuleFilePath();
	url += L"\\baidu.html";
	m_map1->Navigate(url.c_str());

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
			// 确保是以太网
			if (pAdapter->Type != MIB_IF_TYPE_ETHERNET)
				continue;
			// 确保MAC地址的长度为 00-00-00-00-00-00
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
	SYSTEM_INFO info;        //用SYSTEM_INFO结构判断64位AMD处理器   
	GetSystemInfo(&info);    //调用GetSystemInfo函数填充结构   
	OSVERSIONINFOEX os;
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	osname = "unknown OperatingSystem.";

	if (GetVersionEx((OSVERSIONINFO *)&os)) {
		//下面根据版本信息判断操作系统名称   
		switch (os.dwMajorVersion)//判断主版本号  
		{
			case 4:
				switch (os.dwMinorVersion)//判断次版本号   
				{
					case 0:
						if (os.dwPlatformId == VER_PLATFORM_WIN32_NT)
							osname = "Microsoft Windows NT 4.0"; //1996年7月发布   
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
				switch (os.dwMinorVersion)   //再比较dwMinorVersion的值  
				{
					case 0:
						osname = "Microsoft Windows 2000";//1999年12月发布  
						break;

					case 1:
						osname = "Microsoft Windows XP";//2001年8月发布  
						break;

					case 2:
						if (os.wProductType == VER_NT_WORKSTATION
							&& info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
							osname = "Microsoft Windows XP Professional x64 Edition";
						} else if (GetSystemMetrics(SM_SERVERR2) == 0)
							osname = "Microsoft Windows Server 2003";//2003年3月发布   
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
							osname = "Microsoft Windows Server 2008";//服务器版本   
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

//读取操作系统的版本名称  
static void GetVersionMark(std::string& vmark)
{
	OSVERSIONINFOEX os;
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	vmark = "";

	if (GetVersionEx((OSVERSIONINFO*)&os)) {
		switch (os.dwMajorVersion) {                //先判断操作系统版本   
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
	CString addr; double x, y;
	addr = csr->get_addr();
	//city_code = csr->get_city_code();
	x = csr->get_x();
	y = csr->get_y();

	m_addr.SetWindowTextW(addr);
	CString s;
	//s.Format(L"%d", city_code);
	s.Format(L"%f", x);
	m_x.SetWindowTextW(s);
	s.Format(L"%f", y);
	m_y.SetWindowTextW(s);
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
	double x, y;
	if (web::CBaiduService::GetInstance()->locate(addr, city_code, x, y)) {
		csr->execute_set_addr(addr.c_str());
		csr->execute_set_city_code(city_code);
		csr->execute_set_x(x);
		csr->execute_set_y(y);

		m_addr.SetWindowTextW(addr.c_str());
		CString s;
		s.Format(L"%f", x);
		m_x.SetWindowTextW(s);
		s.Format(L"%f", y);
		m_y.SetWindowTextW(s);

		//const wchar_t* fmt = L"http://api.map.baidu.com/marker?location=%f,%f&title=我的位置&content=接警中心&output=html&src=HB|AlarmCenter";

		CRect rc;
		m_map1->GetClientRect(rc);

		std::wstring  url = GetModuleFilePath();
		url += L"\\baidu.html";
		if (GenerateHtml(url, x, y)) {
			m_map1->Navigate(url.c_str());
		}
	}
}


bool CQrcodeViewerDlg::GenerateHtml(std::wstring& url, double x, double y)
{
	std::wostringstream wostr;
	std::wstring html;
	wostr << L"\
<!DOCTYPE html>\r\n\
<html>\r\n\
<head>\r\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\r\n\
<title>百度地图</title>\r\n\
<script type=\"text/javascript\">\r\n\
	var g_x = 0.0;\r\n\
	var g_y = 0.0;\r\n\
	function GetX(){\r\n\
		return g_x;\r\n\
	}\r\n\
\r\n\
	function GetY() {\r\n\
		return g_y;\r\n\
	}\r\n\
\r\n\
	function initialize() {\r\n\
		g_x = " << x << L";\r\n\
		g_y = " << y << L";\r\n\
		var point = new BMap.Point(" << x << L"," << y << L");\r\n\
		var map = new BMap.Map(\"allmap\",{minZoom:10,maxZoom:20});\r\n\
		map.centerAndZoom(point, 14);  \r\n\
		map.enableScrollWheelZoom(true);\r\n\
		map.addControl(new BMap.NavigationControl());\r\n\
		\r\n\
		var marker = new BMap.Marker(point);  \r\n\
		var label = new BMap.Label(\"接警中心\",{offset:new BMap.Size(20,-10)});\r\n\
		marker.setLabel(label) \r\n\
		map.addOverlay(marker);  \r\n\
		marker.enableDragging(); \r\n\
		marker.addEventListener(\"dragend\", function(e){ \r\n\
			document.getElementById(\"r-result\").innerHTML = e.point.lng + \", \" + e.point.lat;\r\n\
			g_x = e.point.lng;\r\n\
			g_y = e.point.lat;\r\n\
		});\r\n\
	}\r\n\
\r\n\
	function loadScript() {\r\n\
	   var script = document.createElement(\"script\");\r\n\
	   script.src = \"http://api.map.baidu.com/api?v=2.0&ak=dEVpRfhLB3ITm2Eenn0uEF3w&callback=initialize\";\r\n\
	   document.body.appendChild(script);\r\n\
	}\r\n\
\r\n\
	window.onload = loadScript;\r\n\
</script></head><body>\r\n\
<div id=\"r-result\" style=\"float:left;width:100px;\">坐标</div>\r\n\
<div id=\"allmap\" style=\"width:500px; height:365px\"></div></body></html>\r\n";
	html = wostr.str();

	//CString url;
	//url.Format(L"%s\\baidu.html", GetModuleFilePath());
	
	CFile file;
	if (file.Open(url.c_str(), CFile::modeCreate | CFile::modeWrite)) {
		//USES_CONVERSION;
		//const char* a = W2A(html);
		//int out_len = 0;
		//const char* utf8 = Utf16ToUtf8(html, out_len);
		std::string utf8;
		utf8::utf16to8(html.begin(), html.end(), std::back_inserter(utf8));
		file.Write(utf8.c_str(), utf8.size());
		file.Close();
		//delete[out_len+1] utf8;
		return true;
	}
	return false;
}


void CQrcodeViewerDlg::OnBnClickedButtonLocateToAddr()
{

}


void CQrcodeViewerDlg::OnBnClickedButtonLocateToCoor()
{

}


afx_msg LRESULT CQrcodeViewerDlg::OnChosenBaiduPt(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	BaiduPoint pt = m_map1->m_pt;
	core::CCsrInfo* csr = core::CCsrInfo::GetInstance();
	BaiduPoint pto(csr->get_x(), csr->get_y());
	if (pto == pt) {
		return 0;
	}

	if (csr->execute_set_x(pt.x) && csr->execute_set_y(pt.y)) {
		CString s;
		s.Format(L"%f", pt.x);
		m_x.SetWindowTextW(s);
		s.Format(L"%f", pt.y);
		m_y.SetWindowTextW(s);

		std::wstring  url = GetModuleFilePath();
		url += L"\\baidu.html";
		if (GenerateHtml(url, pt.x, pt.y)) {
			m_map1->Navigate(url.c_str());
		}
	}

	return 0;
}
