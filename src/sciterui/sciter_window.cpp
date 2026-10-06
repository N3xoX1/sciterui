#include "sciter_window.h"
#include "event_handler.h"
#include "sciter.h"
#include "sciter_dpi.h"
#include "sciter_handler_internal.h"
#include "std_string.h"
#include "sciter_hwindow.h"
#if defined(__linux__)
#include "linux_engine_loop.h"
#include <sciter_wayland_native.h>
#endif
#include <sciter_element.h>
#include <sciter_handler.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <stdint.h>

#undef max
#undef min

namespace SciterUI
{

#ifdef __APPLE__
void DetachMacOSWindowTerminationObserver(const void * handle);
void ScheduleMacOSWindowClose(std::shared_ptr<SciterWindow> window);
#endif

SciterWindow::SciterWindow(Sciter & sciter) :
    m_sciter(sciter),
    m_hWnd(nullptr),
    m_hParent(nullptr),
    m_createParent(nullptr),
    m_parentState(0),
    m_layoutWidth(0),
    m_layoutHeight(0),
    m_bound(false),
    m_destroyed(false),
    m_parentEnabled(true)
{
}

SciterWindow::~SciterWindow()
{
}

void SciterWindow::Show()
{
#if defined(__linux__)
    SciterUIConfigureGtkWindow(m_hWnd, m_hParent, m_applicationId.c_str());
#endif
    ::SciterWindowExec((SciterHWINDOW)m_hWnd, SCITER_WINDOW_SET_STATE, SCITER_WINDOW_STATE_SHOWN, 0);
}

bool SciterWindow::Create(HWINDOW parentWinow, const char * htmlFile, int x, int y, int width, int height, unsigned int flags)
{
    m_createParent = parentWinow;
    m_layoutWidth = width;
    m_layoutHeight = height;

    bool childWindow = parentWinow != nullptr && (flags & SUIW_CHILD) != 0;
    bool startHidden = (flags & SUIW_HIDDEN) != 0;
    flags &= ~((uint32_t)SUIW_HIDDEN);

#ifdef WIN32
    DWORD exStyle = childWindow ? (WS_EX_DLGMODALFRAME | WS_EX_TOOLWINDOW) : WS_EX_APPWINDOW;
    DWORD style = childWindow ? (DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS) : (WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
    m_hWnd = CreateWindowEx(exStyle, m_sciter.WindowClass().c_str(), L"", style, x, y, width, height, (HWND)parentWinow, nullptr, GetModuleHandle(nullptr), &m_sciter);
#else
    RECT Frame{};
    Frame.left = x;
    Frame.top = y;
    Frame.right = x + width;
    Frame.bottom = y + height;

    m_hWnd = ::SciterCreateWindow(flags, (Frame.right - Frame.left) > 0 ? &Frame : nullptr, nullptr, nullptr, (SciterHWINDOW)parentWinow);
#endif
    if (m_hWnd != nullptr)
    {
        if (childWindow)
        {
            m_hParent = parentWinow;
            m_parentState = (int)::SciterWindowExec((SciterHWINDOW)parentWinow, SCITER_WINDOW_GET_STATE, 0, 0);
#ifdef WIN32
            m_parentEnabled = IsWindowEnabled((HWND)parentWinow) != FALSE;
            EnableWindow((HWND)parentWinow, FALSE);
#endif
        }
        SciterSetOption((SciterHWINDOW)m_hWnd, SCITER_SET_SCRIPT_RUNTIME_FEATURES, ALLOW_FILE_IO | ALLOW_SOCKET_IO | ALLOW_EVAL | ALLOW_SYSINFO);

        m_sciter.WindowCreated(this);
        if (!LoadHtml(htmlFile))
        {
            SetDestroyed();
            if (m_hWnd != nullptr)
            {
#ifdef WIN32
                DestroyWindow((HWND)m_hWnd);
#else
                ::SciterWindowExec((SciterHWINDOW)m_hWnd, SCITER_WINDOW_SET_STATE, SCITER_WINDOW_STATE_CLOSED, TRUE);
#endif
                m_hWnd = nullptr;
            }
            m_sciter.WindowDestroyed(this);
            return false;
        }
#if defined(__linux__)
        m_applicationId = SciterElement(GetRootElement()).GetAttribute("data-application-id");
        if (m_applicationId.empty())
        {
            if (const auto* parent = m_sciter.FindSciterWindow(m_createParent))
                m_applicationId = parent->m_applicationId;
        }
#endif
        SetDefaultWindowSize(x, y, width, height);
        if (!startHidden)
        {
            Show();
        }
    }
    return m_hWnd != nullptr;
}

void SciterWindow::CenterWindow(void)
{
#ifdef WIN32
    const HWND hwnd = (HWND)m_hWnd;
    RECT windowRect{};
    if (!GetWindowRect(hwnd, &windowRect))
    {
        return;
    }

    const HWND parent = GetParent(hwnd);
    const HWND monitorWindow = parent != nullptr && IsWindow(parent) ? parent : hwnd;
    const HMONITOR monitor = MonitorFromWindow(monitorWindow, MONITOR_DEFAULTTONEAREST);
    MONITORINFO monitorInfo{sizeof(MONITORINFO)};
    if (monitor == nullptr || !GetMonitorInfoW(monitor, &monitorInfo))
    {
        return;
    }

    const RECT & workArea = monitorInfo.rcWork;
    const int32_t width = windowRect.right - windowRect.left;
    const int32_t height = windowRect.bottom - windowRect.top;
    int32_t x;
    int32_t y;

    RECT parentRect{};
    if (parent != nullptr && !IsIconic(parent) && IsWindowVisible(parent) && GetWindowRect(parent, &parentRect))
    {
        x = ((parentRect.right - parentRect.left) - (width)) / 2 + parentRect.left;
        y = ((parentRect.bottom - parentRect.top) - (height)) / 2 + parentRect.top;
    }
    else
    {
        x = workArea.left + ((workArea.right - workArea.left) - width) / 2;
        y = workArea.top + ((workArea.bottom - workArea.top) - height) / 2;
    }

    x = width >= workArea.right - workArea.left ? workArea.left : std::max<int32_t>(workArea.left, std::min<int32_t>(x, workArea.right - width));
    y = height >= workArea.bottom - workArea.top ? workArea.top : std::max<int32_t>(workArea.top, std::min<int32_t>(y, workArea.bottom - height));

    SetWindowPos(hwnd, nullptr, x, y, 0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE);
#elif defined(__linux__)
    // Native Wayland toplevel positioning is owned by the compositor.
#endif
}

void SciterWindow::FixMinSize()
{
    if (m_hWnd == nullptr)
    {
        return;
    }

    SciterUpdateWindow((SciterHWINDOW)m_hWnd);

#ifdef WIN32
    int scaledLayoutWidth = 0;
    int scaledLayoutHeight = 0;
    if (m_layoutWidth > 0 || m_layoutHeight > 0)
    {
        scaledLayoutWidth = m_layoutWidth > 0 ? m_layoutWidth : 0;
        scaledLayoutHeight = m_layoutHeight > 0 ? m_layoutHeight : 0;
        ScaleWindowSizeForDpi(m_createParent, scaledLayoutWidth, scaledLayoutHeight);
        if (m_layoutWidth <= 0)
        {
            scaledLayoutWidth = 0;
        }
        if (m_layoutHeight <= 0)
        {
            scaledLayoutHeight = 0;
        }
    }

    const uint32_t minWidth = SciterGetMinWidth((SciterHWINDOW)m_hWnd);
    const uint32_t widthForHeight = scaledLayoutWidth > 0 ? static_cast<uint32_t>(scaledLayoutWidth) : minWidth;
    const uint32_t minHeight = SciterGetMinHeight((SciterHWINDOW)m_hWnd, widthForHeight);

    int width = static_cast<int>(scaledLayoutWidth > 0 ? std::max(minWidth, static_cast<uint32_t>(scaledLayoutWidth)) : minWidth);
    int height = static_cast<int>(scaledLayoutHeight > 0 ? std::max(minHeight, static_cast<uint32_t>(scaledLayoutHeight)) : minHeight);

    ClampWindowSizeToWorkArea(m_createParent, width, height);
    SetWindowPos((HWND)m_hWnd, nullptr, 0, 0, width, height, SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER);
#elif defined(__linux__)
    UINT ppiX = 96, ppiY = 96;
    SciterGetPPI((SciterHWINDOW)m_hWnd, &ppiX, &ppiY);
    const double scaleX = ppiX ? ppiX / 96.0 : 1.0;
    const double scaleY = ppiY ? ppiY / 96.0 : 1.0;
    const int layoutWidth = static_cast<int>(std::ceil(std::max(0, m_layoutWidth) * scaleX));
    const int layoutHeight = static_cast<int>(std::ceil(std::max(0, m_layoutHeight) * scaleY));
    SIZE placement{};
    SciterWindowExec((SciterHWINDOW)m_hWnd, SCITER_WINDOW_GET_PLACEMENT, 0,
                     reinterpret_cast<UINT_PTR>(&placement));
    RECT rootBox{};
    HELEMENT root = nullptr;
    int frameWidth = 0;
    int frameHeight = 0;
    // Placement includes native shadows; intrinsic dimensions describe the
    // document. Measure the insets instead of assuming a theme or DPI scale.
    if (SciterGetRootElement((SciterHWINDOW)m_hWnd, &root) == SCDOM_OK &&
        SciterGetElementLocation(root, &rootBox, VIEW_RELATIVE | BORDER_BOX) == SCDOM_OK)
    {
        frameWidth = std::max(0, placement.cx - (rootBox.right - rootBox.left));
        frameHeight = std::max(0, placement.cy - (rootBox.bottom - rootBox.top));
    }
    const uint32_t minWidth = SciterGetMinWidth((SciterHWINDOW)m_hWnd);
    int width = std::max(layoutWidth, static_cast<int>(minWidth));
    int contentHeight = 0;
    RECT contentBox{};
    UINT childCount = 0;
    if (root != nullptr &&
        SciterGetElementLocation(root, &contentBox, VIEW_RELATIVE | CONTENT_BOX) == SCDOM_OK &&
        SciterGetChildrenCount(root, &childCount) == SCDOM_OK)
    {
        // Intrinsic dimensions may still describe the previous page. Include
        // overflow of the current visible content without retaining unused space.
        for (UINT i = 0; i < childCount; ++i)
        {
            HELEMENT child = nullptr;
            SBOOL visible = FALSE;
            RECT box{};
            if (SciterGetNthChild(root, i, &child) != SCDOM_OK ||
                SciterIsElementVisible(child, &visible) != SCDOM_OK || !visible ||
                SciterGetElementLocation(child, &box, VIEW_RELATIVE | MARGIN_BOX) != SCDOM_OK)
                continue;
            if (box.right > rootBox.right)
                width = std::max(width, box.right - rootBox.left + rootBox.right - contentBox.right);
            if (box.bottom > rootBox.bottom)
                contentHeight = std::max(contentHeight, box.bottom - rootBox.top + rootBox.bottom - contentBox.bottom);
        }
    }
    // A different width changes text wrapping; use the intrinsic height for it.
    if (std::abs(width - (rootBox.right - rootBox.left)) > 1)
        contentHeight = 0;
    const uint32_t minHeight = SciterGetMinHeight((SciterHWINDOW)m_hWnd, width);
    SIZE size{width + frameWidth, std::max({layoutHeight, static_cast<int>(minHeight), contentHeight}) + frameHeight};
    if (size.cx != placement.cx || size.cy != placement.cy)
    {
        SciterWindowExec((SciterHWINDOW)m_hWnd, SCITER_WINDOW_SET_PLACEMENT, 0,
                         reinterpret_cast<UINT_PTR>(&size));
        SciterElement(root).Eval("Window.this.update()");
    }
#endif
}

HWINDOW SciterWindow::GetHandle() const
{
    return m_hWnd;
}

uint32_t SciterWindow::GetMinWidth() const
{
    return SciterGetMinWidth((SciterHWINDOW)m_hWnd);
}

uint32_t SciterWindow::GetMinHeight(uint32_t width) const
{
    return SciterGetMinHeight((SciterHWINDOW)m_hWnd, width);
}

SCITER_ELEMENT SciterWindow::GetRootElement(void) const
{
    HELEMENT h = 0;
    SciterGetRootElement((SciterHWINDOW)m_hWnd, &h);
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
    const WinCloseSinks sinks = m_onCloseSink;
    for (WinCloseSinks::const_iterator itr = sinks.begin(); itr != sinks.end(); itr++)
    {
        if (m_onCloseSink.find(*itr) == m_onCloseSink.end())
        {
            continue;
        }
        if (!(*itr)->OnWindowCloseRequest(m_hWnd))
        {
            return false;
        }
    }
    return true;
}

bool SciterWindow::Destroy()
{
    // Engine destruction removes the owner's reference synchronously. Keep the
    // wrapper alive until this call (including close sinks) has returned.
    const auto keepAlive = shared_from_this();
    if (m_hWnd == nullptr || m_destroyed)
    {
        return false;
    }
#ifdef WIN32
    const HWND hwnd = (HWND)m_hWnd;
#endif
    if (!QueryClose())
    {
        return false;
    }
    if (m_destroyed || m_hWnd == nullptr)
    {
        return false;
    }
    SetDestroyed();
#ifdef WIN32
    return PostMessage(hwnd, WM_CLOSE, 0, 0) != 0;
#elif defined(__linux__)
    // Destroy after the current native DOM/timer dispatch has returned.
    QueueLinuxEngineClose(keepAlive);
#elif defined(__APPLE__)
    // Closing from a timer or DOM callback must not destroy the native engine
    // while its heartbeat is still traversing the engine's dispatch list.
    ScheduleMacOSWindowClose(keepAlive);
#else
    ::SciterWindowExec((SciterHWINDOW)m_hWnd, SCITER_WINDOW_SET_STATE, SCITER_WINDOW_STATE_CLOSED, TRUE);
#endif
    return true;
}

void SciterWindow::RunModal()
{
    // The modal loop resumes after the callback that removes this window from
    // m_CreatedWindows. Its loop condition must still have a live wrapper.
    const auto keepAlive = shared_from_this();
    if (m_hWnd == nullptr || m_destroyed)
    {
        return;
    }
#ifdef WIN32
    const HWND hwnd = (HWND)m_hWnd;
    while (m_hWnd != nullptr && IsWindow(hwnd))
    {
        if (!SciterExec(SCITER_APP_LOOP_ITERATION, 0, 0))
        {
            break;
        }
    }
#elif defined(__linux__)
    while (m_hWnd != nullptr && LinuxEngineLoopIteration()) {}
#else
    while (m_hWnd != nullptr)
    {
        if (!SciterExec(SCITER_APP_LOOP_ITERATION, 0, 0))
        {
            break;
        }
    }
#endif
}

bool SciterWindow::IsClosed() const
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
#ifdef __APPLE__
    DetachMacOSWindowTerminationObserver(m_hWnd);
#endif
#if !defined(__linux__)
    if (m_hParent != nullptr)
    {
        if (m_hWnd != nullptr)
        {
            ::SciterWindowExec((SciterHWINDOW)m_hWnd, SCITER_WINDOW_SET_STATE, SCITER_WINDOW_STATE_HIDDEN, 0);
        }
#ifdef WIN32
        EnableWindow((HWND)m_hParent, m_parentEnabled ? TRUE : FALSE);
#endif
        if (m_parentEnabled &&
            (m_parentState == SCITER_WINDOW_STATE_SHOWN ||
             m_parentState == SCITER_WINDOW_STATE_MAXIMIZED ||
             m_parentState == SCITER_WINDOW_STATE_FULL_SCREEN))
        {
            ::SciterWindowExec((SciterHWINDOW)m_hParent, SCITER_WINDOW_SET_STATE, (UINT_PTR)m_parentState, 0);
            ::SciterWindowExec((SciterHWINDOW)m_hParent, SCITER_WINDOW_ACTIVATE, TRUE, 0);
        }
    }
#endif
    for (EventSinks::iterator itr = m_eventSinks.begin(); itr != m_eventSinks.end(); itr++)
    {
        EventHandler * handler = itr->Sink.get();
        LPELEMENT_EVENT_PROC eventProc = nullptr;
        uint32_t subscription = 0;
        if (GetEventProc(itr->riid.c_str(), eventProc, subscription) && eventProc != nullptr)
        {
            SciterDetachEventHandler((HELEMENT)(SCITER_ELEMENT)itr->Element, (::LPELEMENT_EVENT_PROC)eventProc, handler);
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
    auto eventHandler = std::make_shared<EventHandler>(m_sciter, element, interfacePtr, subscription);
    if (eventHandler.get())
    {
        SCDOM_RESULT hr = SciterAttachEventHandler((HELEMENT)element, (::LPELEMENT_EVENT_PROC)eventProc, eventHandler.get());
        if (hr == SCDOM_OK)
        {
            m_eventSinks.push_back(RegisteredSink(element, riid, interfacePtr, std::move(eventHandler)));
            return true;
        }
    }
    return false;
}

bool SciterWindow::HasHandler(SCITER_ELEMENT element, const char * riid, void * interfacePtr) const
{
    if (m_destroyed || riid == nullptr)
    {
        return false;    
    }
    for (const RegisteredSink & sink : m_eventSinks)
    {
        if ((SCITER_ELEMENT)sink.Element == element && sink.Interface == interfacePtr && sink.riid == riid)
        {
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

    EventSinks::iterator iter = m_eventSinks.begin();
    for (; iter != m_eventSinks.end(); ++iter)
    {
        if ((SCITER_ELEMENT)iter->Element == Element && iter->Interface == interfacePtr && iter->riid == riid)
        {
            break;
        }
    }
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
        if (result)
        {
            m_eventSinks.erase(iter);
        }
    }
    return result;
}

void SciterWindow::Bind()
{
    if (m_hWnd && !m_bound)
    {
        m_bound = true;
        SciterSetCallback((SciterHWINDOW)m_hWnd, (LPSciterHostCallback)SciterCallback, this);
    }
}

#if defined(__linux__)
int sui_callback SciterWindow::LinuxWindowEvent(void * tag, SCITER_ELEMENT /*element*/, uint32_t eventGroup, void * params)
{
    if (eventGroup == SUBSCRIPTIONS_REQUEST && params != nullptr)
    {
        *static_cast<UINT *>(params) = HANDLE_BEHAVIOR_EVENT;
        return true;
    }
    if (eventGroup == HANDLE_INITIALIZATION)
    {
        return true;
    }
    if (eventGroup != HANDLE_BEHAVIOR_EVENT || params == nullptr)
    {
        return false;
    }
    auto * event = static_cast<BEHAVIOR_EVENT_PARAMS *>(params);
    auto * window = static_cast<SciterWindow *>(tag);
    if (window == nullptr || event->cmd != CUSTOM || event->name == nullptr ||
        sui_wcsicmp(event->name, SUI_WSTR("sciterui-close-request")) != 0)
    {
        return false;
    }
    const auto keepAlive = window->shared_from_this();
    // Only the queued close may proceed synchronously. User requests first
    // honor the application's sinks and leave destruction to the engine loop.
    const bool closing = window->m_destroyed;
    if (!closing)
    {
        window->Destroy();
    }
    event->data = sciter::value(closing);
    return true;
}
#endif

bool SciterWindow::LoadHtml(const char * url)
{
    Bind();
    sui_ustring loadUrl = stdstr_f(sui_strnicmp(url, "file://", 7) == 0 ? "%s" : "file://%s", url).ToUTF16();
    if (!::SciterLoadFile((SciterHWINDOW)m_hWnd, loadUrl.c_str()))
    {
        return false;
    }
#if defined(__linux__)
    if (SciterWindowAttachEventHandler((SciterHWINDOW)m_hWnd,
            (::LPELEMENT_EVENT_PROC)LinuxWindowEvent, this,
            HANDLE_BEHAVIOR_EVENT) != SCDOM_OK)
    {
        return false;
    }
    // Use the documented closerequest cancellation API. Native close events
    // can run during paint/timer dispatch, before the wrapper may be destroyed.
    const WCHAR closeHandler[] = u"Window.this.on('closerequest', event => {"
        u"const request = new Event('sciterui-close-request');"
        u"request.data = false; Window.this.dispatchEvent(request);"
        u"if (!request.data) event.preventDefault();"
        u"});";
    sciter::value result;
    return ::SciterEval((SciterHWINDOW)m_hWnd, closeHandler,
        sizeof(closeHandler) / sizeof(WCHAR) - 1, &result) != FALSE;
#else
    return true;
#endif
}

bool SciterWindow::GetEventProc(const char * riid, LPELEMENT_EVENT_PROC & eventProc, uint32_t & subscription)
{
    if (strcmp(IID_ICLICKSINK, riid) == 0)
    {
        eventProc = &EventHandler::Dispatch<&EventHandler::OnClick>;
        subscription = HANDLE_MOUSE | HANDLE_BEHAVIOR_EVENT;
    }
    else if (strcmp(IID_IDBLCLICKSINK, riid) == 0)
    {
        eventProc = &EventHandler::Dispatch<&EventHandler::OnDoubleClick>;
        subscription = HANDLE_MOUSE;
    }
    else if (strcmp(IID_ITIMERSINK, riid) == 0)
    {
        eventProc = &EventHandler::Dispatch<&EventHandler::OnTimer>;
        subscription = HANDLE_TIMER;
    }
    else if (strcmp(IID_IMOUSEUPDOWNSINK, riid) == 0)
    {
        eventProc = &EventHandler::Dispatch<&EventHandler::OnMouseUpDown>;
        subscription = HANDLE_MOUSE;
    }
    else if (strcmp(IID_ICONTEXTMENUSINK, riid) == 0)
    {
        eventProc = &EventHandler::Dispatch<&EventHandler::OnContextMenu>;
        subscription = HANDLE_MOUSE;
    }
    else if (strcmp(IID_IMOUSEMOVESINK, riid) == 0)
    {
        eventProc = &EventHandler::Dispatch<&EventHandler::OnMouseMove>;
        subscription = HANDLE_MOUSE;
    }
    else if (strcmp(IID_IKEYSINK, riid) == 0)
    {
        eventProc = &EventHandler::Dispatch<&EventHandler::OnKey>;
        subscription = HANDLE_KEY;
    }
    else if (strcmp(IID_IRESIZESINK, riid) == 0)
    {
        eventProc = &EventHandler::Dispatch<&EventHandler::OnResize>;
        subscription = HANDLE_SIZE;
    }
    else if (strcmp(IID_FORWARD_BEHAVIOUR, riid) == 0)
    {
        eventProc = &EventHandler::Dispatch<&EventHandler::OnForwardBehavior>;
        subscription = HANDLE_BEHAVIOR_EVENT;
    }
    else if (strcmp(IID_ISTATECHANGESINK, riid) == 0)
    {
        eventProc = &EventHandler::Dispatch<&EventHandler::OnStateChange>;
        subscription = HANDLE_BEHAVIOR_EVENT;
    }
    else if (strcmp(IID_EVENTSINK, riid) == 0)
    {
        eventProc = &EventHandler::Dispatch<&EventHandler::OnEventSink>;
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
#endif
}

int64_t SciterWindow::HandleNotification(LPSCITER_CALLBACK_NOTIFICATION pnm)
{
    if (pnm == nullptr)
    {
        return 0;
    }
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

int64_t SciterWindow::OnLoadData(LPSCN_LOAD_DATA pnmld)
{
    if (pnmld == nullptr)
    {
        return LOAD_DISCARD;
    }
    if (pnmld->uri &&
        pnmld->uri[0] == u'd' &&
        pnmld->uri[1] == u'a' &&
        pnmld->uri[2] == u't' &&
        pnmld->uri[3] == u'a' &&
        pnmld->uri[4] == u':')
    {
        return LOAD_OK;
    }

    ResourceManager & manager = m_sciter.GetResourceManager();
    std::unique_ptr<uint8_t[]> data;
    uint32_t dataSize = 0;
    if (!manager.LoadResource(pnmld->uri, data, dataSize))
    {
        return LOAD_DISCARD;
    }
    ::SciterDataReady((SciterHWINDOW)pnmld->hwnd, pnmld->uri, data.get(), dataSize);
    return LOAD_OK;
}


int64_t SciterWindow::OnAttachBehavior(LPSCN_ATTACH_BEHAVIOR pnmld)
{
    return m_sciter.AttachWidget((Sciter::LPSCN_ATTACH_BEHAVIOR)pnmld);
}

int64_t SciterWindow::OnEngineDestroyed(void)
{
#ifdef __APPLE__
    DetachMacOSWindowTerminationObserver(m_hWnd);
#endif
    if (!m_destroyed)
    {
        m_destroyed = true;
#if   defined(__linux__)
        // SC_ENGINE_DESTROYED arrives after the native engine has gone.
        // Calling WindowExec here can access an invalid Wayland window.
#else
        ::SciterWindowExec((SciterHWINDOW)m_hWnd, SCITER_WINDOW_SET_STATE, SCITER_WINDOW_STATE_HIDDEN, 0);
#endif
#ifdef WIN32
        if (m_hParent != nullptr)
        {
            EnableWindow((HWND)m_hParent, m_parentEnabled ? TRUE : FALSE);
        }
#endif
    }
    m_eventSinks.clear();
    m_onCloseSink.clear();

    WinDestroySinks sinks = m_onDestroySink;
    for (WinDestroySinks::const_iterator itr = sinks.begin(); itr != sinks.end(); itr++)
    {
        if (m_onDestroySink.find(*itr) == m_onDestroySink.end())
        {
            continue;
        }
        (*itr)->OnWindowDestroy(m_hWnd);
    }
    m_onDestroySink.clear();
    m_hWnd = nullptr;
    m_sciter.WindowDestroyed(this);
    return 0;
}

UINT SciterWindow::SciterCallback(LPSCITER_CALLBACK_NOTIFICATION pnm, LPVOID param)
{
    SciterWindow * Self = (SciterWindow *)param;
    if (Self == nullptr)
    {
        return 0;
    }
    const auto keepAlive = Self->shared_from_this();
    return (UINT)Self->HandleNotification(pnm);
}
} // namespace SciterUI
