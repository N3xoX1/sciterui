module sciter.dom.events.types;

import sciter.types;
import sciter.om.types;
import sciter.om.value;
import sciter.dom.types;
import sciter.graphics.types;

// events are sinking / bubbling, these are ORable event direction mask
// see: http://www.w3.org/TR/xml-events/Overview.html#s_intro
enum PHASE
{
    BUBBLING = 0,       // bubbling (emersion) phase
    SINKING  = 0x8000,  // capture (immersion) phase, this flag is or'ed with EVENTS codes below
    HANDLED  = 0x10000
}

//|
//| Mouse events
//| 

enum MOUSE // mouse event type
{
    ENTER = 0,
    LEAVE,
    MOVE,
    UP,
    DOWN,
    DCLICK, /// double click
    WHEEL,
    TICK,   /// generated periodically while mouse is pressed
    IDLE,   /// generated when mouse stays idle for some time

    TCLICK = 0xF, // tripple click

    DRAG_REQUEST = 0xFE, // mouse drag start detected event

    CLICK = 0xFF, // mouse click event

    HIT_TEST = 0xFFE,    // sent to element, allows to handle elements with non-trivial shapes.
}

/// Mouse event passed by Sciter to the app:
struct MOUSE_EVENT  
{
    UINT      type;         // see above, can be ORed with PHASE
    HELEMENT  target;       // target element
    POINT     pos;          // position of cursor, element relative
    POINT     posView;      // position of cursor, view relative
    UINT      buttonStates; // MOUSE_BUTTON combination
    UINT      keyMods;      // KEYBOARD_MOD combination
    UINT      cursorType;   // CURSOR_TYPE to set, see CURSOR_TYPE
    SBOOL     isOnIcon;     // mouse is over icon (foreground-image, foreground-repeat:no-repeat)
}

enum CURSOR
{
    ARROW, //0
    IBEAM, //1
    WAIT,  //2
    CROSS, //3
    UPARROW,  //4
    SIZENWSE, //5
    SIZENESW, //6
    SIZEWE,   //7
    SIZENS,   //8
    SIZEALL,  //9
    NO,       //10
    APPSTARTING, //11
    HELP,        //12
    HAND,        //13
    DRAG_MOVE,   //14
    DRAG_COPY,   //15
}

enum MOUSE_BUTTON
{
    MAIN = 1,   //aka left button
    PROP = 2,   //aka right button
    MIDDLE = 4,
}

enum KEYBOARD_MOD
  {
    LSHIFT = 0x0001,
    RSHIFT = 0x0002,
    LCONTROL = 0x0040,
    RCONTROL = 0x0080,
    LALT = 0x0100,
    RALT = 0x0200,
    LCOMMAND = 0x0400,
    RCOMMAND = 0x0800,
    NUM = 0x1000,
    CAPS = 0x2000,
    MODE = 0x4000,

    CONTROL = (LCONTROL | RCONTROL),
    SHIFT = (LSHIFT | RSHIFT),
    ALT = (LALT | RALT),
    COMMAND = (LCOMMAND | RCOMMAND), // "command key" on OSX, "win" on Windows
}


//|
//| Keyboard events
//| 

enum KEY
{
  PRESSED = 0, // 
  RELEASED,
  CHAR
}

struct KEY_EVENT
{

    UINT      type;         // see above
    HELEMENT  target;       // target element
    UINT      keyCode;      // key scan code, or character unicode for KEY_CHAR
    UINT      keyMods;      // KEYBOARD_MOD combination
}


//|
//| Focus events
//| 

enum FOCUS
{
    OUT = 0,            /// container lost focus from any element inside it, target is an element that lost focus 
    IN = 1,             /// container got focus on element inside it, target is an element that got focus
    GOT = 2,            /// target element got focus
    LOST = 3,           /// target element lost focus
    REQUEST = 4,        /// bubbling event/request, gets sent on child-parent chain to accept/reject focus to be set on the child (target)
    ADVANCE_REQUEST = 5,/// bubbling event/request, gets sent on child-parent chain to advance focus
}

struct FOCUS_EVENT
{

  UINT      type;           /// see above
  HELEMENT  target;         /// target element, for #FOCUS_LOST it is a handle of new focus element
                            /// and for #GOT it is a handle of old focus element, can be NULL
  UINT      cause;          /// focus cause params or FOCUS_CMD_TYPE for FOCUS_ADVANCE_REQUEST 
  SBOOL     cancel;         /// in #REQUEST setting this field to true will cancel transfer focus from old element to the new one.
}

