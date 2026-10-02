#import <AppKit/AppKit.h>
#include <csignal>
#include "sciter_window.h"
#include "sciter_hwindow.h"
#include "sciter_window_macos.h"
#include <cstring>

// Retain DOM rows while Cocoa tracks the menu, and keep command delivery in
// Sciter's existing MENU_ITEM_CLICK path. Native menu item targets are weak;
// the menu itself owns the action, so there is no separate dangling delegate.
@interface SciterNativeContextMenu : NSMenu <NSMenuDelegate>
{
@public
    std::weak_ptr<SciterUI::SciterWindow> owner;
    SciterElement sourceMenu;
    SciterElement source;
    std::vector<SciterElement> rows;
}
- (void)invokeRow:(NSMenuItem *)item;
@end

@implementation SciterNativeContextMenu
- (void)invokeRow:(NSMenuItem *)item
{
    auto window = owner.lock();
    if (!window || window->IsClosed() || !item.enabled || item.tag < 0 ||
        size_t(item.tag) >= rows.size() || (rows[item.tag].GetState() & SciterElement::STATE_DISABLED)) return;
    BEHAVIOR_EVENT_PARAMS event{};
    event.cmd = MENU_ITEM_CLICK;
    event.he = (HELEMENT)(SCITER_ELEMENT)rows[item.tag];
    event.heTarget = (HELEMENT)(SCITER_ELEMENT)source;
    event.reason = BY_MOUSE_CLICK;
    SBOOL handled = false;
    SciterFireEvent(&event, false, &handled);
}
- (void)menu:(NSMenu *)menu willHighlightItem:(NSMenuItem *)item
{
    (void)menu;
    auto window = owner.lock();
    if (!window || window->IsClosed() || !item || item.isSeparatorItem ||
        item.tag < 0 || size_t(item.tag) >= rows.size()) return;
    BEHAVIOR_EVENT_PARAMS event{};
    event.cmd = MENU_ITEM_ACTIVE;
    event.he = (HELEMENT)(SCITER_ELEMENT)rows[item.tag];
    event.heTarget = (HELEMENT)(SCITER_ELEMENT)source;
    event.reason = BY_MOUSE_CLICK;
    SBOOL handled = false;
    SciterFireEvent(&event, false, &handled);
}
@end

// Cocoa's default terminate: calls exit(), bypassing C++ stack owners and
// leaving emulation workers alive during static destruction. Forward the other
// application delegate methods, but unwind the host loop for native Quit.
@interface SciterApplicationDelegate : NSObject <NSApplicationDelegate>
{
@public
    std::weak_ptr<SciterUI::SciterWindow> mainWindow;
    id<NSApplicationDelegate> previousDelegate;
}
@end

@implementation SciterApplicationDelegate
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    (void)sender;
    if (auto window = mainWindow.lock())
    {
        window->Destroy();
        // A live native handle means either a close veto or an asynchronous
        // close still pending. Let that close path unwind the host loop.
        if (window->GetHandle() != nullptr) return NSTerminateCancel;
    }
    // No native main window remains, including a retained closed wrapper.
    // Stop the host explicitly; NSTerminateNow would call exit() and skip its
    // C++ cleanup just as Cocoa's default terminate: does.
    SciterExec(SCITER_APP_STOP, 0, 0);
    return NSTerminateCancel;
}
- (BOOL)respondsToSelector:(SEL)selector
{
    return [super respondsToSelector:selector] || [previousDelegate respondsToSelector:selector];
}
- (id)forwardingTargetForSelector:(SEL)selector
{
    if ([previousDelegate respondsToSelector:selector]) return previousDelegate;
    return [super forwardingTargetForSelector:selector];
}
- (void)dealloc
{
    [previousDelegate release];
    [super dealloc];
}
@end

