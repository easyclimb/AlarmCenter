
// AlarmCenter.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
//#include "vld/vld.h"
//#pragma comment(lib, "vld.lib")
#include <memory>
#include "mdump.h"
#include "AlarmCenter.h"
#include "AlarmCenterDlg.h"
#include "ConfigHelper.h"
#include "SetupNetworkDlg.h"
#include "LoginDlg.h"
#include "UserInfo.h"
#include "baidu.h"
#include "SdkMgrEzviz.h"
#include "AppResource.h"
//#include "ClientApp.h"

#include "D:/dev_libs/CrashRpt_v.1.4.3_r1645/include/CrashRpt.h"
#pragma comment(lib, "D:/dev_libs/CrashRpt_v.1.4.3_r1645/lib/CrashRpt1403.lib")

namespace detail {

#define MUTEX_NAME _T("Global//AlarmCenter2015Mutex")

	BOOL GetProductVersion(CString& version)
	{
		CString path = _T("");
		path.Format(_T("%s\\VersionNo.ini"), GetModuleFilePath());
		CFile file;
		if (file.Open(path, CFile::modeRead)) {
			size_t length = static_cast<size_t>(file.GetLength());
			auto buff = std::unique_ptr<char[]>(new char[length + 1]);
			file.Read(buff.get(), length);
			buff[length-2] = 0;
			auto wbuff = std::unique_ptr<wchar_t[]>(AnsiToUtf16(buff.get()));
			version = wbuff.get();
			file.Close();
			return TRUE;
		}
		return FALSE;
	}




}
// CAlarmCenterApp

BEGIN_MESSAGE_MAP(CAlarmCenterApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CAlarmCenterApp construction

CAlarmCenterApp::CAlarmCenterApp()
	: m_hMutex(INVALID_HANDLE_VALUE)
{
	//_CrtSetBreakAlloc(12363);
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	
}


// The one and only CAlarmCenterApp object

CAlarmCenterApp theApp;

BOOL CAlarmCenterApp::InitInstance()
{
	do {
		if (IfProcessRunning())
			break;
		auto log = log::get_instance();
		log->set_output_to_dbg_view();
		log->set_line_prifix("HB");
		log->set_log_file_foler(utf8::w2a(get_exe_path() + L"\\Log"));
		log->set_output_to_file();
		
		JLOG(L"AlarmCenter startup.\n");
		AUTO_LOG_FUNCTION;

		int	nRet;
		WSAData	wsData;

		nRet = WSAStartup(MAKEWORD(2, 2), &wsData);
		if (nRet < 0) {
			JLOG(L"Can't load winsock.dll.\n");
			break;
		}


#pragma region init crashrpt
		// Place all significant initialization in InitInstance
		// Define CrashRpt configuration parameters
		CR_INSTALL_INFO info;
		memset(&info, 0, sizeof(CR_INSTALL_INFO));
		info.cb = sizeof(CR_INSTALL_INFO);
		info.pszAppName = _T("AlarmCenter");
		static CString version;
		detail::GetProductVersion(version);
		info.pszAppVersion = version;
		info.pszEmailSubject = _T("AlarmCenter Error Report");
		info.pszEmailTo = _T("captainj@qq.com");
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
			break;
		}

		// Set crash callback function
		//crSetCrashCallback(CrashCallback, nullptr);

		// Add our log file to the error report
		crAddFile2(utf8::a2w(log::get_instance()->get_log_file_path()).c_str(), nullptr, _T("Log File"), CR_AF_MAKE_FILE_COPY);

		// We want the screenshot of the entire desktop is to be added on crash
		crAddScreenshot2(CR_AS_VIRTUAL_SCREEN, 0);

		// Add a named property that means what graphics adapter is
		// installed on user's machine
		//crAddProperty(_T("VideoCard"), _T("nVidia GeForce 8600 GTS"));



#pragma endregion

		auto cfg = util::CConfigHelper::get_instance();
		auto lang = cfg->get_language();
		switch (lang) {	
		case util::AL_TAIWANESE:
			SetThreadUILanguage(MAKELCID(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL), SORT_DEFAULT));
			break;
		case util::AL_ENGLISH:
			SetThreadUILanguage(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT));
			break;
		case util::AL_CHINESE:
		default:
			SetThreadUILanguage(MAKELCID(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED), SORT_DEFAULT));
			break;
		}

		CAppResource::get_instance();

#pragma region test load sdk from ezviz
		auto sdk = video::ezviz::sdk_mgr_ezviz::get_instance();
		if (!sdk->InitLibrary()) {
			QuitApplication(8858);
			break;
		}
		sdk = nullptr;

#pragma endregion

		

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
		auto pShellManager = std::make_unique<CShellManager>();

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
			JLOG(L"user canceled login.\n");
			break;
		}

		CSetupNetworkDlg setupDlg;
		if (setupDlg.DoModal() != IDOK) {
			JLOG(L"user canceled setup network.\n");
			break;
		}

		CAlarmCenterDlg dlg;
		m_pMainWnd = &dlg;
		INT_PTR nResponse = dlg.DoModal();
		if (nResponse == IDOK) {

		} else if (nResponse == IDCANCEL) {

		} else if (nResponse == -1) {
			TRACE(L"Warning: dialog creation failed, so application is terminating unexpectedly.\n");
			TRACE(L"Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
		}

		

	} while (false);

	CAppResource::release_singleton();
	util::CConfigHelper::release_singleton();
	core::user_manager::release_singleton();
	//video::ezviz::sdk_mgr_ezviz::release_singleton();
	

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}


int CAlarmCenterApp::ExitInstance()
{
	ControlBarCleanUp();
	CLOSEHANDLE(m_hMutex);
	//CefShutdown();
	WSACleanup();
	CWinApp::ExitInstance();


	return exit_code_;
}


BOOL CAlarmCenterApp::IfProcessRunning()
{
	m_hMutex = CreateMutex(nullptr, FALSE, MUTEX_NAME);
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		CLOSEHANDLE(m_hMutex);
		return TRUE;
	} else {
		return FALSE;
	}
}
