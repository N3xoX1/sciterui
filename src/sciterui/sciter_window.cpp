#include "sciter_window.h"
#include "event_handler.h"
#include "sciter.h"
#include "sciter_dpi.h"
#include "sciter_handler_internal.h"
#include "std_string.h"
#include "sciter_hwindow.h"
#if defined(__linux__)
#include "x11_host.h"
#endif
#include <sciter_element.h>
#include <sciter_handler.h>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <stdint.h>

#undef max
#undef min

namespace SciterUI
{

namespace
{

SciterHWINDOW EngineHandle(const SciterWindow & window)
{
#if defined(__linux__)
    return (SciterHWINDOW)const_cast<SciterWindow *>(&window);
#else
    return (SciterHWINDOW)window.GetHandle();
#endif
}

#if defined(__linux__)
bool IsNameChar(unsigned char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_';
}

bool UriEndsWith(const sui_wchar * uri, const char * suffix)
{
    if (uri == nullptr || suffix == nullptr)
    {
        return false;
    }
    size_t length = 0;
    while (uri[length] != 0)
    {
        ++length;
    }
    const size_t suffixLength = std::strlen(suffix);
    if (length < suffixLength)
    {
        return false;
    }
    for (size_t i = 0; i < suffixLength; ++i)
    {
        unsigned char got = static_cast<unsigned char>(uri[length - suffixLength + i]);
        unsigned char expect = static_cast<unsigned char>(suffix[i]);
        if (got >= 'A' && got <= 'Z')
        {
            got = static_cast<unsigned char>(got - 'A' + 'a');
        }
        if (expect >= 'A' && expect <= 'Z')
        {
            expect = static_cast<unsigned char>(expect - 'A' + 'a');
        }
        if (got != expect)
        {
            return false;
        }
    }
    return true;
}

bool TakeAttribute(std::string & tag, const char * name, std::string * value)
{
    const size_t nameLength = std::strlen(name);
    for (size_t pos = 0; pos < tag.size(); ++pos)
    {
        if (tag.compare(pos, nameLength, name) != 0)
        {
            continue;
        }
        const bool boundaryBefore = pos == 0 || !IsNameChar(static_cast<unsigned char>(tag[pos - 1]));
        const size_t after = pos + nameLength;
        const bool boundaryAfter = after >= tag.size() || !IsNameChar(static_cast<unsigned char>(tag[after]));
        if (!boundaryBefore || !boundaryAfter)
        {
            continue;
        }
        size_t end = after;
        std::string captured;
        if (end < tag.size() && tag[end] == '=')
        {
            ++end;
            if (end < tag.size() && (tag[end] == '"' || tag[end] == '\''))
            {
                const char quote = tag[end++];
                const size_t valueStart = end;
                while (end < tag.size() && tag[end] != quote)
                {
                    ++end;
                }
                captured = tag.substr(valueStart, end - valueStart);
                if (end < tag.size())
                {
                    ++end;
                }
            }
            else
            {
                const size_t valueStart = end;
                while (end < tag.size() && tag[end] != ' ' && tag[end] != '\t' && tag[end] != '\n' && tag[end] != '\r')
                {
                    ++end;
                }
                captured = tag.substr(valueStart, end - valueStart);
            }
        }
        size_t start = pos;
        if (start > 0 && (tag[start - 1] == ' ' || tag[start - 1] == '\t' || tag[start - 1] == '\n' || tag[start - 1] == '\r'))
        {
            --start;
        }
        tag.erase(start, end - start);
        if (value != nullptr)
        {
            *value = captured;
        }
        return true;
    }
    return false;
}

bool StripNativeWindowAttributes(std::string & html, int & minWidth, int & minHeight, std::string & icon, bool & resizable)
{
    const size_t start = html.find("<html");
    if (start == std::string::npos)
    {
        return false;
    }
    if (start + 5 < html.size() && IsNameChar(static_cast<unsigned char>(html[start + 5])))
    {
        return false;
    }
    bool inQuote = false;
    char quote = 0;
    size_t end = start;
    for (; end < html.size(); ++end)
    {
        const char c = html[end];
        if (inQuote)
        {
            if (c == quote)
            {
                inQuote = false;
            }
        }
        else if (c == '"' || c == '\'')
        {
            inQuote = true;
            quote = c;
        }
        else if (c == '>')
        {
            break;
        }
    }
    if (end >= html.size())
    {
        return false;
    }
    std::string tag = html.substr(start, end - start);
    std::string value;
    bool changed = false;
    if (TakeAttribute(tag, "window-min-width", &value))
    {
        minWidth = std::atoi(value.c_str());
        changed = true;
    }
    if (TakeAttribute(tag, "window-min-height", &value))
    {
        minHeight = std::atoi(value.c_str());
        changed = true;
    }
    if (TakeAttribute(tag, "window-icon", &value))
    {
        icon = value;
        changed = true;
    }
    if (TakeAttribute(tag, "window-resizable", &value))
    {
        resizable = value != "false" && value != "0";
        changed = true;
    }
    const char * names[] = {
        "window-blurbehind",
        "window-max-width",
        "window-max-height",
        "window-minimizable",
        "window-maximizable",
    };
    for (const char * name : names)
    {
        if (TakeAttribute(tag, name, nullptr))
        {
            changed = true;
        }
    }
    if (changed)
    {
        html.replace(start, end - start, tag);
    }
    return changed;
}

void InjectWidgetCss(std::string & html, const std::string & css)
{
    if (css.empty() || html.find("<style id=\"sciterui-widgets\">") != std::string::npos)
    {
        return;
    }
    const std::string block = "<style id=\"sciterui-widgets\">" + css + "</style>";
    const size_t head = html.find("<head");
    if (head != std::string::npos)
    {
        const size_t end = html.find('>', head);
        if (end != std::string::npos)
        {
            html.insert(end + 1, block);
            return;
        }
    }
    html.insert(0, block);
}
#endif
} // namespace

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
#if defined(__linux__)
    ,
    m_documentMinWidth(0),
    m_documentMinHeight(0),
    m_documentResizable(false)
#endif
{
}

SciterWindow::~SciterWindow()
{
}

void SciterWindow::Show()
{
#if defined(__linux__)
    X11Host::Instance().Show(*this);
#else
    ::SciterWindowExec((SciterHWINDOW)m_hWnd, SCITER_WINDOW_SET_STATE, SCITER_WINDOW_STATE_SHOWN, 0);
#endif
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
#elif defined(__linux__)
    m_hWnd = X11Host::Instance().Create(*this, parentWinow, x, y, width, height, flags, startHidden);
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
#if !defined(__linux__)
            m_parentState = (int)::SciterWindowExec((SciterHWINDOW)parentWinow, SCITER_WINDOW_GET_STATE, 0, 0);
#endif
#ifdef WIN32
            m_parentEnabled = IsWindowEnabled((HWND)parentWinow) != FALSE;
            EnableWindow((HWND)parentWinow, FALSE);
#endif
        }
        SciterSetOption(EngineHandle(*this), SCITER_SET_SCRIPT_RUNTIME_FEATURES, ALLOW_FILE_IO | ALLOW_SOCKET_IO | ALLOW_EVAL | ALLOW_SYSINFO);

