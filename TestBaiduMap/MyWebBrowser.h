#pragma once
#include "WebBrowser.h"


class ClientCall;
class CMyWebBrowser : public CWebBrowserBase
{
	HWND m_hWnd;
	ClientCall *m_pClientCall;
public:
	CMyWebBrowser();
	~CMyWebBrowser();
	void SetHwnd(HWND hwnd) { m_hWnd = hwnd; }
	virtual void OnDocumentCompleted();
	virtual HWND GetHWND() { return m_hWnd; }
	virtual HRESULT STDMETHODCALLTYPE GetExternal(IDispatch **ppDispatch)
	{
		//重写GetExternal返回一个ClientCall对象
		*ppDispatch = reinterpret_cast<IDispatch*>(m_pClientCall);
		return S_OK;
	}
	DECLARE_UNCOPYABLE(CMyWebBrowser)
};

