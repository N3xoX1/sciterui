#include "std_string.h"
#include <cctype>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <sciter_element.h>
#include <sciter_handler.h>
#include <sciter_ui.h>
#include <widgets/menubar.h>

#include <value.h>
#include <sciter-x-behavior.h>
#include "sciter_hwindow.h"

class WidgetMenuBar :
    public std::enable_shared_from_this<WidgetMenuBar>,
    public IWidget,
    public IMenuBar,
    public IClickSink,
    public IMouseUpDownSink,
    public IMouseMoveSink,
    public IKeySink,
    public IEventSink,
    public ISciterElementCallback
{
    typedef std::map<IWidget *, std::shared_ptr<WidgetMenuBar>> MenuBars;
    typedef std::set<IMenuBarSink *> IMenuBarSinkSet;

public:
    static void Register(ISciterUI& sciterUI);

    //IMenuBar
    void SetMenuContent(MenuBarItemList& items) const;
    void AddSink(IMenuBarSink* sink);
    void RemoveSink(IMenuBarSink* sink);

private:
    WidgetMenuBar(ISciterUI& SciterUI);

    WidgetMenuBar(void) = delete;
    WidgetMenuBar(const WidgetMenuBar&) = delete;
    WidgetMenuBar& operator=(const WidgetMenuBar&) = delete;

    // IWidget
    void Attached(SCITER_ELEMENT element, IBaseElement* baseElement);
    void Detached(SCITER_ELEMENT element);
    std::shared_ptr<void>  GetInterface(const char* riid);

    // ISciterElementCallback
    bool OnSciterElement(SCITER_ELEMENT he);

    // IClickSink
    bool OnClick(SCITER_ELEMENT element, SCITER_ELEMENT source, uint32_t reason);

    // IMouseUpDownSink
    bool OnMouseDown(SCITER_ELEMENT element, SCITER_ELEMENT source, uint32_t x, uint32_t y) override;
    bool OnMouseUp(SCITER_ELEMENT element, SCITER_ELEMENT source, uint32_t x, uint32_t y) override;

    // IMouseMoveSink
    bool OnMouseMove(SCITER_ELEMENT element, SCITER_ELEMENT source, uint32_t x, uint32_t y);

    // IKeySink
    bool OnKeyDown(SCITER_ELEMENT element, SCITER_ELEMENT item, SciterKeys keyCode, uint32_t keyboardState) override;
    bool OnKeyUp(SCITER_ELEMENT element, SCITER_ELEMENT item, SciterKeys keyCode, uint32_t keyboardState) override;
    bool OnKeyChar(SCITER_ELEMENT element, SCITER_ELEMENT item, SciterKeys keyCode, uint32_t keyboardState) override;
    void SyncMenuMnemonicsAttribute();

    // IEventSink
    bool OnEvent(SCITER_ELEMENT element, SCITER_ELEMENT source, uint32_t event_code, uint64_t reason) override;

    void HideShownMenuPopup();
    void HideSubMenu();
    void ShowTopMenu(SciterElement topItem);
    void ShowSubMenu(SciterElement item);
    void NotifySinksMenuItem(int32_t id, SCITER_ELEMENT item) const;
    SciterElement FindTopLevelItem(SciterElement from) const;
    SciterElement FindPopupMenuItem(SciterElement from) const;
    SciterElement DirectChildMenu(const SciterElement & item) const;
    bool ElementIsUnder(const SciterElement & hit, const SciterElement & ancestor) const;
    static bool ElementIsUnderMainMenuWidget(const SciterElement & hit, const SciterElement & mainMenuElem);

    static std::string MenuItemHtml(const MenuBarItem & item, uint32_t indent);
    static IWidget * sui_callback CreateWidget(ISciterUI & sciterUI);
    static void sui_callback ReleaseWidget(IWidget * widget);

    static MenuBars m_instances;

    ISciterUI & m_sciterUI;
    IBaseElement * m_baseElement;
    SciterElement m_menuBarElem;
    SciterElement m_keySinkRoot;
    SciterElement m_openTopItem;
    SciterElement m_openTopMenu;
    SciterElement m_openSubItem;
    SciterElement m_openSubMenu;
    IMenuBarSinkSet m_sinks;
    bool m_leftAltDown;
    bool m_rightAltDown;
    bool m_menuActive;
};

WidgetMenuBar::MenuBars WidgetMenuBar::m_instances;

