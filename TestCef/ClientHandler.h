/************************************************************************************************
originally based on: Álan Crístoffer's "ClientHandler.h" file 
************************************************************************************************/

#ifndef __CEFSimpleSample__ClientHandler__
#define __CEFSimpleSample__ClientHandler__

#include "include/cef_client.h"

class ClientHandler : public CefClient, public CefLifeSpanHandler {

public:
    ClientHandler();

    CefRefPtr<CefBrowser> GetBrowser() { return m_Browser; }
    CefWindowHandle GetBrowserHwnd() { return m_BrowserHwnd; }
    
    // CefClient methods
    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE {
        return this;
    }

    // Virtual on CefLifeSpanHandler
    virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
    //virtual bool OnBeforePopup(CefRefPtr<CefBrowser> parentBrowser, const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo, const CefString& url, CefRefPtr<CefClient>& client, CefBrowserSettings& settings) OVERRIDE;
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
        
    // Virtual on CefV8ContextHandler
    //void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) OVERRIDE;
    //void OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) OVERRIDE;
    
    // Virtual on CefV8Handler
    //bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception) OVERRIDE;
private: 
    //std::string m_StartupURL; 

public: 
    //std::string GetStartupURL() { return m_StartupURL; }

protected:
    // The child browser window
    CefRefPtr<CefBrowser> m_Browser;
    
    // The child browser window handle
    CefWindowHandle m_BrowserHwnd;
    
    ///
    // Macro that provides a reference counting implementation for classes extending
    // CefBase.
    ///
    IMPLEMENT_REFCOUNTING(ClientHandler);
};

#endif /* defined(__CEFSimpleSample__ClientHandler__) */
