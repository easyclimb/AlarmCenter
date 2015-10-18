#include"stdafx.h"
#include"CWebClient.h"

void CWebClient::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
	m_Browser = browser;
}
