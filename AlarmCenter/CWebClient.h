#pragma once
#include <cef_client.h>

class CWebClient 
	: public CefClient
	, public CefLifeSpanHandler
{
protected:
	CefRefPtr<CefBrowser> m_Browser;

public:
	CWebClient(void){};
	virtual ~CWebClient(void){};

	CefRefPtr<CefBrowser> GetBrowser() { return m_Browser; }

	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE
	{ return this; }

	virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;

	// Ìí¼ÓCEFµÄSPÐéº¯Êý
	IMPLEMENT_REFCOUNTING(CWebClient);
	IMPLEMENT_LOCKING(CWebClient);
};
