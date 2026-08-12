#include "sciter_iewebview.h"
#include "aux-cvt.h"

CComModule _Module;

namespace webview
{
    webbrowser2_com_handler::webbrowser2_com_handler(SciterIEWebView *webview)
        : m_webview(webview)
    {
    }

    webbrowser2_com_handler::~webbrowser2_com_handler()
    {
    }

    void webbrowser2_com_handler::setInitJS(const std::string &js)
    {
        m_injectedJS.push_back(js);
    }

    void webbrowser2_com_handler::eval(const std::string &js)
    {
        std::wstring strJS(aux::utf2w(aux::chars_of(js)).as_string());
        CComPtr<IDispatch> pDispatch, pSrcDispatch;
        m_webview->m_webbrowser2->get_Document(&pDispatch);
        if (nullptr == pDispatch)
        {
            return;
        }
        CComPtr<IHTMLDocument2> pDoc2;
        pDispatch->QueryInterface(IID_IHTMLDocument2, (void **)&pDoc2);
        if (nullptr == pDoc2)
        {
            return;
        }
        pDoc2->get_Script(&pSrcDispatch);

        CComBSTR bstrEval("eval");
        DISPID dispid = 0;
        BSTR props[1] = {bstrEval};
        if (S_OK != pSrcDispatch->GetIDsOfNames(IID_NULL, props, 1, LOCALE_SYSTEM_DEFAULT, &dispid))
        {
            return;
        }
        DISPPARAMS params;
        _variant_t arg(js.c_str());
        _variant_t result;
        EXCEPINFO excepInfo;
        UINT nArgErr = (UINT)-1;
        params.cArgs = 1;
        params.cNamedArgs = 0;
        params.rgvarg = &arg;
        pSrcDispatch->Invoke(dispid, IID_NULL, 0, DISPATCH_METHOD, &params, &result, &excepInfo, &nArgErr);
    }

    ULONG webbrowser2_com_handler::AddRef()
    {
        return 1;
    }

    ULONG webbrowser2_com_handler::Release()
    {
        return 1;
    }

    STDMETHODIMP webbrowser2_com_handler::QueryInterface(REFIID riid, LPVOID *ppv)
    {
        if ((riid == IID_IDispatch) || (riid == IID_IUnknown))
        {
            (*ppv) = static_cast<IDispatch *>(this);
            return S_OK;
        }
        else if (riid == IID_IDocHostUIHandler)
        {
            (*ppv) = static_cast<IDocHostUIHandler *>(this);
            return S_OK;
        }
        else
        {
            return E_NOINTERFACE;
        }
    }

    // IDocHostUIHandler Method
    STDMETHODIMP webbrowser2_com_handler::ShowContextMenu(
        /* [in] */ DWORD dwID,
        /* [in] */ POINT *ppt,
        /* [in] */ IUnknown *pcmdtReserved,
        /* [in] */ IDispatch *pdispReserved)
    {
        const char* allow = m_webview->m_allowContextMenu.c_str();
            return (0 == stricmp("true", allow))
                ? m_spDefaultDocHostUIHandler->ShowContextMenu(dwID, ppt, pcmdtReserved, pdispReserved)
                : S_OK;
    }

    STDMETHODIMP webbrowser2_com_handler::GetHostInfo(
        /* [out][in] */ DOCHOSTUIINFO *pInfo)
    {
        m_spDefaultDocHostUIHandler->GetHostInfo(pInfo);
        pInfo->dwFlags = pInfo->dwFlags | DOCHOSTUIFLAG_NO3DBORDER | DOCHOSTUIFLAG_DPI_AWARE;
        return S_OK;
    }

    STDMETHODIMP webbrowser2_com_handler::ShowUI(
        /* [in] */ DWORD dwID,
        /* [in] */ IOleInPlaceActiveObject *pActiveObject,
        /* [in] */ IOleCommandTarget *pCommandTarget,
        /* [in] */ IOleInPlaceFrame *pFrame,
        /* [in] */ IOleInPlaceUIWindow *pDoc)
    {
        return m_spDefaultDocHostUIHandler->ShowUI(dwID, pActiveObject, pCommandTarget, pFrame, pDoc);
    }

    STDMETHODIMP webbrowser2_com_handler::HideUI(void) { return m_spDefaultDocHostUIHandler->HideUI(); }

    STDMETHODIMP webbrowser2_com_handler::UpdateUI(void) { return m_spDefaultDocHostUIHandler->UpdateUI(); }

    STDMETHODIMP webbrowser2_com_handler::EnableModeless(
        /* [in] */ BOOL fEnable)
    {
        return m_spDefaultDocHostUIHandler->EnableModeless(fEnable);
    }

