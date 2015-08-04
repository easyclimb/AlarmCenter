
// TestHikvisionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TestHikvision.h"
#include "TestHikvisionDlg.h"
#include "afxdialogex.h"
#include "InputVerifyCodeDlg.h"
#include "json\\json.h"
#include "OpenSdkMgr.h"
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

std::list<Json::Value> m_devList;

static void __stdcall messageHandler(const char *szSessionId,
									 unsigned int iMsgType,
									 unsigned int iErrorCode,
									 const char *pMessageInfo,
									 void *pUser);

static void __stdcall videoDataHandler(DataType enType,
									   char* const pData,
									   int iLen,
									   void* pUser);

static void __stdcall alarmMessageHandler(const char* szCameraId, const char* szContent, 
										  const char* szAlarmTime, void* pUser)
{
	AUTO_LOG_FUNCTION;
}

static void __stdcall publishMessageHandler(const char* szContent, void* pUser)
{
	AUTO_LOG_FUNCTION;
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CTestHikvisionDlg dialog



CTestHikvisionDlg::CTestHikvisionDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTestHikvisionDlg::IDD, pParent)
	, m_appKey("52c8edc727cd4d4a81bb1d6c7e884fb5")
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestHikvisionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Control(pDX, IDC_EDIT1, m_cameraId);
	DDX_Control(pDX, IDC_EDIT2, m_cameraName);
	DDX_Control(pDX, IDC_EDIT3, m_cameraNo);
	DDX_Control(pDX, IDC_EDIT4, m_defence);
	DDX_Control(pDX, IDC_EDIT5, m_deviceId);
	DDX_Control(pDX, IDC_EDIT6, m_deviceName);
	DDX_Control(pDX, IDC_EDIT7, m_deviceSerial);
	DDX_Control(pDX, IDC_EDIT8, m_isEncrypt);
	DDX_Control(pDX, IDC_EDIT9, m_isShared);
	DDX_Control(pDX, IDC_EDIT10, m_picUrl);
	DDX_Control(pDX, IDC_EDIT11, m_status);
	DDX_Control(pDX, IDC_STATIC_VIDEO, m_ctrlVideo);
}

BEGIN_MESSAGE_MAP(CTestHikvisionDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_LBN_SELCHANGE(IDC_LIST1, &CTestHikvisionDlg::OnLbnSelchangeList1)
	ON_BN_CLICKED(IDC_BUTTON1, &CTestHikvisionDlg::OnBnClickedButton1)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON2, &CTestHikvisionDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CTestHikvisionDlg::OnBnClickedButton3)
END_MESSAGE_MAP()


// CTestHikvisionDlg message handlers

BOOL CTestHikvisionDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	int ret = 0;
	COpenSdkMgr* dll = COpenSdkMgr::GetInstance();
	ret = dll->initLibrary("https://auth.ys7.com", "https://open.ys7.com", m_appKey);
	m_sessonId = dll->allocSession(messageHandler, this);
	ret = dll->setAlarmMsgCallBack(alarmMessageHandler, publishMessageHandler, this);

	m_accessToken = dll->login();;

	void* buff = NULL;
	int l = 0;
	dll->getDevList(m_accessToken.c_str(), 0, 1500, &buff, &l);
	std::string json = static_cast<char*>(buff);
	LOGA(json.c_str());
	dll->freeData(buff);

	USES_CONVERSION;
	Json::Reader reader;
	Json::Value	value;
	if (reader.parse(json.data(), value) && value["resultCode"].asString() == "200") {
		m_devList.clear();
		Json::Value &cameraListVal = value["cameraList"];
		CString txt;
		if (cameraListVal.isArray()) {
			int cameraCount = cameraListVal.size();
			for (int i = 0; i < cameraCount; i++) {
				m_devList.push_back(cameraListVal[i]);
				txt = A2W(cameraListVal[i]["cameraName"].asString().c_str());
				m_list.InsertString(i, txt);
			}
		}
	}

	

	SetVideoPath();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTestHikvisionDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTestHikvisionDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTestHikvisionDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CTestHikvisionDlg::OnLbnSelchangeList1()
{
	int ndx = m_list.GetCurSel();
	if (ndx < 0 || ndx >= (int)m_devList.size())return;
	Json::Value json;
	std::list<Json::Value>::iterator iter = m_devList.begin();
	while (iter != m_devList.end() && ndx--) { iter++; }
	json = *iter;

	USES_CONVERSION;
	CString cameraId;// = A2W(json["cameraId"].asString().c_str());
	CString cameraName;
	CString cameraNo;
	CString defence;
	CString deviceId;
	CString deviceName;
	CString deviceSerial;
	CString isEncrypt;
	CString isShared;
	CString picUrl;
	CString status;

#define GET_STRING(VAL) VAL = A2W(json[#VAL].asString().c_str()); m_##VAL.SetWindowText(VAL);
#define GET_INT(VAL) VAL.Format(L"%d", json[#VAL].asInt()); m_##VAL.SetWindowText(VAL);

	GET_STRING(cameraId);
	GET_STRING(cameraName);
	GET_INT(cameraNo);
	GET_INT(defence);
	GET_STRING(deviceId);
	GET_STRING(deviceName);
	GET_STRING(deviceSerial);
	GET_INT(isEncrypt);
	GET_STRING(isShared);
	GET_STRING(picUrl);
	GET_INT(status);
}


