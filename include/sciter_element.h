#pragma once
#include "sciter_ui.h"
#include "sciter_value.h"
#include <vector>

suinterface ISciterElementCallback
{
    virtual bool OnSciterElement(SCITER_ELEMENT he) = 0;
};

class SciterElement;
using SciterElements = std::vector<SciterElement>;

class SciterElement
{
public:
    typedef enum ELEMENT_STATE_BITS
    {
        STATE_LINK = 0x00000001,       // :link pseudo-class in CSS
        STATE_HOVER = 0x00000002,      // :hover pseudo-class in CSS
        STATE_ACTIVE = 0x00000004,     // :active pseudo-class in CSS
        STATE_FOCUS = 0x00000008,      // :focus pseudo-class in CSS
        STATE_VISITED = 0x00000010,    // :visited pseudo-class in CSS
        STATE_CURRENT = 0x00000020,    // :current, current (hot) item
        STATE_CHECKED = 0x00000040,    // :checked, element is checked (or selected)
        STATE_DISABLED = 0x00000080,   // :disabled, element is disabled
        STATE_READONLY = 0x00000100,   // :read-only, readonly input element
        STATE_EXPANDED = 0x00000200,   // :expanded, expanded state - nodes in tree view
        STATE_COLLAPSED = 0x00000400,  // :collapsed, collapsed state - nodes in tree view - mutually exclusive with
        STATE_INCOMPLETE = 0x00000800, // :incomplete, fore (content) image was requested but not delivered
        STATE_ANIMATING = 0x00001000,  // :animating, is animating currently
        STATE_FOCUSABLE = 0x00002000,  // :focusable, will accept focus
        STATE_ANCHOR = 0x00004000,     // :anchor, anchor in selection (used with current in selects)
        STATE_SYNTHETIC = 0x00008000,  // :synthetic, this is a synthetic element - don't emit it's head/tail
        STATE_OWNS_POPUP = 0x00010000, // :owns-popup, has visible popup element (tooltip, menu, etc.)
        STATE_TABFOCUS = 0x00020000,   // :tab-focus, focus gained by tab traversal
        STATE_EMPTY = 0x00040000,      // :empty, empty - element is empty (text.size() == 0 && subs.size() == 0)
                                       //  if element has behavior attached then the behavior is responsible for the value of this flag.
        STATE_BUSY = 0x00080000,       // :busy, busy doing something (e.g. loading data)

        STATE_DRAG_OVER = 0x00100000,   // drag over the block that can accept it (so is current drop target). Flag is set for the drop target block
        STATE_DROP_TARGET = 0x00200000, // active drop target.
        STATE_MOVING = 0x00400000,      // dragging/moving - the flag is set for the moving block.
        STATE_COPYING = 0x00800000,     // dragging/copying - the flag is set for the copying block.
        STATE_DRAG_SOURCE = 0x01000000, // element that is a drag source.
        STATE_DROP_MARKER = 0x02000000, // element is drop marker

        STATE_PRESSED = 0x04000000, // pressed - close to active but has wider life span - e.g. in MOUSE_UP it
                                    //   is still on; so behavior can check it in MOUSE_UP to discover CLICK condition.
        STATE_POPUP = 0x08000000,   // :popup, this element is out of flow - shown as popup

        STATE_IS_LTR = 0x10000000, // the element or one of its containers has dir=ltr declared
        STATE_IS_RTL = 0x20000000, // the element or one of its containers has dir=rtl declared

    } ELEMENT_STATE_BITS;

