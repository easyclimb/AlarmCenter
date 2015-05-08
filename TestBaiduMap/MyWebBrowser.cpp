#include "stdafx.h"
#include "MyWebBrowser.h"
#include "js.h"


class ClientCall :public IDispatch
{
	long _refNum;
public:
	ClientCall() { _refNum = 1; }
	~ClientCall(void) {}
public:
	// IUnknown Methods

	STDMETHODIMP QueryInterface(REFIID iid, void**ppvObject)
	{
		*ppvObject = NULL;
		if (iid == IID_IUnknown) *ppvObject = this;
		else if (iid == IID_IDispatch) *ppvObject = (IDispatch*)this;
		if (*ppvObject) { AddRef(); return S_OK; }
		return E_NOINTERFACE;
	}

	STDMETHODIMP_(ULONG) AddRef() { return ::InterlockedIncrement(&_refNum); }

	STDMETHODIMP_(ULONG) Release()
	{
		::InterlockedDecrement(&_refNum);
		if (_refNum == 0) { delete this; }
		return _refNum;
	}

	// IDispatch Methods

	HRESULT _stdcall GetTypeInfoCount(unsigned int * pctinfo) { return E_NOTIMPL; }

	HRESULT _stdcall GetTypeInfo(unsigned int iTInfo, LCID lcid, 
								 ITypeInfo FAR* FAR* ppTInfo)
	{
		return E_NOTIMPL;
	}

	HRESULT _stdcall GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames,
								   unsigned int cNames, LCID lcid, DISPID FAR* rgDispId)
	{
		if (lstrcmp(rgszNames[0], L"CppCall") == 0) {
			//��ҳ����window.external.CppCallʱ����������������ȡCppCall��ID
			*rgDispId = 100;
		}
		return S_OK;
	}

	HRESULT _stdcall Invoke(DISPID dispIdMember, REFIID riid, LCID lcid,
							WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult,
							EXCEPINFO* pExcepInfo, unsigned int* puArgErr)
	{
		if (dispIdMember == 100) {
			//��ҳ����CppCallʱ������ݻ�ȡ����ID����Invoke����
			int cnt = pDispParams->cArgs;
			int a = pDispParams->rgvarg[0].intVal;
			int b = pDispParams->rgvarg[1].intVal;
			//CppCall(a, b);
		}
		return S_OK;
	}
};

///////////////////////////////////////////////////////////////////////////////

CMyWebBrowser::CMyWebBrowser()
	: m_pClientCall(NULL)
{
	m_pClientCall = new ClientCall();
}


CMyWebBrowser::~CMyWebBrowser() 
{
	//SAFEDELETEP(m_pClientCall);
}




static void __stdcall button1_onclick(LPVOID pParam)
{
	CMyWebBrowser *pMainForm = (CMyWebBrowser*)pParam;
	//MessageBox(pMainForm->m_hWnd, L"�������button1", L"��ʾ(C++)", 0);
}


void CMyWebBrowser::OnDocumentCompleted()
{
	VARIANT params[10];

	//��ȡwindow
	IDispatch *pHtmlWindow = CWebBrowserBase::GetHtmlWindow();

	//��ȡdocument
	CVariant document;
	params[0].vt = VT_BSTR;
	params[0].bstrVal = L"document";
	CWebBrowserBase::GetProperty(pHtmlWindow, L"document", &document);

	//��ȡbutton1
	CVariant button1;
	params[0].vt = VT_BSTR;
	params[0].bstrVal = L"button1";
	CWebBrowserBase::InvokeMethod(document.pdispVal, L"getElementById", &button1, params, 1);

	//����button1��onclick�¼�
	params[0].vt = VT_DISPATCH;
	params[0].pdispVal = new JsFunction(button1_onclick, this);
	CWebBrowserBase::SetProperty(button1.pdispVal, L"onclick", params);
}