static void __stdcall messageHandler(const char *szSessionId,
									 unsigned int iMsgType,
									 unsigned int iErrorCode,
									 const char *pMessageInfo,
									 void *pUser)
{
	AUTO_LOG_FUNCTION;
	LOGA("(const char *szSessionId, %s\r\n\
		 unsigned int iMsgType, %d\r\n\
		 unsigned int iErrorCode, %d\r\n\
		 const char *pMessageInfo, %s\r\n\
		 void *pUser)\r\n", szSessionId, iMsgType, iErrorCode, pMessageInfo);

	CTestHikvisionDlg* pInstance = static_cast<CTestHikvisionDlg*>(pUser);
	if (!pInstance) {
		return;
	}

	switch (iMsgType) {
		case INS_PLAY_EXCEPTION:
			//pInstance->insPlayException(iErrorCode, pMessageInfo);
			break;
		case INS_PLAY_RECONNECT:
			break;
		case INS_PLAY_RECONNECT_EXCEPTION:
			//pInstance->insPlayReconnectException(iErrorCode, pMessageInfo);
			break;
		case INS_PLAY_START:
			break;
		case INS_PLAY_STOP:
			break;
		case INS_PLAY_ARCHIVE_END:
			break;
		case INS_RECORD_FILE:
			//pInstance->insRecordFile(pMessageInfo);
			break;
		case INS_RECORD_SEARCH_END:
			break;
		case INS_RECORD_SEARCH_FAILED:
			//pInstance->insRecordSearchFailed(iErrorCode, pMessageInfo);
			break;
	}
}


void CTestHikvisionDlg::StartRealPlay(int iVideoLevel)
{
	int ndx = m_list.GetCurSel();
	if (ndx < 0 || ndx >= (int)m_devList.size())return;
	Json::Value json;
	std::list<Json::Value>::iterator iter = m_devList.begin();
	while (iter != m_devList.end() && ndx--) { iter++; }
	json = *iter;
	std::string cameraId = json["cameraId"].asString();

	std::string safeKey;
	int ret;
	bool bEncrypt = false;
	COpenSdkMgr* dll = COpenSdkMgr::GetInstance();
	ret = dll->UpdateCameraInfo(cameraId.c_str(), m_accessToken.c_str(), bEncrypt);
	if (ret == -1) {
		MessageBox(L"预览", L"无法获取监控点设备信息");
		return;
	}

	if (bEncrypt) {
		CInputVerifyCodeDlg dlg;
		if (dlg.DoModal() != IDOK) {
			return;
		}
		USES_CONVERSION;
		safeKey = W2A(dlg.m_result);
	}

	//ret = OpenSDK_SetDataCallBack(m_sessonId.c_str(), videoDataHandler, this);

	ret = dll->startRealPlay(m_sessonId.c_str(), m_ctrlVideo.m_hWnd, 
								cameraId.c_str(), m_accessToken.c_str(),
								safeKey.c_str(), m_appKey.c_str(), iVideoLevel);
}


void CTestHikvisionDlg::OnBnClickedButton1()
{
	CRect rc;
	((CButton*)GetDlgItem(IDC_BUTTON1))->GetWindowRect(rc);
	CMenu menu;
	menu.CreatePopupMenu();
	menu.InsertMenu(0, MF_BYPOSITION, 0, L"smooth");
	menu.InsertMenu(1, MF_BYPOSITION, 1, L"balance");
	menu.InsertMenu(2, MF_BYPOSITION, 2, L"HD");
	int ret = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
									  rc.left, rc.bottom, 
									  //point.x, point.y,
									  this);
	StartRealPlay(ret);
}


static void __stdcall videoDataHandler(DataType enType,
									   char* const pData,
									   int iLen,
									   void* pUser)
{
	AUTO_LOG_FUNCTION;
	//qDebug() << __LINE__ << __FUNCTION__ << "enType:" << enType << "pData:" << pData << "iLen:" << iLen;
	LOGA("enType %d, pData %p, iLen %d\n");
	CTestHikvisionDlg * mainWins = (CTestHikvisionDlg *)pUser;
	std::ofstream file;
	file.open(mainWins->m_videoPath.c_str(), std::ios::binary | std::ios::app);
	file.write(pData, iLen);
	file.flush();
	file.close();
}


void CTestHikvisionDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	COpenSdkMgr::GetInstance()->freeSession(m_sessonId);
	COpenSdkMgr::GetInstance()->releaseLibrary();
	COpenSdkMgr::ReleaseObject();
}


void CTestHikvisionDlg::OnBnClickedButton2()
{
	COpenSdkMgr::GetInstance()->stopRealPlay(m_sessonId);
}


void CTestHikvisionDlg::OnBnClickedButton3()
{
	USES_CONVERSION;
	std::string name = W2A(CTime::GetCurrentTime().Format(L"%Y-%m-%d-%H-%M-%S.jpg"));
	COpenSdkMgr::GetInstance()->capturePicture(m_sessonId, name);
}