        m_sciter.WindowCreated(this);
        if (!LoadHtml(htmlFile))
        {
            SetDestroyed();
            if (m_hWnd != nullptr)
            {
#ifdef WIN32
                DestroyWindow((HWND)m_hWnd);
#elif defined(__linux__)
                X11Host::Instance().Abandon(*this);
#else
                ::SciterWindowExec((SciterHWINDOW)m_hWnd, SCITER_WINDOW_SET_STATE, SCITER_WINDOW_STATE_CLOSED, TRUE);
#endif
                m_hWnd = nullptr;
            }
            m_sciter.WindowDestroyed(this);
            return false;
        }
#if defined(__linux__)
        ApplyDocumentChrome();
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
    X11Host::Instance().Center(*this);
#endif
}

void SciterWindow::FixMinSize()
{
    if (m_hWnd == nullptr)
    {
        return;
    }

#if defined(__linux__)
    X11Host::Instance().FixMinSize(*this, m_layoutWidth, m_layoutHeight);
#else
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
#endif
#endif
}

HWINDOW SciterWindow::GetHandle() const
{
    return m_hWnd;
}

uint32_t SciterWindow::GetMinWidth() const
{
    return SciterGetMinWidth(EngineHandle(*this));
}

uint32_t SciterWindow::GetMinHeight(uint32_t width) const
{
    return SciterGetMinHeight(EngineHandle(*this), width);
}

SCITER_ELEMENT SciterWindow::GetRootElement(void) const
{
    HELEMENT h = 0;
    SciterGetRootElement(EngineHandle(*this), &h);
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
    X11Host::Instance().Close(*this);
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
    X11Host::Instance().RunModal(this);
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
    if (m_hParent != nullptr)
    {
        if (m_hWnd != nullptr)
        {
#if defined(__linux__)
            X11Host::Instance().Hide(*this);
#else
            ::SciterWindowExec((SciterHWINDOW)m_hWnd, SCITER_WINDOW_SET_STATE, SCITER_WINDOW_STATE_HIDDEN, 0);
#endif
        }
#ifdef WIN32
        EnableWindow((HWND)m_hParent, m_parentEnabled ? TRUE : FALSE);
#endif
#if !defined(__linux__)
        if (m_parentEnabled &&
            (m_parentState == SCITER_WINDOW_STATE_SHOWN ||
             m_parentState == SCITER_WINDOW_STATE_MAXIMIZED ||
             m_parentState == SCITER_WINDOW_STATE_FULL_SCREEN))
        {
            ::SciterWindowExec((SciterHWINDOW)m_hParent, SCITER_WINDOW_SET_STATE, (UINT_PTR)m_parentState, 0);
            ::SciterWindowExec((SciterHWINDOW)m_hParent, SCITER_WINDOW_ACTIVATE, TRUE, 0);
        }
#endif
    }
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
        SciterSetCallback(EngineHandle(*this), (LPSciterHostCallback)SciterCallback, this);
    }
}

