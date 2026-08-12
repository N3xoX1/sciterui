module sciter.window;

import sciter.api;
import sciter.dom;
import sciter.om.types;
import sciter.utils.cvt;
import archive = sciter.utils.archive;
import std.utf;
import std.stdio;
import std.string;
import std.algorithm;

public class Window : EventHandler {
  private HWINDOW _handle;
  private bool _wired;

  private this(HWINDOW handle) { 
    _handle = handle; 
  }

  public enum {
     AS_CHILD        = (1 << 0),   /// child window only, if this flag is set all other flags ignored
     AS_MAIN         = (1 << 7),   /// main window of the app, will terminate the app on close
     AS_POPUP        = (1 << 8),   /// the window is created as topmost window.
     WITH_DEBUG_ENABLED = (1 << 9), /// make this window inspector ready
  }

  // CTOR
  public this(uint flags = 0, LPRECT placement = null) { 
    flags |= WITH_DEBUG_ENABLED;
    HWINDOW hw = SciterCreateWindow(flags, placement, null, null, null);
    this(hw);
  }

  bool load(wstring url)  { wire(); assert(url.ptr[url.length] == 0); return 0 != SciterLoadFile(_handle, &url.ptr[0]); }
  bool load(string url)   { return load(toUTF16(url)); }
  bool load(ubyte[] html, wstring baseUrl) { wire(); /*assert(baseUrl.ptr[baseUrl.length] == 0);*/ return 0 != SciterLoadHtml(_handle, &html.ptr[0], cast(uint) html.length, &baseUrl.ptr[0]); }
  bool load(ubyte[] html, string baseUrl = "") { return load(html, toUTF16(baseUrl)); }

  public enum STATE {
    CLOSED = 0, // window is destroyed
    HIDDEN = 1,
    SHOWN = 2,
    MINIMIZED = 3,
    MAXIMIZED = 4,
    FULL_SCREEN = 5,
  }

  /// get/set window visibility state: shwn normal, maximized, minimized, fullscreen, hidden 
  STATE state() { return cast(STATE)SciterWindowExec(_handle, WINDOW_EXEC_CMD.WINDOW_GET_STATE,0,0); }
  void  state(STATE st) { wire(); SciterWindowExec(_handle, WINDOW_EXEC_CMD.WINDOW_SET_STATE, st,0); }

  /// root element of the window, usually <html> element of loaded document
  Element root() const { HELEMENT h; checkDOMop(SciterGetRootElement(_handle,&h)); return Element(h); }
  /// focused element, if any
  Element focus() const { HELEMENT h; checkDOMop(SciterGetFocusElement(_handle,&h)); return Element(h); }

  Element elementAt(POINT pt) const { HELEMENT h; checkDOMop(SciterFindElement(_handle,pt,&h)); return Element(h); }

  protected {
    /// setup callbacks, this needs to be called after event handlers are set 
    void wire() {
      if(!_wired && _handle) {
        _wired = true;
        SciterSetCallback(_handle, &windowCallback, cast(void*)this);
        SciterWindowAttachEventHandler( _handle, &elementEventProc, cast(void*)this, EVENT_GROUP.ALL);
      }
    }

    //bool getAsset(ref AssetT pa) { return false; }
    //bool getAssetPassport(ref AssetPassportT pap) { return false; }

    uint handleDataLoadRequest(ref DATA_TO_LOAD dataToLoad) {
       auto url = fromStringz(dataToLoad.uri);
       if(url.startsWith("this://app/"w)) {
         // special url to get content of linked-in archive;
         const(ubyte)[] data = archive.get(url[11..$]);
         SciterDataReady(dataToLoad.hwnd, dataToLoad.uri , data.ptr, cast(UINT)(data.length));
       }
       return DATA_TO_LOAD.LOAD_OK; // proceed with the intrinsic loader
     }

    uint handleBehaviorCreateRequest(ref BEHAVIOR_TO_ATTACH request) {
       auto name = charsOf(request.behaviorName);
       if(auto controller = createElementController(name,request.element)) {
           request.elementProc = &elementEventProc;
           request.elementTag = cast(void*)controller;
           return 1;
       }
       return 0;
     }

  }

