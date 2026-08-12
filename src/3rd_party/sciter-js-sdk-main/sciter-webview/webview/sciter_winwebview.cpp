#define WIN32_LEAN_AND_MEAN

#include "sciter_winwebview.h"
#include "sciter_edgewebview.h"
#include "sciter_iewebview.h"
#include <versionhelpers.h>

namespace webview {

    sciter_winwebview::sciter_winwebview(bool debug /*= false*/, void* parent /*= nullptr*/)
    {
        m_isEdge = IsWindows10OrGreater();
        m_winbrowser = nullptr;
        HWND parentWnd = (HWND)parent;
        if (m_isEdge) {
            SciterEdgeWebView* browser = new SciterEdgeWebView(debug);
            if (NULL != browser->Create(parentWnd, 0, 0, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS)) {
                m_winbrowser = browser;
            }
            else {
                delete browser;
            }
        }
        if (nullptr == m_winbrowser) {
            m_isEdge = false;
            SciterIEWebView* browser = new SciterIEWebView(debug);
            if (NULL != browser->Create(parentWnd, 0, 0, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS)) {
                m_winbrowser = browser;
            }
            else {
                delete browser;
            }
        }
    }

    sciter_winwebview::~sciter_winwebview()
    {
        if (m_isEdge) {
            if (nullptr != m_winbrowser) {
                ((SciterEdgeWebView*)m_winbrowser)->DestroyWindow();
                delete m_winbrowser;
            }
        }
        else {
            if (nullptr != m_winbrowser) {
                ((SciterIEWebView*)m_winbrowser)->DestroyWindow();
                delete m_winbrowser;
            }
        }
        m_winbrowser = nullptr;
    }

    void sciter_winwebview::load_engine(const completion_fn_t& completion) {
        if (nullptr == m_winbrowser) {
            return;
        }
        if (m_isEdge) {
            ((SciterEdgeWebView*)m_winbrowser)->load_engine(completion);
        }
        else {
            ((SciterIEWebView*)m_winbrowser)->load_engine(completion);
        }
    }

    void sciter_winwebview::navigate(const std::string& url)
    {
        if (nullptr == m_winbrowser) {
            return;
        }
        if (m_isEdge) {
            ((SciterEdgeWebView*)m_winbrowser)->navigate(url);
        }
        else {
            ((SciterIEWebView*)m_winbrowser)->navigate(url);
        }
    }

    void sciter_winwebview::reload()
    {
        if (nullptr == m_winbrowser) {
            return;
        }
        if (m_isEdge) {
            ((SciterEdgeWebView*)m_winbrowser)->reload();
        }
        else {
            ((SciterIEWebView*)m_winbrowser)->reload();
        }
    }

    void sciter_winwebview::go_back() {
        if (nullptr == m_winbrowser) {
            return;
        }
        if (m_isEdge) {
            ((SciterEdgeWebView*)m_winbrowser)->go_back();
        }
        else {
            ((SciterIEWebView*)m_winbrowser)->go_back();
        }
    }

    void sciter_winwebview::go_forward() {
        if (nullptr == m_winbrowser) {
            return;
        }
        if (m_isEdge) {
            ((SciterEdgeWebView*)m_winbrowser)->go_forward();
        }
        else {
            ((SciterIEWebView*)m_winbrowser)->go_forward();
        }
    }

    void sciter_winwebview::stop() {
        if (nullptr == m_winbrowser) {
            return;
        }
        if (m_isEdge) {
            ((SciterEdgeWebView*)m_winbrowser)->stop();
        }
        else {
            ((SciterIEWebView*)m_winbrowser)->stop();
        }
    }

    void sciter_winwebview::advance_focus(bool forward) {
      if (nullptr == m_winbrowser) {
        return;
      }
      if (m_isEdge) {
        ((SciterEdgeWebView*)m_winbrowser)->advance_focus(forward);
      }
      else {
        ((SciterIEWebView*)m_winbrowser)->advance_focus(forward);
      }
    }

