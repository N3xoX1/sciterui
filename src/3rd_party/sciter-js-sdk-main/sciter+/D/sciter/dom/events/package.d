module sciter.dom.events;

import sciter.api;
import sciter.types;
import sciter.om.types;
import sciter.dom;
public import sciter.dom.events.types;
import sciter.utils.cvt;

import std.string;
import std.utf;
import std.typecons; // for tuple
import std.sumtype;
import core.memory : GC;

class EventHandlerDef(ET) { 

  alias HEP = bool delegate(ref ET, ref Element principal);
  alias HP = bool delegate(ref Element principal);
  alias HE = bool delegate(ref ET);
  alias H = bool delegate();

  alias Handler = SumType!(HEP,HP,HE,H);

  protected uint type; 
  protected string selector; 
  protected Handler handler;

  //protected Full full; 
  //protected Basic basic; 

  this(uint t) { type = t; }

  protected bool matches(ref Element el, ref ET evt) { 
    if( evt.type != type ) return false;
    if( selector.length ) {
       el = Element(evt.target).selectNearestParent(selector);
       if(!el) return false;
    }
    return true;
  }

  protected bool invoke(ref ET event, ref Element principal) {
    //if(full) { return full(event); }
    //if(basic) return basic(); 
    bool res = false;
    handler.match!(
      (HEP a) => res = a(event,principal),
      (HP b) => res = b(principal),
      (HE c) => res = c(event),
      (H d) => res = d());
    return res;
  }

  EventHandlerDef at(string s) { selector = s; return this;}
  EventHandlerDef sinking() { type |= PHASE.SINKING; return this;}
  void opAssign(HEP h) { handler = h; }
  void opAssign(HP h) { handler = h; }
  void opAssign(HE h) { handler = h; }
  void opAssign(H h) { handler = h; }
}

// bubbling
alias MouseHandlerDef = EventHandlerDef!(MOUSE_EVENT);
alias KeyHandlerDef = EventHandlerDef!(KEY_EVENT);
alias FocusHandlerDef = EventHandlerDef!(FOCUS_EVENT);
alias TimerCallback = bool delegate();

class SemanticHandlerDef : EventHandlerDef!(SEMANTIC_EVENT) {
  protected wstring name;

  this(uint t) { super(t); }
  this(wstring n) { super(EVENT.CUSTOM); name = n;}

  override protected bool matches(ref Element el, ref SEMANTIC_EVENT evt) {
    if( evt.type == EVENT.CUSTOM ) {
      if(fromStringz!(wchar)(evt.name) != name) 
        return false;
    }
    return super.matches(el,evt);
  }
}

public class EventHandler {
 
  void didStart() {}
  void willStop() {}
  
  void eventHandlerDidAttach(HELEMENT he)  /// event handler did attach
  {
    // event handlers are passed to the engine
    GC.addRoot(cast(void*)this);
    GC.setAttr(cast(void*)this, GC.BlkAttr.NO_MOVE);
    didStart();

  }
  void eventHandlerWillDetach(HELEMENT he) /// will be detached 
  {
    willStop();
    GC.removeRoot(cast(void*)this);
    GC.clrAttr(cast(void*)this, GC.BlkAttr.NO_MOVE);    
  }

  uint eventHandlerSubscriptions() const {
      uint groups = EVENT_GROUP.MOUSE 
      | EVENT_GROUP.KEYBOARD
      | EVENT_GROUP.FOCUS
      | EVENT_GROUP.SEMANTIC_EVENT
      | EVENT_GROUP.DRAW
      | EVENT_GROUP.SCROLL
      | EVENT_GROUP.ATTRIBUTE_CHANGE
      | EVENT_GROUP.STYLE_CHANGE
      | EVENT_GROUP.SIZE
      | EVENT_GROUP.TIMER
      ;
      //if(somHandler) groups |= EVENT_GROUP.SOM; not yet
      return groups;
  } 

  /// PubSub style subscription
  MouseHandlerDef    on(MOUSE t) { MouseHandlerDef hd = new MouseHandlerDef(t); mouseHandlers ~= hd; return hd; }
  SemanticHandlerDef on(EVENT t) { SemanticHandlerDef hd = new SemanticHandlerDef(t); semanticHandlers ~= hd; return hd; }
  SemanticHandlerDef on(string t) { SemanticHandlerDef hd = new SemanticHandlerDef(toUTF16(t)); semanticHandlers ~= hd; return hd; }
  KeyHandlerDef      on(KEY t) { KeyHandlerDef hd = new KeyHandlerDef(t); keyHandlers ~= hd; return hd; }
  FocusHandlerDef    on(FOCUS t) { FocusHandlerDef hd = new FocusHandlerDef(t); focusHandlers ~= hd; return hd; }

  void startTimer(HELEMENT he, uint milliseconds, TimerCallback callback) {
    UINT_PTR hash = cast(UINT_PTR)callback.funcptr + cast(UINT_PTR)callback.ptr /*stable as not movable*/;
    SciterSetTimer(he,milliseconds,hash);
    timerCallbacks[hash] = callback;
  }