    STDMETHODIMP webbrowser2_com_handler::OnDocWindowActivate(
        /* [in] */ BOOL fActivate)
    {
        return m_spDefaultDocHostUIHandler->OnDocWindowActivate(fActivate);
    }

    STDMETHODIMP webbrowser2_com_handler::OnFrameWindowActivate(
        /* [in] */ BOOL fActivate)
    {
        return m_spDefaultDocHostUIHandler->OnFrameWindowActivate(fActivate);
        ;
    }

    STDMETHODIMP webbrowser2_com_handler::ResizeBorder(
        /* [in] */ LPCRECT prcBorder,
        /* [in] */ IOleInPlaceUIWindow *pUIWindow,
        /* [in] */ BOOL fRameWindow)
    {
        return m_spDefaultDocHostUIHandler->ResizeBorder(prcBorder, pUIWindow, fRameWindow);
    }

    STDMETHODIMP webbrowser2_com_handler::TranslateAccelerator(
        /* [in] */ LPMSG lpMsg,
        /* [in] */ const GUID *pguidCmdGroup,
        /* [in] */ DWORD nCmdID)
    {
        return m_spDefaultDocHostUIHandler->TranslateAccelerator(lpMsg, pguidCmdGroup, nCmdID);
    }

    STDMETHODIMP webbrowser2_com_handler::GetOptionKeyPath(
        /* [out] */ LPOLESTR *pchKey,
        /* [in] */ DWORD dw)
    {
        return m_spDefaultDocHostUIHandler->GetOptionKeyPath(pchKey, dw);
    }

    STDMETHODIMP webbrowser2_com_handler::GetDropTarget(
        /* [in] */ IDropTarget *pDropTarget,
        /* [out] */ IDropTarget **ppDropTarget)
    {
        return m_spDefaultDocHostUIHandler->GetDropTarget(pDropTarget, ppDropTarget);
    }

    STDMETHODIMP webbrowser2_com_handler::GetExternal(
        /* [out] */ IDispatch **ppDispatch)
    {
        *ppDispatch = this;
        return S_OK;
    }

    STDMETHODIMP webbrowser2_com_handler::TranslateUrl(
        /* [in] */ DWORD dwTranslate,
        /* [in] */ OLECHAR *pchURLIn,
        /* [out] */ OLECHAR **ppchURLOut)
    {
        return m_spDefaultDocHostUIHandler->TranslateUrl(dwTranslate, pchURLIn, ppchURLOut);
    }

    STDMETHODIMP webbrowser2_com_handler::FilterDataObject(
        /* [in] */ IDataObject *pDO,
        /* [out] */ IDataObject **ppDORet)
    {
        return m_spDefaultDocHostUIHandler->FilterDataObject(pDO, ppDORet);
    }

    // Implement IDispatch Functions
    STDMETHODIMP webbrowser2_com_handler::GetTypeInfoCount(UINT *pctinfo)
    {
        *pctinfo = 0;
        return S_OK;
    }

    STDMETHODIMP webbrowser2_com_handler::GetTypeInfo(UINT iTInfo, LCID lcid,
                                                      ITypeInfo **ppTInfo)
    {
        return E_FAIL;
    }

    STDMETHODIMP webbrowser2_com_handler::GetIDsOfNames(REFIID riid,
                                                        LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId)
    {
        if (cNames == 0 || rgszNames == NULL || rgszNames[0] == NULL || rgDispId == NULL)
        {
            return E_INVALIDARG;
        }
        if (lstrcmp(rgszNames[0], L"invoke") == 0)
        {
            *rgDispId = 12300;
            return S_OK;
        }
        return E_INVALIDARG;
    }