    void* sciter_winwebview::window()
    {
        if (nullptr == m_winbrowser) {
            return nullptr;
        }
        if (m_isEdge) {
            return ((SciterEdgeWebView*)m_winbrowser)->m_hWnd;
        }
        else {
            return ((SciterIEWebView*)m_winbrowser)->m_hWnd;
        }
    }

    void sciter_winwebview::set_size(int width, int height, int hints)
    {
        /*
        if (m_isEdge) {
            return ((SciterEdgeWebView*)m_winbrowser)->set_size(width, height, hints);
        }
        else {
            return ((SciterIEWebView*)m_winbrowser)->set_size(width, height, hints);
        }
        */
    }

    void sciter_winwebview::set_title(const std::string& url)
    {
    }

    void sciter_winwebview::init(const std::string& js)
    {
        if (nullptr == m_winbrowser) {
            return;
        }
        if (m_isEdge) {
            ((SciterEdgeWebView*)m_winbrowser)->init(js);
        }
        else {
            ((SciterIEWebView*)m_winbrowser)->init(js);
        }
    }

    void sciter_winwebview::eval(const std::string& js)
    {
        if (nullptr == m_winbrowser) {
            return;
        }
        if (m_isEdge) {
            ((SciterEdgeWebView*)m_winbrowser)->eval(js);
        }
        else {
            ((SciterIEWebView*)m_winbrowser)->eval(js);
        }
    }

    void sciter_winwebview::set_html(const std::string& html)
    {
        if (nullptr == m_winbrowser) {
            return;
        }
        if (m_isEdge) {
            ((SciterEdgeWebView*)m_winbrowser)->set_html(html);
        }
        else {
            ((SciterIEWebView*)m_winbrowser)->set_html(html);
        }
    }

    void sciter_winwebview::dispatch(std::function<void()> f)
    {
        if (nullptr == m_winbrowser) {
            return;
        }
        if (m_isEdge) {
            ((SciterEdgeWebView*)m_winbrowser)->dispatch(f);
        }
        else {
            ((SciterIEWebView*)m_winbrowser)->dispatch(f);
        }
    }

    void sciter_winwebview::set_navigation_callback(const navigation_callback_t& cb)
    {
        if (nullptr == m_winbrowser) {
            return;
        }
        if (m_isEdge) {
            ((SciterEdgeWebView*)m_winbrowser)->set_navigation_callback(cb);
        }
        else {
            ((SciterIEWebView*)m_winbrowser)->set_navigation_callback(cb);
        }
    }

    void sciter_winwebview::set_msg_callback(const msg_callback_t& cb)
    {
        if (nullptr == m_winbrowser) {
            return;
        }
        if (m_isEdge) {
            ((SciterEdgeWebView*)m_winbrowser)->set_msg_callback(cb);
        }
        else {
            ((SciterIEWebView*)m_winbrowser)->set_msg_callback(cb);
        }
    }

    void sciter_winwebview::set_allowWindowOpen(const std::string& val)
    {
        if (nullptr == m_winbrowser) {
            return;
        }
        if (m_isEdge) {
            ((SciterEdgeWebView*)m_winbrowser)->set_allowWindowOpen(val);
        }
        else {
            ((SciterIEWebView*)m_winbrowser)->set_allowWindowOpen(val);
        }
    }

    void sciter_winwebview::set_allowContextMenu(const std::string& val)
    {
        if (nullptr == m_winbrowser) {
            return;
        }
        if (m_isEdge) {
            ((SciterEdgeWebView*)m_winbrowser)->set_allowContextMenu(val);
        }
        else {
            ((SciterIEWebView*)m_winbrowser)->set_allowContextMenu(val);
        }
    }

    std::string sciter_winwebview::currentSrc() {
        if (nullptr == m_winbrowser) {
            return "";
        }
        if (m_isEdge) {
            return ((SciterEdgeWebView*)m_winbrowser)->currentSrc();
        }
        else {
            return ((SciterIEWebView*)m_winbrowser)->currentSrc();
        }
    }

}