  /// low level but overridable handlers
  bool handleMouse(HELEMENT he, ref MOUSE_EVENT evt) {
      Element el = he;
      foreach (MouseHandlerDef eh; mouseHandlers) {
        if(!eh.matches(el,evt)) continue;
        if(eh.invoke(evt,el)) return true; // handled
      }
      return false;
  }
  bool handleKey(HELEMENT he, ref KEY_EVENT evt) {
      Element el = he;
      foreach (KeyHandlerDef eh; keyHandlers) {
        if(!eh.matches(el,evt)) continue;
        if(eh.invoke(evt,el)) 
          return true; // handled
      }
      return false;
  }
  bool handleFocus(HELEMENT he, ref FOCUS_EVENT evt) {
      Element el = he;
      foreach (FocusHandlerDef eh; focusHandlers) {
        if(!eh.matches(el,evt)) continue;
        if(eh.invoke(evt,el)) 
          return true; // handled
      }
      return false;
  }

  bool handleSemantic(HELEMENT he, ref SEMANTIC_EVENT evt) {
      Element el = he;
      foreach (SemanticHandlerDef eh; semanticHandlers) {
        if(!eh.matches(el,evt)) continue;
        if(eh.invoke(evt,el)) 
          return true; // handled
      }
      return false;
  }

  bool handleTimer(UINT_PTR timerId) {
        auto cb = timerCallbacks[timerId];
      if(cb) return cb();
      return false;
  }


  bool handleScroll(HELEMENT he, ref SCROLL_EVENT evt) { return false; }
  bool handleDraw(HELEMENT he, ref DRAW_EVENT evt) { return false; }
  void handleSizeChange(HELEMENT he) { }
  void handleAttributeChange(HELEMENT he,ref ATTRIBUTE_CHANGE_EVENT evt) {}
  void handleStyleChange(HELEMENT he,uint changes) {}

  MouseHandlerDef[]      mouseHandlers;
  KeyHandlerDef[]        keyHandlers;
  FocusHandlerDef[]      focusHandlers;
  SemanticHandlerDef[]   semanticHandlers;
  TimerCallback[ulong]   timerCallbacks;

}

interface ObjectModelHandler {
   bool getAsset(ref AssetT pa);
   bool getAssetPassport(ref AssetPassportT pap);
} 

// ElementEventProcF implementeation
extern(System) 
SBOOL elementEventProc(LPVOID tag, HELEMENT he, UINT evtg, LPVOID param ) 
{
  auto self = cast(EventHandler)tag;

  switch( evtg )
  {
      case EVENT_GROUP.LIFECYCLE:
        {
          LIFECYCLE_EVENT *p = cast(LIFECYCLE_EVENT *)param;
          if (p.type == LIFECYCLE_EVENT.DETACH) {
            self.eventHandlerWillDetach(he);
          }
          else if (p.type == LIFECYCLE_EVENT.ATTACH) {
            self.eventHandlerDidAttach(he);
          }
          return true;
        }

      case EVENT_GROUP.SUBSCRIPTIONS:
        {
          UINT *p = cast(UINT*)param;
          *p = self.eventHandlerSubscriptions;
          return 1;
        }
      case EVENT_GROUP.SOM: {  
        SOM_EVENT *p = cast(SOM_EVENT *)param; 
        if(auto pif = cast(ObjectModelHandler)self) {
          switch(p.type) {
            case SOM_EVENT.GET_PASSPORT:
              return pif.getAssetPassport(*p.passport);    
            case SOM_EVENT.GET_ASSET:
              return pif.getAsset(*p.asset);    
            default:
              break;
          }
        }
        return false;
      }
      case EVENT_GROUP.MOUSE: {  
        MOUSE_EVENT *p = cast(MOUSE_EVENT *)param; 
        return self.handleMouse(he,*p);  
      }
      case EVENT_GROUP.KEYBOARD:   {  
        KEY_EVENT *p = cast(KEY_EVENT *)param; 
        return self.handleKey(he,*p);  
      }
      case EVENT_GROUP.FOCUS: {  
        FOCUS_EVENT *p = cast(FOCUS_EVENT *)param; 
        return self.handleFocus(he,*p);  
      }
      case EVENT_GROUP.SEMANTIC_EVENT: { 
        SEMANTIC_EVENT *p = cast(SEMANTIC_EVENT *)param; 
        return self.handleSemantic(he,*p);  
      }
      case EVENT_GROUP.DRAW:  {  
        DRAW_EVENT *p = cast(DRAW_EVENT *)param; 
        return self.handleDraw(he,*p);  
      }
      case EVENT_GROUP.SCROLL: { 
        SCROLL_EVENT *p = cast(SCROLL_EVENT *)param; 
        return self.handleScroll(he,*p);
      }      
      case EVENT_GROUP.SIZE: {
        self.handleSizeChange(he);
        return false;
      }
      case EVENT_GROUP.ATTRIBUTE_CHANGE: {
        ATTRIBUTE_CHANGE_EVENT *p = cast(ATTRIBUTE_CHANGE_EVENT *)param;
        self.handleAttributeChange(he,*p);
        return false;
      } 
      case EVENT_GROUP.STYLE_CHANGE: {
        UINT changeKind = cast(UINT)param;
        self.handleStyleChange(he,changeKind);
        return false;        
      }
      case EVENT_GROUP.TIMER: {
        TIMER_EVENT *p = cast(TIMER_EVENT *)param;
        return self.handleTimer(p.timerId);
      }

      case EVENT_GROUP.SCRIPTING_METHOD_CALL: break;
      case EVENT_GROUP.GESTURE : break;
      case EVENT_GROUP.EXCHANGE: break;
      case EVENT_GROUP.METHOD_CALL: break;
      case EVENT_GROUP.DATA_ARRIVED: break;
      default:
        assert(false);
        break;
  }
  return false;
}
