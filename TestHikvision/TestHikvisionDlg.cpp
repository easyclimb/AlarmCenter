
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
#include "md5.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

std::list<Json::Value> m_devList;

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


static void __stdcall videoDataHandler(DataType enType,
									   char* const pData,
									   int iLen,
									   void* pUser)
{
	AUTO_LOG_FUNCTION;
	LOGA("enType %d, pData %p, iLen %d\n", enType, pData, iLen);
	CTestHikvisionDlg * mainWins = (CTestHikvisionDlg *)pUser;
	std::ofstream file;
	file.open(mainWins->m_videoPath, std::ios::binary | std::ios::app);
	if (file.is_open()) {
		file.write(pData, iLen);
		file.flush();
		file.close();
	}
}

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
	, m_appSecret("51431820c54a14a74db0e374c784cd5d")
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
	DDX_Control(pDX, IDC_BUTTON4, m_btnUp);
	DDX_Control(pDX, IDC_BUTTON5, m_btnDown);
	DDX_Control(pDX, IDC_BUTTON6, m_btnLeft);
	DDX_Control(pDX, IDC_BUTTON7, m_btnRight);
	DDX_Control(pDX, IDC_BUTTON8, m_btnLogin);
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
	ON_BN_CLICKED(IDC_BUTTON4, &CTestHikvisionDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CTestHikvisionDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &CTestHikvisionDlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, &CTestHikvisionDlg::OnBnClickedButton7)
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDC_BUTTON8, &CTestHikvisionDlg::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON_GET_DEV_LIST, &CTestHikvisionDlg::OnBnClickedButtonGetDevList)
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
	m_sessionId = dll->allocSession(messageHandler, this);
	ret = dll->setAlarmMsgCallBack(alarmMessageHandler, publishMessageHandler, this);

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

#define GET_STRING(VAL) { CString VAL = A2W(json[#VAL].asString().c_str()); m_##VAL.SetWindowText(VAL); }
#define GET_INT(VAL) { CString VAL; VAL.Format(L"%d", json[#VAL].asInt()); m_##VAL.SetWindowText(VAL); }

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


std::string CTestHikvisionDlg::getCameraId()
{
	int ndx = m_list.GetCurSel();
	if (ndx < 0 || ndx >= (int)m_devList.size())return "";
	Json::Value json;
	std::list<Json::Value>::iterator iter = m_devList.begin();
	while (iter != m_devList.end() && ndx--) { iter++; }
	json = *iter;
	return json["cameraId"].asString();
}


void CTestHikvisionDlg::StartRealPlay(int iVideoLevel)
{
	std::string cameraId = getCameraId();
	if (cameraId.size() == 0)
		return;

	std::string safeKey;
	int ret;
	bool bEncrypt = false;
	COpenSdkMgr* dll = COpenSdkMgr::GetInstance();
	ret = dll->UpdateCameraInfo(cameraId, m_accessToken, bEncrypt);
	if (ret != 0) {
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

	SetVideoPath();
	ret = dll->setDataCallBack(m_sessionId, videoDataHandler, this);

	ret = dll->startRealPlay(m_sessionId, m_ctrlVideo.m_hWnd, 
								cameraId, m_accessToken,
								safeKey, m_appKey, iVideoLevel);
}


void CTestHikvisionDlg::OnBnClickedButton1()
{
	CRect rc;
	((CButton*)GetDlgItem(IDC_BUTTON1))->GetWindowRect(rc);
	CMenu menu;
	menu.CreatePopupMenu();
	menu.InsertMenu(0, MF_BYPOSITION, 1, L"smooth");
	menu.InsertMenu(1, MF_BYPOSITION, 2, L"balance");
	menu.InsertMenu(2, MF_BYPOSITION, 3, L"HD");
	int ret = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
									  rc.left, rc.bottom, 
									  //point.x, point.y,
									  this);
	if (ret > 0)
		StartRealPlay(ret - 1);
}





void CTestHikvisionDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	COpenSdkMgr::GetInstance()->freeSession(m_sessionId);
	COpenSdkMgr::GetInstance()->releaseLibrary();
	COpenSdkMgr::ReleaseObject();
}


void CTestHikvisionDlg::OnBnClickedButton2()
{
	COpenSdkMgr::GetInstance()->stopRealPlay(m_sessionId);
}


void CTestHikvisionDlg::OnBnClickedButton3()
{
	USES_CONVERSION;
	std::string name = W2A(CTime::GetCurrentTime().Format(L"%Y-%m-%d-%H-%M-%S.jpg"));
	COpenSdkMgr::GetInstance()->capturePicture(m_sessionId, name);
}


void CTestHikvisionDlg::OnBnClickedButton4()
{
	ptzCtrlStart(DIRECT_UP);
	ptzCtrlStop(DIRECT_UP);
}


