#include "x11_host.h"

#include "sciter.h"
#include "sciter_hwindow.h"
#include "sciter_window.h"

#include <sciter-x-graphics.h>
#include <sciter-x-key-codes.h>

#include <sciter_element.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <vector>

namespace SciterUI
{
namespace
{

constexpr unsigned long kMotifHintsFunctions = 1;
constexpr unsigned long kMotifHintsDecorations = 2;
constexpr unsigned long kMotifFuncAll = 1;
constexpr unsigned long kMotifFuncMove = 4;
constexpr unsigned long kMotifFuncMinimize = 8;
constexpr unsigned long kMotifFuncMaximize = 16;
constexpr unsigned long kMotifFuncClose = 32;
constexpr int kResizeGrip = 6;

struct Record
{
    SciterWindow * owner = nullptr;
    Window window = 0;
    Window parent = 0;
    SciterHWINDOW api = nullptr;
    GC gc = nullptr;
    Pixmap pixmap = 0;
    XImage * image = nullptr;
    int width = 0;
    int height = 0;
    int minWidth = 1;
    int minHeight = 1;
    bool resizable = false;
    int cursorId = 0;
    uint32_t mouseButtons = 0;
    bool mouseInside = false;
    bool mapped = false;
    bool mainWindow = false;
    bool blocksParent = false;
    bool maximized = false;
    bool closing = false;
    bool xDestroyed = false;
    int blockInput = 0;
    bool needsPaint = false;
    bool hasDirty = false;
    RECT dirty{};
    uint64_t lastClickMs = 0;
    int lastClickX = 0;
    int lastClickY = 0;
    unsigned lastClickButton = 0;
    std::string pressedRole;
};

HWINDOW PublicFromWindow(Window window)
{
    return reinterpret_cast<HWINDOW>(static_cast<uintptr_t>(window));
}

Window WindowFromPublic(HWINDOW hwnd)
{
    return static_cast<Window>(reinterpret_cast<uintptr_t>(hwnd));
}

uint64_t NowMs()
{
    timespec ts{};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<uint64_t>(ts.tv_sec) * 1000ull + static_cast<uint64_t>(ts.tv_nsec) / 1000000ull;
}

int XError(Display * display, XErrorEvent * error)
{
    char text[256];
    XGetErrorText(display, error->error_code, text, sizeof(text));
    std::fprintf(stderr, "sciterui: X11 error: %s\n", text);
    return 0;
}

uint32_t KeySymToSciter(KeySym sym)
{
    if (sym >= XK_a && sym <= XK_z)
    {
        return static_cast<uint32_t>(KB_A + (sym - XK_a));
    }
    if (sym >= XK_A && sym <= XK_Z)
    {
        return static_cast<uint32_t>(KB_A + (sym - XK_A));
    }
    if (sym >= XK_0 && sym <= XK_9)
    {
        return static_cast<uint32_t>(KB_0 + (sym - XK_0));
    }
    if (sym >= XK_KP_0 && sym <= XK_KP_9)
    {
        return static_cast<uint32_t>(KB_KP_0 + (sym - XK_KP_0));
    }
    if (sym >= XK_F1 && sym <= XK_F25)
    {
        return static_cast<uint32_t>(KB_F1 + (sym - XK_F1));
    }

    switch (sym)
    {
    case XK_space: return KB_SPACE;
    case XK_apostrophe: return KB_APOSTROPHE;
    case XK_comma: return KB_COMMA;
    case XK_minus: return KB_MINUS;
    case XK_period: return KB_PERIOD;
    case XK_slash: return KB_SLASH;
    case XK_semicolon: return KB_SEMICOLON;
    case XK_equal: return KB_EQUAL;
    case XK_bracketleft: return KB_LEFT_BRACKET;
    case XK_backslash: return KB_BACKSLASH;
    case XK_bracketright: return KB_RIGHT_BRACKET;
    case XK_grave: return KB_GRAVE_ACCENT;
    case XK_Escape: return KB_ESCAPE;
    case XK_Return: return KB_ENTER;
    case XK_KP_Enter: return KB_KP_ENTER;
    case XK_Tab: return KB_TAB;
    case XK_BackSpace: return KB_BACKSPACE;
    case XK_Insert: return KB_INSERT;
    case XK_Delete: return KB_DELETE;
    case XK_Right: return KB_RIGHT;
    case XK_Left: return KB_LEFT;
    case XK_Down: return KB_DOWN;
    case XK_Up: return KB_UP;
    case XK_Page_Up: return KB_PAGE_UP;
    case XK_Page_Down: return KB_PAGE_DOWN;
    case XK_Home: return KB_HOME;
    case XK_End: return KB_END;
    case XK_Caps_Lock: return KB_CAPS_LOCK;
    case XK_Scroll_Lock: return KB_SCROLL_LOCK;
    case XK_Num_Lock: return KB_NUM_LOCK;
    case XK_Print: return KB_PRINT_SCREEN;
    case XK_Pause: return KB_PAUSE;
    case XK_KP_Decimal: return KB_KP_DECIMAL;
    case XK_KP_Divide: return KB_KP_DIVIDE;
    case XK_KP_Multiply: return KB_KP_MULTIPLY;
    case XK_KP_Subtract: return KB_KP_SUBTRACT;
    case XK_KP_Add: return KB_KP_ADD;
    case XK_KP_Equal: return KB_KP_EQUAL;
    case XK_Shift_L: return KB_LEFT_SHIFT;
    case XK_Control_L: return KB_LEFT_CONTROL;
    case XK_Alt_L: return KB_LEFT_ALT;
    case XK_Super_L: return KB_LEFT_SUPER;
    case XK_Shift_R: return KB_RIGHT_SHIFT;
    case XK_Control_R: return KB_RIGHT_CONTROL;
    case XK_Alt_R: return KB_RIGHT_ALT;
    case XK_Super_R: return KB_RIGHT_SUPER;
    case XK_Menu: return KB_MENU;
    default: return 0;
    }
}

KEYBOARD_STATES ModifierState(unsigned state)
{
    unsigned mods = 0;
    if ((state & ShiftMask) != 0)
    {
        mods |= KEYBOARD_STATE_LSHIFT;
    }
    if ((state & ControlMask) != 0)
    {
        mods |= KEYBOARD_STATE_LCONTROL;
    }
    if ((state & Mod1Mask) != 0)
    {
        mods |= KEYBOARD_STATE_LALT;
    }
    if ((state & Mod4Mask) != 0)
    {
        mods |= KEYBOARD_STATE_LCOMMAND;
    }
    if ((state & LockMask) != 0)
    {
        mods |= KEYBOARD_STATE_CAPS;
    }
    return KEYBOARD_STATES(mods);
}

MOUSE_BUTTONS ButtonBit(unsigned button)
{
    switch (button)
    {
    case Button1: return MAIN_MOUSE_BUTTON;
    case Button2: return MIDDLE_MOUSE_BUTTON;
    case Button3: return PROP_MOUSE_BUTTON;
    default: return MOUSE_BUTTONS(0);
    }
}

std::string NearestRole(HELEMENT element)
{
    SciterElement current(element);
    while (current.IsValid())
    {
        std::string role = current.GetAttribute("role");
        if (!role.empty())
        {
            return role;
        }
        current = current.GetParent();
    }
    return {};
}

void EmitCharacters(SciterHWINDOW api, const char * text, int length, KEYBOARD_STATES mods)
{
    const unsigned char * cursor = reinterpret_cast<const unsigned char *>(text);
    const unsigned char * end = cursor + length;
    while (cursor < end)
    {
        uint32_t codepoint = 0;
        if (*cursor < 0x80)
        {
            codepoint = *cursor++;
        }
        else if ((*cursor & 0xE0) == 0xC0 && cursor + 1 < end)
        {
            codepoint = (*cursor & 0x1F) << 6 | (cursor[1] & 0x3F);
            cursor += 2;
        }
        else if ((*cursor & 0xF0) == 0xE0 && cursor + 2 < end)
        {
            codepoint = (*cursor & 0x0F) << 12 | (cursor[1] & 0x3F) << 6 | (cursor[2] & 0x3F);
            cursor += 3;
        }
        else if ((*cursor & 0xF8) == 0xF0 && cursor + 3 < end)
        {
            codepoint = (*cursor & 0x07) << 18 | (cursor[1] & 0x3F) << 12 | (cursor[2] & 0x3F) << 6 | (cursor[3] & 0x3F);
            cursor += 4;
        }
        else
        {
            ++cursor;
            continue;
        }
        if (codepoint >= 32 || codepoint == 9 || codepoint == 13)
        {
            SciterProcX(api, SCITER_X_MSG_KEY(KEY_CHAR, codepoint, mods));
        }
    }
}

int ChannelMemoryByte(unsigned long mask, int byteOrder)
{
    int lsbIndex = 0;
    while (lsbIndex < 4 && (mask & 0xfful) == 0)
    {
        mask >>= 8;
        ++lsbIndex;
    }
    if (byteOrder == MSBFirst)
    {
        return 3 - lsbIndex;
    }
    return lsbIndex;
}

void SwapRedBlue(uint8_t * pixels, int x, int y, int width, int height, int stride)
{
    for (int row = 0; row < height; ++row)
    {
        uint8_t * pixel = pixels + static_cast<size_t>(y + row) * static_cast<size_t>(stride) + static_cast<size_t>(x) * 4;
        for (int column = 0; column < width; ++column, pixel += 4)
        {
            std::swap(pixel[0], pixel[2]);
        }
    }
}

} // namespace

struct HostData
{
    Display * display = nullptr;
    int screen = 0;
    Window root = 0;
    Visual * visual = nullptr;
    int depth = 0;
    int dpi = 96;
    bool swapRedBlue = false;
    bool quit = false;
    int pumpDepth = 0;
    bool painting = false;
    int wakeRead = -1;
    int wakeWrite = -1;
    XIM inputMethod = nullptr;
    XIC inputContext = nullptr;
    bool pointerGrabbed = false;
    std::thread::id uiThread;
    std::atomic<bool> repaintAll{false};
    Cursor cursors[16]{};
    Atom wmDelete = 0;
    Atom wmProtocols = 0;
    Atom netWmName = 0;
    Atom netWmIcon = 0;
    Atom utf8String = 0;
    Atom netWmState = 0;
    Atom netWmStateMaxV = 0;
    Atom netWmStateMaxH = 0;
    Atom netWmStateAbove = 0;
    Atom netWmWindowType = 0;
    Atom netWmWindowTypeNormal = 0;
    Atom netWmWindowTypeDialog = 0;
    Atom netWmWindowTypeUtility = 0;
    Atom netWmPid = 0;
    Atom motifHints = 0;
    Atom netWorkArea = 0;
    std::unordered_map<Window, std::unique_ptr<Record>> records;
    std::unordered_map<SciterWindow *, Window> owners;
    std::vector<Window> stack;
};

struct X11HostStorage
{
    HostData state;
};

HostData * g_state = nullptr;

X11Host & X11Host::Instance()
{
    static X11Host host;
    if (g_state == nullptr)
    {
        static X11HostStorage storage;
        g_state = &storage.state;
    }
    return host;
}

int X11Host::Dpi() const
{
    return g_state != nullptr && g_state->dpi > 0 ? g_state->dpi : 96;
}

void X11Host::ScaleSize(int & width, int & height) const
{
    const int dpi = Dpi();
    if (dpi == 96)
    {
        return;
    }
    if (width > 0)
    {
        width = width * dpi / 96;
    }
    if (height > 0)
    {
        height = height * dpi / 96;
    }
}

void WorkArea(int & x, int & y, int & width, int & height)
{
    HostData * state = g_state;
    x = 0;
    y = 0;
    width = 1280;
    height = 720;
    if (state == nullptr || state->display == nullptr)
    {
        return;
    }
    width = DisplayWidth(state->display, state->screen);
    height = DisplayHeight(state->display, state->screen);

    Atom actualType = 0;
    int actualFormat = 0;
    unsigned long count = 0;
    unsigned long after = 0;
    unsigned char * data = nullptr;
    if (XGetWindowProperty(state->display, state->root, state->netWorkArea, 0, 4, False, XA_CARDINAL, &actualType, &actualFormat, &count, &after, &data) == Success && data != nullptr && count >= 4)
    {
        const long * values = reinterpret_cast<const long *>(data);
        x = static_cast<int>(values[0]);
        y = static_cast<int>(values[1]);
        width = static_cast<int>(values[2]);
        height = static_cast<int>(values[3]);
    }
    if (data != nullptr)
    {
        XFree(data);
    }
}

void X11Host::ClampToWorkArea(int & width, int & height) const
{
    int originX = 0;
    int originY = 0;
    int areaWidth = 0;
    int areaHeight = 0;
    WorkArea(originX, originY, areaWidth, areaHeight);
    (void)originX;
    (void)originY;
    if (areaWidth > 0 && width > areaWidth)
    {
        width = std::max(320, areaWidth);
    }
    if (areaHeight > 0 && height > areaHeight)
    {
        height = std::max(240, areaHeight);
    }
}

bool X11Host::Init()
{
    if (g_state != nullptr && g_state->display != nullptr)
    {
        return true;
    }
    Instance();
    HostData * state = g_state;
    XInitThreads();
    XSetErrorHandler(XError);
    state->display = XOpenDisplay(nullptr);
    if (state->display == nullptr)
    {
        std::fprintf(stderr, "sciterui: unable to open X11 display\n");
        return false;
    }
    state->screen = DefaultScreen(state->display);
    state->root = RootWindow(state->display, state->screen);
    state->visual = DefaultVisual(state->display, state->screen);
    state->depth = DefaultDepth(state->display, state->screen);
    const int byteOrder = ImageByteOrder(state->display);
    const int redByte = ChannelMemoryByte(state->visual->red_mask, byteOrder);
    const int blueByte = ChannelMemoryByte(state->visual->blue_mask, byteOrder);
    // Sciter.Lite writes RGBA. Swap when the XImage stores blue in the first byte.
    state->swapRedBlue = redByte != 0 || blueByte != 2;
    state->uiThread = std::this_thread::get_id();

    XrmInitialize();
    if (char * resource = XResourceManagerString(state->display))
    {
        XrmDatabase database = XrmGetStringDatabase(resource);
        if (database != nullptr)
        {
            XrmValue value;
            char * type = nullptr;
            if (XrmGetResource(database, "Xft.dpi", "Xft.Dpi", &type, &value) && value.addr != nullptr)
            {
                const int dpi = std::atoi(value.addr);
                if (dpi >= 72 && dpi <= 800)
                {
                    state->dpi = dpi;
                }
            }
            XrmDestroyDatabase(database);
        }
    }

    state->wmDelete = XInternAtom(state->display, "WM_DELETE_WINDOW", False);
    state->wmProtocols = XInternAtom(state->display, "WM_PROTOCOLS", False);
    state->netWmName = XInternAtom(state->display, "_NET_WM_NAME", False);
    state->netWmIcon = XInternAtom(state->display, "_NET_WM_ICON", False);
    state->utf8String = XInternAtom(state->display, "UTF8_STRING", False);
    state->netWmState = XInternAtom(state->display, "_NET_WM_STATE", False);
    state->netWmStateMaxV = XInternAtom(state->display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
    state->netWmStateMaxH = XInternAtom(state->display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
    state->netWmStateAbove = XInternAtom(state->display, "_NET_WM_STATE_ABOVE", False);
    state->netWmWindowType = XInternAtom(state->display, "_NET_WM_WINDOW_TYPE", False);
    state->netWmWindowTypeNormal = XInternAtom(state->display, "_NET_WM_WINDOW_TYPE_NORMAL", False);
    state->netWmWindowTypeDialog = XInternAtom(state->display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
    state->netWmWindowTypeUtility = XInternAtom(state->display, "_NET_WM_WINDOW_TYPE_UTILITY", False);
    state->netWmPid = XInternAtom(state->display, "_NET_WM_PID", False);
    state->motifHints = XInternAtom(state->display, "_MOTIF_WM_HINTS", False);
    state->netWorkArea = XInternAtom(state->display, "_NET_WORKAREA", False);

    int pipes[2] = {-1, -1};
    if (pipe(pipes) == 0)
    {
        fcntl(pipes[0], F_SETFL, O_NONBLOCK);
        fcntl(pipes[1], F_SETFL, O_NONBLOCK);
        state->wakeRead = pipes[0];
        state->wakeWrite = pipes[1];
    }
    state->quit = false;
    return true;
}

void Wake()
{
    HostData * state = g_state;
    if (state == nullptr || state->wakeWrite < 0)
    {
        return;
    }
    const char byte = 1;
    const ssize_t wrote = write(state->wakeWrite, &byte, 1);
    (void)wrote;
}

void X11Host::Stop()
{
    if (g_state == nullptr)
    {
        return;
    }
    g_state->quit = true;
    Wake();
}

Record * Find(Window window)
{
    if (g_state == nullptr)
    {
        return nullptr;
    }
    const auto found = g_state->records.find(window);
    if (found == g_state->records.end())
    {
        return nullptr;
    }
    return found->second.get();
}

Record * Find(SciterWindow & window)
{
    if (g_state == nullptr)
    {
        return nullptr;
    }
    const auto owner = g_state->owners.find(&window);
    if (owner == g_state->owners.end())
    {
        return nullptr;
    }
    return Find(owner->second);
}

Record * Find(HWINDOW hwnd)
{
    return Find(WindowFromPublic(hwnd));
}

void FreeBuffers(Record & record)
{
    HostData * state = g_state;
    if (record.image != nullptr)
    {
        void * bits = record.image->data;
        record.image->data = nullptr;
        XDestroyImage(record.image);
        std::free(bits);
        record.image = nullptr;
    }
    if (record.pixmap != 0 && state != nullptr && state->display != nullptr)
    {
        XFreePixmap(state->display, record.pixmap);
        record.pixmap = 0;
    }
}

void ResizeSurface(Record & record, int width, int height)
{
    HostData * state = g_state;
    if (state == nullptr || state->display == nullptr)
    {
        return;
    }
    if (width < 1)
    {
        width = 1;
    }
    if (height < 1)
    {
        height = 1;
    }
    if (record.image != nullptr && record.width == width && record.height == height)
    {
        return;
    }
    FreeBuffers(record);
    record.width = width;
    record.height = height;
    record.image = XCreateImage(state->display, state->visual, static_cast<unsigned>(state->depth), ZPixmap, 0, nullptr, static_cast<unsigned>(width), static_cast<unsigned>(height), 32, 0);
    if (record.image == nullptr)
    {
        return;
    }
    const size_t bytes = static_cast<size_t>(record.image->bytes_per_line) * static_cast<size_t>(height);
    record.image->data = static_cast<char *>(std::calloc(bytes, 1));
    if (record.image->data == nullptr)
    {
        record.image->data = nullptr;
        XDestroyImage(record.image);
        record.image = nullptr;
        return;
    }
    record.pixmap = XCreatePixmap(state->display, record.window, static_cast<unsigned>(width), static_cast<unsigned>(height), static_cast<unsigned>(state->depth));
    SL_SURFACE surface{};
    surface.bitmap.pixels = record.image->data;
    surface.bitmap.stride = static_cast<UINT>(record.image->bytes_per_line);
    SciterProcX(record.api, SCITER_X_MSG_SIZE(static_cast<UINT>(width), static_cast<UINT>(height), surface));
    record.needsPaint = true;
    record.hasDirty = true;
    record.dirty = RECT{0, 0, width, height};
}

void PaintRect(Record & record, int left, int top, int right, int bottom)
{
    HostData * state = g_state;
    if (state == nullptr || record.image == nullptr || record.image->data == nullptr || record.pixmap == 0)
    {
        return;
    }
    if (right <= left || bottom <= top)
    {
        left = 0;
        top = 0;
        right = record.width;
        bottom = record.height;
    }
    left = std::max(0, left);
    top = std::max(0, top);
    right = std::min(record.width, right);
    bottom = std::min(record.height, bottom);
    if (right <= left || bottom <= top)
    {
        return;
    }
    const RECT paint{left, top, right, bottom};
    state->painting = true;
    SciterProcX(record.api, SCITER_X_MSG_PAINT(paint));
    state->painting = false;

    const int width = right - left;
    const int height = bottom - top;
    auto * pixels = reinterpret_cast<uint8_t *>(record.image->data);
    if (state->swapRedBlue)
    {
        SwapRedBlue(pixels, left, top, width, height, record.image->bytes_per_line);
    }
    XPutImage(state->display, record.pixmap, record.gc, record.image, left, top, left, top, static_cast<unsigned>(width), static_cast<unsigned>(height));
    if (state->swapRedBlue)
    {
        SwapRedBlue(pixels, left, top, width, height, record.image->bytes_per_line);
    }
    if (record.mapped)
    {
        XCopyArea(state->display, record.pixmap, record.window, record.gc, left, top, static_cast<unsigned>(width), static_cast<unsigned>(height), left, top);
    }
}

void PaintPending()
{
    HostData * state = g_state;
    if (state == nullptr || state->display == nullptr)
    {
        return;
    }
    for (int pass = 0; pass < 3; ++pass)
    {
        const bool repaintAll = state->repaintAll.exchange(false);
        std::vector<Window> windows;
        windows.reserve(state->records.size());
        for (const auto & entry : state->records)
        {
            windows.push_back(entry.first);
        }
        bool painted = false;
        for (Window window : windows)
        {
            Record * record = Find(window);
            if (record == nullptr || record->closing)
            {
                continue;
            }
            if (repaintAll)
            {
                record->needsPaint = true;
                record->hasDirty = false;
            }
            if (!record->needsPaint)
            {
                continue;
            }
            RECT dirty = record->hasDirty ? record->dirty : RECT{0, 0, record->width, record->height};
            record->needsPaint = false;
            record->hasDirty = false;
            PaintRect(*record, dirty.left, dirty.top, dirty.right, dirty.bottom);
            painted = true;
        }
        if (!painted)
        {
            break;
        }
    }
    XFlush(state->display);
}

void MarkDirty(Record & record, int left, int top, int right, int bottom)
{
    if ((left == 0 && top == 0 && right == 0 && bottom == 0) || right <= left || bottom <= top)
    {
        left = 0;
        top = 0;
        right = record.width;
        bottom = record.height;
    }
    if (!record.hasDirty)
    {
        record.dirty = RECT{left, top, right, bottom};
        record.hasDirty = true;
    }
    else
    {
        record.dirty.left = std::min(record.dirty.left, left);
        record.dirty.top = std::min(record.dirty.top, top);
        record.dirty.right = std::max(record.dirty.right, right);
        record.dirty.bottom = std::max(record.dirty.bottom, bottom);
    }
    record.needsPaint = true;
}

void X11Host::Invalidate(HWINDOW hwnd, int left, int top, int right, int bottom)
{
    HostData * state = g_state;
    if (state == nullptr)
    {
        return;
    }
    if (std::this_thread::get_id() != state->uiThread)
    {
        state->repaintAll = true;
        Wake();
        return;
    }
    Record * record = Find(hwnd);
    if (record == nullptr)
    {
        return;
    }
    MarkDirty(*record, left, top, right, bottom);
    if (!state->painting && state->pumpDepth == 0)
    {
        PaintPending();
    }
}

Cursor CursorFor(uint32_t cursorId)
{
    HostData * state = g_state;
    if (state == nullptr || state->display == nullptr)
    {
        return None;
    }
    if (cursorId >= 16)
    {
        cursorId = 0;
    }
    if (state->cursors[cursorId] != 0)
    {
        return state->cursors[cursorId];
    }
    static const unsigned shapes[16] = {
        XC_left_ptr,
        XC_xterm,
        XC_watch,
        XC_crosshair,
        XC_sb_up_arrow,
        XC_top_left_corner,
        XC_top_right_corner,
        XC_sb_h_double_arrow,
        XC_sb_v_double_arrow,
        XC_fleur,
        XC_X_cursor,
        XC_watch,
        XC_question_arrow,
        XC_hand2,
        XC_fleur,
        XC_hand2,
    };
    state->cursors[cursorId] = XCreateFontCursor(state->display, shapes[cursorId]);
    return state->cursors[cursorId];
}

void X11Host::SetCursor(HWINDOW hwnd, uint32_t cursorId)
{
    Record * record = Find(hwnd);
    HostData * state = g_state;
    if (record == nullptr || state == nullptr || state->display == nullptr)
    {
        return;
    }
    record->cursorId = static_cast<int>(cursorId);
    XDefineCursor(state->display, record->window, CursorFor(cursorId));
}

void ApplyFrameHints(Record & record)
{
    HostData * state = g_state;
    if (state == nullptr || state->display == nullptr)
    {
        return;
    }
    XSizeHints * hints = XAllocSizeHints();
    if (hints == nullptr)
    {
        return;
    }
    hints->flags = PMinSize | PWinGravity;
    hints->min_width = std::max(1, record.minWidth);
    hints->min_height = std::max(1, record.minHeight);
    hints->win_gravity = NorthWestGravity;
    if (!record.resizable)
    {
        hints->flags |= PMaxSize;
        hints->max_width = std::max(hints->min_width, record.width > 0 ? record.width : hints->min_width);
        hints->max_height = std::max(hints->min_height, record.height > 0 ? record.height : hints->min_height);
    }
    XSetWMNormalHints(state->display, record.window, hints);
    XFree(hints);

    const unsigned long functions = record.resizable ? kMotifFuncAll : (kMotifFuncMove | kMotifFuncMinimize | kMotifFuncMaximize | kMotifFuncClose);
    const unsigned long motif[5] = {kMotifHintsFunctions | kMotifHintsDecorations, functions, 0, 0, 0};
    XChangeProperty(state->display, record.window, state->motifHints, state->motifHints, 32, PropModeReplace, reinterpret_cast<const unsigned char *>(motif), 5);
}

void X11Host::SetDocumentMinSize(SciterWindow & window, int cssWidth, int cssHeight)
{
    Record * record = Find(window);
    if (record == nullptr)
    {
        return;
    }
    int scaledWidth = cssWidth;
    int scaledHeight = cssHeight;
    ScaleSize(scaledWidth, scaledHeight);
    const UINT documentWidth = SciterGetMinWidth(record->api);
    const UINT documentHeight = SciterGetMinHeight(record->api, documentWidth > 0 ? documentWidth : static_cast<UINT>(std::max(scaledWidth, 1)));
    int minWidth = std::max(scaledWidth, static_cast<int>(documentWidth));
    int minHeight = std::max(scaledHeight, static_cast<int>(documentHeight));
    ClampToWorkArea(minWidth, minHeight);
    record->minWidth = std::max(1, minWidth);
    record->minHeight = std::max(1, minHeight);
    ApplyFrameHints(*record);
}

void X11Host::SetResizable(SciterWindow & window, bool resizable)
{
    Record * record = Find(window);
    if (record == nullptr)
    {
        return;
    }
    record->resizable = resizable;
    ApplyFrameHints(*record);
}

void X11Host::SetIcon(SciterWindow & window, const uint8_t * png, uint32_t size)
{
    Record * record = Find(window);
    HostData * state = g_state;
    if (record == nullptr || state == nullptr || state->display == nullptr || png == nullptr || size == 0 || state->netWmIcon == 0)
    {
        return;
    }
    LPSciterGraphicsAPI graphics = gapi();
    if (graphics == nullptr)
    {
        return;
    }
    HIMG image = nullptr;
    if (graphics->imageLoad(png, size, &image) != GRAPHIN_OK || image == nullptr)
    {
        return;
    }
    UINT width = 0;
    UINT height = 0;
    SBOOL usesAlpha = FALSE;
    if (graphics->imageGetInfo(image, &width, &height, &usesAlpha) != GRAPHIN_OK || width == 0 || height == 0)
    {
        graphics->imageRelease(image);
        return;
    }
    std::vector<uint8_t> raw;
    auto collect = [](LPVOID param, const BYTE * data, UINT length) -> SBOOL {
        auto * bytes = static_cast<std::vector<uint8_t> *>(param);
        bytes->insert(bytes->end(), data, data + length);
        return TRUE;
    };
    if (graphics->imageSave(image, collect, &raw, SCITER_IMAGE_ENCODING_RAW, 100) != GRAPHIN_OK)
    {
        graphics->imageRelease(image);
        return;
    }
    graphics->imageRelease(image);
    const size_t pixels = static_cast<size_t>(width) * static_cast<size_t>(height);
    if (raw.size() != pixels * 4)
    {
        return;
    }
    std::vector<unsigned long> icon;
    icon.reserve(2 + pixels);
    icon.push_back(width);
    icon.push_back(height);
    for (size_t index = 0; index < raw.size(); index += 4)
    {
        const unsigned long blue = raw[index];
        const unsigned long green = raw[index + 1];
        const unsigned long red = raw[index + 2];
        const unsigned long alpha = raw[index + 3];
        icon.push_back((alpha << 24) | (red << 16) | (green << 8) | blue);
    }
    XChangeProperty(state->display, record->window, state->netWmIcon, XA_CARDINAL, 32, PropModeReplace, reinterpret_cast<const unsigned char *>(icon.data()), static_cast<int>(icon.size()));
    XFlush(state->display);
}

void X11Host::SetTitle(SciterWindow & window, const char * title)
{
    Record * record = Find(window);
    HostData * state = g_state;
    if (record == nullptr || state == nullptr || state->display == nullptr || title == nullptr)
    {
        return;
    }
    XStoreName(state->display, record->window, title);
    XChangeProperty(state->display, record->window, state->netWmName, state->utf8String, 8, PropModeReplace, reinterpret_cast<const unsigned char *>(title), static_cast<int>(std::strlen(title)));
}

void NetMoveResize(Record & record, int rootX, int rootY, long direction)
{
    HostData * state = g_state;
    if (state == nullptr || state->display == nullptr)
    {
        return;
    }
    if (state->pointerGrabbed)
    {
        XUngrabPointer(state->display, CurrentTime);
        state->pointerGrabbed = false;
    }
    XEvent event{};
    event.xclient.type = ClientMessage;
    event.xclient.window = record.window;
    event.xclient.message_type = XInternAtom(state->display, "_NET_WM_MOVERESIZE", False);
    event.xclient.format = 32;
    event.xclient.data.l[0] = rootX;
    event.xclient.data.l[1] = rootY;
    event.xclient.data.l[2] = direction;
    event.xclient.data.l[3] = Button1;
    event.xclient.data.l[4] = 1;
    XSendEvent(state->display, state->root, False, SubstructureRedirectMask | SubstructureNotifyMask, &event);
    XFlush(state->display);
}

int EdgeResizeDirection(const Record & record, int x, int y)
{
    if (!record.resizable || record.maximized || record.width <= 0 || record.height <= 0)
    {
        return -1;
    }
    const bool left = x >= 0 && x < kResizeGrip;
    const bool right = x < record.width && x >= record.width - kResizeGrip;
    const bool top = y >= 0 && y < kResizeGrip;
    const bool bottom = y < record.height && y >= record.height - kResizeGrip;
    if (top && left)
    {
        return 0;
    }
    if (top && right)
    {
        return 2;
    }
    if (bottom && right)
    {
        return 4;
    }
    if (bottom && left)
    {
        return 6;
    }
    if (top)
    {
        return 1;
    }
    if (right)
    {
        return 3;
    }
    if (bottom)
    {
        return 5;
    }
    if (left)
    {
        return 7;
    }
    return -1;
}

int CursorForResizeDirection(int direction)
{
    switch (direction)
    {
    case 0:
    case 4:
        return CURSOR_SIZENWSE;
    case 2:
    case 6:
        return CURSOR_SIZENESW;
    case 3:
    case 7:
        return CURSOR_SIZEWE;
    case 1:
    case 5:
        return CURSOR_SIZENS;
    default:
        return -1;
    }
}

bool RoleBlocksResize(const std::string & role)
{
    return role == "window-close" || role == "window-ok" || role == "window-minimize" || role == "window-maximize";
}

void DefineCursor(Record & record, int cursorId)
{
    HostData * state = g_state;
    if (state == nullptr || state->display == nullptr || cursorId < 0)
    {
        return;
    }
    record.cursorId = cursorId;
    XDefineCursor(state->display, record.window, CursorFor(static_cast<uint32_t>(cursorId)));
}

void GrabPointer(Record & record)
{
    HostData * state = g_state;
    if (state == nullptr || state->pointerGrabbed)
    {
        return;
    }
    if (XGrabPointer(state->display, record.window, False, ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime) == GrabSuccess)
    {
        state->pointerGrabbed = true;
    }
}

void UngrabPointer()
{
    HostData * state = g_state;
    if (state == nullptr || !state->pointerGrabbed)
    {
        return;
    }
    XUngrabPointer(state->display, CurrentTime);
    state->pointerGrabbed = false;
}

void EnsureInput(Window window)
{
    HostData * state = g_state;
    if (state == nullptr || state->display == nullptr)
    {
        return;
    }
    if (state->inputMethod == nullptr)
    {
        state->inputMethod = XOpenIM(state->display, nullptr, nullptr, nullptr);
        if (state->inputMethod == nullptr)
        {
            XSetLocaleModifiers("");
            state->inputMethod = XOpenIM(state->display, nullptr, nullptr, nullptr);
        }
    }
    if (state->inputMethod == nullptr)
    {
        return;
    }
    if (state->inputContext == nullptr)
    {
        state->inputContext = XCreateIC(state->inputMethod, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, window, XNFocusWindow, window, nullptr);
    }
    else
    {
        XSetICValues(state->inputContext, XNClientWindow, window, XNFocusWindow, window, nullptr);
    }
}

void SetActive(Record & record, bool active)
{
    HELEMENT root = 0;
    if (SciterGetRootElement(record.api, &root) != SCDOM_OK || root == 0)
    {
        return;
    }
    SciterElement element(root);
    element.SetAttribute("data-active", active ? "true" : "false");
}

Window TopModalChild(Window parent)
{
    HostData * state = g_state;
    if (state == nullptr)
    {
        return 0;
    }
    for (auto it = state->stack.rbegin(); it != state->stack.rend(); ++it)
    {
        Record * record = Find(*it);
        if (record != nullptr && record->parent == parent && record->mapped && !record->closing)
        {
            return record->window;
        }
    }
    return 0;
}

bool Blocked(Record & record, int type)
{
    if (record.blockInput <= 0)
    {
        return false;
    }
    if (type != ButtonPress && type != ButtonRelease && type != MotionNotify && type != KeyPress && type != KeyRelease)
    {
        return false;
    }
    HostData * state = g_state;
    const Window child = TopModalChild(record.window);
    if (child != 0 && state != nullptr)
    {
        XRaiseWindow(state->display, child);
        XSetInputFocus(state->display, child, RevertToParent, CurrentTime);
    }
    return true;
}

void UnblockParent(Record & record)
{
    if (!record.blocksParent)
    {
        return;
    }
    record.blocksParent = false;
    Record * parent = Find(record.parent);
    if (parent == nullptr)
    {
        return;
    }
    if (parent->blockInput > 0)
    {
        parent->blockInput -= 1;
    }
    HostData * state = g_state;
    if (state != nullptr && parent->mapped)
    {
        XRaiseWindow(state->display, parent->window);
        XSetInputFocus(state->display, parent->window, RevertToParent, CurrentTime);
    }
}

void DestroyNative(SciterWindow & window, bool destroyEngine)
{
    HostData * state = g_state;
    if (state == nullptr)
    {
        return;
    }
    const auto owner = state->owners.find(&window);
    if (owner == state->owners.end())
    {
        return;
    }
    const Window xid = owner->second;
    std::unique_ptr<Record> held;
    const auto found = state->records.find(xid);
    if (found == state->records.end())
    {
        state->owners.erase(owner);
        return;
    }
    Record & record = *found->second;
    if (record.closing)
    {
        return;
    }
    record.closing = true;
    const bool mainWindow = record.mainWindow;
    const SciterHWINDOW api = record.api;
    const bool destroyX = !record.xDestroyed;
    UngrabPointer();
    UnblockParent(record);
    FreeBuffers(record);
    if (record.gc != nullptr)
    {
        XFreeGC(state->display, record.gc);
        record.gc = nullptr;
    }
    state->stack.erase(std::remove(state->stack.begin(), state->stack.end(), xid), state->stack.end());
    state->owners.erase(&window);
    held = std::move(found->second);
    state->records.erase(found);
    if (destroyX && state->display != nullptr)
    {
        XDestroyWindow(state->display, xid);
        XFlush(state->display);
    }
    if (destroyEngine)
    {
        SciterProcX(api, SCITER_X_MSG_DESTROY());
    }
    if (mainWindow)
    {
        X11Host::Instance().Stop();
    }
    (void)held;
}

HWINDOW X11Host::Create(SciterWindow & window, HWINDOW parent, int x, int y, int width, int height, unsigned flags, bool startHidden)
{
    if (!Init())
    {
        return nullptr;
    }
    HostData * state = g_state;
    int pixelWidth = width > 0 ? width : 760;
    int pixelHeight = height > 0 ? height : 507;
    ScaleSize(pixelWidth, pixelHeight);
    ClampToWorkArea(pixelWidth, pixelHeight);

    XSetWindowAttributes attributes{};
    attributes.background_pixel = 0x2d2d2d;
    attributes.border_pixel = 0;
    attributes.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | EnterWindowMask | LeaveWindowMask | FocusChangeMask | StructureNotifyMask | PropertyChangeMask;
    attributes.bit_gravity = NorthWestGravity;
    const unsigned long mask = CWBackPixel | CWBorderPixel | CWEventMask | CWBitGravity;
    Window created = XCreateWindow(state->display, state->root, x, y, static_cast<unsigned>(pixelWidth), static_cast<unsigned>(pixelHeight), 0, state->depth, InputOutput, state->visual, mask, &attributes);
    if (created == 0)
    {
        return nullptr;
    }

    auto record = std::make_unique<Record>();
    record->owner = &window;
    record->window = created;
    record->parent = WindowFromPublic(parent);
    // Lite instance id must not be an X11 window id. xwing treats those as native windows.
    record->api = reinterpret_cast<SciterHWINDOW>(&window);
    record->mainWindow = (flags & SUIW_MAIN) != 0;
    record->minWidth = 320;
    record->minHeight = 240;

    XGCValues gcValues{};
    gcValues.graphics_exposures = False;
    record->gc = XCreateGC(state->display, created, GCGraphicsExposures, &gcValues);

    const bool child = parent != nullptr && (flags & SUIW_CHILD) != 0;
    const bool popup = (flags & SUIW_POPUP) != 0;
    Atom windowType = state->netWmWindowTypeNormal;
    if (popup)
    {
        windowType = state->netWmWindowTypeUtility;
    }
    else if (child)
    {
        windowType = state->netWmWindowTypeDialog;
    }
    XChangeProperty(state->display, created, state->netWmWindowType, XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char *>(&windowType), 1);
    if (popup)
    {
        XChangeProperty(state->display, created, state->netWmState, XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char *>(&state->netWmStateAbove), 1);
    }
    XSetWMProtocols(state->display, created, &state->wmDelete, 1);
    const long pid = static_cast<long>(getpid());
    XChangeProperty(state->display, created, state->netWmPid, XA_CARDINAL, 32, PropModeReplace, reinterpret_cast<const unsigned char *>(&pid), 1);

    XClassHint classHint;
    classHint.res_name = const_cast<char *>("nxemu");
    classHint.res_class = const_cast<char *>("NxEmu");
    XSetClassHint(state->display, created, &classHint);

    if (child && record->parent != 0)
    {
        XSetTransientForHint(state->display, created, record->parent);
        Record * parentRecord = Find(record->parent);
        if (parentRecord != nullptr)
        {
            parentRecord->blockInput += 1;
            record->blocksParent = true;
        }
    }

    Record * stored = record.get();
    state->records.emplace(created, std::move(record));
    state->owners.emplace(&window, created);
    state->stack.push_back(created);

    if ((flags & SUIW_ENABLE_DEBUG) != 0)
    {
        SciterSetOption(stored->api, SCITER_SET_DEBUG_MODE, TRUE);
    }
    SCITER_X_MSG_CREATE create(SL_TARGET_BITMAP, nullptr);
    if (!SciterProcX(stored->api, create))
    {
        DestroyNative(window, false);
        return nullptr;
    }
    // SCITER_X_MSG_RESOLUTION posts a media change that calls
    // setup_window_frame on a null wing::window and segfaults in Sciter.Lite.
    ResizeSurface(*stored, pixelWidth, pixelHeight);
    ApplyFrameHints(*stored);
    (void)startHidden;
    return PublicFromWindow(created);
}

void X11Host::Show(SciterWindow & window)
{
    Record * record = Find(window);
    HostData * state = g_state;
    if (record == nullptr || state == nullptr || state->display == nullptr)
    {
        return;
    }
    record->mapped = true;
    XMapRaised(state->display, record->window);
    PaintRect(*record, 0, 0, record->width, record->height);
    XFlush(state->display);
}

void X11Host::Hide(SciterWindow & window)
{
    Record * record = Find(window);
    HostData * state = g_state;
    if (record == nullptr || state == nullptr || state->display == nullptr || record->closing)
    {
        return;
    }
    record->mapped = false;
    XUnmapWindow(state->display, record->window);
}

void X11Host::Center(SciterWindow & window)
{
    Record * record = Find(window);
    HostData * state = g_state;
    if (record == nullptr || state == nullptr || state->display == nullptr)
    {
        return;
    }
    int areaX = 0;
    int areaY = 0;
    int areaW = 0;
    int areaH = 0;
    WorkArea(areaX, areaY, areaW, areaH);
    int originX = areaX;
    int originY = areaY;
    int outerW = areaW;
    int outerH = areaH;
    if (record->parent != 0)
    {
        XWindowAttributes attributes{};
        if (XGetWindowAttributes(state->display, record->parent, &attributes) != 0)
        {
            Window child = 0;
            int rootX = 0;
            int rootY = 0;
            XTranslateCoordinates(state->display, record->parent, state->root, 0, 0, &rootX, &rootY, &child);
            originX = rootX;
            originY = rootY;
            outerW = attributes.width;
            outerH = attributes.height;
        }
    }
    int x = originX + (outerW - record->width) / 2;
    int y = originY + (outerH - record->height) / 2;
    const int maxX = areaX + std::max(0, areaW - record->width);
    const int maxY = areaY + std::max(0, areaH - record->height);
    x = std::max(areaX, std::min(x, maxX));
    y = std::max(areaY, std::min(y, maxY));
    XMoveWindow(state->display, record->window, x, y);
}

void X11Host::ApplySize(SciterWindow & window, int x, int y, int width, int height)
{
    Record * record = Find(window);
    HostData * state = g_state;
    if (record == nullptr || state == nullptr || state->display == nullptr)
    {
        return;
    }
    ScaleSize(width, height);
    ClampToWorkArea(width, height);
    if (width <= 0)
    {
        width = record->width;
    }
    if (height <= 0)
    {
        height = record->height;
    }
    XMoveResizeWindow(state->display, record->window, x, y, static_cast<unsigned>(width), static_cast<unsigned>(height));
    ResizeSurface(*record, width, height);
    ApplyFrameHints(*record);
}

void X11Host::FixMinSize(SciterWindow & window, int layoutWidth, int layoutHeight)
{
    Record * record = Find(window);
    HostData * state = g_state;
    if (record == nullptr || state == nullptr || state->display == nullptr)
    {
        return;
    }
    ScaleSize(layoutWidth, layoutHeight);
    const UINT minWidth = SciterGetMinWidth(record->api);
    const UINT widthForHeight = layoutWidth > 0 ? static_cast<UINT>(layoutWidth) : minWidth;
    const UINT minHeight = SciterGetMinHeight(record->api, widthForHeight);
    int width = static_cast<int>(layoutWidth > 0 ? std::max(minWidth, static_cast<UINT>(layoutWidth)) : minWidth);
    int height = static_cast<int>(layoutHeight > 0 ? std::max(minHeight, static_cast<UINT>(layoutHeight)) : minHeight);
    if (width < record->minWidth)
    {
        width = record->minWidth;
    }
    if (height < record->minHeight)
    {
        height = record->minHeight;
    }
    ClampToWorkArea(width, height);
    XResizeWindow(state->display, record->window, static_cast<unsigned>(width), static_cast<unsigned>(height));
    ResizeSurface(*record, width, height);
    ApplyFrameHints(*record);
}

void X11Host::Close(SciterWindow & window)
{
    window.SetDestroyed();
    DestroyNative(window, true);
}

void X11Host::Abandon(SciterWindow & window)
{
    DestroyNative(window, true);
}

void ToggleMaximize(Record & record)
{
    HostData * state = g_state;
    if (state == nullptr)
    {
        return;
    }
    const bool add = !record.maximized;
    XEvent event{};
    event.xclient.type = ClientMessage;
    event.xclient.window = record.window;
    event.xclient.message_type = state->netWmState;
    event.xclient.format = 32;
    event.xclient.data.l[0] = add ? 1 : 0;
    event.xclient.data.l[1] = static_cast<long>(state->netWmStateMaxV);
    event.xclient.data.l[2] = static_cast<long>(state->netWmStateMaxH);
    event.xclient.data.l[3] = 1;
    XSendEvent(state->display, state->root, False, SubstructureRedirectMask | SubstructureNotifyMask, &event);
    record.maximized = add;
}

void OnButton(Record & record, const XButtonEvent & event, bool pressed)
{
    HostData * state = g_state;
    if (event.button >= 4 && event.button <= 7)
    {
        if (!pressed)
        {
            return;
        }
        int deltaX = 0;
        int deltaY = 0;
        if (event.button == Button4)
        {
            deltaY = 120;
        }
        else if (event.button == Button5)
        {
            deltaY = -120;
        }
        else if (event.button == 6)
        {
            deltaX = -120;
        }
        else
        {
            deltaX = 120;
        }
        const unsigned deltas = (static_cast<uint16_t>(static_cast<int16_t>(deltaX)) << 16) | static_cast<uint16_t>(static_cast<int16_t>(deltaY));
        const POINT position{event.x, event.y};
        SciterProcX(record.api, SCITER_X_MSG_MOUSE(MOUSE_WHEEL, MOUSE_BUTTONS(deltas), ModifierState(event.state), position));
        return;
    }

    const MOUSE_BUTTONS bit = ButtonBit(event.button);
    if (bit == 0 || state == nullptr)
    {
        return;
    }
    const POINT position{event.x, event.y};
    if (pressed)
    {
        std::string role;
        if (event.button == Button1)
        {
            HELEMENT hit = 0;
            if (SciterFindElement(record.api, position, &hit) == SCDOM_OK)
            {
                role = NearestRole(hit);
            }
            const int direction = RoleBlocksResize(role) ? -1 : EdgeResizeDirection(record, event.x, event.y);
            if (direction >= 0)
            {
                NetMoveResize(record, event.x_root, event.y_root, direction);
                return;
            }
            if (role == "window-caption")
            {
                NetMoveResize(record, event.x_root, event.y_root, 8);
                return;
            }
        }
        GrabPointer(record);
        const uint64_t now = NowMs();
        const bool doubleClick = record.lastClickButton == event.button && record.lastClickMs != 0 && now - record.lastClickMs < 400 && std::abs(event.x - record.lastClickX) <= 4 && std::abs(event.y - record.lastClickY) <= 4;
        record.lastClickButton = event.button;
        record.lastClickX = event.x;
        record.lastClickY = event.y;
        record.lastClickMs = doubleClick ? 0 : now;
        record.mouseButtons |= static_cast<uint32_t>(bit);
        record.pressedRole = (role == "window-close" || role == "window-ok" || role == "window-minimize" || role == "window-maximize") ? role : std::string();
        SciterProcX(record.api, SCITER_X_MSG_MOUSE(doubleClick ? MOUSE_DCLICK : MOUSE_DOWN, bit, ModifierState(event.state), position));
        return;
    }

    UngrabPointer();
    record.mouseButtons &= ~static_cast<uint32_t>(bit);
    const std::string pressedRole = record.pressedRole;
    record.pressedRole.clear();
    const Window window = record.window;
    SciterWindow * owner = record.owner;
    SciterProcX(record.api, SCITER_X_MSG_MOUSE(MOUSE_UP, bit, ModifierState(event.state), position));
    Record * alive = Find(window);
    if (alive == nullptr || pressedRole.empty())
    {
        return;
    }
    HELEMENT hit = 0;
    std::string role;
    if (SciterFindElement(alive->api, position, &hit) == SCDOM_OK)
    {
        role = NearestRole(hit);
    }
    if (role != pressedRole)
    {
        return;
    }
    if (pressedRole == "window-close" || pressedRole == "window-ok")
    {
        owner->Destroy();
    }
    else if (pressedRole == "window-minimize")
    {
        XIconifyWindow(state->display, window, state->screen);
    }
    else if (pressedRole == "window-maximize")
    {
        ToggleMaximize(*alive);
    }
}

void OnKey(Record & record, XKeyEvent & event, bool pressed)
{
    HostData * state = g_state;
    if (state == nullptr)
    {
        return;
    }
    if (!pressed && XEventsQueued(state->display, QueuedAfterReading) != 0)
    {
        XEvent next{};
        XPeekEvent(state->display, &next);
        if (next.type == KeyPress && next.xkey.window == event.window && next.xkey.keycode == event.keycode && next.xkey.time == event.time)
        {
            return;
        }
    }

    EnsureInput(record.window);
    const KeySym physical = XkbKeycodeToKeysym(state->display, event.keycode, 0, 0);
    const uint32_t code = KeySymToSciter(physical);
    const KEYBOARD_STATES mods = ModifierState(event.state);
    if (pressed && code == KB_F4 && (mods & KEYBOARD_STATE_ALT) != 0)
    {
        record.owner->Destroy();
        return;
    }
    if (code != 0)
    {
        SciterProcX(record.api, SCITER_X_MSG_KEY(pressed ? KEY_DOWN : KEY_UP, code, mods));
    }
    if (!pressed)
    {
        return;
    }
    char text[64];
    KeySym lookedUp = 0;
    int length = 0;
    if (state->inputContext != nullptr)
    {
        Status status = 0;
        length = Xutf8LookupString(state->inputContext, &event, text, static_cast<int>(sizeof(text) - 1), &lookedUp, &status);
    }
    else
    {
        length = XLookupString(&event, text, static_cast<int>(sizeof(text) - 1), &lookedUp, nullptr);
    }
    if (length > 0)
    {
        EmitCharacters(record.api, text, length, mods);
    }
}

void Dispatch(XEvent & event)
{
    Record * record = Find(event.xany.window);
    if (record == nullptr || record->closing)
    {
        return;
    }
    if (Blocked(*record, event.type))
    {
        return;
    }
    HostData * state = g_state;
    switch (event.type)
    {
    case Expose:
        if (event.xexpose.count == 0)
        {
            MarkDirty(*record, event.xexpose.x, event.xexpose.y, event.xexpose.x + event.xexpose.width, event.xexpose.y + event.xexpose.height);
        }
        else
        {
            MarkDirty(*record, event.xexpose.x, event.xexpose.y, event.xexpose.x + event.xexpose.width, event.xexpose.y + event.xexpose.height);
        }
        break;
    case ConfigureNotify:
        if (event.xconfigure.width != record->width || event.xconfigure.height != record->height)
        {
            int width = event.xconfigure.width;
            int height = event.xconfigure.height;
            if (width < record->minWidth)
            {
                width = record->minWidth;
            }
            if (height < record->minHeight)
            {
                height = record->minHeight;
            }
            if (width != event.xconfigure.width || height != event.xconfigure.height)
            {
                XResizeWindow(state->display, record->window, static_cast<unsigned>(width), static_cast<unsigned>(height));
            }
            ResizeSurface(*record, width, height);
        }
        break;
    case FocusIn:
        if (event.xfocus.detail == NotifyPointer)
        {
            break;
        }
        SciterProcX(record->api, SCITER_X_MSG_FOCUS(TRUE));
        SetActive(*record, true);
        if (state->inputContext != nullptr)
        {
            XSetICFocus(state->inputContext);
        }
        break;
    case FocusOut:
        if (event.xfocus.detail == NotifyPointer)
        {
            break;
        }
        SciterProcX(record->api, SCITER_X_MSG_FOCUS(FALSE));
        SetActive(*record, false);
        if (state->inputContext != nullptr)
        {
            XUnsetICFocus(state->inputContext);
        }
        break;
    case EnterNotify:
        record->mouseInside = true;
        SciterProcX(record->api, SCITER_X_MSG_MOUSE(MOUSE_ENTER, MOUSE_BUTTONS(record->mouseButtons), ModifierState(event.xcrossing.state), POINT{event.xcrossing.x, event.xcrossing.y}));
        break;
    case LeaveNotify:
        if (state->pointerGrabbed)
        {
            break;
        }
        record->mouseInside = false;
        SciterProcX(record->api, SCITER_X_MSG_MOUSE(MOUSE_LEAVE, MOUSE_BUTTONS(0), ModifierState(event.xcrossing.state), POINT{event.xcrossing.x, event.xcrossing.y}));
        break;
    case MotionNotify:
        if (!record->mouseInside)
        {
            record->mouseInside = true;
            SciterProcX(record->api, SCITER_X_MSG_MOUSE(MOUSE_ENTER, MOUSE_BUTTONS(record->mouseButtons), ModifierState(event.xmotion.state), POINT{event.xmotion.x, event.xmotion.y}));
        }
        SciterProcX(record->api, SCITER_X_MSG_MOUSE(MOUSE_MOVE, MOUSE_BUTTONS(record->mouseButtons), ModifierState(event.xmotion.state), POINT{event.xmotion.x, event.xmotion.y}));
        {
            std::string role;
            HELEMENT hit = 0;
            if (SciterFindElement(record->api, POINT{event.xmotion.x, event.xmotion.y}, &hit) == SCDOM_OK)
            {
                role = NearestRole(hit);
            }
            const int direction = RoleBlocksResize(role) ? -1 : EdgeResizeDirection(*record, event.xmotion.x, event.xmotion.y);
            const int cursor = CursorForResizeDirection(direction);
            if (cursor >= 0)
            {
                DefineCursor(*record, cursor);
            }
            else if (record->cursorId == CURSOR_SIZENWSE || record->cursorId == CURSOR_SIZENESW || record->cursorId == CURSOR_SIZEWE || record->cursorId == CURSOR_SIZENS)
            {
                DefineCursor(*record, CURSOR_ARROW);
            }
        }
        break;
    case ButtonPress:
        OnButton(*record, event.xbutton, true);
        break;
    case ButtonRelease:
        OnButton(*record, event.xbutton, false);
        break;
    case KeyPress:
        OnKey(*record, event.xkey, true);
        break;
    case KeyRelease:
        OnKey(*record, event.xkey, false);
        break;
    case ClientMessage:
        if (event.xclient.message_type == state->wmProtocols && static_cast<Atom>(event.xclient.data.l[0]) == state->wmDelete)
        {
            record->owner->Destroy();
        }
        break;
    case PropertyNotify:
        if (event.xproperty.atom == state->netWmState)
        {
            Atom actualType = 0;
            int actualFormat = 0;
            unsigned long count = 0;
            unsigned long after = 0;
            unsigned char * data = nullptr;
            bool maximized = false;
            if (XGetWindowProperty(state->display, record->window, state->netWmState, 0, 32, False, XA_ATOM, &actualType, &actualFormat, &count, &after, &data) == Success && data != nullptr)
            {
                const Atom * atoms = reinterpret_cast<const Atom *>(data);
                for (unsigned long index = 0; index < count; ++index)
                {
                    if (atoms[index] == state->netWmStateMaxV || atoms[index] == state->netWmStateMaxH)
                    {
                        maximized = true;
                    }
                }
            }
            if (data != nullptr)
            {
                XFree(data);
            }
            record->maximized = maximized;
        }
        break;
    default:
        break;
    }
}

void Heartbeat()
{
    HostData * state = g_state;
    if (state == nullptr)
    {
        return;
    }
    const UINT now = static_cast<UINT>(NowMs());
    std::vector<Window> windows;
    windows.reserve(state->records.size());
    for (const auto & entry : state->records)
    {
        windows.push_back(entry.first);
    }
    for (Window window : windows)
    {
        Record * record = Find(window);
        if (record == nullptr || record->closing)
        {
            continue;
        }
        SciterProcX(record->api, SCITER_X_MSG_HEARTBIT(now));
    }
}

void Pump(bool block)
{
    HostData * state = g_state;
    if (state == nullptr || state->display == nullptr)
    {
        return;
    }
    state->pumpDepth += 1;
    if (block && XPending(state->display) == 0)
    {
        pollfd fds[2]{};
        fds[0].fd = ConnectionNumber(state->display);
        fds[0].events = POLLIN;
        unsigned count = 1;
        if (state->wakeRead >= 0)
        {
            fds[1].fd = state->wakeRead;
            fds[1].events = POLLIN;
            count = 2;
        }
        poll(fds, count, 16);
        if (count == 2 && (fds[1].revents & POLLIN) != 0)
        {
            char buffer[32];
            while (read(state->wakeRead, buffer, sizeof(buffer)) > 0)
            {
            }
        }
    }
    while (XPending(state->display) != 0)
    {
        XEvent event{};
        XNextEvent(state->display, &event);
        if (XFilterEvent(&event, None))
        {
            continue;
        }
        Dispatch(event);
        if (state->quit)
        {
            break;
        }
    }
    if (!state->quit)
    {
        Heartbeat();
    }
    PaintPending();
    state->pumpDepth -= 1;
}

void X11Host::Run()
{
    if (g_state == nullptr)
    {
        return;
    }
    g_state->quit = false;
    while (g_state != nullptr && !g_state->quit)
    {
        Pump(true);
    }
}

void X11Host::RunModal(SciterWindow * window)
{
    if (window == nullptr || g_state == nullptr)
    {
        return;
    }
    while (g_state != nullptr && !g_state->quit && g_state->owners.find(window) != g_state->owners.end())
    {
        Pump(true);
    }
}

void X11Host::Shutdown()
{
    HostData * state = g_state;
    if (state == nullptr)
    {
        return;
    }
    std::vector<SciterWindow *> owners;
    owners.reserve(state->owners.size());
    for (const auto & entry : state->owners)
    {
        owners.push_back(entry.first);
    }
    for (SciterWindow * owner : owners)
    {
        if (owner != nullptr && state->owners.find(owner) != state->owners.end())
        {
            DestroyNative(*owner, true);
        }
    }
    if (state->inputContext != nullptr)
    {
        XDestroyIC(state->inputContext);
        state->inputContext = nullptr;
    }
    if (state->inputMethod != nullptr)
    {
        XCloseIM(state->inputMethod);
        state->inputMethod = nullptr;
    }
    for (Cursor cursor : state->cursors)
    {
        if (cursor != 0 && state->display != nullptr)
        {
            XFreeCursor(state->display, cursor);
        }
    }
    std::memset(state->cursors, 0, sizeof(state->cursors));
    if (state->wakeRead >= 0)
    {
        close(state->wakeRead);
        state->wakeRead = -1;
    }
    if (state->wakeWrite >= 0)
    {
        close(state->wakeWrite);
        state->wakeWrite = -1;
    }
    if (state->display != nullptr)
    {
        XCloseDisplay(state->display);
        state->display = nullptr;
    }
}

} // namespace SciterUI