namespace
{

static const uint32_t kMenuItemClickEvent = (uint32_t)MENU_ITEM_CLICK;
static const uint32_t kPopupDismissedEvent = (uint32_t)POPUP_DISMISSED;
static const uint64_t kClickReasonByKey = (uint64_t)BY_KEY_CLICK;

enum class InWindowMenuPlacement
{
    BelowAnchor,
    RightOfAnchor,
};

#ifndef _WIN32
static void SizeInWindowMenuToContent(SciterElement menu)
{
    menu.Update(true);

    const SciterElement::RECT contentBox = menu.GetLocation(SciterElement::ROOT_RELATIVE | SciterElement::CONTENT_BOX);
    int32_t maxBottom = contentBox.top;
    int32_t maxRight = contentBox.left;
    const uint32_t childCount = menu.GetChildCount();
    for (uint32_t i = 0; i < childCount; ++i)
    {
        SciterElement child(menu.GetChild(i));
        if (!child.IsValid())
        {
            continue;
        }
        const SciterElement::RECT childBox = child.GetLocation(SciterElement::ROOT_RELATIVE | SciterElement::MARGIN_BOX);
        if (childBox.bottom > maxBottom)
        {
            maxBottom = childBox.bottom;
        }
        if (childBox.right > maxRight)
        {
            maxRight = childBox.right;
        }
    }

    const int32_t contentHeight = maxBottom - contentBox.top;
    const int32_t contentWidth = maxRight - contentBox.left;
    if (contentHeight > 0)
    {
        menu.SetStyleAttribute("height", SciterUI::stdstr_f("%dpx", contentHeight).c_str());
    }
    if (contentWidth > 0)
    {
        menu.SetStyleAttribute("min-width", SciterUI::stdstr_f("%dpx", contentWidth).c_str());
    }
}
#endif

static void HideMenu(SciterElement menu)
{
    if (!menu.IsValid())
    {
        return;
    }
    menu.RemoveClassName("menu-open");
#ifdef _WIN32
    menu.HidePopup();
#else
    menu.SetStyleAttribute("display", "none");
#endif
}

static void ShowMenu(ISciterUI & sciterUI, SciterElement menu, SciterElement anchor, InWindowMenuPlacement placement)
{
    if (!menu.IsValid())
    {
        return;
    }

    menu.SetStyleAttribute("popup-animation", "none");
#ifdef _WIN32
    const uint32_t popupPlacement = (placement == InWindowMenuPlacement::RightOfAnchor)
        ? (7u | (9u << 16))
        : 2u;
    sciterUI.PopupShow(menu, anchor.IsValid() ? (SCITER_ELEMENT)anchor : (SCITER_ELEMENT)menu, popupPlacement);
#else
    (void)sciterUI;
    menu.SetStyleAttribute("behavior", "none");
    menu.SetStyleAttribute("flow", "vertical");
    menu.SetStyleAttribute("width", "max-content");
    menu.SetStyleAttribute("height", "max-content");
    menu.SetStyleAttribute("position", "absolute");
    if (placement == InWindowMenuPlacement::BelowAnchor && anchor.IsValid())
    {
        const SciterElement::RECT box = anchor.GetLocation(SciterElement::SELF_RELATIVE | SciterElement::BORDER_BOX);
        menu.SetStyleAttribute("left", SciterUI::stdstr_f("%dpx", box.left).c_str());
        menu.SetStyleAttribute("top", SciterUI::stdstr_f("%dpx", box.bottom).c_str());
    }
    menu.SetStyleAttribute("display", "block");
    SizeInWindowMenuToContent(menu);
#endif
    menu.AddClassName("menu-open");
}

} // namespace

static std::string PrepareMenuTitleForMnemonics(const char * title)
{
    SciterUI::stdstr t(title ? title : "");
    t.Replace("...", "\xE2\x80\xA6");
    return std::string(t.c_str());
}

static void AppendMenuTitleHtml(const std::string & in, std::string & out, char & accesskey)
{
    accesskey = 0;
    for (size_t i = 0; i < in.size(); )
    {
        const unsigned char c = (unsigned char)in[i];
        if (c == '&' && i + 1 < in.size())
        {
            const unsigned char n = (unsigned char)in[i + 1];
            if (n == '&')
            {
                out += '&';
                i += 2;
                continue;
            }
            if (!accesskey && std::isalnum(n))
            {
                accesskey = (char)std::toupper(n);
                out += "<u>";
                out += (char)n;
                out += "</u>";
                i += 2;
                continue;
            }
            out += "&amp;";
            i += 1;
            continue;
        }
        if (c == '<')
        {
            out += "&lt;";
        }
        else if (c == '>')
        {
            out += "&gt;";
        }
        else
        {
            out += (char)c;
        }
        i += 1;
    }
}