  private {

    extern(System) 
    static UINT windowCallback(SCITER_CALLBACK_NOTIFICATION* pns, LPVOID param) {

      Window window = cast(Window)param; 
      switch(pns.code) {
        case CALLBACK_NOTIFICATION_TYPE.LOAD_DATA: { 
          DATA_TO_LOAD* pd = cast(DATA_TO_LOAD*)pns;
          return window.handleDataLoadRequest(*pd); 
        }
        case CALLBACK_NOTIFICATION_TYPE.ATTACH_BEHAVIOR: { 
          BEHAVIOR_TO_ATTACH* pd = cast(BEHAVIOR_TO_ATTACH*)pns;
          return window.handleBehaviorCreateRequest(*pd); 
        }
        //case CALLBACK_NOTIFICATION_TYPE.ENGINE_DESTROYED: 
        //  that.onWindowGone(); 
        //  break;
        default: 
          break;
      }
      return 0;
    }
  }
}

private enum CALLBACK_NOTIFICATION_TYPE {
  LOAD_DATA = 0x01,
  DATA_LOADED = 0x02,
  ATTACH_BEHAVIOR = 0x04, /// a.k.a. ElementController
  ENGINE_DESTROYED = 0x05
}

struct DATA_TO_LOAD
{
    // return codes of CALLBACK_NOTIFICATION_TYPE.LOAD_DATA requests
    enum
    {
      LOAD_OK = 0,      /**< do default loading if data not set */
      LOAD_DISCARD = 1, /**< discard request completely */
      LOAD_DELAYED = 2, /**< data will be delivered later by the host application.
                             Host application must call SciterDataReadyAsync(,,, requestId) on each LOAD_DELAYED request to avoid memory leaks. */
      LOAD_MYSELF  = 3, /**< you return LOAD_MYSELF result to indicate that your application (the host) took or will take care about HREQUEST in your code completely.
                             Use sciter-x-request.h[pp] API functions with DATA_TO_LOAD::requestId handle */
    }

    UINT code;                 /**< [in] is CALLBACK_NOTIFICATION_TYPE.LOAD_DATA */
    HWINDOW hwnd;              /**< [in] HWINDOW of the window this callback was attached to.*/

    LPCWSTR  uri;              /**< [in] Zero terminated string, fully qualified uri, for example "http://server/folder/file.ext".*/

    LPCBYTE  outData;          /**< [in,out] pointer to loaded data to return. if data exists in the cache then this field contain pointer to it*/
    UINT     outDataSize;      /**< [in,out] loaded data size to return.*/
    UINT     dataType;         /**< [in] SciterResourceType */

    HREQUEST requestId;        /**< [in] request handle that can be used with sciter-x-request API */

    HELEMENT principal;
    HELEMENT initiator;
}

struct BEHAVIOR_TO_ATTACH
{
    UINT code;                        /**< [in] one of the codes above.*/
    HWINDOW hwnd;                     /**< [in] HWINDOW of the window this callback was attached to.*/

    HELEMENT element;                 /**< [in] target DOM element handle*/
    LPCSTR   behaviorName;            /**< [in] zero terminated string, string appears as value of CSS behavior:"???" attribute.*/

    ElementEventProcF elementProc;    /**< [out] pointer to ElementEventProc function.*/
    LPVOID            elementTag;     /**< [out] tag value, passed as is into pointer ElementEventProc function.*/

}


private enum WINDOW_EXEC_CMD {
  WINDOW_SET_STATE = 1, // p1 - Window.STATE, p2 - N/A
  WINDOW_GET_STATE = 2, // p1 - N/A , p2 - N/A, returns Window.STATE
  WINDOW_ACTIVATE  = 3, // p1 - BOOL, true - bring_to_front , p2 - N/A
  WINDOW_SET_PLACEMENT = 4, // p1 - const POINT*, position, p2 const SIZE* - dimension, in ppx, either one can be null
  WINDOW_GET_PLACEMENT = 5, // p1 - POINT*, position, p2 SIZE* - dimension, in ppx, either one can be null
}

