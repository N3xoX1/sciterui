#pragma once
#include <sciter_ui.h>
#include <stdint.h>

namespace SciterUI
{

class Sciter;

class EventHandler
{
public:
    EventHandler(Sciter & sciter, SCITER_ELEMENT element, void * interfacePtr, uint32_t subscription);

    static int sui_callback ClickHandler(void * tag, SCITER_ELEMENT he, uint32_t evtg, void * prms);
    static int sui_callback DoubleClickHandler(void * tag, SCITER_ELEMENT he, uint32_t evtg, void * prms);
    static int sui_callback TimerHandler(void * tag, SCITER_ELEMENT he, uint32_t evtg, void * prms);
    static int sui_callback KeyHandler(void * tag, SCITER_ELEMENT he, uint32_t evtg, void * prms);
    static int sui_callback MousedUpDownHandler(void * tag, SCITER_ELEMENT he, uint32_t evtg, void * prms);
    static int sui_callback MousedMoveHandler(void * tag, SCITER_ELEMENT he, uint32_t evtg, void * prms);
    static int sui_callback ResizeHandler(void * tag, SCITER_ELEMENT he, uint32_t evtg, void * prms);
    static int sui_callback ForwardBehaviorHandler(void* tag, SCITER_ELEMENT he, uint32_t evtg, void* prms);
    static int sui_callback StateChangeHandler(void* tag, SCITER_ELEMENT he, uint32_t evtg, void* prms);
    static int sui_callback EventSinkHandler(void* tag, SCITER_ELEMENT he, uint32_t evtg, void* prms);

private:
    EventHandler() = delete;
    EventHandler(const EventHandler &) = delete;
    EventHandler & operator=(const EventHandler &) = delete;

    Sciter & m_Sciter;
    SCITER_ELEMENT m_Element;
    void * m_Interface;
    uint32_t m_Subscription;
    bool m_MouseDown;
    bool m_InElement;
};

} // namespace SciterUI
