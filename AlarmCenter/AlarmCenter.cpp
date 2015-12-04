
// AlarmCenter.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
//#include "vld/vld.h"
//#pragma comment(lib, "vld.lib")
#include "mdump.h"
#include "AlarmCenter.h"
#include "AlarmCenterDlg.h"
#include "ConfigHelper.h"
#include "SetupNetworkDlg.h"
#include "./tinyxml/tinyxml.h"
using namespace tinyxml;
#include "LoginDlg.h"
#include "UserInfo.h"
#include "baidu.h"
//#include "ClientApp.h"

#include "C:/dev/CrashRpt_v.1.4.3_r1645/include/CrashRpt.h"
#pragma comment(lib, "C:/dev/CrashRpt_v.1.4.3_r1645/lib/CrashRpt1403.lib")

#define MUTEX_NAME _T("Global//AlarmCenter2013Mutex")

// CAlarmCenterApp

BEGIN_MESSAGE_MAP(CAlarmCenterApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CAlarmCenterApp construction

CAlarmCenterApp::CAlarmCenterApp()
	: m_hMutex(INVALID_HANDLE_VALUE)
{
	//_CrtSetBreakAlloc(5557);
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	
}


// The one and only CAlarmCenterApp object

CAlarmCenterApp theApp;

namespace {
	//MiniDumper theMiniDumper;
};
//class w
//{
//public:
//	w() : _w(nullptr)
//	{
//		_w = new wchar_t[1]; 
//		memset(_w, 0, sizeof(wchar_t) * 1);
//	}
//	~w() { if (_w) delete[] _w; }
//
//	DECLARE_GETTER_STRING(_w);
//	void set_w(const wchar_t* param)
//	{
//		if (param) { 
//			int len = wcslen(param); 
//			if (_w) { delete[] _w; }
//			_w = new wchar_t[len + 1];
//			wcscpy_s(_w, len + 1, param);
//		} else { 
//			if (_w) { delete[] _w; }
//			_w = new wchar_t[1];
//			_w[0] = 0;
//		} 
//	}
//private:
//	wchar_t* _w;
//};
// CAlarmCenterApp initialization

BOOL CAlarmCenterApp::InitInstance()
{
	//const wchar_t* test = L"我是中国人";
	/*w ww;

	const wchar_t* www = ww.get_w();
	ww.set_w(L"abc");*/
	if (IfProcessRunning()) {
		return FALSE;
	}




	CLog::GetInstance();
	CLog::SetOutputDbgView(1);
	//#if !defined(_DEBUG)
	CLog::SetOutputLogFile(1);
	//#endif
	JLOG(L"AlarmCenter startup.\n");
	AUTO_LOG_FUNCTION;

#pragma region do some test
	char* pack = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int pack_len = strlen(pack);
	JLOG(_T("Lpref not found!\n")); JLOGB(pack, pack_len);
	/*int b = 1;
	b--;
	int a = 1 / b + 2013;
	a++;
	JLOG(L"%d", a);*/
	/*std::wstring addr;
	int city_code;
	double x, y;
	if (web::CBaiduService::GetInstance()->locate(addr, city_code, x, y)) {
		CString s;
		s.Format(L"addr:%s, code %d, x %f, y %f", addr.c_str(), city_code, x, y);
		AfxMessageBox(s);
		web::CBaiduService::ReleaseObject();
	}*/

#pragma endregion

#pragma region init crashrpt
	// Place all significant initialization in InitInstance
	// Define CrashRpt configuration parameters
	CR_INSTALL_INFO info;
	memset(&info, 0, sizeof(CR_INSTALL_INFO));
	info.cb = sizeof(CR_INSTALL_INFO);
	info.pszAppName = _T("AlarmCenter");
#define PSZ_APP_VERSION L"1.0.10"
	info.pszAppVersion = PSZ_APP_VERSION;
	info.pszEmailSubject = PSZ_APP_VERSION _T(" Error Report");
	info.pszEmailTo = _T("wangyapengyu@163.com");
	info.pszUrl = _T("http://113.140.30.118/crashrpt.php");
	info.uPriorities[CR_HTTP] = 3;  // First try send report over HTTP 
	info.uPriorities[CR_SMTP] = 2;  // Second try send report over SMTP  
	info.uPriorities[CR_SMAPI] = 1; // Third try send report over Simple MAPI    
									// Install all available exception handlers
	info.dwFlags |= CR_INST_ALL_POSSIBLE_HANDLERS;
	// Restart the app on crash 
	info.dwFlags |= CR_INST_APP_RESTART;
	info.dwFlags |= CR_INST_SEND_QUEUED_REPORTS;
	//info.pszRestartCmdLine = _T("/restart");
	// Define the Privacy Policy URL 
	//info.pszPrivacyPolicyURL = _T("http://myapp.com/privacypolicy.html");

	// Install crash reporting
	int nResult = crInstall(&info);
	if (nResult != 0) {
		// Something goes wrong. Get error message.
		TCHAR szErrorMsg[512] = _T("");
		crGetLastErrorMsg(szErrorMsg, 512);
		_tprintf_s(_T("%s\n"), szErrorMsg);
		MessageBox(nullptr, szErrorMsg, L"Error", MB_ICONERROR);
		ExitProcess(0);
	}

	// Set crash callback function
	//crSetCrashCallback(CrashCallback, NULL);

	// Add our log file to the error report
	crAddFile2(CLog::GetLogFilePath(), NULL, _T("Log File"), CR_AF_MAKE_FILE_COPY);

	// We want the screenshot of the entire desktop is to be added on crash
	crAddScreenshot2(CR_AS_VIRTUAL_SCREEN, 0);

	// Add a named property that means what graphics adapter is
	// installed on user's machine
	//crAddProperty(_T("VideoCard"), _T("nVidia GeForce 8600 GTS"));



#pragma endregion
	util::CConfigHelper::GetInstance();

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	//CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CLoginDlg loginDlg;
	if (loginDlg.DoModal() != IDOK) {
		if (pShellManager != nullptr) {
			delete pShellManager;
		}
		util::CConfigHelper::ReleaseObject();
		core::CUserManager::ReleaseObject();
		JLOG(L"user canceled login.\n");
		return FALSE;
	}

	CSetupNetworkDlg setupDlg;
	if (setupDlg.DoModal() != IDOK) {
		if (pShellManager != nullptr) {
			delete pShellManager;
		}
		util::CConfigHelper::ReleaseObject();
		core::CUserManager::ReleaseObject();
		JLOG(L"user canceled setup network.\n");
		return FALSE;
	} else {
		m_local_port = static_cast<unsigned short>(setupDlg.m_local_port);
		strcpy_s(m_transmit_server_ip, setupDlg.m_tranmit_ipA);
		m_transmit_server_port = static_cast<unsigned short>(setupDlg.m_transmit_port);

		TiXmlDocument doc;
		TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "", "");
		doc.LinkEndChild(decl);
		TiXmlElement *root = new TiXmlElement("NetworkInfo");
		doc.LinkEndChild(root);

		TiXmlElement *port = new TiXmlElement("local_port");
		root->LinkEndChild(port);
		char xxx[128] = { 0 };
		sprintf_s(xxx, "%d", m_local_port);
		TiXmlText *value = new TiXmlText(xxx);
		port->LinkEndChild(value);

		TiXmlElement *tip = new TiXmlElement("transmit_server_ip");
		root->LinkEndChild(tip);
		sprintf_s(xxx, "%s", m_transmit_server_ip);
		TiXmlText *tip_value = new TiXmlText(xxx);
		tip->LinkEndChild(tip_value);

		TiXmlElement *tport = new TiXmlElement("transmit_server_port");
		root->LinkEndChild(tport);
		sprintf_s(xxx, "%d", m_transmit_server_port);
		TiXmlText *tport_value = new TiXmlText(xxx);
		tport->LinkEndChild(tport_value);

		CString path;
		path.Format(L"%s\\config", GetModuleFilePath());
		CreateDirectory(path, nullptr);
		path += L"\\network.xml";
		USES_CONVERSION;
		doc.SaveFile(W2A(path));
	}

	CAlarmCenterDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK) {
		//  dismissed with OK
	} else if (nResponse == IDCANCEL) {
		//  dismissed with Cancel
	} else if (nResponse == -1) {
		TRACE(L"Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(L"Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}


	// Delete the shell manager created above.
	if (pShellManager != nullptr) {
		delete pShellManager;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}



int CAlarmCenterApp::ExitInstance()
{
	ControlBarCleanUp();
	CLOSEHANDLE(m_hMutex);
	//CefShutdown();
	return CWinApp::ExitInstance();
}


BOOL CAlarmCenterApp::IfProcessRunning()
{
	m_hMutex = CreateMutex(NULL, FALSE, MUTEX_NAME);
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		CLOSEHANDLE(m_hMutex);
		return TRUE;
	} else {
		return FALSE;
	}
}
