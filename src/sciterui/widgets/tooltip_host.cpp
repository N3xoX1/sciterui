#include <sciter_handler.h>
#include <sciter_element.h>
#include <widgets/tooltip_host.h>
#include <map>
#include "std_string.h"

namespace SciterUI
{

class WidgetToolTipHost :
    public std::enable_shared_from_this<WidgetToolTipHost>,
    public IMouseMoveSink,
    public ITimerSink,
    public IWidget
{
    typedef std::map<IWidget *, std::shared_ptr<WidgetToolTipHost>> ToolTips;
    
    enum
    {
        TIMER_ID_HIDETOOLTIP = 0xFD,
        TIMER_ID_MOVETOOLTIP = 0xFE,
        TIMER_ID_SHOWTOOLTIP = 0xFF,
        TIMER_INTERVAL_HIDETOOLTIP = 200,
        TIMER_INTERVAL_MOVETOOLTIP = 300,
        TIMER_INTERVAL_SHOWTOOLTIP = 400,
    };

public:
    static bool Register(ISciterUI & SciterUI);

private:
    WidgetToolTipHost() = delete;
    WidgetToolTipHost(const WidgetToolTipHost &) = delete;
    WidgetToolTipHost& operator=(const WidgetToolTipHost &) = delete;

    WidgetToolTipHost(ISciterUI & SciterUI);

    void DisplayPopup(void);

    //IMouseMoveSink
    bool OnMouseMove(SCITER_ELEMENT element, SCITER_ELEMENT source, uint32_t x, uint32_t y) override;

    //ITimerSink
    bool OnTimer(SCITER_ELEMENT Element, uint32_t* TimerId) override;

    //IWidget
    void Attached(SCITER_ELEMENT element, IBaseElement* baseElement) override;
    void Detached(SCITER_ELEMENT element) override;
    std::shared_ptr<void> GetInterface(const char* riid) override;

    static IWidget * sui_callback CreateWidget(ISciterUI & SciterUI);
    static void sui_callback ReleaseWidget(IWidget * widget);

    static ToolTips m_instances;

    SciterElement m_host;
    SciterElement m_lastSource;
    SciterElement m_anchor;
    SciterElement m_popup;
    SCITER_POINT m_mousePos;
    std::string m_toolTipText;
    bool m_tooltipVisible;
    ISciterUI & m_sciterUI;
};

WidgetToolTipHost::ToolTips WidgetToolTipHost::m_instances;

WidgetToolTipHost::WidgetToolTipHost(ISciterUI & sciterUI) :
    m_sciterUI(sciterUI),
    m_tooltipVisible(false)
{
    m_mousePos.x = 0;
    m_mousePos.y = 0;
}

void WidgetToolTipHost::DisplayPopup(void)
{
    SciterElement menu = SciterElement(m_host).FindFirst("[window-state=\"shown\"]");

    SCITER_POINT sciterMousePos;
    sciterMousePos.x = m_mousePos.x + 5;
    sciterMousePos.y = m_mousePos.y + 15;
    m_sciterUI.PopupShowAt(m_popup, sciterMousePos, 7);
    m_tooltipVisible = true;
}

bool WidgetToolTipHost::OnMouseMove(SCITER_ELEMENT /*element*/, SCITER_ELEMENT source, uint32_t x, uint32_t y)
{
    m_mousePos.x = x;
    m_mousePos.y = y;
    if (m_popup.IsValid())
    {
        m_host.SetTimer(TIMER_INTERVAL_MOVETOOLTIP, (uint32_t*)TIMER_ID_MOVETOOLTIP);
    }
    if (m_lastSource == source)
    {
        return false;
    }
    m_lastSource = source;
    SciterElement anchor = source;
    std::string tooltipText;
    while (anchor.IsValid())
    {
        std::string forElement = anchor.GetAttribute("for");
        if (!forElement.empty())
        {
            SciterElement targetAnchor = SciterElement(m_host.GetElementByID(forElement.c_str()));
            if (targetAnchor.IsValid())
            {
                anchor = targetAnchor;
            }
        }
        tooltipText = anchor.GetAttribute("tooltip");
        if (!tooltipText.empty())
        {
            anchor.RemoveAttribute("tooltip");
            anchor.SetAttribute("data-tooltip", tooltipText.c_str());
        }
        else
        {
            tooltipText = anchor.GetAttribute("data-tooltip");
        }
        if (!tooltipText.empty())
        {
            break;
        }
        anchor = anchor.GetParent();
    }

    if (anchor != nullptr && m_anchor == anchor)
    {
        return false;
    }
    if (tooltipText.empty())
    {
        if (m_anchor != nullptr)
        {
            m_host.SetTimer(TIMER_INTERVAL_HIDETOOLTIP, (uint32_t*)TIMER_ID_HIDETOOLTIP);
            m_host.SetTimer(0, (uint32_t*)TIMER_ID_SHOWTOOLTIP);
            m_anchor = nullptr;
        }
        return false;
    }
    m_anchor = anchor;
    m_toolTipText = tooltipText;

    size_t pos = 0;
    while ((pos = m_toolTipText.find("\\n", pos)) != std::string::npos)
    {
        m_toolTipText.replace(pos, 2, "<br>");
        pos += 4;
    }

    m_host.SetTimer(0, (uint32_t*)TIMER_ID_HIDETOOLTIP);
    if (m_popup.IsValid())
    {
        m_popup.SetHTML((const uint8_t*)m_toolTipText.data(), m_toolTipText.size());
        DisplayPopup();
    }
    else
    {
        m_host.SetTimer(TIMER_INTERVAL_SHOWTOOLTIP, (uint32_t*)TIMER_ID_SHOWTOOLTIP);
    }
    return true;
}

bool WidgetToolTipHost::OnTimer(SCITER_ELEMENT /*Element*/, uint32_t* TimerId)
{
    if (TimerId == (uint32_t*)TIMER_ID_SHOWTOOLTIP)
    {
        SciterElement currentPopup = m_host.FindFirst(":popup");
        if (currentPopup.IsValid())
        {
            return false;
        }

        m_host.SetTimer(0, (uint32_t*)TIMER_ID_SHOWTOOLTIP);
        m_popup.Create("popup", "");
        m_popup.SetAttribute("class", "tooltip");
        m_host.Insert(m_popup, m_host.GetChildCount());
        m_popup.SetHTML((const uint8_t*)m_toolTipText.data(), m_toolTipText.size());
        DisplayPopup();
    }
    if (TimerId == (uint32_t*)TIMER_ID_MOVETOOLTIP)
    {
        m_host.SetTimer(0, (uint32_t*)TIMER_ID_MOVETOOLTIP);
        if (m_tooltipVisible && (m_popup.GetState() & SciterElement::STATE_POPUP) != 0)
        {
            DisplayPopup();
        }
    }
    if (TimerId == (uint32_t*)TIMER_ID_HIDETOOLTIP)
    {
        m_sciterUI.PopupHide(m_popup);
        m_tooltipVisible = false;
    }
    return false;
}

void WidgetToolTipHost::Attached(SCITER_ELEMENT element, IBaseElement * /*baseElement*/)
{
    m_host = element;
    m_sciterUI.AttachHandler(element, IID_IMOUSEMOVESINK, (IMouseMoveSink*)this);
    m_sciterUI.AttachHandler(element, IID_ITIMERSINK, (ITimerSink*)this);
}

void WidgetToolTipHost::Detached(SCITER_ELEMENT /*element*/)
{
    m_sciterUI.DetachHandler(m_host, IID_IMOUSEMOVESINK, (IMouseMoveSink*)this);
    m_sciterUI.DetachHandler(m_host, IID_ITIMERSINK, (ITimerSink*)this);
    m_host = nullptr;
    m_lastSource = nullptr;
    m_anchor = nullptr;
}

std::shared_ptr<void> WidgetToolTipHost::GetInterface(const char * /*riid*/)
{
    return nullptr;
}

bool WidgetToolTipHost::Register(ISciterUI & SciterUI)
{
    const char * WidgetCss =
        "[tooltip-host] {"
        "    behavior: tooltip-host;"
        "}";
    return SciterUI.RegisterWidgetType("tooltip-host", WidgetToolTipHost::CreateWidget, WidgetToolTipHost::ReleaseWidget, WidgetCss);
}

IWidget * sui_callback WidgetToolTipHost::CreateWidget(ISciterUI & sciterUI)
{
    std::shared_ptr<WidgetToolTipHost> instance(new WidgetToolTipHost(sciterUI));
    IWidget * widget = (IWidget*)instance.get();
    WidgetToolTipHost * tooltip = instance.get();
    m_instances.insert(ToolTips::value_type(tooltip, std::move(instance)));
    return widget;
}

void sui_callback WidgetToolTipHost::ReleaseWidget(IWidget * widget)
{
    ToolTips::iterator it = m_instances.find(widget);
    if (it != m_instances.end())
    {
        m_instances.erase(it);
    }
}

}

bool Register_WidgetToolTipHost(ISciterUI & sciterUI)
{
    return SciterUI::WidgetToolTipHost::Register(sciterUI);
}