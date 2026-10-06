#include "event_handler.h"
#include "sciter.h"
#include "std_string.h"
#include <sciter_element.h>
#include <sciter_handler.h>

#include <value.h>

#include <sciter-x-api.h>
#include <sciter-x-behavior.h>

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

template <int (EventHandler::*Fn)(SCITER_ELEMENT, uint32_t, void *)>
int sui_callback EventHandler::Dispatch(void * tag, SCITER_ELEMENT he, uint32_t evtg, void * prms)
{
    EventHandler * handler = (EventHandler *)tag;
    if (handler == nullptr)
    {
        return false;
    }
    if (evtg == HANDLE_INITIALIZATION)
    {
        if (prms != nullptr)
        {
            const INITIALIZATION_PARAMS * p = (const INITIALIZATION_PARAMS *)prms;
            if (p->cmd == BEHAVIOR_ATTACH)
            {
                handler->m_engineReference = handler->shared_from_this();
            }
            else if (p->cmd == BEHAVIOR_DETACH)
            {
                handler->m_engineReference.reset();
            }
        }
        return true;
    }

    if (evtg == SUBSCRIPTIONS_REQUEST)
    {
        *(uint32_t *)prms = handler->m_Subscription;
        return true;
    }
    return (handler->*Fn)(he, evtg, prms);
}

template int sui_callback EventHandler::Dispatch<&EventHandler::OnClick>(void *, SCITER_ELEMENT, uint32_t, void *);
template int sui_callback EventHandler::Dispatch<&EventHandler::OnDoubleClick>(void *, SCITER_ELEMENT, uint32_t, void *);
template int sui_callback EventHandler::Dispatch<&EventHandler::OnTimer>(void *, SCITER_ELEMENT, uint32_t, void *);
template int sui_callback EventHandler::Dispatch<&EventHandler::OnKey>(void *, SCITER_ELEMENT, uint32_t, void *);
template int sui_callback EventHandler::Dispatch<&EventHandler::OnMouseUpDown>(void *, SCITER_ELEMENT, uint32_t, void *);
template int sui_callback EventHandler::Dispatch<&EventHandler::OnContextMenu>(void *, SCITER_ELEMENT, uint32_t, void *);
template int sui_callback EventHandler::Dispatch<&EventHandler::OnMouseMove>(void *, SCITER_ELEMENT, uint32_t, void *);
template int sui_callback EventHandler::Dispatch<&EventHandler::OnResize>(void *, SCITER_ELEMENT, uint32_t, void *);
template int sui_callback EventHandler::Dispatch<&EventHandler::OnForwardBehavior>(void *, SCITER_ELEMENT, uint32_t, void *);
template int sui_callback EventHandler::Dispatch<&EventHandler::OnStateChange>(void *, SCITER_ELEMENT, uint32_t, void *);
template int sui_callback EventHandler::Dispatch<&EventHandler::OnEventSink>(void *, SCITER_ELEMENT, uint32_t, void *);

int EventHandler::OnClick(SCITER_ELEMENT he, uint32_t evtg, void * prms)
{
    IClickSink * clickSink = (IClickSink *)m_Interface;
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
        if (p->cmd == MOUSE_DOWN)
        {
            if (p->button_state == (uint32_t)MAIN_MOUSE_BUTTON)
            {
                SciterElement element(he);
                if (element.SetCapture())
                {
                    element.SetState(SciterElement::STATE_PRESSED, 0, true);
                    m_MouseDown = true;
                    m_InElement = true;
                }
            }
        }
        else if (p->cmd == MOUSE_UP)
        {
            if (m_MouseDown)
            {
                SciterElement element(he);
                element.ReleaseCapture();
                element.SetState(0, SciterElement::STATE_PRESSED, true);

                const bool inElement = m_InElement;
                m_MouseDown = false;
                m_InElement = false;
                if (inElement)
                {
                    return clickSink->OnClick(he, p->target, BY_MOUSE_CLICK);
                }
            }
        }
        else if (p->cmd == MOUSE_MOVE)
        {
            if (m_MouseDown)
            {
                SciterElement element(he);
                SciterElement::RECT rc = element.GetLocation(SciterElement::SELF_RELATIVE | SciterElement::BORDER_BOX);
                POINT & pt = p->pos;
                if (pt.x < rc.left || pt.y < rc.top || pt.x > rc.right || pt.y > rc.bottom)
                {
                    if (m_InElement)
                    {
                        element.SetState(0, SciterElement::STATE_ACTIVE, true);
                        m_InElement = false;
                    }
                }
                else
                {
                    if (!m_InElement)
                    {
                        element.SetState(SciterElement::STATE_ACTIVE, 0, true);
                        m_InElement = true;
                    }
                }
            }
        }
    }
    return false;
}

int EventHandler::OnDoubleClick(SCITER_ELEMENT he, uint32_t evtg, void * prms)
{
    IDoubleClickSink * clickSink = (IDoubleClickSink *)m_Interface;
    if (evtg == HANDLE_MOUSE && clickSink)
    {
        MOUSE_PARAMS * p = (MOUSE_PARAMS *)prms;
        const uint32_t event = p->cmd & ~(uint32_t)SINKING & ~(uint32_t)HANDLED;
        if (event == MOUSE_DCLICK)
        {
            return clickSink->OnDoubleClick(he, p->target);
        }
    }
    return false;
}