static std::string MenuBarAcceleratorKeyLabel(uint32_t key)
{
    if (key >= (uint32_t)SCITER_KEY_A && key <= (uint32_t)SCITER_KEY_Z)
    {
        char buf[2] = { (char)('A' + (key - (uint32_t)SCITER_KEY_A)), '\0' };
        return std::string(buf);
    }
    if (key >= (uint32_t)SCITER_KEY_0 && key <= (uint32_t)SCITER_KEY_9)
    {
        char buf[2] = { (char)('0' + (key - (uint32_t)SCITER_KEY_0)), '\0' };
        return std::string(buf);
    }
    if (key >= (uint32_t)SCITER_KEY_F1 && key <= (uint32_t)SCITER_KEY_F25)
    {
        return SciterUI::stdstr_f("F%u", (unsigned)(1u + (key - (uint32_t)SCITER_KEY_F1)));
    }
    switch (key)
    {
    case (uint32_t)SCITER_KEY_ESCAPE:
        return "Esc";
    case (uint32_t)SCITER_KEY_ENTER:
        return "Enter";
    case (uint32_t)SCITER_KEY_TAB:
        return "Tab";
    case (uint32_t)SCITER_KEY_BACKSPACE:
        return "Backspace";
    case (uint32_t)SCITER_KEY_INSERT:
        return "Insert";
    case (uint32_t)SCITER_KEY_DELETE:
        return "Delete";
    case (uint32_t)SCITER_KEY_HOME:
        return "Home";
    case (uint32_t)SCITER_KEY_END:
        return "End";
    case (uint32_t)SCITER_KEY_PAGE_UP:
        return "PgUp";
    case (uint32_t)SCITER_KEY_PAGE_DOWN:
        return "PgDn";
    case (uint32_t)SCITER_KEY_LEFT:
        return "Left";
    case (uint32_t)SCITER_KEY_UP:
        return "Up";
    case (uint32_t)SCITER_KEY_RIGHT:
        return "Right";
    case (uint32_t)SCITER_KEY_DOWN:
        return "Down";
    case (uint32_t)SCITER_KEY_CAPS_LOCK:
        return "CapsLock";
    case (uint32_t)SCITER_KEY_SCROLL_LOCK:
        return "ScrollLock";
    case (uint32_t)SCITER_KEY_NUM_LOCK:
        return "NumLock";
    case (uint32_t)SCITER_KEY_PRINT_SCREEN:
        return "PrtSc";
    case (uint32_t)SCITER_KEY_PAUSE:
        return "Pause";
    case (uint32_t)SCITER_KEY_SPACE:
        return "Space";
    case (uint32_t)SCITER_KEY_MENU:
        return "Menu";
    case (uint32_t)SCITER_KEY_KP_0:
        return "Numpad0";
    case (uint32_t)SCITER_KEY_KP_1:
        return "Numpad1";
    case (uint32_t)SCITER_KEY_KP_2:
        return "Numpad2";
    case (uint32_t)SCITER_KEY_KP_3:
        return "Numpad3";
    case (uint32_t)SCITER_KEY_KP_4:
        return "Numpad4";
    case (uint32_t)SCITER_KEY_KP_5:
        return "Numpad5";
    case (uint32_t)SCITER_KEY_KP_6:
        return "Numpad6";
    case (uint32_t)SCITER_KEY_KP_7:
        return "Numpad7";
    case (uint32_t)SCITER_KEY_KP_8:
        return "Numpad8";
    case (uint32_t)SCITER_KEY_KP_9:
        return "Numpad9";
    case (uint32_t)SCITER_KEY_KP_DECIMAL:
        return "Numpad.";
    case (uint32_t)SCITER_KEY_KP_DIVIDE:
        return "Numpad/";
    case (uint32_t)SCITER_KEY_KP_MULTIPLY:
        return "Numpad*";
    case (uint32_t)SCITER_KEY_KP_SUBTRACT:
        return "Numpad-";
    case (uint32_t)SCITER_KEY_KP_ADD:
        return "Numpad+";
    case (uint32_t)SCITER_KEY_KP_ENTER:
        return "NumpadEnter";
    case (uint32_t)SCITER_KEY_KP_EQUAL:
        return "Numpad=";
    case (uint32_t)SCITER_KEY_COMMA:
        return ",";
    case (uint32_t)SCITER_KEY_MINUS:
        return "-";
    case (uint32_t)SCITER_KEY_PERIOD:
        return ".";
    case (uint32_t)SCITER_KEY_SLASH:
        return "/";
    case (uint32_t)SCITER_KEY_SEMICOLON:
        return ";";
    case (uint32_t)SCITER_KEY_EQUAL:
        return "=";
    case (uint32_t)SCITER_KEY_LEFT_BRACKET:
        return "[";
    case (uint32_t)SCITER_KEY_BACKSLASH:
        return "\\";
    case (uint32_t)SCITER_KEY_RIGHT_BRACKET:
        return "]";
    case (uint32_t)SCITER_KEY_GRAVE_ACCENT:
        return "`";
    case (uint32_t)SCITER_KEY_APOSTROPHE:
        return "'";
    default:
        break;
    }
    return SciterUI::stdstr_f("%u", (unsigned)key);
}

