// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_

#include "include/cef_app.h"

class SimpleApp :
	public CefApp,
	public CefBrowserProcessHandler,
	public CefRenderProcessHandler
{
public:
	SimpleApp();

	// CefApp methods:
	virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
		return this;
	}

	virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override {
		return this;
	}

	// CefBrowserProcessHandler methods:
	virtual void OnContextInitialized() override;
	virtual void OnContextCreated(CefRefPtr<CefBrowser> browser,
								  CefRefPtr<CefFrame> frame,
								  CefRefPtr<CefV8Context> context) override;

	// CefRenderProcessHandler methods:
	virtual void OnWebKitInitialized() override;

	

private:
	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(SimpleApp);
};

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
