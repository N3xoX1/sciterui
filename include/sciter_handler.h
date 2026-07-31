#pragma once
#include "sciter_ui.h"
#include <stdint.h>

suinterface IClickSink
{
    virtual bool OnClick(SCITER_ELEMENT element, SCITER_ELEMENT source, uint32_t reason) = 0;
};
static const char * IID_ICLICKSINK = "591A7458-0F80-4A52-A68E-89B1E4F16FB3";

suinterface IDoubleClickSink
{
    virtual bool OnDoubleClick(SCITER_ELEMENT element, SCITER_ELEMENT source) = 0;
};
static const char * IID_IDBLCLICKSINK = "D0D54987-9FF2-4D2A-83FB-77F996433351";

suinterface IMouseUpDownSink
{
    virtual bool OnMouseUp(SCITER_ELEMENT element, SCITER_ELEMENT source, uint32_t x, uint32_t y) = 0;
    virtual bool OnMouseDown(SCITER_ELEMENT element, SCITER_ELEMENT source, uint32_t x, uint32_t y) = 0;
};
static const char * IID_IMOUSEUPDOWNSINK = "4C82DCC8-FEC7-4F74-A0EC-399F56742F59";

suinterface IMouseMoveSink
{
    virtual bool OnMouseMove(SCITER_ELEMENT element, SCITER_ELEMENT source, uint32_t x, uint32_t y) = 0;
};
static const char * IID_IMOUSEMOVESINK = "BA281D38-FED5-46B1-BD95-3584C37107E4";

enum SciterKeys : uint32_t
{
    SCITER_KEY_SPACE = 32,
    SCITER_KEY_APOSTROPHE = 39,  /* ' */
    SCITER_KEY_COMMA = 44,       /* , */
    SCITER_KEY_MINUS = 45,       /* - */
    SCITER_KEY_PERIOD = 46,      /* . */
    SCITER_KEY_SLASH = 47,       /* / */
    SCITER_KEY_0 = 48,
    SCITER_KEY_1 = 49,
    SCITER_KEY_2 = 50,
    SCITER_KEY_3 = 51,
    SCITER_KEY_4 = 52,
    SCITER_KEY_5 = 53,
    SCITER_KEY_6 = 54,
    SCITER_KEY_7 = 55,
    SCITER_KEY_8 = 56,
    SCITER_KEY_9 = 57,
    SCITER_KEY_SEMICOLON = 59,   /* ; */
    SCITER_KEY_EQUAL = 61,       /* = */
    SCITER_KEY_A = 65,
    SCITER_KEY_B = 66,
    SCITER_KEY_C = 67,
    SCITER_KEY_D = 68,
    SCITER_KEY_E = 69,
    SCITER_KEY_F = 70,
    SCITER_KEY_G = 71,
    SCITER_KEY_H = 72,
    SCITER_KEY_I = 73,
    SCITER_KEY_J = 74,
    SCITER_KEY_K = 75,
    SCITER_KEY_L = 76,
    SCITER_KEY_M = 77,
    SCITER_KEY_N = 78,
    SCITER_KEY_O = 79,
    SCITER_KEY_P = 80,
    SCITER_KEY_Q = 81,
    SCITER_KEY_R = 82,
    SCITER_KEY_S = 83,
    SCITER_KEY_T = 84,
    SCITER_KEY_U = 85,
    SCITER_KEY_V = 86,
    SCITER_KEY_W = 87,
    SCITER_KEY_X = 88,
    SCITER_KEY_Y = 89,
    SCITER_KEY_Z = 90,
    SCITER_KEY_LEFT_BRACKET = 91,  /* [ */
    SCITER_KEY_BACKSLASH = 92,     /* \ */
    SCITER_KEY_RIGHT_BRACKET = 93, /* ] */
    SCITER_KEY_GRAVE_ACCENT = 96,  /* ` */
    SCITER_KEY_WORLD_1 = 161,      /* non-US #1 */
    SCITER_KEY_WORLD_2 = 162,      /* non-US #2 */

