
// TestBaiduMapDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TestBaiduMap.h"
#include "TestBaiduMapDlg.h"
#include "afxdialogex.h"
#include <Mshtml.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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


// CTestBaiduMapDlg dialog



CTestBaiduMapDlg::CTestBaiduMapDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTestBaiduMapDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestBaiduMapDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EXPLORER1, m_ie);
	DDX_Control(pDX, IDC_EDIT1, m_url);
}

BEGIN_MESSAGE_MAP(CTestBaiduMapDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CTestBaiduMapDlg::OnBnClickedButton1)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CTestBaiduMapDlg message handlers

BOOL CTestBaiduMapDlg::OnInitDialog()
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

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTestBaiduMapDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CTestBaiduMapDlg::OnPaint()
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
HCURSOR CTestBaiduMapDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CTestBaiduMapDlg::OnBnClickedButton1()
{
	//const wchar_t* url = L"http://map.baidu.com/";
	//const wchar_t* url = L"http://api.map.baidu.com/marker?location=39.916979519873,116.41004950566&title=我的位置&content=百度奎科大厦&output=html";
	//const wchar_t* url = L"http://api.map.baidu.com/geocoder?location=39.990912172420714,116.32715863448607&coord_type=gcj02&output=html&src=HB|AlarmCenter";
	//const wchar_t* url = L"http://api.map.baidu.com/place/search?query=海底捞&location=31.204055632862,121.41117785465&radius=1000&region=上海&output=html&src=HB|AlarmCenter";
	//const wchar_t* url = L"http://api.map.baidu.com/place/detail?uid=d3099bdd81c525dbd1f49ee6&output=html&src=HB|AlarmCenter";
	//const wchar_t* url = L"http://j.map.baidu.com/cKY91";
	UpdateData();
	CString url;
	m_url.GetWindowTextW(url);
	m_ie.Navigate(url, NULL, NULL, NULL, NULL);
}


void CTestBaiduMapDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	if (IsWindow(m_hWnd) && IsWindow(m_ie.m_hWnd)) {
		CRect rc;
		GetClientRect(rc);
		rc.DeflateRect(0, 50, 0, 0);
		m_ie.MoveWindow(rc);
	}
}
BEGIN_EVENTSINK_MAP(CTestBaiduMapDlg, CDialogEx)
	ON_EVENT(CTestBaiduMapDlg, IDC_EXPLORER1, 259, CTestBaiduMapDlg::DocumentCompleteExplorer1, VTS_DISPATCH VTS_PVARIANT)
END_EVENTSINK_MAP()


void CTestBaiduMapDlg::DocumentCompleteExplorer1(LPDISPATCH pDisp, VARIANT* URL)
{
	CComPtr<IDispatch> disp = pDisp;
	/*CComPtr<IHTMLDocument2>	doc = NULL;
	HRESULT hr = disp->QueryInterface(IID_IHTMLDocument2, (void**)&doc);
	if (FAILED(hr)) {
		return;
	}
	hr = doc->get_Script(&disp);*/
	CString strFunc = L"Add";
	CComBSTR bstrMember(strFunc);
	DISPID dispid = NULL;
	HRESULT hr = disp->GetIDsOfNames(IID_NULL, &bstrMember, 1,
									 LOCALE_SYSTEM_DEFAULT, &dispid);
	if (FAILED(hr)) {
		return;
	}

	CStringArray paramArray;
	paramArray.Add(L"1");
	paramArray.Add(L"2");

	const int arraySize = paramArray.GetSize();

	DISPPARAMS dispparams;
	memset(&dispparams, 0, sizeof dispparams);
	dispparams.cArgs = 1;
	dispparams.rgvarg = new VARIANT[dispparams.cArgs];

	
	for (int i = 0; i < arraySize; i++) {
		CComBSTR bstr = paramArray.GetAt(arraySize - 1 - i); // back reading
		bstr.CopyTo(&dispparams.rgvarg[i].bstrVal);
		dispparams.rgvarg[i].vt = VT_BSTR;
	}
	dispparams.cNamedArgs = 0;

	EXCEPINFO excepInfo;
	memset(&excepInfo, 0, sizeof excepInfo);
	CComVariant vaResult;
	UINT nArgErr = (UINT)-1;  // initialize to invalid arg

	hr = disp->Invoke(dispid, IID_NULL, 0,
						  DISPATCH_METHOD, &dispparams, &vaResult, &excepInfo, &nArgErr);

	delete[] dispparams.rgvarg;
	if (FAILED(hr)) {
		return;
	}

}
