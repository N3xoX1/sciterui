#include <atomic> 
#include "sciter_edgewebview.h"
#include "aux-cvt.h"

namespace webview
{
    webview2_com_handler::webview2_com_handler(SciterEdgeWebView *webview, webview2_com_handler_cb_t cb)
        : m_webview(webview), m_cb(cb), m_handlerIID(GUID_NULL), m_titleHandler(nullptr), m_closeHandler(nullptr)
    {
        CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    }

    webview2_com_handler::~webview2_com_handler()
    {
        m_webview = nullptr;
        if (nullptr != m_titleHandler)
        {
            delete m_titleHandler;
        }
        if (nullptr != m_closeHandler)
        {
            delete m_closeHandler;
        }
        CoUninitialize();
    }

    void webview2_com_handler::setHandlerIID(REFIID riid)
    {
        m_handlerIID = riid;
    }

    ULONG webview2_com_handler::AddRef()
    {
        return 1;
    }

    ULONG webview2_com_handler::Release()
    {
        return 1;
    }

    STDMETHODIMP webview2_com_handler::QueryInterface(REFIID riid, LPVOID *ppv)
    {
        return S_OK;
    }

    STDMETHODIMP webview2_com_handler::Invoke(HRESULT res, ICoreWebView2Environment *env)
    {
        if (S_OK == res) {
            env->CreateCoreWebView2Controller(m_webview->m_hWnd, this);
        }
        return res;
    }

    STDMETHODIMP webview2_com_handler::Invoke(HRESULT res, ICoreWebView2Controller *controller)
    {
        ICoreWebView2 *webview = nullptr;
        if (S_OK == controller->get_CoreWebView2(&webview))
        {
            webview->add_WebMessageReceived(this, nullptr);
            webview->add_PermissionRequested(this, nullptr);
            webview->add_NavigationStarting(this, nullptr);
            webview->add_NavigationCompleted(this, nullptr);
            webview->add_NewWindowRequested(this, nullptr);

            CComPtr<ICoreWebView2_11> webview11;
            if (SUCCEEDED(webview->QueryInterface(IID_PPV_ARGS(&webview11))) && webview11)
            {
                webview11->add_ContextMenuRequested(this, nullptr);
            }

            m_titleHandler = new webview2_com_handler(m_webview, m_cb);
            m_titleHandler->setHandlerIID(IID_ICoreWebView2DocumentTitleChangedEventHandler);
            webview->add_DocumentTitleChanged(m_titleHandler, nullptr);

            m_closeHandler = new webview2_com_handler(m_webview, m_cb);
            m_closeHandler->setHandlerIID(IID_ICoreWebView2WindowCloseRequestedEventHandler);
            webview->add_WindowCloseRequested(m_closeHandler, nullptr);

            webview->Release();
        }
        m_cb(controller);
        return S_OK;
    }

    STDMETHODIMP webview2_com_handler::Invoke(ICoreWebView2 *sender, ICoreWebView2WebMessageReceivedEventArgs *args)
    {
        LPWSTR message = nullptr;
        if (S_OK == args->TryGetWebMessageAsString(&message))
        {
            aux::w2utf strMsg(message);
            SendMessage(m_webview->m_hWnd, WMU_JS_BRIDGE_CALL, 0, (LPARAM)strMsg.c_str());
            sender->PostWebMessageAsString(message);
            ::CoTaskMemFree(message);
        }
        return S_OK;
    }

    STDMETHODIMP webview2_com_handler::Invoke(ICoreWebView2 *sender, ICoreWebView2PermissionRequestedEventArgs *args)
    {
        COREWEBVIEW2_PERMISSION_KIND kind;
        args->get_PermissionKind(&kind);
        if (kind == COREWEBVIEW2_PERMISSION_KIND_CLIPBOARD_READ)
        {
            args->put_State(COREWEBVIEW2_PERMISSION_STATE_ALLOW);
        }
        return S_OK;
    }

    STDMETHODIMP webview2_com_handler::Invoke(ICoreWebView2 *sender, ICoreWebView2NavigationStartingEventArgs *args)
    {
        LPWSTR uri = nullptr;
        if (S_OK == args->get_Uri(&uri))
        {
            aux::w2utf strUrl(uri);
            SendMessage(m_webview->m_hWnd, WMU_NAVIGATION_CALL, DISPID_BEFORENAVIGATE2, (LPARAM)strUrl.c_str());
            ::CoTaskMemFree(uri);
        }
        return S_OK;
    }

