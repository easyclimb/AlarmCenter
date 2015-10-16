
// BaiduMapDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "BaiduMap.h"
#include "BaiduMapDlg.h"
#include "afxdialogex.h"
#include <sstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CBaiduMapDlg 对话框

BEGIN_DHTML_EVENT_MAP(CBaiduMapDlg)
	DHTML_EVENT_ONCLICK(_T("ButtonOK"), OnButtonOK)
	DHTML_EVENT_ONCLICK(_T("ButtonCancel"), OnButtonCancel)
END_DHTML_EVENT_MAP()


CBaiduMapDlg::CBaiduMapDlg(CWnd* pParent /*=nullptr*/)
	: CDHtmlDialog(CBaiduMapDlg::IDD, CBaiduMapDlg::IDH, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_title = L"";
	m_url = L"";
}

void CBaiduMapDlg::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CBaiduMapDlg, CDHtmlDialog)
	ON_WM_COPYDATA()
END_MESSAGE_MAP()


// CBaiduMapDlg 消息处理程序

BOOL CBaiduMapDlg::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CBaiduMapDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDHtmlDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CBaiduMapDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

HRESULT CBaiduMapDlg::OnButtonOK(IHTMLElement* /*pElement*/)
{
	OnOK();
	return S_OK;
}

HRESULT CBaiduMapDlg::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK;
}


BOOL CBaiduMapDlg::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct)
{
	std::wstring data = reinterpret_cast<const wchar_t*>(pCopyDataStruct->lpData);
	double x, y;
	int level;
	wchar_t title[1024] = { 0 };
	try {
		int ret = _tscanf(data.c_str(), L"%lf, %lf, %d, %s", &x, &y, &level, &title);
		if (ret == 4) {
			GenerateHtml(m_url, x, y, level, title);
			Navigate(m_url.c_str());
		}
	} catch (...) {}

	return CDHtmlDialog::OnCopyData(pWnd, pCopyDataStruct);
}


bool CBaiduMapDlg::GenerateHtml(std::wstring& url, 
								double x,
								double y,
								int zoomLevel,
								const CString& title)
{
	AUTO_LOG_FUNCTION;
	m_title = title;
	CRect rc;
	GetClientRect(rc);
	CString /*sAlarmCenter, */sCoordinate;
	//sAlarmCenter.LoadStringW(IDS_STRING_ALARM_CENTER);
	sCoordinate.LoadStringW(IDS_STRING_COORDINATE);
	LPCTSTR stitle = m_title.LockBuffer();
	LPCTSTR scoor = sCoordinate.LockBuffer();
	std::wostringstream wostr;
	std::wstring html;
	wostr << L"\
<!DOCTYPE html>\r\n\
<html>\r\n\
<head>\r\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\r\n\
<title>" << stitle << L"</title>\r\n\
<script type=\"text/javascript\">\r\n\
	function MyRefresh() {\r\n\
		window.location.reload(true);\r\n\
	}\r\n\
\r\n\
	var g_x = 0.0;\r\n\
	var g_y = 0.0;\r\n\
	var g_zoomLevel = 14;\r\n\
	var g_map;\r\n\
\r\n\
	function GetX(){\r\n\
		return g_x;\r\n\
	}\r\n\
\r\n\
	function GetY() {\r\n\
		return g_y;\r\n\
	}\r\n\
\r\n\
	function GetZoom() {\r\n\
		return g_map.getZoom();\r\n\
	}\r\n\
\r\n\
	function initialize() {\r\n\
		g_x = " << x << L";\r\n\
		g_y = " << y << L";\r\n\
		g_zoomLevel = " << zoomLevel << L";\r\n\
		var point = new BMap.Point(" << x << L"," << y << L");\r\n\
		g_map = new BMap.Map(\"allmap\",{minZoom:1,maxZoom:20});\r\n\
		g_map.centerAndZoom(point, g_zoomLevel);  \r\n\
		g_map.enableScrollWheelZoom(true);\r\n\
		g_map.addControl(new BMap.NavigationControl());\r\n\
		\r\n\
		var marker = new BMap.Marker(point);  \r\n\
		var label = new BMap.Label(\"" << stitle << L"\",{offset:new BMap.Size(20,-10)});\r\n\
		marker.setLabel(label) \r\n\
		g_map.addOverlay(marker);  \r\n\
		marker.enableDragging(); \r\n\
		marker.addEventListener(\"dragend\", function(e){ \r\n\
			document.getElementById(\"r-result\").innerHTML = e.point.lng + \", \" + e.point.lat;\r\n\
			g_x = e.point.lng;\r\n\
			g_y = e.point.lat;\r\n\
		});\r\n\
	}\r\n\
\r\n\
	function loadScript() {\r\n\
	   var script = document.createElement(\"script\");\r\n\
	   script.src = \"http://api.map.baidu.com/api?v=2.0&ak=dEVpRfhLB3ITm2Eenn0uEF3w&callback=initialize\";\r\n\
	   document.body.appendChild(script);\r\n\
	}\r\n\
\r\n\
	window.onload = loadScript;\r\n\
</script></head><body>\r\n\
<div id=\"r-result\" style=\"float:left;width:100px;\">" << scoor << L"</div>\r\n\
<div id=\"allmap\" style=\"width:" << rc.Width() << L"px; height:" << rc.Height() << L"px\"></div></body></html>\r\n";
	html = wostr.str();
	m_title.UnlockBuffer();
	sCoordinate.UnlockBuffer();
	
	CFile file;
	if (file.Open(url.c_str(), CFile::modeCreate | CFile::modeWrite)) {
		//USES_CONVERSION;
		//const char* a = W2A(html);
		//int out_len = 0;
		//const char* utf8 = Utf16ToUtf8(html, out_len);
		std::string utf8;
		utf8::utf16to8(html.begin(), html.end(), std::back_inserter(utf8));
		file.Write(utf8.c_str(), utf8.size());
		//file.Write(html.c_str(), html.size());
		file.Close();
		//delete[out_len+1] utf8;
		return true;
	}
	return false;
}


