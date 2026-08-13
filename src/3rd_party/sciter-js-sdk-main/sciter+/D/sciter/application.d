module sciter.application;

import std.stdio;
import std.string;
import std.conv;
import std.exception;
import std.algorithm;
import std.array;
import std.utf;
import sciter.api;
import sciter.om.value;
import sciter.utils.cvt;

static this() {
  enforce(loadApi(), "Failed to load Sciter Engine");  
}

struct Application {
  this(string[] args) { 

    SciterSetOption(null, SCITER_RT_OPTION.SET_DEBUG_MODE, 1); // enable inspector 
    
    SciterSetOption(null, SCITER_RT_OPTION.SET_SCRIPT_RUNTIME_FEATURES,
      SCRIPT_RUNTIME_FEATURE.ALLOW_FILE_IO |
      SCRIPT_RUNTIME_FEATURE.ALLOW_SOCKET_IO |
      SCRIPT_RUNTIME_FEATURE.ALLOW_EVAL |
      SCRIPT_RUNTIME_FEATURE.ALLOW_SYSINFO); // these options are needed for communication with the inspector 

    auto wargs = args.map!( arg => toUTF16(arg) ).array; 
    auto wargsz = wargs.map!( warg => warg.toWstringz ).array;
    enforce(SciterExec(SCITER_APP_CMD.INIT,cast(UINT_PTR)wargsz.length,cast(UINT_PTR)wargsz.ptr), "Failed to initialize Sciter Engine");  
  }
  ~this() {
    SciterExec(SCITER_APP_CMD.SHUTDOWN,0,0);
    unloadApi();
  }

  /// run "message pump" loop until quit() issued explicitly or main window closed
  int  run() { return to!int(SciterExec(SCITER_APP_CMD.LOOP,0,0)); }
  /// run one iteration of "message pump" - get and dispatch messages 
  bool runIteration() { return to!bool(SciterExec(SCITER_APP_CMD.LOOP_ITERATION,0,0)); }
  /// run timing checks, no message retreival is happening
  bool runHeartbit() { return to!bool(SciterExec(SCITER_APP_CMD.LOOP_HEARTBIT,0,0)); }
  /// request quit from the app, all windows will be sent requests for closure (that they can reject) 
  bool requestQuit(int retval) { return to!bool(SciterExec(SCITER_APP_CMD.STOP,retval,0)); }

  /// Sets global variable accessible by JS in all windows of the app
  /// 
  /// Params: 
  ///   path - usually is just a name of a global variable that will be available in JS
  ///          as any other JS global like `Window`, `globalThis`, etc. 
  ///   toSet - D entity wrapped into VALUE, can be any atomic value, struct or some callable (e.g. function).
  void globalVar( string path, VALUE toSet ) { SciterSetVariable(null,path.toStringz, &toSet); } 

  /// Gets global variable value
  /// 
  /// Params: 
  ///   path - could be a name like `API` or a path `API.something`
  /// Returns: the value 

  VALUE globalVar( string path ) { VALUE toGet; SciterGetVariable(null,path.toStringz, &toGet); return toGet; }
}
/*
bool start() { 
    if(!loadApi()) return false;

    SciterSetOption(null, SCITER_RT_OPTION.SET_DEBUG_MODE, 1); // enable inspector 
    
    SciterSetOption(null, SCITER_RT_OPTION.SET_SCRIPT_RUNTIME_FEATURES,
      SCRIPT_RUNTIME_FEATURE.ALLOW_FILE_IO |
      SCRIPT_RUNTIME_FEATURE.ALLOW_SOCKET_IO |
      SCRIPT_RUNTIME_FEATURE.ALLOW_EVAL |
      SCRIPT_RUNTIME_FEATURE.ALLOW_SYSINFO); // these options are needed for communication with the inspector 
    
    return to!bool(SciterExec(SCITER_APP_CMD.INIT,0,0)); 
}
void shutdown() { SciterExec(SCITER_APP_CMD.SHUTDOWN,0,0); unloadApi(); } // uncoditonal terminate
int  run() { return to!int(SciterExec(SCITER_APP_CMD.LOOP,0,0)); }
bool runIteration() { return to!bool(SciterExec(SCITER_APP_CMD.LOOP_ITERATION,0,0)); }
bool runHeartbit() { return to!bool(SciterExec(SCITER_APP_CMD.LOOP_HEARTBIT,0,0)); }
bool quit(int retval) { return to!bool(SciterExec(SCITER_APP_CMD.STOP,retval,0)); }

/// Sets global variable accessible by JS in all windows of the app
/// 
/// Params: 
///   path - usually is just a name of a global variable that will be available in JS
///          as any other JS global like `Window`, `globalThis`, etc. 
///   toSet - D entity wrapped into VALUE, can be any atomic value, struct or some callable (e.g. function).
void globalVar( string path, VALUE toSet ) { SciterSetVariable(null,path.toStringz, &toSet); } 

/// Gets global variable value
/// 
/// Params: 
///   path - could be a name like `API` or a path `API.something`
/// Returns: the value 

VALUE globalVar( string path ) { VALUE toGet; SciterGetVariable(null,path.toStringz, &toGet); return toGet; }

*/