enum FOCUS_RQ { /// FOCUS_EVENT.REQUEST cause
  NEXT,
  PREV,
  HOME,
  END,
  LEFT,
  RIGHT,
  UP,
  DOWN,  // all these - by key
  FIRST, // these two - by_code
  LAST,  //
  END_REACHED = 0x8000 // ored
}


//|
//| Scroll events, not bubbling
//| 

enum SCROLL
{
  // scroll commands that are coming from scrollbar
  // or keyaboard 
  HOME = 0,
  END,
  STEP_PLUS,
  STEP_MINUS,
  PAGE_PLUS,
  PAGE_MINUS,
  SET_POS,
  SLIDER_RELEASED,
  CORNER_PRESSED,
  CORNER_RELEASED,
  SLIDER_PRESSED,

  ANIMATION_START,
  ANIMATION_END,

  // notifications, sent what scroll is performed
  TARGET_POS, 
  POS,       
}

struct SCROLL_EVENT
{
    UINT      type;         /// see above
    HELEMENT  target;       /// target element
    INT       pos;          /// scroll position if SCROLL_POS
    SBOOL     vertical;     /// true if from vertical scrollbar
    UINT      source;       /// SCROLL_SOURCE
    UINT      reason;       /// key or scrollbar part
}

enum SCROLL_SOURCE {
  UNKNOWN,
  KEYBOARD,  // SCROLL_EVENT.reason <- keyCode
  SCROLLBAR, // SCROLL_EVENT.reason <- SCROLLBAR_PART
  ANIMATOR,
  WHEEL,
}

enum SCROLLBAR_PART {
  SCROLLBAR_BASE,
  SCROLLBAR_PLUS,
  SCROLLBAR_MINUS,
  SCROLLBAR_SLIDER,
  SCROLLBAR_PAGE_MINUS,
  SCROLLBAR_PAGE_PLUS,
  SCROLLBAR_CORNER,
}

//|
//| Drawing events, not bubbling
//| 

enum DRAW
{
    BACKGROUND = 0,
    CONTENT = 1,
    FOREGROUND = 2,
    OUTLINE = 3,
}

struct DRAW_EVENT
{
    UINT             type;      // see above
    HGFX             gfx;       // hdc to paint on
    RECT             area;      // element area:
    UINT             reserved;  //   for BACKGROUND/FOREGROUND - it is a border box
                                //   for CONTENT - it is a content(a.k.a. inner) box
}

//|
//| Behavior'al (logical, synthesized) events
//| 

enum EVENT
{
    CLICK = 0,               // click
    PRESS = 1,               // mouse down or key down in button
    
    CHANGED = 2,             // value did change
    CHANGING = 3,            // value will change
    
    SELECTION_CHANGED = 5,         // selection changed in <select>, <textarea>, etc
    SELECTION_CHANGING = 0xC,      // selection is changeing in <select>, <textarea>, etc

    POPUP_REQUEST   = 7,           // request to show popup just received,
                                   //     here DOM of popup element can be modifed.
    POPUP_READY     = 8,           // popup element has been measured and ready to be shown on screen,
                                   //     here you can use functions like ScrollToView.
    POPUP_DISMISSED = 9,           // popup element is closed,
                                   //     here DOM of popup element can be modifed again - e.g. some items can be removed
                                   //     to free memory.

    MENU_ITEM_ACTIVE = 0xA,        // menu item activated by mouse hover or by keyboard,
    MENU_ITEM_CLICK = 0xB,         // menu item click,
                                   //   BEHAVIOR_EVENT_PARAMS structure layout
                                   //   BEHAVIOR_EVENT_PARAMS.cmd - MENU_ITEM_CLICK/MENU_ITEM_ACTIVE
                                   //   BEHAVIOR_EVENT_PARAMS.heTarget - owner(anchor) of the menu
                                   //   BEHAVIOR_EVENT_PARAMS.he - the menu item, presumably <li> element
                                   //   BEHAVIOR_EVENT_PARAMS.reason - BY_MOUSE_CLICK | BY_KEY_CLICK


    CONTEXT_MENU_REQUEST = 0x10,   // "right-click", BEHAVIOR_EVENT_PARAMS::he is current popup menu HELEMENT being processed or NULL.
                                   // application can provide its own HELEMENT here (if it is NULL) or modify current menu element.

    VISUAL_STATUS_CHANGED = 0x11,  // sent to the element being shown or hidden
    DISABLED_STATUS_CHANGED = 0x12,// broadcast notification, sent to all elements of some container that got new value of :disabled state

