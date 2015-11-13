// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
#include "stdafx.h"
#include "simple_app.h"

#include <string>

#include "simple_handler.h"
#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/wrapper/cef_helpers.h"
#include "include/cef_v8.h"

SimpleApp::SimpleApp() {
}

void SimpleApp::OnContextInitialized() {
	CEF_REQUIRE_UI_THREAD();

	
}

//class CCefV8Accessor : public CefV8Accessor
//{
//public:
//	CCefV8Accessor() {}
//	virtual bool Get(const CefString& name,
//					 const CefRefPtr<CefV8Value> object,
//					 CefRefPtr<CefV8Value>& retval,
//					 CefString& exception) OVERRIDE {
//		return true;
//	}
//	virtual bool Set(const CefString& name,
//					 const CefRefPtr<CefV8Value> object,
//					 const CefRefPtr<CefV8Value> value,
//					 CefString& exception) OVERRIDE {
//		return true;
//	}
//	IMPLEMENT_REFCOUNTING(CCefV8Accessor);
//};

class CCefV8Handler : public CefV8Handler
{
public:
	CCefV8Handler(){}
	virtual ~CCefV8Handler(){}

	virtual bool Execute(const CefString& name,
						 CefRefPtr<CefV8Value> object,
						 const CefV8ValueList& arguments,
						 CefRefPtr<CefV8Value>& retval,
						 CefString& exception) override;
private:
	IMPLEMENT_REFCOUNTING(CCefV8Handler);
};

void SimpleApp::OnContextCreated(CefRefPtr<CefBrowser> brawser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
{
	/*CefRefPtr<CefV8Value> window = context->GetGlobal();
	
	CefRefPtr<CefV8Accessor> myV8Acc = new CCefV8Accessor;
	CefRefPtr<CefV8Value> val = CefV8Value::CreateString(L"Application");
	CefString cefException;
	myV8Acc->Set(L"name", window, val, cefException);
	CefRefPtr<CefV8Value> pObjApp = CefV8Value::CreateObject(myV8Acc);
	window->SetValue(L"Application", pObjApp, V8_PROPERTY_ATTRIBUTE_NONE);

	CefRefPtr<CefV8Handler> myV8handle = new CCefV8Handler();
	CefRefPtr<CefV8Value> myFun = CefV8Value::CreateFunction(L"SetAppState", myV8handle);
	static_cast<CCefV8Handler*>(myV8handle.get())->AddFun(L"SetAppState", &CChromeJsCallback::JsSetAppState);
	pObjApp->SetValue(L"SetAppState", myFun, V8_PROPERTY_ATTRIBUTE_NONE);

	myFun = CefV8Value::CreateFunction(L"OneClickInstall", myV8handle);
	static_cast<CCefV8Handler*>(myV8handle.get())->AddFun(L"OneClickInstall", &CChromeJsCallback::JsOneKeyInstall);
	pObjApp->SetValue(L"OneClickInstall", myFun, V8_PROPERTY_ATTRIBUTE_NONE);

	myFun = CefV8Value::CreateFunction(L"DownLoadFile", myV8handle);
	static_cast<CCefV8Handler*>(myV8handle.get())->AddFun(L"DownLoadFile", &CChromeJsCallback::JsDownloadFile);
	pObjApp->SetValue(L"DownLoadFile", myFun, V8_PROPERTY_ATTRIBUTE_NONE);*/
}


void SimpleApp::OnWebKitInitialized()
{
	// Define the extension contents.
	std::string extensionCode =
		"var test;"
		"if (!test)"
		"  test = {};"
		"(function() {"
		"  test.__defineSetter__('x', function(x) {"
		"    native function updateX();"
		"    updateX(x);"
		"  });"
		"})();";

	// Create an instance of my CefV8Handler object.
	CefRefPtr<CefV8Handler> handler = new CCefV8Handler();

	// Register the extension.
	CefRegisterExtension("v8/test", extensionCode, handler);
}


bool CCefV8Handler::Execute(const CefString& name,
							CefRefPtr<CefV8Value> object,
							const CefV8ValueList& arguments,
							CefRefPtr<CefV8Value>& retVal,
							CefString& exception)
{
	if (name == "updateX") {
		if (arguments.size() != 1 || !arguments[0]->IsDouble())
			return false;
		double x = arguments[0]->GetDoubleValue();
		return true;
	}
	return false;
}