std::string MenuBarAccelerator::Format() const
{
    if (IsNone())
    {
        return std::string();
    }
    std::string s;
    if (ctrl)
    {
        s += "Ctrl+";
    }
    if (shift)
    {
        s += "Shift+";
    }
    if (alt)
    {
        s += "Alt+";
    }
    s += MenuBarAcceleratorKeyLabel(key);
    return s;
}

MenuBarItem::MenuBarItem(int32_t id, const char * title, MenuBarItemList * subMenu, const MenuBarAccelerator * shortcutAccel, CheckState checkState, const std::string * iconSvg)
{
    Reset(id, title, subMenu, shortcutAccel, checkState, iconSvg);
}

void MenuBarItem::Reset(int32_t id, const char * title, MenuBarItemList * subMenu, const MenuBarAccelerator * shortcutAccel, CheckState checkState, const std::string * iconSvg)
{
    m_id = id;
    m_title = title;
    m_subMenu = subMenu;
    m_checkState = checkState;
    if (iconSvg != nullptr)
    {
        m_iconSvg = *iconSvg;
    }
    else
    {
        m_iconSvg.clear();
    }
    if (shortcutAccel != nullptr)
    {
        m_shortcutAccel = *shortcutAccel;
    }
    else
    {
        m_shortcutAccel = {};
    }
}

int MenuBarItem::ID() const
{
    return m_id;
}
    
const char * MenuBarItem::Title() const
{
    return m_title.c_str();
}

const MenuBarItemList * MenuBarItem::SubMenu() const
{
    return m_subMenu;
}

const MenuBarAccelerator & MenuBarItem::ShortcutAccel() const
{
    return m_shortcutAccel;
}

MenuBarItem::CheckState MenuBarItem::ItemCheckState() const
{
    return m_checkState;
}

const std::string & MenuBarItem::IconSvg() const
{
    return m_iconSvg;
}

void WidgetMenuBar::Register(ISciterUI & sciterUI)
{
    const char * WidgetCss =
        "mainmenu {"
        "    display-model: block-inside;"
        "    display: block;"
        "    flow: vertical;"
        "    behavior: MainMenu;"
        "    overflow: hidden;"
        "    position: relative;"
        "    z-index: 1000;"
        "    width: *;"
        "    height: max-content;"
        "    flex-grow: 0;"
        "    flex-shrink: 0;"
        "}";
    sciterUI.RegisterWidgetType("MainMenu", WidgetMenuBar::CreateWidget, WidgetMenuBar::ReleaseWidget, WidgetCss);
}

void WidgetMenuBar::SetMenuContent(MenuBarItemList & items) const
{
    std::string html("<ul id=\"menu-bar\">\n");
    for (MenuBarItemList::iterator itr = items.begin(); itr != items.end(); itr++)
    {
        html += MenuItemHtml(*itr, 2);
    }
    html += "</ul>";
    m_menuBarElem.SetHTML((const uint8_t *)html.data(), html.size(), SciterElement::SIH_REPLACE_CONTENT);
}

void WidgetMenuBar::AddSink(IMenuBarSink * sink)
{
    m_sinks.insert(sink);
}

void WidgetMenuBar::RemoveSink(IMenuBarSink * sink)
{
    IMenuBarSinkSet::iterator itr = m_sinks.find(sink);
    if (itr != m_sinks.end())
    {
        m_sinks.erase(itr);
    }
}

void WidgetMenuBar::Attached(SCITER_ELEMENT element, IBaseElement * baseElement)
{
    m_baseElement = baseElement;
    m_menuBarElem = element;
    m_keySinkRoot = SciterElement();
    HWINDOW hwnd = (HWINDOW)m_menuBarElem.GetElementHwnd(true);
    if (hwnd != nullptr)
    {
        HELEMENT hRoot = 0;
        if (SciterGetRootElement((SciterUI::SciterHWINDOW)hwnd, &hRoot) == SCDOM_OK && hRoot != 0)
        {
            m_keySinkRoot = SciterElement(hRoot);
        }
    }
    if (!m_keySinkRoot.IsValid())
    {
        m_keySinkRoot = m_menuBarElem.GetRoot();
    }
    if (m_keySinkRoot.IsValid())
    {
        m_sciterUI.AttachHandler(m_keySinkRoot, IID_IKEYSINK, (IKeySink *)this);
        m_sciterUI.AttachHandler(m_keySinkRoot, IID_EVENTSINK, (IEventSink *)this);
        m_sciterUI.AttachHandler(m_keySinkRoot, IID_IMOUSEUPDOWNSINK, (IMouseUpDownSink *)this);
        m_sciterUI.AttachHandler(m_keySinkRoot, IID_IMOUSEMOVESINK, (IMouseMoveSink *)this);
    }
    m_sciterUI.AttachHandler(m_menuBarElem, IID_ICLICKSINK, (IClickSink *)this);
}

