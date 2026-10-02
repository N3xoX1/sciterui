#import <AppKit/AppKit.h>
#include "sciter.h"
#include "sciter_hwindow.h"
#include <sciter_element.h>
#include <sciter_handler.h>
#include <cstdio>
#include <csignal>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <vector>

namespace
{
unsigned stopRequests = 0;
bool recordStops = false;
decltype(ISciterAPI::SciterExec) originalExec = nullptr;
INT_PTR SCAPI RecordExec(UINT command, UINT_PTR first, UINT_PTR second)
{
    if (recordStops)
    {
        if (command == SCITER_APP_STOP) ++stopRequests;
        return 1;
    }
    return originalExec(command, first, second);
}

void Require(bool condition, const char * message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

void Pump()
{
    for (int i = 0; i < 4; ++i)
    {
        if (!SciterExec(SCITER_APP_LOOP_HEARTBIT, 0, 0) ||
            !SciterExec(SCITER_APP_LOOP_ITERATION, 0, 0))
        {
            break;
        }
    }
}

struct CloseSink : IWindowDestroySink, IWindowCloseSink, ITimerSink
{
    ISciterWindow * window = nullptr;
    unsigned int notifications = 0;
    unsigned int closeRequests = 0;
    bool allowClose = true;
    void OnWindowDestroy(HWINDOW handle) override
    {
        Require(window != nullptr && window->GetHandle() == handle, "invalid destroy notification");
        ++notifications;
        Require(!window->Destroy(), "recursive destruction must be ignored");
        window = nullptr;
    }
    bool OnWindowCloseRequest(HWINDOW) override { ++closeRequests; return allowClose; }
    bool OnTimer(SCITER_ELEMENT, uint32_t *) override
    {
        Require(window != nullptr, "timer dispatched to a closed window");
        window->Destroy();
        return false;
    }
};

struct ClickSink : IClickSink
{
    bool OnClick(SCITER_ELEMENT, SCITER_ELEMENT, uint32_t) override { return false; }
};
} // namespace

int main(int argc, char ** argv)
{
    @autoreleasepool
    {
        try
        {
            Require(argc == 2, "expected Sciter library path");
            void * library = dlopen(argv[1], RTLD_LOCAL | RTLD_NOW);
            Require(library != nullptr, "cannot load Sciter");
            auto getApi = reinterpret_cast<ISciterAPI * (*)()>(dlsym(library, "SciterAPI"));
            Require(getApi != nullptr, "SciterAPI missing");
            // SAPI caches its table on first use. Install the recording wrapper
            // once, before initialization, and forward normal engine calls.
            ISciterAPI api = *getApi();
            originalExec = api.SciterExec;
            api.SciterExec = RecordExec;
            _SAPI(&api);

            char tempPath[] = "/tmp/nxemu-window-tests-XXXXXX";
            Require(mkdtemp(tempPath) != nullptr, "cannot create fixture directory");
            const std::filesystem::path fixture(tempPath);
            std::filesystem::create_directories(fixture / "english/html");
            std::ofstream(fixture / "english/html/window.html") << "<html><body><button id='retired'>lifetime test</button></body></html>";
            SciterUI::Sciter sciter(tempPath);
            Require(sciter.Initialize("english", "english", false), "Sciter initialization failed");

            CloseSink mainSink;
            ClickSink clickSink;
            Require(sciter.WindowCreate(nullptr, "window.html", 0, 0, 400, 300,
                                        SUIW_MAIN, mainSink.window), "main window creation failed");
            mainSink.window->OnDestroySinkAdd(&mainSink);
            mainSink.window->OnCloseSinkAdd(&mainSink);
            mainSink.window->Show();
            id nativeHandle = (id)const_cast<void *>(mainSink.window->GetHandle());
            Require([nativeHandle isKindOfClass:NSWindow.class], "SciterCreateWindow must return NSWindow in this SDK");
            std::printf("SciterCreateWindow returned %s (NSWindow: %d, NSView: %d)\n",
                object_getClassName(nativeHandle), [nativeHandle isKindOfClass:NSWindow.class],
                [nativeHandle isKindOfClass:NSView.class]);

            std::vector<NSView *> retainedViews;
            for (unsigned int i = 0; i < 32; ++i)
            {
                CloseSink sink;
                Require(sciter.WindowCreate(mainSink.window->GetHandle(), "window.html", 0, 0, 200, 150,
                                            SUIW_CHILD, sink.window), "child creation failed");
                sink.window->OnDestroySinkAdd(&sink);
                sink.window->OnCloseSinkAdd(&sink);
                auto weak = static_cast<SciterUI::SciterWindow *>(sink.window)->weak_from_this();
                NSWindow * native = (NSWindow *)const_cast<void *>(sink.window->GetHandle());
                Require([native isKindOfClass:NSWindow.class], "child handle must also be an NSWindow in this SDK");
                retainedViews.push_back([native.contentView retain]);
                {
                    SciterElement root(sink.window->GetRootElement());
                    {
                        SciterElement button(root.GetElementByID("retired"));
                        Require(sciter.AttachHandler(button, IID_ICLICKSINK, &clickSink), "click handler failed");
                    }
                    // Remove the element and all caller references before window
                    // destruction. The registry must not keep a dangling handle.
                    const char * replacement = "<body>replaced DOM</body>";
                    root.SetHTML((const uint8_t *)replacement, std::strlen(replacement));
                }
                // Close refusal must leave the wrapper and its native view usable.
                sink.allowClose = false;
                Require(!sink.window->Destroy() && !weak.expired(), "refused close destroyed the window");
                sink.allowClose = true;
                if (i % 2 == 0)
                {
                    Require(sink.window->Destroy(), "explicit close failed");
                }
                else
                {
                    // Native/script closure bypasses SciterWindow::Destroy().
                    SciterWindowExec((SciterUI::SciterHWINDOW)sink.window->GetHandle(),
                                     SCITER_WINDOW_SET_STATE, SCITER_WINDOW_STATE_CLOSED, TRUE);
                }
                Pump();
                Require(sink.notifications == 1 && sink.window == nullptr, "child did not close exactly once");
                Require(weak.expired(), "closed wrapper was retained");
            }

            CloseSink modal;
            std::puts("native and explicit close cycles passed");
            std::fflush(stdout);
            Require(sciter.WindowCreate(mainSink.window->GetHandle(), "window.html", 0, 0, 200, 150,
                                        SUIW_CHILD, modal.window), "modal creation failed");
            modal.window->OnDestroySinkAdd(&modal);
            auto modalWeak = static_cast<SciterUI::SciterWindow *>(modal.window)->weak_from_this();
            {
                SciterElement root(modal.window->GetRootElement());
                Require(sciter.AttachHandler(root, IID_ITIMERSINK, static_cast<ITimerSink *>(&modal)), "timer handler failed");
                root.SetTimer(10, nullptr);
                modal.window->RunModal();
            }
            Require(modal.notifications == 1 && modalWeak.expired(), "modal wrapper outlived its loop");

            ISciterWindow * failed = nullptr;
            std::puts("modal close passed");
            std::fflush(stdout);
            Require(!sciter.WindowCreate(mainSink.window->GetHandle(), "missing.html", 0, 0, 200, 150,
                                         SUIW_CHILD, failed), "missing HTML should fail");
            Require(failed == nullptr, "failed creation published a window");
            Pump();
            std::puts("failed creation passed");
            std::fflush(stdout);

            // Reuse freed allocation slots before Cocoa notifies the retained
            // closed views. The unfixed engine calls a freed native delegate here.
            std::vector<std::unique_ptr<unsigned char[]>> churn;
            for (unsigned int i = 0; i < 20000; ++i)
            {
                const size_t size = 128 + (i % 40) * 16;
                auto allocation = std::make_unique<unsigned char[]>(size);
                std::memset(allocation.get(), 0x5a, size);
                churn.push_back(std::move(allocation));
            }
            // Native Quit must respect refusal and return to the C++ host;
            // NSApplication's default implementation exits the process here.
            mainSink.allowClose = false;
            kill(getpid(), SIGTERM);
            Pump();
            Require(mainSink.notifications == 0, "native quit ignored refusal");
            Require(mainSink.closeRequests != 0, "SIGTERM did not reach the close handler");
            mainSink.allowClose = true;
            auto closedMain = static_cast<SciterUI::SciterWindow *>(mainSink.window)->shared_from_this();
            auto mainWeak = closedMain->weak_from_this();
            [NSApp terminate:nil];
            Pump();
            Require(mainSink.notifications == 1, "native quit did not close main window");
            [[NSNotificationCenter defaultCenter] postNotificationName:NSApplicationWillTerminateNotification
                                                               object:NSApp];
            Pump();
            Require(mainSink.notifications == 1, "live main window did not receive termination");
            // Quit after native destruction must still unwind the C++ host,
            // even if a caller retains the wrapper or its weak reference expires.
            recordStops = true;
            auto delegate = NSApp.delegate;
            Require([delegate applicationShouldTerminate:NSApp] == NSTerminateCancel,
                    "quit bypassed C++ host cleanup");
            Require(stopRequests == 1, "quit with a retained closed wrapper did not stop the host loop");
            closedMain.reset();
            Require(mainWeak.expired(), "closed main wrapper is unexpectedly retained");
            Require([delegate applicationShouldTerminate:NSApp] == NSTerminateCancel,
                    "quit with an expired main window bypassed C++ cleanup");
            Require(stopRequests == 2, "quit with an expired main window did not stop the host loop");
            recordStops = false;
            for (NSView * view : retainedViews)
            {
                [view release];
            }
            sciter.Shutdown();
            std::filesystem::remove_all(fixture);
            std::puts("32 native/explicit closes, refusal, recursive close, modal loop, failed creation, and Cocoa termination passed");
            return 0;
        }
        catch (const std::exception & error)
        {
            std::fprintf(stderr, "%s\n", error.what());
            return 1;
        }
    }
}
