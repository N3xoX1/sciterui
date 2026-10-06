#pragma once
#include <sciter_ui.h>
#include <stdint.h>
#include <memory>

namespace SciterUI
{

class Sciter;
class SciterWindow;

class EventHandler : public std::enable_shared_from_this<EventHandler>
{
    friend class SciterWindow;

public:
    EventHandler(Sciter & sciter, SCITER_ELEMENT element, void * interfacePtr, uint32_t subscription);

    template <int (EventHandler::*Fn)(SCITER_ELEMENT, uint32_t, void *)>
    static int sui_callback Dispatch(void * tag, SCITER_ELEMENT he, uint32_t evtg, void * prms);

private:
    EventHandler() = delete;
    EventHandler(const EventHandler &) = delete;
    EventHandler & operator=(const EventHandler &) = delete;

    int OnClick(SCITER_ELEMENT he, uint32_t evtg, void * prms);
    int OnDoubleClick(SCITER_ELEMENT he, uint32_t evtg, void * prms);
    int OnTimer(SCITER_ELEMENT he, uint32_t evtg, void * prms);
    int OnKey(SCITER_ELEMENT he, uint32_t evtg, void * prms);
    int OnMouseUpDown(SCITER_ELEMENT he, uint32_t evtg, void * prms);
    int OnContextMenu(SCITER_ELEMENT he, uint32_t evtg, void * prms);
    int OnMouseMove(SCITER_ELEMENT he, uint32_t evtg, void * prms);
    int OnResize(SCITER_ELEMENT he, uint32_t evtg, void * prms);
    int OnForwardBehavior(SCITER_ELEMENT he, uint32_t evtg, void * prms);
    int OnStateChange(SCITER_ELEMENT he, uint32_t evtg, void * prms);
    int OnEventSink(SCITER_ELEMENT he, uint32_t evtg, void * prms);

    std::shared_ptr<EventHandler> m_engineReference;
    Sciter & m_Sciter;
    SCITER_ELEMENT m_Element;
    void * m_Interface;
    uint32_t m_Subscription;
    bool m_MouseDown;
    bool m_InElement;
};

extern template int sui_callback EventHandler::Dispatch<&EventHandler::OnClick>(void *, SCITER_ELEMENT, uint32_t, void *);
extern template int sui_callback EventHandler::Dispatch<&EventHandler::OnDoubleClick>(void *, SCITER_ELEMENT, uint32_t, void *);
extern template int sui_callback EventHandler::Dispatch<&EventHandler::OnTimer>(void *, SCITER_ELEMENT, uint32_t, void *);
extern template int sui_callback EventHandler::Dispatch<&EventHandler::OnKey>(void *, SCITER_ELEMENT, uint32_t, void *);
extern template int sui_callback EventHandler::Dispatch<&EventHandler::OnMouseUpDown>(void *, SCITER_ELEMENT, uint32_t, void *);
extern template int sui_callback EventHandler::Dispatch<&EventHandler::OnContextMenu>(void *, SCITER_ELEMENT, uint32_t, void *);
extern template int sui_callback EventHandler::Dispatch<&EventHandler::OnMouseMove>(void *, SCITER_ELEMENT, uint32_t, void *);
extern template int sui_callback EventHandler::Dispatch<&EventHandler::OnResize>(void *, SCITER_ELEMENT, uint32_t, void *);
extern template int sui_callback EventHandler::Dispatch<&EventHandler::OnForwardBehavior>(void *, SCITER_ELEMENT, uint32_t, void *);
extern template int sui_callback EventHandler::Dispatch<&EventHandler::OnStateChange>(void *, SCITER_ELEMENT, uint32_t, void *);
extern template int sui_callback EventHandler::Dispatch<&EventHandler::OnEventSink>(void *, SCITER_ELEMENT, uint32_t, void *);

} // namespace SciterUI
