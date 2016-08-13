
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
#include "AlarmCenter.h"
//#include "ClientApp.h"

#include "D:/dev_libs/CrashRpt_v.1.4.3_r1645/include/CrashRpt.h"
#pragma comment(lib, "D:/dev_libs/CrashRpt_v.1.4.3_r1645/lib/CrashRpt1403.lib")

namespace detail {

#define MUTEX_NAME _T("Global//AlarmCenter2015Mutex")

	BOOL GetProductVersion(CString& version)
	{
		std::string path = get_exe_path_a() + "\\VersionNo.ini";
		std::ifstream in(path);
		if (in) {
			std::stringstream ss;
			ss << in.rdbuf();
			auto v = ss.str();
			v.erase(std::remove(v.begin(), v.end(), '\r'), v.end());
			v.erase(std::remove(v.begin(), v.end(), '\n'), v.end());
			version = utf8::a2w(v).c_str();
			in.close();
			return true;
		}

		return false;
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
	/*{
		bool connect_by_sse_or_ip_ = true;
		std::string cloud_sse_id_ = "";
		std::string device_ipv4_ = "";
		int device_port_ = 0;
		std::wstring user_name_ = L"";
		int user_id = 0;
		std::wstring _device_note = L"";
		int _id = 0;

		CString sql;
		sql.Format(L"update table_device_info_jovision set \
connect_by_sse_or_ip=%d,\
cloud_sse_id='%s',\
device_ipv4='%s',\
device_port=%d,\
user_name='%s',\
user_passwd='%s',\
user_info_id=%d,\
device_note='%s' where id=%d",
connect_by_sse_or_ip_ ? 1 : 0,
utf8::a2w(cloud_sse_id_).c_str(),
utf8::a2w(device_ipv4_).c_str(),
device_port_,
user_name_.c_str(),
user_id,
_device_note.c_str(),
_id);

		JLOG(sql);
	}*/


	do {
		if (IfProcessRunning())
			break;
		auto log = log::get_instance();
		log->set_output_to_dbg_view();
		log->set_line_prifix("HB");
		log->set_log_file_foler(get_exe_path_a() + "\\Log");
		log->set_log_file_prefix("AlarmCenter");
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

		auto res = res::get_instance();
		auto cfg = util::CConfigHelper::get_instance();
		auto lang = cfg->get_language();
		auto path = get_exe_path();
#ifdef _DEBUG
		path = path.substr(0, path.find_last_of(L'\\'));
		path += L"\\installer";
#endif
		switch (lang) {	
		case util::AL_TAIWANESE:
			res->parse_file(path + L"\\lang\\zh-tw.txt");
			SetThreadUILanguage(MAKELCID(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL), SORT_DEFAULT));
			break;
		case util::AL_ENGLISH:
			res->parse_file(path + L"\\lang\\en-us.txt");
			SetThreadUILanguage(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT));
			break;
		case util::AL_CHINESE:
		default:
			res->parse_file(path + L"\\lang\\zh-cn.txt");
			SetThreadUILanguage(MAKELCID(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED), SORT_DEFAULT));
			break;
		}

		CAppResource::get_instance();

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

	CAppResource::release_singleton();
	util::CConfigHelper::release_singleton();
	core::user_manager::release_singleton();
	res::release_singleton();

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
