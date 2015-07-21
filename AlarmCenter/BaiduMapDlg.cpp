// BaiduMapDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "BaiduMapDlg.h"
#include <sstream>
#include "UserInfo.h"

// CBaiduMapDlg dialog

IMPLEMENT_DYNCREATE(CBaiduMapDlg, CDHtmlDialog)

CBaiduMapDlg::CBaiduMapDlg(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(CBaiduMapDlg::IDD, CBaiduMapDlg::IDH, pParent)
	, m_pRealParent(NULL)
{

}

CBaiduMapDlg::~CBaiduMapDlg()
{
}

void CBaiduMapDlg::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnUsePt);
}


static void __stdcall OnCurUserChanged(void* udata, const core::CUserInfo* user)
{
	if (!udata || !user)
		return;

	CBaiduMapDlg* dlg = reinterpret_cast<CBaiduMapDlg*>(udata);
	if (user->get_user_priority() == core::UP_OPERATOR) {
		dlg->m_btnUsePt.EnableWindow(0);
	} else {
		dlg->m_btnUsePt.EnableWindow(1);
	}
}


BOOL CBaiduMapDlg::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();
	m_url = GetModuleFilePath();
	m_url += L"\\config";
	CreateDirectory(m_url.c_str(), NULL);
	m_url += L"\\baidu.html";


	core::CUserManager::GetInstance()->RegisterObserver(this, OnCurUserChanged);
	OnCurUserChanged(this, core::CUserManager::GetInstance()->GetCurUserInfo());

	return TRUE;  // return TRUE  unless you set the focus to a control
}




BEGIN_MESSAGE_MAP(CBaiduMapDlg, CDHtmlDialog)
	ON_BN_CLICKED(IDOK, &CBaiduMapDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_RESET, &CBaiduMapDlg::OnBnClickedButtonReset)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CBaiduMapDlg)
	DHTML_EVENT_ONCLICK(_T("ButtonOK"), OnButtonOK)
	DHTML_EVENT_ONCLICK(_T("ButtonCancel"), OnButtonCancel)
END_DHTML_EVENT_MAP()


// CBaiduMapDlg message handlers

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


