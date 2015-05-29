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
		//��дGetExternal����һ��ClientCall����
		*ppDispatch = reinterpret_cast<IDispatch*>(m_pClientCall);
		return S_OK;
	}
	DECLARE_UNCOPYABLE(CMyWebBrowser)
};

