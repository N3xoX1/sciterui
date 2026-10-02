#include "event_handler.h"
#include "sciter.h"
#include "sciter_hwindow.h"
#include <sciter_handler.h>
#include <cstdio>
#include <stdexcept>

namespace
{
void Require(bool condition, const char * message)
{
    if (!condition) throw std::runtime_error(message);
}

// Model a sink that detaches itself or closes its window synchronously. The
// registry's last reference disappears while the callback is still executing.
struct Sink : IClickSink, IDoubleClickSink, ITimerSink, IMouseUpDownSink,
              IMouseMoveSink, IKeySink, IResizeSink, IStateChangeSink,
              IEventSink
{
    std::shared_ptr<SciterUI::EventHandler> owner;
    std::weak_ptr<SciterUI::EventHandler> weak;
    unsigned calls = 0;

    bool Detach()
    {
        ++calls;
        owner.reset();
        Require(!weak.expired(), "handler died inside its sink callback");
        return true;
    }
    bool OnClick(SCITER_ELEMENT, SCITER_ELEMENT, uint32_t) override { return Detach(); }
    bool OnDoubleClick(SCITER_ELEMENT, SCITER_ELEMENT) override { return Detach(); }
    bool OnTimer(SCITER_ELEMENT, uint32_t *) override { return Detach(); }
    bool OnMouseUp(SCITER_ELEMENT, SCITER_ELEMENT, uint32_t, uint32_t) override { return Detach(); }
    bool OnMouseDown(SCITER_ELEMENT, SCITER_ELEMENT, uint32_t, uint32_t) override { return Detach(); }
    bool OnMouseMove(SCITER_ELEMENT, SCITER_ELEMENT, uint32_t, uint32_t) override { return Detach(); }
    bool OnKeyDown(SCITER_ELEMENT, SCITER_ELEMENT, SciterKeys, uint32_t) override { return Detach(); }
    bool OnKeyUp(SCITER_ELEMENT, SCITER_ELEMENT, SciterKeys, uint32_t) override { return Detach(); }
    bool OnKeyChar(SCITER_ELEMENT, SCITER_ELEMENT, SciterKeys, uint32_t) override { return Detach(); }
    bool OnSizeChanged(SCITER_ELEMENT) override { return Detach(); }
    bool OnStateChange(SCITER_ELEMENT, uint32_t, void *) override { return Detach(); }
    bool OnEvent(SCITER_ELEMENT, SCITER_ELEMENT, uint32_t, uint64_t) override { return Detach(); }
};

Sink * forwardingSink;
SCDOM_RESULT SCAPI FireEvent(const BEHAVIOR_EVENT_PARAMS *, SBOOL, SBOOL * handled)
{
    *handled = forwardingSink->Detach();
    return SCDOM_OK;
}
UINT SCAPI InitValue(VALUE * value) { *value = VALUE{}; return HV_OK; }
UINT SCAPI CopyValue(VALUE * destination, const VALUE * source) { *destination = *source; return HV_OK; }
} // namespace

int main()
{
    try
    {
        ISciterAPI api{};
        api.SciterFireEvent = FireEvent;
        api.ValueInit = InitValue;
        api.ValueClear = InitValue;
        api.ValueCopy = CopyValue;
        _SAPI(&api);
        SciterUI::Sciter sciter(".");
        Sink sink;
        using Handler = SciterUI::EventHandler;
        auto exercise = [&](const char * name, decltype(&Handler::ClickHandler) proc,
                            void * interfacePtr, uint32_t event, void * params) {
            sink.owner = std::make_shared<Handler>(sciter, nullptr, interfacePtr, event);
            sink.weak = sink.owner;
            sink.calls = 0;
            Require(proc(sink.owner.get(), nullptr, event, params), "callback result changed");
            Require(sink.calls == 1, "sink was not called exactly once");
            Require(sink.weak.expired(), "handler leaked after callback returned");
            Require(!proc(nullptr, nullptr, event, params), "null tag was accepted");
            std::printf("PASS: %s detaches during callback\n", name);
        };
        BEHAVIOR_EVENT_PARAMS behavior{};
        behavior.cmd = BUTTON_CLICK;
        exercise("click", Handler::ClickHandler, static_cast<IClickSink *>(&sink), HANDLE_BEHAVIOR_EVENT, &behavior);
        MOUSE_PARAMS mouse{};
        mouse.cmd = MOUSE_DCLICK;
        exercise("double click", Handler::DoubleClickHandler, static_cast<IDoubleClickSink *>(&sink), HANDLE_MOUSE, &mouse);
        TIMER_PARAMS timer{};
        exercise("timer", Handler::TimerHandler, static_cast<ITimerSink *>(&sink), HANDLE_TIMER, &timer);
        for (auto command : {MOUSE_UP, MOUSE_DOWN})
        {
            mouse.cmd = command;
            exercise("mouse up/down", Handler::MousedUpDownHandler, static_cast<IMouseUpDownSink *>(&sink), HANDLE_MOUSE, &mouse);
        }
        mouse.cmd = MOUSE_MOVE;
        exercise("mouse move", Handler::MousedMoveHandler, static_cast<IMouseMoveSink *>(&sink), HANDLE_MOUSE, &mouse);
        KEY_PARAMS key{};
        for (auto command : {KEY_DOWN, KEY_UP, KEY_CHAR})
        {
            key.cmd = command;
            exercise("key", Handler::KeyHandler, static_cast<IKeySink *>(&sink), HANDLE_KEY, &key);
        }
        exercise("resize", Handler::ResizeHandler, static_cast<IResizeSink *>(&sink), HANDLE_SIZE, nullptr);
        behavior.cmd = VALUE_CHANGED;
        exercise("state change", Handler::StateChangeHandler, static_cast<IStateChangeSink *>(&sink), HANDLE_BEHAVIOR_EVENT, &behavior);
        exercise("event", Handler::EventSinkHandler, static_cast<IEventSink *>(&sink), HANDLE_BEHAVIOR_EVENT, &behavior);
        forwardingSink = &sink;
        exercise("forwarded behavior", Handler::ForwardBehaviorHandler, nullptr, HANDLE_BEHAVIOR_EVENT, &behavior);
        return 0;
    }
    catch (const std::exception & error)
    {
        std::fprintf(stderr, "FAIL: %s\n", error.what());
        return 1;
    }
}
