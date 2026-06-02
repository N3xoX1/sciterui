#include "sciter_window.h"
#include "event_handler.h"
#include "sciter.h"
#include "sciter_dpi.h"
#include "sciter_handler_internal.h"
#include "std_string.h"
#include <sciter-x-api.h>
#include <sciter-x-def.h>
#include <sciter_handler.h>
#include <algorithm>
#include <stdint.h>

#undef HWINDOW
#undef max
#undef min

namespace SciterUI
{

SciterWindow::SciterWindow(Sciter & sciter) :
    m_sciter(sciter),
    m_hWnd(nullptr),
    m_hParent(nullptr),
    m_createParent(nullptr),
    m_layoutWidth(0),
    m_layoutHeight(0),
    m_bound(false),
    m_destroyed(false)
{
}

SciterWindow::~SciterWindow()
{
}

void SciterWindow::Show()
{
    ::SciterWindowExec((HWND)m_hWnd, SCITER_WINDOW_SET_STATE, SCITER_WINDOW_STATE_SHOWN, 0);
}

bool SciterWindow::Create(HWINDOW parentWinow, const char * htmlFile, int x, int y, int width, int height, unsigned int flags)
{
    m_createParent = parentWinow;
    m_layoutWidth = width;
    m_layoutHeight = height;

    bool childWindow = parentWinow != nullptr && (flags & SUIW_CHILD) != 0;
    bool startHidden = (flags & SUIW_HIDDEN) != 0;

#ifdef WIN32
    DWORD exStyle = childWindow ? (WS_EX_DLGMODALFRAME | WS_EX_TOOLWINDOW) : WS_EX_APPWINDOW;
    DWORD style = childWindow ? (DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS) : (WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
    m_hWnd = CreateWindowEx(exStyle, m_sciter.WindowClass().c_str(), L"", style, x, y, width, height, (HWND)parentWinow, nullptr, GetModuleHandle(nullptr), &m_sciter);
#else
    RECT Frame;
    Frame.left = x;
    Frame.top = y;
    Frame.right = x + width;
    Frame.bottom = y + height;

    m_hWnd = ::SciterCreateWindow(flags, (Frame.right - Frame.left) > 0 ? &Frame : nullptr, nullptr, this, (HWND)parentWinow);
#endif
    if (m_hWnd != nullptr)
    {
        if (childWindow)
        {
            m_hParent = parentWinow;
            EnableWindow((HWND)parentWinow, FALSE);
        }
        SciterSetOption((HWND)m_hWnd, SCITER_SET_SCRIPT_RUNTIME_FEATURES, ALLOW_FILE_IO | ALLOW_SOCKET_IO | ALLOW_EVAL | ALLOW_SYSINFO);

        m_sciter.WindowCreated(this);
        LoadHtml(htmlFile);
        SetDefaultWindowSize(x, y, width, height);
        if (!startHidden)
        {
            ::SciterWindowExec((HWND)m_hWnd, SCITER_WINDOW_SET_STATE, SCITER_WINDOW_STATE_SHOWN, 0);
        }
    }
    return m_hWnd != nullptr;
}

void SciterWindow::CenterWindow(void)
{
    int32_t xScreen = GetSystemMetrics(SM_CXSCREEN), x;
    int32_t yScreen = GetSystemMetrics(SM_CYSCREEN), y;
    RECT rc;
    if (!GetWindowRect((HWND)m_hWnd, &rc))
    {
        return;
    }
    HWND parent = GetParent((HWND)m_hWnd);
    RECT parentRect;
    if (parent != nullptr && !::IsIconic(parent) && ::IsWindowVisible(parent) && GetWindowRect(parent, &parentRect))
    {
        int32_t width = rc.right - rc.left;
        int32_t height = rc.bottom - rc.top;
        x = ((parentRect.right - parentRect.left) - (width)) / 2 + parentRect.left;
        y = ((parentRect.bottom - parentRect.top) - (height)) / 2 + parentRect.top;
        if (x < 0 && width < xScreen)
        {
            x = 0;
        }
        if (y < 0 && height < yScreen)
        {
            y = 0;
        }
        if (x + width > xScreen)
        {
            x = xScreen - width;
        }
        if (y + height > yScreen)
        {
            y = yScreen - height;
        }
    }
    else
    {
        x = (xScreen - (rc.right - rc.left)) / 2;
        y = (yScreen - (rc.bottom - rc.top)) / 2;
    }
    SetWindowPos((HWND)m_hWnd, 0, x, y, 0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE);
}

void SciterWindow::FixMinSize()
{
    if (m_hWnd == nullptr)
    {
        return;
    }

    SciterUpdateWindow((HWND)m_hWnd);

#ifdef WIN32
    const uint32_t minWidth = SciterGetMinWidth((HWND)m_hWnd);
    const uint32_t layoutWidth = m_layoutWidth > 0 ? static_cast<uint32_t>(m_layoutWidth) : minWidth;
    const uint32_t minHeight = SciterGetMinHeight((HWND)m_hWnd, layoutWidth);

    uint32_t width = std::max(minWidth, layoutWidth);
    uint32_t height;
    if (m_layoutHeight > 0)
    {
        height = std::max(minHeight, static_cast<uint32_t>(m_layoutHeight));
    }
    else
    {
        height = minHeight;
    }

    int w = static_cast<int>(width);
    int h = static_cast<int>(height);
    ScaleWindowSizeForDpi(m_createParent, w, h);
    SetWindowPos((HWND)m_hWnd, nullptr, 0, 0, w, h, SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER);
#else
    RECT rc;
    if (!GetWindowRect((HWND)m_hWnd, &rc))
    {
        return;
    }
    uint32_t minWidth = SciterGetMinWidth((HWND)m_hWnd);
    uint32_t minHeight = SciterGetMinHeight((HWND)m_hWnd, minWidth);
    uint32_t currentWidth = (rc.right - rc.left);
    uint32_t currentHeight = (rc.bottom - rc.top);
    uint32_t targetWidth = currentWidth < minWidth ? minWidth : currentWidth;
    uint32_t targetHeight = currentHeight < minHeight ? minHeight : currentHeight;
    if (currentWidth != targetWidth || currentHeight != targetHeight)
    {
        SetWindowPos((HWND)m_hWnd, 0, 0, 0, targetWidth, targetHeight, SWP_NOOWNERZORDER | SWP_NOMOVE);
    }
#endif
}

HWINDOW SciterWindow::GetHandle() const
{
    return m_hWnd;
}

uint32_t SciterWindow::GetMinWidth() const
{
    return SciterGetMinWidth((HWND)m_hWnd);
}

uint32_t SciterWindow::GetMinHeight(uint32_t width) const
{
    return SciterGetMinHeight((HWND)m_hWnd, width);
}

SCITER_ELEMENT SciterWindow::GetRootElement(void) const
{
    HELEMENT h = 0;
    SciterGetRootElement((HWND)m_hWnd, &h);
    return h;
}

void SciterWindow::OnDestroySinkAdd(IWindowDestroySink * Sink)
{
    m_onDestroySink.insert(Sink);
}

void SciterWindow::OnDestroySinkRemove(IWindowDestroySink * Sink)
{
    WinDestroySinks::iterator itr = m_onDestroySink.find(Sink);
    if (itr != m_onDestroySink.end())
    {
        m_onDestroySink.erase(itr);
    }
}

void SciterWindow::OnCloseSinkAdd(IWindowCloseSink * Sink)
{
    m_onCloseSink.insert(Sink);
}

void SciterWindow::OnCloseSinkRemove(IWindowCloseSink * Sink)
{
    WinCloseSinks::iterator itr = m_onCloseSink.find(Sink);
    if (itr != m_onCloseSink.end())
    {
        m_onCloseSink.erase(itr);
    }
}

bool SciterWindow::QueryClose() const
{
    for (WinCloseSinks::const_iterator itr = m_onCloseSink.begin(); itr != m_onCloseSink.end(); itr++)
    {
        if (!(*itr)->OnWindowCloseRequest(m_hWnd))
        {
            return false;
        }
    }
    return true;
}

bool SciterWindow::Destroy()
{
    return PostMessage((HWND)m_hWnd, WM_CLOSE, 0, 0) != 0;
}

void SciterWindow::RunModal()
{
    if (m_hWnd == nullptr || m_destroyed)
    {
        return;
    }
#ifdef WIN32
    const HWND hwnd = (HWND)m_hWnd;
    while (!m_destroyed && IsWindow(hwnd))
#else
    while (!m_destroyed)
#endif
    {
        if (!SciterExec(SCITER_APP_LOOP_ITERATION, 0, 0))
        {
            break;
        }
    }
}

bool SciterWindow::IsClosed() const
{
    return m_destroyed;
}

bool SciterWindow::GetDestroyed(void) const
{
    return m_destroyed;
}

void SciterWindow::SetDestroyed(void)
{
    if (m_destroyed)
    {
        return;
    }
    m_destroyed = true;
    if (m_hParent != nullptr)
    {
        EnableWindow((HWND)m_hParent, TRUE);
        SetForegroundWindow((HWND)m_hParent);
        SetFocus((HWND)m_hParent);
    }
    for (EventSinks::iterator itr = m_eventSinks.begin(); itr != m_eventSinks.end(); itr++)
    {
        EventHandler * handler = itr->Sink.get();
        if (IID_ICLICKSINK == itr->riid)
        {
            SciterDetachEventHandler((HELEMENT)itr->Element, (::LPELEMENT_EVENT_PROC)&EventHandler::ClickHandler, handler);
        }
    }
    m_eventSinks.clear();
}

bool SciterWindow::AttachHandler(SCITER_ELEMENT element, const char * riid, void * interfacePtr)
{
    if (m_destroyed)
    {
        return false;
    }

    LPELEMENT_EVENT_PROC eventProc = nullptr;
    UINT subscription = 0;
    GetEventProc(riid, eventProc, subscription);

    if (eventProc == nullptr || subscription == 0)
    {
        return false;
    }
    std::unique_ptr<EventHandler> eventHandler(new EventHandler(m_sciter, element, interfacePtr, subscription));
    if (eventHandler.get())
    {
        SCDOM_RESULT hr = SciterAttachEventHandler((HELEMENT)element, (::LPELEMENT_EVENT_PROC)eventProc, eventHandler.get());
        if (SUCCEEDED(hr))
        {
            m_eventSinks.push_back(RegisteredSink(element, riid, interfacePtr, std::move(eventHandler)));
            return true;
        }
    }
    return false;
}

bool SciterWindow::DetachHandler(SCITER_ELEMENT Element, const char * riid, void * interfacePtr)
{
    if (m_destroyed)
    {
        return false;
    }

    RegisteredSink Sink(Element, riid, interfacePtr, nullptr);
    EventSinks::iterator iter = std::find(m_eventSinks.begin(), m_eventSinks.end(), Sink);
    bool result = false;
    if (iter != m_eventSinks.end())
    {
        EventHandler * handler = iter->Sink.get();
        LPELEMENT_EVENT_PROC EventProc = nullptr;
        UINT Subscription = 0;
        if (GetEventProc(riid, EventProc, Subscription))
        {
            SCDOM_RESULT r = SciterDetachEventHandler((HELEMENT)Element, (::LPELEMENT_EVENT_PROC)EventProc, handler);
            result = r == SCDOM_OK;
        }
        m_eventSinks.erase(iter);
    }
    return result;
}

void SciterWindow::Bind()
{
    if (m_hWnd && !m_bound)
    {
        m_bound = true;
        SciterSetCallback((HWND)m_hWnd, (LPSciterHostCallback)SciterCallback, this);
    }
}

bool SciterWindow::LoadHtml(const char * url)
{
    Bind();
    std::wstring loadUrl = stdstr_f(_strnicmp(url, "file://", 7) == 0 ? "%s" : "file://%s", url).ToUTF16();
    return FALSE != ::SciterLoadFile((HWND)m_hWnd, loadUrl.c_str());
}

bool SciterWindow::GetEventProc(const char * riid, LPELEMENT_EVENT_PROC & eventProc, uint32_t & subscription)
{
    if (strcmp(IID_ICLICKSINK, riid) == 0)
    {
        eventProc = &EventHandler::ClickHandler;
        subscription = HANDLE_MOUSE | HANDLE_BEHAVIOR_EVENT;
    }
    else if (strcmp(IID_IDBLCLICKSINK, riid) == 0)
    {
        eventProc = &EventHandler::DoubleClickHandler;
        subscription = HANDLE_MOUSE;
    }
    else if (strcmp(IID_ITIMERSINK, riid) == 0)
    {
        eventProc = &EventHandler::TimerHandler;
        subscription = HANDLE_TIMER;
    }
    else if (strcmp(IID_IMOUSEUPDOWNSINK, riid) == 0)
    {
        eventProc = &EventHandler::MousedUpDownHandler;
        subscription = HANDLE_MOUSE;
    }
    else if (strcmp(IID_IMOUSEMOVESINK, riid) == 0)
    {
        eventProc = &EventHandler::MousedMoveHandler;
        subscription = HANDLE_MOUSE;
    }
    else if (strcmp(IID_IKEYSINK, riid) == 0)
    {
        eventProc = &EventHandler::KeyHandler;
        subscription = HANDLE_KEY;
    }
    else if (strcmp(IID_IRESIZESINK, riid) == 0)
    {
        eventProc = &EventHandler::ResizeHandler;
        subscription = HANDLE_SIZE;
    }
    else if (strcmp(IID_FORWARD_BEHAVIOUR, riid) == 0)
    {
        eventProc = &EventHandler::ForwardBehaviorHandler;
        subscription = HANDLE_BEHAVIOR_EVENT;
    }
    else if (strcmp(IID_ISTATECHANGESINK, riid) == 0)
    {
        eventProc = &EventHandler::StateChangeHandler;
        subscription = HANDLE_BEHAVIOR_EVENT;
    }
    else if (strcmp(IID_EVENTSINK, riid) == 0)
    {
        eventProc = &EventHandler::EventSinkHandler;
        subscription = HANDLE_BEHAVIOR_EVENT;
    }
    else
    {
        eventProc = nullptr;
        subscription = 0;
        return false;
    }
    return true;
}

void SciterWindow::SetDefaultWindowSize(int x, int y, int width, int height)
{
    if (!m_hWnd)
    {
        return;
    }

    if (width <= 0 && height <= 0)
    {
        return;
    }

#ifdef WIN32
    if (height <= 0)
    {
        return;
    }

    int w = width;
    int h = height;
    ScaleWindowSizeForDpi(m_createParent, w, h);
    SetWindowPos((HWND)m_hWnd, nullptr, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
#else
    SetWindowPos((HWND)m_hWnd, nullptr, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
#endif
}

LRESULT SciterWindow::HandleNotification(LPSCITER_CALLBACK_NOTIFICATION pnm)
{
    switch (pnm->code)
    {
    case SC_LOAD_DATA:
        return OnLoadData((LPSCN_LOAD_DATA)pnm);
    case SC_ATTACH_BEHAVIOR:
        return OnAttachBehavior((LPSCN_ATTACH_BEHAVIOR)pnm);
    case SC_ENGINE_DESTROYED:
        return OnEngineDestroyed();
    }
    return 0;
}

LRESULT SciterWindow::OnLoadData(LPSCN_LOAD_DATA pnmld)
{
    if (pnmld == nullptr)
    {
        return LOAD_DISCARD;
    }
    if (wcsncmp(pnmld->uri, L"data:", 5) == 0)
    {
        return LOAD_OK;
    }

    ResourceManager & manager = m_sciter.GetResourceManager();
    std::unique_ptr<uint8_t> data;
    uint32_t dataSize = 0;
    if (!manager.LoadResource(pnmld->uri, data, dataSize))
    {
        return LOAD_DISCARD;
    }
    ::SciterDataReady((HWND)pnmld->hwnd, pnmld->uri, data.get(), dataSize);
    return LOAD_OK;
}

LRESULT SciterWindow::OnAttachBehavior(LPSCN_ATTACH_BEHAVIOR pnmld)
{
    return m_sciter.AttachWidget((Sciter::LPSCN_ATTACH_BEHAVIOR)pnmld);
}

LRESULT SciterWindow::OnEngineDestroyed(void)
{
    for (WinDestroySinks::const_iterator itr = m_onDestroySink.begin(); itr != m_onDestroySink.end(); itr++)
    {
        (*itr)->OnWindowDestroy(m_hWnd);
    }
    m_sciter.WindowDestroyed(this);
    return 0;
}

UINT SciterWindow::SciterCallback(LPSCITER_CALLBACK_NOTIFICATION pnm, LPVOID param)
{
    SciterWindow * Self = (SciterWindow *)param;
    return (UINT)Self->HandleNotification(pnm);
}
} // namespace SciterUI