    STDMETHODIMP webview2_com_handler::Invoke(ICoreWebView2 *sender, ICoreWebView2NavigationCompletedEventArgs *args)
    {
        BOOL success = FALSE;
        args->get_IsSuccess(&success);
        if (success)
        {
            SendMessage(m_webview->m_hWnd, WMU_NAVIGATION_CALL, DISPID_NAVIGATECOMPLETE2, 0);
        }
        else
        {
            SendMessage(m_webview->m_hWnd, WMU_NAVIGATION_CALL, DISPID_NAVIGATEERROR, 0);
        }
        return S_OK;
    }

    STDMETHODIMP webview2_com_handler::Invoke(ICoreWebView2 *sender, ICoreWebView2ContextMenuRequestedEventArgs *args)
    {
        const char* allow = m_webview->m_allowContextMenu.c_str();
        if (0 != stricmp("true", allow)) {
            args->put_Handled(TRUE);
        }
        return S_OK;
    }

    STDMETHODIMP webview2_com_handler::Invoke(ICoreWebView2 *sender, ICoreWebView2NewWindowRequestedEventArgs *args)
    {
        const char* allow = m_webview->m_allowWindowOpen.c_str();
        if (0 == stricmp("false", allow)) {
            args->put_Handled(true);
            return S_OK;
        }
        else if (0 == stricmp("popup", allow)) {
            ICoreWebView2Deferral* deferral = nullptr;
            args->GetDeferral(&deferral);

            CComPtr<ICoreWebView2WindowFeatures> windowFeatures;
            args->get_WindowFeatures(&windowFeatures);

            BOOL hasPos = false, hasSize = false;
            windowFeatures->get_HasPosition(&hasPos);
            windowFeatures->get_HasSize(&hasSize);
            UINT32 left = 0, top = 0, width = 800, height = 600;
            if (hasPos)
            {
                windowFeatures->get_Left(&left);
                windowFeatures->get_Top(&top);
            }
            if (hasSize)
            {
                windowFeatures->get_Width(&width);
                windowFeatures->get_Height(&height);
            }
            RECT rect = { left, top, width, height };

            SciterEdgeWebView* webView = new SciterEdgeWebView(m_webview->m_debugtools);
            webView->Create(NULL, &rect, 0, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
            webView->CenterWindow();
            webView->set_allowWindowOpen(m_webview->m_allowWindowOpen);
            webView->set_allowContextMenu(m_webview->m_allowContextMenu);
            webView->load_engine([webView, args, deferral](bool succeed) -> void
                {
                    args->put_NewWindow(webView->m_webview);
                    args->put_Handled(true);

                    deferral->Complete();
                    deferral->Release(); });
            return S_OK;
        }
        else {
            args->put_Handled(true);

            LPWSTR uri = nullptr;
            args->get_Uri(&uri);
            if (nullptr != uri) {
                m_webview->m_webview->Navigate(uri);
                ::CoTaskMemFree(uri);
            }
            return S_OK;
        }
    }

    STDMETHODIMP webview2_com_handler::Invoke(ICoreWebView2 *sender, IUnknown *args)
    {
        if (IsEqualIID(m_handlerIID, IID_ICoreWebView2DocumentTitleChangedEventHandler))
        {
            LPWSTR title = nullptr;
            if (S_OK == sender->get_DocumentTitle(&title))
            {
                aux::w2utf strTitle(title);
                SendMessage(m_webview->m_hWnd, WMU_NAVIGATION_CALL, DISPID_TITLECHANGE, (LPARAM)strTitle.c_str());
                ::CoTaskMemFree(title);
            }
        }
        else if (IsEqualIID(m_handlerIID, IID_ICoreWebView2WindowCloseRequestedEventHandler))
        {
            //The window opened by `window.open` can be closed
            if (nullptr == m_webview->m_navigationCallback) {
                m_webview->PostMessage(WM_CLOSE);
            }
        }
        return S_OK;
    }

    /*SciterEdgeWebView*/
    SciterEdgeWebView::SciterEdgeWebView(bool debug /*= false */)
    {
        m_debugtools = debug;
    }

    void SciterEdgeWebView::load_engine(const completion_fn_t &completion)
    {
        wchar_t currentExePath[MAX_PATH];
        GetModuleFileNameW(NULL, currentExePath, MAX_PATH);
        wchar_t *currentExeName = PathFindFileNameW(currentExePath);

        wchar_t dataPath[MAX_PATH];
        if (!SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, dataPath)))
        {
            completion(false);
            return;
        }
        wchar_t userDataFolder[MAX_PATH];
        PathCombineW(userDataFolder, dataPath, currentExeName);