namespace SciterUI
{
namespace
{
SciterNativeContextMenu * trackingContextMenu = nil;

void FirePopupEvent(UINT command, SCITER_ELEMENT popup, SCITER_ELEMENT anchor)
{
    // Match the bundled engine's anchored popup notifications explicitly:
    // request/ready/dismissing target the popup with its owner as source;
    // dismissed targets the owner with the retired popup as source.
    BEHAVIOR_EVENT_PARAMS event{};
    event.cmd = command;
    event.he = (HELEMENT)(command == POPUP_DISMISSED ? popup : anchor);
    event.heTarget = (HELEMENT)(command == POPUP_DISMISSED ? anchor : popup);
    SBOOL handled = false;
    SciterFireEvent(&event, false, &handled);
}
}

NSMenu * CreateMacOSContextMenu(std::shared_ptr<SciterWindow> window, SCITER_ELEMENT menuElement,
                              SCITER_ELEMENT source)
{
    SciterElement element(menuElement);
    // Preserve Sciter's rendering/behavior for submenus, alternate menu-item
    // roles and interactive DOM. Never silently flatten unsupported content.
    for (uint32_t i = 0; i < element.GetChildCount(); ++i)
    {
        SciterElement row(element.GetChild(i));
        LPCSTR type = nullptr;
        SciterGetElementType((HELEMENT)(SCITER_ELEMENT)row, &type);
        if (!type || (std::strcmp(type, "hr") != 0 && std::strcmp(type, "li") != 0) ||
            row.FindFirst("menu, [role=menu-item], input, button, select, textarea").IsValid()) return nil;
    }
    auto * menu = [[[SciterNativeContextMenu alloc] initWithTitle:@""] autorelease];
    menu->owner = window;
    menu->sourceMenu = menuElement;
    menu->source = source ? source : menuElement;
    menu.delegate = menu;
    menu.autoenablesItems = NO;
    for (uint32_t i = 0; i < menu->sourceMenu.GetChildCount(); ++i)
    {
        SciterElement row(menu->sourceMenu.GetChild(i));
        LPCSTR type = nullptr;
        SciterGetElementType((HELEMENT)(SCITER_ELEMENT)row, &type);
        if (type && std::strcmp(type, "hr") == 0)
        {
            [menu addItem:NSMenuItem.separatorItem];
            continue;
        }
        if (!type || std::strcmp(type, "li") != 0) continue;
        SciterElement label(row.FindFirst(".menu-item-label"));
        if (!label.IsValid()) label = row;
        std::basic_string<WCHAR> text;
        SciterGetElementTextCB((HELEMENT)(SCITER_ELEMENT)label,
            [](LPCWSTR value, UINT length, LPVOID context) {
                static_cast<std::basic_string<WCHAR> *>(context)->assign(value, length);
            }, &text);
        NSString * title = [[[NSString alloc] initWithCharacters:(const unichar *)text.data()
                                                         length:text.size()] autorelease];
        NSMenuItem * item = [[[NSMenuItem alloc] initWithTitle:title action:@selector(invokeRow:)
                                                keyEquivalent:@""] autorelease];
        item.target = menu;
        item.tag = menu->rows.size();
        item.enabled = !(row.GetState() & SciterElement::STATE_DISABLED);
        menu->rows.push_back(row);
        [menu addItem:item];
    }
    return menu->rows.empty() ? nil : menu;
}

NSPoint MacOSContextMenuPoint(NSView * view, SCITER_POINT point, SciterElement root, NSEvent * event)
{
    NSPoint position;
    const bool mousePress = event && event.window == view.window &&
        (event.type == NSEventTypeRightMouseDown ||
         (event.type == NSEventTypeLeftMouseDown && (event.modifierFlags & NSEventModifierFlagControl)));
    if (mousePress)
    {
        // Native mouse coordinates are already Cocoa points. Do not multiply
        // by the backing scale, or reinterpret them as Sciter device pixels.
        position = [view convertPoint:event.locationInWindow fromView:nil];
    }
    else
    {
        const auto bounds = root.GetLocation(SciterElement::VIEW_RELATIVE | SciterElement::BORDER_BOX);
        const CGFloat scaleX = bounds.right > bounds.left ? NSWidth(view.bounds) / (bounds.right - bounds.left) : 1;
        const CGFloat scaleY = bounds.bottom > bounds.top ? NSHeight(view.bounds) / (bounds.bottom - bounds.top) : 1;
        position = NSMakePoint(NSMinX(view.bounds) + point.x * scaleX,
            view.isFlipped ? NSMinY(view.bounds) + point.y * scaleY : NSMaxY(view.bounds) - point.y * scaleY);
    }
    position.x += 4;
    position.y += view.isFlipped ? 4 : -4;
    return position;
}

bool ShowMacOSContextMenu(std::shared_ptr<SciterWindow> window, SCITER_ELEMENT menuElement, SCITER_POINT point,
                         SCITER_ELEMENT source, bool & nativeShown)
{
    if (!window || window->IsClosed()) return false;
    if (SciterElement(source).GetElementHwnd(true) != window->GetHandle()) return false;
    NSWindow * native = (NSWindow *)const_cast<void *>(window->GetHandle());
    NSView * view = native.contentView;
    if (!view) return false;
    @autoreleasepool
    {
        // Only replace a simple menu with a known owner. Programmatic popups
        // without an owner stay on Sciter's original popup path.
        NSMenu * menu = CreateMacOSContextMenu(window, menuElement, source);
        if (!menu.numberOfItems) return false;
        SciterElement popup(menuElement);
        SciterElement anchor(source);
        FirePopupEvent(POPUP_REQUEST, menuElement, source);
        if (window->IsClosed()) { nativeShown = true; return true; }
        // Request handlers may mutate the DOM, including adding submenus.
        menu = CreateMacOSContextMenu(window, menuElement, source);
        if (!menu.numberOfItems) return false;
        nativeShown = true;
        FirePopupEvent(POPUP_READY, menuElement, source);
        if (window->IsClosed()) return true;
        const NSPoint location = MacOSContextMenuPoint(view, point, SciterElement(window->GetRootElement()), NSApp.currentEvent);
        // The shared window reference spans Cocoa's tracking loop. A close
        // during tracking cannot release the wrapper or the retained DOM rows.
        auto * previousMenu = trackingContextMenu;
        trackingContextMenu = (SciterNativeContextMenu *)menu;
        [menu popUpMenuPositioningItem:nil atLocation:location inView:view];
        trackingContextMenu = previousMenu;
        if (!window->IsClosed())
        {
            FirePopupEvent(POPUP_DISMISSING, menuElement, source);
            if (!window->IsClosed())
            {
                FirePopupEvent(POPUP_DISMISSED, menuElement, source);
            }
        }
        return true;
    }
}

namespace
{
SciterApplicationDelegate * applicationDelegate = nil;
dispatch_source_t terminationSources[2]{};
struct sigaction previousSignalActions[2]{};
constexpr int terminationSignals[2] = {SIGINT, SIGTERM};
}

void InstallMacOSApplicationTerminationHandler(std::shared_ptr<SciterWindow> window)
{
    if (applicationDelegate == nil)
    {
        applicationDelegate = [[SciterApplicationDelegate alloc] init];
        applicationDelegate->previousDelegate = [NSApp.delegate retain];
        NSApp.delegate = applicationDelegate;
        for (int i = 0; i < 2; ++i)
        {
            terminationSources[i] = dispatch_source_create(DISPATCH_SOURCE_TYPE_SIGNAL,
                terminationSignals[i], 0, dispatch_get_main_queue());
            if (terminationSources[i] == nullptr) continue;
            struct sigaction ignored{};
            ignored.sa_handler = SIG_IGN;
            sigemptyset(&ignored.sa_mask);
            sigaction(terminationSignals[i], &ignored, &previousSignalActions[i]);
            dispatch_source_set_event_handler(terminationSources[i], ^{
                // Run shutdown on the UI thread, never inside a signal handler.
                if (applicationDelegate != nil) [NSApp terminate:nil];
            });
            dispatch_resume(terminationSources[i]);
        }
    }
    applicationDelegate->mainWindow = window;
}

void RemoveMacOSApplicationTerminationHandler()
{
    if (applicationDelegate != nil)
    {
        if (NSApp.delegate == applicationDelegate)
        {
            NSApp.delegate = applicationDelegate->previousDelegate;
        }
        for (int i = 0; i < 2; ++i)
        {
            if (terminationSources[i] == nullptr) continue;
            dispatch_source_cancel(terminationSources[i]);
            dispatch_release(terminationSources[i]);
            terminationSources[i] = nullptr;
            sigaction(terminationSignals[i], &previousSignalActions[i], nullptr);
        }
        [applicationDelegate release];
        applicationDelegate = nil;
    }
}

void ScheduleMacOSWindowClose(std::shared_ptr<SciterWindow> window)
{
    if (trackingContextMenu && trackingContextMenu->owner.lock() == window)
        [trackingContextMenu cancelTrackingWithoutAnimation];
    dispatch_async(dispatch_get_main_queue(), ^{
        if (window->GetHandle() != nullptr)
        {
            SciterWindowExec((SciterHWINDOW)window->GetHandle(), SCITER_WINDOW_SET_STATE,
                             SCITER_WINDOW_STATE_CLOSED, TRUE);
        }
    });
}

void DetachMacOSWindowTerminationObserver(const void * handle)
{
    // This SDK defines HWINDOW as NSWindow*. Its SciterCreateWindow returns
    // an NSWindow at runtime, including for child windows; SciterCreateNSView
    // is a separate API. The older API comment saying NSView* is misleading.
    NSWindow * window = (NSWindow *)const_cast<void *>(handle);
    // The Sciter Cocoa content view can outlive its native wing::window when
    // a caller retains DOM elements or the view. Its onAppTerminate: observer
    // then dereferences the freed native window. Retire only that subscription
    // while the NSWindow/contentView are still valid during engine destruction.
    if (window != nil && window.contentView != nil)
    {
        [[NSNotificationCenter defaultCenter] removeObserver:window.contentView
                                                       name:NSApplicationWillTerminateNotification
                                                     object:nil];
    }
}

} // namespace SciterUI