    /* Function keys */
    SCITER_KEY_ESCAPE = 256,
    SCITER_KEY_ENTER = 257,
    SCITER_KEY_TAB = 258,
    SCITER_KEY_BACKSPACE = 259,
    SCITER_KEY_INSERT = 260,
    SCITER_KEY_DELETE = 261,
    SCITER_KEY_RIGHT = 262,
    SCITER_KEY_LEFT = 263,
    SCITER_KEY_DOWN = 264,
    SCITER_KEY_UP = 265,
    SCITER_KEY_PAGE_UP = 266,
    SCITER_KEY_PAGE_DOWN = 267,
    SCITER_KEY_HOME = 268,
    SCITER_KEY_END = 269,
    SCITER_KEY_CAPS_LOCK = 280,
    SCITER_KEY_SCROLL_LOCK = 281,
    SCITER_KEY_NUM_LOCK = 282,
    SCITER_KEY_PRINT_SCREEN = 283,
    SCITER_KEY_PAUSE = 284,
    SCITER_KEY_F1 = 290,
    SCITER_KEY_F2 = 291,
    SCITER_KEY_F3 = 292,
    SCITER_KEY_F4 = 293,
    SCITER_KEY_F5 = 294,
    SCITER_KEY_F6 = 295,
    SCITER_KEY_F7 = 296,
    SCITER_KEY_F8 = 297,
    SCITER_KEY_F9 = 298,
    SCITER_KEY_F10 = 299,
    SCITER_KEY_F11 = 300,
    SCITER_KEY_F12 = 301,
    SCITER_KEY_F13 = 302,
    SCITER_KEY_F14 = 303,
    SCITER_KEY_F15 = 304,
    SCITER_KEY_F16 = 305,
    SCITER_KEY_F17 = 306,
    SCITER_KEY_F18 = 307,
    SCITER_KEY_F19 = 308,
    SCITER_KEY_F20 = 309,
    SCITER_KEY_F21 = 310,
    SCITER_KEY_F22 = 311,
    SCITER_KEY_F23 = 312,
    SCITER_KEY_F24 = 313,
    SCITER_KEY_F25 = 314,
    SCITER_KEY_KP_0 = 320,
    SCITER_KEY_KP_1 = 321,
    SCITER_KEY_KP_2 = 322,
    SCITER_KEY_KP_3 = 323,
    SCITER_KEY_KP_4 = 324,
    SCITER_KEY_KP_5 = 325,
    SCITER_KEY_KP_6 = 326,
    SCITER_KEY_KP_7 = 327,
    SCITER_KEY_KP_8 = 328,
    SCITER_KEY_KP_9 = 329,
    SCITER_KEY_KP_DECIMAL = 330,
    SCITER_KEY_KP_DIVIDE = 331,
    SCITER_KEY_KP_MULTIPLY = 332,
    SCITER_KEY_KP_SUBTRACT = 333,
    SCITER_KEY_KP_ADD = 334,
    SCITER_KEY_KP_ENTER = 335,
    SCITER_KEY_KP_EQUAL = 336,
    SCITER_KEY_LEFT_SHIFT = 340,
    SCITER_KEY_LEFT_CONTROL = 341,
    SCITER_KEY_LEFT_ALT = 342,
    SCITER_KEY_LEFT_SUPER = 343,
    SCITER_KEY_RIGHT_SHIFT = 344,
    SCITER_KEY_RIGHT_CONTROL = 345,
    SCITER_KEY_RIGHT_ALT = 346,
    SCITER_KEY_RIGHT_SUPER = 347,
    SCITER_KEY_MENU = 348,
};

suinterface IKeySink
{
    virtual bool OnKeyDown(SCITER_ELEMENT element, SCITER_ELEMENT target, SciterKeys keyCode, uint32_t keyboardState) = 0;
    virtual bool OnKeyUp(SCITER_ELEMENT element, SCITER_ELEMENT target, SciterKeys keyCode, uint32_t keyboardState) = 0;
    virtual bool OnKeyChar(SCITER_ELEMENT element, SCITER_ELEMENT target, SciterKeys keyCode, uint32_t keyboardState) = 0;
};
static const char * IID_IKEYSINK = "DC6A7757-D203-4636-A738-998C688E602C";

suinterface IResizeSink
{
    virtual bool OnSizeChanged(SCITER_ELEMENT elem) = 0;
};
static const char * IID_IRESIZESINK = "AE59F6AB-045C-4CCC-A2C6-3CFBD6F288F0";

suinterface IStateChangeSink
{
    virtual bool OnStateChange(SCITER_ELEMENT elem, uint32_t eventReason, void * data) = 0;
};
static const char * IID_ISTATECHANGESINK = "C2534439-8130-400A-A0F9-B4E5B1B1952B";

suinterface ITimerSink
{
    virtual bool OnTimer(SCITER_ELEMENT Element, uint32_t * TimerId) = 0;
};
static const char * IID_ITIMERSINK = "9B8F8D75-6C09-40D8-AB24-6C2849A2F07E";

enum class SciterBehaviorEvent : uint32_t
{
    ButtonClick = 0,
    ButtonPress = 1,
    ValueChanged = 2,
    ValueChanging = 3,
    SelectionChanged = 5,
    SelectionChanging = 0xC,
    PopupRequest = 7,
    PopupReady = 8,
    PopupDismissed = 9,
    MenuItemActive = 0xA,
    MenuItemClick = 0xB,
    ContextMenuRequest = 0x10,
    VisualStatusChanged = 0x11,
    DisabledStatusChanged = 0x12,
    PopupDismissing = 0x13,
    ContentChanged = 0x15,
    HyperlinkClick = 0x80,
    ElementCollapsed = 0x90,
    ElementExpanded = 0x91,
    ActivateChild = 0x92,
    FormSubmit = 0x96,
    FormReset = 0x97,
    DocumentComplete = 0x98,
    HistoryPush = 0x99,
    HistoryDrop = 0x9A,
    HistoryPrior = 0x9B,
    HistoryNext = 0x9C,
    HistoryStateChanged = 0x9D,
    ClosePopup = 0x9E,
    RequestTooltip = 0x9F,
    Animation = 0xA0,
    Transition = 0xA1,
    Swipe = 0xB0,
    DocumentCreated = 0xC0,
    DocumentCloseRequest = 0xC1,
    DocumentClose = 0xC2,
    DocumentReady = 0xC3,
    DocumentParsed = 0xC4,
    DocumentClosing = 0xC6,
    ContainerCloseRequest = 0xC7,
    ContainerClosing = 0xC8,
    VideoInitialized = 0xD1,
    VideoStarted = 0xD2,
    VideoStopped = 0xD3,
    VideoBindRq = 0xD4,
    VideoSourceCreated = 0xD5,
    VideoFrameRequest = 0xD8,
    PaginationStarts = 0xE0,
    PaginationPage = 0xE1,
    PaginationEnds = 0xE2,
    Custom = 0xF0,
    EglRender = 0x20,
    FirstApplicationEventCode = 0x100,
};

suinterface IEventSink
{
    virtual bool OnEvent(SCITER_ELEMENT element, SCITER_ELEMENT source, uint32_t event_code, uint64_t reason) = 0;
};
static const char * IID_EVENTSINK = "B3F842D6-C07D-49F9-8798-945A3F78F5EB";