    typedef enum ELEMENT_AREAS
    {
        ROOT_RELATIVE = 0x01,      // - or this flag if you want to get Sciter window relative coordinates,
                                   //   otherwise it will use nearest windowed container e.g. popup window.
        SELF_RELATIVE = 0x02,      // - "or" this flag if you want to get coordinates relative to the origin
                                   //   of element iself.
        CONTAINER_RELATIVE = 0x03, // - position inside immediate container.
        VIEW_RELATIVE = 0x04,      // - position relative to view - Sciter window

        CONTENT_BOX = 0x00, // content (inner)  box
        PADDING_BOX = 0x10, // content + paddings
        BORDER_BOX = 0x20,  // content + paddings + border
        MARGIN_BOX = 0x30,  // content + paddings + border + margins

        BACK_IMAGE_AREA = 0x40, // relative to content origin - location of background image (if it set no-repeat)
        FORE_IMAGE_AREA = 0x50, // relative to content origin - location of foreground image (if it set no-repeat)

        SCROLLABLE_AREA = 0x60, // scroll_area - scrollable area in content box

    } ELEMENT_AREAS;

    enum SET_ELEMENT_HTML
    {
        SIH_REPLACE_CONTENT = 0,
        SIH_INSERT_AT_START = 1,
        SIH_APPEND_AFTER_LAST = 2,
        SOH_REPLACE = 3,
        SOH_INSERT_BEFORE = 4,
        SOH_INSERT_AFTER = 5
    };

    typedef struct tagRECT
    {
        int32_t left;
        int32_t top;
        int32_t right;
        int32_t bottom;
    } RECT;

    SciterElement();
    SciterElement(SCITER_ELEMENT h);
    SciterElement(const SciterElement & e);
    ~SciterElement();

    SciterElement & operator=(SCITER_ELEMENT h);
    SciterElement & operator=(const SciterElement & e);

    operator bool() const;
    operator SCITER_ELEMENT() const;
    bool operator==(const SciterElement & rs) const;
    bool operator==(SCITER_ELEMENT rs) const;
    bool operator!=(const SciterElement & rs) const;
    bool operator!=(SCITER_ELEMENT rs) const;

    bool IsValid() const;
    void AddClassName(const char * value) const;
    void RemoveClassName(const char * value) const;

    void Clear();
    bool Create(const char * tagName, const char * text);
    void Detach() const;
    void Destroy();
    SciterElements FindAll(const char * selector, ...) const;
    SciterElement FindFirst(const char * selector, ...) const;
    std::string GetAttribute(const char * name) const;
    void SetAttribute(const char * name, const char * value) const;
    void RemoveAttribute(const char* name) const;
    std::string GetAttributeByName(const char* id) const;
    SCITER_ELEMENT GetChild(uint32_t Index) const;
    uint32_t GetChildCount() const;
    SciterElement GetElementByID(const char* id) const;
    HWINDOW GetElementHwnd(bool rootWindow) const;
    std::string GetHTML(bool OuterHtml) const;
    SciterElement GetParent() const;
    SciterElement GetRoot() const;
    uint32_t GetState() const;
    SciterValue GetValue() const;
    void HidePopup() const;
    void Insert(const SCITER_ELEMENT & e, uint32_t Index);
    bool ReleaseCapture(void) const;
    void PostEvent(uint32_t event_code, uint64_t reason = 0, SCITER_ELEMENT heSource = 0) const;
    bool SendEvent(uint32_t event_code, uint64_t reason = 0, SCITER_ELEMENT heSource = 0) const;
    void SelectElements(ISciterElementCallback * pcall, const char * selectors) const;
    bool SetCapture(void) const;
    void SetHTML(const uint8_t * hHtml, size_t htmlLength, int where = SIH_REPLACE_CONTENT) const;
    RECT GetLocation(uint32_t area = ROOT_RELATIVE | CONTENT_BOX) const;
    void SetState(uint32_t bitsToSet, uint32_t bitsToClear, bool update) const;
    void SetStyleAttribute(const char * Name, const char * Value) const;
    void SetText(const char * text) const;
    void SetTimer(uint32_t milliseconds, uint32_t * timer_id) const;
    void SetValue(SciterValue value) const;
    void Update(bool renderNow = false) const;
    SciterValue Eval(const char * script);

private:
    void Use(SCITER_ELEMENT h);
    void Unuse();
    void Set(SCITER_ELEMENT h);

    SCITER_ELEMENT m_he;
};
