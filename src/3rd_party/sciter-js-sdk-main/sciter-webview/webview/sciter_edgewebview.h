#ifndef _sciter_edgewebview_h_
#define _sciter_edgewebview_h_

#define WIN32_LEAN_AND_MEAN
#include <string>
#include <functional>
#include <string>

#include <atlbase.h>
#include <atlwin.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <exdispid.h>
#include <windows.h>

#include "webview_callbacks.h"

#include "win/include/WebView2EnvironmentOptions.h"
#ifdef _M_ARM64
#pragma comment(lib, "../../sciter-webview/webview/win/arm64/WebView2LoaderStatic.lib")
#elif defined(_WIN64)
#pragma comment(lib, "../../sciter-webview/webview/win/x64/WebView2LoaderStatic.lib")
#else
#pragma comment(lib, "../../sciter-webview/webview/win/x86/WebView2LoaderStatic.lib")
#endif

#pragma comment(lib, "Shlwapi.lib")

namespace webview
{

#define WMU_NAVIGATION_CALL (WM_USER + 1)
#define WMU_JS_BRIDGE_CALL (WM_USER + 2)
#define WMU_DISPATCH_RUN (WM_USER + 3)

    class SciterEdgeWebView;
    class webview2_com_handler
        : public ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler,
          public ICoreWebView2CreateCoreWebView2ControllerCompletedHandler,
          public ICoreWebView2WebMessageReceivedEventHandler,
          public ICoreWebView2PermissionRequestedEventHandler,
          public ICoreWebView2NavigationStartingEventHandler,
          public ICoreWebView2NavigationCompletedEventHandler,
          public ICoreWebView2ContextMenuRequestedEventHandler,
          public ICoreWebView2DocumentTitleChangedEventHandler,
          public ICoreWebView2NewWindowRequestedEventHandler,
          public ICoreWebView2WindowCloseRequestedEventHandler
    {
        using webview2_com_handler_cb_t =
            std::function<void(ICoreWebView2Controller *)>;

    public:
        webview2_com_handler(SciterEdgeWebView* webview, webview2_com_handler_cb_t cb);
        virtual ~webview2_com_handler();

        void setHandlerIID(REFIID riid);

        STDMETHODIMP_(ULONG) AddRef();
        STDMETHODIMP_(ULONG) Release();
        STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppv);
        STDMETHODIMP Invoke(HRESULT res, ICoreWebView2Environment* env);
        STDMETHODIMP Invoke(HRESULT res, ICoreWebView2Controller* controller);
        STDMETHODIMP Invoke(ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args);
        STDMETHODIMP Invoke(ICoreWebView2* sender, ICoreWebView2PermissionRequestedEventArgs* args);
        STDMETHODIMP Invoke(ICoreWebView2* sender, ICoreWebView2NavigationStartingEventArgs* args);
        STDMETHODIMP Invoke(ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args);
        STDMETHODIMP Invoke(ICoreWebView2* sender, ICoreWebView2ContextMenuRequestedEventArgs* args);
        STDMETHODIMP Invoke(ICoreWebView2* sender, ICoreWebView2NewWindowRequestedEventArgs* args);
        STDMETHODIMP Invoke(ICoreWebView2* sender, IUnknown* args);

    private:
        SciterEdgeWebView* m_webview;
        webview2_com_handler_cb_t m_cb;

        IID m_handlerIID;
        webview2_com_handler* m_titleHandler;
        webview2_com_handler* m_closeHandler;
    };

    class SciterEdgeWebView : public CWindowImpl<SciterEdgeWebView>
    {
    public:
        SciterEdgeWebView(bool debug = false);

        // Optionally specify name of the new Windows class
        DECLARE_WND_CLASS(_T("SciterWebView"))

        BEGIN_MSG_MAP(SciterEdgeWebView)
            MESSAGE_HANDLER(WM_CREATE, OnCreate)
            MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
            MESSAGE_HANDLER(WM_SIZE, OnSize)
            MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBKG)
            MESSAGE_HANDLER(WMU_NAVIGATION_CALL, OnNavigationCall)
            MESSAGE_HANDLER(WMU_JS_BRIDGE_CALL, OnJsBridgeCall)
            MESSAGE_HANDLER(WMU_DISPATCH_RUN, OnDispatchRun)
        END_MSG_MAP()

        void load_engine(const completion_fn_t& completion);

        void navigate(const std::string& url);
        void set_html(const std::string& html);
        void reload();
        void go_back();
        void go_forward();
        void stop();
        void advance_focus(bool forward);
        void init(const std::string& js);
        void eval(const std::string& js);
        void dispatch(std::function<void()> f);
        void set_navigation_callback(const navigation_callback_t& cb);
        void set_msg_callback(const msg_callback_t& cb);
        void set_allowWindowOpen(const std::string& val);
        void set_allowContextMenu(const std::string& val);
        std::string currentSrc();

        ICoreWebView2 *m_webview = nullptr;
        ICoreWebView2Controller *m_controller = nullptr;
        navigation_callback_t m_navigationCallback;
        msg_callback_t m_msgCallback;
        std::string m_allowWindowOpen = "nopopup";
        std::string m_allowContextMenu = "true";
        bool m_debugtools = false;

    protected:
        virtual void OnFinalMessage(_In_ HWND /*hWnd*/);

        LRESULT OnCreate(UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
        LRESULT OnDestroy(UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
        LRESULT OnEraseBKG(UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
        LRESULT OnSize(UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);

        LRESULT OnNavigationCall(UINT /*nMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnJsBridgeCall(UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
        LRESULT OnDispatchRun(UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);

        webview2_com_handler *m_webview2_handler = nullptr;
    };

} // webview

#endif //_sciter_edgewebview_h_