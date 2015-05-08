// TestDHtmlDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TestBaiduMap.h"
#include "TestDHtmlDlg.h"
#include <MsHTML.h>

// CTestDHtmlDlg dialog

IMPLEMENT_DYNCREATE(CTestDHtmlDlg, CDHtmlDialog)

CTestDHtmlDlg::CTestDHtmlDlg(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(CTestDHtmlDlg::IDD, CTestDHtmlDlg::IDH, pParent)
{

}

CTestDHtmlDlg::~CTestDHtmlDlg()
{
}

void CTestDHtmlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
}

BOOL CTestDHtmlDlg::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();
	Navigate(L"D:\\dev\\web\\a.html");
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CTestDHtmlDlg, CDHtmlDialog)
	ON_BN_CLICKED(IDOK, &CTestDHtmlDlg::OnBnClickedOk)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CTestDHtmlDlg)
	DHTML_EVENT_ONCLICK(_T("ButtonOK"), OnButtonOK)
	DHTML_EVENT_ONCLICK(_T("ButtonCancel"), OnButtonCancel)
END_DHTML_EVENT_MAP()



// CTestDHtmlDlg message handlers

HRESULT CTestDHtmlDlg::OnButtonOK(IHTMLElement* /*pElement*/)
{
	


	OnOK();
	return S_OK;
}

HRESULT CTestDHtmlDlg::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK;
}


void CTestDHtmlDlg::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
	CDHtmlDialog::OnDocumentComplete(pDisp, szUrl);
	

}


void CTestDHtmlDlg::OnBnClickedOk()
{
	/*IHTMLDocument2*   pDocument;
	HRESULT   hr = GetDHtmlDocument(&pDocument);
	if (FAILED(hr)) {
		return ;
	}
	IHTMLWindow2*   pWindow;
	hr = pDocument->get_parentWindow(&pWindow);
	if (FAILED(hr)) {
		return ;
	}

	VARIANT   ret;
	ret.vt = VT_EMPTY;
	hr = pWindow->execScript(L"GetX()", L"javascript", &ret);
	if (FAILED(hr)) {
		return ;
	}
	CString x, y;
	x.Format(L"x=%f", ret.dblVal);
	hr = pWindow->execScript(L"GetY()", L"javascript", &ret);
	if (FAILED(hr)) {
		return ;
	}
	y.Format(L"y=%f", ret.dblVal);
	MessageBox(x + y);*/


	IHTMLDocument2 *pDocument;
	HRESULT hr = GetDHtmlDocument(&pDocument);

	CComQIPtr<IHTMLDocument2> spDoc(pDocument);
	IDispatchPtr spDisp;
	hr = spDoc->get_Script(&spDisp);
	

	if (spDisp) {
		OLECHAR FAR *szMember = L"GetX";
		DISPID dispid;
		CComVariant varRet;
		static BYTE params[] = VTS_BSTR;

		hr = spDisp->GetIDsOfNames(IID_NULL, &szMember, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
		COleDispatchDriver dispDriver(spDisp, FALSE);
		dispDriver.InvokeHelper(dispid, DISPATCH_METHOD, VT_VARIANT, &varRet, params, L"GetX");

		varRet.ChangeType(VT_BSTR);
		//CString x, y;
		AfxMessageBox(TEXT("∑µªÿ÷µ «£∫") + CString(varRet.bstrVal));
	}
}
