#pragma once
#include <sciter_ui.h>
#include <stdint.h>
#include <string>
#include <list>

class MenuBarItem;
typedef std::list<MenuBarItem> MenuBarItemList;

struct MenuBarAccelerator
{
    uint32_t key = 0;
    bool ctrl = false;
    bool alt = false;
    bool shift = false;

    bool IsNone() const { return key == 0; }
    std::string Format() const;
};

class MenuBarItem
{
public:
    enum MenuID
    {
        SPLITER = -1,
        SUB_MENU = -2,
        NO_ID = -3
    };

    enum class CheckState : uint8_t
    {
        None,
        Unchecked,
        Checked,
    };

    MenuBarItem(int32_t id, const char * title = "", MenuBarItemList * subMenu = nullptr, const MenuBarAccelerator * shortcutAccel = nullptr, CheckState checkState = CheckState::None, const std::string * iconSvg = nullptr);
    void Reset(int32_t id, const char * title = "", MenuBarItemList * subMenu = nullptr, const MenuBarAccelerator * shortcutAccel = nullptr, CheckState checkState = CheckState::None, const std::string * iconSvg = nullptr);

    int ID() const;
    const char * Title() const;
    const MenuBarItemList * SubMenu() const;
    const MenuBarAccelerator & ShortcutAccel() const;
    CheckState ItemCheckState() const;
    const std::string & IconSvg() const;

private:
    MenuBarItem(void) = delete;
    MenuBarItem & operator=(const MenuBarItem &) = delete;

    int32_t m_id;
    std::string m_title;
    MenuBarItemList * m_subMenu;
    MenuBarAccelerator m_shortcutAccel;
    CheckState m_checkState;
    std::string m_iconSvg;
};

suinterface IMenuBarSink
{
    virtual void OnMenuItem(int32_t id, SCITER_ELEMENT item) = 0;
};

static const char * IID_IMENUBAR = "C890DFA5-2762-4727-A9E7-00F9B972ACE0";
suinterface IMenuBar
{
    virtual void SetMenuContent(MenuBarItemList & items) const = 0;
    virtual void AddSink(IMenuBarSink * sink) = 0;
    virtual void RemoveSink(IMenuBarSink * sink) = 0;
};

void Register_WidgetMenuBar(ISciterUI & sciterUI);