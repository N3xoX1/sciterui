#ifndef _sciter_iewebview_h_
#define _sciter_iewebview_h_

#define WIN32_LEAN_AND_MEAN
#include <string>
#include <vector>
#include <functional>
#include <atlbase.h>
#include <atlwin.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <mshtml.h>
#include <mshtmdid.h>
#include <exdispid.h>

#include <comutil.h>
#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "Shlwapi.lib")

#include "webview_callbacks.h"

#ifdef WINDOWS
#ifndef THIS_HINSTANCE
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define THIS_HINSTANCE ((HINSTANCE)&__ImageBase)
#endif
#endif

#define WMU_NAVIGATION_CALL (WM_USER + 1)
#define WMU_JS_BRIDGE_CALL (WM_USER + 2)
#define WMU_DISPATCH_RUN (WM_USER + 3)
#define WMU_LOAD_ENGINE (WM_USER + 4)

namespace webview
{
    // ---------------------------------------------------------------------
    // RKEY, quick registry access
    // Code From https://www.codeproject.com/Articles/1040618/Happy-Registry-A-Quick-Wrapper-for-the-Win32-Datab
    class RKEY
    {
    private:
        HKEY k = 0;

    public:
        class VALUE
        {
        public:
            std::wstring name;
            std::vector<char> value; // For enums
            HKEY k = 0;
            mutable DWORD ty = 0;

            VALUE(const wchar_t *s, HKEY kk)
            {
                if (s)
                    name = s;
                k = kk;
            }

            bool operator=(const wchar_t *val)
            {
                ty = REG_SZ;
                return RegSetValueEx(k, name.c_str(), 0, REG_SZ,
                                     (BYTE *)val, (DWORD)(wcslen(val) * sizeof(wchar_t))) == ERROR_SUCCESS;
            }
            bool operator=(unsigned long val)
            {
                ty = REG_DWORD;
                return RegSetValueEx(k, name.c_str(), 0, REG_DWORD,
                                     (BYTE *)&val, sizeof(val)) == ERROR_SUCCESS;
            }
            bool operator=(unsigned long long val)
            {
                ty = REG_QWORD;
                return RegSetValueEx(k, name.c_str(), 0, REG_QWORD,
                                     (BYTE *)&val, sizeof(val)) == ERROR_SUCCESS;
            }

            template <typename T>
            operator T() const
            {
                T ch = 0;
                RegQueryValueEx(k, name.c_str(), 0, &ty, 0, &ch);
                std::vector<char> d(ch + 10);
                ch += 10;
                RegQueryValueEx(k, name.c_str(), 0, &ty, (LPBYTE)d.data(), &ch);
                T ret = 0;
                memcpy(&ret, d.data(), sizeof(T));
                return ret;
            }

            operator std::wstring() const
            {
                DWORD ch = 0;
                RegQueryValueEx(k, name.c_str(), 0, &ty, 0, &ch);
                std::vector<char> d(ch + 10);
                ch += 10;
                RegQueryValueEx(k, name.c_str(), 0, &ty, (LPBYTE)d.data(), &ch);
                return std::wstring((const wchar_t *)d.data());
            }

            bool Delete()
            {
                return (RegDeleteValue(k, name.c_str()) == ERROR_SUCCESS);
            }
        };

        RKEY()
        {
            k = 0;
        }

        RKEY(HKEY kk)
        {
            k = kk;
        }

        RKEY(const RKEY &k)
        {
            operator=(k);
        }
        void operator=(const RKEY &r)
        {
            Close();
            DuplicateHandle(GetCurrentProcess(), r.k, GetCurrentProcess(),
                            (LPHANDLE)&k, 0, false, DUPLICATE_SAME_ACCESS);
        }

        RKEY(RKEY &&k)
        {
            operator=(std::forward<RKEY>(k));
        }
        void operator=(RKEY &&r)
        {
            Close();
            k = r.k;
            r.k = 0;
        }

        void operator=(HKEY kk)
        {
            Close();
            k = kk;
        }

