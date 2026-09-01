#include "debug_break.h"
#include "sciter.h"
#include "sciter_element.h"
#include "sciter_window.h"
#include "std_string.h"

#include "sciter_hwindow.h"

namespace SciterUI
{

Sciter::Sciter(const char * languageDir) :
    m_resourceManager(languageDir),
    m_nextWidgetId(1)
{
}

bool Sciter::Initialize(const char * baseLanguage, const char * currentLanguage, bool /*console*/)
{
    if (!m_resourceManager.Initialize(baseLanguage, currentLanguage))
    {
        return false;
    }
    SciterExec(SCITER_APP_INIT, (UINT_PTR)0, (UINT_PTR) nullptr);
    SciterSetOption(NULL, SCITER_SET_SCRIPT_RUNTIME_FEATURES, ALLOW_FILE_IO | ALLOW_SOCKET_IO | ALLOW_EVAL | ALLOW_SYSINFO);
    return true;
}

void Sciter::UpdateWindow(HWINDOW hwnd)
{
    SciterUpdateWindow((SciterHWINDOW)hwnd);
}

bool Sciter::AttachHandler(SCITER_ELEMENT elemHandle, const char * riid, void * pinterface)
{
    if (elemHandle == nullptr || riid == nullptr || pinterface == nullptr)
    {
        return false;
    }
    HWINDOW hWnd = SciterElement(elemHandle).GetElementHwnd(true);
    SciterWindow * window = FindSciterWindow(hWnd);
    if (window == nullptr)
    {
        return false;
    }
    return window->AttachHandler(elemHandle, riid, pinterface);
}

bool Sciter::DetachHandler(SCITER_ELEMENT elemHandle, const char * riid, void * pinterface)
{
    if (elemHandle == nullptr || pinterface == nullptr)
    {
        return false;
    }
    HWINDOW hWnd = SciterElement(elemHandle).GetElementHwnd(true);
    SciterWindow * window = FindSciterWindow(hWnd);
    if (window == nullptr)
    {
        return false;
    }
    return window->DetachHandler(elemHandle, riid, pinterface);
}

std::shared_ptr<void> Sciter::GetElementInterface(SCITER_ELEMENT he, const char * riid)
{
    if (he == nullptr)
    {
        return nullptr;
    }
    std::string widgetValue = SciterElement(he).GetAttribute("data-widget");
    if (widgetValue.size() == 0)
    {
        return nullptr;
    }
    int windgetId = std::stoi(widgetValue, nullptr, 10);
    ElementMap::iterator itr = m_elementBases.find(windgetId);
    if (itr != m_elementBases.end())
    {
        return itr->second->GetInterface(riid);
    }
    return nullptr;
}

void Sciter::WindowCreated(SciterWindow * window)
{
    m_windows.insert(window);
}

void Sciter::WindowDestroyed(SciterWindow * window)
{
    WindowSet::iterator itr = m_windows.find(window);
    if (itr != m_windows.end())
    {
        if (!window->GetDestroyed())
        {
            window->SetDestroyed();
        }
        m_windows.erase(itr);
    }
}

SciterWindow * Sciter::FindSciterWindow(HWINDOW hwnd) const
{
    for (WindowSet::const_iterator itr = m_windows.begin(); itr != m_windows.end(); itr++)
    {
        SciterWindow * window = *itr;
        if (window->GetHandle() == hwnd)
        {
            return window;
        }
    }
    return nullptr;
}

bool Sciter::SetElementHtmlFromResource(SCITER_ELEMENT elemHandle, const char * uri)
{
    SciterElement element(elemHandle);
    if (!element.IsValid() || uri == nullptr)
    {
        return false;
    }
    std::unique_ptr<uint8_t[]> Data;
    uint32_t Size = 0;
    if (!m_resourceManager.LoadResource(stdstr(uri).ToUTF16().c_str(), Data, Size))
    {
        element.SetHTML((uint8_t *)"", 0, SciterElement::SIH_REPLACE_CONTENT);
        return false;
    }
    element.SetHTML(Data.get(), Size, SciterElement::SIH_REPLACE_CONTENT);
    return true;
}

bool Sciter::LoadResource(const char * uri, std::vector<uint8_t> & out)
{
    out.clear();
    if (uri == nullptr)
    {
        return false;
    }
    std::unique_ptr<uint8_t[]> data;
    uint32_t size = 0;
    if (!m_resourceManager.LoadResource(stdstr(uri).ToUTF16().c_str(), data, size))
    {
        return false;
    }
    out.assign(data.get(), data.get() + static_cast<size_t>(size));
    return true;
}

bool Sciter::WindowCreate(HWINDOW parent, const char * baseHtml, int x, int y, int width, int height, unsigned int flags, ISciterWindow *& window)
{
    std::unique_ptr<SciterWindow> sciterWindow(new SciterWindow(*this));
    if (!sciterWindow->Create(parent, baseHtml, x, y, width, height, flags))
    {
        return false;
    }
    window = sciterWindow.get();
    m_CreatedWindows.emplace_back(std::move(sciterWindow));
    return true;
}

void Sciter::PopupShow(SCITER_ELEMENT hePopup, SCITER_ELEMENT heAnchor, uint32_t placement)
{
    SciterShowPopup((HELEMENT)hePopup, (HELEMENT)heAnchor, placement);
}

void Sciter::PopupShowAt(SCITER_ELEMENT hePopup, SCITER_POINT pos, uint32_t placement)
{
    POINT point;
    point.x = pos.x;
    point.y = pos.y;
    SciterShowPopupAt((HELEMENT)hePopup, point, placement);
}

void Sciter::PopupHide(SCITER_ELEMENT he)
{
    SciterHidePopup((HELEMENT)he);
}

bool Sciter::RegisterWidgetType(const char * name, tyCreateWidget createWidget, tyReleaseWidget releaseWidget, const char * widgetCss)
{
    if (name == nullptr || createWidget == nullptr)
    {
        return false;
    }
    WidgetMap::iterator iter = m_widgetFactory.find(name);
    if (iter != m_widgetFactory.end())
    {
        return false;
    }
    WidgetCallbackInfo widgetCallbackInfo;
    widgetCallbackInfo.sciter = this;
    widgetCallbackInfo.createWidget = createWidget;
    widgetCallbackInfo.releaseWidget = releaseWidget;
    m_widgetFactory.insert(WidgetMap::value_type(name, widgetCallbackInfo));
    if (widgetCss != nullptr && strlen(widgetCss) > 0)
    {
        if (!SciterAppendMasterCSS((LPCBYTE)widgetCss, (uint32_t)strlen(widgetCss)))
        {
            return false;
        }
    }
    return true;
}

uint32_t Sciter::AttachWidget(LPSCN_ATTACH_BEHAVIOR lpab)
{
    if (lpab == nullptr)
    {
        return 0;
    }
    WidgetMap::iterator itr = m_widgetFactory.find(lpab->behaviorName);
    if (itr == m_widgetFactory.end())
    {
        return 0;
    }
    IWidget * pWidget = (itr->second.createWidget)(*this);
    if (pWidget == nullptr)
    {
        return 0;
    }
    uint32_t widgetID = m_nextWidgetId++;
    SciterElement(lpab->element).SetAttribute("data-widget", stdstr_f("%d", widgetID).c_str());
    itr->second.widgets.insert(IWidgetMap::value_type(widgetID, pWidget));
    lpab->elementTag = (void *)&itr->second;
    lpab->elementProc = (ElementEventProc *)stAttachWidgetProc;
    return 1;
}

void Sciter::Run()
{
    SciterExec(SCITER_APP_LOOP, 0, 0);
}

void Sciter::Stop()
{
    SciterExec(SCITER_APP_STOP, 0, 0);
}

void Sciter::Shutdown()
{
    SciterExec(SCITER_APP_SHUTDOWN, 0, 0);
}

ResourceManager & Sciter::GetResourceManager(void)
{
    return m_resourceManager;
}

#ifdef WIN32
const std::wstring & Sciter::WindowClass()
{
    if (!m_windowClass.empty())
    {
        return m_windowClass;
    }
    m_windowClass = L"sciterui_window";
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = sizeof(Sciter *);
    wcex.hInstance = GetModuleHandle(nullptr);
    wcex.hIcon = nullptr;
    wcex.hCursor = nullptr;
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = m_windowClass.c_str();
    wcex.hIconSm = nullptr;

    RegisterClassExW(&wcex);
    return m_windowClass;
}

LRESULT CALLBACK Sciter::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_NCCREATE: {
        Sciter* pThis = (Sciter*)(((CREATESTRUCT*)lParam)->lpCreateParams);
        SetWindowLongPtr(hwnd, 0, (LONG_PTR)pThis);
        break;
    }
    case WM_ACTIVATE: {
        HELEMENT h = 0;
        SciterGetRootElement((SciterHWINDOW)hwnd, &h);
        SciterElement root(h);
        if (root.IsValid())
        {
            root.SetAttribute("data-active", wParam == WA_INACTIVE ? "false" : "true");
        }
        break;
    }
    case WM_GETMINMAXINFO: {
        HELEMENT h = 0;
        SciterGetRootElement((SciterHWINDOW)hwnd, &h);
        SciterElement root(h);
        if (root.IsValid())
        {
            int32_t minWidth = atoi(root.GetAttribute("window-min-width").c_str());
            int32_t minHeight = atoi(root.GetAttribute("window-min-height").c_str());
            if (minWidth != 0 && minHeight != 0)
            {
                LRESULT lr = DefWindowProc(hwnd, message, wParam, lParam);
                MINMAXINFO* pmmi = (MINMAXINFO*)lParam;
                pmmi->ptMinTrackSize.x = minWidth;
                pmmi->ptMinTrackSize.y = minHeight;
                return lr;
            }
        }
        break;
    }
    case WM_CLOSE: {
        Sciter * sciter = (Sciter *)GetWindowLongPtr(hwnd, 0);
        if (sciter != nullptr)
        {
            SciterWindow * window = sciter->FindSciterWindow(hwnd);
            if (window != nullptr && !window->QueryClose())
            {
                return 0;
            }
        }
        break;
    }
    }

    BOOL handled = FALSE;
    LRESULT lResult = ::SciterProcND(hwnd, message, wParam, lParam, &handled);
    if (handled)
    {
        return lResult;
    }
    return ::DefWindowProc(hwnd, message, wParam, lParam);
}
#endif