void CTestHikvisionDlg::OnBnClickedButton5()
{
	ptzCtrlStart(DIRECT_DOWN);
	ptzCtrlStop(DIRECT_DOWN);
}


void CTestHikvisionDlg::OnBnClickedButton6()
{
	ptzCtrlStart(DIRECT_LEFT);
	ptzCtrlStop(DIRECT_LEFT);
}


void CTestHikvisionDlg::OnBnClickedButton7()
{
	ptzCtrlStart(DIRECT_RIGHT);
	ptzCtrlStop(DIRECT_RIGHT);
}


void CTestHikvisionDlg::ptzCtrlStart(PTZCMD cmd)
{
	std::string cameraId = getCameraId();
	if (cameraId.size() == 0)
		return;
	
	switch (cmd) {
		case DIRECT_UP:
			COpenSdkMgr::GetInstance()->PTZCtrl(m_sessionId, m_accessToken, cameraId, UP, START, PTZ_SPEED);
			break;
		case DIRECT_DOWN:
			COpenSdkMgr::GetInstance()->PTZCtrl(m_sessionId, m_accessToken, cameraId, DOWN, START, PTZ_SPEED);
			break;
		case DIRECT_LEFT:
			COpenSdkMgr::GetInstance()->PTZCtrl(m_sessionId, m_accessToken, cameraId, LEFT, START, PTZ_SPEED);
			break;
		case DIRECT_RIGHT:
			COpenSdkMgr::GetInstance()->PTZCtrl(m_sessionId, m_accessToken, cameraId, RIGHT, START, PTZ_SPEED);
			break;
		case DIRECT_UPLEFT:
			COpenSdkMgr::GetInstance()->PTZCtrl(m_sessionId, m_accessToken, cameraId, UPLEFT, START, PTZ_SPEED);
			break;
		case DIRECT_DOWNLEFT:
			COpenSdkMgr::GetInstance()->PTZCtrl(m_sessionId, m_accessToken, cameraId, DOWNLEFT, START, PTZ_SPEED);
			break;
		case DIRECT_UPRIGHT:
			COpenSdkMgr::GetInstance()->PTZCtrl(m_sessionId, m_accessToken, cameraId, UPRIGHT, START, PTZ_SPEED);
			break;
		case DIRECT_DOWNRIGHT:
			COpenSdkMgr::GetInstance()->PTZCtrl(m_sessionId, m_accessToken, cameraId, DOWNRIGHT, START, PTZ_SPEED);
			break;
		case PTZ_ZOOMIN:
			COpenSdkMgr::GetInstance()->PTZCtrl(m_sessionId, m_accessToken, cameraId, ZOOMIN, START, PTZ_SPEED);
			break;
		case PTZ_ZOOMOUT:
			COpenSdkMgr::GetInstance()->PTZCtrl(m_sessionId, m_accessToken, cameraId, ZOOMOUT, START, PTZ_SPEED);
			break;
		case PTZ_IRISSTARTUP:
			COpenSdkMgr::GetInstance()->PTZCtrl(m_sessionId, m_accessToken, cameraId, IRISSTARTUP, START, PTZ_SPEED);
			break;
		case PTZ_IRISSTOPDOWN:
			COpenSdkMgr::GetInstance()->PTZCtrl(m_sessionId, m_accessToken, cameraId, IRISSTOPDOWN, START, PTZ_SPEED);
			break;
		default:
			break;
	}
}


void CTestHikvisionDlg::ptzCtrlStop(PTZCMD cmd)
{
	std::string cameraId = getCameraId();
	if (cameraId.size() == 0)
		return;

	switch (cmd) {
		case DIRECT_UP:
			COpenSdkMgr::GetInstance()->PTZCtrl(m_sessionId, m_accessToken, cameraId, UP, STOP, PTZ_SPEED);
			break;
		case DIRECT_DOWN:
			COpenSdkMgr::GetInstance()->PTZCtrl(m_sessionId, m_accessToken, cameraId, DOWN, STOP, PTZ_SPEED);
			break;
		case DIRECT_LEFT:
			COpenSdkMgr::GetInstance()->PTZCtrl(m_sessionId, m_accessToken, cameraId, LEFT, STOP, PTZ_SPEED);
			break;
		case DIRECT_RIGHT:
			COpenSdkMgr::GetInstance()->PTZCtrl(m_sessionId, m_accessToken, cameraId, RIGHT, STOP, PTZ_SPEED);
			break;
		case DIRECT_UPLEFT:
			COpenSdkMgr::GetInstance()->PTZCtrl(m_sessionId, m_accessToken, cameraId, UPLEFT, STOP, PTZ_SPEED);
			break;
		case DIRECT_DOWNLEFT:
			COpenSdkMgr::GetInstance()->PTZCtrl(m_sessionId, m_accessToken, cameraId, DOWNLEFT, STOP, PTZ_SPEED);
			break;
		case DIRECT_UPRIGHT:
			COpenSdkMgr::GetInstance()->PTZCtrl(m_sessionId, m_accessToken, cameraId, UPRIGHT, STOP, PTZ_SPEED);
			break;
		case DIRECT_DOWNRIGHT:
			COpenSdkMgr::GetInstance()->PTZCtrl(m_sessionId, m_accessToken, cameraId, DOWNRIGHT, STOP, PTZ_SPEED);
			break;
		case PTZ_ZOOMIN:
			COpenSdkMgr::GetInstance()->PTZCtrl(m_sessionId, m_accessToken, cameraId, ZOOMIN, STOP, PTZ_SPEED);
			break;
		case PTZ_ZOOMOUT:
			COpenSdkMgr::GetInstance()->PTZCtrl(m_sessionId, m_accessToken, cameraId, ZOOMOUT, STOP, PTZ_SPEED);
			break;
		//case PTZ_IRISSTOPUP:
		//	COpenSdkMgr::GetInstance()->PTZCtrl(m_sessionId, m_accessToken, cameraId, IRISSTOPUP, STOP, PTZ_SPEED);
		//	break;
		case PTZ_IRISSTOPDOWN:
			COpenSdkMgr::GetInstance()->PTZCtrl(m_sessionId, m_accessToken, cameraId, IRISSTOPDOWN, STOP, PTZ_SPEED);
			break;
		default:
			break;
	}
}




void CTestHikvisionDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CDialogEx::OnLButtonDown(nFlags, point);
}


void CTestHikvisionDlg::OnBnClickedButton8()
{
	CRect rc;
	m_btnLogin.GetWindowRect(rc);
	CMenu menu;
	menu.CreatePopupMenu();
	menu.InsertMenu(0, MF_BYPOSITION, 1, L"by OAuth");
	menu.InsertMenu(1, MF_BYPOSITION, 2, L"by sms sign");
	menu.InsertMenu(2, MF_BYPOSITION, 3, L"by hdware sign");
	int ret = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
								  rc.left, rc.bottom,
								  //point.x, point.y,
								  this);
	switch (ret) {
		case 1:
			LoginByOAuth();
			break;
		case 2:
			LoginByPrivateCloud();
			break;
		case 3:
			break;
		default:
			break;
	}
}


void CTestHikvisionDlg::LoginByOAuth()
{
	m_accessToken = COpenSdkMgr::GetInstance()->login();
	if (m_accessToken.size() > 0)
		OnBnClickedButtonGetDevList();
}


bool generate_signature(std::string& signature, 
						const std::string& userId, 
						const std::string& phone,
						const std::string& method,
						const time_t& timestamp,
						std::string& appSecret)
{
	// type:1,
	char buff[1024] = { 0 };
	sprintf_s(buff, "phone:%s,userId:%s,method:%s,time:%d,secret:",
			  phone.c_str(), userId.c_str(), method.c_str(), timestamp);
	std::string strbuff = buff;
	strbuff += appSecret;
	const char* a = strbuff.c_str();
	LOGA(a);
	util::MD5 md5;
	md5.update(strbuff);
	md5.digest();
	signature = md5.toString();
	return true;
}


void CTestHikvisionDlg::LoginByPrivateCloud()
{
	const std::string userId("00000001");
	const std::string phone("18706734652");
	const std::string method("token/accessToken/get");

	time_t now = time(NULL);
	std::string signature;
	generate_signature(signature, userId, phone, method, now, m_appSecret);
	char raw_sign[1024] = { 0 };
	//const char* sig = signature.c_str();\"type\":1,
	sprintf_s(raw_sign, 
		"{\"id\":\"123456\",\"method\":\"%s\",\"params\":{\"phone\":\"%s\",\"userId\":\"%s\"},\"system\":{\"key\":\"%s\",\"sign\":\"%s\",\"time\":%d,\"ver\":\"1.0\"}}",
		method.c_str(), phone.c_str(), userId.c_str(), m_appKey.c_str(),
		signature.c_str(), now);


	LOGA(signature.c_str());
	LOGA(raw_sign);

	COpenSdkMgr* dll = COpenSdkMgr::GetInstance();
	const char* url = "https://open.ys7.com/api/method";
	//int ret = dll->GetAccessTokenSmsCode(raw_sign);
	char* buf = NULL; int len = 0;
	int ret = dll->HttpSendWithWait(url, raw_sign, "", &buf, &len);
	if (ret == 0 && buf) {
		std::string rep = buf;
		dll->freeData(buf);
	}
}


void CTestHikvisionDlg::OnBnClickedButtonGetDevList()
{
	void* buff = NULL;
	int l = 0;
	COpenSdkMgr::GetInstance()->getDevList(m_accessToken, 0, 1500, &buff, &l);
	std::string json = static_cast<char*>(buff);
	LOGA(json.c_str());
	COpenSdkMgr::GetInstance()->freeData(buff);

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
}
