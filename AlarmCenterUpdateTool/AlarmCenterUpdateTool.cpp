
// AlarmCenterUpdateTool.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "AlarmCenterUpdateTool.h"
#include "AlarmCenterUpdateToolDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAlarmCenterUpdateToolApp

BEGIN_MESSAGE_MAP(CAlarmCenterUpdateToolApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CAlarmCenterUpdateToolApp ����

CAlarmCenterUpdateToolApp::CAlarmCenterUpdateToolApp()
{
	// ֧����������������
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CAlarmCenterUpdateToolApp ����

CAlarmCenterUpdateToolApp theApp;

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
// CAlarmCenterUpdateToolApp ��ʼ��

BOOL CAlarmCenterUpdateToolApp::InitInstance()
{
	auto version_ini = "VersionNo.ini";
	version_no local_ver;
	version_no ver;
	ver.from_string("1.4.0.9302");
	if (!get_version_no_from_ini(local_ver, get_exe_path_a() + "\\" + version_ini) || ver == local_ver || ver < local_ver) {
		return FALSE;
	}


	SetThreadUILanguage(MAKELCID(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED), SORT_DEFAULT));
	auto path = get_exe_path_a() + "\\update_log";
	CreateDirectoryA(path.c_str(), nullptr);
	auto log = log::get_instance();
	log->set_line_prifix("up");
	log->set_output_to_dbg_view(true);
	log->set_log_file_foler(path);
	log->set_output_to_file(true);

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

	CAlarmCenterUpdateToolDlg dlg;
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

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}