int EventHandler::OnTimer(SCITER_ELEMENT he, uint32_t evtg, void * prms)
{
    if (evtg == HANDLE_TIMER)
    {
        ITimerSink * TimerSink = (ITimerSink *)m_Interface;
        TIMER_PARAMS * p = (TIMER_PARAMS *)prms;
        return TimerSink->OnTimer(he, (uint32_t *)p->timerId);
    }
    return false;
}

int EventHandler::OnMouseUpDown(SCITER_ELEMENT he, uint32_t evtg, void * prms)
{
    IMouseUpDownSink * mouseUpDownSink = (IMouseUpDownSink *)m_Interface;
    if (evtg == HANDLE_MOUSE && mouseUpDownSink)
    {
        MOUSE_PARAMS * p = (MOUSE_PARAMS *)prms;
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

int EventHandler::OnContextMenu(SCITER_ELEMENT he, uint32_t evtg, void * prms)
{
    if (evtg == HANDLE_MOUSE)
    {
        MOUSE_PARAMS * p = (MOUSE_PARAMS *)prms;
        if (p->cmd == ((uint32_t)MOUSE_UP | (uint32_t)SINKING) && m_MouseDown)
        {
            m_MouseDown = false;
            return true;
        }
        bool contextPress = p->button_state == PROP_MOUSE_BUTTON;
#ifdef __APPLE__
        contextPress = contextPress || (p->button_state == MAIN_MOUSE_BUTTON && (p->alt_state & KEYBOARD_STATE_CONTROL) != 0);
#endif
        if (p->cmd != ((uint32_t)MOUSE_DOWN | (uint32_t)SINKING))
        {
            return false;
        }
        m_MouseDown = false;
        if (!contextPress)
        {
            return false;
        }
        IContextMenuSink * sink = (IContextMenuSink *)m_Interface;
        Sciter::ContextMenuScope context(m_Sciter, p->target);
        const bool handled = sink && sink->OnContextMenu(he, p->target, p->pos_view.x, p->pos_view.y);
        m_MouseDown = handled && !context.nativeShown;
        return handled;
    }
    return false;
}

int EventHandler::OnMouseMove(SCITER_ELEMENT he, uint32_t evtg, void * prms)
{
    IMouseMoveSink * mouseMoveSink = (IMouseMoveSink *)m_Interface;
    if (evtg == HANDLE_MOUSE && mouseMoveSink)
    {
        MOUSE_PARAMS * p = (MOUSE_PARAMS *)prms;
        if (p->cmd == MOUSE_MOVE || p->cmd == ((uint32_t)MOUSE_MOVE | (uint32_t)SINKING))
        {
            return mouseMoveSink->OnMouseMove(he, p->target, p->pos.x, p->pos.y);
        }
    }
    return false;
}

int EventHandler::OnKey(SCITER_ELEMENT he, uint32_t evtg, void * prms)
{
    if (evtg == HANDLE_KEY)
    {
        IKeySink * keySink = (IKeySink *)m_Interface;
        KEY_PARAMS * p = (KEY_PARAMS *)prms;
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

int EventHandler::OnResize(SCITER_ELEMENT he, uint32_t evtg, void * prms)
{
    if (evtg == HANDLE_SIZE)
    {
        IResizeSink * resizeSink = (IResizeSink *)m_Interface;
        if (resizeSink)
        {
            return resizeSink->OnSizeChanged(he);
        }
        return false;
    }
    return false;
}

int EventHandler::OnForwardBehavior(SCITER_ELEMENT /*he*/, uint32_t evtg, void * prms)
{
    if (evtg == HANDLE_BEHAVIOR_EVENT)
    {
        BEHAVIOR_EVENT_PARAMS * p = (BEHAVIOR_EVENT_PARAMS *)prms;
        if ((p->cmd & (SINKING | HANDLED)) == 0)
        {
            BEHAVIOR_EVENT_PARAMS params = {};
            params.cmd = p->cmd;
            params.heTarget = (HELEMENT)m_Interface;
            params.he = (HELEMENT)m_Interface;
            params.name = p->name;
            params.data = p->data;
            SBOOL handled = false;
            SCDOM_RESULT r = SciterFireEvent(&params, false, &handled);
            assert(r == SCDOM_OK);
            (void)r;
            return handled != 0;
        }
    }
    return false;
}

int EventHandler::OnStateChange(SCITER_ELEMENT he, uint32_t evtg, void * prms)
{
    if (evtg == HANDLE_BEHAVIOR_EVENT)
    {
        BEHAVIOR_EVENT_PARAMS * p = (BEHAVIOR_EVENT_PARAMS *)prms;
        if (p->cmd == VALUE_CHANGED)
        {
            IStateChangeSink * stateChangeSink = (IStateChangeSink *)m_Interface;
            if (stateChangeSink)
            {
                return stateChangeSink->OnStateChange(he, evtg, prms);
            }
        }
    }
    return false;
}

int EventHandler::OnEventSink(SCITER_ELEMENT /*he*/, uint32_t evtg, void * prms)
{
    if (evtg == HANDLE_BEHAVIOR_EVENT)
    {
        BEHAVIOR_EVENT_PARAMS * p = (BEHAVIOR_EVENT_PARAMS *)prms;
        if ((p->cmd & (SINKING | HANDLED)) == 0)
        {
            IEventSink * eventSink = (IEventSink *)m_Interface;
            if (eventSink)
            {
                return eventSink->OnEvent(p->he, p->heTarget, p->cmd, p->reason);
            }
        }
    }
    return false;
}

} // namespace SciterUI
