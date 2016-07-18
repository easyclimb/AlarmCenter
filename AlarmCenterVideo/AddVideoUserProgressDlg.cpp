// AddVideoUserProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenterVideo.h"
#include "AddVideoUserProgressDlg.h"
#include "afxdialogex.h"
#include "SdkMgrEzviz.h"


#include <future>
#include <chrono>
#include <thread>

// CAddVideoUserProgressDlg dialog

IMPLEMENT_DYNAMIC(CAddVideoUserProgressDlg, CDialogEx)

CAddVideoUserProgressDlg::CAddVideoUserProgressDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_ADD_VIDEO_USER_PROGRESS, pParent)
{

}

CAddVideoUserProgressDlg::~CAddVideoUserProgressDlg()
{
}

void CAddVideoUserProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
	DDX_Control(pDX, IDC_STATIC_TIME, m_staticTime);
}


BEGIN_MESSAGE_MAP(CAddVideoUserProgressDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAddVideoUserProgressDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CAddVideoUserProgressDlg::OnBnClickedCancel)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CAddVideoUserProgressDlg message handlers


void CAddVideoUserProgressDlg::OnBnClickedOk()
{
	//CDialogEx::OnOK();
}


void CAddVideoUserProgressDlg::OnBnClickedCancel()
{
	//CDialogEx::OnCancel();
}


static std::future<video::video_manager::VideoEzvizResult> g_future;
static std::wstring g_name = L"";
static std::string g_phone8 = "";
static std::shared_ptr<video::ezviz::video_user_info_ezviz> g_user = nullptr;
static bool g_first_time = false;

BOOL CAddVideoUserProgressDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	g_first_time = true;
	m_progress.SetRange(0, 10);
	m_dwStart = GetTickCount();

	g_name = m_name.LockBuffer(); m_name.UnlockBuffer();
	std::wstring phone = m_phone.LockBuffer(); m_phone.UnlockBuffer();
	g_phone8.clear();
	utf8::utf16to8(phone.begin(), phone.end(), std::back_inserter(g_phone8));
	g_user = std::make_shared<video::ezviz::video_user_info_ezviz>();
	g_user->set_user_name(g_name);
	g_user->set_user_phone(g_phone8);

	SetTimer(1, 1000, nullptr);
	

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CAddVideoUserProgressDlg::OnTimer(UINT_PTR nIDEvent)
{
	using namespace video;
	using namespace video::ezviz;
	int pos = m_progress.GetPos();
	pos = ++pos % 10;
	m_progress.SetPos(pos);

	auto seconds = (GetTickCount() - m_dwStart) / 1000;
	auto minutes = seconds / 60;
	CString txt;
	txt.Format(L"%02d:%02d", minutes, seconds);
	m_staticTime.SetWindowTextW(txt);

	if (g_first_time) {
		auto_timer timer(m_hWnd, 1, 1000);

		g_future = std::async(std::launch::async, [] {
			video::video_manager::VideoEzvizResult result = video::video_manager::RESULT_OK;
			auto sdkEzvizResult = sdk_mgr_ezviz::get_instance()->VerifyUserAccessToken(g_user, video::TYPE_GET);
			if (sdkEzvizResult == sdk_mgr_ezviz::RESULT_PRIVATE_CLOUD_CONNECT_FAILED_OR_USER_NOT_EXSIST) {
				result = video::video_manager::RESULT_PRIVATE_CLOUD_CONNECT_FAILED_OR_USER_NOT_EXIST; ;
			} else if (sdkEzvizResult == sdk_mgr_ezviz::RESULT_OK) {
			} else { assert(0); }
			return result;
		});

		g_first_time = false;
	} else {
		auto status = g_future.wait_for(std::chrono::milliseconds(0));
		if (status == std::future_status::ready) {
			KillTimer(1);
			m_result = g_future.get();
			if (m_result != video::video_manager::RESULT_OK) {
				g_user = nullptr;
				m_result = video::video_manager::RESULT_PRIVATE_CLOUD_CONNECT_FAILED_OR_USER_NOT_EXIST;
			} else {
				auto mgr = video::video_manager::get_instance();
				m_result = mgr->AddVideoUserEzviz(g_user);
			}
			CDialogEx::OnOK();
			return;
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}