    POPUP_DISMISSING = 0x13,       // popup is about to be closed

    CONTENT_CHANGED = 0x15,        // content has been changed, is posted to the element that gets content changed,  reason is combination of CONTENT_CHANGE_BITS.
                                   // target == NULL means the window got new document and this event is dispatched only to the window.

    // "grey" event codes  - notfications from behaviors from this SDK
    HYPERLINK_CLICK = 0x80,        // hyperlink click

    ELEMENT_COLLAPSED = 0x90,      // element was collapsed, so far only behavior:tabs is sending these two to the panels
    ELEMENT_EXPANDED = 0x91,       // element was expanded,

    ACTIVATE_CHILD = 0x92,         // activate (select) child,
                                   // used for example by accesskeys behaviors to send activation request, e.g. tab on behavior:tabs.

    FORM_SUBMIT = 0x96,            // behavior:form detected submission event. BEHAVIOR_EVENT_PARAMS::data field contains data to be posted.
                                   // BEHAVIOR_EVENT_PARAMS::data is of type T_MAP in this case key/value pairs of data that is about
                                   // to be submitted. You can modify the data or discard submission by returning true from the handler.
    FORM_RESET  = 0x97,            // behavior:form detected reset event (from button type=reset). BEHAVIOR_EVENT_PARAMS::data field contains data to be reset.
                                   // BEHAVIOR_EVENT_PARAMS::data is of type T_MAP in this case key/value pairs of data that is about
                                   // to be rest. You can modify the data or discard reset by returning true from the handler.

    DOCUMENT_COMPLETE = 0x98,             // document in behavior:frame or root document is complete.

    HISTORY_PUSH = 0x99,                  // requests to behavior:history (commands)
    HISTORY_DROP = 0x9A,
    HISTORY_PRIOR = 0x9B,
    HISTORY_NEXT = 0x9C,
    HISTORY_STATE_CHANGED = 0x9D,  // behavior:history notification - history stack has changed

    CLOSE_POPUP = 0x9E,            // close popup request,
    REQUEST_TOOLTIP = 0x9F,        // request tooltip, evt.source <- is the tooltip element.

    ANIMATION         = 0xA0,      // animation started (reason=1) or ended(reason=0) on the element.
    TRANSITION        = 0xA1,      // transition started (reason=1) or ended(reason=0) on the element.
    SWIPE             = 0xB0,      // swipe gesture detected, reason=4,8,2,6 - swipe direction, only from behavior:swipe-touch

    DOCUMENT_CREATED  = 0xC0,      // document created, script namespace initialized. target -> the document
    DOCUMENT_CLOSE_REQUEST = 0xC1, // document is about to be closed, to cancel closing do: evt.data = sciter::value("cancel");
    DOCUMENT_CLOSE    = 0xC2,      // last notification before document removal from the DOM
    DOCUMENT_READY    = 0xC3,      // document has got DOM structure, styles and behaviors of DOM elements. Script loading run is complete at this moment.
    DOCUMENT_PARSED   = 0xC4,      // document just finished parsing - has got DOM structure. This event is generated before DOCUMENT_READY
    //DOCUMENT_RELOAD        = 0xC5, // request to reload the document
    DOCUMENT_CLOSING  = 0xC6, // view::notify_close
    CONTAINER_CLOSE_REQUEST = 0xC7, // window of host document is processing DOCUMENT_CLOSE_REQUEST
    CONTAINER_CLOSING = 0xC8,       // window of host document is processing DOCUMENT_CLOSING

    VIDEO_INITIALIZED = 0xD1,      // <video> "ready" notification
    VIDEO_STARTED     = 0xD2,      // <video> playback started notification
    VIDEO_STOPPED     = 0xD3,      // <video> playback stoped/paused notification
    VIDEO_BIND_RQ     = 0xD4,      // <video> request for frame source binding,
                                   //   If you want to provide your own video frames source for the given target <video> element do the following:
                                   //   1. Handle and consume this VIDEO_BIND_RQ request
                                   //   2. You will receive second VIDEO_BIND_RQ request/event for the same <video> element
                                   //      but this time with the 'reason' field set to an instance of sciter::video_destination interface.
                                   //   3. add_ref() it and store it for example in worker thread producing video frames.
                                   //   4. call sciter::video_destination::start_streaming(...) providing needed parameters
                                   //      call sciter::video_destination::render_frame(...) as soon as they are available
                                   //      call sciter::video_destination::stop_streaming() to stop the rendering (a.k.a. end of movie reached)

