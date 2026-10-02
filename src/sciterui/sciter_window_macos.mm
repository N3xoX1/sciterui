#import <AppKit/AppKit.h>
#include <csignal>
#include "sciter_window.h"
#include "sciter_hwindow.h"

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
    // an NSWindow at runtime; SciterCreateNSView is a separate API. Keep a
    // native regression test because the older API comment says NSView*.
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