    STDMETHODIMP webbrowser2_com_handler::Invoke(DISPID dispIdMember,
                                                 REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *Params,
                                                 VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
    {
        std::string strParam;
        if (DISPID_BEFORENAVIGATE2 == dispIdMember)
        {
            if (Params->cArgs >= 6)
            {
                VARIANT val = *Params->rgvarg[5].pvarVal;
                if (NULL != val.bstrVal)
                {
                    strParam = aux::w2utf(val.bstrVal);
                }
            }
            SendMessage(m_webview->m_hWnd, WMU_NAVIGATION_CALL, DISPID_BEFORENAVIGATE2, (LPARAM)strParam.c_str());
        }
        else if (DISPID_NAVIGATECOMPLETE2 == dispIdMember)
        {
            SendMessage(m_webview->m_hWnd, WMU_NAVIGATION_CALL, DISPID_NAVIGATECOMPLETE2, 0);
        }
        else if (DISPID_NAVIGATEERROR == dispIdMember)
        {
            SendMessage(m_webview->m_hWnd, WMU_NAVIGATION_CALL, DISPID_NAVIGATEERROR, 0);
        }
        else if (DISPID_TITLECHANGE == dispIdMember)
        {
            if (1 == Params->cArgs)
            {
                VARIANT val = Params->rgvarg[0];
                if (NULL != val.bstrVal)
                {
                    strParam = aux::w2utf(val.bstrVal);
                }
                SendMessage(m_webview->m_hWnd, WMU_NAVIGATION_CALL, DISPID_TITLECHANGE, (LPARAM)strParam.c_str());
            }
        }
        else if (DISPID_NEWWINDOW3 == dispIdMember)
        {
            if (5 == Params->cArgs)
            {
                VARIANT valUrl = Params->rgvarg[0];
                strParam = aux::w2utf(valUrl.bstrVal);

                const char* allow = m_webview->m_allowWindowOpen.c_str();
                if (0 == stricmp("false", allow)) {
                    *(Params->rgvarg[3].pboolVal) = VARIANT_TRUE;
                    *(Params->rgvarg[4].ppdispVal) = nullptr;
                }
                else if (0 == stricmp("popup", allow)) {
                    RECT rc = { 0, 0, 800, 600 };
                    SciterIEWebView* webView = new SciterIEWebView(m_webview->m_debugtools);
                    webView->Create(NULL, &rc, 0, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
                    webView->CenterWindow();
                    webView->set_allowWindowOpen(m_webview->m_allowWindowOpen);
                    webView->set_allowContextMenu(m_webview->m_allowContextMenu);
                    webView->load_engine([=](bool succeed) -> void
                        { webView->navigate(strParam); });
                    *(Params->rgvarg[3].pboolVal) = VARIANT_FALSE;
                    *(Params->rgvarg[4].ppdispVal) = webView->m_webbrowser2;
                }
                else {
                    *(Params->rgvarg[3].pboolVal) = VARIANT_TRUE;
                    *(Params->rgvarg[4].ppdispVal) = nullptr;

                    _variant_t strUrl = valUrl;
                    _variant_t flags(navNoHistory);
                    m_webview->m_webbrowser2->Navigate2(&strUrl, &flags, nullptr, nullptr, nullptr);
                }
                return S_OK;
            }
        }
        else if (DISPID_WINDOWCLOSING == dispIdMember)
        {
            if (2 == Params->cArgs)
            {
                //The window opened by `window.open` can be closed
                *(Params->rgvarg[0].pboolVal) = VARIANT_TRUE;
                if (nullptr == m_webview->m_navigationCallback) {
                    m_webview->PostMessage(WM_CLOSE);
                }
            }
        }
        else if (DISPID_BEFORESCRIPTEXECUTE == dispIdMember)
        {
            injectExternal();
        }
        else if (12300 == dispIdMember)
        {
            if (Params->cArgs == 1)
            {
                VARIANT val = Params->rgvarg[0];
                if (NULL != val.bstrVal)
                {
                    strParam = aux::w2utf(val.bstrVal);
                }
                SendMessage(m_webview->m_hWnd, WMU_JS_BRIDGE_CALL, 0, (LPARAM)strParam.c_str());
            }
        }
        return S_OK;
    }

    void webbrowser2_com_handler::injectExternal()
    {
        CComPtr<IDispatch> pDispatch;
        m_webview->m_webbrowser2->get_Document(&pDispatch);
        if (nullptr == pDispatch)
        {
            return;
        }
        CComPtr<IHTMLDocument2> pDoc2;
        pDispatch->QueryInterface(IID_IHTMLDocument2, (void **)&pDoc2);
        if (nullptr == pDoc2)
        {
            return;
        }
        // Request default handler from MSHTML client site
        CComPtr<IOleObject> spOleObject;
        if (SUCCEEDED(pDoc2.QueryInterface(&spOleObject)))
        {
            CComPtr<IOleClientSite> spClientSite;
            HRESULT hr = spOleObject->GetClientSite(&spClientSite);
            if (SUCCEEDED(hr) && spClientSite)
            {
                // Save pointer for delegation to default
                m_spDefaultDocHostUIHandler = spClientSite;
            }
        }
        CComPtr<ICustomDoc> pCustomDoc;
        pDoc2->QueryInterface(IID_ICustomDoc, (void **)&pCustomDoc);
        if (nullptr != pCustomDoc)
        {
            pCustomDoc->SetUIHandler(this);
        }

        CComBSTR tagName;
        CComPtr<IHTMLElement> pInsertElem, pElement;
        CComPtr<IHTMLElementCollection> pCollection;
        pDoc2->get_all(&pCollection);
        LONG num = 0;
        pCollection->get_length(&num);
        CComPtr<IDispatch> pDisp;
        for (int i = 0; i < num; i++)
        {
            _variant_t idx = i;
            pCollection->item(idx, idx, &pDisp);
            pElement = nullptr;
            pDisp.QueryInterface(&pElement);
            pDisp = nullptr;
            pElement->get_tagName(&tagName);
            if (!_wcsicmp(L"head", tagName))
            {
                pInsertElem = pElement;
                break;
            }
            else if (!_wcsicmp(L"body", tagName))
            {
                pInsertElem = pElement;
                break;
            }
        }

        CComBSTR pos = "beforeEnd", jsHTML;
        for (std::vector<std::string>::iterator iter = m_injectedJS.begin(), iterEnd = m_injectedJS.end();
             iter != iterEnd; iter++)
        {
            eval(*iter);
            continue;
            _bstr_t strJS = iter->c_str();
            pElement = nullptr;
            _bstr_t bstrScript = "script", bstrType = "text/javascript";
            pDoc2->createElement(bstrScript, &pElement);
            CComPtr<IHTMLScriptElement> pScriptElem;
            pElement->QueryInterface(IID_IHTMLScriptElement, (void **)&pScriptElem);
            pScriptElem->put_type(bstrType);
            pScriptElem->put_text(strJS);
            pElement->get_outerHTML(&jsHTML);
            pInsertElem->insertAdjacentHTML(pos, jsHTML);
        }
    }

    /*SciterIEWebView*/
    SciterIEWebView::SciterIEWebView(bool debug /*= false */)
    {
        m_debugtools = debug;
    }

    void SciterIEWebView::load_engine(const completion_fn_t &completion)
    {
        // Browser Emulation
        std::vector<wchar_t> fn(1000);
        GetModuleFileName(0, fn.data(), 1000);
        PathStripPath(fn.data());
        RKEY k(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_BROWSER_EMULATION");
        k[fn.data()] = 11000UL; // Use IE 11

        m_browser.Create(m_hWnd, 0, 0, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPCHILDREN);
        m_browser.CreateControl(OLESTR("shell.Explorer.2"));
        m_browser.QueryControl(&m_webbrowser2);
        m_browser.ShowWindow(SW_SHOW);
        m_handler = new webbrowser2_com_handler(this);

        CComPtr<IConnectionPointContainer> spConnectionPointContainer;
        CComPtr<IConnectionPoint> spConnectionPointBrowserEvents;
        m_webbrowser2->QueryInterface(IID_IConnectionPointContainer, (void **)&spConnectionPointContainer);
        spConnectionPointContainer->FindConnectionPoint(DIID_DWebBrowserEvents2, &spConnectionPointBrowserEvents);
        CComPtr<IUnknown> pUnk;
        m_handler->QueryInterface(IID_IUnknown, (void **)&pUnk);
        spConnectionPointBrowserEvents->Advise(pUnk, &m_dwCookie);

        PostMessage(WMU_LOAD_ENGINE, 0, (LPARAM) new completion_fn_t(completion));
    }

    void SciterIEWebView::navigate(const std::string &url)
    {
        _variant_t strUrl = url.c_str();
        _variant_t flags(navNoHistory);
        m_webbrowser2->Navigate2(&strUrl, &flags, nullptr, nullptr, nullptr);
    }

    void SciterIEWebView::set_html(const std::string &html)
    {
        CComPtr<IDispatch> pDisp;
        m_webbrowser2->get_Document(&pDisp);
        if (nullptr == pDisp)
        {
            return;
        }
        aux::utf2w strHtml(html);
        int size = strHtml.length() + 1;
        HGLOBAL hHTMLText = GlobalAlloc(GPTR, size * sizeof(wchar_t));
        if (nullptr == hHTMLText)
        {
            return;
        }
        wcscpy_s((wchar_t *)hHTMLText, size, strHtml.c_str());
        CComPtr<IStream> pStream;
        if (SUCCEEDED(CreateStreamOnHGlobal(hHTMLText, TRUE, &pStream)))
        {
            CComPtr<IPersistStreamInit> pPersistStreamInit;
            if (SUCCEEDED(pDisp.QueryInterface(&pPersistStreamInit)))
            {
                if (SUCCEEDED(pPersistStreamInit->InitNew()))
                {
                    pPersistStreamInit->Load(pStream);
                }
            }
        }
        else
        {
            GlobalFree(hHTMLText);
        }
    }

    void SciterIEWebView::reload()
    {
        VARIANT flags;
        flags.vt = VT_I4;
        flags.intVal = REFRESH_COMPLETELY;
        m_webbrowser2->Refresh2(&flags);
    }

    void SciterIEWebView::go_back()
    {
        m_webbrowser2->GoBack();
    }

    void SciterIEWebView::go_forward()
    {
        m_webbrowser2->GoForward();
    }

    void SciterIEWebView::stop()
    {
        m_webbrowser2->Stop();
    }

    void SciterIEWebView::init(const std::string &js)
    {
        m_handler->setInitJS(js);
    }

    void SciterIEWebView::eval(const std::string &js)
    {
        m_handler->eval(js);
    }

    void SciterIEWebView::dispatch(std::function<void()> f)
    {
        PostMessage(WMU_DISPATCH_RUN, 0, (LPARAM) new dispatch_fn_t(f));
    }

    void SciterIEWebView::set_navigation_callback(const navigation_callback_t &cb)
    {
        m_navigationCallback = cb;
    }

    void SciterIEWebView::set_msg_callback(const msg_callback_t &cb)
    {
        m_msgCallback = cb;
    }

    void SciterIEWebView::set_allowWindowOpen(const std::string& val)
    {
        m_allowWindowOpen = val;
    }

    void SciterIEWebView::set_allowContextMenu(const std::string& val)
    {
        m_allowContextMenu = val;
    }

    std::string SciterIEWebView::currentSrc()
    {
        BSTR url = nullptr;
        m_webbrowser2->get_LocationURL(&url);
        if (nullptr == url) {
            return "";
        }
        else {
            aux::w2utf strUrl(url);
            SysFreeString(url);
            return std::string(strUrl.c_str());
        }
    }

    void SciterIEWebView::OnFinalMessage(_In_ HWND /*hWnd*/)
    {
        if (nullptr == m_navigationCallback)
        {
            delete this;
        }
    }

    LRESULT SciterIEWebView::OnCreate(UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/,
                                      BOOL & /*bHandled*/)
    {
        ShowWindow(SW_SHOW);
        UpdateWindow();
        SetFocus();

        return 0;
    }

    LRESULT SciterIEWebView::OnDestroy(UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/,
                                       BOOL & /*bHandled*/)
    {
        {
            CComPtr<IConnectionPointContainer> spConnectionPointContainer;
            CComPtr<IConnectionPoint> spConnectionPointBrowserEvents;
            m_webbrowser2->QueryInterface(IID_IConnectionPointContainer, (void **)&spConnectionPointContainer);
            spConnectionPointContainer->FindConnectionPoint(DIID_DWebBrowserEvents2, &spConnectionPointBrowserEvents);
            spConnectionPointBrowserEvents->Unadvise(m_dwCookie);
        }
        m_webbrowser2 = nullptr;
        m_browser.DestroyWindow();
        delete m_handler;
        return 1;
    }

    LRESULT SciterIEWebView::OnEraseBKG(UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/,
                                        BOOL &bHandled)
    {
        bHandled = TRUE;
        return 1;
    }

    LRESULT SciterIEWebView::OnSize(UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM lParam,
                                    BOOL & /*bHandled*/)
    {
        if (m_browser.IsWindow())
        {
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);
            m_browser.ResizeClient(width, height);
        }
        return 1;
    }

    LRESULT SciterIEWebView::OnNavigationCall(UINT /*nMsg*/, WPARAM wParam, LPARAM lParam,
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

    LRESULT SciterIEWebView::OnJsBridgeCall(UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM lParam,
                                            BOOL &bHandled)
    {
        bHandled = TRUE;
        const std::string str = (const char *)lParam;
        m_msgCallback(str);
        return 0;
    }

    LRESULT SciterIEWebView::OnDispatchRun(UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM lParam,
                                           BOOL &bHandled)
    {
        bHandled = TRUE;
        dispatch_fn_t *fn = (dispatch_fn_t *)(lParam);
        (*fn)();
        delete fn;
        return 0;
    }

    LRESULT SciterIEWebView::OnLoadEngine(UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM lParam,
                                          BOOL &bHandled)
    {
        RECT rc = {0, 0, 0, 0};
        GetClientRect(&rc);
        m_browser.MoveWindow(&rc);

        const completion_fn_t *completion = (completion_fn_t *)lParam;
        (*completion)(true);
        delete completion;

        return 0;
    }

} // webview