        m_webview2_handler = new webview2_com_handler(
            this, [=](ICoreWebView2Controller *controller)
            {
                    RECT rc = { 0, 0, 0, 0 };
                    GetClientRect(&rc);
                    m_controller = controller;
                    m_controller->put_Bounds(rc);
                    m_controller->AddRef();
                    m_controller->get_CoreWebView2(&m_webview);

                    CComPtr<ICoreWebView2Settings> settings;
                    m_webview->get_Settings(&settings);
                    if (nullptr != settings) {
                        settings->put_AreDevToolsEnabled(m_debugtools);
                    }
                    init("{window.external={invoke:(s)=>{window.chrome.webview.postMessage(s)}}}");
                    completion(true); });
        if (S_OK != CreateCoreWebView2EnvironmentWithOptions(nullptr, userDataFolder, nullptr, m_webview2_handler))
        {
            delete m_webview2_handler;
            m_webview2_handler = nullptr;
            completion(false);
            return;
        }
    }

    void SciterEdgeWebView::navigate(const std::string &url)
    {
        aux::utf2w wurl(url.c_str());
        HRESULT hr = m_webview->Navigate(wurl.c_str());
        assert(SUCCEEDED(hr));
    }

    void SciterEdgeWebView::set_html(const std::string &html)
    {
        aux::utf2w whtml(html.c_str());
        m_webview->NavigateToString(whtml.c_str());
    }

    void SciterEdgeWebView::reload()
    {
        m_webview->Reload();
    }

    void SciterEdgeWebView::go_back()
    {
        m_webview->GoBack();
    }

    void SciterEdgeWebView::go_forward()
    {
        m_webview->GoForward();
    }

    void SciterEdgeWebView::stop()
    {
        m_webview->Stop();
    }

    void SciterEdgeWebView::advance_focus(bool forward) {
      m_controller->MoveFocus(forward ? COREWEBVIEW2_MOVE_FOCUS_REASON_NEXT: COREWEBVIEW2_MOVE_FOCUS_REASON_PREVIOUS);
    }
    

    void SciterEdgeWebView::init(const std::string &js)
    {
        aux::utf2w wjs(js);
        m_webview->AddScriptToExecuteOnDocumentCreated(wjs.c_str(), nullptr);
    }

    void SciterEdgeWebView::eval(const std::string &js)
    {
        aux::utf2w wjs(js);
        m_webview->ExecuteScript(wjs.c_str(), nullptr);
    }

    void SciterEdgeWebView::dispatch(std::function<void()> f)
    {
        PostMessage(WMU_DISPATCH_RUN, 0, (LPARAM) new dispatch_fn_t(f));
    }

    void SciterEdgeWebView::set_navigation_callback(const navigation_callback_t &cb)
    {
        m_navigationCallback = cb;
    }

    void SciterEdgeWebView::set_msg_callback(const msg_callback_t &cb)
    {
        m_msgCallback = cb;
    }

    void SciterEdgeWebView::set_allowWindowOpen(const std::string& val)
    {
        m_allowWindowOpen = val;
    }

    void SciterEdgeWebView::set_allowContextMenu(const std::string& val)
    {
        m_allowContextMenu = val;
    }

    std::string SciterEdgeWebView::currentSrc()
    {
        LPWSTR url = nullptr;
        m_webview->get_Source(&url);
        if (nullptr == url) {
            return "";
        }
        else {
            aux::w2utf strUrl(url);
            ::CoTaskMemFree(url);
            return std::string(strUrl.c_str());
        }
    }

    void SciterEdgeWebView::OnFinalMessage(_In_ HWND /*hWnd*/)
    {
        if (nullptr == m_navigationCallback)
        {
            delete this;
        }
    }

    LRESULT SciterEdgeWebView::OnCreate(UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM lParam,
                                        BOOL & /*bHandled*/)
    {
        //Check WebView2 Running
        static bool s_CheckRunning = false;
        if (!s_CheckRunning) {
            s_CheckRunning = true;
            wchar_t currentExePath[MAX_PATH];
            GetModuleFileNameW(NULL, currentExePath, MAX_PATH);
            wchar_t* currentExeName = PathFindFileNameW(currentExePath);

            wchar_t dataPath[MAX_PATH];
            if (!SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, dataPath))) {
                return -1;
            }
            wchar_t userDataFolder[MAX_PATH];
            PathCombineW(userDataFolder, dataPath, currentExeName);

            std::atomic_flag flag = ATOMIC_FLAG_INIT;
            flag.test_and_set();
            webview2_com_handler* checkHandler = new webview2_com_handler(
                this, [&](ICoreWebView2Controller* controller)
                {
                    controller->Close();
                    flag.clear();
                }
            );
            HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(nullptr, userDataFolder, nullptr, checkHandler);
            if (S_OK != hr) {
                delete checkHandler;
                return -1;
            }
            MSG msg = {};
            while (flag.test_and_set() && GetMessage(&msg, NULL, 0, 0))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            delete checkHandler;
        }

        ShowWindow(SW_SHOW);
        UpdateWindow();
        //SetFocus();
        return 0;
    }

    LRESULT SciterEdgeWebView::OnDestroy(UINT /*nMsg*/, WPARAM wParam, LPARAM /*lParam*/,
                                         BOOL & /*bHandled*/)
    {
        if (nullptr != m_controller)
        {
            m_controller->Release();
            m_controller = nullptr;
        }
        if (nullptr != m_webview)
        {
            m_webview->Release();
            m_webview = nullptr;
        }
        if (nullptr != m_webview2_handler)
        {
            delete m_webview2_handler;
            m_webview2_handler = nullptr;
        }
        return 1;
    }

    LRESULT SciterEdgeWebView::OnEraseBKG(UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/,
                                          BOOL &bHandled)
    {
        bHandled = TRUE;
        return 1;
    }

    LRESULT SciterEdgeWebView::OnSize(UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM lParam,
                                      BOOL & /*bHandled*/)
    {
        LONG width = LOWORD(lParam);
        LONG height = HIWORD(lParam);
        RECT rc = {0, 0, width, height};
        if (nullptr != m_controller)
        {
            m_controller->put_Bounds(rc);
        }
        return 1;
    }

    LRESULT SciterEdgeWebView::OnNavigationCall(UINT /*nMsg*/, WPARAM wParam, LPARAM lParam,
                                                BOOL &bHandled)
    {
        bHandled = TRUE;
        if (nullptr == m_navigationCallback)
        {
            if (DISPID_TITLECHANGE == wParam)
            {
                const std::string title = (const char*)lParam;
                std::wstring strTitle = aux::utf2w(title.c_str());
                SetWindowText(strTitle.c_str());
            }
            return 0;
        }
        if (DISPID_BEFORENAVIGATE2 == wParam)
        {
            const std::string url = (const char *)lParam;
            m_navigationCallback(navigation_event::will_navigate, url);
        }
        else if (DISPID_NAVIGATECOMPLETE2 == wParam)
        {
            m_navigationCallback(navigation_event::did_navigate, std::string());
        }
        else if (DISPID_NAVIGATEERROR == wParam)
        {
            m_navigationCallback(navigation_event::navigate_failure, std::string());
        }
        else if (DISPID_TITLECHANGE == wParam)
        {
            const std::string title = (const char *)lParam;
            m_navigationCallback(navigation_event::title_did_change, title);
        }
        return 0;
    }

    LRESULT SciterEdgeWebView::OnJsBridgeCall(UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM lParam,
                                              BOOL &bHandled)
    {
        bHandled = TRUE;
        const std::string str = (const char *)lParam;
        m_msgCallback(str);
        return 0;
    }

    LRESULT SciterEdgeWebView::OnDispatchRun(UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM lParam,
                                             BOOL &bHandled)
    {
        bHandled = TRUE;
        dispatch_fn_t *fn = (dispatch_fn_t *)(lParam);
        (*fn)();
        delete fn;
        return 0;
    }

} // webview
