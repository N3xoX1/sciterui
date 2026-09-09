#include "event_handler.h"
#include "std_string.h"
#include <sciter_element.h>
#include <sciter_handler.h>

#include <value.h>

#include <sciter-x-behavior.h>
#include <sciter-x-api.h>

namespace SciterUI
{

EventHandler::EventHandler(Sciter & sciter, SCITER_ELEMENT element, void * interfacePtr, uint32_t subscription) :
    m_Sciter(sciter),
    m_Element(element),
    m_Interface(interfacePtr),
    m_Subscription(subscription),
    m_MouseDown(false),
    m_InElement(false)
{
}

int EventHandler::ClickHandler(void * tag, SCITER_ELEMENT he, uint32_t evtg, void * prms)
{
    EventHandler * handler = (EventHandler *)tag;
    IClickSink * clickSink = handler != nullptr ? (IClickSink *)handler->m_Interface : nullptr;
    if (evtg == SUBSCRIPTIONS_REQUEST && handler != nullptr)
    {
        uint32_t * p = (uint32_t *)prms;
        *p = handler->m_Subscription;
        return true;
    }
    else if (evtg == HANDLE_INITIALIZATION)
    {
        return true;
    }
    if (evtg == HANDLE_BEHAVIOR_EVENT && clickSink)
    {
        BEHAVIOR_EVENT_PARAMS * p = (BEHAVIOR_EVENT_PARAMS *)prms;
        if (p->cmd == BUTTON_CLICK || p->cmd == HYPERLINK_CLICK)
        {
            clickSink->OnClick(he, p->he, SYNTHESIZED);
            return true;
        }
    }
    else if (evtg == HANDLE_MOUSE && clickSink)
    {
        MOUSE_PARAMS * p = (MOUSE_PARAMS *)prms;
        if (p->cmd == MOUSE_DOWN || p->cmd == ((uint32_t)MOUSE_DOWN | (uint32_t)SINKING))
        {
            if (p->button_state == (uint32_t)MAIN_MOUSE_BUTTON)
            {
                SciterElement element(he);
                if (element.SetCapture())
                {
                    element.SetState(SciterElement::STATE_PRESSED, 0, true);
                    handler->m_MouseDown = true;
                    handler->m_InElement = true;
                }
            }
        }
        else if (p->cmd == MOUSE_UP || p->cmd == ((uint32_t)MOUSE_UP | (uint32_t)SINKING))
        {
            if (handler->m_MouseDown)
            {
                SciterElement element(he);
                element.ReleaseCapture();
                element.SetState(0, SciterElement::STATE_PRESSED, true);

                const bool inElement = handler->m_InElement;
                handler->m_MouseDown = false;
                handler->m_InElement = false;
                if (inElement)
                {
                    return clickSink->OnClick(he, p->target, BY_MOUSE_CLICK);
                }
            }
        }
        else if (p->cmd == MOUSE_MOVE)
        {
            if (handler->m_MouseDown)
            {
                SciterElement element(he);
                SciterElement::RECT rc = element.GetLocation(SciterElement::SELF_RELATIVE | SciterElement::BORDER_BOX);
                POINT & pt = p->pos;
                if (pt.x < rc.left || pt.y < rc.top || pt.x > rc.right || pt.y > rc.bottom)
                {
                    if (handler->m_InElement)
                    {
                        element.SetState(0, SciterElement::STATE_ACTIVE, true);
                        handler->m_InElement = false;
                    }
                }
                else
                {
                    if (!handler->m_InElement)
                    {
                        element.SetState(SciterElement::STATE_ACTIVE, 0, true);
                        handler->m_InElement = true;
                    }
                }
            }
        }
    }
    return false;
}

int EventHandler::DoubleClickHandler(void* tag, SCITER_ELEMENT he, uint32_t evtg, void* prms)
{
    EventHandler * handler = (EventHandler*)tag;
    IDoubleClickSink * clickSink = handler != nullptr ? (IDoubleClickSink*)handler->m_Interface : nullptr;
    if (evtg == SUBSCRIPTIONS_REQUEST && handler != nullptr)
    {
        uint32_t* p = (uint32_t*)prms;
        *p = handler->m_Subscription;
        return true;
    }
    else if (evtg == HANDLE_INITIALIZATION)
    {
        return true;
    }
    if (evtg == HANDLE_MOUSE && clickSink)
    {
        MOUSE_PARAMS* p = (MOUSE_PARAMS*)prms;
        const uint32_t event = p->cmd & ~(uint32_t)SINKING & ~(uint32_t)HANDLED;
        if (event == MOUSE_DCLICK)
        {
            return clickSink->OnDoubleClick(he, p->target);
        }
    }
    return false;
}

int EventHandler::TimerHandler(void * tag, SCITER_ELEMENT he, uint32_t evtg, void * prms)
{
    EventHandler * handler = (EventHandler*)tag;
    ITimerSink* TimerSink = handler != nullptr ? (ITimerSink*)handler->m_Interface : nullptr;
    if (evtg == SUBSCRIPTIONS_REQUEST && handler != nullptr)
    {
        UINT* p = (UINT*)prms;
        *p = handler->m_Subscription;
        return true;
    }
    if (evtg == HANDLE_INITIALIZATION)
    {
        return false;
    }
    if (evtg == HANDLE_TIMER)
    {
        TIMER_PARAMS* p = (TIMER_PARAMS*)prms;
        return TimerSink->OnTimer(he, (uint32_t*)p->timerId);
    }
    return false;
}

int EventHandler::MousedUpDownHandler(void* tag, SCITER_ELEMENT he, uint32_t evtg, void* prms)
{
    EventHandler * handler = (EventHandler*)tag;
    IMouseUpDownSink * mouseUpDownSink = handler != nullptr ? (IMouseUpDownSink*)handler->m_Interface : nullptr;
    if (evtg == SUBSCRIPTIONS_REQUEST && handler != nullptr)
    {
        uint32_t* p = (uint32_t*)prms;
        *p = handler->m_Subscription;
        return true;
    }
    else if (evtg == HANDLE_INITIALIZATION)
    {
        return true;
    }
    else if (evtg == HANDLE_MOUSE && mouseUpDownSink)
    {
        MOUSE_PARAMS * p = (MOUSE_PARAMS*)prms;
        if (p->cmd == MOUSE_DOWN || p->cmd == ((uint32_t)MOUSE_DOWN | (uint32_t)SINKING))
        {
            return mouseUpDownSink->OnMouseDown(he, p->target, p->pos.x, p->pos.y);
        }
        if (p->cmd == MOUSE_UP || p->cmd == ((uint32_t)MOUSE_UP | (uint32_t)SINKING))
        {
            return mouseUpDownSink->OnMouseUp(he, p->target, p->pos.x, p->pos.y);
        }
    }
    return false;
}

int EventHandler::MousedMoveHandler(void * tag, SCITER_ELEMENT he, uint32_t evtg, void * prms)
{
    EventHandler * handler = (EventHandler*)tag;
    IMouseMoveSink * mouseMoveSink = handler != nullptr ? (IMouseMoveSink*)handler->m_Interface : nullptr;
    if (evtg == SUBSCRIPTIONS_REQUEST && handler != nullptr)
    {
        uint32_t* p = (uint32_t*)prms;
        *p = handler->m_Subscription;
        return true;
    }
    else if (evtg == HANDLE_INITIALIZATION)
    {
        return true;
    }
    else if (evtg == HANDLE_MOUSE && mouseMoveSink)
    {
        MOUSE_PARAMS * p = (MOUSE_PARAMS *)prms;
        if (p->cmd == MOUSE_MOVE || p->cmd == ((uint32_t)MOUSE_MOVE | (uint32_t)SINKING))
        {
            return mouseMoveSink->OnMouseMove(he, p->target, p->pos.x, p->pos.y);
        }
    }
    return false;
}

int EventHandler::KeyHandler(void* tag, SCITER_ELEMENT he, uint32_t evtg, void* prms)
{
    EventHandler * handler = (EventHandler *)tag;
    if (evtg == SUBSCRIPTIONS_REQUEST && handler != nullptr)
    {
        uint32_t * p = (uint32_t *)prms;
        *p = handler->m_Subscription;
        return true;
    }
    else if (evtg == HANDLE_INITIALIZATION)
    {
        return true;
    }
    else if (evtg == HANDLE_KEY)
    {
        IKeySink * keySink = handler != nullptr ? (IKeySink*)handler->m_Interface : nullptr;
        KEY_PARAMS * p = (KEY_PARAMS*)prms;
        if (p->cmd == KEY_DOWN)
        {
            return keySink->OnKeyDown(he, p->target, (SciterKeys)p->key_code, p->alt_state);
        }
        if (p->cmd == KEY_UP)
        {
            return keySink->OnKeyUp(he, p->target, (SciterKeys)p->key_code, p->alt_state);
        }
        if (p->cmd == KEY_CHAR)
        {
            return keySink->OnKeyChar(he, p->target, (SciterKeys)p->key_code, p->alt_state);
        }
    }
    return false;
}

int EventHandler::ResizeHandler(void* tag, SCITER_ELEMENT he, uint32_t evtg, void* prms)
{
    EventHandler* handler = (EventHandler*)tag;
    if (evtg == SUBSCRIPTIONS_REQUEST && handler != nullptr)
    {
        uint32_t* p = (uint32_t*)prms;
        *p = handler->m_Subscription;
        return true;
    }
    else if (evtg == HANDLE_INITIALIZATION)
    {
        return true;
    }
    else if (evtg == HANDLE_SIZE)
    {
        IResizeSink * resizeSink = handler != nullptr ? (IResizeSink *)handler->m_Interface : nullptr;
        if (resizeSink)
        {
            return resizeSink->OnSizeChanged(he);
        }
        return false;
    }
    return false;
}

int EventHandler::ForwardBehaviorHandler(void* tag, SCITER_ELEMENT he, uint32_t evtg, void* prms)
{
    EventHandler* handler = (EventHandler*)tag;
    if (evtg == SUBSCRIPTIONS_REQUEST && handler != nullptr)
    {
        uint32_t* p = (uint32_t*)prms;
        *p = handler->m_Subscription;
        return true;
    }
    else if (evtg == HANDLE_INITIALIZATION)
    {
        return true;
    }
    else if (evtg == HANDLE_BEHAVIOR_EVENT)
    {
        BEHAVIOR_EVENT_PARAMS * p = (BEHAVIOR_EVENT_PARAMS*)prms;
        if ((p->cmd & (SINKING | HANDLED)) == 0)
        {
            BEHAVIOR_EVENT_PARAMS params = {};
            params.cmd = p->cmd;
            params.heTarget = (HELEMENT)handler->m_Interface;
            params.he = (HELEMENT)handler->m_Interface;
            params.name = p->name;
            params.data = p->data;
            SBOOL handled = false;
            SCDOM_RESULT r = SciterFireEvent(&params, false, &handled);
            assert(r == SCDOM_OK); (void)r;
            return handled != 0;
        }
    }
    return false;
}

int EventHandler::StateChangeHandler(void* tag, SCITER_ELEMENT he, uint32_t evtg, void* prms)
{
    EventHandler* handler = (EventHandler*)tag;
    if (evtg == SUBSCRIPTIONS_REQUEST && handler != nullptr)
    {
        uint32_t* p = (uint32_t*)prms;
        *p = handler->m_Subscription;
        return true;
    }
    else if (evtg == HANDLE_INITIALIZATION)
    {
        return true;
    }
    else if (evtg == HANDLE_BEHAVIOR_EVENT)
    {
        BEHAVIOR_EVENT_PARAMS* p = (BEHAVIOR_EVENT_PARAMS*)prms;
        if (p->cmd == VALUE_CHANGED)
        {
            IStateChangeSink * stateChangeSink = handler != nullptr ? (IStateChangeSink*)handler->m_Interface : nullptr;
            if (stateChangeSink)
            {
                return stateChangeSink->OnStateChange(he, evtg, prms);
            }
        }
    }
    return false;
}

int EventHandler::EventSinkHandler(void* tag, SCITER_ELEMENT he, uint32_t evtg, void* prms)
{
    EventHandler* handler = (EventHandler*)tag;
    if (evtg == SUBSCRIPTIONS_REQUEST && handler != nullptr)
    {
        uint32_t* p = (uint32_t*)prms;
        *p = handler->m_Subscription;
        return true;
    }
    else if (evtg == HANDLE_INITIALIZATION)
    {
        return true;
    }
    else if (evtg == HANDLE_BEHAVIOR_EVENT)
    {
        BEHAVIOR_EVENT_PARAMS* p = (BEHAVIOR_EVENT_PARAMS*)prms;
        if ((p->cmd & (SINKING | HANDLED)) == 0)
        {
            IEventSink * eventSink = handler != nullptr ? (IEventSink*)handler->m_Interface : nullptr;
            if (eventSink)
            {
                return eventSink->OnEvent(p->he, p->heTarget, p->cmd, p->reason);
            }
        }
    }
    return false;
}

} // namespace SciterUI