        RKEY(HKEY root, const wchar_t *subkey, DWORD acc = KEY_ALL_ACCESS)
        {
            Load(root, subkey, acc);
        }
        bool Load(HKEY root, const wchar_t *subkey, DWORD acc = KEY_ALL_ACCESS)
        {
            Close();
            return (RegCreateKeyEx(root, subkey, 0, 0, 0, acc, 0, &k, 0) == ERROR_SUCCESS);
        }
        bool Open(HKEY root, const wchar_t *subkey, DWORD acc = KEY_ALL_ACCESS)
        {
            Close();
            return (RegOpenKeyEx(root, subkey, 0, acc, &k) == ERROR_SUCCESS);
        }

        void Close()
        {
            if (k)
                RegCloseKey(k);
            k = 0;
        }

        ~RKEY()
        {
            Close();
        }

        bool Valid() const
        {
            if (k)
                return true;
            return false;
        }

        bool DeleteSingle(const wchar_t *sub)
        {
            return (RegDeleteKey(k, sub) == ERROR_SUCCESS);
        }

        bool Delete(const wchar_t *sub = 0)
        {
            return (RegDeleteTree(k, sub) == ERROR_SUCCESS);
        }

        bool Flush()
        {
            return (RegFlushKey(k) == ERROR_SUCCESS);
        }

        std::vector<std::wstring> EnumSubkeys() const
        {
            std::vector<std::wstring> data;
            for (int i = 0;; i++)
            {
                std::vector<wchar_t> n(300);
                DWORD sz = (DWORD)n.size();
                if (RegEnumKeyEx(k, i, n.data(), &sz, 0, 0, 0, 0) != ERROR_SUCCESS)
                    break;
                data.push_back(n.data());
            }
            return data;
        }

        std::vector<VALUE> EnumValues() const
        {
            std::vector<VALUE> data;
            for (int i = 0;; i++)
            {
                std::vector<wchar_t> n(300);
                DWORD sz = (DWORD)n.size();
                DWORD ay = 0;
                RegEnumValue(k, i, n.data(), &sz, 0, 0, 0, &ay);
                std::vector<char> v(ay);
                DWORD ty = 0;
                sz = (DWORD)n.size();
                if (RegEnumValue(k, i, n.data(), &sz, 0, &ty, (LPBYTE)v.data(), &ay) != ERROR_SUCCESS)
                    break;

                VALUE x(n.data(), k);
                x.ty = ty;
                x.value = v;
                data.push_back(x);
            }
            return data;
        }

        VALUE operator[](const wchar_t *v) const
        {
            VALUE kv(v, k);
            return kv;
        }

        operator HKEY()
        {
            return k;
        }
    };

    class SciterIEWebView;
    class webbrowser2_com_handler : public IDocHostUIHandler, public IDispatch
    {
    public:
        webbrowser2_com_handler(SciterIEWebView* webview);
        virtual ~webbrowser2_com_handler();

        void setInitJS(const std::string &js);
        void eval(const std::string &js);

        STDMETHODIMP_(ULONG) AddRef();
        STDMETHODIMP_(ULONG) Release();
        STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppv);

        // IDocHostUIHandler Method
        STDMETHODIMP ShowContextMenu(
            /* [in] */ DWORD dwID,
            /* [in] */ POINT *ppt,
            /* [in] */ IUnknown *pcmdtReserved,
            /* [in] */ IDispatch *pdispReserved);

        STDMETHODIMP GetHostInfo(
            /* [out][in] */ DOCHOSTUIINFO *pInfo);

        STDMETHODIMP ShowUI(
            /* [in] */ DWORD dwID,
            /* [in] */ IOleInPlaceActiveObject *pActiveObject,
            /* [in] */ IOleCommandTarget *pCommandTarget,
            /* [in] */ IOleInPlaceFrame *pFrame,
            /* [in] */ IOleInPlaceUIWindow *pDoc);

        STDMETHODIMP HideUI(void);

