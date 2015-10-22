/************************************************************************************************
*   Copyright (c) 2013 Álan Crístoffer
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy
*   of this software and associated documentation files (the "Software"), to deal
*   in the Software without restriction, including without limitation the rights
*   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
*   of the Software, and to permit persons to whom the Software is furnished to do so,
*   subject to the following conditions:
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
*   INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
*   PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
*   FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
*   OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
*   DEALINGS IN THE SOFTWARE.
************************************************************************************************/
#include "stdafx.h"  
#include "ClientApp.h"  

#include "ClientHandler.h"  
#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/wrapper/cef_helpers.h"

ClientApp::ClientApp()
{}

void ClientApp::OnContextInitialized()
{
//	CEF_REQUIRE_UI_THREAD();
//
//	// Information used when creating the native window.
//	CefWindowInfo window_info;
//
//#if defined(OS_WIN)
//	// On Windows we need to specify certain flags that will be passed to
//	// CreateWindowEx().
//	window_info.SetAsPopup(NULL, "cefsimple");
//#endif
//
//	// SimpleHandler implements browser-level callbacks.
//	CefRefPtr<ClientHandler> handler(new ClientHandler());
//
//	// Specify CEF browser settings here.
//	CefBrowserSettings browser_settings;
//
//	std::string url;
//
//	// Check if a "--url=" value was provided via the command-line. If so, use
//	// that instead of the default URL.
//	CefRefPtr<CefCommandLine> command_line =
//		CefCommandLine::GetGlobalCommandLine();
//	url = command_line->GetSwitchValue("url");
//	if (url.empty())
//		url = "http://www.baidu.com";
//
//	// Create the first browser window.
//	CefBrowserHost::CreateBrowser(window_info, handler.get(), url,
//								  browser_settings, NULL);
}
