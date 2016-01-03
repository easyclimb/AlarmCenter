// AddVideoUserProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AddVideoUserProgressDlg.h"
#include "afxdialogex.h"
#include "SdkMgrEzviz.h"


#include <future>
#include <chrono>
#include <thread>

// CAddVideoUserProgressDlg dialog

IMPLEMENT_DYNAMIC(CAddVideoUserProgressDlg, CDialogEx)

CAddVideoUserProgressDlg::CAddVideoUserProgressDlg(CWnd* pParent /*=NULL*/)
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


static std::future<video::CVideoManager::VideoEzvizResult> future;
static std::wstring name = L"";
static std::string phone8 = "";
static std::shared_ptr<video::ezviz::CVideoUserInfoEzviz> user = nullptr;
static bool g_first_time = false;

BOOL CAddVideoUserProgressDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	g_first_time = true;
	m_progress.SetRange(0, 10);
	m_dwStart = GetTickCount();

	name = m_name.LockBuffer(); m_name.UnlockBuffer();
	std::wstring phone = m_phone.LockBuffer(); m_phone.UnlockBuffer();
	phone8.clear();
	utf8::utf16to8(phone.begin(), phone.end(), std::back_inserter(phone8));
	user = std::make_shared<video::ezviz::CVideoUserInfoEzviz>();
	user->set_user_name(name);
	user->set_user_phone(phone8);

	SetTimer(1, 1000, nullptr);
	

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CAddVideoUserProgressDlg::OnTimer(UINT_PTR nIDEvent)
{
	int pos = m_progress.GetPos();
	pos = ++pos % 10;
	m_progress.SetPos(pos);

	auto seconds = (GetTickCount() - m_dwStart) / 1000;
	auto minutes = seconds / 60;
	CString txt;
	txt.Format(L"%02d:%02d", minutes, seconds);
	m_staticTime.SetWindowTextW(txt);

	if (g_first_time) {
		video::CVideoManager::VideoEzvizResult result = video::CVideoManager::RESULT_OK;
		
		bool ok = false;
		do {
			
			auto sdkEzvizResult = video::ezviz::CSdkMgrEzviz::GetInstance()->VerifyUserAccessToken(user, video::TYPE_GET);
			if (sdkEzvizResult == video::ezviz::CSdkMgrEzviz::RESULT_PRIVATE_CLOUD_CONNECT_FAILED_OR_USER_NOT_EXSIST) {
				result = video::CVideoManager::RESULT_PRIVATE_CLOUD_CONNECT_FAILED_OR_USER_NOT_EXIST; break;
			} else if (sdkEzvizResult == video::ezviz::CSdkMgrEzviz::RESULT_OK) {
			} else { assert(0); }


			future = std::async(std::launch::async, [] {
				video::CVideoManager* mgr = video::CVideoManager::GetInstance();
				video::CVideoManager::VideoEzvizResult result = mgr->AddVideoUserEzviz(user);
				return result;
			});

			ok = true;
		} while (0);

		if (!ok) {
			user = nullptr;
			m_result = video::CVideoManager::RESULT_PRIVATE_CLOUD_CONNECT_FAILED_OR_USER_NOT_EXIST;
			KillTimer(1);
			CDialogEx::OnOK();
			return;
		}
		g_first_time = false;
	} else {
		auto status = future.wait_for(std::chrono::milliseconds(0));
		if (status == std::future_status::ready) {
			m_result = future.get();
			if (m_result != video::CVideoManager::RESULT_OK) {
				user = nullptr;
			}
			KillTimer(1);
			CDialogEx::OnOK();
			return;
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}