        STDMETHODIMP UpdateUI(void);

        STDMETHODIMP EnableModeless(
            /* [in] */ BOOL fEnable);

        STDMETHODIMP OnDocWindowActivate(
            /* [in] */ BOOL fActivate);

        STDMETHODIMP OnFrameWindowActivate(
            /* [in] */ BOOL fActivate);

        STDMETHODIMP ResizeBorder(
            /* [in] */ LPCRECT prcBorder,
            /* [in] */ IOleInPlaceUIWindow *pUIWindow,
            /* [in] */ BOOL fRameWindow);

        STDMETHODIMP TranslateAccelerator(
            /* [in] */ LPMSG lpMsg,
            /* [in] */ const GUID *pguidCmdGroup,
            /* [in] */ DWORD nCmdID);

        STDMETHODIMP GetOptionKeyPath(
            /* [out] */ LPOLESTR *pchKey,
            /* [in] */ DWORD dw);

        STDMETHODIMP GetDropTarget(
            /* [in] */ IDropTarget *pDropTarget,
            /* [out] */ IDropTarget **ppDropTarget);

        STDMETHODIMP GetExternal(
            /* [out] */ IDispatch **ppDispatch);

        STDMETHODIMP TranslateUrl(
            /* [in] */ DWORD dwTranslate,
            /* [in] */ OLECHAR *pchURLIn,
            /* [out] */ OLECHAR **ppchURLOut);

        STDMETHODIMP FilterDataObject(
            /* [in] */ IDataObject *pDO,
            /* [out] */ IDataObject **ppDORet);

        // Implement IDispatch Functions
        STDMETHODIMP GetTypeInfoCount(UINT *pctinfo);

        STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid,
                                 ITypeInfo **ppTInfo);

        STDMETHODIMP GetIDsOfNames(REFIID riid,
                                   LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId);

        STDMETHODIMP Invoke(DISPID dispIdMember,
                            REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *Params,
                            VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);

        void injectExternal();

    private:
        std::vector<std::string> m_injectedJS;

        SciterIEWebView* m_webview;
        CComPtr<IDocHostUIHandler> m_spDefaultDocHostUIHandler;
    };

    class SciterIEWebView : public CWindowImpl<SciterIEWebView>
    {
    public:
        SciterIEWebView(bool debug = false);

        // Optionally specify name of the new Windows class
        DECLARE_WND_CLASS(_T("SciterIEWebView"))

        BEGIN_MSG_MAP(SciterIEWebView)
            MESSAGE_HANDLER(WM_CREATE, OnCreate)
            MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
            MESSAGE_HANDLER(WM_SIZE, OnSize)
            MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBKG)
            MESSAGE_HANDLER(WMU_NAVIGATION_CALL, OnNavigationCall)
            MESSAGE_HANDLER(WMU_JS_BRIDGE_CALL, OnJsBridgeCall)
            MESSAGE_HANDLER(WMU_DISPATCH_RUN, OnDispatchRun)
            MESSAGE_HANDLER(WMU_LOAD_ENGINE, OnLoadEngine)
        END_MSG_MAP()

        void load_engine(const completion_fn_t& completion);

        void navigate(const std::string& url);
        void set_html(const std::string& html);
        void reload();
        void go_back();
        void go_forward();
        void stop();
        void advance_focus(bool forward) {}
        void init(const std::string& js);
        void eval(const std::string& js);
        void dispatch(std::function<void()> f);
        void set_navigation_callback(const navigation_callback_t& cb);
        void set_msg_callback(const msg_callback_t& cb);
        void set_allowWindowOpen(const std::string& val);
        void set_allowContextMenu(const std::string& val);
        std::string currentSrc();

        CComPtr<IWebBrowser2> m_webbrowser2;
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
        LRESULT OnLoadEngine(UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);

        CAxWindow m_browser;
        webbrowser2_com_handler *m_handler = nullptr;
        DWORD m_dwCookie;
    };

} // webview

#endif //_sciter_iewebview_h_