void CBaiduMapDlg::OnBnClickedOk()
{
	AUTO_LOG_FUNCTION;
	IHTMLDocument2 *pDocument;
	HRESULT hr = GetDHtmlDocument(&pDocument);

	CComQIPtr<IHTMLDocument2> spDoc(pDocument);
	IDispatchPtr spDisp;
	hr = spDoc->get_Script(&spDisp);

	double x, y;
	if (spDisp) {
		{
			OLECHAR FAR *szMember = L"GetX";
			DISPID dispid;
			CComVariant varRet;
			static BYTE params[] = VTS_BSTR;

			hr = spDisp->GetIDsOfNames(IID_NULL, &szMember, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
			COleDispatchDriver dispDriver(spDisp, FALSE);
			dispDriver.InvokeHelper(dispid, DISPATCH_METHOD, VT_VARIANT, &varRet, params, L"GetX");

			varRet.ChangeType(VT_R8);
			x = varRet.dblVal;
		}

		{
			OLECHAR FAR *szMember = L"GetY";
			DISPID dispid;
			CComVariant varRet;
			static BYTE params[] = VTS_BSTR;

			hr = spDisp->GetIDsOfNames(IID_NULL, &szMember, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
			COleDispatchDriver dispDriver(spDisp, FALSE);
			dispDriver.InvokeHelper(dispid, DISPATCH_METHOD, VT_VARIANT, &varRet, params, L"GetY");

			varRet.ChangeType(VT_R8);
			y = varRet.dblVal;
		}
		
		m_coor.x = x;
		m_coor.y = y;
		if (m_pRealParent) {
			m_pRealParent->PostMessageW(WM_CHOSEN_BAIDU_PT);
		}

		ShowCoordinate(m_coor, m_title);
	}
}


void CBaiduMapDlg::OnCancel()
{
	return;
}


bool CBaiduMapDlg::VoidCall(const wchar_t* funcName)
{
	AUTO_LOG_FUNCTION;
	IHTMLDocument2 *pDocument;
	HRESULT hr = GetDHtmlDocument(&pDocument);

	CComQIPtr<IHTMLDocument2> spDoc(pDocument);
	IDispatchPtr spDisp;
	hr = spDoc->get_Script(&spDisp);
	if (FAILED(hr)) {
		return false;
	}

	OLECHAR FAR *szMember = const_cast<wchar_t*>(funcName);
	DISPID dispid;
	hr = spDisp->GetIDsOfNames(IID_NULL, &szMember, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
	if (FAILED(hr)) {
		return false;
	}

	CComVariant varRet;
	COleDispatchDriver dispDriver(spDisp, FALSE);
	dispDriver.InvokeHelper(dispid, DISPATCH_METHOD, VT_VARIANT, &varRet, NULL);

	return true;
}


void CBaiduMapDlg::OnBnClickedButtonReset()
{
	AUTO_LOG_FUNCTION;
	VoidCall(L"MyRefresh");
}


bool CBaiduMapDlg::GenerateHtml(std::wstring& url, 
								const web::BaiduCoordinate& coor, 
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
	function GetX(){\r\n\
		return g_x;\r\n\
	}\r\n\
\r\n\
	function GetY() {\r\n\
		return g_y;\r\n\
	}\r\n\
\r\n\
	function initialize() {\r\n\
		g_x = " << coor.x << L";\r\n\
		g_y = " << coor.y << L";\r\n\
		var point = new BMap.Point(" << coor.x << L"," << coor.y << L");\r\n\
		var map = new BMap.Map(\"allmap\",{minZoom:1,maxZoom:20});\r\n\
		map.centerAndZoom(point, 14);  \r\n\
		map.enableScrollWheelZoom(true);\r\n\
		map.addControl(new BMap.NavigationControl());\r\n\
		\r\n\
		var marker = new BMap.Marker(point);  \r\n\
		var label = new BMap.Label(\"" << stitle << L"\",{offset:new BMap.Size(20,-10)});\r\n\
		marker.setLabel(label) \r\n\
		map.addOverlay(marker);  \r\n\
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


bool CBaiduMapDlg::ShowCoordinate(const web::BaiduCoordinate& coor, const CString& title)
{
	AUTO_LOG_FUNCTION;
	if (GenerateHtml(m_url, coor, title)) {
		Navigate(m_url.c_str());
		return true;
	}
	return false;
}





bool CBaiduMapDlg::ShowDrivingRoute(const web::BaiduCoordinate& coor_start,
									const web::BaiduCoordinate& coor_end,
									const std::wstring& name_start,
									const std::wstring& name_end)
{
	AUTO_LOG_FUNCTION;
	CRect rc;
	GetWindowRect(rc);
	CRect rcLeft(rc);
	CRect rcRight(rc);
	rcRight.left = rc.right - 350;
	rcLeft.right = rcRight.left - 50;

	std::wostringstream wos;
	wos << L"<!DOCTYPE html>\r\n\
<html>\r\n\
<head>\r\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\r\n\
<title>百度驾车导航</title>\r\n\
<script type=\"text/javascript\">\r\n\
	function initialize(){\r\n\
		var map = new BMap.Map(\"container\",{minZoom:1,maxZoom:20});\r\n\
		var pt_start = new BMap.Point(" << coor_start.x << L", " << coor_start.y << L"); \r\n\
		var pt_end   = new BMap.Point(" << coor_end.x << L", " << coor_end.y << L");\r\n\
		map.centerAndZoom(pt_start, 14);\r\n\
		map.enableScrollWheelZoom(true);\r\n\
		map.addControl(new BMap.NavigationControl());\r\n\
\r\n\
		var marker_start = new BMap.Marker(pt_start);\r\n\
		var label_start = new BMap.Label(\"" << name_start.c_str() << L"\",{offset:new BMap.Size(20,-10)});\r\n\
		marker_start.setLabel(label_start);\r\n\
\r\n\
		var marker_end = new BMap.Marker(pt_end);\r\n\
		var label_end = new BMap.Label(\"" << name_end.c_str() << L"\",{offset:new BMap.Size(20,-10)});\r\n\
		marker_end.setLabel(label_end);\r\n\
\r\n\
		map.addOverlay(marker_start);\r\n\
		map.addOverlay(marker_end);\r\n\
\r\n\
		function findWay(){\r\n\
			var driving = new BMap.DrivingRoute(map, {renderOptions:{map:map, panel:\"divResult\", autoViewport:true}});\r\n\
			driving.setSearchCompleteCallback(function(result){\r\n\
				if(driving.getStatus() == BMAP_STATUS_SUCCESS) {\r\n\
					//alert(\"搜索到路线!\");\r\n\
				} else {\r\n\
					alert(\"Route failed!\");\r\n\
				}\r\n\
			})\r\n\
			driving.search(pt_start,pt_end);\r\n\
		}\r\n\
		findWay();\r\n\
	}\r\n\
\r\n\
	function loadScript() {\r\n\
	   var script = document.createElement(\"script\");\r\n\
	   script.src = \"http://api.map.baidu.com/api?v=2.0&ak=dEVpRfhLB3ITm2Eenn0uEF3w&callback=initialize&app=hb|alarmcenter\";\r\n\
	   document.body.appendChild(script);\r\n\
	}\r\n\
\r\n\
    window.onload = loadScript;\r\n\
</script>\r\n\
</head>\r\n\
<body>\r\n\
    <div style=\"clear:both;\">\r\n\
		<div style=\"float:left;width:" << rcLeft.Width() << L"px; height:" << rc.Height() << L"px; border:1px solid gray\" id=\"container\"></div> \r\n\
		<div id=\"divResult\" style=\"float:left;width:350px; height:" << rc.Height() << L"px; background:#eee\"></div>\r\n\
    </div>\r\n\
</body>\r\n\
</html>";

	std::wstring html;
	html = wos.str();
	CFile file;
	if (file.Open(m_url.c_str(), CFile::modeCreate | CFile::modeWrite)) {
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
		Navigate(m_url.c_str());
		return true;
	}
	return false;
}



void CBaiduMapDlg::OnDestroy()
{
	CDHtmlDialog::OnDestroy();

	core::CUserManager::GetInstance()->UnRegisterObserver(this);
}