void WidgetMenuBar::Detached(SCITER_ELEMENT /*element*/)
{
    HideShownMenuPopup();
    if (m_keySinkRoot.IsValid())
    {
        m_sciterUI.DetachHandler(m_keySinkRoot, IID_IMOUSEMOVESINK, (IMouseMoveSink *)this);
        m_sciterUI.DetachHandler(m_keySinkRoot, IID_IMOUSEUPDOWNSINK, (IMouseUpDownSink *)this);
        m_sciterUI.DetachHandler(m_keySinkRoot, IID_EVENTSINK, (IEventSink *)this);
        m_sciterUI.DetachHandler(m_keySinkRoot, IID_IKEYSINK, (IKeySink *)this);
        m_keySinkRoot = nullptr;
    }
    m_sciterUI.DetachHandler(m_menuBarElem, IID_ICLICKSINK, (IClickSink *)this);
    m_baseElement = nullptr;
    m_menuBarElem = nullptr;
}

std::shared_ptr<void> WidgetMenuBar::GetInterface(const char * riid)
{
    if (strcmp(riid, IID_IMENUBAR) == 0)
    {
        return std::static_pointer_cast<IMenuBar>(shared_from_this());
    }
    return nullptr;
}

bool WidgetMenuBar::OnSciterElement(SCITER_ELEMENT he)
{
    std::string menuIdvalue = SciterElement(he).GetAttribute("data-menu_id");
    if (!menuIdvalue.empty())
    {
        m_sciterUI.AttachHandler(he, IID_ICLICKSINK, (IClickSink *)this);
    }
    return false;
}

void WidgetMenuBar::SyncMenuMnemonicsAttribute()
{
    const bool on = m_leftAltDown || m_rightAltDown;
    if (m_keySinkRoot.IsValid())
    {
        if (on)
        {
            m_keySinkRoot.SetAttribute("data-menu-mnemonics", "1");
        }
        else
        {
            m_keySinkRoot.RemoveAttribute("data-menu-mnemonics");
        }
    }
    else if (m_menuBarElem.IsValid())
    {
        if (on)
        {
            m_menuBarElem.SetAttribute("data-menu-mnemonics", "1");
        }
        else
        {
            m_menuBarElem.RemoveAttribute("data-menu-mnemonics");
        }
    }
    else
    {
        return;
    }
    SciterElement menuUl = m_menuBarElem.IsValid() ? m_menuBarElem.FindFirst("ul#menu-bar") : SciterElement();
    if (menuUl.IsValid())
    {
        menuUl.Update(true);
    }
}

bool WidgetMenuBar::OnKeyDown(SCITER_ELEMENT /*element*/, SCITER_ELEMENT /*item*/, SciterKeys keyCode, uint32_t /*keyboardState*/)
{
    if (keyCode == SCITER_KEY_LEFT_ALT)
    {
        if (!m_leftAltDown)
        {
            m_leftAltDown = true;
            SyncMenuMnemonicsAttribute();
        }
    }
    else if (keyCode == SCITER_KEY_RIGHT_ALT)
    {
        if (!m_rightAltDown)
        {
            m_rightAltDown = true;
            SyncMenuMnemonicsAttribute();
        }
    }
    else if (keyCode == SCITER_KEY_ESCAPE && m_menuActive)
    {
        HideShownMenuPopup();
        return true;
    }
    return false;
}

bool WidgetMenuBar::OnKeyUp(SCITER_ELEMENT /*element*/, SCITER_ELEMENT /*item*/, SciterKeys keyCode, uint32_t /*keyboardState*/)
{
    if (keyCode == SCITER_KEY_LEFT_ALT)
    {
        m_leftAltDown = false;
        SyncMenuMnemonicsAttribute();
    }
    else if (keyCode == SCITER_KEY_RIGHT_ALT)
    {
        m_rightAltDown = false;
        SyncMenuMnemonicsAttribute();
    }
    return false;
}

bool WidgetMenuBar::OnKeyChar(SCITER_ELEMENT /*element*/, SCITER_ELEMENT /*item*/, SciterKeys /*keyCode*/, uint32_t /*keyboardState*/)
{
    return false;
}

void WidgetMenuBar::HideSubMenu()
{
    if (m_openSubMenu.IsValid())
    {
        HideMenu(m_openSubMenu);
    }
    if (m_openSubItem.IsValid())
    {
        m_openSubItem.SetState(0, SciterElement::STATE_OWNS_POPUP | SciterElement::STATE_CURRENT, true);
    }
    m_openSubMenu = nullptr;
    m_openSubItem = nullptr;
}

void WidgetMenuBar::HideShownMenuPopup()
{
    HideSubMenu();
    if (m_openTopMenu.IsValid())
    {
        HideMenu(m_openTopMenu);
    }
    if (m_openTopItem.IsValid())
    {
        m_openTopItem.SetState(0, SciterElement::STATE_OWNS_POPUP | SciterElement::STATE_CURRENT, true);
    }
    m_openTopMenu = nullptr;
    m_openTopItem = nullptr;
    m_menuActive = false;
}