int Sciter::AttachWidgetProc(WidgetCallbackInfo * info, SCITER_ELEMENT he, uint32_t evtg, void * prms)
{
    if (info == nullptr)
    {
        return 0;
    }

    if (evtg == HANDLE_INITIALIZATION)
    {
        std::string widgetValue = SciterElement(he).GetAttribute("data-widget");
        if (widgetValue.size() == 0)
        {
            return 0;
        }
        int windgetId = std::stoi(widgetValue, nullptr, 10);
        IWidgetMap::const_iterator IWidgetIter = info->widgets.find(windgetId);
        if (IWidgetIter == info->widgets.end())
        {
            return 0;
        }

        INITIALIZATION_PARAMS * p = (INITIALIZATION_PARAMS *)prms;
        if (p->cmd == BEHAVIOR_DETACH)
        {
            ElementMap::iterator itr = m_elementBases.find(windgetId);
            if (itr != m_elementBases.end())
            {
                std::shared_ptr<BaseElement> elementBase = itr->second;
                if (elementBase != nullptr)
                {
                    IWidget * widget = IWidgetIter->second;
                    widget->Detached(he);
                    elementBase->RemoveWidget(widget);
                    
                    if (info->releaseWidget != nullptr)
                    {
                        info->releaseWidget(widget);
                    }
                }
                m_elementBases.erase(itr);
            }
        }
        else if (p->cmd == BEHAVIOR_ATTACH)
        {
            std::shared_ptr<BaseElement> elementBase;
            ElementMap::iterator BaseIter = m_elementBases.find(windgetId);
            if (BaseIter != m_elementBases.end())
            {
                elementBase = BaseIter->second;
            }
            else
            {
                std::pair<ElementMap::iterator, bool> ret = m_elementBases.emplace(ElementMap::value_type(windgetId, std::make_shared<BaseElement>(*this, he)));
                elementBase = ret.first->second;
            }

            if (elementBase != nullptr)
            {
                IWidget * widget = IWidgetIter->second;
                elementBase->AddWidget(widget);
                widget->Attached(he, elementBase.get());
            }
        }
        else
        {
            DebugBreak();
        }
    }
    return 0;
}

int Sciter::stAttachWidgetProc(void * tag, SCITER_ELEMENT he, uint32_t evtg, void * prms)
{
    WidgetCallbackInfo * info = (WidgetCallbackInfo *)tag;
    if (info != nullptr)
    {
        return info->sciter->AttachWidgetProc(info, he, evtg, prms);
    }
    return false;
}

} // namespace SciterUI
