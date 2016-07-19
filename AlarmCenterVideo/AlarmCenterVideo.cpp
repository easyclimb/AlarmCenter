
// AlarmCenterVideo.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "AlarmCenterVideo.h"
#include "AlarmCenterVideoDlg.h"
#include "ConfigHelper.h"
#include "alarm_center_video_client.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAlarmCenterVideoApp

BEGIN_MESSAGE_MAP(CAlarmCenterVideoApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CAlarmCenterVideoApp construction

CAlarmCenterVideoApp::CAlarmCenterVideoApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CAlarmCenterVideoApp object

CAlarmCenterVideoApp theApp;


// CAlarmCenterVideoApp initialization

BOOL CAlarmCenterVideoApp::InitInstance()
{
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
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));


	auto lg = log::get_instance();

	lg->set_line_prifix("video");
	lg->set_log_file_foler(get_exe_path_a() + "\\log");
	lg->set_log_file_prefix("AlarmCenterVideo");
	lg->set_output_to_file();
	lg->set_output_to_dbg_view();
	lg->log_utf8("AlarmCenterVideo start running...");

	auto res = res::get_instance();
	auto cfg = util::CConfigHelper::get_instance();
	auto lang = cfg->get_language();
	switch (lang) {
	case util::AL_TAIWANESE:
		res->parse_file(get_exe_path() + L"\\lang\\zh-tw.txt");
		SetThreadUILanguage(MAKELCID(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL), SORT_DEFAULT));
		break;
	case util::AL_ENGLISH:
		res->parse_file(get_exe_path() + L"\\lang\\en-us.txt");
		SetThreadUILanguage(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT));
		break;
	case util::AL_CHINESE:
	default:
		res->parse_file(get_exe_path() + L"\\lang\\zh-cn.txt");
		SetThreadUILanguage(MAKELCID(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED), SORT_DEFAULT));
		break;
	}


	CString ez, ezdesc;
	ez = TR(IDS_STRING_EZVIZ);
	ezdesc = TR(IDS_STRING_EZVIZ_DESC);
	ProductorEzviz.set_name(ez.LockBuffer());
	ProductorEzviz.set_description(ezdesc.LockBuffer());
	ez.UnlockBuffer();
	ezdesc.UnlockBuffer();

	ez = TR(IDS_STRING_JOVISION);
	ezdesc = TR(IDS_STRING_JOVISION_DESC);
	ProductorJovision.set_name(ez.LockBuffer());
	ProductorJovision.set_description(ezdesc.LockBuffer());
	ez.UnlockBuffer();
	ezdesc.UnlockBuffer();

	CAlarmCenterVideoDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	res = nullptr;
	res::release_singleton();
	cfg = nullptr;
	util::CConfigHelper::release_singleton();
	ipc::alarm_center_video_client::release_singleton();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