SciterElement WidgetMenuBar::DirectChildMenu(const SciterElement & item) const
{
    if (!item.IsValid())
    {
        return SciterElement();
    }
    for (uint32_t i = 0, n = item.GetChildCount(); i < n; ++i)
    {
        SciterElement child(item.GetChild(i));
        if (!child.IsValid())
        {
            continue;
        }
        LPCSTR type = nullptr;
        if (SciterGetElementType((HELEMENT)(SCITER_ELEMENT)child, &type) == SCDOM_OK && type != nullptr && sui_stricmp(type, "menu") == 0)
        {
            return child;
        }
    }
    return SciterElement();
}

SciterElement WidgetMenuBar::FindTopLevelItem(SciterElement from) const
{
    SciterElement e(from);
    for (int depth = 0; depth < 64 && e.IsValid(); ++depth)
    {
        SciterElement parent = e.GetParent();
        if (parent.IsValid() && parent.GetAttributeByName("id") == "menu-bar")
        {
            return e;
        }
        e = parent;
    }
    return SciterElement();
}

SciterElement WidgetMenuBar::FindPopupMenuItem(SciterElement from) const
{
    SciterElement e(from);
    for (int depth = 0; depth < 64 && e.IsValid(); ++depth)
    {
        SciterElement parent = e.GetParent();
        if (!parent.IsValid())
        {
            break;
        }
        LPCSTR type = nullptr;
        if (SciterGetElementType((HELEMENT)(SCITER_ELEMENT)parent, &type) == SCDOM_OK && type != nullptr && sui_stricmp(type, "menu") == 0)
        {
            return e;
        }
        e = parent;
    }
    return SciterElement();
}

bool WidgetMenuBar::ElementIsUnder(const SciterElement & hit, const SciterElement & ancestor) const
{
    if (!hit.IsValid() || !ancestor.IsValid())
    {
        return false;
    }
    SciterElement e(hit);
    for (int depth = 0; depth < 64 && e.IsValid(); ++depth)
    {
        if (e == ancestor)
        {
            return true;
        }
        e = e.GetParent();
    }
    return false;
}

void WidgetMenuBar::ShowTopMenu(SciterElement topItem)
{
    if (!topItem.IsValid())
    {
        return;
    }
    SciterElement menu = DirectChildMenu(topItem);
    if (!menu.IsValid())
    {
        return;
    }
    if (m_openTopItem == topItem && m_menuActive)
    {
        return;
    }
    HideSubMenu();
    if (m_openTopMenu.IsValid() && m_openTopMenu != menu)
    {
        HideMenu(m_openTopMenu);
    }
    if (m_openTopItem.IsValid() && m_openTopItem != topItem)
    {
        m_openTopItem.SetState(0, SciterElement::STATE_OWNS_POPUP | SciterElement::STATE_CURRENT, true);
    }

    ShowMenu(m_sciterUI, menu, topItem, InWindowMenuPlacement::BelowAnchor);

    topItem.SetState(SciterElement::STATE_OWNS_POPUP | SciterElement::STATE_CURRENT, 0, true);
    m_openTopItem = topItem;
    m_openTopMenu = menu;
    m_menuActive = true;
}

void WidgetMenuBar::ShowSubMenu(SciterElement item)
{
    if (!item.IsValid())
    {
        return;
    }
    SciterElement menu = DirectChildMenu(item);
    if (!menu.IsValid())
    {
        return;
    }
    if (m_openSubItem == item && m_openSubMenu.IsValid())
    {
        return;
    }
    HideSubMenu();

    ShowMenu(m_sciterUI, menu, item, InWindowMenuPlacement::RightOfAnchor);

    item.SetState(SciterElement::STATE_OWNS_POPUP | SciterElement::STATE_CURRENT, 0, true);
    m_openSubItem = item;
    m_openSubMenu = menu;
}

void WidgetMenuBar::NotifySinksMenuItem(int32_t id, SCITER_ELEMENT item) const
{
    const IMenuBarSinkSet sinks = m_sinks;
    for (IMenuBarSinkSet::const_iterator itr = sinks.begin(); itr != sinks.end(); itr++)
    {
        if (m_sinks.find(*itr) == m_sinks.end())
        {
            continue;
        }
        (*itr)->OnMenuItem(id, item);
    }
}

bool WidgetMenuBar::ElementIsUnderMainMenuWidget(const SciterElement & hit, const SciterElement & mainMenuElem)
{
    SciterElement e(hit);
    for (int depth = 0; depth < 64 && e.IsValid(); depth++)
    {
        if (e == mainMenuElem)
        {
            return true;
        }
        e = e.GetParent();
    }
    return false;
}

