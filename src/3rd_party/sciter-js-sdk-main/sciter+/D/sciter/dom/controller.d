module sciter.dom.controller;

import sciter.types;
import sciter.dom;

/// a.k.a. Behavior - native controller of DOM element,
///                   essentially this allow to make custom element types
/// Usually controllers are created by CSS:
///
///   div.my-element { behavior: my-element-controller; }
///
/// This will create instance of 
///    class MyElementController : ElementController {...}  
/// that was globally registered by calling in D:
///    registerElementController!MyElementController("my-element-controller");

public class ElementController : EventHandler 
{
  HELEMENT _self;

  this() { _self = null; }
  this(HELEMENT he) { _self = he; }

  Element self() const { return Element(_self); }
  
  void startTimer(uint milliseconds, TimerCallback callback) {  super.startTimer(_self, milliseconds, callback); }
}

private { 

  alias ElementControllerFactory = ElementController function(HELEMENT he);

  ElementControllerFactory[string] list;

  template MakeElementControllerFactory(T)
  {
    ElementController factory(HELEMENT he) { return new T(he); }
  }
 
}

public void registerElementController(T)(string name) {
  list[name] = &MakeElementControllerFactory!T.factory;
}

public ElementController createElementController(string name, HELEMENT he) {
  auto factory = list[name];
  if( factory ) 
    return factory(he);
  return null;
}