    VIDEO_SOURCE_CREATED = 0xD5,

    VIDEO_FRAME_REQUEST = 0xD8,    // animation step, a.k.a. animation frame

    PAGINATION_STARTS  = 0xE0,     // behavior:pager starts pagination
    PAGINATION_PAGE    = 0xE1,     // behavior:pager paginated page no, reason -> page no
    PAGINATION_ENDS    = 0xE2,     // behavior:pager end pagination, reason -> total pages

    CUSTOM             = 0xF0,     // event with custom name

    EGL_RENDER         = 0x20,    //  

    FIRST_APPLICATION_EVENT_CODE = 0x100
    // all custom event codes shall be greater
    // than this number. All codes below this will be used
    // solely by application - Sciter will not intrepret it
    // and will do just dispatching.
    // To send event notifications with  these codes use
    // SciterSend/PostEvent API.
}


struct SEMANTIC_EVENT
{
    UINT     type;       // see above
    HELEMENT target;     // target element handler, in MENU_ITEM_CLICK this is owner element that caused this menu - e.g. context menu owner
                         // In scripting this field named as Event.owner
    HELEMENT related;    // source element e.g. in SELECTION_CHANGED it is new selected <option>, in MENU_ITEM_CLICK it is menu item (LI) element
    UINT_PTR reason;     // CLICK_REASON or EDIT_CHANGED_REASON - UI action causing change.
                         // In case of custom event notifications this may be any
                         // application specific value.
    VALUE    data;       // auxiliary data accompanied with the event. E.g. FORM_SUBMIT event is using this field to pass collection of values.

    LPCWSTR  name;       // name of custom event (when cmd == CUSTOM)
}

enum CLICK_REASON
{
    BY_MOUSE_CLICK,
    BY_KEY_CLICK,
    SYNTHESIZED,       // synthesized, programmatically generated.
    BY_MOUSE_ON_ICON,
}

enum EDIT_CHANGED_REASON
{
    BY_INS_CHAR,  // single char insertion
    BY_INS_CHARS, // character range insertion, clipboard
    BY_DEL_CHAR,  // single char deletion
    BY_DEL_CHARS, // character range deletion (selection)
    BY_UNDO_REDO, // undo/redo
}


struct LIFECYCLE_EVENT
{
  enum
  {
    DETACH = 0,
    ATTACH = 1
  }
  UINT type; // see above
}

struct ATTRIBUTE_CHANGE_EVENT
{
  HELEMENT  he;           // this element
  LPCSTR    name;         // attribute name
  LPCWSTR   value;        // new attribute value, NULL if attribute was deleted
}

struct SOM_EVENT
{
  enum
  {
    GET_PASSPORT = 0,
    GET_ASSET = 1
  }
  UINT type; // see above
  union {
    AssetPassportT* passport;
    AssetT*         asset;
  }
}

struct TIMER_EVENT
{
  UINT_PTR timerId;    // timerId that was used to create timer by using SciterSetTimer
}


enum EVENT_GROUP
{
    LIFECYCLE = 0x0000,      /**< attached/detached, always sent */
    MOUSE = 0x0001,          /**< mouse events */
    KEYBOARD = 0x0002,       /**< key events */
    FOCUS = 0x0004,          /**< focus events, if this flag is set it also means that element it attached to is focusable */
    SCROLL = 0x0008,         /**< scroll events */
    TIMER = 0x0010,          /**< timer event */
    SIZE = 0x0020,           /**< size changed event */
    DRAW = 0x0040,           /**< drawing request (event) */
    DATA_ARRIVED = 0x080,    /**< requested data () has been delivered */
    SEMANTIC_EVENT = 0x0100, /**< logical, synthetic events:
                                  BUTTON_CLICK, HYPERLINK_CLICK, etc.,
                                  a.k.a. notifications from intrinsic behaviors */
    METHOD_CALL           = 0x0200, /**< behavior specific methods */
    SCRIPTING_METHOD_CALL = 0x0400, /**< behavior specific methods */

    STYLE_CHANGE          = 0x0800, /**< element's style has changed */

    EXCHANGE              = 0x1000, /**< system drag-n-drop */
    GESTURE               = 0x2000, /**< touch input events */
    ATTRIBUTE_CHANGE      = 0x4000, /**< attribute change notification */

    SOM                   = 0x8000, /**< som_asset_t request */

    ALL                   = 0xFFFF, /*< all of them */

    SUBSCRIPTIONS         = 0xFFFFFFFF, /**< special value for getting subscription flags */
}