bool WidgetMenuBar::OnEvent(SCITER_ELEMENT /*element*/, SCITER_ELEMENT source, uint32_t event_code, uint64_t reason)
{
    if (event_code == kPopupDismissedEvent)
    {
        SciterElement dismissed(source);
        if (dismissed == m_openTopMenu || dismissed == m_openSubMenu ||
            ElementIsUnder(dismissed, m_openTopMenu) || ElementIsUnder(dismissed, m_openSubMenu))
        {
            if (m_openSubItem.IsValid())
            {
                m_openSubItem.SetState(0, SciterElement::STATE_OWNS_POPUP | SciterElement::STATE_CURRENT, true);
            }
            if (m_openTopItem.IsValid())
            {
                m_openTopItem.SetState(0, SciterElement::STATE_OWNS_POPUP | SciterElement::STATE_CURRENT, true);
            }
            m_openSubMenu = nullptr;
            m_openSubItem = nullptr;
            m_openTopMenu = nullptr;
            m_openTopItem = nullptr;
            m_menuActive = false;
        }
        return false;
    }
    if (event_code != kMenuItemClickEvent || reason != kClickReasonByKey)
    {
        return false;
    }
    if (!m_menuBarElem.IsValid())
    {
        return false;
    }
    SciterElement item(source);
    if (!item.IsValid() || !ElementIsUnderMainMenuWidget(item, m_menuBarElem))
    {
        return false;
    }
    for (uint32_t i = 0; i < 5; i++)
    {
        std::string menuIdvalue = item.GetAttribute("data-menu_id");
        if (!menuIdvalue.empty())
        {
            HideShownMenuPopup();
            NotifySinksMenuItem(std::stoi(menuIdvalue), item);
            return true;
        }
        item = item.GetParent();
        if (!item.IsValid())
        {
            break;
        }
    }
    return false;
}

bool WidgetMenuBar::OnMouseMove(SCITER_ELEMENT /*element*/, SCITER_ELEMENT source, uint32_t /*x*/, uint32_t /*y*/)
{
    if (!m_menuActive)
    {
        return false;
    }

    SciterElement popupItem = FindPopupMenuItem(source);
    if (popupItem.IsValid())
    {
        if (DirectChildMenu(popupItem).IsValid())
        {
            ShowSubMenu(popupItem);
        }
        else if (m_openSubItem.IsValid() && !ElementIsUnder(popupItem, m_openSubMenu))
        {
            HideSubMenu();
        }
        return false;
    }

    SciterElement topItem = FindTopLevelItem(source);
    if (topItem.IsValid() && DirectChildMenu(topItem).IsValid())
    {
        ShowTopMenu(topItem);
    }
    return false;
}

bool WidgetMenuBar::OnMouseDown(SCITER_ELEMENT /*element*/, SCITER_ELEMENT source, uint32_t /*x*/, uint32_t /*y*/)
{
    if (!m_menuActive)
    {
        return false;
    }
    SciterElement item(source);
    for (uint32_t i = 0; i < 8 && item.IsValid(); i++)
    {
        if (!item.GetAttribute("data-menu_id").empty())
        {
            return true;
        }
        item = item.GetParent();
    }
    SciterElement src(source);
    const bool inBar = m_menuBarElem.IsValid() && ElementIsUnder(src, m_menuBarElem);
    const bool inTop = ElementIsUnder(src, m_openTopMenu);
    const bool inSub = ElementIsUnder(src, m_openSubMenu);
    if (!inBar && !inTop && !inSub)
    {
        HideShownMenuPopup();
    }
    return false;
}

bool WidgetMenuBar::OnMouseUp(SCITER_ELEMENT /*element*/, SCITER_ELEMENT source, uint32_t /*x*/, uint32_t /*y*/)
{
    if (!m_menuActive)
    {
        return false;
    }
    SciterElement item(source);
    for (uint32_t i = 0; i < 8 && item.IsValid(); i++)
    {
        std::string menuIdvalue = item.GetAttribute("data-menu_id");
        if (!menuIdvalue.empty())
        {
            HideShownMenuPopup();
            NotifySinksMenuItem(std::stoi(menuIdvalue), item);
            return true;
        }
        item = item.GetParent();
    }
    return false;
}

