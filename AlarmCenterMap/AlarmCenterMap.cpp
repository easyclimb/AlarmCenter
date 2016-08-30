
// AlarmCenterMap.cpp : ����Ӧ�ó��������Ϊ��
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


// CAlarmCenterMapApp ����

CAlarmCenterMapApp::CAlarmCenterMapApp()
{
	// ֧����������������
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CAlarmCenterMapApp ����

CAlarmCenterMapApp theApp;


// CAlarmCenterMapApp ��ʼ��

BOOL CAlarmCenterMapApp::InitInstance()
{
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()��  ���򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// ���� shell ���������Է��Ի������
	// �κ� shell ����ͼ�ؼ��� shell �б���ͼ�ؼ���
	CShellManager *pShellManager = new CShellManager;

	// ���Windows Native���Ӿ����������Ա��� MFC �ؼ�����������
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

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
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "����: �Ի��򴴽�ʧ�ܣ�Ӧ�ó���������ֹ��\n");
		TRACE(traceAppMsg, 0, "����: ������ڶԻ�����ʹ�� MFC �ؼ������޷� #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS��\n");
	}

	// ɾ�����洴���� shell ��������
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	
	res = nullptr;
	res::release_singleton();
	cfg = nullptr;
	util::CConfigHelper::release_singleton();
	ipc::alarm_center_map_client::release_singleton();

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}