bool SciterWindow::LoadHtml(const char * url)
{
    Bind();
    sui_ustring loadUrl = stdstr_f(sui_strnicmp(url, "file://", 7) == 0 ? "%s" : "file://%s", url).ToUTF16();
    if (!::SciterLoadFile(EngineHandle(*this), loadUrl.c_str()))
    {
        return false;
    }
    return true;
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
    else if (strcmp(IID_ICONTEXTMENUSINK, riid) == 0)
    {
        eventProc = &EventHandler::ContextMenuHandler;
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
#elif defined(__linux__)
    X11Host::Instance().ApplySize(*this, x, y, width, height);
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
#if defined(__linux__)
    case SC_INVALIDATE_RECT:
    {
        const SCN_INVALIDATE_RECT * invalidated = reinterpret_cast<const SCN_INVALIDATE_RECT *>(pnm);
        X11Host::Instance().Invalidate(m_hWnd, invalidated->invalidRect.left, invalidated->invalidRect.top, invalidated->invalidRect.right, invalidated->invalidRect.bottom);
        return 0;
    }
    case SC_SET_CURSOR:
    {
        const SCN_SET_CURSOR * cursor = reinterpret_cast<const SCN_SET_CURSOR *>(pnm);
        X11Host::Instance().SetCursor(m_hWnd, cursor->cursorId);
        return 0;
    }
#endif
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
#if defined(__linux__)
    if (UriEndsWith(pnmld->uri, ".html") || UriEndsWith(pnmld->uri, ".htm"))
    {
        std::string html(reinterpret_cast<const char *>(data.get()), dataSize);
        int minWidth = m_documentMinWidth;
        int minHeight = m_documentMinHeight;
        std::string icon;
        bool resizable = m_documentResizable;
        if (StripNativeWindowAttributes(html, minWidth, minHeight, icon, resizable))
        {
            m_documentMinWidth = minWidth;
            m_documentMinHeight = minHeight;
            m_documentResizable = resizable;
            if (!icon.empty())
            {
                m_documentIcon = icon;
            }
        }
        InjectWidgetCss(html, m_sciter.WidgetCss());
        ::SciterDataReady((SciterHWINDOW)pnmld->hwnd, pnmld->uri, reinterpret_cast<const unsigned char *>(html.data()), static_cast<UINT>(html.size()));
        return LOAD_OK;
    }
#endif
    ::SciterDataReady((SciterHWINDOW)pnmld->hwnd, pnmld->uri, data.get(), dataSize);
    return LOAD_OK;
}

#if defined(__linux__)
void SciterWindow::ApplyDocumentChrome()
{
    SciterElement root(GetRootElement());
    if (!root.IsValid())
    {
        return;
    }
    int minWidth = std::atoi(root.GetAttribute("window-min-width").c_str());
    int minHeight = std::atoi(root.GetAttribute("window-min-height").c_str());
    if (minWidth <= 0)
    {
        minWidth = m_documentMinWidth;
    }
    if (minHeight <= 0)
    {
        minHeight = m_documentMinHeight;
    }
    if (minWidth > 0 || minHeight > 0)
    {
        X11Host::Instance().SetDocumentMinSize(*this, minWidth, minHeight);
    }
    X11Host::Instance().SetResizable(*this, m_documentResizable);
    SciterElement title(root.FindFirst("title"));
    if (title.IsValid())
    {
        std::string text = title.GetHTML(false);
        const size_t start = text.find_first_not_of(" \t\r\n");
        const size_t end = text.find_last_not_of(" \t\r\n");
        if (start != std::string::npos && end != std::string::npos && text.find('<') == std::string::npos)
        {
            text = text.substr(start, end - start + 1);
            X11Host::Instance().SetTitle(*this, text.c_str());
        }
    }
    if (!m_documentIcon.empty())
    {
        std::vector<uint8_t> png;
        if (m_sciter.LoadResource(m_documentIcon.c_str(), png) && !png.empty())
        {
            X11Host::Instance().SetIcon(*this, png.data(), static_cast<uint32_t>(png.size()));
        }
    }
}
#endif

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
#if defined(__linux__)
        X11Host::Instance().Hide(*this);
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
