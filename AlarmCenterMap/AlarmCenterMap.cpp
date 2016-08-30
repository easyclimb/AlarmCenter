
// AlarmCenterMap.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "AlarmCenterMap.h"
#include "AlarmCenterMapDlg.h"
#include "BaiduMapViewerDlg.h"
#include "ConfigHelper.h"
#include "alarm_center_map_client.h"

#include "D:/dev_libs/CrashRpt_v.1.4.3_r1645/include/CrashRpt.h"
#pragma comment(lib, "D:/dev_libs/CrashRpt_v.1.4.3_r1645/lib/CrashRpt1403.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace detail {
struct version_no {
	int major = 0;
	int minor = 0;
	int revision = 0;
	int build = 0;

	version_no& from_string(const std::string& s) {
		std::sscanf(s.c_str(), "%d.%d.%d.%d", &major, &minor, &revision, &build);
		if (major < 0) major = 0;
		if (minor < 0) minor = 0;
		if (revision < 0) revision = 0;
		if (build < 0) build = 0;
		return *this;
	}

	std::string to_string() const {
		std::stringstream ss;
		ss << major << "." << minor << "." << revision << "." << build;
		return ss.str();
	}

	bool valid() {
		return !(major == 0 && minor == 0 && revision == 0 && build == 0);
	}

	void reset() {
		major = minor = revision = build = 0;
	}

	bool operator == (const version_no& ver) {
		return major == ver.major
			&& minor == ver.minor
			&& revision == ver.revision
			&& build == ver.build;
	}

	bool operator < (const version_no& ver) {
		if (major > ver.major) return false;
		if (minor > ver.minor) return false;
		if (revision > ver.revision) return false;
		if (build > ver.build) return false;
		return true;
	}


};

bool get_version_no_from_ini(version_no& ver, const std::string& ini_path) {
	std::ifstream in(ini_path);
	if (!in)return false;
	std::stringstream is;
	is << in.rdbuf();
	version_no file_ver;
	file_ver.from_string(is.str());
	if (file_ver.valid()) {
		ver = file_ver;
		return true;
	}
	return false;
}

}

// CAlarmCenterMapApp

BEGIN_MESSAGE_MAP(CAlarmCenterMapApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CAlarmCenterMapApp 构造

CAlarmCenterMapApp::CAlarmCenterMapApp()
{
	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CAlarmCenterMapApp 对象

CAlarmCenterMapApp theApp;


// CAlarmCenterMapApp 初始化

BOOL CAlarmCenterMapApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// 创建 shell 管理器，以防对话框包含
	// 任何 shell 树视图控件或 shell 列表视图控件。
	CShellManager *pShellManager = new CShellManager;

	// 激活“Windows Native”视觉管理器，以便在 MFC 控件中启用主题
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

	auto lg = log::get_instance();
	
	lg->set_line_prifix("map");
	lg->set_log_file_foler(get_exe_path_a() + "\\log");
	lg->set_log_file_prefix("AlarmCenterMap");
	lg->set_output_to_file();
	lg->set_output_to_dbg_view();
	lg->log_utf8("AlarmCenterMap start running...");

#pragma region init crashrpt
	// Place all significant initialization in InitInstance
	// Define CrashRpt configuration parameters
	CR_INSTALL_INFO info;
	memset(&info, 0, sizeof(CR_INSTALL_INFO));
	info.cb = sizeof(CR_INSTALL_INFO);
	info.pszAppName = _T("AlarmCenterMap");
	//static CString version;
	////detail::GetProductVersion(version);
	static CString version = L"1.0.0.0";
	detail::version_no local_ver;
	auto version_ini = "VersionNo.ini";
	if (detail::get_version_no_from_ini(local_ver, get_exe_path_a() + "\\" + version_ini)) {
		version = utf8::a2w(local_ver.to_string()).c_str();
	}
	info.pszAppVersion = version;
	info.pszEmailSubject = _T("AlarmCenterMap Error Report");
	info.pszEmailTo = _T("captainj@qq.com");
	info.pszUrl = _T("http://113.140.84.234/crashrpt.php");
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
		return FALSE;
	}

	// Set crash callback function
	//crSetCrashCallback(CrashCallback, nullptr);

	// Add our log file to the error report
	crAddFile2(utf8::a2w(lg->get_log_file_path()).c_str(), nullptr, _T("Log File"), CR_AF_MAKE_FILE_COPY);

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

	

	//CAlarmCenterMapDlg dlg;

	CBaiduMapViewerDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "警告: 对话框创建失败，应用程序将意外终止。\n");
		TRACE(traceAppMsg, 0, "警告: 如果您在对话框上使用 MFC 控件，则无法 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS。\n");
	}

	// 删除上面创建的 shell 管理器。
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	
	res = nullptr;
	res::release_singleton();
	cfg = nullptr;
	util::CConfigHelper::release_singleton();
	ipc::alarm_center_map_client::release_singleton();

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}