bool WidgetMenuBar::OnClick(SCITER_ELEMENT /*element*/, SCITER_ELEMENT source, uint32_t /*reason*/)
{
    SciterElement item(source);
    for (uint32_t i = 0; i < 8 && item.IsValid(); i++)
    {
        std::string menuIdvalue = item.GetAttribute("data-menu_id");
        if (!menuIdvalue.empty())
        {
            HideShownMenuPopup();
            NotifySinksMenuItem(std::stoi(menuIdvalue), item);
            return true;
        }
        item = item.GetParent();
    }

    SciterElement popupItem = FindPopupMenuItem(source);
    if (popupItem.IsValid() && DirectChildMenu(popupItem).IsValid())
    {
        ShowSubMenu(popupItem);
        return true;
    }

    SciterElement topItem = FindTopLevelItem(source);
    if (topItem.IsValid() && DirectChildMenu(topItem).IsValid())
    {
        if (m_menuActive && m_openTopItem == topItem)
        {
            HideShownMenuPopup();
        }
        else
        {
            ShowTopMenu(topItem);
        }
        return true;
    }

    if (m_menuActive)
    {
        const bool inBar = m_menuBarElem.IsValid() && ElementIsUnder(source, m_menuBarElem);
        const bool inTop = ElementIsUnder(source, m_openTopMenu);
        const bool inSub = ElementIsUnder(source, m_openSubMenu);
        if (!inBar && !inTop && !inSub)
        {
            HideShownMenuPopup();
        }
    }
    return false;
}

std::string WidgetMenuBar::MenuItemHtml(const MenuBarItem & item, uint32_t indent)
{
    enum
    {
        kPopupMenuMinIndent = 6
    };

    if (item.ID() == MenuBarItem::SPLITER)
    {
        return SciterUI::stdstr_f("%*s<hr />\n", indent, "");
    }
    const std::string rawTitle = PrepareMenuTitleForMnemonics(item.Title());
    char accesskey = 0;
    std::string labelHtml;
    AppendMenuTitleHtml(rawTitle, labelHtml, accesskey);
    const std::string accesskeyAttr = accesskey ? SciterUI::stdstr_f(" accesskey=\"%c\"", accesskey) : std::string();
    SciterUI::stdstr icon;
    if (!item.IconSvg().empty())
    {
        icon = std::string("<span class=\"menu-item-glyph menu-item-icon-svg\" aria-hidden=\"true\">") + item.IconSvg() + "</span>";
    }
    else
    {
        switch (item.ItemCheckState())
        {
        case MenuBarItem::CheckState::Unchecked: 
            icon = "<span class='menu-item-glyph menu-item-checkbox' role='checkbox' aria-checked='false'></span>";
            break;
        case MenuBarItem::CheckState::Checked: 
            icon = "<span class='menu-item-glyph menu-item-checkbox checked' role='checkbox' aria-checked='true'><span class='menu-item-checkmark'>&#x2713;</span></span>";
            break;
        case MenuBarItem::CheckState::None:
        default:
            icon = "<span class='menu-item-glyph menu-item-glyph-spacer' aria-hidden='true'></span>";
            break;
        }
    }

    SciterUI::stdstr title;
    std::string submenu;
    if (item.SubMenu() != nullptr && item.SubMenu()->size() > 0)
    {
        for (MenuBarItemList::const_iterator itr = item.SubMenu()->begin(); itr != item.SubMenu()->end(); itr++)
        {
            submenu += MenuItemHtml(*itr, indent + 4);
        }
    }
    if (!submenu.empty())
    {
        submenu = SciterUI::stdstr_f("\n%*s<menu>\n%s%*s</menu>\n%*s", indent + 2, "", submenu.c_str(), indent + 2, "", indent, "");
        const std::string glyphLead = (indent >= kPopupMenuMinIndent) ? icon : std::string();
        title = SciterUI::stdstr_f("<li%s>\n%*s%s<caption>%s</caption>", accesskeyAttr.c_str(), indent + 2, "", glyphLead.c_str(), labelHtml.c_str());
    }
    else
    {
        const std::string glyphHtml = (indent >= kPopupMenuMinIndent) ? icon : std::string();
        title = SciterUI::stdstr_f("<li data-menu_id=\"%d\"%s>%s<span class='menu-item-label'>%s</span><span class='menu-accelerator'>%s</span>", item.ID(), accesskeyAttr.c_str(), glyphHtml.c_str(), labelHtml.c_str(), item.ShortcutAccel().Format().c_str());
    }
    return SciterUI::stdstr_f("%*s%s%s</li>\n", indent, "", title.c_str(), submenu.c_str());
}

IWidget * WidgetMenuBar::CreateWidget(ISciterUI & sciterUI)
{
    std::shared_ptr<WidgetMenuBar> instance(new WidgetMenuBar(sciterUI));
    IWidget * widget = (IWidget *)instance.get();
    m_instances.insert(MenuBars::value_type(widget, std::move(instance)));
    return widget;
}

void WidgetMenuBar::ReleaseWidget(IWidget * widget)
{
    MenuBars::iterator it = m_instances.find(widget);
    if (it != m_instances.end())
    {
        m_instances.erase(it);
    }
}

WidgetMenuBar::WidgetMenuBar(ISciterUI & sciterUI) :
    m_sciterUI(sciterUI),
    m_baseElement(nullptr),
    m_leftAltDown(false),
    m_rightAltDown(false),
    m_menuActive(false)
{
}

void Register_WidgetMenuBar(ISciterUI& sciterUI)
{
    WidgetMenuBar::Register(sciterUI);
}